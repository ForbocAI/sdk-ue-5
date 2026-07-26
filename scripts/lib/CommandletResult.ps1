function Invoke-ForbocCommandlet {
    param(
        [Parameter(Mandatory = $true)][string]$Executable,
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [string]$SuccessMarker = "",
        [string]$LogPath = ""
    )

    $TemporaryOutput = -not $LogPath
    $OutputPath = if ($TemporaryOutput) {
        [System.IO.Path]::GetTempFileName()
    } else {
        $Parent = Split-Path -Parent $LogPath
        New-Item -ItemType Directory -Force -Path $Parent | Out-Null
        $LogPath
    }

    try {
        & $Executable @Arguments "-stdout" "-FullStdOutLogOutput" "-FORCELOGFLUSH" 2>&1 | Tee-Object -FilePath $OutputPath
        $CommandStatus = $LASTEXITCODE
        $CommandText = Get-Content -Raw -Path $OutputPath

        if ($CommandStatus -ne 0) {
            throw "Unreal commandlet exited with status $CommandStatus."
        }
        if ($SuccessMarker -and -not $CommandText.Contains($SuccessMarker)) {
            throw "Unreal exited without the required commandlet success marker: $SuccessMarker"
        }
    } finally {
        if ($TemporaryOutput) {
            Remove-Item -Force -Path $OutputPath -ErrorAction SilentlyContinue
        }
    }
}
