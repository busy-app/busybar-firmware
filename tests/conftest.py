import json
import logging
import os
import tempfile
import time
from datetime import datetime
from typing import Iterator, Optional

import allure
import pytest
import requests

from utils.logging_config import (
    get_cli_logger,
    get_web_logger,
    log_web_request,
    setup_logging,
)
from utils.crash_detector import (
    CrashDetector,
    attach_forced_traces_to_allure,
    request_forced_trace_for_node,
)
from utils.device_flasher import DeviceFlasher

# API client imports
from clients.api import (
    SystemAPI,
    WifiAPI,
    StorageAPI,
    AssetsAPI,
    AccountAPI,
    BleAPI,
    SettingsAPI,
    InputAPI,
    StreamingAPI,
    UpdateAPI,
    BusyAPI,
    SmartHomeAPI,
)
from clients.cli import SimpleCLIConnection
from config.config import Config


_API_503_INCIDENT_FILENAME = "api_503_incidents.jsonl"
_USER_AGENT = "BSB-AutoTest/1.0"
_PYTEST_MARKERS = (
    "cli: CLI command tests",
    "frontend: Web frontend tests",
    "api: API endpoint tests",
    "story_commands_check: Commands Check story",
    "story_ui_validation: UI validation story",
    "story_ui_interaction: UI interaction story",
    "story_interface_status: Interface status story",
    "story_mqtt: MQTT story",
    "feature_cli: Feature 6. CLI",
    "feature_web_frontend: Feature 5. Web Frontend",
    "connection_test: Fresh connection tests",
    "schemathesis: OpenAPI schema conformance tests (schemathesis)",
    "uses_si917: test exercises the Si917 coprocessor; forced GDB trace targets Si917",
    "mqtt: MQTT functionality (local or configured broker/cloud)",
    "mtls: mutual TLS authentication tests",
    "uses_cloud: uses real test cloud account/session service",
    "external_service: talks to service outside the device",
    "state_publisher: state publisher transport and protobuf contract tests",
    "uses_ble: requires BLE adapter/hardware path",
    "matter: requires a Matter controller (Home Assistant)",
    "rate_limiter: state publisher rate limiter tests",
    "long_running: long-running hardware regression tests",
    "regression: Heavy regression tests; excluded from PR/dev runs, only fire on -rc tags",
)


def _api_503_incidents_path() -> str:
    """Where to append 503 incident records.

    Prefers `$SESSION_LOG_DIR` (matches `serial_logger` / `_write_test_context`
    output) and falls back to `tests/logs/` so the file is captured even when
    running locally without a session-log directory.
    """
    base = os.environ.get("SESSION_LOG_DIR")
    if not base:
        base = os.path.join(os.path.dirname(__file__), "logs")
    os.makedirs(base, exist_ok=True)
    return os.path.join(base, _API_503_INCIDENT_FILENAME)


def _record_api_503_incident(
    *, nodeid: str, phase: str, raw_error: str, reset_ok: Optional[bool]
) -> Optional[str]:
    """Append one JSONL record for a tolerated 503. Returns the file path or None."""
    try:
        path = _api_503_incidents_path()
        record = {
            "ts": datetime.now().isoformat(timespec="seconds"),
            "nodeid": nodeid,
            "phase": phase,
            "error": raw_error,
            "reset_ok": reset_ok,
        }
        with open(path, "a") as f:
            f.write(json.dumps(record) + "\n")
        return path
    except Exception as exc:
        logger.warning("Failed to record 503 incident: %s", exc)
        return None


def _write_test_context(test_name: str, **extra) -> None:
    """Atomically write test_context.json to $SESSION_LOG_DIR.

    Silent no-op if SESSION_LOG_DIR is not set.
    """
    log_dir = os.environ.get("SESSION_LOG_DIR")
    if not log_dir:
        return

    ctx = {
        "test_name": test_name,
        "updated_at": datetime.now().isoformat(),
        **extra,
    }

    dest = os.path.join(log_dir, "test_context.json")
    try:
        fd, tmp = tempfile.mkstemp(dir=log_dir, suffix=".tmp")
        with os.fdopen(fd, "w") as f:
            json.dump(ctx, f)
        os.rename(tmp, dest)
    except Exception:
        pass


