function Initialize-ForbocPluginHost {
    param([Parameter(Mandatory = $true)][string]$Root)

    $PluginHost = Join-Path $Root "test-game-cli\.forbocai-plugin-host"
    $PluginRoot = Join-Path $PluginHost "ForbocAI_SDK"

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
        $SourcePath = Join-Path $Root $_
        if (-not (Test-Path $SourcePath -PathType Container)) {
            return
        }

        $PluginPath = Join-Path $PluginRoot $_
        $Item = Get-Item $PluginPath -Force -ErrorAction SilentlyContinue
        if ($null -ne $Item) {
            $IsExpectedLink =
                ($Item.LinkType -eq "Junction" -or $Item.LinkType -eq "SymbolicLink") -and
                $Item.Target -eq $SourcePath
            if ($IsExpectedLink) {
                return
            }
            if ($Item.LinkType -ne "Junction" -and $Item.LinkType -ne "SymbolicLink") {
                throw "Plugin host directory already exists and is not a junction/symlink: $PluginPath"
            }
            [System.IO.Directory]::Delete($PluginPath)
        }

        New-Item -ItemType Junction -Path $PluginPath -Target $SourcePath | Out-Null
    }
}
