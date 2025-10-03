"""Base page object with common functionality."""

try:
    import allure
except ImportError:
    # Mock allure if not available
    class allure:
        @staticmethod
        def step(title):
            def decorator(func):
                return func

            return decorator


import logging
from typing import Any, Optional

from config.settings import settings
from playwright.sync_api import Locator, Page, expect

logger = logging.getLogger(__name__)


class BasePage:
    """Base page object containing common methods for all pages."""

    def __init__(self, page: Page):
        self.page = page
        self.timeout = settings.element_timeout * 1000  # Convert to ms
        self.fast_timeout = settings.element_timeout * 1000  # 2s in ms
        self.modal_timeout = settings.modal_timeout * 1000  # 3s in ms
        self.long_timeout = settings.long_timeout * 1000  # 15s in ms

    # === Navigation Methods ===

    @allure.step("Navigate to {url}")
    def navigate_to(self, url: str = None) -> None:
        """Navigate to specified URL or base URL."""
        target_url = url or settings.base_url
        logger.info(f"Navigating to: {target_url}")
        self.page.goto(
            target_url,
            timeout=settings.navigation_timeout * 1000,
            wait_until="networkidle",
        )

    @allure.step("Reload page")
    def reload(self) -> None:
        """Reload the current page."""
        self.page.reload(wait_until="networkidle")

    @allure.step("Go back")
    def go_back(self) -> None:
        """Navigate back in browser history."""
        self.page.go_back()

    # === Timeout Helper Methods ===

    def get_timeout(
        self, timeout: Optional[int] = None, timeout_type: str = "default"
    ) -> int:
        """Get appropriate timeout value.

        Args:
            timeout: Custom timeout in milliseconds
            timeout_type: Type of timeout - 'fast', 'modal', 'long', or 'default'
        """
        if timeout is not None:
            return timeout

        timeout_map = {
            "fast": self.fast_timeout,
            "modal": self.modal_timeout,
            "long": self.long_timeout,
            "default": self.timeout,
        }

        return timeout_map.get(timeout_type, self.timeout)

    def with_fast_timeout(self, timeout: Optional[int] = None) -> int:
        """Get fast timeout (2s default)."""
        return timeout or self.fast_timeout

    def with_modal_timeout(self, timeout: Optional[int] = None) -> int:
        """Get modal timeout (3s default)."""
        return timeout or self.modal_timeout

    def with_long_timeout(self, timeout: Optional[int] = None) -> int:
        """Get long timeout (15s default)."""
        return timeout or self.long_timeout

    # === Element Interaction Methods ===

    def get_element(self, selector: str, nth: int = 0) -> Locator:
        """Get element by selector with optional index."""
        return self.page.locator(selector).nth(nth)

    @allure.step("Click element: {selector}")
    def click(self, selector: str, timeout: Optional[int] = None, **kwargs) -> None:
        """Click an element with enhanced error handling."""
        wait_timeout = timeout or self.timeout
        try:
            element = self.page.locator(selector)
            element.wait_for(state="visible", timeout=wait_timeout)
            element.click(**kwargs)
            logger.info(f"Clicked element: {selector}")
        except Exception as e:
            self.take_screenshot(f"click_failed_{selector}")
            raise AssertionError(f"Failed to click {selector}: {str(e)}")

    @allure.step("Fill input: {selector} with value: {value}")
    def fill(
        self,
        selector: str,
        value: str,
        clear_first: bool = True,
        timeout: Optional[int] = None,
    ) -> None:
        """Fill input field with value."""
        wait_timeout = timeout or self.timeout
        element = self.page.locator(selector)
        element.wait_for(state="visible", timeout=wait_timeout)
        if clear_first:
            element.clear()
        element.fill(value)
        logger.info(f"Filled {selector} with: {value}")

    @allure.step("Select option: {value} from {selector}")
    def select_option(self, selector: str, value: str) -> None:
        """Select option from dropdown."""
        self.page.select_option(selector, value)

    @allure.step("Check checkbox: {selector}")
    def check(self, selector: str) -> None:
        """Check a checkbox if not already checked."""
        element = self.page.locator(selector)
        if not element.is_checked():
            element.check()

    @allure.step("Uncheck checkbox: {selector}")
    def uncheck(self, selector: str) -> None:
        """Uncheck a checkbox if checked."""
        element = self.page.locator(selector)
        if element.is_checked():
            element.uncheck()

    # === Wait Methods ===

    def wait_for_element(
        self, selector: str, state: str = "visible", timeout: Optional[int] = None
    ) -> Locator:
        """Wait for element to reach specified state."""
        timeout = timeout or self.timeout
        element = self.page.locator(selector)
        element.wait_for(state=state, timeout=timeout)
        return element

    def wait_for_text(self, text: str, timeout: Optional[int] = None) -> None:
        """Wait for text to appear on page."""
        timeout = timeout or self.timeout
        self.page.wait_for_function(
            f"document.body.innerText.includes('{text}')", timeout=timeout
        )

    def wait_for_url(self, url_pattern: str, timeout: Optional[int] = None) -> None:
        """Wait for URL to match pattern."""
        timeout = timeout or settings.navigation_timeout * 1000
        self.page.wait_for_url(url_pattern, timeout=timeout)

    # === Verification Methods ===

    def is_visible(self, selector: str) -> bool:
        """Check if element is visible."""
        return self.page.locator(selector).is_visible()

    def is_enabled(self, selector: str) -> bool:
        """Check if element is enabled."""
        return self.page.locator(selector).is_enabled()

    def get_text(self, selector: str) -> str:
        """Get text content of element."""
        return self.page.locator(selector).text_content() or ""

    def get_attribute(self, selector: str, attribute: str) -> Optional[str]:
        """Get attribute value of element."""
        return self.page.locator(selector).get_attribute(attribute)

    @allure.step("Verify element visible: {selector}")
    def verify_element_visible(
        self, selector: str, message: str = None, timeout: Optional[int] = None
    ) -> None:
        """Verify element is visible with custom message."""
        wait_timeout = timeout or self.timeout
        try:
            expect(self.page.locator(selector)).to_be_visible(timeout=wait_timeout)
        except Exception as e:
            self.take_screenshot(f"element_not_visible_{selector}")
            raise AssertionError(message or f"Element not visible: {selector}")

    @allure.step("Verify text present: {text}")
    def verify_text_present(self, text: str, selector: str = "body") -> None:
        """Verify text is present in element or page."""
        expect(self.page.locator(selector)).to_contain_text(text, timeout=self.timeout)

    # === JavaScript Execution ===

    def execute_script(self, script: str, *args) -> Any:
        """Execute JavaScript in page context."""
        return self.page.evaluate(script, *args)

    def scroll_to_element(self, selector: str) -> None:
        """Scroll element into view."""
        self.page.locator(selector).scroll_into_view_if_needed()

    # === Screenshot & Debug ===

    @allure.step("Take screenshot: {name}")
    def take_screenshot(self, name: str = "screenshot") -> None:
        """Take screenshot and attach to Allure report."""
        screenshot = self.page.screenshot()
        allure.attach(
            screenshot, name=f"{name}.png", attachment_type=allure.attachment_type.PNG
        )

    def get_page_source(self) -> str:
        """Get current page HTML source."""
        return self.page.content()

    def get_console_logs(self) -> list:
        """Get browser console logs."""
        # This requires setup in conftest.py
        return []

    # === Modal/Dialog Handling ===

    def handle_dialog(self, accept: bool = True, text: str = None) -> None:
        """Handle JavaScript dialogs."""

        def dialog_handler(dialog):
            if text:
                dialog.accept(text)
            elif accept:
                dialog.accept()
            else:
                dialog.dismiss()

        self.page.on("dialog", dialog_handler)

    # === Frame Handling ===

    def switch_to_frame(self, selector: str) -> None:
        """Switch to iframe context."""
        frame = self.page.frame_locator(selector)
        return frame

    # === Network Interception ===

    def wait_for_request(self, url_pattern: str, timeout: int = 30000) -> None:
        """Wait for specific network request."""
        with self.page.expect_request(url_pattern, timeout=timeout) as request:
            pass
        return request.value

    def wait_for_response(self, url_pattern: str, timeout: int = 30000) -> None:
        """Wait for specific network response."""
        with self.page.expect_response(url_pattern, timeout=timeout) as response:
            pass
        return response.value
