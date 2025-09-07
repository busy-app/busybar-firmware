import pytest
import asyncio
import requests
import telnetlib3
import telnetlib
import os
import logging
from typing import Optional, AsyncGenerator
import allure
from dotenv import load_dotenv
import time
from utils.logging_config import (
    setup_logging, get_cli_logger, get_web_logger,
    log_cli_command, log_web_request, TestLogContext
)

load_dotenv()

# Setup logging
logger = setup_logging(
    log_level=os.getenv("LOG_LEVEL", "INFO"),
    log_to_file=os.getenv("LOG_TO_FILE", "true").lower() == "true"
)


class CLIFixture:
    """Simple, fast CLI fixture for telnet-based BSB commands"""

    def __init__(self, host: str = "busybar.local", port: int = 23):
        self.host = host
        self.port = port
        self.reader: Optional[telnetlib3.TelnetReader] = None
        self.writer: Optional[telnetlib3.TelnetWriter] = None
        self.connected = False
        self.logger = get_cli_logger()
        self._in_sl_cli = False

    async def connect(self, timeout: float = 10.0) -> bool:
        """Connect to CLI via telnet"""
        try:
            self.logger.info(f"Connecting to CLI at {self.host}:{self.port}")

            # Connect to telnet server
            self.reader, self.writer = await asyncio.wait_for(
                telnetlib3.open_connection(self.host, self.port),
                timeout=timeout
            )
            
            self.logger.info(f"TCP connection established to {self.host}:{self.port}")

            # Read welcome message until we see the prompt
            welcome_msg = await self._read_until_prompt(initial_connect=True)
            self.logger.info(f"CLI connected, welcome length: {len(welcome_msg)}")
            self.logger.debug(f"Welcome message last 100 chars: {repr(welcome_msg[-100:])}")

            # Mark as connected
            self.connected = True
            allure.attach(welcome_msg, "CLI Welcome", allure.attachment_type.TEXT)
            return True

        except Exception as e:
            self.logger.error(f"Failed to connect to {self.host}:{self.port} - {type(e).__name__}: {e}")
            # Clean up
            self.reader = None
            self.writer = None
            self.connected = False
            allure.attach(str(e), "CLI Connection Error", allure.attachment_type.TEXT)
            return False

    async def disconnect(self):
        """Disconnect from CLI"""
        if self.writer and not self.writer.is_closing():
            self.logger.info("Disconnecting from CLI")
            # Exit 917 mode if we're in it
            if self._in_sl_cli:
                try:
                    await self.execute_command("exit")
                except:
                    pass
            try:
                self.writer.close()
                await self.writer.wait_closed()
            except:
                pass
        self.connected = False

    async def reset_state(self):
        """Reset CLI state between tests - clear buffers and ensure prompt"""
        if not self.connected:
            return
        
        self.logger.debug("Reset state called - doing nothing to avoid connection issues")
        # Temporarily disabled to debug connection issues
        pass

    async def execute_command(self, command: str, timeout: float = 5.0, slow_command: bool = False) -> str:
        """Execute a CLI command and return cleaned output"""
        if not self.connected:
            raise RuntimeError("CLI not connected")

        if slow_command:
            timeout = 10.0

        start_time = time.time()

        with allure.step(f"Execute CLI command: {command}"):
            self.logger.debug(f"Executing: '{command}' (slow_command={slow_command})")
            
            # Send command
            cmd_str = f"{command}\r\n"
            self.logger.debug(f"Sending: {repr(cmd_str)}")
            self.writer.write(cmd_str)
            await self.writer.drain()

            # Wait a bit for the command to be processed
            await asyncio.sleep(0.5)
            
            # Read response - simple approach that works
            response = ""
            empty_reads = 0
            max_empty_reads = 3
            
            while empty_reads < max_empty_reads:
                try:
                    # Read available data
                    data = await asyncio.wait_for(self.reader.read(4096), timeout=0.5)
                    if data:
                        response += data
                        self.logger.debug(f"Read {len(data)} bytes, total: {len(response)}")
                        empty_reads = 0  # Reset counter when we get data
                        
                        # Check if we have a complete response (ends with prompt)
                        if response.endswith(">: ") or response.endswith("917>: "):
                            self.logger.debug("Found prompt, response complete")
                            break
                    else:
                        empty_reads += 1
                        self.logger.debug(f"Empty read #{empty_reads}")
                except asyncio.TimeoutError:
                    empty_reads += 1
                    self.logger.debug(f"Read timeout #{empty_reads}")
                    # If we have data and hit timeout, we're probably done
                    if response:
                        break
                        
            self.logger.debug(f"Response complete: {len(response)} bytes")
            
            duration = time.time() - start_time

            # Clean the response
            cleaned = self._clean_response(response, command)
            if cleaned is None:
                cleaned = ""

            self.logger.debug(f"Command '{command}' took {duration:.2f}s")
            self.logger.debug(f"Raw response preview: {repr(response[:100])}...")
            self.logger.debug(f"Cleaned response preview: {repr(cleaned[:100])}...")

            # Update 917 CLI state
            if command == "sl_cli" and "Welcome to BUSY Bar 917" in response:
                self._in_sl_cli = True
            elif command == "exit" and self._in_sl_cli:
                self._in_sl_cli = False

            log_cli_command(command, cleaned, duration)
            allure.attach(f"Command: {command}", "CLI Command", allure.attachment_type.TEXT)
            allure.attach(cleaned or "No response", "CLI Response", allure.attachment_type.TEXT)

            return cleaned.strip()

    async def _read_until_prompt(self, initial_connect: bool = False, timeout: float = 3.0) -> str:
        """
        Read data until we see a prompt

        Args:
            initial_connect: True if this is the initial connection, False for command execution
            timeout: Maximum time to wait for response
        """
        output = ""
        start_time = time.time()

        # Different strategies for initial connect vs command execution
        if initial_connect:
            # For initial connection, we need to read the welcome message
            read_timeout = 0.5
            max_empty_reads = 5
        else:
            # For command execution, allow more time for response
            # Increased from 0.2 to 0.3 to match the wait time after sending command
            read_timeout = 0.3
            max_empty_reads = 15

        empty_read_count = 0
        total_reads = 0

        while (time.time() - start_time) < timeout:
            total_reads += 1
            try:
                # Try to read available data
                remaining_time = timeout - (time.time() - start_time)
                if remaining_time <= 0:
                    break
                    
                # Debug: Check reader state
                if not initial_connect and total_reads == 1:
                    self.logger.debug(f"Reader type: {type(self.reader)}, at_eof: {self.reader.at_eof()}")
                    
                data = await asyncio.wait_for(
                    self.reader.read(4096), 
                    timeout=min(read_timeout, remaining_time)
                )
                
                if not data:
                    empty_read_count += 1
                    self.logger.debug(f"Empty read #{empty_read_count}/{max_empty_reads}, total_reads: {total_reads}")
                    if empty_read_count >= max_empty_reads:
                        self.logger.debug(
                            f"Breaking after {empty_read_count} empty reads, output length: {len(output)}")
                        break
                    continue

                # Reset empty read counter when we get data
                empty_read_count = 0
                output += data
                self.logger.debug(f"Read {len(data)} chars, total output: {len(output)} chars")

                # Log data for debugging
                self.logger.debug(f"Data repr: {repr(data[:200])}")
                
                # Log a preview of what we just read
                data_preview = repr(data[:100])
                self.logger.debug(f"Data preview: {data_preview}")

                # Check for prompt patterns
                prompt_found = False
                if self._in_sl_cli:
                    # In 917 CLI mode, look for 917>:
                    if "917>: " in output:
                        prompt_found = True
                        self.logger.debug("Found 917 CLI prompt")
                else:
                    # In main CLI mode, look for >:
                    if ">: " in output:
                        prompt_found = True
                        self.logger.debug("Found main CLI prompt")

                if prompt_found:
                    if initial_connect and len(output) < 100:
                        # For initial connection, wait for substantial welcome message
                        self.logger.debug("Initial connect with small output, continuing...")
                        continue
                    else:
                        # For command execution or sufficient initial data, break
                        self.logger.debug(f"Prompt found, breaking with {len(output)} chars")
                        break

            except asyncio.TimeoutError:
                empty_read_count += 1
                self.logger.debug(
                    f"Timeout after {read_timeout}s, output: {len(output)} chars, empty_reads: {empty_read_count}, total_reads: {total_reads}")

                # Check if we have a complete response
                if output and (">: " in output or "917>: " in output):
                    self.logger.debug("Found prompt in output after timeout, breaking")
                    break

                # For command execution, if we have substantial content, probably done
                if not initial_connect and len(output) > 10:
                    self.logger.debug("Substantial content found, breaking")
                    break

                # If we've been waiting too long without any data, break
                if empty_read_count >= max_empty_reads:
                    self.logger.debug(f"Max empty reads ({max_empty_reads}) reached, breaking")
                    break

        self.logger.debug(f"_read_until_prompt returning {len(output)} chars")
        if output:
            self.logger.debug(f"Output preview: {repr(output[:200])}")
        return output

    def _clean_response(self, response: str, command: str) -> str:
        """Clean response by removing command echo and prompts"""
        if not response:
            return ""

        # Remove ANSI escape sequences
        import re
        cleaned = re.sub(r'\x1b\[[0-9;]*[A-Za-z]', '', response)
        cleaned = re.sub(r'\x1b\([A-Z]', '', cleaned)
        cleaned = re.sub(r'\x1b[>=]', '', cleaned)
        
        # Split into lines
        lines = cleaned.split('\n')
        cleaned_lines = []
        command_seen = False

        for line in lines:
            line = line.strip('\r').strip()
            
            if not line:
                continue

            # Skip the command echo (only first occurrence)
            if not command_seen and line == command:
                command_seen = True
                continue

            # Skip prompt-only lines
            if line in ['>:', '917>:', '>', '917>'] or line.endswith('>:'):
                continue
                
            # Remove prompt if it's at the end of a line with content
            if line.endswith('>:'):
                line = line[:-2].strip()
            
            if line:  # Only add non-empty lines
                cleaned_lines.append(line)

        return '\n'.join(cleaned_lines)

    # 917 CLI helper methods
    async def enter_sl_cli(self) -> str:
        """Enter 917 CLI mode"""
        return await self.execute_command("sl_cli", slow_command=True)

    async def exit_sl_cli(self) -> str:
        """Exit 917 CLI mode"""
        if not self._in_sl_cli:
            raise RuntimeError("Not in 917 CLI mode")
        return await self.execute_command("exit")

    async def execute_917_command(self, command: str) -> str:
        """Execute command in 917 CLI mode"""
        if not self._in_sl_cli:
            raise RuntimeError("Not in 917 CLI mode")
        return await self.execute_command(command, slow_command=True)


