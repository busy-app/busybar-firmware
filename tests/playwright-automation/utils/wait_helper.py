"""Custom wait helpers for test automation."""

import logging
import time
from typing import Any, Callable, Optional

from playwright.sync_api import Locator, Page, expect

logger = logging.getLogger(__name__)


class WaitHelpers:
    """Custom wait conditions and helpers."""

    @staticmethod
    def wait_for_condition(
        condition: Callable[[], bool],
        timeout: int = 30,
        poll_interval: float = 0.5,
        message: str = "Condition not met",
    ) -> bool:
        """
        Wait for a custom condition to be true.

        Args:
            condition: Callable that returns True when condition is met
            timeout: Maximum wait time in seconds
            poll_interval: Time between condition checks in seconds
            message: Error message if timeout occurs

        Returns:
            True if condition met, raises TimeoutError otherwise
        """
        end_time = time.time() + timeout

        while time.time() < end_time:
            try:
                if condition():
                    return True
            except Exception as e:
                logger.debug(f"Condition check failed: {e}")

            time.sleep(poll_interval)

        raise TimeoutError(f"Timeout after {timeout}s: {message}")

    @staticmethod
    def wait_for_element_attribute(
        page: Page, selector: str, attribute: str, value: str, timeout: int = 10000
    ) -> None:
        """Wait for element attribute to have specific value."""
        page.wait_for_function(
            f"""
            (selector, attr, val) => {{
                const element = document.querySelector(selector);
                return element && element.getAttribute(attr) === val;
            }}
            """,
            [selector, attribute, value],
            timeout=timeout,
        )

    @staticmethod
    def wait_for_element_text(
        page: Page, selector: str, text: str, exact: bool = False, timeout: int = 10000
    ) -> None:
        """Wait for element to contain or have exact text."""
        if exact:
            expect(page.locator(selector)).to_have_text(text, timeout=timeout)
        else:
            expect(page.locator(selector)).to_contain_text(text, timeout=timeout)

    @staticmethod
    def wait_for_element_count(
        page: Page, selector: str, count: int, timeout: int = 10000
    ) -> None:
        """Wait for specific number of elements to be present."""
        expect(page.locator(selector)).to_have_count(count, timeout=timeout)

    @staticmethod
    def wait_for_element_state(
        element: Locator, state: str = "visible", timeout: int = 10000
    ) -> None:
        """
        Wait for element to reach specific state.

        States: visible, hidden, stable, enabled, disabled, editable
        """
        element.wait_for(state=state, timeout=timeout)

    @staticmethod
    def wait_for_network_idle(page: Page, timeout: int = 30000) -> None:
        """Wait for network to be idle."""
        page.wait_for_load_state("networkidle", timeout=timeout)

    @staticmethod
    def wait_for_animation(page: Page, selector: str, timeout: int = 5000) -> None:
        """Wait for CSS animation to complete."""
        page.wait_for_function(
            """
            (selector) => {
                const element = document.querySelector(selector);
                if (!element) return false;

                const animations = element.getAnimations();
                return animations.length === 0 || 
                       animations.every(animation => animation.playState === 'finished');
            }
            """,
            selector,
            timeout=timeout,
        )

    @staticmethod
    def wait_for_value_change(
        page: Page, selector: str, initial_value: str, timeout: int = 10000
    ) -> str:
        """Wait for element value to change from initial value."""
        page.wait_for_function(
            """
            (selector, initialValue) => {
                const element = document.querySelector(selector);
                return element && element.value !== initialValue;
            }
            """,
            [selector, initial_value],
            timeout=timeout,
        )
        return page.locator(selector).input_value()

    @staticmethod
    def wait_for_download(page: Page, timeout: int = 30000) -> Any:
        """Wait for download to start and return download object."""
        with page.expect_download(timeout=timeout) as download_info:
            pass
        return download_info.value

    @staticmethod
    def wait_for_console_message(
        page: Page, message_pattern: str, timeout: int = 10000
    ) -> Optional[str]:
        """Wait for specific console message."""
        messages = []

        def handle_console(msg):
            if message_pattern in msg.text:
                messages.append(msg.text)

        page.on("console", handle_console)

        end_time = time.time() + (timeout / 1000)
        while time.time() < end_time and not messages:
            page.wait_for_timeout(100)

        page.remove_listener("console", handle_console)
        return messages[0] if messages else None

    @staticmethod
    def wait_for_url_change(page: Page, initial_url: str, timeout: int = 10000) -> str:
        """Wait for URL to change from initial URL."""
        page.wait_for_function(
            """
            (initialUrl) => window.location.href !== initialUrl
            """,
            initial_url,
            timeout=timeout,
        )
        return page.url

    @staticmethod
    def wait_for_element_clickable(
        page: Page, selector: str, timeout: int = 10000
    ) -> Locator:
        """Wait for element to be clickable (visible and enabled)."""
        element = page.locator(selector)
        element.wait_for(state="visible", timeout=timeout)
        element.wait_for(state="enabled", timeout=timeout)
        return element

    @staticmethod
    def wait_for_all_images_loaded(page: Page, timeout: int = 30000) -> None:
        """Wait for all images on page to be loaded."""
        page.wait_for_function(
            """
            () => {
                const images = Array.from(document.querySelectorAll('img'));
                return images.every(img => img.complete && img.naturalHeight !== 0);
            }
            """,
            timeout=timeout,
        )

    @staticmethod
    def wait_for_ajax_complete(page: Page, timeout: int = 30000) -> None:
        """Wait for jQuery AJAX requests to complete (if jQuery is present)."""
        page.wait_for_function(
            """
            () => {
                if (typeof jQuery === 'undefined') return true;
                return jQuery.active === 0;
            }
            """,
            timeout=timeout,
        )


# Convenience function for direct import
wait_for = WaitHelpers.wait_for_condition
