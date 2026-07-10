param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$CliArgs
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$Project = Join-Path $Root "test-game-cli\ForbocAI_SDK.uproject"
$UE_ROOT = if ($env:UE_ROOT) { $env:UE_ROOT } else { "C:\Program Files\Epic Games\UE_5.8" }
$Build = Join-Path $UE_ROOT "Engine\Build\BatchFiles\Build.bat"
$EditorCmd = Join-Path $UE_ROOT "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$PluginHost = Join-Path $Root "test-game-cli\.forbocai-plugin-host"
$PluginLink = Join-Path $PluginHost "ForbocAI_SDK"

function Ensure-PluginHost {
    if (-not (Test-Path $PluginHost)) {
        New-Item -ItemType Directory -Path $PluginHost | Out-Null
    }

    if (Test-Path $PluginLink) {
        $Item = Get-Item $PluginLink
        if ($Item.LinkType -eq "Junction" -or $Item.LinkType -eq "SymbolicLink") {
            if ($Item.Target -ne $Root) {
                Remove-Item $PluginLink -Force
                New-Item -ItemType Junction -Path $PluginLink -Target $Root | Out-Null
            }
            return
        }
        throw "Plugin host path already exists and is not a junction/symlink: $PluginLink"
    }

    New-Item -ItemType Junction -Path $PluginLink -Target $Root | Out-Null
}

if (-not (Test-Path $Project)) {
    throw "UE test-game host project not found: $Project"
}
if (-not (Test-Path $Build)) {
    throw "Unreal Build.bat not found: $Build. Set UE_ROOT to your UE 5.8 install."
}
if (-not (Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $EditorCmd. Set UE_ROOT to your UE 5.8 install."
}

Ensure-PluginHost

if ($env:FORBOCAI_UE_SKIP_BUILD -ne "1") {
    & $Build "ForbocAI_SDK_Editor" "Win64" "Development" "-Project=$Project" "-WaitMutex" "-NoHotReloadFromIDE"
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

$CommandletArgs = @(
    $Project,
    "-run=ForbocAITestGame",
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

& $EditorCmd @CommandletArgs
exit $LASTEXITCODE