@pytest.fixture(scope="function")
def test_logger(request) -> logging.Logger:
    """Test-specific logger"""
    from utils.logging_config import get_test_logger
    return get_test_logger(request.node.name)


@pytest.fixture(scope="session")
async def cli_session() -> AsyncGenerator[CLIFixture, None]:
    """Session-scoped CLI fixture - one connection for all tests"""
    cli_client = CLIFixture()
    cli_logger = get_cli_logger()

    cli_logger.info("Setting up session CLI fixture")

    connected = await cli_client.connect()
    if not connected:
        cli_logger.error("CLI connection failed")
        pytest.skip("Could not connect to CLI")

    try:
        yield cli_client
    finally:
        cli_logger.info("Cleaning up session CLI fixture")
        await cli_client.disconnect()


@pytest.fixture(scope="function", autouse=True)
async def reset_cli_session(request, cli_session):
    """Auto-reset CLI session state between tests when using session fixture"""
    # Only apply to tests using cli_session
    if 'cli_session' in request.fixturenames:
        cli_logger = get_cli_logger()
        cli_logger.debug(f"Resetting CLI for test: {request.node.name}")
        await cli_session.reset_state()
    yield


@pytest.fixture(scope="function")
async def cli(request) -> AsyncGenerator[CLIFixture, None]:
    """Function-scoped CLI fixture - fresh connection per test"""
    cli_client = CLIFixture()
    cli_logger = get_cli_logger()

    cli_logger.info(f"Setting up CLI for {request.node.name}")

    connected = await cli_client.connect()
    if not connected:
        cli_logger.error("CLI connection failed")
        pytest.skip("Could not connect to CLI")

    try:
        yield cli_client
    finally:
        cli_logger.info("Cleaning up CLI fixture")
        await cli_client.disconnect()


