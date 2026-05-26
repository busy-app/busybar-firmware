# Bootstrap local Python environment for the test suite and run a smoke check.
# Windows / PowerShell counterpart of bootstrap_local.sh.
#
# Optional env vars (use $env:NAME=... before invocation):
#   PYTHON_BIN      path to Python >= 3.12 (default: auto-detect via 'py -3.12')
#   DEVICE_IP       override device IP for HTTP probe
#   FORCE_RECREATE  if set, wipe .venv before bootstrap
#   SKIP_PROBE      if set, skip device HTTP probe
#   SKIP_SMOKE      if set, skip the pytest smoke run
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File .\bootstrap_local.ps1

$ErrorActionPreference = 'Stop'

$TestsDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $TestsDir '..')).Path
$Venv     = Join-Path $TestsDir '.venv'
Set-Location $TestsDir

function Say($msg) { Write-Host "`n==> $msg" -ForegroundColor Cyan }
function Warn($msg) { Write-Host "    WARN: $msg" -ForegroundColor Yellow }

# ---------------------------------------------------------------------------
# 1. Pick Python interpreter
# ---------------------------------------------------------------------------
Say 'Selecting Python interpreter'
$PY = $env:PYTHON_BIN
if (-not $PY) {
    try {
        $PY = (& py -3.12 -c "import sys; print(sys.executable)").Trim()
    } catch {
        $cand = Get-Command python3.12 -ErrorAction SilentlyContinue
        if ($cand) { $PY = $cand.Source }
    }
}
if (-not $PY -or -not (Test-Path $PY)) {
    Write-Error "Python 3.12 not found. Install from https://www.python.org or set `$env:PYTHON_BIN."
}
$pyver = (& $PY --version)
Write-Host "    $PY ($pyver)"

# ---------------------------------------------------------------------------
# 2. Create / reuse virtualenv
# ---------------------------------------------------------------------------
Say 'Preparing virtualenv at .venv'
if ($env:FORCE_RECREATE -and (Test-Path $Venv)) { Remove-Item -Recurse -Force $Venv }
if (-not (Test-Path $Venv)) { & $PY -m venv $Venv }

$VenvPy     = Join-Path $Venv 'Scripts\python.exe'
$VenvPip    = Join-Path $Venv 'Scripts\pip.exe'
$VenvPoetry = Join-Path $Venv 'Scripts\poetry.exe'
$VenvPytest = Join-Path $Venv 'Scripts\pytest.exe'

& $VenvPip install -q --upgrade pip
if (-not (Test-Path $VenvPoetry)) { & $VenvPip install -q poetry }

# ---------------------------------------------------------------------------
# 3. Install dependencies
# ---------------------------------------------------------------------------
Say 'Installing dependencies via Poetry'
$env:VIRTUAL_ENV = $Venv
$env:POETRY_VIRTUALENVS_IN_PROJECT = 'true'
$env:POETRY_VIRTUALENVS_CREATE     = 'false'

$needLock = $true
if (Test-Path 'poetry.lock') {
    $lockTime = (Get-Item 'poetry.lock').LastWriteTime
    $projTime = (Get-Item 'pyproject.toml').LastWriteTime
    if ($projTime -le $lockTime) { $needLock = $false }
}
if ($needLock) { & $VenvPoetry lock }
& $VenvPoetry install --no-root

# Anchor Poetry to .venv so `poetry run pytest` works regardless of shell state.
$prevVE = $env:VIRTUAL_ENV
$env:VIRTUAL_ENV = $null
& $VenvPoetry env use $VenvPy | Out-Null
$env:VIRTUAL_ENV = $prevVE

# ---------------------------------------------------------------------------
# 4. Ensure .env file exists
# ---------------------------------------------------------------------------
Say 'Checking .env file'
$EnvFile = Join-Path $TestsDir 'config\.env'
if (-not (Test-Path $EnvFile)) {
    if (Test-Path 'config\.env.example') {
        Copy-Item 'config\.env.example' $EnvFile
    }
    Warn 'tests\config\.env created from template — fill in real values'
}

# Patch Linux placeholder paths with Windows-correct values.
# Only lines that still hold Linux defaults (start with / or ~) are replaced;
# values already customized by the user are left unchanged.
$envLines = Get-Content $EnvFile
$needsPatch = $envLines | Where-Object { $_ -match '^(BSB_FIRMWARE_PATH|PROJECT_WORKSPACE|CRASH_FLAG_PATH)=[/~]' }
if ($needsPatch) {
    $envLines | ForEach-Object {
        switch -Regex ($_) {
            '^BSB_FIRMWARE_PATH=[/~]' { "BSB_FIRMWARE_PATH=$RepoRoot" }
            '^PROJECT_WORKSPACE=[/~]' { "PROJECT_WORKSPACE=$RepoRoot" }
            '^CRASH_FLAG_PATH=/tmp/'  { "CRASH_FLAG_PATH=$env:TEMP\crash_detected.flag" }
            default                   { $_ }
        }
    } | Set-Content $EnvFile -Encoding utf8
}

# ---------------------------------------------------------------------------
# 5. Validate Config paths
# ---------------------------------------------------------------------------
Say 'Validating Config.validate_paths()'
& $VenvPy -c @"
import sys; sys.path.insert(0, '.')
from config.config import Config
Config.validate_paths()
print(f'    BSB_FIRMWARE_PATH = {Config.BSB_FIRMWARE_PATH}')
print(f'    DAPLINK_U5_ID     = {Config.DAPLINK_U5_ID or \"<unset>\"}')
"@

# ---------------------------------------------------------------------------
# 6. Probe device
# ---------------------------------------------------------------------------
if (-not $env:SKIP_PROBE) {
    $IP = $env:DEVICE_IP
    if (-not $IP -and (Test-Path 'config\.env')) {
        $line = Select-String -Path 'config\.env' -Pattern '^BUSYBAR_IP=' | Select-Object -First 1
        if ($line) { $IP = ($line.Line -replace '^BUSYBAR_IP=', '').Trim('"', "'") }
    }
    if (-not $IP) { $IP = '10.0.4.20' }
    Say "Probing device at $IP"
    try {
        $resp = Invoke-RestMethod -Uri "http://$IP/api/version" -TimeoutSec 3
        Write-Host "    $($resp | ConvertTo-Json -Compress)"
    } catch {
        Warn "$IP /api/version unreachable"
    }
}

# ---------------------------------------------------------------------------
# 7. Smoke pytest run
# ---------------------------------------------------------------------------
if (-not $env:SKIP_SMOKE) {
    Say 'Running smoke tests'
    & $VenvPytest -v --timeout=60 -o 'addopts=' `
        'integration/cli/test_commands.py::TestCLICommandsSession::test_cli_command_help' `
        'integration/cli/test_commands.py::TestCLICommandsSession::test_cli_command_uptime'
}

Say 'Done. Activate with: .\.venv\Scripts\Activate.ps1'
