function Invoke-ForbocCommandlet {
    param(
        [Parameter(Mandatory = $true)][string]$Executable,
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [Parameter(Mandatory = $true)][string]$SuccessMarker
    )

    & $Executable @Arguments "-stdout" "-FullStdOutLogOutput" "-FORCELOGFLUSH" 2>&1 | Tee-Object -Variable CapturedOutput
    $CommandStatus = $LASTEXITCODE
    $CommandText = $CapturedOutput -join [Environment]::NewLine

    if ($CommandStatus -ne 0) {
        throw "Unreal commandlet exited with status $CommandStatus."
    }
    if (-not $CommandText.Contains($SuccessMarker)) {
        throw "Unreal exited without the required commandlet success marker: $SuccessMarker"
    }
}
