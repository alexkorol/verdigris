param(
  [switch]$Local,
  [switch]$Rebuild,
  [switch]$LifecycleSelfTest,
  [switch]$ReadinessFaultControl,
  [int]$Port = 0,
  [string]$BuildSubdirectory = ""
)

$ErrorActionPreference = "Stop"

$nativeRoot = Split-Path $PSScriptRoot -Parent
$buildRoot = Join-Path $nativeRoot "build"
if ($BuildSubdirectory -ne "") {
  if ($BuildSubdirectory -notmatch '^[a-zA-Z0-9_-]+$') {
    throw "BuildSubdirectory must be a single directory name under native/build."
  }
  $buildRoot = Join-Path $buildRoot $BuildSubdirectory
}
$logDir = Join-Path $buildRoot "logs"
$serverExe = Join-Path $buildRoot "verdigris_server.exe"
$clientExe = Join-Path $buildRoot "verdigris_client.exe"
$buildScript = Join-Path $nativeRoot "build.ps1"
$capsuleStart = 6520
$capsuleEnd = 6539

function Fail([string]$message) {
  throw "play-native: $message"
}

function Test-PidAlive([int]$processId) {
  if ($processId -le 0) { return $false }
  return $null -ne (Get-Process -Id $processId -ErrorAction SilentlyContinue)
}

function Get-ListenerPids([int]$portNumber) {
  $holderPids = @()
  try {
    $holderPids = @(
      Get-NetTCPConnection -LocalPort $portNumber -State Listen -ErrorAction SilentlyContinue |
        Select-Object -ExpandProperty OwningProcess -Unique
    )
  } catch { }
  return $holderPids
}

function Assert-PortInCapsule([int]$candidate) {
  if ($candidate -eq 6500) {
    Fail "port 6500 is reserved for the historical browser server. The native owner-play capsule is $capsuleStart-$capsuleEnd."
  }
  if ($candidate -lt $capsuleStart -or $candidate -gt $capsuleEnd) {
    Fail "port $candidate is outside the owner-play capsule $capsuleStart-$capsuleEnd."
  }
}

function Test-PortFree([int]$candidate) {
  if ((Get-ListenerPids $candidate).Count -gt 0) { return $false }
  try {
    $listener = [System.Net.Sockets.TcpListener]::new(
      [System.Net.IPAddress]::Loopback, $candidate)
    $listener.Start()
    $listener.Stop()
    return $true
  } catch {
    return $false
  }
}

function Assert-PortUsable([int]$candidate) {
  Assert-PortInCapsule $candidate
  $holders = @(Get-ListenerPids $candidate)
  if ($holders.Count -gt 0) {
    $owners = @($holders | ForEach-Object {
      $owner = Get-Process -Id $_ -ErrorAction SilentlyContinue
      if ($owner) { "$($owner.ProcessName):$_" } else { "pid:$_" }
    })
    Fail "port $candidate is already listening ($($owners -join ', ')); pick another free port in $capsuleStart-$capsuleEnd."
  }
}

function Find-OwnerPlayPort {
  foreach ($candidate in $capsuleStart..$capsuleEnd) {
    if (Test-PortFree $candidate) { return $candidate }
  }
  Fail "no free owner-play port in $capsuleStart-$capsuleEnd; free one or pass -Port <free port in the capsule>"
}

function Test-ExeStale([string]$exe) {
  if (-not (Test-Path $exe)) { return $true }
  $exeTime = (Get-Item $exe).LastWriteTimeUtc
  $roots = @(
    (Join-Path $nativeRoot "src"),
    (Join-Path $nativeRoot "client"),
    (Join-Path $nativeRoot "include"),
    (Join-Path $nativeRoot "build.ps1")
  )
  $newest = Get-ChildItem -Path $roots -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in ".cpp", ".hpp", ".h", ".ps1" } |
    Sort-Object LastWriteTimeUtc -Descending |
    Select-Object -First 1
  if (-not $newest) { return $false }
  return $newest.LastWriteTimeUtc -gt $exeTime
}

