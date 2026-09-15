# Run with Windows PowerShell (powershell.exe), matching package tooling.
param(
  [Parameter(Mandatory=$true)][string]$PackageDirectory,
  [Parameter(Mandatory=$true)][string]$Endpoint,
  [Parameter(Mandatory=$true)][string]$EvidenceDirectory,
  [ValidateRange(10,120)][int]$TimeoutSeconds=45
)
$ErrorActionPreference='Stop'
$package=[IO.Path]::GetFullPath($PackageDirectory)
$evidence=[IO.Path]::GetFullPath($EvidenceDirectory)
$entry=Join-Path $package 'Verdigris.exe'
$client=Join-Path $package 'native/build/verdigris_client.exe'
foreach($file in @($entry,$client)){if(-not (Test-Path -LiteralPath $file -PathType Leaf)){throw "Package executable missing: $file"}}
if(Test-Path -LiteralPath $evidence){throw 'Use a fresh evidence directory, including a fresh isolated online profile.'}
$uri=$null
if(-not [Uri]::TryCreate($Endpoint,[UriKind]::Absolute,[ref]$uri) -or $uri.Scheme -notin @('ws','wss') -or $uri.UserInfo -or $uri.Query -or $uri.Fragment -or $Endpoint -match '[\s"]'){throw 'Expected a websocket endpoint without credentials, query, fragment or whitespace.'}
New-Item -ItemType Directory -Path $evidence | Out-Null
$profile=Join-Path $evidence 'online-profile'
$entryHash=(Get-FileHash -LiteralPath $entry -Algorithm SHA256).Hash
$clientHash=(Get-FileHash -LiteralPath $client -Algorithm SHA256).Hash
$identity=(& $client --build-info | Out-String).Trim()
if($LASTEXITCODE -ne 0){throw 'Could not identify packaged native client.'}

