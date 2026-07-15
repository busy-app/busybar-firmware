#!/usr/bin/env bash
#
# Bootstrap local Python environment for the test suite and run a smoke check.
#
# Optional env vars:
#   PYTHON_BIN      path to Python >= 3.12 (default: auto-detect)
#   DEVICE_IP       override device IP for HTTP probe
#   FORCE_RECREATE  if set, wipe .venv before bootstrap
#   SKIP_PROBE      if set, skip device HTTP probe
#   SKIP_SMOKE      if set, skip the pytest smoke run
#

set -euo pipefail

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV="$TESTS_DIR/.venv"
cd "$TESTS_DIR"

say() { printf '\n\033[1;34m==>\033[0m %s\n' "$*"; }

say "Selecting Python interpreter"
PY="${PYTHON_BIN:-$(command -v python3.12 || command -v /opt/homebrew/bin/python3.12 || true)}"
if [[ ! -x "$PY" ]]; then
    echo "Python 3.12 not found. Install via 'brew install python@3.12' or set PYTHON_BIN." >&2
    exit 1
fi
echo "    $PY ($("$PY" --version 2>&1))"

say "Preparing virtualenv at .venv"
[[ -n "${FORCE_RECREATE:-}" ]] && rm -rf "$VENV"
[[ -d "$VENV" ]] || "$PY" -m venv "$VENV"

"$VENV/bin/pip" install -q --upgrade pip
[[ -x "$VENV/bin/poetry" ]] || "$VENV/bin/pip" install -q poetry


say "Installing dependencies via Poetry"
export VIRTUAL_ENV="$VENV"
export POETRY_VIRTUALENVS_IN_PROJECT=true
export POETRY_VIRTUALENVS_CREATE=false

if [[ ! -f poetry.lock || pyproject.toml -nt poetry.lock ]]; then
    "$VENV/bin/poetry" lock
fi
"$VENV/bin/poetry" install --no-root

# Anchor Poetry to .venv so `poetry run pytest` works regardless of shell state.
( unset VIRTUAL_ENV && "$VENV/bin/poetry" env use "$VENV/bin/python" >/dev/null )


say "Checking .env file"
if [[ ! -f config/.env ]]; then
    [[ -f config/.env.example ]] && cp config/.env.example config/.env
    echo "    WARN: tests/config/.env created from template — fill in real values" >&2
fi

say "Validating Config.validate_paths()"
"$VENV/bin/python" -c "
import sys; sys.path.insert(0, '.')
from config.config import Config
Config.validate_paths()
print(f'    BSB_FIRMWARE_PATH = {Config.BSB_FIRMWARE_PATH}')
print(f'    DAPLINK_U5_ID     = {Config.DAPLINK_U5_ID or \"<unset>\"}')
"

if [[ -z "${SKIP_PROBE:-}" ]]; then
    IP="${DEVICE_IP:-$(awk -F= '/^BUSYBAR_IP=/{gsub(/["'\'']/,"",$2); print $2; exit}' config/.env 2>/dev/null)}"
    IP="${IP:-10.0.4.20}"
    say "Probing device at $IP"
    if resp="$(curl -fs --max-time 3 "http://$IP/api/version")"; then
        echo "    $resp"
    else
        echo "    WARN: $IP /api/version unreachable" >&2
    fi
fi

if [[ -z "${SKIP_SMOKE:-}" ]]; then
    say "Running smoke tests"
    "$VENV/bin/pytest" -v --timeout=60 -o "addopts=" \
        integration/cli/test_shell.py::TestCLIShell::test_cli_command_help \
        integration/cli/test_diagnostics.py::TestCLIDiagnostics::test_cli_command_uptime
fi

say "Done. Activate with: source .venv/bin/activate"
