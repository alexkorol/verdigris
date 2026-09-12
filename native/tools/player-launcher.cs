using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;

// Unsigned local-review entry point. No developer console or checkout required.
internal static class PlayerLauncher
{
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    static extern IntPtr CreateJobObject(IntPtr attributes, string name);
    [DllImport("kernel32.dll")]
    static extern bool SetInformationJobObject(IntPtr job, int kind, IntPtr info, uint length);
    [DllImport("kernel32.dll")]
    static extern bool AssignProcessToJobObject(IntPtr job, IntPtr process);
    [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
    [StructLayout(LayoutKind.Sequential)] struct BasicLimits {
        public long ProcessTime, JobTime; public uint Flags;
        public UIntPtr MinimumWorkingSet, MaximumWorkingSet;
        public uint ActiveProcessLimit; public UIntPtr Affinity;
        public uint Priority, Scheduling;
    }
    [StructLayout(LayoutKind.Sequential)] struct IoCounters {
        public ulong ReadOps, WriteOps, OtherOps, ReadBytes, WriteBytes, OtherBytes;
    }
    [StructLayout(LayoutKind.Sequential)] struct ExtendedLimits {
        public BasicLimits Basic; public IoCounters Io;
        public UIntPtr ProcessMemory, JobMemory, PeakProcessMemory, PeakJobMemory;
    }
    static readonly object LogLock = new object();
    static string logPath;
    static void Log(string text) { lock (LogLock) File.AppendAllText(logPath, DateTime.UtcNow.ToString("O") + " " + text + Environment.NewLine); }

    static IntPtr NewJob() {
        IntPtr job = CreateJobObject(IntPtr.Zero, null);
        if (job == IntPtr.Zero) throw new InvalidOperationException("Cannot create game process group.");
        var limits = new ExtendedLimits(); limits.Basic.Flags = 0x2000; // KILL_ON_JOB_CLOSE
        int size = Marshal.SizeOf(limits); IntPtr data = Marshal.AllocHGlobal(size);
        try {
            Marshal.StructureToPtr(limits, data, false);
            if (!SetInformationJobObject(job, 9, data, (uint)size)) {
                CloseHandle(job); throw new InvalidOperationException("Cannot configure game process cleanup.");
            }
        } finally { Marshal.FreeHGlobal(data); }
        return job;
    }
    static Process Start(string exe, string args, string root, string profile, bool settingsOverride, IntPtr job) {
        var info = new ProcessStartInfo(exe, args) {
            WorkingDirectory = root, UseShellExecute = false, CreateNoWindow = true,
            RedirectStandardInput = true, RedirectStandardOutput = true, RedirectStandardError = true
        };
        info.EnvironmentVariables["VERDIGRIS_SAVE_DIR"] = Path.Combine(profile, "saves");
        if (settingsOverride) info.EnvironmentVariables["VERDIGRIS_SETTINGS_PATH"] = Path.Combine(profile, "settings.ini");
        var process = Process.Start(info);
        if (!AssignProcessToJobObject(job, process.Handle)) {
            process.Kill(); process.WaitForExit(); process.Dispose();
            throw new InvalidOperationException("Cannot attach game process to cleanup group.");
        }
        Log(Path.GetFileName(exe) + " pid=" + process.Id + " " + args);
        return process;
    }
    static int FreePort() {
        for (int port = 6520; port <= 6539; ++port) {
            var probe = new TcpListener(IPAddress.Loopback, port);
            try { probe.Start(); return port; }
            catch (SocketException) { }
            finally { probe.Stop(); }
        }
        throw new InvalidOperationException("No free local game port (6520-6539). Close another Verdigris review session and retry.");
    }
    static FileStream AcquireProfileLock(string profile) {
        try { return new FileStream(Path.Combine(profile, "session.lock"), FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None); }
        catch (IOException) { throw new InvalidOperationException("This review profile is already open or unavailable."); }
    }
    static void Stop(Process process, bool graceful) {
        if (process == null) return;
        try {
            if (!process.HasExited && graceful) { process.StandardInput.WriteLine("quit"); process.StandardInput.Flush(); }
            if (!process.HasExited && !process.WaitForExit(5000)) { process.Kill(); process.WaitForExit(); }
            Log("cleaned pid=" + process.Id + " exit=" + process.ExitCode);
        } finally { process.Dispose(); }
    }
    [STAThread] static int Main(string[] args) {
        IntPtr job = IntPtr.Zero; Process server = null, client = null; FileStream profileLock = null;
        var ready = new ManualResetEvent(false);
        try {
            string root = AppDomain.CurrentDomain.BaseDirectory;
            string profile = Path.Combine(root, "profile"); bool isolated = false, quick = false;
            for (int i = 0; i < args.Length; ++i) {
                if (args[i] == "--profile" && i + 1 < args.Length) { profile = Path.GetFullPath(args[++i]); isolated = true; }
                else if (args[i] == "--quick") quick = true;
                else throw new ArgumentException("Usage: Verdigris.exe [--profile <isolated review directory>] [--quick]");
            }
            Directory.CreateDirectory(profile); Directory.CreateDirectory(Path.Combine(profile, "saves"));
            Directory.CreateDirectory(Path.Combine(profile, "logs"));
            // Lock the actual directory, so trailing separators and junction aliases
            // cannot create two server writers for the same review saves.
            profileLock = AcquireProfileLock(profile);
            logPath = Path.Combine(profile, "logs", "session-" + DateTime.UtcNow.ToString("yyyyMMdd-HHmmss-fff") + ".log");
            Log("root=" + root + " profile=" + profile + " isolatedSettings=" + isolated);
            string serverExe = Path.Combine(root, "native", "build", "verdigris_server.exe");
            string clientExe = Path.Combine(root, "native", "build", "verdigris_client.exe");
            if (!File.Exists(serverExe) || !File.Exists(clientExe)) throw new FileNotFoundException("Game files are missing. Extract the complete review package and run Verdigris.exe.");
            int port = FreePort(); job = NewJob();
            server = Start(serverExe, port.ToString(), root, profile, isolated, job);
            {
                server.OutputDataReceived += delegate(object sender, DataReceivedEventArgs e) {
                    if (e.Data == null) return; Log("server: " + e.Data);
                    if (e.Data == "verdigris_server listening on ws://127.0.0.1:" + port) ready.Set();
                };
                server.ErrorDataReceived += delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) Log("server error: " + e.Data); };
                server.BeginOutputReadLine(); server.BeginErrorReadLine();
                if (!ready.WaitOne(12000) || server.HasExited) throw new InvalidOperationException("The local game server did not become ready. See " + logPath);
            }
            client = Start(clientExe, "--remote 127.0.0.1 " + port + " review-player" + (quick ? " --quick" : ""), root, profile, isolated, job);
            client.OutputDataReceived += delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) Log("client: " + e.Data); };
            client.ErrorDataReceived += delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) Log("client error: " + e.Data); };
            client.BeginOutputReadLine(); client.BeginErrorReadLine();
            client.WaitForExit(); int exitCode = client.ExitCode;
            Log("client exit=" + exitCode);
            if (exitCode != 0) throw new InvalidOperationException("The game exited unexpectedly. See " + logPath);
            return 0;
        } catch (Exception error) {
            if (logPath != null) { try { Log("FAILED: " + error); } catch { } }
            MessageBox.Show(error.Message, "Verdigris could not start", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        } finally {
            try { Stop(client, false); } finally {
                try { Stop(server, true); } finally { if (job != IntPtr.Zero) CloseHandle(job); ready.Dispose(); if (profileLock != null) profileLock.Dispose(); }
            }
        }
    }
}
