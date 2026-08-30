param()

$ErrorActionPreference = "Stop"
$Python = if (Get-Command python -ErrorAction SilentlyContinue) { "python" } else { "python3" }
& (Join-Path $PSScriptRoot "build-plugin.ps1")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $Python (Join-Path $PSScriptRoot "archive_package.py")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $Python (Join-Path $PSScriptRoot "render_listing.py")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $Python (Join-Path $PSScriptRoot "generate_media.py")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $Python (Join-Path $PSScriptRoot "validate_listing.py")
exit $LASTEXITCODE
