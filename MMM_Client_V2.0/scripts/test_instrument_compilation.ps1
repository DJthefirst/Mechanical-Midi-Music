param(
    [string]$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$ConfigFile = "platformio.ini",
    [int]$MaxConfigs = 0
)

$ErrorActionPreference = "Stop"

$projectIniPath = Join-Path $ProjectRoot $ConfigFile
if (-not (Test-Path $projectIniPath)) {
    throw "Could not find PlatformIO config: $projectIniPath"
}

$platformioExe = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\platformio.exe"
if (-not (Test-Path $platformioExe)) {
    $platformioExe = "platformio"
}

$lines = Get-Content -Path $projectIniPath

$selectedStart = -1
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match '^\s*\[env:selected\]\s*$') {
        $selectedStart = $i
        break
    }
}

if ($selectedStart -lt 0) {
    throw "[env:selected] section not found in $projectIniPath"
}

$selectedEnd = $lines.Count
for ($i = $selectedStart + 1; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match '^\s*\[.*\]\s*$') {
        $selectedEnd = $i
        break
    }
}

$extendEntries = @()
for ($i = $selectedStart + 1; $i -lt $selectedEnd; $i++) {
    if ($lines[$i] -match '^\s*;?\s*extends\s*=\s*(.+?)\s*$') {
        $extendValue = $Matches[1].Trim()
        if (-not [string]::IsNullOrWhiteSpace($extendValue)) {
            $extendEntries += [PSCustomObject]@{
                LineIndex = $i
                Value = $extendValue
            }
        }
    }
}

if ($extendEntries.Count -eq 0) {
    throw "No extends entries found under [env:selected] in $projectIniPath"
}

if ($MaxConfigs -gt 0 -and $MaxConfigs -lt $extendEntries.Count) {
    $extendEntries = @($extendEntries | Select-Object -First $MaxConfigs)
}

Write-Host "Found $($extendEntries.Count) selectable configurations in [env:selected]." -ForegroundColor Cyan

$results = @()
$tempFiles = @()

try {
    foreach ($entry in $extendEntries) {
        $tempLines = [System.Collections.Generic.List[string]]::new()
        for ($lineIndex = 0; $lineIndex -lt $lines.Count; $lineIndex++) {
            $line = $lines[$lineIndex]
            if ($lineIndex -ge ($selectedStart + 1) -and $lineIndex -lt $selectedEnd -and $line -match '^\s*;?\s*extends\s*=') {
                if ($lineIndex -eq $entry.LineIndex) {
                    $tempLines.Add("extends = $($entry.Value)")
                }
                else {
                    $tempLines.Add("; extends = $(([regex]::Match($line, '^\s*;?\s*extends\s*=\s*(.+?)\s*$')).Groups[1].Value.Trim())")
                }
            }
            else {
                $tempLines.Add($line)
            }
        }

        $safeName = ($entry.Value -replace '[^A-Za-z0-9_\-]+', '_')
        $tempPath = Join-Path $ProjectRoot ("platformio.selected.$safeName.ini")
        $tempFiles += $tempPath

        $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
        [System.IO.File]::WriteAllLines($tempPath, $tempLines, $utf8NoBom)

        Write-Host "\n=== Building: $($entry.Value) ===" -ForegroundColor Yellow
        Push-Location $ProjectRoot
        try {
            & $platformioExe run -e selected --project-conf $tempPath
            $exitCode = $LASTEXITCODE
        }
        finally {
            Pop-Location
        }

        $passed = ($exitCode -eq 0)
        $results += [PSCustomObject]@{
            Config = $entry.Value
            Status = if ($passed) { "PASS" } else { "FAIL" }
            ExitCode = $exitCode
        }

        if ($passed) {
            Write-Host "Result: PASS ($($entry.Value))" -ForegroundColor Green
        }
        else {
            Write-Host "Result: FAIL ($($entry.Value))" -ForegroundColor Red
        }
    }
}
finally {
    foreach ($tmp in $tempFiles) {
        if (Test-Path $tmp) {
            Remove-Item $tmp -Force -ErrorAction SilentlyContinue
        }
    }
}

Write-Host "\n=== Selected Config Build Summary ===" -ForegroundColor Cyan
$results | Format-Table -AutoSize

$failures = @($results | Where-Object { $_.Status -eq "FAIL" })
if ($failures.Count -gt 0) {
    Write-Host "\nFailed configurations:" -ForegroundColor Red
    $failures | ForEach-Object { Write-Host " - $($_.Config) (exit code $($_.ExitCode))" -ForegroundColor Red }
    exit 1
}

Write-Host "\nAll selected configurations compiled successfully." -ForegroundColor Green
exit 0
