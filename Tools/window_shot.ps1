# 언리얼 에디터 메인 창을 PNG로 찍는다(다른 창에 가려져 있어도 PrintWindow로 그린다).
#   powershell -ExecutionPolicy Bypass -File Tools/window_shot.ps1 -Out Saved/Captures/editor.png
param([string]$Out = "Saved/Captures/editor.png", [string]$Crop = "")   # Crop "x,y,w,h" (창 픽셀) 이면 그 부분만 저장

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class Win {
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  public struct RECT { public int Left, Top, Right, Bottom; }
}
"@

$p = Get-Process UnrealEditor -ErrorAction Stop | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
$h = $p.MainWindowHandle
$r = New-Object Win+RECT
[Win]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.Right - $r.Left; $ht = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap $w, $ht
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[Win]::PrintWindow($h, $hdc, 2) | Out-Null   # PW_RENDERFULLCONTENT
$g.ReleaseHdc($hdc); $g.Dispose()
if ($Crop) { $c = $Crop.Split(",") | ForEach-Object { [int]$_ }; $part = $bmp.Clone((New-Object System.Drawing.Rectangle $c[0], $c[1], $c[2], $c[3]), $bmp.PixelFormat); $bmp.Dispose(); $bmp = $part }
$full = [System.IO.Path]::GetFullPath($Out)
[System.IO.Directory]::CreateDirectory([System.IO.Path]::GetDirectoryName($full)) | Out-Null
$bmp.Save($full, [System.Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
Write-Output "saved $full ($w x $ht)"
