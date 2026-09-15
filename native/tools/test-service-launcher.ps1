param([string]$EvidenceDirectory)
$ErrorActionPreference = 'Stop'
if (-not $EvidenceDirectory) { $EvidenceDirectory = Join-Path $PSScriptRoot ('../build/online-launcher-' + [Guid]::NewGuid().ToString('N')) }
$root = [IO.Path]::GetFullPath($EvidenceDirectory)
if (Test-Path -LiteralPath $root) { throw 'Use a fresh launcher test evidence directory.' }
New-Item -ItemType Directory -Path (Join-Path $root 'native/build') -Force | Out-Null
$launcher = Join-Path $root 'Verdigris.exe'
Add-Type -Path (Join-Path $PSScriptRoot 'player-launcher.cs') -OutputAssembly $launcher -OutputType WindowsApplication -ReferencedAssemblies System.Windows.Forms
# This fixture verifies real launcher process/environment behavior only. It is
# explicitly not graphical client or gameplay acceptance.
$fixture = @'
using System;
using System.IO;
public static class LauncherProbe {
  public static int Main(string[] args) {
    string profile = Environment.GetEnvironmentVariable("VERDIGRIS_SERVICE_PROFILE");
    if (args.Length == 4 && args[0] == "--remote") {
      if (profile != null || args[1] != "127.0.0.1" || args[3] != "review-player") return 30;
      string saves = Environment.GetEnvironmentVariable("VERDIGRIS_SAVE_DIR");
      if (String.IsNullOrEmpty(saves)) return 31;
      File.WriteAllText(Path.Combine(saves, "local-launcher-probe.txt"), "local review arguments and save isolation verified");
      return 0;
    }
    if (String.IsNullOrEmpty(profile)) return 20;
    if (args.Length != 2 || args[0] != "--online" || args[1] != "ws://127.0.0.1:65419/game") return 21;
    if (Environment.GetEnvironmentVariable("VERDIGRIS_SAVE_DIR") != null) return 22;
    if (Environment.GetEnvironmentVariable("VERDIGRIS_SETTINGS_PATH") != Path.Combine(profile, "settings.ini")) return 23;
    File.WriteAllText(Path.Combine(profile, "launcher-probe.txt"), "online arguments, service profile and isolated settings verified");
    return 0;
  }
}
'@
Add-Type -TypeDefinition $fixture -OutputAssembly (Join-Path $root 'native/build/verdigris_client.exe') -OutputType ConsoleApplication
if (Test-Path -LiteralPath (Join-Path $root 'native/build/verdigris_server.exe')) { throw 'Fixture must have no server executable.' }
$profile = Join-Path $root 'online-profile'
$process = Start-Process -FilePath $launcher -ArgumentList @('--online', 'ws://127.0.0.1:65419/game', '--profile', ('"' + $profile + '"')) -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit(15000)) { throw 'Launcher fixture did not finish within 15 seconds.' }
if ($process.ExitCode -ne 0 -or -not (Test-Path -LiteralPath (Join-Path $profile 'launcher-probe.txt'))) { throw 'Online launcher did not pass its real child process probe.' }
if (Test-Path -LiteralPath (Join-Path $profile 'saves')) { throw 'Online launcher created local-review saves.' }
$localServer = @'
using System;
using System.IO;
public static class LocalServerProbe {
  public static int Main(string[] args) {
    Console.WriteLine("verdigris_server listening on ws://127.0.0.1:" + args[0]);
    Console.Out.Flush();
    if (Console.ReadLine() != "quit") return 40;
    File.WriteAllText(Path.Combine(Environment.GetEnvironmentVariable("VERDIGRIS_SAVE_DIR"), "server-stopped.txt"), "graceful quit");
    return 0;
  }
}
'@
Add-Type -TypeDefinition $localServer -OutputAssembly (Join-Path $root 'native/build/verdigris_server.exe') -OutputType ConsoleApplication
$localProfile = Join-Path $root 'local-profile'
$local = Start-Process -FilePath $launcher -ArgumentList @('--profile', ('"' + $localProfile + '"')) -WindowStyle Hidden -PassThru
if (-not $local.WaitForExit(15000) -or $local.ExitCode -ne 0 -or
    -not (Test-Path -LiteralPath (Join-Path $localProfile 'saves/local-launcher-probe.txt')) -or
    -not (Test-Path -LiteralPath (Join-Path $localProfile 'saves/server-stopped.txt'))) { throw 'Local review child/start/readiness/graceful-stop contract regressed.' }
$assembly = [Reflection.Assembly]::LoadFile($launcher)
$type = $assembly.GetType('PlayerLauncher')
$flags = [Reflection.BindingFlags]'NonPublic,Static'
$validate = $type.GetMethod('ValidateOnlineEndpoint', $flags)
foreach ($endpoint in @('ws://example.com', 'ws://127.1', 'ws://127.0.0.1.evil.test', 'wss://u:p@example.com', 'wss://example.com/?secret=x', 'wss://example.com/#x', 'https://example.com', 'wss://example.com:65536')) {
  $rejected = $false
  try { $null = $validate.Invoke($null, [object[]]@($endpoint)) } catch { $rejected = $true }
  if (-not $rejected) { throw "Launcher accepted invalid endpoint: $endpoint" }
}
foreach ($endpoint in @('wss://game.example.com/socket', 'ws://localhost:65419/game', 'ws://[::1]:65419/game')) { $null = $validate.Invoke($null, [object[]]@($endpoint)) }
$defaults = $type.GetMethod('DefaultOnlineProfile', $flags)
$a = $defaults.Invoke($null, [object[]]@('wss://one.example.com/game'))
$b = $defaults.Invoke($null, [object[]]@('wss://two.example.com/game'))
if ($a -eq $b -or -not $a.StartsWith((Join-Path $env:LOCALAPPDATA 'Verdigris/online'), [StringComparison]::OrdinalIgnoreCase)) { throw 'Default profiles are not isolated by service endpoint.' }
$lockMethod = $type.GetMethod('AcquireProfileLock', $flags)
$first = $lockMethod.Invoke($null, [object[]]@([string]$profile))
try {
  $denied = $false
  try { $second = $lockMethod.Invoke($null, [object[]]@([string]($profile + '\'))); $second.Dispose() } catch { $denied = $true }
  if (-not $denied) { throw 'Profile lock admitted an alias.' }
} finally { $first.Dispose() }
Write-Output "PASS real launcher processes: online without server executable, isolated child environment, endpoint validation, per-service profile, profile lock; local review startup/readiness/client/graceful-stop preserved. Evidence: $root"
