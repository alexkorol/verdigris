# Captures the running native client window to a PNG via PrintWindow.
# This is how an agent (or CI) SEES the game: agent-harness screenshots of
# the desktop are typically masked or unavailable, which is exactly how
# "renders technically, unplayable visually" shipped repeatedly. Run the
# client (native/tools/play-native.ps1), then:
#
#   powershell -NoProfile -File native/tools/capture-window.ps1 -OutPath out.png
#
# Works while the window is unfocused or behind other windows. Post keys to
# the window with PostMessageA (WM_KEYDOWN/UP on the returned class name
# 'VerdigrisNativeClient') to drive flows headlessly; see
# docs/rebuild/HANDOFF.md 2026-08-30 for worked examples.
param([string]$OutPath = "native\build\logs\capture.png")

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System; using System.Runtime.InteropServices; using System.Text;
public struct VGCapRect { public int L,T,R,B; }
public class VGCap {
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc cb, IntPtr l);
  public delegate bool EnumWindowsProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out VGCapRect r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
  public static IntPtr Found = IntPtr.Zero;
  public static bool Cb(IntPtr h, IntPtr l) {
    if (!IsWindowVisible(h)) return true;
    StringBuilder sb = new StringBuilder(256);
    GetClassName(h, sb, 256);
    if (sb.ToString() == "VerdigrisNativeClient") { Found = h; return false; }
    return true;
  }
}
'@
[VGCap]::EnumWindows([VGCap+EnumWindowsProc]{ param($h, $l) [VGCap]::Cb($h, $l) }, [IntPtr]::Zero) | Out-Null
$hwnd = [VGCap]::Found
if ($hwnd -eq [IntPtr]::Zero) {
  Write-Output "capture-window: no VerdigrisNativeClient window found (is the client running?)"
  exit 1
}
$rect = New-Object VGCapRect
[void][VGCap]::GetWindowRect($hwnd, [ref]$rect)
$width = $rect.R - $rect.L
$height = $rect.B - $rect.T
if ($width -le 0 -or $height -le 0) { Write-Output "capture-window: degenerate window rect"; exit 1 }
$bmp = New-Object System.Drawing.Bitmap $width, $height
$graphics = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $graphics.GetHdc()
$ok = [VGCap]::PrintWindow($hwnd, $hdc, 2)  # PW_RENDERFULLCONTENT
$graphics.ReleaseHdc($hdc)
$graphics.Dispose()
$dir = Split-Path -Parent $OutPath
if ($dir -and -not (Test-Path $dir)) { New-Item -ItemType Directory -Force $dir | Out-Null }
$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output "capture-window: ok=$ok $OutPath ($width x $height)"
