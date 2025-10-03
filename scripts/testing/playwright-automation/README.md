# BUSY Bar Test Automation Framework

Enterprise-level test automation framework for BUSY Bar device using Playwright, pytest, and Allure reporting.

## 🎯 Features

- **Multi-browser support**: Chromium (Chrome/Edge), Firefox, and WebKit (Safari)
- **Page Object Model**: Clean, maintainable test structure
- **Allure Reports**: Beautiful, detailed test reports with screenshots and traces
- **CI/CD Ready**: GitHub Actions workflow included
- **Parallel Execution**: Run tests in parallel for faster feedback
- **Auto-retry**: Automatic retry for flaky tests
- **Rich debugging**: Screenshots, traces, and video recording on failures

## 🚀 Quick Start

### Prerequisites

- Python 3.9+
- BUSY Bar device connected via USB-Ethernet at `http://busybar.local`

### Installation

#### Automated Setup (Recommended)

```bash
chmod +x setup.sh
./setup.sh
```

#### Manual Setup

1. **Create virtual environment:**
```bash
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

2. **Install dependencies:**
```bash
pip install -r requirements.txt
```

3. **Install browsers:**
```bash
# Install all browsers
playwright install chromium firefox webkit
playwright install-deps

# Or specific browser only
playwright install chromium
playwright install-deps chromium
```

4. **Install Allure (for reports):**

**macOS:**
```bash
brew install allure
```

**Linux:**
```bash
wget https://github.com/allure-framework/allure2/releases/download/2.25.0/allure-2.25.0.tgz
tar -zxf allure-2.25.0.tgz
sudo mv allure-2.25.0 /opt/allure
sudo ln -s /opt/allure/bin/allure /usr/local/bin/allure
```

**Windows:**
Download from [Allure GitHub](https://github.com/allure-framework/allure2/releases)

## 🧪 Running Tests

### Basic Commands

```bash
# Run all tests
pytest

# Run smoke tests only
pytest -m smoke

# Run specific test file
pytest tests/test_connectivity.py

# Run with specific browser
pytest --browser=firefox

# Run in headless mode
pytest --headless

# Run tests in parallel
pytest -n auto

# Generate Allure report after tests
allure serve allure-results
```

### Test Markers

- `@pytest.mark.smoke` - Quick smoke tests
- `@pytest.mark.regression` - Full regression suite
- `@pytest.mark.critical` - Critical functionality
- `@pytest.mark.wifi` - WiFi configuration tests
- `@pytest.mark.bluetooth` - Bluetooth tests
- `@pytest.mark.firmware` - Firmware update tests
- `@pytest.mark.slow` - Long-running tests
- `@pytest.mark.flaky` - Tests that might need retry

### Examples

```bash
# Run critical tests only in headless mode
pytest -m critical --headless

# Run WiFi tests with Firefox
pytest -m wifi --browser=firefox

# Run with custom device URL
pytest --device-url=http://10.0.4.20

# Run with slow motion (debugging)
pytest --slow-mo=1000

# Generate HTML report (backup)
pytest --html=report.html --self-contained-html
```

## 📁 Project Structure

```
busybar-tests/
├── config/
│   └── settings.py         # Configuration management
├── pages/
│   ├── base_page.py       # Base page object
│   ├── control_panel.py   # Main control panel page
│   └── components/        # UI components
├── tests/
│   ├── test_connectivity.py
│   ├── test_wifi_config.py
│   └── test_firmware.py
├── utils/
│   ├── wait_helpers.py
│   └── logger.py
├── conftest.py            # Pytest fixtures
├── pytest.ini             # Pytest configuration
└── requirements.txt       # Python dependencies
```

## ⚙️ Configuration

### Environment Variables (.env)

```env
# Device Configuration
BASE_URL=http://busybar.local
DEVICE_IP=10.0.4.20  # Optional: specific IP
DEVICE_PORT=80

# Browser Settings
BROWSER=chromium  # chromium, firefox, webkit
HEADLESS=false
SLOW_MO=0  # Milliseconds delay between actions

# Test Settings
DEFAULT_TIMEOUT=30
NAVIGATION_TIMEOUT=60
SCREENSHOT_ON_FAILURE=true
VIDEO_ON_FAILURE=false
TRACE_ON_FAILURE=true

# Environment
ENVIRONMENT=local  # local, ci, staging
CI_MODE=false
```

### pytest.ini Configuration

Key settings in `pytest.ini`:
- Test discovery patterns
- Logging configuration
- Allure report settings
- Timeout configurations
- Retry settings for flaky tests

## 📊 Allure 3 Reporting

### What's New in Allure 3

- **Modern UI**: Completely redesigned interface with better navigation
- **Enhanced Performance**: Faster report generation and loading
- **Better Filtering**: Advanced test filtering and search capabilities
- **Improved Timeline**: Enhanced test execution timeline visualization
- **Native Dark Mode**: Built-in dark mode support
- **Better Integration**: Improved CI/CD integration capabilities

### Generate and View Reports

```bash
# Generate report from last run
allure generate allure-results --clean -o allure-report

# Serve report (opens in browser with live reload)
allure serve allure-results

# Generate single-file report for CI
allure generate allure-results --clean -o allure-report --single-file

