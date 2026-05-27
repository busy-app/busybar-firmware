# BSB Firmware Integration Tests

Automated pytest suite for the BSB CLI, HTTP API, MQTT, BLE and state-publisher
paths. Runs against a real BusyBar device on the local network.

## Quick start

macOS / Linux:
```bash
cd tests
./bootstrap_local.sh
```

Windows (PowerShell):
```powershell
cd tests
powershell -ExecutionPolicy Bypass -File .\bootstrap_local.ps1
```

The script creates `.venv` (Python 3.12), installs dependencies via Poetry,
validates `config.Config.validate_paths()`, probes the device at `BUSYBAR_IP`,
and runs two CLI smoke tests.

Useful env vars: `PYTHON_BIN`, `DEVICE_IP`, `FORCE_RECREATE`, `SKIP_PROBE`,
`SKIP_SMOKE`.

## Configuration

All settings live in a single file: **`tests/config/.env`** (gitignored).
Loaded by `config/config.py` at import time via `python-dotenv`. Bootstrap
copies `tests/config/.env.example` if no `.env` exists yet — fill it in
before running real tests.

| Variable | Purpose |
| --- | --- |
| `BUSYBAR_IP` | Device IP for HTTP/CLI/MQTT (default `10.0.4.20`) |
| `DEVICE_CHECK_PORT` | TCP port used for liveness probe (default `80`) |
| `BSB_FIRMWARE_PATH` | Local checkout of `bsb-firmware` (validated at startup) |
| `DAPLINK_U5_ID` / `DAPLINK_917_ID` | DAPLink serial numbers for reflash/reset |
| `PROJECT_WORKSPACE` | Workspace path passed to the device flasher |
| `CLOUD_BASE_URL` / `CLOUD_EMAIL` / `CLOUD_PASSWORD` | Cloud-linking tests |
| `WIFI_SSID` / `WIFI_PASSWORD` / `WIFI_SECURITY` | Wi-Fi join tests |
| `BLE_DEVICE_NAME` | BLE advertising name for `uses_ble` tests |
| `SESSION_LOG_DIR` | Where crash detector & 503-incident logs are written |
| `LOG_LEVEL` / `LOG_TO_FILE` | Optional logging overrides |

`Config.validate_paths()` runs on every pytest invocation and aborts the
session if `BSB_FIRMWARE_PATH` or its required sub-paths
(`scripts/debug/platforms/stm32u595.json`, `scripts/toolchain/fbtenv.sh`,
`scripts/debug/platforms/stm32u5/stm32u5x.cfg`) are missing.

Sanity check the resolved values without launching pytest:

```bash
.venv/bin/python -c "
import sys; sys.path.insert(0, '.')
from config.config import Config
Config.validate_paths()
print('BSB_FIRMWARE_PATH =', Config.BSB_FIRMWARE_PATH)
print('BUSYBAR_IP        =', Config.BUSYBAR_IP)
print('DAPLINK_U5_ID     =', Config.DAPLINK_U5_ID or '<unset>')
"
```

## Running tests

```bash
# Activate the venv once
source .venv/bin/activate

# Single file
pytest -v integration/cli/test_commands.py --timeout=120 -o "addopts="

# By marker
pytest -v -m "cli and not regression"
pytest -v -m "state_publisher and not uses_si917"

# Allure report
pytest integration/cli/test_commands.py
allure serve allure-results
```

Pytest defaults (from `pyproject.toml`): `-q --strict-markers
--alluredir=allure-results --clean-alluredir`. Override with `-o "addopts="`
when debugging individual tests.

## Layout

```
tests/
├── bootstrap_local.sh        # one-shot env setup + smoke
├── conftest.py               # global fixtures, device health monitor
├── clients/                  # device clients (CLI, API, BLE, MQTT, state_pb)
├── config/                   # Config + .env loader
├── integration/              # test suites (cli, mqtt, ble, frontend, ...)
└── utils/                    # logging, crash detector, device flasher
```
