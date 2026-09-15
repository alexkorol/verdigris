using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;
using System.Text;
using System.Security.Cryptography;
using System.Windows.Forms;

// Native entry point: explicit online service mode or preserved local review.
internal static class PlayerLauncher
{
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    static extern IntPtr CreateJobObject(IntPtr attributes, string name);
    [DllImport("kernel32.dll")]
    static extern bool SetInformationJobObject(IntPtr job, int kind, IntPtr info, uint length);
    [DllImport("kernel32.dll")]
    static extern bool AssignProcessToJobObject(IntPtr job, IntPtr process);
    [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern bool QueryFullProcessImageName(IntPtr process, uint flags, StringBuilder name, ref uint size);
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
    static Process Start(string exe, string args, string root, string profile, bool settingsOverride, IntPtr job, bool online = false) {
        var info = new ProcessStartInfo(exe, args) {
            WorkingDirectory = root, UseShellExecute = false, CreateNoWindow = true,
            RedirectStandardInput = true, RedirectStandardOutput = true, RedirectStandardError = true
        };
        if (online) {
            info.EnvironmentVariables.Remove("VERDIGRIS_SAVE_DIR");
            info.EnvironmentVariables["VERDIGRIS_SERVICE_PROFILE"] = profile;
        } else {
            info.EnvironmentVariables["VERDIGRIS_SAVE_DIR"] = Path.Combine(profile, "saves");
            info.EnvironmentVariables.Remove("VERDIGRIS_SERVICE_PROFILE");
        }
        if (settingsOverride) info.EnvironmentVariables["VERDIGRIS_SETTINGS_PATH"] = Path.Combine(profile, "settings.ini");
        else info.EnvironmentVariables.Remove("VERDIGRIS_SETTINGS_PATH"); // Normal launch must not inherit a QA settings override.
        var process = Process.Start(info);
        if (!AssignProcessToJobObject(job, process.Handle)) {
            process.Kill(); process.WaitForExit(); process.Dispose();
            throw new InvalidOperationException("Cannot attach game process to cleanup group.");
        }
        Log(Path.GetFileName(exe) + " pid=" + process.Id + " " + args);
        // Query the process handle directly: MainModule enumeration can race
        // startup and return a null module even after Process.Start succeeds.
        var image = new StringBuilder(32768); uint imageSize = (uint)image.Capacity;
        if (!QueryFullProcessImageName(process.Handle, 0, image, ref imageSize)) {
            int error = Marshal.GetLastWin32Error();
            Stop(process, false);
            throw new InvalidOperationException("Cannot identify launched game process (Win32 " + error + ").");
        }
        Log("image=" + image.ToString() + " cwd=" + root);
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
    static string ValidateOnlineEndpoint(string endpoint) {
        if (string.IsNullOrEmpty(endpoint) || endpoint.Length > 2048) throw new ArgumentException("Enter a service endpoint of at most 2048 characters.");
        foreach (char c in endpoint) if (c <= 32 || c >= 127 || c == '\\' || c == '"' || c == '?' || c == '#' || c == '@')
            throw new ArgumentException("Service endpoints cannot contain credentials, queries, fragments, spaces or quotes.");
        Uri uri;
        if ((!endpoint.StartsWith("wss://", StringComparison.Ordinal) && !endpoint.StartsWith("ws://", StringComparison.Ordinal)) ||
            !Uri.TryCreate(endpoint, UriKind.Absolute, out uri) || string.IsNullOrEmpty(uri.Host) || uri.Port < 1 || uri.Port > 65535)
            throw new ArgumentException("Use wss://hostname[:port]/path for the service endpoint.");
        // Check the original host too: Uri normalizes ambiguous 127.1 spellings.
        string authority = endpoint.Substring(endpoint.IndexOf("://", StringComparison.Ordinal) + 3).Split('/')[0];
        string rawHost = authority.StartsWith("[", StringComparison.Ordinal) ? authority.Substring(0, authority.IndexOf(']') + 1) : authority.Split(':')[0];
        if (uri.Scheme == "ws" && rawHost != "127.0.0.1" && !rawHost.Equals("localhost", StringComparison.OrdinalIgnoreCase) && rawHost != "[::1]")
            throw new ArgumentException("Unencrypted ws:// is available only for explicit loopback QA. Online services require wss://.");
        return endpoint;
    }
    static string DefaultOnlineProfile(string endpoint) {
        using (var hash = SHA256.Create()) {
            byte[] bytes = hash.ComputeHash(Encoding.UTF8.GetBytes(new Uri(endpoint).AbsoluteUri));
            string key = BitConverter.ToString(bytes).Replace("-", "").ToLowerInvariant();
            return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Verdigris", "online", key);
        }
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
        string verifyLaunch = null;
        try {
            string root = AppDomain.CurrentDomain.BaseDirectory;
            string profile = Path.Combine(root, "profile"); bool isolated = false, quick = false; string onlineEndpoint = null;
            for (int i = 0; i < args.Length; ++i) {
                if (args[i] == "--profile" && i + 1 < args.Length) { profile = Path.GetFullPath(args[++i]); isolated = true; }
                else if (args[i] == "--quick") quick = true;
                else if (args[i] == "--online" && i + 1 < args.Length) onlineEndpoint = ValidateOnlineEndpoint(args[++i]);
                else if (args[i] == "--verify-launch" && i + 1 < args.Length) verifyLaunch = args[++i];
                else throw new ArgumentException("Usage: Verdigris.exe [--online <service endpoint>] [--profile <isolated directory>] [--quick] [--verify-launch save|reload|smoke]");
            }
            if (onlineEndpoint != null) {
                if (quick || verifyLaunch != null) throw new ArgumentException("Online mode uses the authenticated native account flow; local quick/review verification arguments do not apply.");
                if (!isolated) profile = DefaultOnlineProfile(onlineEndpoint);
                if (Directory.Exists(Path.Combine(profile, "saves")) ||
                    string.Equals(profile.TrimEnd('\\'), Path.Combine(root, "profile").TrimEnd('\\'), StringComparison.OrdinalIgnoreCase))
                    throw new ArgumentException("Choose a separate online profile; existing local-review saves cannot be used for a service account.");
                isolated = true;
            }
            if (verifyLaunch == "smoke" && (isolated || quick))
                throw new ArgumentException("Normal-launch smoke uses the normal profile without QA or quick-start arguments.");
            if (verifyLaunch != null && verifyLaunch != "smoke" && (!isolated || (verifyLaunch != "save" && verifyLaunch != "reload") ||
                string.Equals(profile.TrimEnd('\\'), Path.Combine(root, "profile").TrimEnd('\\'), StringComparison.OrdinalIgnoreCase)))
                throw new ArgumentException("Launch verification requires a separate explicit QA profile and save or reload phase.");
            Directory.CreateDirectory(profile);
            if (onlineEndpoint == null) Directory.CreateDirectory(Path.Combine(profile, "saves"));
            Directory.CreateDirectory(Path.Combine(profile, "logs"));
            // Lock the actual directory, so trailing separators and junction aliases
            // cannot create two server writers for the same review saves.
            profileLock = AcquireProfileLock(profile);
            logPath = Path.Combine(profile, "logs", "session-" + DateTime.UtcNow.ToString("yyyyMMdd-HHmmss-fff") + ".log");
            Log("root=" + root + " profile=" + profile + " isolatedSettings=" + isolated);
            string serverExe = Path.Combine(root, "native", "build", "verdigris_server.exe");
            string clientExe = Path.Combine(root, "native", "build", "verdigris_client.exe");
            if (!File.Exists(clientExe) || (onlineEndpoint == null && !File.Exists(serverExe))) throw new FileNotFoundException("Game files are missing. Extract the complete native package and run Verdigris.exe.");
            int port = 0; job = NewJob();
            if (onlineEndpoint == null) {
                port = FreePort();
                server = Start(serverExe, port.ToString(), root, profile, isolated, job);
                server.OutputDataReceived += delegate(object sender, DataReceivedEventArgs e) {
                    if (e.Data == null) return; Log("server: " + e.Data);
                    if (e.Data == "verdigris_server listening on ws://127.0.0.1:" + port) ready.Set();
                };
                server.ErrorDataReceived += delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) Log("server error: " + e.Data); };
                server.BeginOutputReadLine(); server.BeginErrorReadLine();
                if (!ready.WaitOne(12000) || server.HasExited) throw new InvalidOperationException("The local game server did not become ready. See " + logPath);
            }
            string clientArgs = onlineEndpoint == null ? "--remote 127.0.0.1 " + port + " review-player" + (quick ? " --quick" : "") +
                (verifyLaunch == null ? "" : " --verify-launch " + verifyLaunch) : "--online \"" + onlineEndpoint + "\"";
            client = Start(clientExe, clientArgs, root, profile, isolated, job, onlineEndpoint != null);
            client.OutputDataReceived += delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) Log("client: " + e.Data); };
            client.ErrorDataReceived += delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) Log("client error: " + e.Data); };
            client.BeginOutputReadLine(); client.BeginErrorReadLine();
            client.WaitForExit(); int exitCode = client.ExitCode;
            Log("client exit=" + exitCode);
            if (exitCode != 0) throw new InvalidOperationException("The game exited unexpectedly. See " + logPath);
            return 0;
        } catch (Exception error) {
            if (logPath != null) { try { Log("FAILED: " + error); } catch { } }
            if (verifyLaunch == null) MessageBox.Show(error.Message, "Verdigris could not start", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        } finally {
            try { Stop(client, false); } finally {
                try { Stop(server, true); } finally { if (job != IntPtr.Zero) CloseHandle(job); ready.Dispose(); if (profileLock != null) profileLock.Dispose(); }
            }
        }
    }
}
