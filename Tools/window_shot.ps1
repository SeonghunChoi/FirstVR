# 언리얼 에디터 창을 PNG로 찍는다(다른 창에 가려져 있어도 PrintWindow로 그린다).
#   powershell -ExecutionPolicy Bypass -File Tools/window_shot.ps1 -Out Saved/Captures/editor.png
#   -Title ST_Enemy  : 메인 창 대신 제목에 그 글자가 든 에디터 창(따로 뜬 에셋 편집기 창 등)
#   -List            : 에디터 프로세스의 보이는 최상위 창 제목만 출력
param([string]$Out = "Saved/Captures/editor.png", [string]$Crop = "", [string]$Title = "", [switch]$List)   # Crop "x,y,w,h" (창 픽셀) 이면 그 부분만 저장

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
public static class Win {
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  public delegate bool EnumProc(IntPtr h, IntPtr p);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr p);
  public struct RECT { public int Left, Top, Right, Bottom; }
  public static List<IntPtr> Windows(uint pid) {
    var list = new List<IntPtr>();
    EnumWindows((h, p) => { uint id; GetWindowThreadProcessId(h, out id); if (id == pid && IsWindowVisible(h)) list.Add(h); return true; }, IntPtr.Zero);
    return list;
  }
  public static string Text(IntPtr h) { var s = new StringBuilder(512); GetWindowText(h, s, 512); return s.ToString(); }
}
"@

$p = Get-Process UnrealEditor -ErrorAction Stop | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if ($List) { foreach ($w in [Win]::Windows([uint32]$p.Id)) { $t = [Win]::Text($w); if ($t) { Write-Output $t } }; exit 0 }
$h = $p.MainWindowHandle
if ($Title) {
  $h = [Win]::Windows([uint32]$p.Id) | Where-Object { ([Win]::Text($_)).Contains($Title) } | Select-Object -First 1
  if (-not $h) { Write-Error "no editor window with title containing '$Title'"; exit 1 }
}
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