function Invoke-NativeBuild {
  Write-Host "play-native: building via native/build.ps1"
  if ($BuildSubdirectory -eq "") {
    & powershell.exe -NoProfile -File $buildScript
  } else {
    & powershell.exe -NoProfile -File $buildScript -BuildSubdirectory $BuildSubdirectory
  }
  if ($LASTEXITCODE -ne 0) { Fail "native/build.ps1 failed with exit $LASTEXITCODE" }
}

if ($Port -ne 0) { Assert-PortInCapsule $Port }
if ($LifecycleSelfTest) {
  if ($Local) { Fail "-LifecycleSelfTest exercises the real remote client/server path; drop -Local." }
  if ($Port -ne 0) { Fail "-LifecycleSelfTest picks its own free capsule ports deterministically; drop -Port." }
  if ($ReadinessFaultControl) { Fail "-LifecycleSelfTest and -ReadinessFaultControl are separate controls; run one at a time." }
}
if ($ReadinessFaultControl) {
  if ($Local) { Fail "-ReadinessFaultControl exercises the real remote spawn path; drop -Local." }
  if ($Port -ne 0) { Fail "-ReadinessFaultControl picks its own free capsule ports deterministically; drop -Port." }
}
if (-not (Test-Path $buildScript)) { Fail "missing $buildScript; run this script from a full native/ checkout" }

if ($Rebuild -or (Test-ExeStale $clientExe) -or (-not $Local -and (Test-ExeStale $serverExe))) {
  Invoke-NativeBuild
}

if (-not (Test-Path $clientExe)) { Fail "missing $clientExe; run play-native.ps1 -Rebuild to produce it" }
if (-not $Local -and -not (Test-Path $serverExe)) { Fail "missing $serverExe; run play-native.ps1 -Rebuild to produce it" }

New-Item -ItemType Directory -Force -Path $logDir | Out-Null

