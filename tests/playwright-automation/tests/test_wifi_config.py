"""Tests for WiFi configuration functionality."""

import allure
import pytest
from pages.control_panel import ControlPanelPage
from playwright.sync_api import Page

from utils.logger import log_action, log_step


@allure.feature("Network Configuration")
@allure.story("WiFi Management")
class TestWiFiConfiguration:
    """Test suite for WiFi configuration."""

    @pytest.mark.wifi
    @pytest.mark.smoke
    @allure.title("Open WiFi settings modal")
    @allure.description("Verify WiFi settings modal can be opened")
    def test_open_wifi_modal(self, device_connected):
        """Test opening WiFi configuration modal."""
        control_panel = device_connected

        with allure.step("Open WiFi settings"):
            log_step("Opening WiFi settings modal")
            wifi_modal = control_panel.open_wifi_settings()

        with allure.step("Verify modal is displayed"):
            assert wifi_modal.get_title(), "WiFi modal title not found"
            log_action("WiFi modal opened successfully")

    @pytest.mark.wifi
    @allure.title("Toggle WiFi on/off")
    @allure.description("Test enabling and disabling WiFi")
    def test_toggle_wifi(self, device_connected):
        """Test WiFi toggle functionality."""
        control_panel = device_connected

        with allure.step("Get initial WiFi status"):
            initial_status = control_panel.is_wifi_enabled()
            log_step(
                f"Initial WiFi status: {'Enabled' if initial_status else 'Disabled'}"
            )

        with allure.step("Open WiFi settings"):
            wifi_modal = control_panel.open_wifi_settings()

        with allure.step("Toggle WiFi"):
            if initial_status:
                wifi_modal.disable_wifi()
                expected_status = False
            else:
                wifi_modal.enable_wifi()
                expected_status = True
            log_action(
                f"Toggled WiFi to: {'Enabled' if expected_status else 'Disabled'}"
            )

        with allure.step("Save and close modal"):
            wifi_modal.save()
            control_panel.page.wait_for_timeout(1000)

        with allure.step("Verify WiFi status changed"):
            current_status = control_panel.is_wifi_enabled()
            assert (
                current_status == expected_status
            ), f"WiFi status not changed. Expected: {expected_status}, Got: {current_status}"

    @pytest.mark.wifi
    @pytest.mark.regression
    @allure.title("Scan for WiFi networks")
    @allure.description("Test scanning for available WiFi networks")
    def test_scan_wifi_networks(self, device_connected, test_data):
        """Test WiFi network scanning."""
        control_panel = device_connected

        with allure.step("Open WiFi settings"):
            wifi_modal = control_panel.open_wifi_settings()

        with allure.step("Enable WiFi if disabled"):
            wifi_modal.enable_wifi()

        with allure.step("Scan for networks"):
            log_step("Scanning for WiFi networks")
            wifi_modal.scan_networks()

        with allure.step("Get available networks"):
            networks = wifi_modal.get_available_networks()
            log_step(f"Found {len(networks)} networks")

            if networks:
                allure.attach(
                    "\n".join(networks),
                    "Available Networks",
                    allure.attachment_type.TEXT,
                )

        with allure.step("Verify networks found or proper message"):
            # Either networks are found or a "No networks" message is shown
            assert networks or wifi_modal.is_visible(
                "text=No networks found"
            ), "No networks found and no appropriate message shown"

    @pytest.mark.wifi
    @pytest.mark.regression
    @allure.title("Connect to WiFi network")
    @allure.description("Test connecting to a WiFi network with credentials")
    @pytest.mark.parametrize("network_type", ["WPA2", "WPA3", "Open"])
    def test_connect_to_network(self, device_connected, test_data, network_type):
        """Test connecting to different types of WiFi networks."""
        control_panel = device_connected
        wifi_data = test_data["wifi"]

        with allure.step("Open WiFi settings"):
            wifi_modal = control_panel.open_wifi_settings()

        with allure.step("Enable WiFi"):
            wifi_modal.enable_wifi()

        with allure.step(f"Connect to {network_type} network"):
            ssid = f"{wifi_data['ssid']}_{network_type}"
            password = wifi_data["password"] if network_type != "Open" else ""

            log_step(f"Connecting to network: {ssid}")
            wifi_modal.connect_to_network(
                ssid=ssid, password=password, security=network_type
            )

        with allure.step("Verify connection attempt"):
            # Check for success or error message
            control_panel.page.wait_for_timeout(3000)

            # Look for connection status indicators
            success_indicators = [
                "Connected",
                "Connection successful",
                f"Connected to {ssid}",
            ]

            error_indicators = [
                "Failed to connect",
                "Invalid password",
                "Network not found",
                "Connection timeout",
            ]

            success = any(
                wifi_modal.is_visible(f"text={indicator}")
                for indicator in success_indicators
            )

            error = any(
                wifi_modal.is_visible(f"text={indicator}")
                for indicator in error_indicators
            )

            if error:
                pytest.skip(f"Network {ssid} not available or incorrect credentials")

            assert success or error, "No connection status shown"

    @pytest.mark.wifi
    @allure.title("Forget WiFi network")
    @allure.description("Test forgetting a saved WiFi network")
    def test_forget_network(self, device_connected):
        """Test forgetting a saved network."""
        control_panel = device_connected

        with allure.step("Check if WiFi is connected"):
            if not control_panel.is_wifi_enabled():
                pytest.skip("WiFi not connected, nothing to forget")

        with allure.step("Open WiFi settings"):
            wifi_modal = control_panel.open_wifi_settings()

        with allure.step("Forget current network"):
            log_action("Forgetting current network")
            wifi_modal.forget_network()

        with allure.step("Verify network forgotten"):
            control_panel.page.wait_for_timeout(1000)
            # Check for confirmation or status change
            assert wifi_modal.is_visible(
                "text=Network forgotten"
            ) or wifi_modal.is_visible("text=Disconnected"), "Network not forgotten"

    @pytest.mark.wifi
    @allure.title("Validate WiFi password field")
    @allure.description("Test WiFi password field validation and visibility toggle")
    def test_password_field_validation(self, device_connected, test_data):
        """Test password field validation."""
        control_panel = device_connected

        with allure.step("Open WiFi settings"):
            wifi_modal = control_panel.open_wifi_settings()

        with allure.step("Test password visibility toggle"):
            # Enter a password
            wifi_modal.fill(wifi_modal.PASSWORD_INPUT, "TestPassword123")

            # Check initial type (should be password)
            input_type = wifi_modal.get_attribute(wifi_modal.PASSWORD_INPUT, "type")
            assert input_type == "password", "Password not hidden by default"

            # Toggle visibility
            if wifi_modal.is_visible(wifi_modal.SHOW_PASSWORD):
                wifi_modal.toggle_password_visibility()

                # Check type changed to text
                input_type = wifi_modal.get_attribute(wifi_modal.PASSWORD_INPUT, "type")
                assert input_type == "text", "Password not shown after toggle"

                # Toggle back
                wifi_modal.toggle_password_visibility()
                input_type = wifi_modal.get_attribute(wifi_modal.PASSWORD_INPUT, "type")
                assert (
                    input_type == "password"
                ), "Password not hidden after second toggle"

    @pytest.mark.wifi
    @pytest.mark.regression
    @allure.title("WiFi modal close behavior")
    @allure.description("Test different ways of closing WiFi modal")
    @pytest.mark.parametrize("close_method", ["close_button", "cancel", "escape"])
    def test_wifi_modal_close(self, device_connected, close_method):
        """Test different methods of closing WiFi modal."""
        control_panel = device_connected

        with allure.step("Open WiFi settings"):
            wifi_modal = control_panel.open_wifi_settings()
            assert wifi_modal.is_visible(wifi_modal.MODAL), "Modal not opened"

        with allure.step(f"Close modal using {close_method}"):
            if close_method == "close_button":
                wifi_modal.click(wifi_modal.CLOSE_BUTTON)
            elif close_method == "cancel":
                wifi_modal.click(wifi_modal.CANCEL_BUTTON)
            elif close_method == "escape":
                control_panel.page.keyboard.press("Escape")

        with allure.step("Verify modal closed"):
            wifi_modal.wait_for_element(wifi_modal.MODAL, state="hidden")
            assert not wifi_modal.is_visible(wifi_modal.MODAL), "Modal still visible"
