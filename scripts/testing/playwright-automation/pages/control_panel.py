"""BUSY Bar Control Panel page object."""

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


from typing import Optional, Tuple

from config.settings import settings
from pages.base_page import BasePage
from pages.components.device_display import DeviceDisplayComponent
from pages.components.modals import (BluetoothModal, FirmwareUpdateModal,
                                     WiFiModal)
from pages.components.navigation import NavigationComponent
from playwright.sync_api import Locator, Page


class ControlPanelPage(BasePage):
    """Main control panel page for BUSY Bar device."""

    # === Selectors (Updated for new DOM structure) ===
    # Main Layout
    HTML_ELEMENT = "html"
    BODY_ELEMENT = "body"
    MAIN_CONTAINER = "div#__nuxt, main"

    # Navigation/Sidebar - Updated to match new structure
    SIDEBAR = "nav.relative.h-12.flex.justify-between.items-center, nav"
    LOGO_AREA = ".iconify.i-busy\\:bar-logo, nav .text-xl"

    # Device Status - Updated selectors
    DEVICE_STATUS_CONNECTED = "text=Connected, div:has-text('Connected')"
    DEVICE_STATUS_DISCONNECTED = "text=Disconnected, div:has-text('Disconnected')"
    USB_INDICATOR = ".iconify.i-ri\\:usb-line, .i-ri\\:usb-line"
    CONNECTION_STATUS = ".flex.items-center.gap-1, .flex.items-center.gap-2"
    IP_ADDRESS = ".text-muted, div.text-muted"
    FIRMWARE_VERSION_LABEL = "text=Firmware Version"
    FIRMWARE_VERSION_VALUE = "[data-testid='firmware-version'], .firmware-version"

    # Device Title and Battery
    DEVICE_TITLE = ".text-xl:has-text('BUSY Bar'), div:has-text('BUSY Bar')"
    BATTERY_INDICATOR = (
        ".iconify.i-busy\\:battery-charging, .iconify.i-busy\\:charging-lightning"
    )
    BATTERY_PERCENTAGE = "div:has-text('%'), span:has-text('%')"

    # Theme Toggle - May not exist in current version, keeping as fallback
    THEME_TOGGLE = "button[role='switch'], button[aria-label*='theme'], button[aria-label*='Theme']"
    THEME_TOGGLE_MOON = ".iconify.i-tabler\\:moon-filled, .i-tabler\\:moon-filled"
    THEME_TOGGLE_SUN = ".iconify.i-tabler\\:sun-filled, .i-tabler\\:sun-filled"
    DARK_MODE_INDICATOR = "html.dark"

    # User Menu Button - Updated
    USER_MENU_BTN = (
        "button[id*='reka-dropdown-menu-trigger'], .iconify.i-busy\\:user-fill"
    )

    # Main Content Area Action Buttons - These may be in different locations now
    UPDATE_FIRMWARE_BTN = "button:has-text('Update from file'), button >> text=Update"
    WIFI_CONFIG_BTN = "button:has(.iconify.i-tabler\\:wifi), button[aria-label*='WiFi'], button[aria-label*='Wi-Fi']"
    BLUETOOTH_CONFIG_BTN = (
        "button:has(.iconify.i-tabler\\:bluetooth), button[aria-label*='Bluetooth']"
    )
    REFRESH_BTN = (
        "button:has(.iconify.i-tabler\\:refresh), button[aria-label*='Refresh']"
    )

    # Cloud Integration
    CLOUD_LOGIN_BTN = (
        "a:has-text('Log in'), a[href*='cloud'], button:has(.iconify.i-tabler\\:login)"
    )

    # Connection Status Cards/Sections
    WIFI_SECTION = (
        "section:has(.iconify.i-tabler\\:wifi), div:has(.iconify.i-tabler\\:wifi)"
    )
    BLUETOOTH_SECTION = "section:has(.iconify.i-tabler\\:bluetooth), div:has(.iconify.i-tabler\\:bluetooth)"

    # Status Text Elements
    WIFI_STATUS_TEXT = ".iconify.i-tabler\\:wifi ~ span, [data-testid='wifi-status']"
    BLUETOOTH_STATUS_TEXT = (
        ".iconify.i-tabler\\:bluetooth ~ span, [data-testid='bluetooth-status']"
    )

    # Device Display - Updated for new structure
    DEVICE_DISPLAY_CONTAINER = (
        ".screen-stream-container, .device-image, div[data-v-ca2a5bb9]"
    )
    DEVICE_DISPLAY_IMAGE = ".device-image img, img[data-savepage-src*='busybar-device']"
    DEVICE_DISPLAY_CANVAS = "canvas"
    DEVICE_IMAGE = "img[src*='busybar-device'], .device-image img"

    def __init__(self, page: Page):
        super().__init__(page)
        # Initialize components
        self.navigation = NavigationComponent(page)
        self.device_display = DeviceDisplayComponent(page)
        self.wifi_modal = WiFiModal(page)
        self.bluetooth_modal = BluetoothModal(page)
        self.firmware_modal = FirmwareUpdateModal(page)

    # === Navigation ===

    @allure.step("Open BUSY Bar Control Panel")
    def open(self) -> None:
        """Navigate to control panel."""
        self.navigate_to()
        self.wait_for_page_load()

    def wait_for_page_load(self, timeout: Optional[int] = None) -> None:
        """Wait for page to be fully loaded."""
        # Use fast timeout for quick failures
        fast_timeout = 2000  # 2 seconds
        extended_timeout = timeout or (
            settings.long_timeout * 1000
        )  # 15 seconds default

        # Wait for the main Nuxt container to be present
        self.wait_for_element("div#__nuxt", timeout=extended_timeout)

        # Wait for either connected or disconnected status
        try:
            self.wait_for_element(self.DEVICE_STATUS_CONNECTED, timeout=fast_timeout)
        except:
            try:
                self.wait_for_element(
                    self.DEVICE_STATUS_DISCONNECTED, timeout=fast_timeout
                )
            except:
                # Fallback to sidebar being visible
                self.wait_for_element(self.SIDEBAR, timeout=extended_timeout)

        # Wait for device display or canvas to be rendered if present
        try:
            self.page.wait_for_function(
                "document.querySelector('canvas') !== null || document.querySelector('.device-image img') !== null",
                timeout=fast_timeout,
            )
        except:
            pass  # Display might not be present initially

    # === Device Status ===

    @allure.step("Check device connection status")
    def is_device_connected(self) -> bool:
        """Check if device is connected via USB."""
        # Check for USB indicator and Connected text
        has_usb_indicator = self.is_visible(self.USB_INDICATOR)
        has_connected_status = self.is_visible(self.DEVICE_STATUS_CONNECTED)
        return has_usb_indicator and has_connected_status

    @allure.step("Get firmware version")
    def get_firmware_version(self) -> str:
        """Get current firmware version."""
        # Try multiple selectors for firmware version
        if self.is_visible(self.FIRMWARE_VERSION_VALUE):
            return self.get_text(self.FIRMWARE_VERSION_VALUE).strip()
        # Fallback - look for text after "Firmware Version" label
        firmware_elements = (
            self.page.locator("text=Firmware Version").locator("..").locator("span")
        )
        if firmware_elements.count() > 1:
            return firmware_elements.nth(-1).text_content().strip()
        return ""

    @allure.step("Get device connection type")
    def get_connection_type(self) -> str:
        """Get current connection type (USB/Bluetooth/WiFi)."""
        if self.is_visible(self.USB_INDICATOR):
            return "USB"
        # Add logic for other connection types
        return "Unknown"

    # === Theme Management ===

    @allure.step("Toggle theme")
    def toggle_theme(self) -> None:
        """Toggle between light and dark theme."""
        self.click(self.THEME_TOGGLE)
        self.page.wait_for_timeout(500)  # Wait for animation

    @allure.step("Get current theme")
    def get_current_theme(self) -> str:
        """Get current theme (light/dark)."""
        html_element = self.page.locator("html")
        classes = html_element.get_attribute("class") or ""
        return "dark" if "dark" in classes else "light"

    @allure.step("Set theme to {theme}")
    def set_theme(self, theme: str) -> None:
        """Set specific theme."""
        current = self.get_current_theme()
        if current != theme:
            self.toggle_theme()

    # === WiFi Configuration ===

    @allure.step("Open WiFi settings")
    def open_wifi_settings(self, timeout: Optional[int] = None) -> WiFiModal:
        """Open WiFi configuration modal."""
        self.click(self.WIFI_CONFIG_BTN)
        self.wifi_modal.wait_for_modal(timeout=timeout)
        return self.wifi_modal

    @allure.step("Get WiFi status")
    def get_wifi_status(self) -> str:
        """Get current WiFi status."""
        if self.is_visible(self.WIFI_STATUS_TEXT):
            return self.get_text(self.WIFI_STATUS_TEXT)
        # Fallback - look in WiFi section for status text
        wifi_section = self.page.locator(self.WIFI_SECTION)
        status_text = wifi_section.locator("span, div").last.text_content()
        return status_text.strip() if status_text else "Unknown"

    @allure.step("Check if WiFi is enabled")
    def is_wifi_enabled(self) -> bool:
        """Check if WiFi is enabled."""
        status = self.get_wifi_status().lower()
        return "on" in status or "enabled" in status or "connected" in status

    # === Bluetooth Configuration ===

    @allure.step("Open Bluetooth settings")
    def open_bluetooth_settings(self, timeout: Optional[int] = None) -> BluetoothModal:
        """Open Bluetooth configuration modal."""
        self.click(self.BLUETOOTH_CONFIG_BTN)
        self.bluetooth_modal.wait_for_modal(timeout=timeout)
        return self.bluetooth_modal

    @allure.step("Get Bluetooth status")
    def get_bluetooth_status(self) -> str:
        """Get current Bluetooth status."""
        if self.is_visible(self.BLUETOOTH_STATUS_TEXT):
            return self.get_text(self.BLUETOOTH_STATUS_TEXT)
        # Fallback - look in Bluetooth section for status text
        bluetooth_section = self.page.locator(self.BLUETOOTH_SECTION)
        status_text = bluetooth_section.locator("span, div").last.text_content()
        return status_text.strip() if status_text else "Unknown"

    @allure.step("Check if Bluetooth is enabled")
    def is_bluetooth_enabled(self) -> bool:
        """Check if Bluetooth is enabled."""
        status = self.get_bluetooth_status().lower()
        return "on" in status or "enabled" in status or "connected" in status

    # === Firmware Update ===

    @allure.step("Open firmware update dialog")
    def open_firmware_update(
        self, timeout: Optional[int] = None
    ) -> FirmwareUpdateModal:
        """Open firmware update modal."""
        self.click(self.UPDATE_FIRMWARE_BTN)
        self.firmware_modal.wait_for_modal(timeout=timeout)
        return self.firmware_modal

    # === Cloud Integration ===

    @allure.step("Click login to BUSY Cloud")
    def login_to_cloud(self) -> None:
        """Navigate to BUSY Cloud login."""
        # This opens in new tab
        with self.page.expect_popup() as popup_info:
            self.click(self.CLOUD_LOGIN_BTN)
        return popup_info.value

    # === Device Display ===

    @allure.step("Check if device display is visible")
    def is_device_display_visible(self) -> bool:
        """Check if device display canvas or image is visible."""
        # Check for both canvas and device image
        return (
            self.device_display.is_display_visible()
            or self.is_visible(self.DEVICE_DISPLAY_IMAGE)
            or self.is_visible(self.DEVICE_DISPLAY_CONTAINER)
        )

    @allure.step("Get display dimensions")
    def get_display_dimensions(self) -> Tuple[int, int]:
        """Get device display dimensions."""
        return self.device_display.get_canvas_dimensions()

    @allure.step("Capture display screenshot")
    def capture_display(self) -> bytes:
        """Capture current device display."""
        return self.device_display.capture_display()

    # === Verification Methods ===

    @allure.step("Verify control panel loaded")
    def verify_page_loaded(self) -> None:
        """Verify control panel is fully loaded."""
        # Verify sidebar/navigation is visible
        self.verify_element_visible(self.SIDEBAR, "Navigation sidebar not visible")

        # Verify device status is shown (either connected or disconnected)
        status_visible = self.is_visible(
            self.DEVICE_STATUS_CONNECTED
        ) or self.is_visible(self.DEVICE_STATUS_DISCONNECTED)
        assert status_visible, "Device status not visible"

        # Verify key action buttons are visible
        self.verify_element_visible(
            self.UPDATE_FIRMWARE_BTN, "Firmware button not visible"
        )
        self.verify_element_visible(self.WIFI_CONFIG_BTN, "WiFi button not visible")
        self.verify_element_visible(
            self.BLUETOOTH_CONFIG_BTN, "Bluetooth button not visible"
        )

        # Verify theme toggle is present
        self.verify_element_visible(self.THEME_TOGGLE, "Theme toggle not visible")

        # Check device display if connected
        if self.is_device_connected():
            assert (
                self.is_device_display_visible()
            ), "Device display not visible when connected"

    @allure.step("Verify device connected")
    def verify_device_connected(self) -> None:
        """Verify device is connected."""
        assert self.is_device_connected(), "Device is not connected"
        firmware = self.get_firmware_version()
        assert firmware, f"Invalid firmware version: {firmware}"
        allure.attach(firmware, "Firmware Version", allure.attachment_type.TEXT)
