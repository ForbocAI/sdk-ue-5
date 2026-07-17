$ErrorActionPreference = "Stop"

$Python = if (Get-Command python -ErrorAction SilentlyContinue) {
    "python"
} elseif (Get-Command py -ErrorAction SilentlyContinue) {
    "py"
} else {
    throw "Python 3 is required to promote an Unreal quality baseline."
}

& $Python (Join-Path $PSScriptRoot "promote-baseline.py") @args
exit $LASTEXITCODE
