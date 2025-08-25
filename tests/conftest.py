import pytest
import asyncio
import requests
import telnetlib3
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
    """CLI fixture for telnet-based BSB commands"""
    
    def __init__(self, host: str = "busybar.local", port: int = 23):
        self.host = host
        self.port = port
        self.reader: Optional[telnetlib3.TelnetReader] = None
        self.writer: Optional[telnetlib3.TelnetWriter] = None
        self.connected = False
        self.logger = get_cli_logger()
    
    async def connect(self, timeout: float = 5.0) -> bool:  # Reduced from 10.0s to 5.0s
        """Connect to the CLI via telnet"""
        try:
            self.logger.info(f"Connecting to CLI at {self.host}:{self.port}")
            
            self.reader, self.writer = await asyncio.wait_for(
                telnetlib3.open_connection(self.host, self.port),
                timeout=timeout
            )
            self.connected = True
            
            # Wait for initial prompt/welcome message
            await asyncio.sleep(0.1)  # Reduced from 1s to 100ms
            welcome_msg = await self._read_until_prompt()
            
            self.logger.info(f"CLI connected successfully, welcome message length: {len(welcome_msg)}")
            self.logger.debug(f"Welcome message (raw): {repr(welcome_msg)}")
            allure.attach(welcome_msg, "CLI Connection Welcome", allure.attachment_type.TEXT)
            return True
            
        except Exception as e:
            self.logger.error(f"CLI connection failed to {self.host}:{self.port} - {str(e)}")
            allure.attach(str(e), "CLI Connection Error", allure.attachment_type.TEXT)
            return False
    
    async def disconnect(self):
        """Disconnect from CLI"""
        if self.writer and not self.writer.is_closing():
            self.logger.info("Disconnecting from CLI")
            self.writer.close()
            # telnetlib3's TelnetWriter doesn't have wait_closed(), just close() is sufficient
        self.connected = False
    
    async def execute_command(self, command: str, timeout: float = 5.0) -> str:
        """Execute a CLI command and return output"""
        if not self.connected:
            self.logger.error("Attempted to execute command on disconnected CLI")
            raise RuntimeError("CLI not connected")
        
        start_time = time.time()
        
        with allure.step(f"Execute CLI command: {command}"):
            self.logger.debug(f"Executing CLI command: {command}")
            
            # Send command with CRLF (telnet standard)
            command_to_send = f"{command}\r\n"
            self.logger.debug(f"Sending command (raw): {repr(command_to_send)}")
            self.writer.write(command_to_send)
            await self.writer.drain()
            
            # Read response
            response = await asyncio.wait_for(
                self._read_until_prompt(),
                timeout=timeout
            )
            
            duration = time.time() - start_time
            
            # Log the command execution
            self.logger.debug(f"Received response (raw): {repr(response)}")
            
            # Clean up the response by removing the command echo and prompt
            cleaned_response = self._clean_response(response, command)
            self.logger.debug(f"Cleaned response: {repr(cleaned_response)}")
            
            log_cli_command(command, cleaned_response, duration)
            
            allure.attach(f"Command: {command}", "CLI Command", allure.attachment_type.TEXT)
            allure.attach(cleaned_response, "CLI Response", allure.attachment_type.TEXT)
            
            return cleaned_response.strip()
    
    async def _read_until_prompt(self, prompt_indicators: list = None) -> str:
        """Read output until we hit a prompt"""
        if prompt_indicators is None:
            prompt_indicators = [">: "]  # BUSY Bar specific prompts - note the space after colon
        
        output = ""
        consecutive_timeouts = 0
        max_consecutive_timeouts = 2  # Reduced from 3 to 2
        
        while True:
            try:
                data = await asyncio.wait_for(self.reader.read(1024), timeout=0.3)  # Reduced from 1.0s to 300ms
                if not data:
                    break
                output += data
                consecutive_timeouts = 0  # Reset timeout counter on successful read
                self.logger.debug(f"Read {len(data)} chars, total: {len(output)}")
                
                # Check if we've hit a prompt - look for the specific ">:" pattern
                if any(indicator in output for indicator in prompt_indicators):
                    self.logger.debug(f"Found prompt indicator, stopping read")
                    break
                    
            except asyncio.TimeoutError:
                consecutive_timeouts += 1
                self.logger.debug(f"Timeout #{consecutive_timeouts} reading CLI output, current length: {len(output)}")
                if consecutive_timeouts >= max_consecutive_timeouts:
                    self.logger.debug(f"Max consecutive timeouts reached, stopping read")
                    break
        
        return output
    
    def _clean_response(self, response: str, command: str) -> str:
        """Clean the response by removing command echo and prompts"""
        lines = response.split('\n')
        cleaned_lines = []
        
        for line in lines:
            # Remove ANSI escape sequences
            import re
            clean_line = re.sub(r'\x1b\[[0-9;]*m', '', line)
            clean_line = clean_line.strip('\r\n ')
            
            # Skip empty lines
            if not clean_line:
                continue
                
            # Skip command echo (line that exactly matches the command at the start)
            # But don't skip commands that appear in lists (like ? in available commands)
            if clean_line == command and not any(keyword in '\n'.join(cleaned_lines) for keyword in ['Available commands:', 'Commands:', 'help']):
                continue
                
            # Skip prompt lines
            if clean_line.endswith('>:') or clean_line.endswith('> '):
                continue
                
            cleaned_lines.append(clean_line)
        
        return '\n'.join(cleaned_lines)
    
    async def check_command_exists(self, command: str) -> bool:
        """Check if a command exists in the CLI"""
        self.logger.debug(f"Checking if command exists: {command}")
        response = await self.execute_command(f"which {command}")
        exists = "not found" not in response.lower()
        self.logger.debug(f"Command existence check result for {command}: {exists}")
        return exists
    
    async def get_help(self, command: str = "") -> str:
        """Get help information for a command"""
        if command:
            self.logger.debug(f"Getting help for specific command: {command}")
            return await self.execute_command(f"{command} --help")
        else:
            self.logger.debug("Getting general help with ? command")
            # Try the ? command first as mentioned in the welcome message
            return await self.execute_command("?")


