$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$artRoot = $PSScriptRoot
$artCanvas = [System.Drawing.Bitmap]::new(1100, 660)
$artGraphics = [System.Drawing.Graphics]::FromImage($artCanvas)
$artGraphics.Clear([System.Drawing.ColorTranslator]::FromHtml('#171d1d'))
$artGraphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$artFont = [System.Drawing.Font]::new('Segoe UI', 12)
$artSmall = [System.Drawing.Font]::new('Segoe UI', 10)
$artInk = [System.Drawing.Brushes]::Gainsboro
function Label($text, $x, $y) { $artGraphics.DrawString($text, $artSmall, $artInk, $x, $y) }
function Draw($file, $x, $y, $w, $h) {
  $artInput = [System.Drawing.Bitmap]::new((Join-Path $artRoot $file))
  $artGraphics.DrawImage($artInput, [System.Drawing.Rectangle]::new($x,$y,$w,$h))
  $artInput.Dispose()
}
$artGraphics.DrawString('Protected art studies - PROPOSALS / awaiting owner approval', $artFont, $artInk, 24, 16)
Label 'Native-size review. Originals preserved. Runtime integration: none. 7 generations / cap 8.' 24 44
Label 'Bowl 01 - bronze / 96 and 64 px' 24 82
Draw 'originals/bowl-01-bronze.png' 24 112 96 96
Draw 'originals/bowl-01-bronze.png' 140 128 64 64
Label 'Bowl 02 - clay / 96 and 64 px' 24 230
Draw 'originals/bowl-02-clay.png' 24 260 96 96
Draw 'originals/bowl-02-clay.png' 140 276 64 64
Label 'Tree 01 / 128 x 256' 262 82
Label 'Tree 02 / 128 x 256' 422 82
Draw 'originals/tree-01-dense.png' 262 112 128 256
Draw 'originals/tree-02-airy.png' 422 112 128 256
Label 'FrameKit source button' 600 82
Draw 'references/btn_primary.png' 600 112 138 55
Label 'Button treatment / 138 x 55' 780 82
Draw 'originals/ui-01-button.png' 780 112 138 55
Label 'FrameKit source panel / 251 x 250' 570 222
Draw 'references/panel_plain.png' 570 254 251 250
Label 'Panel treatment / 251 x 250' 833 222
Draw 'originals/ui-02-panel.png' 833 254 251 250
Label 'Panel held: baked exterior checkerboard; targeted retry also retained for audit.' 570 520
Label 'Bowl materials and tree species are proposals. Tree 02 crown reads pruned; needs taste review.' 24 570
Label 'Review PNG scales images for display only. No cropping, keying, recoloring or runtime asset changes.' 24 598
$artCanvas.Save((Join-Path $artRoot 'native-size-review.png'), [System.Drawing.Imaging.ImageFormat]::Png)
$artGraphics.Dispose(); $artCanvas.Dispose(); $artFont.Dispose(); $artSmall.Dispose()
$artRecords = foreach ($artFile in Get-ChildItem -LiteralPath (Join-Path $artRoot 'originals') -Filter '*.png') {
  $artInput = [System.Drawing.Bitmap]::new($artFile.FullName)
  $artTriage = if ($artFile.Name -like 'ui-02*') { 'reject_for_runtime_baked_checkerboard' } elseif ($artFile.Name -in @('tree-02-airy.png','ui-01-button.png')) { 'needs_visual_review' } else { 'proposal' }
  [ordered]@{ file=('originals/'+$artFile.Name); width=$artInput.Width; height=$artInput.Height; bytes=$artFile.Length; sha256=(Get-FileHash -LiteralPath $artFile.FullName -Algorithm SHA256).Hash.ToLower(); pixelFormat=$artInput.PixelFormat.ToString(); exteriorCornerAlpha=$artInput.GetPixel(0,0).A; centerAlpha=$artInput.GetPixel([int]($artInput.Width/2),[int]($artInput.Height/2)).A; triage=$artTriage; approval='awaiting_user_art_approval'; runtimeIntegrated=$false }
  $artInput.Dispose()
}
$artRecords | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $artRoot 'image-metadata.json') -Encoding utf8
