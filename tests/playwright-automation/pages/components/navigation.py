"""Navigation sidebar component for BUSY Bar."""

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


from typing import List, Optional

from pages.base_page import BasePage
from playwright.sync_api import Locator, Page


class NavigationComponent(BasePage):
    """Navigation sidebar component."""

    # Selectors (Updated for new DOM structure)
    NAV_CONTAINER = "nav.relative.h-12.flex.justify-between.items-center, nav"
    SIDEBAR = "nav.relative.h-12.flex.justify-between.items-center, nav"  # Alias for consistency

    # Brand/Title - Updated selectors
    DEVICE_TITLE = (
        ".text-xl:has-text('BUSY Bar'), div:has-text('BUSY Bar'), text=BUSY Bar"
    )
    LOGO_AREA = ".iconify.i-busy\\:bar-logo, nav .text-xl, nav .brand"
    DEVICE_LOGO = ".iconify.i-busy\\:bar-logo"

    # Connection Status - Updated for new structure
    CONNECTION_STATUS = (
        ".flex.items-center.gap-1, .flex.items-center.gap-2, .connection-status"
    )
    CONNECTION_ICON = ".iconify.i-ri\\:usb-line, .i-ri\\:usb-line"
    USB_INDICATOR = ".iconify.i-ri\\:usb-line, .i-ri\\:usb-line"
    CONNECTED_TEXT = "text=Connected, div:has-text('Connected')"
    DISCONNECTED_TEXT = "text=Disconnected, div:has-text('Disconnected')"
    IP_ADDRESS = "nav .text-muted:first-of-type, nav div.text-muted:first-of-type"

    # Battery Status - New elements
    BATTERY_INDICATOR = (
        ".iconify.i-busy\\:battery-charging, .iconify.i-busy\\:charging-lightning"
    )
    BATTERY_PERCENTAGE = "nav div:has-text('%') >> nth=-1"

    # Firmware Info
    FIRMWARE_VERSION_LABEL = "text=Firmware Version"
    FIRMWARE_VERSION_VALUE = "[data-testid='firmware-version'], .firmware-version"

    # Theme Toggle (May not exist in current version)
    THEME_TOGGLE = "button[role='switch'], button[aria-label*='theme'], button[aria-label*='Theme']"
    THEME_TOGGLE_MOON = ".iconify.i-tabler\\:moon-filled, .i-tabler\\:moon-filled"
    THEME_TOGGLE_SUN = ".iconify.i-tabler\\:sun-filled, .i-tabler\\:sun-filled"

    # User Menu
    USER_MENU_BTN = (
        "button[id*='reka-dropdown-menu-trigger'], .iconify.i-busy\\:user-fill"
    )

    # Device info section
    DEVICE_INFO_SECTION = ".ring-default, .device-info, nav .border"
    DEVICE_IMAGE = ".device-image img, nav img"

    def __init__(self, page: Page):
        super().__init__(page)

    @allure.step("Check if navigation is visible")
    def is_navigation_visible(self) -> bool:
        """Check if navigation sidebar is visible."""
        return self.is_visible(self.NAV_CONTAINER)

    @allure.step("Get device title")
    def get_device_title(self) -> str:
        """Get device title from navigation."""
        return self.get_text(self.DEVICE_TITLE)

    @allure.step("Get connection status")
    def get_connection_status(self) -> str:
        """Get current connection status."""
        # Check for explicit Connected/Disconnected text
        if self.is_visible(self.CONNECTED_TEXT):
            return "Connected"
        elif self.is_visible(self.DISCONNECTED_TEXT):
            return "Disconnected"

        # Fallback - check connection status container
        if self.is_visible(self.CONNECTION_STATUS):
            status_text = self.get_text(self.CONNECTION_STATUS)
            if status_text:
                return status_text

        return "Unknown"

    @allure.step("Check if device is connected")
    def is_device_connected(self) -> bool:
        """Check if device shows as connected."""
        # Check for USB indicator and Connected text
        has_usb_indicator = self.is_visible(self.USB_INDICATOR)
        has_connected_text = self.is_visible(self.CONNECTED_TEXT)
        return has_usb_indicator and has_connected_text

    @allure.step("Get connection type")
    def get_connection_type(self) -> str:
        """Get connection type (USB/Bluetooth/WiFi)."""
        if self.is_visible(".iconify.i-ri\\:usb-line") or self.is_visible(
            ".i-ri\\:usb-line"
        ):
            return "USB"
        elif self.is_visible(".iconify.i-tabler\\:bluetooth") or self.is_visible(
            ".i-tabler\\:bluetooth"
        ):
            return "Bluetooth"
        elif self.is_visible(".iconify.i-tabler\\:wifi") or self.is_visible(
            ".i-tabler\\:wifi"
        ):
            return "WiFi"
        return "Unknown"

    @allure.step("Get firmware version")
    def get_firmware_version(self) -> str:
        """Get firmware version from navigation."""
        # Try specific firmware version selector first
        if self.is_visible(self.FIRMWARE_VERSION_VALUE):
            return self.get_text(self.FIRMWARE_VERSION_VALUE).strip()

        # Fallback - look for text after "Firmware Version" label
        try:
            firmware_elements = (
                self.page.locator("text=Firmware Version").locator("..").locator("span")
            )
            if firmware_elements.count() > 1:
                return firmware_elements.nth(-1).text_content().strip()
        except:
            pass

        return ""

    @allure.step("Toggle theme")
    def toggle_theme(self) -> None:
        """Toggle between light and dark theme."""
        self.click(self.THEME_TOGGLE)
        self.page.wait_for_timeout(300)  # Wait for animation

    @allure.step("Get theme toggle state")
    def get_theme_toggle_state(self) -> bool:
        """Get current theme toggle state."""
        if self.is_visible(self.THEME_TOGGLE):
            toggle = self.page.locator(self.THEME_TOGGLE)
            return toggle.get_attribute("aria-checked") == "true"
        return False

    @allure.step("Get IP address")
    def get_ip_address(self) -> str:
        """Get device IP address from navigation."""
        if self.is_visible(self.IP_ADDRESS):
            return self.get_text(self.IP_ADDRESS).strip()
        return ""

    @allure.step("Get battery percentage")
    def get_battery_percentage(self) -> str:
        """Get battery percentage from navigation."""
        if self.is_visible(self.BATTERY_PERCENTAGE):
            return self.get_text(self.BATTERY_PERCENTAGE).strip()
        return ""

    @allure.step("Check if battery is charging")
    def is_battery_charging(self) -> bool:
        """Check if battery is currently charging."""
        return self.is_visible(".iconify.i-busy\\:battery-charging") or self.is_visible(
            ".iconify.i-busy\\:charging-lightning"
        )

    @allure.step("Verify navigation elements")
    def verify_navigation_elements(self) -> None:
        """Verify all navigation elements are present."""
        assert self.is_navigation_visible(), "Navigation not visible"
        assert self.get_device_title() == "BUSY Bar", "Invalid device title"
        assert self.is_visible(self.THEME_TOGGLE), "Theme toggle not visible"

        # Verify connection status is shown
        status = self.get_connection_status()
        assert status, "Connection status not displayed"

    @allure.step("Wait for device connection")
    def wait_for_device_connection(self, timeout: Optional[int] = None) -> None:
        """Wait for device to connect."""
        from config.settings import settings

        wait_timeout = timeout or (
            settings.long_timeout * 1000
        )  # 15 seconds default in ms

        # Wait for Connected text and USB indicator
        self.page.wait_for_function(
            """
            () => {
                const connectedText = document.querySelector('*:has-text("Connected")');
                const usbIcon = document.querySelector('.i-tabler\\\\:usb');
                return connectedText && usbIcon;
            }
            """,
            timeout=wait_timeout,
        )

    @allure.step("Get navigation width")
    def get_navigation_width(self) -> int:
        """Get navigation sidebar width in pixels."""
        nav = self.page.locator(self.NAV_CONTAINER)
        box = nav.bounding_box()
        return int(box["width"]) if box else 0

    @allure.step("Check if navigation is collapsed")
    def is_navigation_collapsed(self) -> bool:
        """Check if navigation is in collapsed state."""
        nav = self.page.locator(self.NAV_CONTAINER)
        # Check if nav has hidden class or width is 0
        classes = nav.get_attribute("class") or ""
        return "hidden" in classes or self.get_navigation_width() == 0
