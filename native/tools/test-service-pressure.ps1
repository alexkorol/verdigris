param([Parameter(Mandatory = $true)][string]$ServerExecutable, [string]$EvidenceDirectory)
$ErrorActionPreference = 'Stop'
if (-not $EvidenceDirectory) { $EvidenceDirectory = Join-Path $PSScriptRoot ('../build/service-pressure-' + [Guid]::NewGuid().ToString('N')) }
$root = [IO.Path]::GetFullPath($EvidenceDirectory)
if (Test-Path -LiteralPath $root) { throw 'Use a fresh pressure-test evidence directory.' }
New-Item -ItemType Directory -Path $root | Out-Null
$source = [IO.Path]::GetFullPath($ServerExecutable)
$sourceHash = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
$server = Join-Path $root 'verdigris_server.exe'
Copy-Item -LiteralPath $source -Destination $server
if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ne $sourceHash -or (Get-FileHash -LiteralPath $server -Algorithm SHA256).Hash -ne $sourceHash) { throw 'Server executable changed during copy.' }
$identity = (& $server --build-info | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw 'Cannot read copied server build identity.' }
$data = Join-Path $root 'data'
$codeA = Join-Path $root 'enrollment-a.txt'; $codeB = Join-Path $root 'enrollment-b.txt'
foreach ($output in @($codeA, $codeB)) {
  & $server --enroll --data $data --output $output | Out-Null
  if ($LASTEXITCODE -ne 0) { throw 'Disposable enrollment failed.' }
}
$a = (Get-Content -LiteralPath $codeA -Raw).Trim(); $b = (Get-Content -LiteralPath $codeB -Raw).Trim()
if ($a -notmatch '^enr_[0-9a-f]{64}$' -or $b -notmatch '^enr_[0-9a-f]{64}$') { throw 'Invalid disposable enrollment format.' }
$probe = New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback, 0)
$probe.Start(); $port = $probe.LocalEndpoint.Port; $probe.Stop()
$info = New-Object Diagnostics.ProcessStartInfo
$info.FileName = $server; $info.Arguments = '--service --data "' + $data + '" --port ' + $port + ' --qa-policy coop-v1'
$info.UseShellExecute = $false; $info.CreateNoWindow = $true
$info.RedirectStandardInput = $true; $info.RedirectStandardOutput = $true; $info.RedirectStandardError = $true
$process = [Diagnostics.Process]::Start($info)
$stdout = $process.StandardOutput.ReadToEndAsync(); $stderr = $process.StandardError.ReadToEndAsync()
function Invoke-Control([string[]]$Arguments) { $ErrorActionPreference = 'Continue'; & $server @Arguments 2>&1 | Out-Null; return $LASTEXITCODE }
$code = @'
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net.Sockets;
using System.Net.WebSockets;
using System.Text;
using System.Threading;
using System.Web.Script.Serialization;
public static class VerdigrisPressure {
    public static string Stage = "starting";
    sealed class Samples : IDisposable {
        readonly object gate = new object(); readonly int pid; readonly Timer timer;
        long maxPrivate, maxWorking; int maxThreads, maxHandles;
        public Samples(int processId) { pid = processId; Sample(null); timer = new Timer(Sample, null, 50, 50); }
        void Sample(object ignored) {
            try { using (var process = Process.GetProcessById(pid)) {
                lock (gate) {
                    maxPrivate = Math.Max(maxPrivate, process.PrivateMemorySize64); maxWorking = Math.Max(maxWorking, process.WorkingSet64);
                    maxThreads = Math.Max(maxThreads, process.Threads.Count); maxHandles = Math.Max(maxHandles, process.HandleCount);
                }
            } } catch (InvalidOperationException) {} catch (ArgumentException) {}
        }
        public void Write(Dictionary<string, object> result) {
            Sample(null);
            lock (gate) {
                result["sampledPeakPrivateBytes"] = maxPrivate; result["sampledPeakWorkingSetBytes"] = maxWorking;
                result["sampledPeakThreads"] = maxThreads; result["sampledPeakHandles"] = maxHandles;
                result["resourceSampleIntervalMilliseconds"] = 50;
            }
        }
        public void Dispose() { timer.Dispose(); }
    }
    static readonly JavaScriptSerializer Json = new JavaScriptSerializer { MaxJsonLength = 2 * 1024 * 1024, RecursionLimit = 64 };
    static void Require(bool condition, string message) { if (!condition) throw new InvalidOperationException(message); }
    static string Envelope(string name, object data) { return Json.Serialize(new { @event = name, data = data }); }
    sealed class Peer : IDisposable {
        public readonly ClientWebSocket Socket = new ClientWebSocket();
        public Peer(int port) { using (var cancel = new CancellationTokenSource(4000)) Socket.ConnectAsync(new Uri("ws://127.0.0.1:" + port + "/game"), cancel.Token).GetAwaiter().GetResult(); }
        public void Send(string message) {
            byte[] bytes = Encoding.UTF8.GetBytes(message);
            using (var cancel = new CancellationTokenSource(4000)) Socket.SendAsync(new ArraySegment<byte>(bytes), WebSocketMessageType.Text, true, cancel.Token).GetAwaiter().GetResult();
        }
        public Dictionary<string, object> Until(string name) {
            using (var cancel = new CancellationTokenSource(4000)) {
                byte[] buffer = new byte[16384];
                for (int messages = 0; messages < 128; ++messages) {
                    using (var message = new MemoryStream()) {
                        WebSocketReceiveResult received;
                        do {
                            received = Socket.ReceiveAsync(new ArraySegment<byte>(buffer), cancel.Token).GetAwaiter().GetResult();
                            Require(received.MessageType == WebSocketMessageType.Text, "Healthy peer was closed or received binary content.");
                            message.Write(buffer, 0, received.Count);
                            Require(message.Length <= 1024 * 1024, "Healthy response exceeds message bound.");
                        } while (!received.EndOfMessage);
                        var envelope = Json.Deserialize<Dictionary<string, object>>(Encoding.UTF8.GetString(message.ToArray()));
                        if (envelope.ContainsKey("event") && (string)envelope["event"] == name) return (Dictionary<string, object>)envelope["data"];
                    }
                }
            }
            throw new InvalidOperationException("Expected service event not observed within bounded response count.");
        }
        public double Ping() { var time = Stopwatch.StartNew(); Send(Envelope("service:ping", new {})); Until("service:pong"); return time.Elapsed.TotalMilliseconds; }
        public string Authenticate(string credential, bool enroll) {
            Send(Envelope("service:authenticate", new { credential = credential, enroll = enroll, protocolVersion = 1 }));
            var admitted = Until("service:authenticated");
            Require(admitted.ContainsKey("accountId") && admitted.ContainsKey("token"), "Authentication lacked account/token.");
            return (string)admitted["token"];
        }
        public void Dispose() { Socket.Abort(); Socket.Dispose(); }
    }
    sealed class Wire : IDisposable {
        public readonly TcpClient Client = new TcpClient();
        public readonly NetworkStream Stream;
        public Wire(int port, bool handshake, bool smallWindow = false) {
            Client.NoDelay = true;
            if (smallWindow) Client.ReceiveBufferSize = 1024;
            var connect = Client.ConnectAsync("127.0.0.1", port);
            Require(connect.Wait(3000), "Wire connect deadline exceeded."); connect.GetAwaiter().GetResult();
            Stream = Client.GetStream(); Stream.ReadTimeout = 4000; Stream.WriteTimeout = 2000;
            if (handshake) {
                byte[] request = Encoding.ASCII.GetBytes("GET /game HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n");
                Stream.Write(request, 0, request.Length);
                var header = new StringBuilder();
                while (!header.ToString().EndsWith("\r\n\r\n", StringComparison.Ordinal) && header.Length < 8192) { int value = Stream.ReadByte(); Require(value >= 0, "Wire upgrade was refused."); header.Append((char)value); }
                Require(header.ToString().StartsWith("HTTP/1.1 101", StringComparison.Ordinal), "Wire did not upgrade.");
            }
        }
        public void Write(byte[] bytes) { Stream.Write(bytes, 0, bytes.Length); }
        public void Text(string text) { Write(Frame(text)); }
        public static byte[] Frame(string text) {
            byte[] bytes = Encoding.UTF8.GetBytes(text);
            using (var frame = new MemoryStream()) {
                frame.WriteByte(0x81);
                if (bytes.Length < 126) frame.WriteByte((byte)(0x80 | bytes.Length));
                else { frame.WriteByte(0xfe); frame.WriteByte((byte)(bytes.Length >> 8)); frame.WriteByte((byte)bytes.Length); }
                byte[] mask = { 0x23, 0x41, 0x55, 0x09 }; frame.Write(mask, 0, mask.Length);
                for (int i = 0; i < bytes.Length; ++i) frame.WriteByte((byte)(bytes[i] ^ mask[i % 4]));
                return frame.ToArray();
            }
        }
        public int Closed() {
            int bytes = 0;
            try {
                byte[] buffer = new byte[8192]; var deadline = Stopwatch.StartNew();
                while (deadline.ElapsedMilliseconds < 4500 && bytes <= 4 * 1024 * 1024) {
                    int count = Stream.Read(buffer, 0, buffer.Length); if (count == 0) return bytes; bytes += count;
                }
            } catch (IOException error) {
                var socket = error.InnerException as SocketException;
                if (socket != null && socket.SocketErrorCode != SocketError.TimedOut) return bytes;
                throw;
            }
            throw new InvalidOperationException("Offending connection was not closed within read/byte bounds.");
        }
        public void Dispose() { Stream.Dispose(); Client.Close(); }
    }
    public static string Run(int port, int processId, string codeA, string codeB) {
        using (var samples = new Samples(processId)) {
        var result = new Dictionary<string, object>(); var clock = Stopwatch.StartNew();
        var process = Process.GetProcessById(processId); process.Refresh();
        long baselinePrivate = process.PrivateMemorySize64, maxPrivate = baselinePrivate, maxWorking = process.WorkingSet64;
        int maxThreads = process.Threads.Count, maxHandles = process.HandleCount;
        double cpuStart = process.TotalProcessorTime.TotalMilliseconds;
        var pingTimes = new List<double>(); string token;
        using (var healthy = new Peer(port)) {
            Stage = "initial authentication";
            token = healthy.Authenticate(codeA, true); pingTimes.Add(healthy.Ping());
            var churn = Stopwatch.StartNew();
            for (int i = 0; i < 1152; ++i) {
                Stage = "malformed handshake churn " + i;
                using (var peer = new Wire(port, false)) {
                    peer.Write(Encoding.ASCII.GetBytes("GET /game HTTP/1.1\r\nHost: pressure.invalid\r\n\r\n"));
                    Require(peer.Closed() == 0, "Malformed handshake received an upgrade response.");
                }
                if ((i + 1) % 128 == 0) {
                    process.Refresh(); maxPrivate = Math.Max(maxPrivate, process.PrivateMemorySize64); maxWorking = Math.Max(maxWorking, process.WorkingSet64);
                    maxThreads = Math.Max(maxThreads, process.Threads.Count); maxHandles = Math.Max(maxHandles, process.HandleCount);
                    pingTimes.Add(healthy.Ping());
                }
                Require(clock.Elapsed.TotalSeconds < 90, "Churn test exceeded total time bound.");
            }
            result["malformedHandshakes"] = 1152; result["churnMilliseconds"] = churn.Elapsed.TotalMilliseconds;
            byte[][] invalid = {
                new byte[] { 0x81, 0xfe, 0x40, 0x01 }, // 16385-byte declaration, rejected before payload.
                new byte[] { 0x81, 0 }, // unmasked text
                new byte[] { 0x01, 0x80, 0, 0, 0, 0 }, // unsupported fragmented message
                new byte[] { 0xc1, 0x80, 0, 0, 0, 0 }, // reserved bit
                Wire.Frame("{") // invalid JSON
            };
            int invalidIndex = 0;
            foreach (var frame in invalid) { Stage = "invalid wire " + invalidIndex++; using (var peer = new Wire(port, true)) { peer.Write(frame); peer.Closed(); } pingTimes.Add(healthy.Ping()); }
            result["malformedWireCases"] = invalid.Length;
            using (var slow = new Wire(port, true, true)) {
                Stage = "slow reader authentication";
                slow.Text(Envelope("service:authenticate", new { credential = codeB, enroll = true, protocolVersion = 1 }));
                // Allow admission before producing snapshots; never read results.
                Thread.Sleep(250); var slowClock = Stopwatch.StartNew(); int sent = 0;
                for (int i = 0; i < 80; ++i) {
                    Stage = "slow reader snapshot " + i;
                    try { slow.Text(Envelope("world:snapshot", new { requestId = "pressure", includeMap = true })); sent++; } catch (IOException) { break; }
                    if (i % 8 == 0) pingTimes.Add(healthy.Ping());
                    Thread.Sleep(75); // <=14/s, below per-connection input quota.
                }
                result["slowReaderSnapshotsSent"] = sent;
                result["slowReaderBufferedBytesDrained"] = slow.Closed();
                result["slowReaderMilliseconds"] = slowClock.Elapsed.TotalMilliseconds;
            }
            pingTimes.Add(healthy.Ping());
        }
        // Separate burst exercises per-peer/global pending command pressure.
        Stage = "burst command flood"; var attackers = new List<Wire>();
        try {
            for (int i = 0; i < 20; ++i) attackers.Add(new Wire(port, true));
            byte[] ping = Wire.Frame(Envelope("service:ping", new {}));
            using (var flood = new MemoryStream()) {
                for (int i = 0; i < 96; ++i) flood.Write(ping, 0, ping.Length);
                byte[] payload = flood.ToArray();
                foreach (var attacker in attackers) try { attacker.Write(payload); } catch (IOException) {}
            }
            foreach (var attacker in attackers) attacker.Closed();
            result["floodPeers"] = 20; result["floodMessagesPerPeer"] = 96;
        } finally { foreach (var attacker in attackers) attacker.Dispose(); }
        // Let the fixed-step dispatcher retire the bounded queued intents from
        // closed offenders before measuring recovery, not admission at overload.
        Thread.Sleep(300);
        result["postFloodSettleMilliseconds"] = 300;
        Stage = "authenticated reconnect after pressure";
        using (var recovered = new Peer(port)) { recovered.Authenticate(token, false); pingTimes.Add(recovered.Ping()); }
        for (int i = 0; i < 32; ++i) {
            Stage = "sequential authenticated reconnect " + i;
            using (var reconnect = new Peer(port)) { reconnect.Authenticate(token, false); }
        }
        result["authenticatedReconnects"] = 33;
        Thread.Sleep(500); process.Refresh();
        result["baselinePrivateBytes"] = baselinePrivate; result["finalPrivateBytes"] = process.PrivateMemorySize64;
        result["sampledPeakPrivateBytes"] = Math.Max(maxPrivate, process.PrivateMemorySize64); result["sampledPeakWorkingSetBytes"] = Math.Max(maxWorking, process.WorkingSet64);
        result["sampledPeakThreads"] = maxThreads; result["finalThreads"] = process.Threads.Count; result["sampledPeakHandles"] = maxHandles; result["finalHandles"] = process.HandleCount;
        result["cpuMilliseconds"] = process.TotalProcessorTime.TotalMilliseconds - cpuStart; result["elapsedMilliseconds"] = clock.Elapsed.TotalMilliseconds;
        pingTimes.Sort(); result["pingSamples"] = pingTimes.Count; result["pingMedianMilliseconds"] = pingTimes[pingTimes.Count / 2]; result["pingMaxMilliseconds"] = pingTimes[pingTimes.Count - 1];
        Require(pingTimes[pingTimes.Count - 1] < 2000, "Healthy peer RTT exceeded 2-second pressure-test bound.");
        Require(process.PrivateMemorySize64 - baselinePrivate < 128L * 1024 * 1024, "Private memory grew beyond 128 MiB bounded-test guard.");
        Require(process.Threads.Count < 80, "Completed connection workers were not reclaimed.");
        samples.Write(result);
        result["status"] = "PASS"; return Json.Serialize(result);
        }
    }
}
'@
try {
  $ready = $false; $deadline = [DateTime]::UtcNow.AddSeconds(10)
  while ([DateTime]::UtcNow -lt $deadline -and -not $process.HasExited) {
    if ((Invoke-Control @('--health', '--data', $data)) -eq 0) { $ready = $true; break }
    Start-Sleep -Milliseconds 100
  }
  if (-not $ready) { throw 'Copied service did not become ready.' }
  Add-Type -TypeDefinition $code -ReferencedAssemblies System.dll,System.Core.dll,System.Web.Extensions.dll
  $measurements = [VerdigrisPressure]::Run($port, $process.Id, $a, $b) | ConvertFrom-Json
  $measurements | Add-Member -NotePropertyName sourceExecutable -NotePropertyValue $source
  $measurements | Add-Member -NotePropertyName copiedExecutableSha256 -NotePropertyValue $sourceHash.ToLowerInvariant()
  $measurements | Add-Member -NotePropertyName embeddedIdentity -NotePropertyValue $identity
  $measurements | Add-Member -NotePropertyName topology -NotePropertyValue 'One Windows host, loopback; no injected latency or jitter.'
  $measurements | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $root 'pressure-results.json') -Encoding UTF8
  if ((Invoke-Control @('--health', '--data', $data)) -ne 0) { throw 'Service lost readiness after pressure.' }
  Write-Output "PASS pressure: 1152 malformed handshakes, bounded malformed wire, slow reader, burst flood and authenticated recovery. Evidence: $root"
} catch {
  ('Stage: ' + [VerdigrisPressure]::Stage + [Environment]::NewLine + $_.Exception.ToString()) | Set-Content -LiteralPath (Join-Path $root 'pressure-failure.txt') -Encoding UTF8
  throw
} finally {
  $a = $null; $b = $null
  if (-not $process.HasExited) { $null = Invoke-Control @('--stop', '--data', $data); if (-not $process.WaitForExit(5000)) { $process.Kill(); $process.WaitForExit() } }
  $stdout.Result | Set-Content -LiteralPath (Join-Path $root 'service-stdout.log')
  $stderr.Result | Set-Content -LiteralPath (Join-Path $root 'service-stderr.log')
  $process.Dispose()
}