@pytest.fixture(scope="function")
def test_logger(request) -> logging.Logger:
    """Provide a test-specific logger for each test function"""
    from utils.logging_config import get_test_logger
    test_name = request.node.name
    return get_test_logger(test_name)


@pytest.fixture(scope="function")
async def cli() -> AsyncGenerator[CLIFixture, None]:
    """Async CLI fixture that provides telnet connection to BSB"""
    cli_client = CLIFixture()
    cli_logger = get_cli_logger()
    
    with allure.step("Initialize CLI connection"):
        cli_logger.info("Setting up CLI fixture")
        connected = await cli_client.connect()
        if not connected:
            cli_logger.error("CLI connection failed, skipping test")
            pytest.skip("Could not connect to CLI")
    
    try:
        yield cli_client
    finally:
        with allure.step("Cleanup CLI connection"):
            cli_logger.info("Cleaning up CLI fixture")
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
    session.headers.update({
        'User-Agent': 'BSB-AutoTest/1.0'
    })
    
    # Add response logging hook
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
    """Pytest configuration hook"""
    logger.info("Configuring pytest")
    
    # Set up Allure TestOps integration
    allure_testops_url = os.getenv("ALLURE_TESTOPS_URL")
    allure_testops_token = os.getenv("ALLURE_TESTOPS_TOKEN")
    allure_project_id = os.getenv("ALLURE_PROJECT_ID")
    
    if all([allure_testops_url, allure_testops_token, allure_project_id]):
        logger.info(f"Allure TestOps configuration found: URL={allure_testops_url}, Project ID={allure_project_id}")
        # Configure Allure TestOps
        os.environ["ALLURE_TESTOPS_URL"] = allure_testops_url
        os.environ["ALLURE_TESTOPS_TOKEN"] = allure_testops_token
        os.environ["ALLURE_PROJECT_ID"] = allure_project_id
    else:
        logger.warning("Allure TestOps configuration incomplete")
    
    # Add custom markers
    markers = [
        "cli: CLI command tests",
        "frontend: Web frontend tests", 
        "story_commands_check: Commands Check story",
        "story_ui_validation: UI validation story",
        "story_ui_interaction: UI interaction story",
        "story_interface_status: Interface status story",
        "story_mqtt: MQTT story",
        "feature_cli: Feature 6. CLI",
        "feature_web_frontend: Feature 5. Web Frontend"
    ]
    
    for marker in markers:
        config.addinivalue_line("markers", marker)
    
    logger.info("Pytest configuration complete")


def pytest_unconfigure(config):
    """Pytest cleanup hook"""


def pytest_runtest_setup(item):
    """Setup hook for each test"""
    logger.info(f"Setting up test: {item.name} in {item.fspath}")
    
    # Add automatic allure labels based on markers
    for marker in item.iter_markers():
        if marker.name.startswith("story_"):
            story = marker.name.replace("story_", "").replace("_", " ").title()
            allure.dynamic.story(story)
        elif marker.name.startswith("feature_"):
            feature = marker.name.replace("feature_", "").replace("_", " ").title()
            allure.dynamic.feature(feature)


def pytest_runtest_teardown(item, nextitem):
    """Teardown hook for each test"""
    logger.info(f"Test completed: {item.name}")


def pytest_collection_modifyitems(config, items):
    """Modify collected test items"""
    # Add default markers based on test location
    for item in items:
        if "cli" in str(item.fspath):
            item.add_marker(pytest.mark.cli)
            item.add_marker(pytest.mark.feature_cli)
        elif "frontend" in str(item.fspath):
            item.add_marker(pytest.mark.frontend)
            item.add_marker(pytest.mark.feature_web_frontend)