function Start-OwnerServer([int]$chosenPort, [string]$outLog, [string]$errLog,
  [string]$exePath = "", [string]$exeArguments = "") {
  if ($exePath -eq "") {
    $exePath = $serverExe
    $exeArguments = "$chosenPort"
    if ($LifecycleSelfTest -or $ReadinessFaultControl) {
      $exeArguments += " --ephemeral"
    }
  }
  Write-Host "play-native: starting verdigris_server on ws://127.0.0.1:$chosenPort (capsule $capsuleStart-$capsuleEnd)"
  $proc = Start-Process -FilePath $exePath -ArgumentList $exeArguments -PassThru `
    -WindowStyle Hidden `
    -RedirectStandardOutput $outLog `
    -RedirectStandardError $errLog
  $script:lastSpawnedServerPid = $proc.Id
  try {
    $deadline = (Get-Date).AddSeconds(12)
    $readyPort = 0
    while ((Get-Date) -lt $deadline) {
      if ($proc.HasExited) { break }
      if (Test-Path $outLog) {
        $text = Get-Content -Path $outLog -Raw -ErrorAction SilentlyContinue
        if ($text -match "listening on ws://127\.0\.0\.1:(\d+)") {
          $readyPort = [int]$Matches[1]
          break
        }
      }
      Start-Sleep -Milliseconds 150
    }
    if ($proc.HasExited) {
      $tail = ""
      if (Test-Path $errLog) {
        $tail = (Get-Content -Path $errLog -Tail 5 -ErrorAction SilentlyContinue) -join " | "
      }
      Fail "verdigris_server (pid $($proc.Id)) exited during startup with code $($proc.ExitCode); stderr: $tail"
    }
    if ($readyPort -eq 0) {
      Fail "verdigris_server (pid $($proc.Id)) printed no listening line within 12s; see $outLog"
    }
    if ($readyPort -ne $chosenPort) {
      Fail "verdigris_server reported port $readyPort but the launcher chose $chosenPort"
    }
  } catch {
    if (-not $proc.HasExited) {
      Stop-OwnerServer $proc 5
      Write-Host "play-native: startup readiness failed; stopped spawned server pid $($proc.Id) to prevent an orphan"
    } else {
      Write-Host "play-native: startup readiness failed; spawned server pid $($proc.Id) already exited"
    }
    throw
  }
  Write-Host "play-native: server ready (pid $($proc.Id), stdout log $outLog, stderr log $errLog)"
  return $proc
}

function Stop-OwnerServer([object]$proc, [int]$timeoutSeconds = 5) {
  if ($null -eq $proc) { return }
  try { Stop-Process -Id $proc.Id -Force -ErrorAction Stop } catch { }
  $deadline = (Get-Date).AddSeconds($timeoutSeconds)
  while ((Get-Date) -lt $deadline) {
    if (-not (Test-PidAlive $proc.Id)) { return }
    Start-Sleep -Milliseconds 100
  }
}

function Wait-ClientExit([object]$proc, [int]$timeoutSeconds) {
  $deadline = (Get-Date).AddSeconds($timeoutSeconds)
  while ((Get-Date) -lt $deadline) {
    if (-not (Test-PidAlive $proc.Id)) { return $true }
    Start-Sleep -Milliseconds 100
  }
  return -not (Test-PidAlive $proc.Id)
}

$script:gameWindowClass = "VerdigrisNativeClient"
$script:enumWindowsFound = @()
$script:lastSpawnedServerPid = 0

function Wait-ClientGameWindow([object]$proc, [int]$timeoutSeconds) {
  if (-not ("VerdigrisLaunch.NativeWindow" -as [type])) {
    Add-Type -Namespace VerdigrisLaunch -Name NativeWindow -MemberDefinition @'
[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint message, IntPtr wParam, IntPtr lParam);
[DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr lParam);
public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
[DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint pid);
[DllImport("user32.dll")] public static extern int GetClassName(IntPtr hWnd, System.Text.StringBuilder text, int count);
[DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
'@ | Out-Null
  }
  $deadline = (Get-Date).AddSeconds($timeoutSeconds)
  while ((Get-Date) -lt $deadline) {
    $proc.Refresh()
    if ($proc.HasExited) { return [IntPtr]::Zero }
    $script:enumWindowsFound = @()
    $callback = [VerdigrisLaunch.NativeWindow+EnumWindowsProc]{
      param([IntPtr]$h, [IntPtr]$l)
      $windowPid = [uint32]0
      $null = [VerdigrisLaunch.NativeWindow]::GetWindowThreadProcessId($h, [ref]$windowPid)
      if ([int]$windowPid -eq $proc.Id) {
        $classText = New-Object System.Text.StringBuilder 256
        $null = [VerdigrisLaunch.NativeWindow]::GetClassName($h, $classText, 256)
        $script:enumWindowsFound += @{
          Hwnd = $h
          Class = $classText.ToString()
          Visible = [VerdigrisLaunch.NativeWindow]::IsWindowVisible($h)
        }
      }
      return $true
    }
    $null = [VerdigrisLaunch.NativeWindow]::EnumWindows($callback, [IntPtr]::Zero)
    $match = $script:enumWindowsFound |
      Where-Object { $_.Class -eq $script:gameWindowClass -and $_.Visible } |
      Select-Object -First 1
    if ($match) { return [IntPtr]$match.Hwnd }
    Start-Sleep -Milliseconds 150
  }
  return [IntPtr]::Zero
}

function Close-WindowGracefully([IntPtr]$windowHandle) {
  if (-not ("VerdigrisLaunch.NativeWindow" -as [type])) {
    throw "play-native: window interop type is not loaded"
  }
  return [VerdigrisLaunch.NativeWindow]::PostMessage(
    $windowHandle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)
}

function Assert-SessionClean([string]$label, [object[]]$tracked) {
  $leaks = @()
  foreach ($entry in $tracked) {
    if ($null -ne $entry -and (Test-PidAlive $entry.Id)) {
      $leaks += "$($entry.Name):$($entry.Id)"
    }
  }
  if ($leaks.Count -gt 0) {
    Fail "$label left orphan processes: $($leaks -join ', ')"
  }
  Write-Host "play-native: $label left no orphan verdigris processes (verified by pid)"
}

function Invoke-LifecycleScenario([string]$name, [switch]$ForceKill) {
  $stamp = Get-Date -Format "yyyyMMdd-HHmmss-fff"
  $outLog = Join-Path $logDir ("selftest-$name-$stamp.log")
  $errLog = Join-Path $logDir ("selftest-$name-$stamp.err.log")
  $chosenPort = Find-OwnerPlayPort
  Write-Host "play-native: selftest $name starting on port $chosenPort (log $outLog)"
  $server = Start-OwnerServer $chosenPort $outLog $errLog
  $client = $null
  try {
    $client = Start-Process -FilePath $clientExe `
      -ArgumentList @("--remote", "127.0.0.1", "$chosenPort") -PassThru
    Write-Host "play-native: selftest $name client pid $($client.Id) (--remote 127.0.0.1 $chosenPort)"
    $windowHandle = Wait-ClientGameWindow $client 20
    if ($windowHandle -eq [IntPtr]::Zero) {
      if (Test-PidAlive $client.Id) {
        Fail "${name}: client (pid $($client.Id)) opened no $script:gameWindowClass window within 20s"
      }
      Fail "${name}: client (pid $($client.Id)) exited before opening a window; server log $outLog"
    }
    if ($ForceKill) {
      Stop-Process -Id $client.Id -Force -ErrorAction Stop
      if (-not (Wait-ClientExit $client 5)) {
        Fail "${name}: forced kill left client pid $($client.Id) alive"
      }
      Write-Host "play-native: selftest $name forced client exit done (pid $($client.Id) killed)"
    } else {
      $posted = Close-WindowGracefully $windowHandle
      if (-not $posted) {
        Fail "${name}: WM_CLOSE could not be posted to client window $windowHandle"
      }
      if (-not (Wait-ClientExit $client 10)) {
        Fail "${name}: client (pid $($client.Id)) ignored a normal close within 10s"
      }
      $client.Refresh()
      if ($client.ExitCode -ne 0) {
        Fail "${name}: normal close of window ${windowHandle} ended with client exit code $($client.ExitCode); expected 0"
      }
      Write-Host "play-native: selftest $name normal close accepted (client pid $($client.Id) exited by itself with code 0)"
    }
  } finally {
    Stop-OwnerServer $server 5
  }
  Assert-SessionClean "$name" @(
    @{ Name = "verdigris_server"; Id = $server.Id },
    @{ Name = "verdigris_client"; Id = $client.Id }
  )
  Write-Host "play-native: selftest $name PASS (port $chosenPort, server log $outLog)"
}