# Adapt the existing PrintWindow helper, but require the exact child PID.
# Never choose an arbitrary owner window by class name, or send desktop input.
Add-Type -AssemblyName System.Drawing
if(-not ('VGOnlinePackageWindow' -as [type])) {
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
public static class VGOnlinePackageWindow {
  public delegate bool EnumProc(IntPtr h, IntPtr p);
  [StructLayout(LayoutKind.Sequential)] public struct Rect { public int L,T,R,B; }
  [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc callback, IntPtr p);
  [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr h, StringBuilder text, int count);
  [DllImport("user32.dll")] static extern bool GetWindowRect(IntPtr h, out Rect rect);
  [DllImport("user32.dll")] static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
  [DllImport("user32.dll")] static extern bool PostMessage(IntPtr h, uint message, IntPtr w, IntPtr l);
  public static bool Owns(IntPtr h, int pid) { uint actual; GetWindowThreadProcessId(h,out actual); return actual==(uint)pid; }
  public static IntPtr Find(int pid) {
    IntPtr found=IntPtr.Zero;
    EnumWindows(delegate(IntPtr h,IntPtr p) {
      if(!Owns(h,pid)) return true;
      var name=new StringBuilder(256); GetClassName(h,name,name.Capacity);
      if(name.ToString()!="VerdigrisNativeClient") return true;
      found=h; return false;
    },IntPtr.Zero); return found;
  }
  public static string Capture(IntPtr h,int pid,string path) {
    if(!Owns(h,pid)) throw new InvalidOperationException("Capture window no longer belongs to QA client");
    Rect r; if(!GetWindowRect(h,out r)||r.R<=r.L||r.B<=r.T) throw new InvalidOperationException("Invalid client window dimensions");
    using(var bitmap=new Bitmap(r.R-r.L,r.B-r.T)) {
      using(var graphics=Graphics.FromImage(bitmap)) {
        IntPtr dc=graphics.GetHdc(); bool ok;
        try { ok=PrintWindow(h,dc,2); } finally { graphics.ReleaseHdc(dc); }
        if(!ok) throw new InvalidOperationException("PrintWindow failed");
      }
      bitmap.Save(path,ImageFormat.Png);
    }
    return (r.R-r.L)+"x"+(r.B-r.T);
  }
  public static bool Close(IntPtr h,int pid) { return Owns(h,pid)&&PostMessage(h,0x0010,IntPtr.Zero,IntPtr.Zero); }
}
'@
}
$launcher=$null;$ownedClient=$null;$hwnd=[IntPtr]::Zero;$observed=@{};$failure=$null;$summary=$null
try {
  $launcher=Start-Process -FilePath $entry -ArgumentList @('--online',('"'+$Endpoint+'"'),'--profile',('"'+$profile+'"')) -WorkingDirectory $package -WindowStyle Hidden -PassThru
  $retainedHandle=$launcher.Handle
  $deadline=[DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
  do {
    if($launcher.HasExited){throw 'Packaged launcher exited before presenting the online account screen.'}
    $children=@(Get-CimInstance Win32_Process -Filter "ParentProcessId=$($launcher.Id)")
    foreach($child in $children) {
      $observed[[string]$child.ProcessId]=@{pid=[int]$child.ProcessId;parent=[int]$child.ParentProcessId;image=$child.ExecutablePath;command=$child.CommandLine}
      if($child.Name -eq 'verdigris_server.exe'){throw 'Online launcher unexpectedly started a local server.'}
      if($child.Name -eq 'verdigris_client.exe') {
        if(-not [string]::Equals($child.ExecutablePath,$client,[StringComparison]::OrdinalIgnoreCase)){throw 'Launcher selected a native client outside this package.'}
        if($child.CommandLine -notmatch '--online\s+' -or -not $child.CommandLine.Contains($Endpoint) -or $child.CommandLine -match '--remote|--scenario'){throw 'Native child did not receive the ordinary online endpoint arguments.'}
        if(-not $ownedClient){$ownedClient=Get-Process -Id $child.ProcessId;$ownedHandle=$ownedClient.Handle}
        $hwnd=[VGOnlinePackageWindow]::Find($ownedClient.Id)
      }
    }
    if($hwnd -ne [IntPtr]::Zero){break}
    Start-Sleep -Milliseconds 150
  } while([DateTime]::UtcNow -lt $deadline)
  if($hwnd -eq [IntPtr]::Zero){throw 'Packaged online native client did not create its account window in time.'}
  # Give ordinary connection/title rendering time to settle. Capture twice so
  # reviewers can distinguish startup from the stable fresh-account frontend.
  Start-Sleep -Seconds 2
  $dimensions=[VGOnlinePackageWindow]::Capture($hwnd,$ownedClient.Id,(Join-Path $evidence 'online-account-start.png'))
  Start-Sleep -Seconds 2
  [void][VGOnlinePackageWindow]::Capture($hwnd,$ownedClient.Id,(Join-Path $evidence 'online-account-ready.png'))
  if(-not [VGOnlinePackageWindow]::Close($hwnd,$ownedClient.Id)){throw 'Could not send WM_CLOSE to the owned native client.'}
  if(-not $launcher.WaitForExit(15000)){throw 'Launcher did not exit after its owned client closed.'}
  $launcher.WaitForExit()
  if($launcher.ExitCode -ne 0){throw "Packaged launcher returned $($launcher.ExitCode)."}
  if(-not $ownedClient.WaitForExit(1000)){throw 'Owned native client survived launcher exit.'}
  $log=Get-ChildItem -LiteralPath (Join-Path $profile 'logs') -File -Filter 'session-*.log' | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
  if(-not $log){throw 'Packaged launcher produced no isolated session log.'}
  $body=Get-Content -LiteralPath $log.FullName -Raw
  Copy-Item -LiteralPath $log.FullName -Destination (Join-Path $evidence 'launcher.log')
  foreach($expected in @('isolatedSettings=True',('profile='+$profile),('image='+$client),('cwd='+$package),'client exit=0',('cleaned pid='+$ownedClient.Id+' exit=0'))) {
    if(-not $body.Contains($expected)){throw "Missing actual launcher evidence: $expected"}
  }
  if($body -match 'verdigris_server\.exe pid='){throw 'Session log records an unexpected local server.'}
  if(Test-Path -LiteralPath (Join-Path $profile 'saves')){throw 'Online launch unexpectedly created local-review saves.'}
  if((Get-FileHash -LiteralPath $entry).Hash -ne $entryHash -or (Get-FileHash -LiteralPath $client).Hash -ne $clientHash){throw 'Package executables changed during the launch check.'}
  $summary=@{status='passed';launcher=$entry;launcher_sha256=$entryHash;client=$client;client_sha256=$clientHash;build=$identity;launcher_pid=$launcher.Id;client_pid=$ownedClient.Id;hwnd=$hwnd.ToInt64();endpoint=$Endpoint;profile=$profile;capture_dimensions=$dimensions;observed_children=@($observed.Values);local_server_started=$false;close_path='PID-scoped native HWND WM_CLOSE';visual_acceptance='Captured; requires image inspection';external_service_health='Managed and verified independently by caller'}
} catch {
  $failure=$_
  $summary=@{status='failed';error=$_.Exception.Message;launcher=$entry;client=$client;profile=$profile;observed_children=@($observed.Values)}
} finally {
  if($ownedClient -and -not $ownedClient.HasExited -and $hwnd -ne [IntPtr]::Zero){[void][VGOnlinePackageWindow]::Close($hwnd,$ownedClient.Id)}
  if($launcher -and -not $launcher.HasExited) {
    if(-not $launcher.WaitForExit(5000)) {
      # Only the Process object created above. Its job cleans up only its own
      # children; never stop the external service or enumerate/kill owner PIDs.
      $launcher.Kill();$launcher.WaitForExit()
    }
  }
  if($ownedClient){$ownedClient.Dispose()}
  if($launcher){$launcher.Dispose()}
  if($summary){$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $evidence 'summary.json') -Encoding UTF8}
}
if($failure){throw $failure}
Write-Output "PASS exact packaged online launcher, actual client image/arguments, account captures, no local server, owned-window close and launcher cleanup. Evidence: $evidence"
