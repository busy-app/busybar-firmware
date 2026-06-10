# tests/ — Agent guide

How to write and modify integration tests in this directory. Read this before
adding new tests.

## Environment

- Python 3.12 + Poetry. Bootstrap: `./bootstrap_local.sh` (macOS/Linux) or
  `./bootstrap_local.ps1` (Windows). Creates `tests/.venv`, installs deps,
  validates config, runs a smoke test to verify setup.
- All settings in `tests/config/.env` (template: `tests/config/.env.example`).
  Loaded explicitly by `config/config.py`.
- Run tests from `tests/`: `poetry run pytest ...` or `.venv/bin/pytest ...`.

## Where things live

| What | Where |
| --- | --- |
| Pydantic models (request/response) | `tests/clients/api/*.py` — each module exports both the client class and its models |
| Typed API clients | `tests/clients/api/{system,wifi,ble,storage,assets,account,settings,input,streaming,update,busy,matter}.py` + `clients/api/__init__.py` re-exports |
| Base API utilities | `tests/clients/api/base.py` (`BaseAPI`, `APIError`) |
| CLI client | `tests/clients/cli.py` (`SimpleCLIConnection`) |
| MQTT cloud client | `tests/clients/mqtt_cloud.py` |
| State publisher transports (WS/MQTT/BLE) | `tests/clients/state_publisher/transports/*.py` |
| Protobuf bindings | `tests/clients/state_pb/_generated/` (auto-generated, gitignored) |
| Test config | `tests/config/config.py` (`Config` class) |
| Utilities | `tests/utils/` — `crash_detector`, `device_flasher`, `logging_config`, `busy_timer`, `simple_websocket`, `protobuf_wire` |

When you need a request/response shape, **import the existing Pydantic model
from `clients/api`** instead of building dicts. Example:
```python
from clients.api import SystemAPI, VersionResponse

def test_version(system_api: SystemAPI):
    resp: VersionResponse = system_api.get_version()
    assert resp.api_semver
```

## Fixtures (global, from `tests/conftest.py`)

Prefer these over building your own clients.

| Fixture | Scope | Use it for                                                                                                                                                                |
| --- | --- |---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `web_base_url` | session | `http://<device-ip>` derived from env                                                                                                                                     |
| `web_session` | function | `requests.Session` with default `timeout=10`, User-Agent set, every request logged via `log_web_request`. **Use this for all raw HTTP**                                   |
| `api_session` | function | `web_session` + `Accept: application/json`                                                                                                                                |
| `api_auth_session` | function | placeholder for future `X-API-Token`; identical to `api_session` today                                                                                                    |
| `api_factory` | function | `lambda cls: cls(api_session, web_base_url)` — only needed if instantiating a client not yet wired                                                                        |
| Typed API fixtures | function | `system_api`, `wifi_api`, `storage_api`, `assets_api`, `account_api`, `ble_api`, `settings_api`, `input_api`, `streaming_api`, `update_api`, `busy_api`, `smart_home_api` |
| `persistent_cli_connection` | module | one CLI connection per module; cheaper for command-spam tests                                                                                                             |
| `fresh_cli_connection` | function | new CLI connection per test; use when testing connection lifecycle                                                                                                        |
| `cli_device_info` | function | runs `device_info`, attaches output to Allure, reconnects once on empty reply                                                                                             |
| `device_flasher` | session | `DeviceFlasher` for explicit reset/flash. Health monitor uses it autouse — you usually don't need to call it                                                              |
| `test_logger` | function | per-test logger; usually unnecessary, the device-health monitor already structures the output                                                                             |

**Autouse (active without being listed in the signature):**
- `skip_hello_screen` — sends `start`+`back` once per session to dismiss the welcome screen after boot.
- `device_health_monitor` — probes `/api/version` around each test, reboots the device on crash/503/TCP-down, marks the test failed with the reason.

**Subdir conftests add domain-specific fixtures** (`busy_timer_active`,
`schemathesis_schema`, `linked_device_session`, `state_event_driver`, …). Check
the nearest `conftest.py` before adding new ones.

## Writing a new test

### Use the typed API client when the endpoint returns JSON

```python
@pytest.mark.frontend
def test_wifi_status_reports_connection(wifi_api):
    status = wifi_api.get_status()
    assert status.connected is True
```