# Setup logging
logger = setup_logging(
    log_level=os.getenv("LOG_LEVEL", "INFO"),
    log_to_file=os.getenv("LOG_TO_FILE", "true").lower() == "true",
)

@pytest.fixture(scope="function")
def test_logger(request) -> logging.Logger:
    """Test-specific logger"""
    from utils.logging_config import get_test_logger

    return get_test_logger(request.node.name)


@pytest.fixture(scope="session")
def web_base_url() -> str:
    """Base URL for web frontend tests"""
    return os.getenv("WEB_BASE_URL", "http://10.0.4.20")


@pytest.fixture
def web_session() -> Iterator[requests.Session]:
    """HTTP session for web frontend tests"""
    logger = get_web_logger()
    logger.info("Creating web session")

    session = requests.Session()
    session.headers.update({"User-Agent": _USER_AGENT})

    # Add response logging and default timeout
    original_request = session.request

    def logged_request(*args, **kwargs):
        # Set default timeout if not provided (prevent infinite hang)
        if "timeout" not in kwargs:
            kwargs["timeout"] = 10
        method = args[0] if args else kwargs.get("method", "GET")
        method_upper = str(method).upper()
        url = args[1] if len(args) > 1 else kwargs.get("url", "")
        params = kwargs.get("params", {})
        if params:
            import urllib.parse
            url = f"{url}?{urllib.parse.urlencode(params, doseq=True)}"
        response = None
        error = None
        start_time = time.time()
        try:
            response = original_request(*args, **kwargs)
            return response
        except requests.ConnectionError as exc:
            error = exc
            if method_upper not in {"GET", "HEAD", "OPTIONS"}:
                raise
            logger.warning(
                "Retrying %s %s after connection error: %s",
                method_upper,
                url,
                exc,
            )
            session.close()
            response = original_request(*args, **kwargs)
            error = None
            return response
        except requests.RequestException as exc:
            error = exc
            raise
        finally:
            duration = time.time() - start_time
            log_web_request(
                method=method,
                url=url,
                duration=duration,
                status_code=getattr(response, "status_code", None),
                error=error,
            )

    session.request = logged_request
    try:
        yield session
    finally:
        logger.info("Closing web session")
        session.close()


@pytest.fixture
def api_session(web_session) -> requests.Session:
    """API session with proper headers for API testing"""
    # Add API-specific headers - only Accept, not Content-Type
    # Content-Type will be set appropriately per request
    web_session.headers.update({"Accept": "application/json"})
    return web_session


@pytest.fixture
def api_auth_session(api_session) -> requests.Session:
    """API session with authentication headers"""
    # TODO: Add X-API-Token header when authentication is required
    return api_session


def pytest_configure(config):
    """Pytest configuration"""
    logger.info("Configuring pytest")

    # Validate required file paths exist before running tests
    try:
        Config.validate_paths()
        logger.info("All required paths validated successfully")
    except FileNotFoundError as e:
        logger.error(f"Path validation failed: {e}")
        raise pytest.UsageError(str(e))

    # Allure TestOps integration
    allure_testops_url = os.getenv("ALLURE_TESTOPS_URL")
    allure_testops_token = os.getenv("ALLURE_TESTOPS_TOKEN")
    allure_project_id = os.getenv("ALLURE_PROJECT_ID")

    if all([allure_testops_url, allure_testops_token, allure_project_id]):
        logger.info(
            f"Allure TestOps: {allure_testops_url}, Project: {allure_project_id}"
        )
        os.environ["ALLURE_TESTOPS_URL"] = allure_testops_url
        os.environ["ALLURE_TESTOPS_TOKEN"] = allure_testops_token
        os.environ["ALLURE_PROJECT_ID"] = allure_project_id
    else:
        logger.warning("Allure TestOps configuration incomplete")

    for marker in _PYTEST_MARKERS:
        config.addinivalue_line("markers", marker)

    logger.info("Pytest configuration complete")