function Invoke-ReadinessFaultScenario([string]$name, [switch]$FakeListeningLine) {
  if ($FakeListeningLine) {
    $fakeInner = "[Console]::Out.WriteLine('verdigris_server listening on ws://127.0.0.1:6599'); Start-Sleep -Seconds 120"
    $expectedReason = "port-mismatch assertion against a live impostor process"
  } else {
    $fakeInner = "Start-Sleep -Seconds 120"
    $expectedReason = "12s readiness deadline against a live silent process"
  }
  $stamp = Get-Date -Format "yyyyMMdd-HHmmss-fff"
  $outLog = Join-Path $logDir ("faultctl-$name-$stamp.log")
  $errLog = Join-Path $logDir ("faultctl-$name-$stamp.err.log")
  $chosenPort = Find-OwnerPlayPort
  Write-Host "play-native: fault-control $name starting on port $chosenPort ($expectedReason)"
  $script:lastSpawnedServerPid = 0
  $threw = $false
  try {
    Start-OwnerServer $chosenPort $outLog $errLog "powershell.exe" "-NoProfile -Command `"$fakeInner`"" | Out-Null
  } catch {
    $threw = $true
    Write-Host "play-native: fault-control $name observed the expected failure - $($_.Exception.Message)"
  }
  if (-not $threw) {
    Fail "fault-control ${name}: a live post-spawn readiness failure did NOT fail the launcher; the control is broken"
  }
  $leakedPid = $script:lastSpawnedServerPid
  if ($leakedPid -le 0) {
    Fail "fault-control ${name}: launcher published no spawned server pid before the readiness checks"
  }
  if (Test-PidAlive $leakedPid) {
    Fail "fault-control ${name}: live post-spawn failure leaked server pid $leakedPid"
  }
  Write-Host "play-native: fault-control $name PASS (published pid $leakedPid is gone; no orphan)"
}

if ($ReadinessFaultControl) {
  Invoke-ReadinessFaultScenario "readiness-timeout"
  Invoke-ReadinessFaultScenario "port-mismatch" -FakeListeningLine
  Write-Host "play-native: readiness fault control PASS (live post-spawn failures left no orphan server)"
  exit 0
}

if ($LifecycleSelfTest) {
  Invoke-LifecycleScenario "normal-close"
  Invoke-LifecycleScenario "forced-exit" -ForceKill
  Write-Host "play-native: lifecycle selftest PASS (normal close and forced client exit both cleaned up)"
  exit 0
}

$server = $null
$client = $null
$chosenPort = 0
$serverLog = ""
$sessionLabel = "local session"

try {
  if (-not $Local) {
    $sessionLabel = "owner session"
    if ($Port -gt 0) {
      $chosenPort = $Port
      Write-Host "play-native: using requested port $chosenPort"
    } else {
      $chosenPort = Find-OwnerPlayPort
      Write-Host "play-native: auto-selected free port $chosenPort (first free in $capsuleStart-$capsuleEnd)"
    }
    Assert-PortUsable $chosenPort
    $staleListeners = @()
    foreach ($candidate in $capsuleStart..$capsuleEnd) {
      if ($candidate -eq $chosenPort) { continue }
      foreach ($holderPid in (Get-ListenerPids $candidate)) {
        $holder = Get-Process -Id $holderPid -ErrorAction SilentlyContinue
        if ($holder) { $staleListeners += "$candidate ($($holder.ProcessName):$holderPid)" }
      }
    }
    if ($staleListeners.Count -gt 0) {
      Write-Host ("play-native: note, other listeners inside the capsule (not this launch): " +
        ($staleListeners -join ", "))
    }
    $stamp = Get-Date -Format "yyyyMMdd-HHmmss-fff"
    $serverLog = Join-Path $logDir "server-$chosenPort-$stamp.log"
    $serverErr = Join-Path $logDir "server-$chosenPort-$stamp.err.log"
    $server = Start-OwnerServer $chosenPort $serverLog $serverErr
    Write-Host "play-native: starting real windowed client: verdigris_client --remote 127.0.0.1 $chosenPort"
  } else {
    Write-Host "play-native: local mode (no server); starting client with in-process simulation"
  }
  Write-Host "play-native: close the window or press Esc to quit"
  if ($Local) {
    $client = Start-Process -FilePath $clientExe -PassThru
  } else {
    $client = Start-Process -FilePath $clientExe `
      -ArgumentList @("--remote", "127.0.0.1", "$chosenPort") -PassThru
  }
  Write-Host "play-native: client pid $($client.Id)"
  $client.WaitForExit()
  Write-Host "play-native: client exit code $($client.ExitCode)"
} catch {
  Write-Host "play-native: FAILED - $($_.Exception.Message)"
  exit 1
} finally {
  if ($server -and (Test-PidAlive $server.Id)) { Stop-OwnerServer $server 5 }
}

if ($Local) {
  Assert-SessionClean $sessionLabel @(@{ Name = "verdigris_client"; Id = $client.Id })
} else {
  Assert-SessionClean $sessionLabel @(
    @{ Name = "verdigris_server"; Id = $server.Id },
    @{ Name = "verdigris_client"; Id = $client.Id }
  )
  Write-Host "play-native: chosen port $chosenPort; server log $serverLog"
}
exit 0
