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
$PluginRoot = Join-Path $PluginHost "ForbocAI_SDK"

function Sync-PluginDirectory {
    param([Parameter(Mandatory = $true)][string]$Name)

    $SourcePath = Join-Path $Root $Name
    if (-not (Test-Path $SourcePath -PathType Container)) {
        return
    }

    $PluginPath = Join-Path $PluginRoot $Name
    $Item = Get-Item $PluginPath -Force -ErrorAction SilentlyContinue
    if ($null -ne $Item) {
        if (($Item.LinkType -eq "Junction" -or $Item.LinkType -eq "SymbolicLink") -and $Item.Target -eq $SourcePath) {
            return
        }
        if ($Item.LinkType -ne "Junction" -and $Item.LinkType -ne "SymbolicLink") {
            throw "Plugin host directory already exists and is not a junction/symlink: $PluginPath"
        }
        [System.IO.Directory]::Delete($PluginPath)
    }

    New-Item -ItemType Junction -Path $PluginPath -Target $SourcePath | Out-Null
}

function Ensure-PluginHost {
    if (-not (Test-Path $PluginHost)) {
        New-Item -ItemType Directory -Path $PluginHost | Out-Null
    }

    $PluginItem = Get-Item $PluginRoot -Force -ErrorAction SilentlyContinue
    if ($null -ne $PluginItem) {
        if ($PluginItem.LinkType -eq "Junction" -or $PluginItem.LinkType -eq "SymbolicLink") {
            [System.IO.Directory]::Delete($PluginRoot)
        } elseif (-not $PluginItem.PSIsContainer) {
            throw "Plugin host path already exists and is not a directory: $PluginRoot"
        }
    }

    if (-not (Test-Path $PluginRoot)) {
        New-Item -ItemType Directory -Path $PluginRoot | Out-Null
    }

    Copy-Item (Join-Path $Root "ForbocAI_SDK.uplugin") (Join-Path $PluginRoot "ForbocAI_SDK.uplugin") -Force
    @("Config", "Content", "Resources", "Source", "ThirdParty") | ForEach-Object {
        Sync-PluginDirectory -Name $_
    }
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