The model validates the payload; `APIError` carries the HTTP status if the
server fails. Don't reimplement request building, JSON parsing, or status
checking on top of the bare session.

### Use `web_session` (not bare `requests`) when you need raw HTTP

Use bare-session calls only for things a typed client deliberately hides:
status codes, response headers, content negotiation, error pages, CORS,
schemathesis fuzzing, non-JSON payloads.

```python
@pytest.mark.frontend
def test_404_has_html_body(web_session, web_base_url):
    response = web_session.get(f"{web_base_url}/nonexistent")
    assert response.status_code == 404
    assert "text/html" in response.headers.get("content-type", "").lower()
```

**Never** write `requests.get(...)` directly — you lose connection reuse
(matters on this device: the LwIP TCP PCB pool is small, see PR #699 and the
503-handling in `conftest._probe_api_health`), default timeout, and logging.

### Allure: do NOT invent `@allure.id(...)`

```python
# WRONG — fabricated ID
@allure.id("9999")
@allure.title("BSB API. New endpoint")
def test_something(...): ...

# RIGHT — leave the ID off
@allure.title("BSB API. New endpoint")
@pytest.mark.frontend
def test_something(...): ...
```

`@allure.id` maps the test node to a specific TestOps case. Making one up
either collides with an existing case or creates an orphan. New tests must
omit `@allure.id` entirely — TestOps takes care of assignment, and the project
owner backfills the decorator afterward.

Use `@allure.title` and `@allure.step` freely — those only affect display.

### Markers

The full list is in `pyproject.toml` under `[tool.pytest.ini_options].markers`
— treat it as the source of truth, not this file. There's no formal hierarchy;
they're flat tags used for `-m "<expr>"` filtering in CI and locally. Pick at
least one that fits.

Two practical things to know:

- **`regression`** is excluded from PR/dev CI runs and fires only on `-rc`
  tags. Don't put a test under `regression` unless you accept it won't run on
  every PR.
- **`story_*` / `feature_*`** are auto-translated to Allure `story` /
  `feature` tags in `pytest_runtest_setup` — no need to also call
  `allure.dynamic.story(...)` manually.

### Step structure

Wrap assertions in `with allure.step(...):` so failures land at the right
place in the Allure report:

```python
with allure.step("Verify 200 response"):
    assert response.status_code == 200, f"Got {response.status_code}"
```

One step per logical check; don't nest more than two levels deep.

### Asserts

Always include the actual value in the failure message — debugging via Allure
logs only is painful otherwise:

```python
assert resp.status_code == 200, f"Expected 200, got {resp.status_code}: {resp.text[:200]}"
```

## Things to avoid

- imports inside tests — if you need something, import it at the top and add a fixture if necessary.
- `import requests` followed by `requests.get(...)` — use `web_session`.
- Fabricated `@allure.id("xxxx")`.
- Custom `requests.Session()` inside tests — use `web_session`.
- Hardcoded device IPs — read `web_base_url`.
- `time.sleep` longer than ~2 s without an explicit reason; poll instead.
- Catching `APIError` to "make the test pass" — if a typed client raises,
  the assertion you wanted to make probably belongs in a `web_session`-based
  raw test instead.
- New `conftest.py` at random depths — prefer extending the nearest existing
  one.

## Anti-patterns

- **Try not to use concurrency with threads to "reproduce a race."** Keep in mind firing two
  HTTP requests from threads isn't deterministic (network + GI scheduling make the ordering random)
  — it adds flakiness. If the repro is "stop, then immediately redraw", first try send the requests sequentially
  with no wait between them. If you genuinely need concurrency, use the provided session, not a bare one. 
- **never `except BaseException`**
- **A similarity threshold is meaningless without a negative control.**
- **Compare the region that matters, not the whole frame.**
- **Reuse the snapshot helpers.**
- **Avoid magic sleeps and magic numbers.**

## When the device is misbehaving

`device_health_monitor` resets the device automatically. If you see your test
fail with `DEVICE CRASH DETECTED`, `API UNHEALTHY`, or
`DEVICE_NOT_RECOVERABLE`, the test logic is probably fine — check serial logs
under `$SESSION_LOG_DIR` (defaults to `/tmp`) and the Allure attachments.

For one-off reproductions, save scripts in `tests/scripts/`