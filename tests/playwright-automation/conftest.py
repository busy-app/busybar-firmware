"""Pytest configuration and fixtures."""

import os

import pytest

try:
    import allure

    ALLURE_AVAILABLE = True
except ImportError:
    ALLURE_AVAILABLE = False

    class MockObject:
        def __getattr__(self, name):
            return lambda *args, **kwargs: None

    class MockAllure(MockObject):
        def attach(*args, **kwargs):
            pass

        class attach:
            @staticmethod
            def file(*args, **kwargs):
                pass

        class attachment_type:
            PNG = "image/png"
            HTML = "text/html"
            TEXT = "text/plain"

    allure = MockAllure()
    allure.dynamic = MockObject()

    allure = MockAllure()
import argparse
import logging
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, Generator

import playwright
from config.settings import settings
from pages.control_panel import ControlPanelPage
from playwright.sync_api import (Browser, BrowserContext, Page, Playwright,
                                 sync_playwright)

# Configure logging
logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


# === Pytest Hooks ===


def pytest_configure(config):
    """Configure pytest with custom markers and setup."""
    # Register custom markers
    config.addinivalue_line("markers", "smoke: Quick smoke tests for CI/CD")
    config.addinivalue_line("markers", "regression: Full regression test suite")
    config.addinivalue_line("markers", "critical: Critical functionality tests")
    config.addinivalue_line("markers", "wifi: WiFi configuration tests")
    config.addinivalue_line("markers", "bluetooth: Bluetooth functionality tests")
    config.addinivalue_line("markers", "firmware: Firmware update tests")
    config.addinivalue_line("markers", "slow: Tests that take more than 30 seconds")
    config.addinivalue_line(
        "markers", "flaky: Tests that might be flaky and need retry"
    )
    config.addinivalue_line(
        "markers", "incremental: Tests that depend on previous test success"
    )

    # Create necessary directories
    for dir_name in [settings.screenshot_dir, settings.video_dir, settings.trace_dir]:
        Path(dir_name).mkdir(exist_ok=True)

    # Set up Allure environment (if allure is available)
    if ALLURE_AVAILABLE:
        allure_dir = Path("allure-results")
        allure_dir.mkdir(exist_ok=True)

        # Write environment properties for Allure
        env_file = allure_dir / "environment.properties"
        with open(env_file, "w") as f:
            f.write(f"Browser={settings.browser}\n")
            f.write(f"Base.URL={settings.base_url}\n")
            f.write(f"Environment={settings.environment}\n")
            f.write(f"Headless={settings.headless}\n")
            f.write(f"Viewport={settings.get_viewport()}\n")
            # Get playwright version safely
            try:
                import playwright

                pw_version = getattr(playwright, "__version__", "Unknown")
            except:
                pw_version = "Unknown"
            f.write(f"Playwright.Version={pw_version}\n")


def pytest_addoption(parser):
    """Add custom command line options."""
    # Add only our custom options that don't conflict with pytest-playwright
    # pytest-playwright already provides: --browser, --headed/--headless, --base-url, --slowmo

    parser.addoption(
        "--device-ip",
        action="store",
        help="Device IP address (overrides busybar.local)",
    )
    parser.addoption(
        "--capture-video",
        action="store_true",
        help="Record video for all tests (in addition to failures)",
    )
    parser.addoption(
        "--capture-trace",
        action="store_true",
        help="Capture trace for all tests (in addition to failures)",
    )


def pytest_runtest_setup(item):
    """Setup for each test."""
    # Override settings with CLI arguments from pytest-playwright and our custom ones
    if hasattr(item.config.option, "device_ip") and item.config.option.device_ip:
        settings.device_ip = item.config.option.device_ip

    # Use pytest-playwright options if available
    if hasattr(item.config.option, "base_url") and item.config.option.base_url:
        settings.base_url = item.config.option.base_url
    if hasattr(item.config.option, "browser_name"):
        settings.browser = getattr(item.config.option, "browser_name", settings.browser)
    if hasattr(item.config.option, "headed"):
        settings.headless = (
            not getattr(item.config.option, "headed", not settings.headless)
            or settings.is_ci
        )
    if hasattr(item.config.option, "slowmo"):
        settings.slow_mo = getattr(item.config.option, "slowmo", settings.slow_mo)