def _cli_connection(label: str) -> Iterator[SimpleCLIConnection]:
    cli = SimpleCLIConnection()
    cli_logger = get_cli_logger()

    cli_logger.info("Setting up %s CLI connection", label)
    if not cli.connect():
        cli_logger.error("%s CLI connection failed", label.title())
        pytest.skip("Could not connect to CLI")

    try:
        yield cli
    finally:
        cli_logger.info("Cleaning up %s CLI connection", label)
        cli.disconnect()


# Fixtures using SimpleCLIConnection
@pytest.fixture(scope="module")
def persistent_cli_connection():
    """Module-scoped CLI fixture.

    Keep the convenience of a persistent CLI within one test module, but do not
    hold a telnet TCP PCB for the full integration suite after CLI tests finish.
    """
    yield from _cli_connection("persistent module-scoped")


@pytest.fixture(scope="function")
def fresh_cli_connection():
    """Function-scoped CLI fixture - creates fresh connection per test (use for connection reliability tests)"""
    yield from _cli_connection("fresh function-scoped")


def pytest_runtest_setup(item):
    """Test setup"""
    for attr in (
        "_connection_error",
        "_crash_info",
        "_device_unavailable",
        "_api_unhealthy",
        "_api_503_tolerated",
        "_api_503_incident_path",
        "_reset_failed",
    ):
        if hasattr(item, attr):
            delattr(item, attr)

    logger.info(f"Setting up: {item.name}")
    _write_test_context(item.name, test_nodeid=item.nodeid, phase="setup")

    for marker in item.iter_markers():
        if marker.name.startswith("story_"):
            story = marker.name.replace("story_", "").replace("_", " ").title()
            allure.dynamic.story(story)
        elif marker.name.startswith("feature_"):
            feature = marker.name.replace("feature_", "").replace("_", " ").title()
            allure.dynamic.feature(feature)


@pytest.fixture(scope="session")
def device_flasher():
    """Session-scoped device flasher for on-demand resets."""
    from config.config import config
    flasher = DeviceFlasher(
        device_ip=config.BUSYBAR_IP,
        firmware_dir=config.BSB_FIRMWARE_PATH,
        serial=config.DAPLINK_U5_ID,
    )
    return flasher


@pytest.fixture(scope="session", autouse=True)
def skip_hello_screen(web_base_url):
    """Send 'start' key to dismiss the Hello/Start screen after boot.

    The device shows a welcome screen after flashing that blocks all
    display draw operations.  Pressing 'start' advances the app to its
    normal state so that tests can interact with the display.
    """
    url = f"{web_base_url}/api/input"
    try:
        with requests.Session() as session:
            session.headers.update({"User-Agent": _USER_AGENT})
            with session.post(url, params={"key": "start"}, data=b"", timeout=5):
                pass
            with session.post(
                url, params={"key": "back"}, data=b"", timeout=5
            ) as resp:
                status_code = resp.status_code
        logger.info(
            "skip_hello_screen: POST /api/input?key=back -> %s", status_code
        )
        time.sleep(1)  # let the device transition
    except requests.RequestException as exc:
        logger.warning("skip_hello_screen: failed to send start key: %s", exc)


def pytest_runtest_teardown(item, nextitem):
    """Test teardown"""
    logger.info(f"Test completed: {item.name}")
    _write_test_context(item.name, test_nodeid=item.nodeid, phase="teardown")


