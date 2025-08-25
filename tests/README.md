# BSB Test Automation

Automated test suite for BSB (BUSY Bar U5) CLI and Web Frontend using pytest with Allure TestOps integration.

## 🚀 Quick Start

### Prerequisites
- Python 3.11+ 
- Poetry (install from https://python-poetry.org/docs/#installation)
- Access to BSB device at `busybar.local` (telnet port 23)
- Web frontend accessible at `http://10.0.4.20/`
- Allure TestOps account (flipper.testops.cloud)

### Setup

1. **Clone and setup the project:**
```bash
git clone <your-repo-url>
cd bsb-automation-tests
```

2. **Run the setup script:**
```bash
python setup.py
```

This script will:
- Install Poetry dependencies
- Create `.env` file from template
- Test CLI and web connections  
- Verify directory structure

3. **Configure Allure TestOps:**
Edit `.env` file and add your TestOps token:
```bash
ALLURE_TESTOPS_TOKEN=your-token-here
```

## 🧪 Running Tests

### Local Test Execution

```bash
# Run all tests
poetry run pytest tests/ -v

# Run only CLI tests
poetry run pytest tests/cli/ -m cli -v

# Run only frontend tests  
poetry run pytest tests/frontend/ -m frontend -v

# Run specific story tests
poetry run pytest -m story_commands_check -v
poetry run pytest -m story_ui_interaction -v
```

### With Allure Reporting

```bash
# Generate local Allure report
poetry run pytest tests/ --alluredir=allure-results
poetry run allure serve allure-results
```

### GitHub Actions (Self-hosted Runner)

The workflow supports:
- Manual triggers with test suite selection
- Scheduled daily runs
- Automatic upload to Allure TestOps

Required GitHub secrets/variables:
- `ALLURE_TESTOPS_TOKEN` (secret)
- `ALLURE_TESTOPS_URL` (variable, defaults to flipper.testops.cloud)
- `ALLURE_PROJECT_ID` (variable, defaults to 232)

## 📁 Project Structure

```
bsb-automation-tests/
├── tests/
│   ├── cli/
│   │   └── test_commands.py        # CLI command tests
│   └── frontend/
│       └── test_web_frontend.py    # Web frontend tests
├── conftest.py                     # Pytest configuration & fixtures
├── pyproject.toml                  # Poetry dependencies
├── .env.template                   # Environment template
├── setup.py                        # Setup script
└── .github/workflows/run-tests.yml # CI/CD workflow
```

## 🔧 Test Architecture

### CLI Testing
- Uses `telnetlib3` for async telnet connections
- Custom `CLIFixture` class for command execution
- Automatic connection/disconnection management
- Command output capture and validation

### Web Frontend Testing  
- Uses `requests` library for HTTP testing
- Basic page load validation
- Response code and content checks
- Browser compatibility testing

### Allure Integration
- Test case mapping with `@allure.testcase()` decorators
- Automatic story/feature labeling from markers
- Step-by-step test execution logging
- Attachment of CLI outputs and web responses
- Integration with Allure TestOps cloud

## 🏷️ Test Markers

Available pytest markers for test filtering:

**By Component:**
- `cli` - CLI command tests
- `frontend` - Web frontend tests

**By Story:**
- `story_commands_check` - Commands Check story
- `story_ui_validation` - UI validation story  
- `story_ui_interaction` - UI interaction story
- `story_interface_status` - Interface status story
- `story_mqtt` - MQTT story

**By Feature:**
- `feature_cli` - Feature 6. CLI
- `feature_web_frontend` - Feature 5. Web Frontend

## 📋 Test Cases Implemented

### CLI Tests (Feature 6)
**Commands Check Story:**
- CLI. Command ? [#2047]
- CLI. Command Exit [#2046] 
- CLI. Command Free [#2043]
- CLI. Command Help [#2045]
- CLI. Command Storage [#2044]
- CLI. Command Sl_cli [#2040]
- CLI. Command Uptime [#2041]
- CLI. Command Device_info [#2035]
- CLI. Command Audio [#2028]
- CLI. Command Display [#2030]
- CLI. Command Echo [#2031]
- And many more...

**UI Validation Story:**
- CLI. UI. Render [#2048]
- CLI. UI. Version [#2049] 
- CLI. UI. Build Info [#2050]
- CLI. UI. Welcome message [#2152]
- CLI. Commands. History [#2127]
- CLI. Commands. Aliases [#2129]
- CLI. Commands. Tab Completion [#2128]

### Frontend Tests (Feature 5)
**UI Interaction Story:**
- BSB Front. Themes Dark/Bright [#2017]
- BSB Front. Cloud Remote Control Behind NAT [#2003]
- BSB Front. Compare to Mock-ups [#2018]
- BSB Front. Accessibility Compliance [#2123]
- BSB Front. Form Validation [#2121]
- BSB Front. Browser Compatibility [#2119]
- BSB Front. Responsive Design Validation [#2118]
- BSB Front. Touch/mouse Input Handling [#2120]

## 🌐 CI/CD Integration

### Self-hosted Runner Setup
For accessing local BSB device, configure a self-hosted GitHub Actions runner:

1. Go to repository Settings → Actions → Runners
2. Add a new self-hosted runner
3. Install on machine with access to BSB device
4. Ensure the runner can reach:
   - `busybar.local:23` (telnet)
   - `10.0.4.20:80` (HTTP)

### Allure TestOps Integration
Tests automatically upload results to Allure TestOps cloud:
- Project: https://flipper.testops.cloud/project/232
- Requires API token configuration
- Supports custom launch names
- Includes test case traceability

## 🔍 Troubleshooting

### CLI Connection Issues
```bash
# Test telnet connection manually
telnet busybar.local 23

# Check if device responds to ping
ping busybar.local
```

### Web Frontend Issues  
```bash
# Test web connection manually
curl -v http://10.0.4.20/

# Check network connectivity
ping 10.0.4.20
```

### Poetry Issues
```bash
# Reset virtual environment
poetry env remove python
poetry install

# Update dependencies
poetry update
```

## 📝 Development

### Adding New Tests

1. **CLI Tests:** Add to `tests/cli/test_commands.py`
2. **Frontend Tests:** Add to `tests/frontend/test_web_frontend.py`
3. **Use appropriate markers and test case IDs**
4. **Follow the existing patterns for fixtures and assertions**

### Test Case Mapping
Each test should include:
```python
@allure.testcase("2047", "CLI. Command ?. [Draft]")
@pytest.mark.story_commands_check
@pytest.mark.cli
```

This ensures proper traceability in Allure TestOps.

## 🤝 Contributing

1. Follow existing code patterns
2. Add appropriate markers to new tests  
3. Update test case IDs from Allure TestOps
4. Test locally before pushing
5. Update documentation as needed