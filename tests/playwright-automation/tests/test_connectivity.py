"""Tests for BUSY Bar connectivity and basic functionality."""

import allure
import pytest
from pages.control_panel import ControlPanelPage
from playwright.sync_api import Page


@allure.feature("Device Connectivity")
@allure.story("USB Connection")
class TestDeviceConnectivity:
    """Test suite for device connectivity."""

    @pytest.mark.smoke
    @pytest.mark.critical
    @allure.title("Verify device connects via USB")
    @allure.description(
        "Test that BUSY Bar device is accessible via USB-Ethernet connection"
    )
    def test_device_usb_connection(self, page: Page):
        """Test USB connection to device."""
        with allure.step("Navigate to control panel"):
            control_panel = ControlPanelPage(page)
            control_panel.open()

        with allure.step("Verify device is connected"):
            control_panel.verify_device_connected()

        with allure.step("Verify connection type is USB"):
            connection_type = control_panel.get_connection_type()
            assert (
                connection_type == "USB"
            ), f"Expected USB connection, got {connection_type}"

    @pytest.mark.smoke
    @allure.title("Verify control panel loads successfully")
    @allure.description("Test that all control panel elements load correctly")
    def test_control_panel_loads(self, page: Page):
        """Test control panel page loads with all elements."""
        control_panel = ControlPanelPage(page)

        with allure.step("Open control panel"):
            control_panel.open()

        with allure.step("Verify all main elements are present"):
            control_panel.verify_page_loaded()

    @pytest.mark.regression
    @allure.title("Verify firmware version is displayed")
    @allure.description("Test that firmware version is correctly displayed")
    def test_firmware_version_displayed(self, device_connected):
        """Test firmware version display."""
        control_panel = device_connected

        with allure.step("Get firmware version"):
            firmware_version = control_panel.get_firmware_version()

        with allure.step("Verify firmware version format"):
            assert firmware_version, "Firmware version is empty"
            # You can add more specific version format validation
            allure.attach(
                firmware_version, "Firmware Version", allure.attachment_type.TEXT
            )

    @pytest.mark.smoke
    @allure.title("Verify device display canvas renders")
    @allure.description("Test that device display canvas is rendered correctly")
    def test_device_display_renders(self, device_connected):
        """Test device display rendering."""
        control_panel = device_connected

        with allure.step("Verify display is visible"):
            assert (
                control_panel.is_device_display_visible()
            ), "Device display not visible"

        with allure.step("Get display dimensions"):
            width, height = control_panel.get_display_dimensions()
            assert width == 360, f"Expected width 360, got {width}"
            assert height == 80, f"Expected height 80, got {height}"

        with allure.step("Capture display screenshot"):
            display_image = control_panel.capture_display()
            allure.attach(display_image, "Device Display", allure.attachment_type.PNG)


@allure.feature("UI Navigation")
@allure.story("Theme Management")
class TestThemeManagement:
    """Test suite for theme switching functionality."""

    @pytest.mark.regression
    @allure.title("Verify theme toggle functionality")
    @allure.description("Test that theme can be toggled between light and dark modes")
    def test_theme_toggle(self, device_connected):
        """Test theme toggle functionality."""
        control_panel = device_connected

        with allure.step("Get initial theme"):
            initial_theme = control_panel.get_current_theme()
            allure.attach(initial_theme, "Initial Theme", allure.attachment_type.TEXT)

        with allure.step("Toggle theme"):
            control_panel.toggle_theme()

        with allure.step("Verify theme changed"):
            new_theme = control_panel.get_current_theme()
            expected_theme = "light" if initial_theme == "dark" else "dark"
            assert (
                new_theme == expected_theme
            ), f"Expected {expected_theme}, got {new_theme}"

        with allure.step("Toggle back to original"):
            control_panel.toggle_theme()
            final_theme = control_panel.get_current_theme()
            assert final_theme == initial_theme, "Theme not restored to original"

    @pytest.mark.regression
    @allure.title("Verify theme persistence")
    @allure.description("Test that theme selection persists across page reloads")
    def test_theme_persistence(self, device_connected):
        """Test theme persistence across reloads."""
        control_panel = device_connected

        with allure.step("Set theme to dark"):
            control_panel.set_theme("dark")
            assert control_panel.get_current_theme() == "dark"

        with allure.step("Reload page"):
            control_panel.reload()
            control_panel.wait_for_page_load()

        with allure.step("Verify theme persisted"):
            current_theme = control_panel.get_current_theme()
            assert current_theme == "dark", f"Theme not persisted, got {current_theme}"


@allure.feature("Network Configuration")
@allure.story("Connection Status")
class TestConnectionStatus:
    """Test suite for network connection status."""

    @pytest.mark.smoke
    @allure.title("Verify WiFi status display")
    @allure.description("Test that WiFi status is correctly displayed")
    def test_wifi_status_display(self, device_connected):
        """Test WiFi status display."""
        control_panel = device_connected

        with allure.step("Get WiFi status"):
            wifi_status = control_panel.get_wifi_status()
            is_enabled = control_panel.is_wifi_enabled()

        with allure.step("Verify status consistency"):
            if is_enabled:
                assert (
                    wifi_status.lower() == "on"
                ), f"WiFi enabled but status shows {wifi_status}"
            else:
                assert (
                    wifi_status.lower() == "off"
                ), f"WiFi disabled but status shows {wifi_status}"

    @pytest.mark.smoke
    @allure.title("Verify Bluetooth status display")
    @allure.description("Test that Bluetooth status is correctly displayed")
    def test_bluetooth_status_display(self, device_connected):
        """Test Bluetooth status display."""
        control_panel = device_connected

        with allure.step("Get Bluetooth status"):
            bt_status = control_panel.get_bluetooth_status()
            is_enabled = control_panel.is_bluetooth_enabled()

        with allure.step("Verify status consistency"):
            if is_enabled:
                assert (
                    bt_status.lower() == "on"
                ), f"Bluetooth enabled but status shows {bt_status}"
            else:
                assert (
                    bt_status.lower() == "off"
                ), f"Bluetooth disabled but status shows {bt_status}"


@allure.feature("Cloud Integration")
@allure.story("Cloud Login")
class TestCloudIntegration:
    """Test suite for BUSY Cloud integration."""

    @pytest.mark.regression
    @allure.title("Verify Cloud login button functionality")
    @allure.description("Test that Cloud login button opens correct URL in new tab")
    def test_cloud_login_button(self, device_connected):
        """Test Cloud login button."""
        control_panel = device_connected

        with allure.step("Click Cloud login button"):
            new_page = control_panel.login_to_cloud()

        with allure.step("Verify new tab opened with correct URL"):
            assert new_page.url.startswith(
                "https://cloud.busy.app"
            ), f"Unexpected URL: {new_page.url}"
            new_page.close()
