param(
    [string]$Filter = "ForbocAI"
)

$ErrorActionPreference = "Stop"

$ScriptsDir = Split-Path -Parent $PSScriptRoot
$Root = Split-Path -Parent $ScriptsDir
$Project = Join-Path $Root "test-game-cli\ForbocAI_SDK.uproject"
$UERoot = if ($env:UE_ROOT) { $env:UE_ROOT } else { "C:\Program Files\Epic Games\UE_5.8" }
$Build = Join-Path $UERoot "Engine\Build\BatchFiles\Build.bat"
$EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

. (Join-Path $ScriptsDir "lib\TestEnvironment.ps1")
Import-ForbocTestEnvironment -KeyRequirement Optional
. (Join-Path $ScriptsDir "lib\PluginHost.ps1")

if (-not (Test-Path $Project)) {
    throw "UE test-game host project not found: $Project"
}
if (-not (Test-Path $Build)) {
    throw "Unreal Build.bat not found: $Build. Set UE_ROOT to your UE 5.8 install."
}
if (-not (Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $EditorCmd. Set UE_ROOT to your UE 5.8 install."
}

Initialize-ForbocPluginHost -Root $Root

if ($env:FORBOCAI_UE_SKIP_BUILD -ne "1") {
    & $Build "ForbocAI_SDK_Editor" "Win64" "Development" "-Project=$Project" "-WaitMutex" "-NoHotReloadFromIDE"
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

$Arguments = @(
    $Project,
    "-ExecCmds=Automation RunTests $Filter",
    "-TestExit=Automation Test Queue Empty",
    "-nosplash",
    "-nopause",
    "-unattended",
    "-NoSound",
    "-NoRHI",
    "-NullRHI",
    "-stdout",
    "-FullStdOutLogOutput",
    "-FORCELOGFLUSH"
)

& $EditorCmd @Arguments 2>&1 | Tee-Object -Variable CapturedOutput
$Status = $LASTEXITCODE
$Output = $CapturedOutput -join [Environment]::NewLine

if ($Status -ne 0) {
    throw "Unreal automation exited with status $Status."
}
if (-not $Output.Contains("Automation Test Queue Empty")) {
    throw "Unreal exited without completing the automation queue."
}
if ($Output -match 'Test Completed\. Result=\{(Fail|NotRun)\}') {
    throw "One or more Unreal automation tests did not pass."
}

$FoundMatches = [regex]::Matches($Output, 'Found ([0-9]+) automation tests based on')
$PerformedMatches = [regex]::Matches($Output, 'Automation Test Queue Empty ([0-9]+) tests performed')
$Successful = [regex]::Matches($Output, 'Test Completed\. Result=\{Success\}').Count

if ($FoundMatches.Count -eq 0) {
    throw "Unreal did not report a discovered test count for '$Filter'."
}
if ($PerformedMatches.Count -eq 0) {
    throw "Unreal did not report a performed test count for '$Filter'."
}

$Found = [int]$FoundMatches[$FoundMatches.Count - 1].Groups[1].Value
$Performed = [int]$PerformedMatches[$PerformedMatches.Count - 1].Groups[1].Value

if ($Found -eq 0) {
    throw "No Unreal automation tests matched '$Filter'."
}
if ($Performed -eq 0) {
    throw "Unreal completed the queue without performing a test for '$Filter'."
}
if (($Found -ne $Performed) -or ($Successful -ne $Performed)) {
    throw "Unreal automation accounting mismatch: found=$Found performed=$Performed successful=$Successful."
}

Write-Host "Unreal automation verified: $Successful/$Found tests passed for $Filter."