# Open existing report
allure open allure-report
```

### Report Features
- Real-time test execution monitoring
- Enhanced test categorization
- Improved failure analysis
- Better attachment handling
- Advanced metrics and analytics
- Test history and trends
- Customizable dashboards

## 🔧 CI/CD Integration

### GitHub Actions

The included workflow (`.github/workflows/test.yml`) provides:
- Automated test runs on push/PR
- Multi-browser testing matrix
- Allure report generation
- Artifact storage
- PR comments with results

### Running in CI

```yaml
# Trigger workflow manually with browser selection
gh workflow run test.yml -f browser=firefox

# View workflow runs
gh run list --workflow=test.yml
```

## 🐳 Docker Support

### Running Tests in Docker

```bash
# Build Docker image
make docker-build
# or
docker-compose build

# Run tests in Docker
docker-compose up

# Run specific tests
docker-compose run busybar-tests poetry run pytest tests/test_connectivity.py

# Run with different markers
TEST_ARGS="-m critical" docker-compose up

# View Allure reports (runs on http://localhost:5050)
docker-compose up allure-service
```

### Using Selenium Grid (Optional)

```bash
# Start Selenium Grid with Chrome and Firefox nodes
docker-compose --profile grid up

# Run tests against grid
SELENIUM_GRID_URL=http://localhost:4444 poetry run pytest
```

## 🛠️ Makefile Commands

The project includes a Makefile for common tasks:

```bash
make help          # Show all available commands
make setup         # Full environment setup
make install       # Install dependencies
make test          # Run all tests
make test-smoke    # Run smoke tests
make test-critical # Run critical tests
make test-browser BROWSER=firefox  # Run with specific browser
make test-headed   # Run tests in headed mode (for debugging)
make report        # Generate and serve Allure report
make format        # Format code with black
make lint          # Run linting checks
make clean         # Clean test artifacts
make check         # Check environment setup
```

### Local Debugging

1. **Run with headed browser:**
```bash
pytest --headed --slow-mo=1000
```

2. **Enable Playwright Inspector:**
```bash
PWDEBUG=1 pytest tests/test_connectivity.py::test_device_usb_connection
```

3. **Capture traces:**
```bash
pytest --trace-on-failure
playwright show-trace traces/trace.zip
```

4. **Video recording:**
```bash
VIDEO_ON_FAILURE=true pytest
```

### Troubleshooting

**Device not accessible:**
```bash
# Check device connectivity
ping busybar.local
curl http://busybar.local

# Use IP address directly
pytest --device-url=http://10.0.4.20
```

**Browser installation issues:**
```bash
# Reinstall browsers
playwright install --force chromium
playwright install-deps chromium
```

**Permission issues on Linux:**
```bash
# Add user to dialout group for USB access
sudo usermod -a -G dialout $USER
# Logout and login again
```

## 📋 Best Practices

1. **Use Page Objects**: Keep selectors and page logic in page classes
2. **Explicit Waits**: Use `wait_for_element()` instead of `time.sleep()`
3. **Meaningful Assertions**: Include clear error messages
4. **Test Independence**: Each test should be runnable independently
5. **Data Separation**: Keep test data in fixtures or external files
6. **Allure Steps**: Use `@allure.step` for better report readability
7. **Screenshots**: Capture screenshots at critical points
8. **Cleanup**: Ensure proper cleanup in test teardown

## 🧩 Extending the Framework

### Adding New Page Objects

```python
# pages/new_page.py
from pages.base_page import BasePage

class NewPage(BasePage):
    # Define selectors
    BUTTON = "button#my-button"
    
    def click_button(self):
        self.click(self.BUTTON)
```

### Adding New Tests

```python
# tests/test_new_feature.py
import pytest
import allure

@allure.feature("New Feature")
class TestNewFeature:
    
    @pytest.mark.smoke
    @allure.title("Test new functionality")
    def test_new_functionality(self, page):
        # Test implementation
        pass
```

### Custom Fixtures

```python
# conftest.py
@pytest.fixture
def logged_in_user(page):
    """Fixture for pre-authenticated user."""
    # Login logic
    yield page
    # Cleanup
```

## 📝 Writing Tests

### Test Structure Example

```python
@allure.feature("Feature Name")
@allure.story("User Story")
class TestExample:
    
    @pytest.mark.smoke
    @pytest.mark.critical
    @allure.title("Descriptive test title")
    @allure.description("Detailed test description")
    @allure.severity(allure.severity_level.CRITICAL)
    def test_example(self, page, test_data):
        with allure.step("Setup"):
            # Preparation steps
            pass
        
        with allure.step("Action"):
            # Test actions
            pass
        
        with allure.step("Verification"):
            # Assertions
            pass
```

## 🤝 Contributing

1. Follow PEP 8 style guide
2. Add tests for new features
3. Update documentation
4. Run `black` for code formatting
5. Run `pylint` for code quality

## 📚 Resources

- [Playwright Python Documentation](https://playwright.dev/python/)
- [Pytest Documentation](https://docs.pytest.org/)
- [Allure Documentation](https://docs.qameta.io/allure/)
- [Page Object Model Best Practices](https://www.selenium.dev/documentation/test_practices/encouraged/page_object_models/)

## 📄 License

[Your License Here]

## 👤 Authors

[Your Name/Team]

---

For questions or issues, please create a GitHub issue or contact the test automation team.