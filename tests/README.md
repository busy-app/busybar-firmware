# BSB Test Automation

Automated test suite for BSB (BUSY Bar U5) CLI and Web Frontend using pytest with Allure TestOps integration.

## Quick Start

### Prerequisites
- Python 3.9+ 
- Poetry (install from https://python-poetry.org/docs/#installation)
- Access to BSB device at `busybar.local` (telnet port 23)
- Web frontend accessible at `http://10.0.4.20/`
- Allure TestOps account (optional, for result upload)

### Setup

1. **Clone and setup the project:**
```bash
git clone <your-repo-url>
cd bsb-automation-tests
poetry install
```

2. **Configure environment:**
Copy `.env.template` to `.env` and update values:
```bash
cp .env.template .env
# Edit .env with your specific settings
```

3. **Test connectivity:**
```bash
# Test CLI connection
telnet busybar.local 23

# Test web frontend
curl http://10.0.4.20/
```

## Running Tests

### Using the Cross-Platform Runner Script

The `run_tests.py` script works on Windows, macOS, and Linux:

```bash
# Run all tests with live output
python run_tests.py

# Run only CLI tests
python run_tests.py --suite cli

# Run only frontend tests  
python run_tests.py --suite frontend

# Run specific story tests
python run_tests.py --markers story_commands_check

# Generate local Allure report
python run_tests.py --report

# Upload to Allure TestOps
python run_tests.py --upload

# Quiet mode (no live output)
python run_tests.py --quiet-runner
```

### Direct Pytest Commands

```bash
# Run all tests
poetry run pytest tests/ -v

# Run with specific markers
poetry run pytest -m cli -v
poetry run pytest -m story_commands_check -v

# Generate Allure results
poetry run pytest tests/ --alluredir=allure-results
```

### Available Test Suites

- `all` - All tests (default)
- `cli` - CLI command tests only
- `frontend` - Web frontend tests only

## Project Structure

```
bsb-automation-tests/
├── tests/
│   ├── cli/
│   │   └── test_commands.py        # CLI command tests
│   └── frontend/
│       └── test_web_frontend.py    # Web frontend tests
├── utils/
│   └── logging_config.py          # Logging configuration
├── conftest.py                     # Pytest fixtures & config
├── run_tests.py                    # Cross-platform runner
├── pyproject.toml                  # Poetry dependencies
├── .env.template                   # Environment template
└── README.md                       # This file
```

## Test Architecture

### CLI Testing Features
- **Session-scoped connections**: Single telnet connection per test session for efficiency
- **917 CLI support**: Automatic handling of nested CLI (sl_cli command)
- **Fresh connection testing**: Separate fixture for connection reliability tests
- **Smart prompt detection**: Handles both main CLI (>:) and 917 CLI (917>:) prompts
- **Response cleaning**: Automatic removal of command echoes and prompts

### Web Frontend Testing
- **Core functionality tests**: Page loading, basic elements, connection status
- **Theme support**: Dark/light theme detection
- **Responsive design**: Viewport and mobile compatibility checks
- **Accessibility basics**: Alt attributes, interactive elements
- **Browser compatibility**: Header validation and encoding checks

### Test Fixtures

#### CLI Fixtures
- `cli` - Function-scoped, reuses session connection (recommended)
- `cli_session` - Session-scoped, maintains connection throughout test session
- `cli_fresh` - Function-scoped, creates fresh connection each time

#### Web Fixtures
- `web_session` - HTTP session with automatic request logging
- `web_base_url` - Configurable base URL for web tests

## Test Markers

Use pytest markers to filter and organize tests:

**By Component:**
- `cli` - CLI command tests
- `frontend` - Web frontend tests

[//]: # (**By Story:**)

[//]: # (- `story_commands_check` - Commands Check story)

[//]: # (- `story_ui_validation` - UI validation story  )

[//]: # (- `story_ui_interaction` - UI interaction story)

[//]: # (- `story_interface_status` - Interface status story)

[//]: # (- `story_mqtt` - MQTT story &#40;mostly skipped/draft&#41;)

[//]: # ()
[//]: # (**By Feature:**)

[//]: # (- `feature_cli` - Feature 6. CLI)

[//]: # (- `feature_web_frontend` - Feature 5. Web Frontend)

**Special Markers:**
- `connection_test` - Tests requiring fresh connections

## CLI Command Tests

### Basic Commands
- `?` - Help command with comprehensive command list validation
- `free` - Memory information with heap size validation
- `uptime` - System uptime with time unit parsing
- `device_info` - Device information with firmware details
- `echo` - Command echo functionality
- And 15+ more commands...

### 917 CLI Tests
- `sl_cli` - Enter 917 CLI mode with welcome message validation
- Multiple entry/exit cycles
- Command execution within 917 CLI
- Automatic state management

### UI Validation
- Welcome message verification
- Command history and aliases
- Tab completion support
- Version and build information

## Web Frontend Tests

### Core Functionality
- Page loading and basic HTML structure
- BUSY Bar branding and connection status
- Interactive elements and navigation

### UI Features
- Dark/light theme support detection
- Responsive design viewport configuration
- Device representation and screen streaming elements
- Wi-Fi and Bluetooth interface elements

### Accessibility & Compatibility
- Form validation and interactive elements
- Basic accessibility compliance checks
- Browser compatibility headers
- Touch/mouse input handling validation

## Configuration

### Environment Variables (.env)
```bash
# Device connection
CLI_HOST=busybar.local
CLI_PORT=23
WEB_BASE_URL=http://10.0.4.20/

# Allure TestOps (optional)
ALLURE_TESTOPS_URL=https://flipper.testops.cloud
ALLURE_TESTOPS_TOKEN=your-token-here
ALLURE_PROJECT_ID=232

# Test settings
LOG_LEVEL=INFO
LOG_TO_FILE=true
TEST_TIMEOUT=30
CLI_COMMAND_TIMEOUT=10
```

## Allure Integration

### Local Reports
```bash
# Generate and serve report
python run_tests.py --report

# Or manually
poetry run pytest tests/ --alluredir=allure-results
allure serve allure-results
```

### TestOps Upload
```bash
# Upload results to TestOps
python run_tests.py --upload --launch-name "My Test Run"

# Or manually with allurectl
allurectl upload allure-results --endpoint https://flipper.testops.cloud --token <token> --project-id 232
```

### Test Case Mapping
Each test includes Allure TestOps test case mapping:
```python
@allure.testcase("2047", "CLI. Command ?. [Draft]")
@pytest.mark.story_commands_check
@pytest.mark.cli
```

## Cross-Platform Support

The test suite runs on:
- **Windows** - PowerShell/CMD support
- **macOS** - Homebrew integration for Allure
- **Linux** - Native package installation

### Platform-Specific Notes

**macOS:**
```bash
# Install Allure via Homebrew
brew install allure
```

**Linux:**
```bash
# Allure auto-installed via script
# Or manual: sudo apt install allure / dnf install allure
```

**Windows:**
```bash
# Install Allure via Scoop
scoop install allure
# Or download from GitHub releases
```

## CI/CD Integration

### GitHub Actions
The repository includes workflow for self-hosted runners:

```yaml
# .github/workflows/run-tests.yml
- name: Run BSB Tests
  run: python run_tests.py --suite all --upload
```

**Required GitHub Secrets:**
- `ALLURE_TESTOPS_TOKEN` - TestOps API token

**Required GitHub Variables:**
- `ALLURE_TESTOPS_URL` - TestOps instance URL
- `ALLURE_PROJECT_ID` - TestOps project ID

### Self-hosted Runner Setup
For local BSB device access:

1. Configure runner with network access to:
   - `busybar.local:23` (telnet)
   - `10.0.4.20:80` (HTTP)

2. Install dependencies:
   - Python 3.9+
   - Poetry
   - Git

## Troubleshooting

### Connection Issues

**CLI Connection:**
```bash
# Test manually
telnet busybar.local 23
# Or check network
ping busybar.local
```

**Web Connection:**
```bash
# Test manually
curl -v http://10.0.4.20/
# Or check network
ping 10.0.4.20
```

### Common Issues

**Poetry Issues:**
```bash
# Reset environment
poetry env remove python
poetry install
```

**Allure Issues:**
```bash
# Check installation
allure --version
allurectl --version

# Regenerate report
rm -rf allure-report allure-results
python run_tests.py --report
```

**917 CLI Issues:**
- Tests automatically handle entry/exit of 917 CLI
- If stuck in 917 CLI, restart test session
- Check for proper prompt detection (>: vs 917>:)

### Test Debugging

**Enable Debug Logging:**
```bash
python run_tests.py --log-level DEBUG --verbose
```

**Check Test Logs:**
```bash
# View latest log
tail -f logs/latest.log

# View specific test logs
ls logs/bsb_tests_*.log
```

## Development

### Adding New Tests

1. **CLI Tests**: Add to `tests/cli/test_commands.py`
2. **Frontend Tests**: Add to `tests/frontend/test_web_frontend.py`
3. **Use appropriate markers**: `@pytest.mark.story_name`
4. **Include test case ID**: `@allure.testcase("ID", "Description")`

### Test Writing Guidelines

```python
@allure.testcase("2xxx", "Test Description [Draft]")
@pytest.mark.story_name
@pytest.mark.component_name
@pytest.mark.asyncio  # For CLI tests
async def test_example(self, cli):  # or web_session for frontend
    with allure.step("Describe what you're doing"):
        result = await cli.execute_command("test")
    
    with allure.step("Verify the result"):
        assert "expected" in result
```

### Performance Considerations

- CLI tests reuse session connections for speed
- Web tests use session-based HTTP connections
- Allure attachments are used selectively to avoid large report files
- Timeouts are optimized for BSB device response times

## Contributing

1. Follow existing code patterns and style
2. Add appropriate test markers and Allure annotations
3. Update test case IDs from Allure TestOps
4. Test on multiple platforms when possible
5. Update documentation for new features

## License

[Add your license information here]