# Incremental test handling is done via pytest_runtest_makereport hook


# === Core Browser Fixtures ===
# pytest-playwright provides: playwright, browser_type, browser, context, page
# We just enhance them with our custom behavior


@pytest.fixture(autouse=True)
def screenshot_on_failure(page: Page, request):
    """Automatically capture screenshots on test failure."""
    yield

    # Check if the test failed and capture screenshot
    if (
        settings.screenshot_on_failure
        and hasattr(request.node, "_test_failed")
        and request.node._test_failed
    ):
        try:
            test_name = _sanitize_filename(request.node.name)
            class_name = (
                _sanitize_filename(request.node.parent.name)
                if request.node.parent
                else "unknown"
            )
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")[:-3]

            # Create screenshot directory
            screenshot_dir = Path(settings.screenshot_dir)
            screenshot_dir.mkdir(parents=True, exist_ok=True)

            # Screenshot filename with test metadata
            screenshot_filename = f"FAILED_{class_name}_{test_name}_{timestamp}.png"
            screenshot_path = screenshot_dir / screenshot_filename

            # Take screenshot
            page.screenshot(path=str(screenshot_path), full_page=True)
            logger.info(f"Failure screenshot captured: {screenshot_path}")

            # Attach to Allure report
            allure.attach.file(
                str(screenshot_path),
                name=f"Failed Test Screenshot - {test_name}",
                attachment_type=allure.attachment_type.PNG,
            )

            # Also attach screenshot bytes for better reliability
            with open(screenshot_path, "rb") as f:
                screenshot_bytes = f.read()
            allure.attach(
                screenshot_bytes,
                name="Failure Screenshot",
                attachment_type=allure.attachment_type.PNG,
            )

            # Capture additional debugging info
            try:
                # Page HTML
                html_content = page.content()
                allure.attach(
                    html_content,
                    name="Page HTML at Failure",
                    attachment_type=allure.attachment_type.HTML,
                )

                # Page URL
                current_url = page.url
                allure.attach(
                    current_url,
                    name="Current URL",
                    attachment_type=allure.attachment_type.TEXT,
                )

            except Exception as e:
                logger.warning(f"Could not capture additional page info: {e}")

        except Exception as e:
            logger.error(f"Failed to capture screenshot: {e}")
            allure.attach(
                f"Screenshot capture failed: {str(e)}",
                name="Screenshot Error",
                attachment_type=allure.attachment_type.TEXT,
            )


@pytest.fixture(autouse=True)
def enhanced_page(page: Page, request) -> None:
    """Enhance pytest-playwright's page fixture with custom behavior."""
    # Set default timeouts
    page.set_default_timeout(settings.default_timeout * 1000)
    page.set_default_navigation_timeout(settings.navigation_timeout * 1000)

    # Console log collection
    console_logs = []
    page.on(
        "console",
        lambda msg: console_logs.append(
            {
                "type": msg.type,
                "text": msg.text,
                "location": f"{msg.location['url']}:{msg.location['lineNumber']}",
            }
        ),
    )

    # Network request/response logging
    requests = []
    responses = []

    page.on(
        "request",
        lambda req: requests.append(
            {"method": req.method, "url": req.url, "headers": dict(req.headers)}
        ),
    )

    page.on(
        "response",
        lambda resp: responses.append(
            {"status": resp.status, "url": resp.url, "headers": dict(resp.headers)}
        ),
    )

    # Error handling
    page.on("pageerror", lambda error: logger.error(f"Page error: {error}"))
    page.on("crash", lambda: logger.error("Page crashed"))

    # Attach logs to page for access in tests
    page.console_logs = console_logs
    page.network_requests = requests
    page.network_responses = responses

    yield

    # Capture console and network logs on failure
    if hasattr(request.node, "_test_failed") and request.node._test_failed:
        # Console logs
        if console_logs:
            try:
                logs_text = "\n".join(
                    [
                        f"[{log['type'].upper()}] {log['text']} - {log['location']}"
                        for log in console_logs
                    ]
                )
                allure.attach(
                    logs_text,
                    name="Console Logs",
                    attachment_type=allure.attachment_type.TEXT,
                )
            except Exception as e:
                logger.warning(f"Could not attach console logs: {e}")

        # Network activity
        if requests:
            try:
                network_text = "REQUESTS:\n" + "\n".join(
                    [f"{req['method']} {req['url']}" for req in requests]
                )
                if responses:
                    network_text += "\n\nRESPONSES:\n" + "\n".join(
                        [f"{resp['status']} {resp['url']}" for resp in responses]
                    )
                allure.attach(
                    network_text,
                    name="Network Activity",
                    attachment_type=allure.attachment_type.TEXT,
                )
            except Exception as e:
                logger.warning(f"Could not attach network logs: {e}")


