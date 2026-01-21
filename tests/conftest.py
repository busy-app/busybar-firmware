import logging
import os
import telnetlib  # TODO: Replace with alternative before Python 3.13 (deprecated)
import time
from typing import Optional

import allure
import pytest
import requests
from dotenv import load_dotenv

from utils.logging_config import (TestLogContext, get_cli_logger,
                                  get_web_logger, log_cli_command,
                                  log_web_request, setup_logging)
from utils.crash_detector import CrashDetector
from utils.device_flasher import DeviceFlasher

# API client imports
from clients.api import (
    SystemAPI, WifiAPI, StorageAPI, AssetsAPI, AccountAPI,
    BleAPI, SettingsAPI, InputAPI, StreamingAPI, UpdateAPI
)
from config.config import Config

load_dotenv()


# Validate critical environment variables
def validate_environment():
    """Validate that critical environment variables are available"""
    required_vars = {
        "CLI_HOST": os.getenv("CLI_HOST"),
        "CLI_PORT": os.getenv("CLI_PORT"),
        "WEB_BASE_URL": os.getenv("WEB_BASE_URL"),
    }

    missing_vars = [var for var, value in required_vars.items() if not value]
    if missing_vars:
        print(f"Warning: Missing environment variables: {', '.join(missing_vars)}")
        print("Using default values. Check your .env file if tests fail.")

    print(f"Test Configuration:")
    print(f"  CLI_HOST: {os.getenv('CLI_HOST', 'Not set (will use default)')}")
    print(f"  CLI_PORT: {os.getenv('CLI_PORT', 'Not set (will use default)')}")
    print(f"  WEB_BASE_URL: {os.getenv('WEB_BASE_URL', 'Not set (will use default)')}")
    print(f"  LOG_LEVEL: {os.getenv('LOG_LEVEL', 'INFO')}")


# Validate environment on import
validate_environment()

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
def web_session() -> requests.Session:
    """HTTP session for web frontend tests"""
    logger = get_web_logger()
    logger.info("Creating web session")

    session = requests.Session()
    session.headers.update({"User-Agent": "BSB-AutoTest/1.0"})

    # Add response logging
    original_request = session.request

    def logged_request(*args, **kwargs):
        start_time = time.time()
        response = original_request(*args, **kwargs)
        duration = time.time() - start_time
        log_web_request(
            method=args[0] if args else kwargs.get("method", "GET"),
            url=args[1] if len(args) > 1 else kwargs.get("url", ""),
            status_code=response.status_code,
            duration=duration,
        )
        return response

    session.request = logged_request
    return session


@pytest.fixture
def api_session(web_session) -> requests.Session:
    """API session with proper headers for API testing"""
    # Add API-specific headers - only Accept, not Content-Type
    # Content-Type will be set appropriately per request
    web_session.headers.update(
        {
            "Accept": "application/json",
        }
    )
    return web_session


@pytest.fixture
def api_auth_session(web_session) -> requests.Session:
    """API session with authentication headers"""
    # TODO: Add X-API-Token header when authentication is required
    # Content-Type will be set appropriately per request
    web_session.headers.update(
        {
            "Accept": "application/json",
            # 'X-API-Token': 'test-token'  # Uncomment when auth is implemented
        }
    )
    return web_session


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

    # Add markers
    markers = [
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
    ]

    for marker in markers:
        config.addinivalue_line("markers", marker)

    logger.info("Pytest configuration complete")


