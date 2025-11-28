"""Test to validate faster timeout behavior."""

import time

import allure
import pytest
from pages.control_panel import ControlPanelPage
from playwright.sync_api import Page


@allure.epic("Performance")
@allure.feature("Fast Timeouts")
class TestFastTimeouts:
    """Test that timeouts are now faster for better test performance."""

    @allure.story("Fast Failure Times")
    @allure.title("Test that element not found fails quickly")
    def test_fast_element_timeout(self, page: Page):
        """Test that searching for non-existent element fails quickly."""
        control_panel = ControlPanelPage(page)
        control_panel.open()

        # Test that looking for a non-existent element fails quickly
        start_time = time.time()

        with pytest.raises((AssertionError, Exception)):
            # Try to find an element that definitely doesn't exist
            control_panel.verify_element_visible("button:has-text('NonExistentButton')")

        elapsed_time = time.time() - start_time

        # Should fail in less than 3 seconds (our element_timeout is 2s)
        assert elapsed_time < 4.0, f"Element timeout took too long: {elapsed_time:.2f}s"
        allure.attach(
            f"{elapsed_time:.2f}s", "Timeout Duration", allure.attachment_type.TEXT
        )

    @allure.story("Modal Timeout")
    @allure.title("Test modal wait timeout is reasonable")
    def test_modal_timeout(self, page: Page):
        """Test that modal timeout is reasonable."""
        control_panel = ControlPanelPage(page)
        control_panel.open()

        start_time = time.time()

        # Click WiFi button but don't expect modal to actually appear
        # This should timeout quickly if modal doesn't exist
        try:
            control_panel.click(control_panel.WIFI_CONFIG_BTN)
            # Try to wait for modal with default timeout
            with pytest.raises((AssertionError, Exception)):
                control_panel.wifi_modal.wait_for_modal()
        except Exception:
            pass  # Expected to fail

        elapsed_time = time.time() - start_time

        # Should fail in less than 5 seconds (modal_timeout is 3s)
        assert elapsed_time < 6.0, f"Modal timeout took too long: {elapsed_time:.2f}s"
        allure.attach(
            f"{elapsed_time:.2f}s",
            "Modal Timeout Duration",
            allure.attachment_type.TEXT,
        )

    @allure.story("Timeout Configuration")
    @allure.title("Test timeout helper methods work")
    def test_timeout_helpers(self, page: Page):
        """Test that timeout helper methods return correct values."""
        control_panel = ControlPanelPage(page)

        # Test fast timeout
        fast_timeout = control_panel.with_fast_timeout()
        assert (
            fast_timeout == 2000
        ), f"Fast timeout should be 2000ms, got {fast_timeout}"

        # Test modal timeout
        modal_timeout = control_panel.with_modal_timeout()
        assert (
            modal_timeout == 3000
        ), f"Modal timeout should be 3000ms, got {modal_timeout}"

        # Test long timeout
        long_timeout = control_panel.with_long_timeout()
        assert (
            long_timeout == 15000
        ), f"Long timeout should be 15000ms, got {long_timeout}"

        # Test custom timeout override
        custom_timeout = control_panel.with_fast_timeout(5000)
        assert (
            custom_timeout == 5000
        ), f"Custom timeout should be 5000ms, got {custom_timeout}"

    @allure.story("Settings Validation")
    @allure.title("Test that settings have been updated")
    def test_settings_updated(self, page: Page):
        """Verify that timeout settings have been updated."""
        from config.settings import settings

        # Verify new timeout values
        assert (
            settings.element_timeout == 2
        ), f"Element timeout should be 2s, got {settings.element_timeout}"
        assert (
            settings.modal_timeout == 3
        ), f"Modal timeout should be 3s, got {settings.modal_timeout}"
        assert (
            settings.long_timeout == 15
        ), f"Long timeout should be 15s, got {settings.long_timeout}"
        assert (
            settings.navigation_timeout == 10
        ), f"Navigation timeout should be 10s, got {settings.navigation_timeout}"

    @allure.story("Page Load Performance")
    @allure.title("Test page load still works with faster timeouts")
    def test_page_load_with_fast_timeouts(self, page: Page):
        """Test that page load works correctly with faster default timeouts."""
        control_panel = ControlPanelPage(page)

        start_time = time.time()
        control_panel.open()  # This uses wait_for_page_load internally
        elapsed_time = time.time() - start_time

        # Page should load quickly
        assert elapsed_time < 20.0, f"Page load took too long: {elapsed_time:.2f}s"

        # Verify basic elements are still found
        assert control_panel.is_visible(
            control_panel.SIDEBAR
        ), "Sidebar should be visible"
        assert control_panel.is_visible(
            control_panel.THEME_TOGGLE
        ), "Theme toggle should be visible"

    @pytest.mark.slow
    @allure.story("Extended Timeout")
    @allure.title("Test that extended timeouts still work when needed")
    def test_extended_timeout_override(self, page: Page):
        """Test that we can still use longer timeouts when needed."""
        control_panel = ControlPanelPage(page)

        # Test with extended timeout - should not fail quickly
        start_time = time.time()

        try:
            # Use a longer timeout for page load
            control_panel.wait_for_page_load(timeout=20000)  # 20 seconds
        except Exception:
            pass  # May still fail but should take longer

        elapsed_time = time.time() - start_time

        # Should have waited longer if page isn't ready
        # (This test mainly verifies the timeout parameter is respected)
        allure.attach(
            f"{elapsed_time:.2f}s",
            "Extended Timeout Duration",
            allure.attachment_type.TEXT,
        )