@pytest.fixture(scope="function")
async def cli_fresh(request) -> AsyncGenerator[CLIFixture, None]:
    """Fresh CLI connection fixture"""
    cli_client = CLIFixture()
    cli_logger = get_cli_logger()

    cli_logger.info(f"Setting up fresh CLI for {request.node.name}")

    connected = await cli_client.connect()
    if not connected:
        cli_logger.error("Fresh CLI connection failed")
        pytest.skip("Could not connect to CLI")

    try:
        yield cli_client
    finally:
        cli_logger.info("Cleaning up fresh CLI fixture")
        await cli_client.disconnect()


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
    session.headers.update({'User-Agent': 'BSB-AutoTest/1.0'})

    # Add response logging
    original_request = session.request

    def logged_request(*args, **kwargs):
        start_time = time.time()
        response = original_request(*args, **kwargs)
        duration = time.time() - start_time
        log_web_request(
            method=args[0] if args else kwargs.get('method', 'GET'),
            url=args[1] if len(args) > 1 else kwargs.get('url', ''),
            status_code=response.status_code,
            duration=duration
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
        logger.info(f"Allure TestOps: {allure_testops_url}, Project: {allure_project_id}")
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
        "connection_test: Fresh connection tests"
    ]

    for marker in markers:
        config.addinivalue_line("markers", marker)

    logger.info("Pytest configuration complete")


