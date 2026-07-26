param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$CliArgs
)

$ErrorActionPreference = "Stop"

$EnvironmentModule = Join-Path $PSScriptRoot "lib\TestEnvironment.ps1"
. $EnvironmentModule
Import-ForbocTestEnvironment -KeyRequirement Required
$CommandletResultModule = Join-Path $PSScriptRoot "lib\CommandletResult.ps1"
. $CommandletResultModule
$PluginHostModule = Join-Path $PSScriptRoot "lib\PluginHost.ps1"
. $PluginHostModule

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$Project = Join-Path $Root "micro-game-cli\ForbocAI_SDK.uproject"
$UE_ROOT = if ($env:UE_ROOT) { $env:UE_ROOT } else { "C:\Program Files\Epic Games\UE_5.8" }
$Build = Join-Path $UE_ROOT "Engine\Build\BatchFiles\Build.bat"
$EditorCmd = Join-Path $UE_ROOT "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $Project)) {
    throw "UE micro-game host project not found: $Project"
}
if (-not (Test-Path $Build)) {
    throw "Unreal Build.bat not found: $Build. Set UE_ROOT to your UE 5.8 install."
}
if (-not (Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $EditorCmd. Set UE_ROOT to your UE 5.8 install."
}

Initialize-ForbocPluginHost -Root $Root

if ($env:FORBOCAI_UE_SKIP_BUILD -ne "1") {
    & $Build "ForbocAI_SDK_Editor" "Win64" "Development" "-Project=$Project" "-WaitMutex" "-NoHotReloadFromIDE" "-NoUBTMakefiles"
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

$CommandletArgs = @(
    $Project,
    "-run=ForbocAIMicroGame",
    "-nosplash",
    "-nopause",
    "-unattended",
    "-NoSound",
    "-NoRHI",
    "-NullRHI"
)

for ($Index = 0; $Index -lt $CliArgs.Count; $Index++) {
    $CommandletArgs += "-CliArg$Index=$($CliArgs[$Index])"
}

$SuccessMarker = if ($CliArgs.Count -gt 0 -and $CliArgs[0] -in @("contract", "--help", "-h")) {
    ""
} else {
    "CLI coverage complete."
}
$LogPath = if ($env:FORBOCAI_MICRO_GAME_LOG_PATH) {
    $env:FORBOCAI_MICRO_GAME_LOG_PATH
} else {
    Join-Path $Root "output\micro-game-cli\last.log"
}

Invoke-ForbocCommandlet -Executable $EditorCmd -Arguments $CommandletArgs -SuccessMarker $SuccessMarker -LogPath $LogPath
exit 0