def _probe_api_health(
    base_url: str, session: Optional[requests.Session] = None, retries: int = 3
) -> Optional[str]:
    """Cheap HTTP liveness probe for /api/version.

    Returns None on success, or a short error string on failure. We only care
    that the server answers 200 — body validation is a test's job.

    Prefer the per-test web_session so test traffic and health traffic can
    reuse the same connection. Avoid `Connection: close`: on the device-side
    lwIP stack that can make the firmware the active TCP closer and keep TCP
    PCBs occupied long enough to trip the PR #699 overload guard.

    Retries up to *retries* times on ConnectionError (e.g. ConnectionResetError
    after a debug-probe reset where TCP is open but the HTTP event loop hasn't
    started yet). Non-connection errors and HTTP error codes are not retried.
    """
    own_session = session is None
    probe_session = session or requests.Session()

    last_error: Optional[str] = None
    try:
        for attempt in range(retries):
            try:
                with probe_session.get(
                    f"{base_url}/api/version", timeout=2.0
                ) as response:
                    if response.status_code != 200:
                        return f"HTTP {response.status_code}"
                # Success — clear error, return.
                last_error = None
                break
            except requests.ConnectionError as exc:
                last_error = f"{type(exc).__name__}: {exc}"
                if attempt < retries - 1:
                    time.sleep(0.5 * (attempt + 1))
            except requests.RequestException as exc:
                last_error = f"{type(exc).__name__}: {exc}"
                break
    finally:
        if own_session:
            probe_session.close()

    return last_error


def _pre_test_reset_reason(device_flasher, web_base_url: str, web_session) -> Optional[str]:
    # Pre-test: trust the previous clean teardown; otherwise one HTTP probe is
    # enough for the normal path and avoids extra TCP connect churn.
    if getattr(device_flasher, "_post_test_healthy", False):
        return None

    api_error = _probe_api_health(web_base_url, web_session)
    if not api_error:
        return None
    if not device_flasher.check_device_available():
        return "TCP port 80 unreachable"
    return f"API unhealthy ({api_error})"


def _reset_before_test(request, device_flasher, reason: str) -> None:
    pre_is_503 = "HTTP 503" in reason
    if not pre_is_503:
        request_forced_trace_for_node(
            request.node,
            reason=f"pre-test: {reason}",
            phase="setup",
        )
    logger.warning("Device not ready before test (%s), resetting...", reason)
    with allure.step(f"Resetting device before test: {reason}"):
        reset_ok = device_flasher.reset_and_wait(wait_timeout=60, reset_interval=15)
        if pre_is_503:
            _record_api_503_incident(
                nodeid=request.node.nodeid,
                phase="pre-test",
                raw_error=reason,
                reset_ok=reset_ok,
            )
        if not reset_ok:
            pytest.fail("Device not recoverable - check hardware connection")


def _post_test_reset_reason(
    request,
    device_flasher,
    web_base_url: str,
    web_session,
    detector: CrashDetector,
) -> tuple[Optional[str], Optional[object]]:
    crash_info = detector.check_for_crash()
    if crash_info:
        request.node._crash_info = crash_info
        return f"crash detected ({crash_info.processor})", crash_info
    if hasattr(request.node, "_connection_error"):
        if request.node.get_closest_marker("schemathesis") is not None:
            api_error = _probe_api_health(web_base_url)
            if not api_error:
                return None, None
            if not device_flasher.check_device_available():
                request.node._device_unavailable = True
                return "TCP port 80 unreachable", None
            if api_error == "HTTP 503":
                request.node._api_503_tolerated = api_error
                return f"API 503 (tolerated): {api_error}", None
            request.node._api_unhealthy = api_error
            return f"API health check failed: {api_error}", None
        return "test raised ConnectionError", None

    api_error = _probe_api_health(web_base_url, web_session)
    if not api_error:
        return None, None
    if not device_flasher.check_device_available():
        request.node._device_unavailable = True
        return "TCP port 80 unreachable", None
    if api_error == "HTTP 503":
        # Tolerated: reboot to free the stack but don't fail the test.
        # The incident is recorded after reset_and_wait below.
        request.node._api_503_tolerated = api_error
        return f"API 503 (tolerated): {api_error}", None

    request.node._api_unhealthy = api_error
    return f"API health check failed: {api_error}", None


