param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$CliArgs
)

$ErrorActionPreference = "Stop"

$EnvironmentModule = Join-Path $PSScriptRoot "lib\TestEnvironment.ps1"
. $EnvironmentModule
Import-ForbocTestEnvironment -KeyRequirement Optional
$CommandletResultModule = Join-Path $PSScriptRoot "lib\CommandletResult.ps1"
. $CommandletResultModule

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$Project = Join-Path $Root "ForbocAI_CLI.uproject"
$UE_ROOT = if ($env:UE_ROOT) { $env:UE_ROOT } else { "C:\Program Files\Epic Games\UE_5.8" }
$Build = Join-Path $UE_ROOT "Engine\Build\BatchFiles\Build.bat"
$EditorCmd = Join-Path $UE_ROOT "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if (-not (Test-Path $Project)) {
    throw "CLI host project not found: $Project"
}
if (-not (Test-Path $Build)) {
    throw "Unreal Build.bat not found: $Build. Set UE_ROOT to your UE 5.8 install."
}
if (-not (Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $EditorCmd. Set UE_ROOT to your UE 5.8 install."
}
if ($CliArgs.Count -eq 0) {
    Write-Host "Usage: scripts\forbocai-ue.cmd <command> [subcommand] [args]"
    Write-Host "Examples:"
    Write-Host "  scripts\forbocai-ue.cmd status"
    Write-Host "  scripts\forbocai-ue.cmd doctor"
    Write-Host "  scripts\forbocai-ue.cmd --api-url https://api.forboc.ai status"
    Write-Host "Environment:"
    Write-Host "  FORBOCAI_API_URL overrides the SDK default; FORBOCAI_API_KEY is loaded from the test env file when available."
    exit 2
}

if ($env:FORBOCAI_UE_SKIP_BUILD -ne "1") {
    & $Build "ForbocAI_CLI_Editor" "Win64" "Development" "-Project=$Project" "-WaitMutex" "-NoHotReloadFromIDE" "-NoUBTMakefiles"
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

$CommandletArgs = @(
    $Project,
    "-run=ForbocAI",
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

Invoke-ForbocCommandlet -Executable $EditorCmd -Arguments $CommandletArgs -SuccessMarker "[RESULT] Command completed successfully"
exit 0