# New SimpleCLIConnection using standard telnetlib
class SimpleCLIConnection:
    """Simple CLI connection using standard telnetlib"""

    def __init__(self, host: str = None, port: int = None):
        self.host = host or os.getenv("CLI_HOST", "10.0.4.20")
        self.port = int(port or os.getenv("CLI_PORT", "23"))
        self.tn: Optional[telnetlib.Telnet] = None
        self.connected = False
        self.logger = get_cli_logger()
        self._in_sl_cli = False  # For 917 CLI mode tracking

    def connect(self, timeout: float = 10.0) -> bool:
        """Connect to CLI via telnet"""
        try:
            self.logger.info(f"Connecting to {self.host}:{self.port}")

            # Create telnet connection
            self.tn = telnetlib.Telnet(self.host, self.port, timeout=timeout)

            # Read welcome message until we see prompt
            welcome = self.tn.read_until(b">: ", timeout=5.0)
            welcome_str = welcome.decode("utf-8", errors="ignore")

            self.logger.info(f"Connected! Welcome message: {len(welcome_str)} chars")
            self.logger.debug(f"Welcome (last 50 chars): {repr(welcome_str[-50:])}")

            self.connected = True
            return True

        except Exception as e:
            self.logger.error(f"Connection failed: {type(e).__name__}: {e}")
            self.connected = False
            if self.tn:
                try:
                    self.tn.close()
                except:
                    pass
                self.tn = None
            return False

    def execute_command(
        self, command: str, timeout: float = 5.0, slow_command: bool = False
    ) -> str:
        """Execute a command and return response"""
        if not self.connected or not self.tn:
            raise RuntimeError("Not connected")

        # Increase timeout for slow commands (only if custom timeout not provided)
        if slow_command and timeout == 5.0:  # Default timeout
            timeout = 15.0

        try:
            self.logger.debug(f"Executing command: {repr(command)}")

            # Send command
            cmd_bytes = f"{command}\r\n".encode("utf-8")
            self.tn.write(cmd_bytes)

            # Read response until we see the prompt again
            # Use appropriate prompt based on CLI mode
            prompt = b"917>: " if self._in_sl_cli else b">: "

            # Special handling for device_info which has delayed response
            if command.strip() == "device_info":
                # First read with shorter timeout to get immediate response
                response = self.tn.read_until(prompt, timeout=5.0)
                response_str = response.decode("utf-8", errors="ignore")

                # If we only got u5_* fields, wait for sl_* fields
                if "u5_firmware" in response_str and "sl_firmware" not in response_str:
                    self.logger.debug("Got u5_ fields, waiting for sl_ fields...")
                    # Wait a bit more for the delayed sl_* response
                    additional_response = self.tn.read_until(
                        prompt, timeout=timeout - 5.0
                    )
                    additional_str = additional_response.decode(
                        "utf-8", errors="ignore"
                    )
                    response_str += additional_str
                    self.logger.debug(
                        f"Added {len(additional_str)} chars from delayed response"
                    )
            else:
                response = self.tn.read_until(prompt, timeout=timeout)
                response_str = response.decode("utf-8", errors="ignore")

            self.logger.debug(
                f"Raw response ({len(response_str)} chars): {repr(response_str[:100])}"
            )

            # Clean response - remove command echo and prompt
            cleaned = self._clean_response(response_str, command)

            if command == "sl_cli":
                # Remove ANSI codes before checking for welcome message
                import re

                clean_response = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", response_str)
                clean_response = re.sub(r"\x1b\([A-Z]", "", clean_response)
                clean_response = re.sub(r"\x1b[>=]", "", clean_response)

                if (
                    "Welcome to BUSY Bar 917" in clean_response
                    or "917 Command Line Interface" in clean_response
                ):
                    self._in_sl_cli = True
                    self.logger.debug(
                        f"Entered 917 CLI mode, response contains welcome message"
                    )
                else:
                    self.logger.warning(
                        f"sl_cli executed but no welcome message found. Clean response: {repr(clean_response[:200])}"
                    )
            elif command == "exit" and self._in_sl_cli:
                self._in_sl_cli = False
                self.logger.debug(f"Exited 917 CLI mode")

            duration = timeout
            log_cli_command(command, cleaned, duration)

            return cleaned

        except Exception as e:
            self.logger.error(f"Command execution failed: {type(e).__name__}: {e}")
            return ""

    def _clean_response(self, response: str, command: str) -> str:
        """Clean response by removing command echo and prompts"""
        if not response:
            return ""

        # Remove ANSI escape sequences
        import re

        cleaned = re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", response)
        cleaned = re.sub(r"\x1b\([A-Z]", "", cleaned)
        cleaned = re.sub(r"\x1b[>=]", "", cleaned)

        # Split into lines
        lines = cleaned.split("\n")
        cleaned_lines = []
        command_seen = False

        for line in lines:
            line = line.strip("\r").strip()

            if not line:
                continue

            # Skip the command echo (only first occurrence)
            if not command_seen and line == command:
                command_seen = True
                continue

            # Skip prompt-only lines
            if line in [
                ">:",
                "917>:",
                ">",
                "917>",
                "busybar>:",
                "busybar>",
            ] or line.endswith(">:"):
                continue

            # Remove prompt if it's at the end of a line with content
            if line.endswith(">:"):
                line = line[:-2].strip()

            if line:  # Only add non-empty lines
                cleaned_lines.append(line)

        return "\n".join(cleaned_lines)

    # 917 CLI helper methods
    def enter_sl_cli(self) -> str:
        """Enter 917 CLI mode"""
        return self.execute_command("sl_cli", slow_command=True)

    def exit_sl_cli(self) -> str:
        """Exit 917 CLI mode"""
        if not self._in_sl_cli:
            raise RuntimeError("Not in 917 CLI mode")
        return self.execute_command("exit")

    def execute_917_command(self, command: str) -> str:
        """Execute command in 917 CLI mode"""
        if not self._in_sl_cli:
            raise RuntimeError("Not in 917 CLI mode")
        return self.execute_command(command, slow_command=True)

    def disconnect(self):
        """Disconnect from CLI"""
        if self.tn:
            self.logger.info("Disconnecting from CLI")
            try:
                # Exit 917 mode if we're in it
                if self._in_sl_cli:
                    try:
                        self.execute_command("exit")
                    except:
                        pass
                self.tn.close()
            except:
                pass
            self.tn = None
        self.connected = False
        self._in_sl_cli = False