def _reset_after_test(
    request,
    device_flasher,
    detector: CrashDetector,
    reason: str,
    crash_info: Optional[object],
) -> None:
    is_503_tolerated = getattr(request.node, "_api_503_tolerated", None) is not None
    if not crash_info and not is_503_tolerated:
        trace = request_forced_trace_for_node(
            request.node,
            reason=f"post-test: {reason}",
            phase="teardown",
        )
        if trace:
            detector.capture_initial_state()

    # Next pre-test must re-verify — reset leaves the health state unknown.
    device_flasher._post_test_healthy = False
    logger.warning("Resetting device after test: %s", reason)
    with allure.step(f"Resetting device: {reason}"):
        reset_ok = device_flasher.reset_and_wait(wait_timeout=60, reset_interval=15)

    if is_503_tolerated:
        incident_path = _record_api_503_incident(
            nodeid=request.node.nodeid,
            phase="post-test",
            raw_error=request.node._api_503_tolerated,
            reset_ok=reset_ok,
        )
        if incident_path:
            request.node._api_503_incident_path = incident_path

    if not reset_ok:
        if is_503_tolerated:
            # Even tolerated 503 needs to flag unrecoverable hardware loudly.
            logger.error("reset_and_wait failed after tolerated 503")
        else:
            logger.error("reset_and_wait failed after test: %s", reason)
        request.node._reset_failed = reason


@pytest.fixture(autouse=True)
def device_health_monitor(request, device_flasher, web_base_url, web_session):
    """
    Auto-use fixture that monitors device health and recovers from failures.

    Before test:
    - If the previous test ended healthy, skip probing entirely.
    - Otherwise probe /api/version through the per-test web_session.
    - If the API probe fails, use a raw TCP check only to classify the failure.

    After test, pick the first matching reason and reset once:
    - Crash flag raised by serial_logger
    - Test raised a ConnectionError
    - HTTP API stopped serving (GET /api/version non-200 / exception)
    - TCP port 80 closed (checked only after API failure)
    """
    pre_reason = _pre_test_reset_reason(device_flasher, web_base_url, web_session)
    if pre_reason:
        _reset_before_test(request, device_flasher, pre_reason)

    # Capture crash detector state
    detector = CrashDetector()
    detector.capture_initial_state()

    yield detector

    # Post-test: evaluate reasons in priority order, reset at most once.
    reset_reason, crash_info = _post_test_reset_reason(
        request, device_flasher, web_base_url, web_session, detector
    )
    if reset_reason:
        _reset_after_test(request, device_flasher, detector, reset_reason, crash_info)
    else:
        # Clean teardown — tell the next pre-test it can skip the API probe.
        device_flasher._post_test_healthy = True


def _append_longrepr(report, msg: str) -> None:
    existing = report.longrepr
    report.longrepr = f"{existing}\n\n{msg}" if existing else msg


def _mark_call_connection_error(item, call) -> None:
    if call.excinfo is None:
        return
    exc_type = call.excinfo.type.__name__
    exc_value = str(call.excinfo.value)
    if (
        "ConnectionError" in exc_type
        or "ConnectTimeout" in exc_type
        or "Connection" in exc_value
        or "No route to host" in exc_value
    ):
        item._connection_error = True
        logger.warning("Connection issue detected in test: %s", exc_value[:100])