@pytest.fixture(scope="function")
def new_page(context: BrowserContext) -> Generator[Page, None, None]:
    """Create additional page instances when needed."""
    page = context.new_page()
    page.set_default_timeout(settings.default_timeout * 1000)
    yield page
    page.close()


# === Application-specific Fixtures ===


@pytest.fixture(scope="function")
def control_panel_page(page: Page) -> ControlPanelPage:
    """Pre-configured control panel page object."""
    return ControlPanelPage(page)


@pytest.fixture(scope="function")
def device_connected(control_panel_page: ControlPanelPage) -> ControlPanelPage:
    """Control panel with verified device connection."""
    with allure.step("Navigate to control panel and verify connection"):
        control_panel_page.open()
        control_panel_page.verify_device_connected()
    return control_panel_page


# === Data Fixtures ===


@pytest.fixture(scope="session")
def test_data() -> Dict[str, Any]:
    """Load test data from JSON files or environment."""
    return {
        "wifi": {
            "ssid": os.getenv("TEST_WIFI_SSID", "TestNetwork"),
            "password": os.getenv("TEST_WIFI_PASSWORD", "TestPassword123"),
            "security_types": ["WPA2", "WPA3", "WEP", "Open"],
        },
        "bluetooth": {
            "device_name": os.getenv("TEST_BT_DEVICE", "TestDevice"),
            "pin": os.getenv("TEST_BT_PIN", "0000"),
        },
        "firmware": {
            "test_file_path": os.getenv("TEST_FIRMWARE_PATH", "test_firmware.bin")
        },
        "device": {
            "expected_display_width": 360,
            "expected_display_height": 80,
            "min_firmware_version": "1.0.0",
        },
    }


# === Utility Fixtures ===


@pytest.fixture(scope="function")
def screenshot_helper(page: Page) -> callable:
    """Helper function to take screenshots with automatic naming."""

    def take_screenshot(name: str = None, full_page: bool = True) -> None:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        screenshot_name = f"{name}_{timestamp}" if name else f"screenshot_{timestamp}"
        screenshot_path = Path(settings.screenshot_dir) / f"{screenshot_name}.png"

        page.screenshot(path=screenshot_path, full_page=full_page)
        allure.attach.file(
            screenshot_path,
            name=screenshot_name,
            attachment_type=allure.attachment_type.PNG,
        )
        logger.info(f"Screenshot saved: {screenshot_path}")

    return take_screenshot


@pytest.fixture(scope="function")
def wait_helper(page: Page) -> callable:
    """Helper for common wait operations."""

    def wait_for_condition(
        condition: callable, timeout: int = 30, interval: int = 1
    ) -> bool:
        """Wait for custom condition with timeout."""
        import time

        start_time = time.time()
        while time.time() - start_time < timeout:
            if condition():
                return True
            time.sleep(interval)
        return False

    return wait_for_condition


# === Artifact Capture Functions ===