# New fixtures using SimpleCLIConnection
@pytest.fixture(scope="session")
def persistent_cli_connection():
    """Session-scoped CLI fixture - maintains single connection across all CLI tests for better performance"""
    cli = SimpleCLIConnection()
    cli_logger = get_cli_logger()

    cli_logger.info("Setting up persistent CLI connection (session-scoped)")

    # Connect
    if not cli.connect():
        cli_logger.error("Persistent CLI connection failed")
        pytest.skip("Could not connect to CLI")

    try:
        yield cli
    finally:
        cli_logger.info("Cleaning up persistent CLI connection")
        cli.disconnect()


@pytest.fixture(scope="function")
def fresh_cli_connection():
    """Function-scoped CLI fixture - creates fresh connection per test (use for connection reliability tests)"""
    cli = SimpleCLIConnection()
    cli_logger = get_cli_logger()

    cli_logger.info("Setting up fresh CLI connection (function-scoped)")

    # Connect
    if not cli.connect():
        cli_logger.error("Fresh CLI connection failed")
        pytest.skip("Could not connect to CLI")

    try:
        yield cli
    finally:
        cli_logger.info("Cleaning up fresh CLI connection")
        cli.disconnect()


def pytest_unconfigure(config):
    """Pytest cleanup"""
    pass


def pytest_runtest_setup(item):
    """Test setup"""
    logger.info(f"Setting up: {item.name}")

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


def pytest_runtest_teardown(item, nextitem):
    """Test teardown"""
    logger.info(f"Test completed: {item.name}")


