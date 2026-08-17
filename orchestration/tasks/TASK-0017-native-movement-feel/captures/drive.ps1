# TASK-0017 driven-input harness: PostMessage key input + PrintWindow captures.
# Runs the windowed native client, walks right for a fixed real duration,
# dashes, exercises the wheel zoom blend, and saves small JPEG evidence next
# to this script. The client writes the authoritative per-second position log
# itself via --log-positions.
param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..\..")).Path
)

$ErrorActionPreference = "Stop"
$captures = $PSScriptRoot
$clientExe = Join-Path $RepoRoot "native\build\verdigris_client.exe"
$positionLog = Join-Path $captures "positions.log"
if (Test-Path $positionLog) { Remove-Item $positionLog -Force }

Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class Win32 {
  [DllImport("user32.dll", SetLastError = true)]
  public static extern IntPtr FindWindow(string lpClassName, string lpWindowName);
  [DllImport("user32.dll")]
  public static extern bool PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);
  [DllImport("user32.dll")]
  public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint nFlags);
  [DllImport("user32.dll")]
  public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")]
  public static extern bool SetForegroundWindow(IntPtr hWnd);
  [StructLayout(LayoutKind.Sequential)]
  public struct RECT { public int Left, Top, Right, Bottom; }
}
"@

$WM_KEYDOWN = 0x0100; $WM_KEYUP = 0x0101; $WM_MOUSEWHEEL = 0x020A
$VK_D = 0x44; $VK_SPACE = 0x20; $VK_HOME = 0x24
$PW_RENDERFULLCONTENT = 2

function Send-Key($hwnd, [int]$vk, [uint32]$msg) {
  [Win32]::PostMessage($hwnd, $msg, [IntPtr]$vk, [IntPtr]::Zero) | Out-Null
}

function Save-Frame($hwnd, [string]$name) {
  $rect = New-Object Win32+RECT
  [Win32]::GetWindowRect($hwnd, [ref]$rect) | Out-Null
  $w = $rect.Right - $rect.Left; $h = $rect.Bottom - $rect.Top
  $bmp = New-Object System.Drawing.Bitmap $w, $h
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $hdc = $g.GetHdc()
  [Win32]::PrintWindow($hwnd, $hdc, $PW_RENDERFULLCONTENT) | Out-Null
  $g.ReleaseHdc($hdc); $g.Dispose()
  $codec = [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() |
    Where-Object { $_.MimeType -eq "image/jpeg" }
  $ep = New-Object System.Drawing.Imaging.EncoderParameters 1
  $ep.Param[0] = New-Object System.Drawing.Imaging.EncoderParameter(
    [System.Drawing.Imaging.Encoder]::Quality, [long]78)
  $path = Join-Path $captures $name
  $bmp.Save($path, $codec, $ep)
  $bmp.Dispose()
  $size = (Get-Item $path).Length
  if ($size -gt 250KB) { throw "capture $name exceeds 250KB: $size" }
  Write-Output "capture $name $size bytes"
}

# Launch the windowed client from the repo root so billboard plates resolve.
$process = Start-Process -FilePath $clientExe `
  -ArgumentList "--log-positions", "`"$positionLog`"" `
  -WorkingDirectory $RepoRoot -PassThru
try {
  $hwnd = [IntPtr]::Zero
  for ($i = 0; $i -lt 50 -and $hwnd -eq [IntPtr]::Zero; ++$i) {
    Start-Sleep -Milliseconds 200
    $hwnd = [Win32]::FindWindow("VerdigrisNativeClient", $null)
  }
  if ($hwnd -eq [IntPtr]::Zero) { throw "client window not found" }
  [Win32]::SetForegroundWindow($hwnd) | Out-Null
  Start-Sleep -Milliseconds 700
  Save-Frame $hwnd "01-walk-start.jpg"

  # Hold D for a fixed real duration (3.0s); the 50ms timer owns dispatch.
  Send-Key $hwnd $VK_D $WM_KEYDOWN
  Start-Sleep -Milliseconds 1500
  Save-Frame $hwnd "02-walk-mid.jpg"
  Start-Sleep -Milliseconds 1500
  Send-Key $hwnd $VK_D $WM_KEYUP
  Save-Frame $hwnd "03-walk-end.jpg"

  # Dash: settle the camera, burst, then catch the chase and the settled end.
  Start-Sleep -Milliseconds 900
  Save-Frame $hwnd "04-dash-before.jpg"
  Send-Key $hwnd $VK_SPACE $WM_KEYDOWN
  Send-Key $hwnd $VK_SPACE $WM_KEYUP
  Start-Sleep -Milliseconds 150
  Save-Frame $hwnd "05-dash-mid.jpg"
  Start-Sleep -Milliseconds 900
  Save-Frame $hwnd "06-dash-settled.jpg"

  # Wheel zoom past ~1.05 blends toward the Miniature treatment.
  for ($i = 0; $i -lt 3; ++$i) {
    [Win32]::PostMessage($hwnd, $WM_MOUSEWHEEL, [IntPtr](120 -shl 16), [IntPtr]::Zero) | Out-Null
    Start-Sleep -Milliseconds 120
  }
  Start-Sleep -Milliseconds 500
  Save-Frame $hwnd "07-zoom-miniature-blend.jpg"
  Send-Key $hwnd $VK_HOME $WM_KEYDOWN
  Send-Key $hwnd $VK_HOME $WM_KEYUP
  Start-Sleep -Milliseconds 300
} finally {
  if (-not $process.HasExited) { Stop-Process -Id $process.Id -Force }
}
Write-Output "driven pass complete"