def _apply_teardown_report_flags(item, report) -> None:
    append = lambda msg: _append_longrepr(report, msg)

    crash_info = getattr(item, "_crash_info", None)
    if crash_info:
        report.outcome = "failed"
        allure.dynamic.tag("DEVICE_CRASH")
        allure.dynamic.tag(f"crash:{crash_info.processor}")
        report.longrepr = (
            "DEVICE CRASH DETECTED during test!\n"
            f"Processor: {crash_info.processor}\n"
            f"Crash line: {crash_info.crash_line}\n"
            f"Timestamp: {crash_info.timestamp}\n"
            "See allure report for full crash trace."
        )

    if getattr(item, "_device_unavailable", False):
        report.outcome = "failed"
        append(
            "DEVICE UNAVAILABLE after test!\n"
            "The device became unreachable during test execution.\n"
            "This may indicate a crash, hang, or network issue."
        )

    api_unhealthy = getattr(item, "_api_unhealthy", None)
    if api_unhealthy and report.outcome != "failed":
        report.outcome = "failed"
        append(
            "API UNHEALTHY after test!\n"
            "Device TCP port 80 was reachable but GET /api/version failed:\n"
            f"{api_unhealthy}"
        )

    api_503_tolerated = getattr(item, "_api_503_tolerated", None)
    if api_503_tolerated:
        allure.dynamic.tag("API_503_TOLERATED")
        note = (
            "API returned 503 after test (tolerated)\n"
            f"Probe result: {api_503_tolerated}\n"
            "Device was rebooted; the test result is preserved.\n"
        )
        incident_path = getattr(item, "_api_503_incident_path", None)
        append(note + (f"Logged to: {incident_path}\n" if incident_path else ""))

    reset_failed = getattr(item, "_reset_failed", None)
    if reset_failed:
        report.outcome = "failed"
        allure.dynamic.tag("DEVICE_NOT_RECOVERABLE")
        append(
            "DEVICE RECOVERY FAILED after test!\n"
            f"Trigger: {reset_failed}\n"
            "reset_and_wait() did not bring the device back — hardware may be "
            "wedged. Subsequent tests are likely to fail until the runner is "
            "manually recovered."
        )


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """Attach diagnostics and convert device health markers into report failures."""
    outcome = yield
    report = outcome.get_result()

    if report.when == "call":
        if report.outcome in ("failed", "error"):
            from clients.api.streaming import attach_failure_screenshots
            attach_failure_screenshots(os.getenv("WEB_BASE_URL", "http://10.0.4.20"))
        if report.failed:
            _mark_call_connection_error(item, call)
    elif report.when == "teardown":
        _apply_teardown_report_flags(item, report)

    attach_forced_traces_to_allure(item, report, lambda msg: _append_longrepr(report, msg))


@pytest.fixture
def api_factory(api_session, web_base_url):
    """Factory for creating API client instances."""
    return lambda api_class: api_class(api_session, web_base_url)


@pytest.fixture
def system_api(api_factory):
    return api_factory(SystemAPI)


@pytest.fixture
def wifi_api(api_factory):
    return api_factory(WifiAPI)


@pytest.fixture
def storage_api(api_factory):
    return api_factory(StorageAPI)


@pytest.fixture
def assets_api(api_factory):
    return api_factory(AssetsAPI)


@pytest.fixture
def account_api(api_factory):
    return api_factory(AccountAPI)


@pytest.fixture
def ble_api(api_factory):
    return api_factory(BleAPI)


@pytest.fixture
def settings_api(api_factory):
    return api_factory(SettingsAPI)


@pytest.fixture
def input_api(api_factory):
    return api_factory(InputAPI)


@pytest.fixture
def streaming_api(api_factory):
    return api_factory(StreamingAPI)


@pytest.fixture
def update_api(api_factory):
    return api_factory(UpdateAPI)


@pytest.fixture
def busy_api(api_factory):
    return api_factory(BusyAPI)


@pytest.fixture
def smart_home_api(api_factory):
    return api_factory(SmartHomeAPI)


@pytest.fixture
def cli_device_info(persistent_cli_connection):
    """Fetch and attach CLI device_info for cross-verification.

    `execute_command` swallows telnet/EOF errors and returns an empty string,
    so a session-scoped connection that silently degraded (e.g. a previous
    test left it stuck in sl_cli, or the socket got dropped) would surface as
    an opaque `assert ''` failure. Retry once with a forced reconnect so the
    test gets a real answer when the degradation is recoverable.
    """
    cli = persistent_cli_connection
    data = cli.execute_command("device_info", timeout=20.0, slow_command=True)

    if not data.strip():
        cli_logger = get_cli_logger()
        cli_logger.warning(
            "device_info returned empty output — reconnecting CLI and retrying"
        )
        try:
            cli.disconnect()
        except Exception:
            pass
        if cli.connect():
            data = cli.execute_command(
                "device_info", timeout=20.0, slow_command=True
            )

    allure.attach(data, name="CLI device_info", attachment_type=allure.attachment_type.TEXT)
    return data