def _capture_test_artifacts(item, rep):
    """Capture screenshots, videos, and other test artifacts on failure."""
    try:
        # Get the page object from the test fixtures
        page = None
        if hasattr(item, "_request") and hasattr(item._request, "getfixturevalue"):
            try:
                page = item._request.getfixturevalue("page")
            except:
                # If page fixture is not available, try to get it from other sources
                pass

        # Alternative way to get page from test function arguments
        if not page and hasattr(item, "funcargs"):
            page = item.funcargs.get("page")

        # Last resort: try to get page from pytest fixtures
        if not page:
            for fixture_name in ["page", "control_panel_page"]:
                try:
                    if hasattr(item.session, "_fixturemanager"):
                        page = item.session._fixturemanager.getfixturevalue(
                            fixture_name, item
                        )
                        if hasattr(
                            page, "screenshot"
                        ):  # Check if it's actually a page object
                            break
                        elif hasattr(page, "page"):  # It might be a page object wrapper
                            page = page.page
                            break
                        else:
                            page = None
                except:
                    continue

        if page and hasattr(page, "screenshot"):
            test_name = _sanitize_filename(item.name)
            class_name = (
                _sanitize_filename(item.parent.name) if item.parent else "unknown"
            )
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")[
                :-3
            ]  # microseconds to milliseconds

            # Create screenshot directory
            screenshot_dir = Path(settings.screenshot_dir)
            screenshot_dir.mkdir(parents=True, exist_ok=True)

            # Screenshot filename with test metadata
            screenshot_filename = f"{class_name}_{test_name}_{timestamp}.png"
            screenshot_path = screenshot_dir / screenshot_filename

            try:
                # Take screenshot
                page.screenshot(path=str(screenshot_path), full_page=True)
                logger.info(f"Screenshot captured: {screenshot_path}")

                # Attach to Allure with both file and bytes for better compatibility
                allure.attach.file(
                    str(screenshot_path),
                    name=f"Screenshot - {test_name}",
                    attachment_type=allure.attachment_type.PNG,
                )

                # Also attach screenshot bytes directly
                try:
                    with open(screenshot_path, "rb") as f:
                        screenshot_bytes = f.read()
                    allure.attach(
                        screenshot_bytes,
                        name=f"Failure Screenshot",
                        attachment_type=allure.attachment_type.PNG,
                    )
                except Exception as e:
                    logger.warning(f"Could not attach screenshot bytes: {e}")

            except Exception as e:
                logger.error(f"Failed to take screenshot: {e}")
                # Attach error info to Allure
                allure.attach(
                    f"Screenshot capture failed: {str(e)}",
                    name="Screenshot Error",
                    attachment_type=allure.attachment_type.TEXT,
                )

            # Capture page HTML for debugging
            try:
                html_content = page.content()
                allure.attach(
                    html_content,
                    name="Page HTML at Failure",
                    attachment_type=allure.attachment_type.HTML,
                )
            except Exception as e:
                logger.warning(f"Could not capture page HTML: {e}")

            # Capture console logs if available
            if hasattr(page, "console_logs") and page.console_logs:
                try:
                    logs_text = "\n".join(
                        [
                            f"[{log['type'].upper()}] {log['text']} - {log['location']}"
                            for log in page.console_logs
                        ]
                    )
                    allure.attach(
                        logs_text,
                        name="Console Logs",
                        attachment_type=allure.attachment_type.TEXT,
                    )
                except Exception as e:
                    logger.warning(f"Could not attach console logs: {e}")

            # Capture network activity if available
            if hasattr(page, "network_requests") and page.network_requests:
                try:
                    network_text = "REQUESTS:\n" + "\n".join(
                        [
                            f"{req['method']} {req['url']}"
                            for req in page.network_requests
                        ]
                    )
                    if hasattr(page, "network_responses") and page.network_responses:
                        network_text += "\n\nRESPONSES:\n" + "\n".join(
                            [
                                f"{resp['status']} {resp['url']}"
                                for resp in page.network_responses
                            ]
                        )
                    allure.attach(
                        network_text,
                        name="Network Activity",
                        attachment_type=allure.attachment_type.TEXT,
                    )
                except Exception as e:
                    logger.warning(f"Could not attach network logs: {e}")

        else:
            logger.warning(
                f"Could not capture screenshot - page object not found for test: {item.name}"
            )
            allure.attach(
                "Page object was not available for screenshot capture",
                name="Screenshot Unavailable",
                attachment_type=allure.attachment_type.TEXT,
            )

    except Exception as e:
        logger.error(f"Error in _capture_test_artifacts: {e}")
        allure.attach(
            f"Artifact capture error: {str(e)}",
            name="Artifact Capture Error",
            attachment_type=allure.attachment_type.TEXT,
        )


