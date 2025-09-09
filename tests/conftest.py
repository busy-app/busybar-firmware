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

load_dotenv()

# Setup logging
logger = setup_logging(
    log_level=os.getenv("LOG_LEVEL", "INFO"),
    log_to_file=os.getenv("LOG_TO_FILE", "true").lower() == "true",
)


# CLIFixture class removed - now using SimpleCLIConnection for better reliability


@pytest.fixture(scope="function")
def test_logger(request) -> logging.Logger:
    """Test-specific logger"""
    from utils.logging_config import get_test_logger

    return get_test_logger(request.node.name)


# Old async CLI fixtures removed - now using synchronous SimpleCLIConnection for better reliability


@pytest.fixture(scope="session")
def web_base_url() -> str:
    """Base URL for web frontend tests"""
    return os.getenv("WEB_BASE_URL", "http://10.0.4.20/")


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


def pytest_configure(config):
    """Pytest configuration"""
    logger.info("Configuring pytest")

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

    #TODO: fix env load here
    def __init__(self, host: str = "10.0.4.20", port: int = 23):
        self.host = host
        self.port = port
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

            # Update 917 CLI state tracking
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

            # Log the command execution
            duration = timeout  # Approximate since we don't track exact timing
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

    # Add allure labels from markers
    for marker in item.iter_markers():
        if marker.name.startswith("story_"):
            story = marker.name.replace("story_", "").replace("_", " ").title()
            allure.dynamic.story(story)
        elif marker.name.startswith("feature_"):
            feature = marker.name.replace("feature_", "").replace("_", " ").title()
            allure.dynamic.feature(feature)


def pytest_runtest_teardown(item, nextitem):
    """Test teardown"""
    logger.info(f"Test completed: {item.name}")


def pytest_collection_modifyitems(config, items):
    """Modify test items"""
    for item in items:
        if "cli" in str(item.fspath):
            item.add_marker(pytest.mark.cli)
            item.add_marker(pytest.mark.feature_cli)
        elif "frontend" in str(item.fspath):
            item.add_marker(pytest.mark.frontend)
            item.add_marker(pytest.mark.feature_web_frontend)