# New SimpleCLIConnection using standard telnetlib
class SimpleCLIConnection:
    """Simple CLI connection using standard telnetlib"""
    
    def __init__(self, host: str = "busybar.local", port: int = 23):
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
            welcome_str = welcome.decode('utf-8', errors='ignore')
            
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
    
    def execute_command(self, command: str, timeout: float = 5.0, slow_command: bool = False) -> str:
        """Execute a command and return response"""
        if not self.connected or not self.tn:
            raise RuntimeError("Not connected")
        
        # Increase timeout for slow commands (only if custom timeout not provided)
        if slow_command and timeout == 5.0:  # Default timeout
            timeout = 15.0
        
        try:
            self.logger.debug(f"Executing command: {repr(command)}")
            
            # Send command
            cmd_bytes = f"{command}\r\n".encode('utf-8')
            self.tn.write(cmd_bytes)
            
            # Read response until we see the prompt again
            # Use appropriate prompt based on CLI mode
            prompt = b"917>: " if self._in_sl_cli else b">: "
            
            # Special handling for device_info which has delayed response
            if command.strip() == "device_info":
                # First read with shorter timeout to get immediate response
                response = self.tn.read_until(prompt, timeout=5.0)
                response_str = response.decode('utf-8', errors='ignore')
                
                # If we only got u5_* fields, wait for sl_* fields
                if "u5_firmware" in response_str and "sl_firmware" not in response_str:
                    self.logger.debug("Got u5_ fields, waiting for sl_ fields...")
                    # Wait a bit more for the delayed sl_* response
                    additional_response = self.tn.read_until(prompt, timeout=timeout-5.0)
                    additional_str = additional_response.decode('utf-8', errors='ignore')
                    response_str += additional_str
                    self.logger.debug(f"Added {len(additional_str)} chars from delayed response")
            else:
                response = self.tn.read_until(prompt, timeout=timeout)
                response_str = response.decode('utf-8', errors='ignore')
            
            self.logger.debug(f"Raw response ({len(response_str)} chars): {repr(response_str[:100])}")
            
            # Clean response - remove command echo and prompt
            cleaned = self._clean_response(response_str, command)
            
            # Update 917 CLI state tracking
            if command == "sl_cli":
                # Remove ANSI codes before checking for welcome message
                import re
                clean_response = re.sub(r'\x1b\[[0-9;]*[A-Za-z]', '', response_str)
                clean_response = re.sub(r'\x1b\([A-Z]', '', clean_response)
                clean_response = re.sub(r'\x1b[>=]', '', clean_response)
                
                if "Welcome to BUSY Bar 917" in clean_response or "917 Command Line Interface" in clean_response:
                    self._in_sl_cli = True
                    self.logger.debug(f"Entered 917 CLI mode, response contains welcome message")
                else:
                    self.logger.warning(f"sl_cli executed but no welcome message found. Clean response: {repr(clean_response[:200])}")
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
        cleaned = re.sub(r'\x1b\[[0-9;]*[A-Za-z]', '', response)
        cleaned = re.sub(r'\x1b\([A-Z]', '', cleaned)
        cleaned = re.sub(r'\x1b[>=]', '', cleaned)
        
        # Split into lines
        lines = cleaned.split('\n')
        cleaned_lines = []
        command_seen = False
        
        for line in lines:
            line = line.strip('\r').strip()
            
            if not line:
                continue
            
            # Skip the command echo (only first occurrence)
            if not command_seen and line == command:
                command_seen = True
                continue
            
            # Skip prompt-only lines
            if line in ['>:', '917>:', '>', '917>', 'busybar>:', 'busybar>'] or line.endswith('>:'):
                continue
                
            # Remove prompt if it's at the end of a line with content
            if line.endswith('>:'):
                line = line[:-2].strip()
            
            if line:  # Only add non-empty lines
                cleaned_lines.append(line)
        
        return '\n'.join(cleaned_lines)
    
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
def simple_cli_session():
    """Session-scoped CLI fixture using simple telnet connection - shared across all tests"""
    cli = SimpleCLIConnection()
    cli_logger = get_cli_logger()
    
    cli_logger.info("Setting up session CLI fixture (SimpleCLI)")
    
    # Connect
    if not cli.connect():
        cli_logger.error("SimpleCLI connection failed")
        pytest.skip("Could not connect to CLI")
    
    try:
        yield cli
    finally:
        cli_logger.info("Cleaning up session CLI fixture (SimpleCLI)")
        cli.disconnect()


@pytest.fixture(scope="function")
def simple_cli():
    """Function-scoped CLI fixture using simple telnet connection - fresh connection per test"""
    cli = SimpleCLIConnection()
    cli_logger = get_cli_logger()
    
    cli_logger.info("Setting up function CLI fixture (SimpleCLI)")
    
    # Connect
    if not cli.connect():
        cli_logger.error("SimpleCLI function connection failed")
        pytest.skip("Could not connect to CLI")
    
    try:
        yield cli
    finally:
        cli_logger.info("Cleaning up function CLI fixture (SimpleCLI)")
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