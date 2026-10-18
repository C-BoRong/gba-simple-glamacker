param([switch]$SkipEmulator)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    & (Join-Path $PSScriptRoot 'build.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
    $testArgs = @('scripts/test_all.py')
    if ($SkipEmulator) { $testArgs += '--skip-emulator' }
    & $env:GBA_PYTHON @testArgs
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
} finally { Pop-Location }