@pytest.fixture(autouse=True)
def device_health_monitor(request, device_flasher):
    """
    Auto-use fixture that monitors device health and recovers from failures.

    Before test:
    - Check if device is reachable
    - If not reachable, reset and wait for recovery

    After test:
    - Check for crash flag
    - Check if test failed with ConnectionError
    - If either, reset device for next test
    """
    # Pre-test: ensure device is reachable
    if not device_flasher.check_device_available():
        logger.warning("Device unreachable before test, resetting...")
        with allure.step("Resetting unreachable device before test"):
            if not device_flasher.reset_and_wait():
                pytest.fail("Device not recoverable - check hardware connection")

    # Capture crash detector state
    detector = CrashDetector()
    detector.capture_initial_state()

    yield detector

    # Post-test: check for crash or connection failure
    crash_info = detector.check_for_crash(flasher=device_flasher)
    if crash_info:
        request.node._crash_info = crash_info

    # Check if test failed with connection error
    if hasattr(request.node, "_connection_error"):
        logger.warning("Test failed with connection error, resetting device...")
        with allure.step("Resetting device after connection error"):
            device_flasher.reset_and_wait()


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """
    Hook to:
    1. Detect connection errors and mark for device reset
    2. Mark test as failed if device crash was detected
    """
    outcome = yield
    report = outcome.get_result()

    # Check for connection errors during test execution
    if report.when == "call" and report.failed:
        if call.excinfo is not None:
            exc_type = call.excinfo.type
            exc_value = str(call.excinfo.value)
            # Check for requests connection errors
            if "ConnectionError" in exc_type.__name__ or "ConnectTimeout" in exc_type.__name__:
                item._connection_error = True
                logger.warning(f"Connection error detected in test: {exc_value[:100]}")
            # Also check the exception message for connection issues
            elif "Connection" in exc_value or "No route to host" in exc_value:
                item._connection_error = True
                logger.warning(f"Connection issue detected in test: {exc_value[:100]}")

    # Check for crash info stored by device_health_monitor fixture
    crash_info = getattr(item, "_crash_info", None)

    if crash_info and report.when == "call":
        # If test passed but crash occurred, mark as failed
        if report.outcome == "passed":
            report.outcome = "failed"
            crash_msg = (
                f"DEVICE CRASH DETECTED!\n"
                f"Processor: {crash_info.processor}\n"
                f"Crash line: {crash_info.crash_line}\n"
                f"Timestamp: {crash_info.timestamp}\n"
                f"See allure report for full crash trace."
            )
            report.longrepr = crash_msg

@pytest.fixture
def api_factory(api_session, web_base_url):
    """Factory for creating API client instances."""
    def _create(api_class):
        return api_class(api_session, web_base_url)
    return _create


@pytest.fixture
def system_api(api_factory):
    """System API client fixture."""
    return api_factory(SystemAPI)


@pytest.fixture
def wifi_api(api_factory):
    """WiFi API client fixture."""
    return api_factory(WifiAPI)


@pytest.fixture
def storage_api(api_factory):
    """Storage API client fixture."""
    return api_factory(StorageAPI)


@pytest.fixture
def assets_api(api_factory):
    """Assets/Display/Audio API client fixture."""
    return api_factory(AssetsAPI)


@pytest.fixture
def account_api(api_factory):
    """Account API client fixture."""
    return api_factory(AccountAPI)


@pytest.fixture
def ble_api(api_factory):
    """BLE API client fixture."""
    return api_factory(BleAPI)


@pytest.fixture
def settings_api(api_factory):
    """Settings API client fixture."""
    return api_factory(SettingsAPI)


@pytest.fixture
def input_api(api_factory):
    """Input API client fixture."""
    return api_factory(InputAPI)


@pytest.fixture
def streaming_api(api_factory):
    """Streaming API client fixture."""
    return api_factory(StreamingAPI)


@pytest.fixture
def update_api(api_factory):
    """Update API client fixture."""
    return api_factory(UpdateAPI)


@pytest.fixture
def cli_device_info(persistent_cli_connection):
    """Fetch and attach CLI device_info for cross-verification."""
    data = persistent_cli_connection.execute_command(
        "device_info", timeout=20.0, slow_command=True
    )
    allure.attach(data, name="CLI device_info", attachment_type=allure.attachment_type.TEXT)
    return data
