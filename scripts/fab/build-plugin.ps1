param()

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = (Resolve-Path (Join-Path $ScriptDir "..\..")).Path
$StageDir = if ($env:FORBOCAI_FAB_STAGE_DIR) { $env:FORBOCAI_FAB_STAGE_DIR } else { Join-Path $ProjectRoot "dist\fab\stage\ForbocAI_SDK" }
$OutputDir = if ($env:FORBOCAI_FAB_BUILD_DIR) { $env:FORBOCAI_FAB_BUILD_DIR } else { Join-Path $ProjectRoot "dist\fab\build\ForbocAI_SDK" }
$PluginPath = Join-Path $StageDir "ForbocAI_SDK.uplugin"
$UnrealRoot = if ($env:UE_ROOT) { $env:UE_ROOT } else { "C:\Program Files\Epic Games\UE_5.8" }
$UatPath = Join-Path $UnrealRoot "Engine\Build\BatchFiles\RunUAT.bat"
$Python = if (Get-Command python -ErrorAction SilentlyContinue) { "python" } else { "python3" }

& $Python (Join-Path $ScriptDir "prepare_package.py")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (-not (Test-Path $PluginPath -PathType Leaf)) { throw "Prepared plugin descriptor not found: $PluginPath" }
if (-not (Test-Path $UatPath -PathType Leaf)) { throw "Unreal Automation Tool not found: $UatPath" }
if (Test-Path $OutputDir) { Remove-Item $OutputDir -Recurse -Force }
New-Item (Split-Path -Parent $OutputDir) -ItemType Directory -Force | Out-Null

& $UatPath BuildPlugin "-Plugin=$PluginPath" "-Package=$OutputDir" -Rocket
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Compiled Fab plugin package: $OutputDir"
