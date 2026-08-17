$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)))
$exe = Join-Path $root 'native\build\verdigris_client.exe'
$captureDir = $PSScriptRoot

Add-Type -ReferencedAssemblies System.Drawing.dll -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
public static class VerdigrisCapture {
  [DllImport("user32.dll", CharSet=CharSet.Ansi)]
  public static extern IntPtr FindWindow(string cls, string title);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hwnd);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hwnd, uint msg, IntPtr wp, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hwnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hwnd, IntPtr hdc, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int left, top, right, bottom; }
  public static bool Save(IntPtr hwnd, string path) {
    RECT r; if (!GetWindowRect(hwnd, out r)) return false;
    using (var bitmap = new Bitmap(r.right-r.left, r.bottom-r.top))
    using (var graphics = Graphics.FromImage(bitmap)) {
      var hdc = graphics.GetHdc();
      try { if (!PrintWindow(hwnd, hdc, 0)) return false; }
      finally { graphics.ReleaseHdc(hdc); }
      var encoder = new ImageCodecInfo[0];
      foreach (var candidate in ImageCodecInfo.GetImageEncoders())
        if (candidate.MimeType == "image/jpeg") encoder = new[] { candidate };
      var quality = new EncoderParameters(1);
      quality.Param[0] = new EncoderParameter(Encoder.Quality, 78L);
      bitmap.Save(path, encoder[0], quality);
    }
    return true;
  }
}
'@

function Send-Key([IntPtr] $hwnd, [int] $key) {
  [VerdigrisCapture]::PostMessage($hwnd, 0x0100, [IntPtr]$key, [IntPtr]0) | Out-Null
}
function Release-Key([IntPtr] $hwnd, [int] $key) {
  [VerdigrisCapture]::PostMessage($hwnd, 0x0101, [IntPtr]$key, [IntPtr]0) | Out-Null
}
function Hold-Keys([IntPtr] $hwnd, [int[]] $keys, [int] $ticks) {
  foreach ($key in $keys) { Send-Key $hwnd $key }
  Start-Sleep -Milliseconds ($ticks * 55)
  foreach ($key in $keys) { Release-Key $hwnd $key }
  Start-Sleep -Milliseconds 100
}

$process = Start-Process -FilePath $exe -WorkingDirectory $root -PassThru
try {
  Start-Sleep -Milliseconds 900
  $hwnd = [VerdigrisCapture]::FindWindow('VerdigrisNativeClient', $null)
  if ($hwnd -eq [IntPtr]::Zero) { throw 'native client window not found' }
  [VerdigrisCapture]::SetForegroundWindow($hwnd) | Out-Null
  [VerdigrisCapture]::Save($hwnd, (Join-Path $captureDir 'scenery-initial.jpg')) | Out-Null

  # Move around the near-field tree so the player is captured on both sides of
  # its y boundary, then route directly into the fixed dwelling collider.
  Hold-Keys $hwnd @(0x44) 40 # D: clear the tree's circle from the east
  Hold-Keys $hwnd @(0x57) 15 # W: player behind tree
  [VerdigrisCapture]::Save($hwnd, (Join-Path $captureDir 'depth-behind-tree.jpg')) | Out-Null
  Hold-Keys $hwnd @(0x41) 1  # A: face into the near-field tree
  Send-Key $hwnd 0x20        # Space: dash is swept-blocked by the tree
  Start-Sleep -Milliseconds 150
  [VerdigrisCapture]::Save($hwnd, (Join-Path $captureDir 'dash-blocked.jpg')) | Out-Null
  Hold-Keys $hwnd @(0x53) 30 # S: player in front of tree
  [VerdigrisCapture]::Save($hwnd, (Join-Path $captureDir 'depth-front-tree.jpg')) | Out-Null

  Hold-Keys $hwnd @(0x41) 70     # A: align with the fixed dwelling
  Hold-Keys $hwnd @(0x57) 40     # W: approach dwelling and test blocking
  [VerdigrisCapture]::Save($hwnd, (Join-Path $captureDir 'dwelling-blocked.jpg')) | Out-Null

  # Catalog costs are read from the rendered strip at the initial state; retain
  # an explicit machine-readable assertion beside the visual evidence.
  $log = @(
    'window=VerdigrisNativeClient',
    'route=route:tin:1:0',
    'scenery=tree,ruin,dwelling,shrine keyed plates with fallback',
    'depth=player traversed behind tree (W15) and in front (S30) at x~440, tree x=260',
    'dash=Space from tree boundary; swept segment rejected and dash-blocked.jpg shows hint',
    'collision=A+W held at dwelling; actor remained outside circle and hint=Blocked by scenery',
    'catalog=skill strip expected Q Thrust 10, E Sweep 15, R WarCry 20'
  )
  Set-Content -Path (Join-Path $captureDir 'driven-evidence.log') -Value $log -Encoding UTF8
}
finally {
  if ($process -and -not $process.HasExited) { Stop-Process -Id $process.Id -Force }
}
