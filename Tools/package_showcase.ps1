# 배포 빌드 패키징: 꾸민 전투 레벨(L_Combat_Factory)만 쿠킹하고, 그 레벨에서 시작하게 만든다. 결과는 Packaged\<구성>\
#   powershell -ExecutionPolicy Bypass -File Tools/package_showcase.ps1 [-Config Development|Shipping]
#
# 공개 저장소의 DefaultEngine.ini(시작 맵 = 템플릿)를 바꾸지 않기 위해, 패키징하는 동안에만 GameDefaultMap을 바꿨다가
# 끝나면(실패해도) 원래 파일로 되돌린다. 런처판 엔진은 타깃별 CustomConfig(고유 빌드 환경)를 쓸 수 없어서 이렇게 한다.
# -skipcookingeditorcontent는 쓰지 않는다: VR 템플릿 BP_XRPawn·WBP_Menu가 엔진 에디터 콘텐츠(GenericHMD, S_NavLink)를 참조해 쿠킹 오류가 난다.
# -iterate: 지난 쿠킹 결과를 다시 쓴다(첫 쿠킹은 셰이더 때문에 1시간 넘게 걸렸다).
param([string]$Config = "Development")

$ErrorActionPreference = "Stop"
$UE = "F:\Epic Games\Games\UE_5.7"
$Root = "D:\Projects\Unreal\FirstVR"
$Project = "$Root\FirstVR.uproject"
$Map = "/Game/Showcase/Map/L_Combat_Factory"
$Ini = "$Root\Config\DefaultEngine.ini"
$Backup = "$Root\Saved\Package\DefaultEngine.ini.bak"
$Out = "$Root\Packaged\$Config"

New-Item -ItemType Directory -Force "$Root\Saved\Package" | Out-Null
Copy-Item $Ini $Backup -Force
try {
    $text = [System.IO.File]::ReadAllText($Ini)
    $text = [regex]::Replace($text, "(?m)^GameDefaultMap=.*$", "GameDefaultMap=$Map.L_Combat_Factory")
    [System.IO.File]::WriteAllText($Ini, $text, (New-Object System.Text.UTF8Encoding($false)))
    Write-Output "GameDefaultMap -> $Map (temporary)"

    & "$UE\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
        "-project=$Project" -noP4 -utf8output `
        -target=FirstVR -platform=Win64 "-clientconfig=$Config" `
        -build -cook -iterate "-map=$Map" `
        -stage -pak -archive "-archivedirectory=$Out"   # 변수가 든 인수는 따옴표로 감싼다(PowerShell 5.1은 -로 시작하는 토큰의 변수를 펼치지 않는다)
    $code = $LASTEXITCODE
}
finally {
    Copy-Item $Backup $Ini -Force
    Write-Output "DefaultEngine.ini restored"
}
Write-Output "RunUAT exit $code"
exit $code