def _sanitize_filename(filename: str) -> str:
    """Sanitize filename for cross-platform compatibility."""
    import re

    # Replace invalid characters with underscore
    sanitized = re.sub(r'[<>:"/\|?*]', "_", str(filename))
    # Remove extra underscores and limit length
    sanitized = re.sub(r"_+", "_", sanitized).strip("_")[:100]
    return sanitized or "unknown"


# === Pytest Hooks for Allure Integration ===

# Temporarily disabled pytest hook due to allure issues
# @pytest.hookimpl(tryfirst=True, hookwrapper=True)
# def pytest_runtest_makereport(item, call):
#     """Enhance test reports for Allure and capture screenshots on failure."""
#     outcome = yield
#     rep = outcome.get_result()
#
#     # Only do allure integration if allure is available
#     if ALLURE_AVAILABLE and rep.when == "call":
#         # Test environment info
#         allure.dynamic.parameter("browser", settings.browser)
#         allure.dynamic.parameter("headless", settings.headless)
#         allure.dynamic.parameter("base_url", settings.base_url)
#
#         # Test duration
#         if hasattr(rep, "duration"):
#             allure.dynamic.parameter("duration", f"{rep.duration:.2f}s")
#
#         # Capture screenshot and artifacts on test failure
#         if rep.failed and settings.screenshot_on_failure:
#             _capture_test_artifacts(item, rep)
#
#     # Store report in item for access in fixtures
#     setattr(item, "rep_" + rep.when, rep)
#
#     # Alternative approach: trigger screenshot capture via fixture finalizer
#     if rep.when == "call" and rep.failed:
#         # Set a flag that can be checked by fixtures
#         setattr(item, "_test_failed", True)


@pytest.fixture(autouse=True)
def allure_environment():
    """Set Allure environment information."""
    if ALLURE_AVAILABLE:
        allure.dynamic.label("layer", "ui")
        allure.dynamic.label("owner", "qa-team")
        allure.dynamic.label("framework", "playwright")


# === Device Health Check Fixtures ===


@pytest.fixture(scope="session")
def device_health_check():
    """Check if device is reachable before running tests."""
    from urllib.parse import urlparse

    import requests

    try:
        parsed_url = urlparse(settings.base_url)
        health_url = f"{parsed_url.scheme}://{parsed_url.netloc}/health"

        response = requests.get(
            settings.base_url,
            timeout=settings.default_timeout,
            headers={"User-Agent": "BUSY-Bar-Tests/1.0"},
        )

        if response.status_code == 200:
            logger.info(f"Device health check passed: {settings.base_url}")
            return True
        else:
            logger.warning(f"Device responded with status {response.status_code}")
            return True  # Still allow tests to run

    except requests.exceptions.RequestException as e:
        logger.error(f"Device health check failed: {e}")
        pytest.skip(f"Device not reachable at {settings.base_url}: {e}")


# === Browser Installation Check ===


@pytest.fixture(scope="session", autouse=True)
def ensure_browsers_installed(playwright: Playwright):
    """Ensure required browsers are installed."""
    try:
        # Try to get browser executable path
        browser_type = getattr(playwright, settings.browser)
        executable_path = browser_type.executable_path

        if not Path(executable_path).exists():
            logger.error(f"Browser {settings.browser} not found at {executable_path}")
            pytest.exit(
                f"Browser {settings.browser} not installed. Run: playwright install {settings.browser}"
            )

    except Exception as e:
        logger.error(f"Error checking browser installation: {e}")
        pytest.exit(f"Error checking browser installation. Run: playwright install")


# === Incremental Test Support ===


class IncrementalFailure(Exception):
    """Exception for incremental test failures."""

    pass


# === Custom Markers Registration ===
# (Already handled in the main pytest_configure function above)
