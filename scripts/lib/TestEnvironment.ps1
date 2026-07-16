function Import-ForbocTestEnvironment {
    param([ValidateSet("Required", "Optional")][string]$KeyRequirement = "Required")

    $EnvFile = if ($env:FORBOCAI_TEST_ENV_FILE) {
        $env:FORBOCAI_TEST_ENV_FILE
    } else {
        Join-Path $HOME ".config\forbocai\test-api.env"
    }

    if (-not $env:FORBOCAI_API_KEY -and (Test-Path $EnvFile -PathType Leaf)) {
        foreach ($RawLine in Get-Content $EnvFile) {
            $Line = $RawLine.Trim()
            if (-not $Line -or $Line.StartsWith("#")) {
                continue
            }
            if ($Line.StartsWith("export ")) {
                $Line = $Line.Substring(7).Trim()
            }
            $Separator = $Line.IndexOf("=")
            if ($Separator -le 0) {
                continue
            }
            $Name = $Line.Substring(0, $Separator).Trim()
            $Value = $Line.Substring($Separator + 1).Trim()
            if ($Value.Length -ge 2 -and (($Value.StartsWith('"') -and $Value.EndsWith('"')) -or ($Value.StartsWith("'") -and $Value.EndsWith("'")))) {
                $Value = $Value.Substring(1, $Value.Length - 2)
            }
            if ($Name -match '^[A-Za-z_][A-Za-z0-9_]*$' -and -not [Environment]::GetEnvironmentVariable($Name, "Process")) {
                [Environment]::SetEnvironmentVariable($Name, $Value, "Process")
            }
        }
    }

    if (-not $env:FORBOCAI_API_URL) {
        $env:FORBOCAI_API_URL = "https://api.forboc.ai"
    }

    if ($KeyRequirement -eq "Required" -and -not $env:FORBOCAI_API_KEY) {
        throw "FORBOCAI_API_KEY is required for live verification. Set it in $EnvFile or FORBOCAI_TEST_ENV_FILE."
    }
}
