"""Modal dialog components for BUSY Bar."""

from typing import List, Optional

import allure
from pages.base_page import BasePage
from playwright.sync_api import Locator, Page


class BaseModal(BasePage):
    """Base modal dialog component."""

    # Modal container selectors (multiple fallbacks)
    MODAL = "[role='dialog'], .modal, .dialog, [data-modal], .overlay"
    MODAL_BACKDROP = ".modal-backdrop, .overlay, .backdrop"

    # Modal content selectors
    MODAL_CONTENT = "[role='dialog'] .modal-content, .dialog-content, .modal-body"
    MODAL_TITLE = "[role='dialog'] h1, [role='dialog'] h2, [role='dialog'] h3, .modal-title, .dialog-title"
    MODAL_HEADER = ".modal-header, .dialog-header"
    MODAL_FOOTER = ".modal-footer, .dialog-footer"

    # Action buttons (multiple text variations and icons)
    CLOSE_BUTTON = "[role='dialog'] button[aria-label='Close'], [role='dialog'] .close, button:has-text('Close'), button:has(.iconify.i-tabler\\:x), button:has(.iconify.i-tabler\\:close)"
    CANCEL_BUTTON = "button:has-text('Cancel'), button:has-text('cancel')"
    SAVE_BUTTON = "button:has-text('Save'), button:has-text('save'), button:has(.iconify.i-tabler\\:device-floppy)"
    APPLY_BUTTON = "button:has-text('Apply'), button:has-text('apply')"
    OK_BUTTON = "button:has-text('OK'), button:has-text('ok'), button:has(.iconify.i-tabler\\:check)"
    CONFIRM_BUTTON = "button:has-text('Confirm'), button:has-text('confirm')"

    def __init__(self, page: Page):
        super().__init__(page)

    @allure.step("Wait for modal to appear")
    def wait_for_modal(self, timeout: Optional[int] = None) -> None:
        """Wait for modal to be visible."""
        from config.settings import settings

        wait_timeout = timeout or (settings.modal_timeout * 1000)  # Convert to ms
        self.wait_for_element(self.MODAL, timeout=wait_timeout)

    @allure.step("Close modal")
    def close(self) -> None:
        """Close the modal."""
        if self.is_visible(self.CLOSE_BUTTON):
            self.click(self.CLOSE_BUTTON)
        elif self.is_visible(self.CANCEL_BUTTON):
            self.click(self.CANCEL_BUTTON)
        else:
            self.page.keyboard.press("Escape")
        self.wait_for_element(self.MODAL, state="hidden")

    @allure.step("Get modal title")
    def get_title(self) -> str:
        """Get modal title text."""
        return self.get_text(self.MODAL_TITLE)

    @allure.step("Save changes")
    def save(self) -> None:
        """Save modal changes."""
        if self.is_visible(self.SAVE_BUTTON):
            self.click(self.SAVE_BUTTON)
        elif self.is_visible(self.APPLY_BUTTON):
            self.click(self.APPLY_BUTTON)


class WiFiModal(BaseModal):
    """WiFi configuration modal."""

    # WiFi specific selectors - multiple fallbacks for better compatibility
    WIFI_TOGGLE = "[role='dialog'] [role='switch'], [role='dialog'] input[type='checkbox'], .wifi-toggle, input[name='wifi-enabled']"

    # Network input fields
    SSID_INPUT = "input[placeholder*='SSID'], input[name='ssid'], input[placeholder*='network'], input[placeholder*='Network']"
    PASSWORD_INPUT = (
        "input[type='password'], input[name='password'], input[placeholder*='Password']"
    )

    # Network list and items
    NETWORK_LIST = "[role='dialog'] [role='list'], [role='dialog'] .network-list, .wifi-networks, ul"
    NETWORK_ITEM = "[role='listitem'], .network-item, .wifi-network, li"

    # Action buttons
    SCAN_BUTTON = (
        "button:has-text('Scan'), button:has-text('scan'), button[aria-label*='Scan']"
    )
    CONNECT_BUTTON = "button:has-text('Connect'), button:has-text('connect'), button[aria-label*='Connect']"
    FORGET_BUTTON = "button:has-text('Forget'), button:has-text('forget'), button:has-text('Remove')"
    DISCONNECT_BUTTON = "button:has-text('Disconnect'), button:has-text('disconnect')"

    # Configuration options
    SECURITY_DROPDOWN = (
        "select[name='security'], select[name='encryption'], .security-select"
    )
    SECURITY_TYPE = ".security-type, .encryption-type"
    SHOW_PASSWORD = (
        "button[aria-label*='Show'], button[aria-label*='password'], .show-password"
    )

    # Status indicators
    WIFI_STATUS = ".wifi-status, .connection-status"
    SIGNAL_STRENGTH = ".signal-strength, .rssi"
    CONNECTION_STATE = ".connection-state, .wifi-state"

    @allure.step("Enable WiFi")
    def enable_wifi(self) -> None:
        """Enable WiFi if disabled."""
        toggle = self.page.locator(self.WIFI_TOGGLE)
        if toggle.get_attribute("aria-checked") == "false":
            toggle.click()

    @allure.step("Disable WiFi")
    def disable_wifi(self) -> None:
        """Disable WiFi if enabled."""
        toggle = self.page.locator(self.WIFI_TOGGLE)
        if toggle.get_attribute("aria-checked") == "true":
            toggle.click()

    @allure.step("Scan for networks")
    def scan_networks(self) -> None:
        """Scan for available WiFi networks."""
        self.click(self.SCAN_BUTTON)
        self.page.wait_for_timeout(2000)  # Wait for scan

    @allure.step("Get available networks")
    def get_available_networks(self) -> List[str]:
        """Get list of available WiFi networks."""
        if not self.is_visible(self.NETWORK_LIST):
            return []

        networks = self.page.locator(self.NETWORK_ITEM).all()
        return [net.text_content() for net in networks if net.text_content()]

    @allure.step("Select network: {ssid}")
    def select_network(self, ssid: str) -> None:
        """Select a network from the list."""
        network = self.page.locator(f"{self.NETWORK_ITEM}:has-text('{ssid}')")
        network.click()

    @allure.step("Connect to network")
    def connect_to_network(
        self, ssid: str, password: str, security: str = "WPA2"
    ) -> None:
        """Connect to a WiFi network."""
        # Enter SSID
        self.fill(self.SSID_INPUT, ssid)

        # Select security type if dropdown exists
        if self.is_visible(self.SECURITY_DROPDOWN):
            self.select_option(self.SECURITY_DROPDOWN, security)

        # Enter password
        self.fill(self.PASSWORD_INPUT, password)

        # Click connect
        self.click(self.CONNECT_BUTTON)

        # Wait for connection
        self.page.wait_for_timeout(3000)

    @allure.step("Forget network")
    def forget_network(self) -> None:
        """Forget current network."""
        if self.is_visible(self.FORGET_BUTTON):
            self.click(self.FORGET_BUTTON)

    @allure.step("Toggle password visibility")
    def toggle_password_visibility(self) -> None:
        """Toggle password field visibility."""
        if self.is_visible(self.SHOW_PASSWORD):
            self.click(self.SHOW_PASSWORD)


class BluetoothModal(BaseModal):
    """Bluetooth configuration modal."""

    # Bluetooth specific selectors - multiple fallbacks
    BLUETOOTH_TOGGLE = "[role='dialog'] [role='switch'], [role='dialog'] input[type='checkbox'], .bluetooth-toggle, input[name='bluetooth-enabled']"

    # Device list and items
    DEVICE_LIST = "[role='dialog'] [role='list'], [role='dialog'] .device-list, .bluetooth-devices, ul"
    DEVICE_ITEM = "[role='listitem'], .device-item, .bluetooth-device, li"

    # Action buttons
    SCAN_BUTTON = (
        "button:has-text('Scan'), button:has-text('scan'), button[aria-label*='Scan']"
    )
    PAIR_BUTTON = (
        "button:has-text('Pair'), button:has-text('pair'), button[aria-label*='Pair']"
    )
    UNPAIR_BUTTON = "button:has-text('Unpair'), button:has-text('Forget'), button:has-text('Remove')"
    CONNECT_BUTTON = "button:has-text('Connect'), button:has-text('connect')"
    DISCONNECT_BUTTON = "button:has-text('Disconnect'), button:has-text('disconnect')"

    # Input fields
    DEVICE_NAME_INPUT = "input[name='deviceName'], input[placeholder*='Device'], input[placeholder*='Name']"
    PIN_INPUT = (
        "input[name='pin'], input[placeholder*='PIN'], input[placeholder*='Code']"
    )

    # Settings and options
    DISCOVERABLE_TOGGLE = "label:has-text('Discoverable') [role='switch'], .discoverable-toggle, input[name='discoverable']"

    # Status indicators
    BLUETOOTH_STATUS = ".bluetooth-status, .connection-status"
    DEVICE_TYPE = ".device-type"
    CONNECTION_STATE = ".connection-state, .bluetooth-state"
    SIGNAL_STRENGTH = ".signal-strength"

    @allure.step("Enable Bluetooth")
    def enable_bluetooth(self) -> None:
        """Enable Bluetooth if disabled."""
        toggle = self.page.locator(self.BLUETOOTH_TOGGLE)
        if toggle.get_attribute("aria-checked") == "false":
            toggle.click()

    @allure.step("Disable Bluetooth")
    def disable_bluetooth(self) -> None:
        """Disable Bluetooth if enabled."""
        toggle = self.page.locator(self.BLUETOOTH_TOGGLE)
        if toggle.get_attribute("aria-checked") == "true":
            toggle.click()

    @allure.step("Scan for devices")
    def scan_devices(self) -> None:
        """Scan for available Bluetooth devices."""
        self.click(self.SCAN_BUTTON)
        self.page.wait_for_timeout(3000)  # Wait for scan

    @allure.step("Get available devices")
    def get_available_devices(self) -> List[str]:
        """Get list of available Bluetooth devices."""
        if not self.is_visible(self.DEVICE_LIST):
            return []

        devices = self.page.locator(self.DEVICE_ITEM).all()
        return [dev.text_content() for dev in devices if dev.text_content()]

    @allure.step("Pair with device: {device_name}")
    def pair_device(self, device_name: str, pin: Optional[str] = None) -> None:
        """Pair with a Bluetooth device."""
        # Select device
        device = self.page.locator(f"{self.DEVICE_ITEM}:has-text('{device_name}')")
        device.click()

        # Click pair
        self.click(self.PAIR_BUTTON)

        # Enter PIN if required
        if pin and self.is_visible(self.PIN_INPUT):
            self.fill(self.PIN_INPUT, pin)
            self.page.keyboard.press("Enter")

        # Wait for pairing
        self.page.wait_for_timeout(3000)

    @allure.step("Unpair device")
    def unpair_device(self) -> None:
        """Unpair current device."""
        if self.is_visible(self.UNPAIR_BUTTON):
            self.click(self.UNPAIR_BUTTON)

    @allure.step("Set device as discoverable")
    def set_discoverable(self, discoverable: bool = True) -> None:
        """Set device discoverable mode."""
        toggle = self.page.locator(self.DISCOVERABLE_TOGGLE)
        is_discoverable = toggle.get_attribute("aria-checked") == "true"

        if discoverable != is_discoverable:
            toggle.click()


class FirmwareUpdateModal(BaseModal):
    """Firmware update modal."""

    # Firmware specific selectors - multiple fallbacks
    FILE_INPUT = "input[type='file'], input[accept*='.bin'], input[accept*='.hex'], input[accept*='firmware']"
    FILE_DROPZONE = (
        "[role='dialog'] .dropzone, [data-dropzone], .file-drop, .upload-area"
    )
    FILE_SELECTOR = ".file-selector, .file-chooser"

    # Action buttons
    BROWSE_BUTTON = "button:has-text('Browse'), button:has-text('Choose'), button:has-text('Select')"
    UPLOAD_BUTTON = "button:has-text('Upload'), button:has-text('upload')"
    UPDATE_BUTTON = "button:has-text('Update'), button:has-text('Install'), button:has-text('Flash')"
    START_BUTTON = "button:has-text('Start'), button:has-text('Begin')"

    # Progress and status
    PROGRESS_BAR = "[role='progressbar'], .progress-bar, .progress, progress"
    PROGRESS_TEXT = ".progress-text, .progress-status"
    STATUS_MESSAGE = ".status-message, .update-status"

    # Information sections
    VERSION_INFO = ".firmware-version, .version-info, .current-version"
    NEW_VERSION_INFO = ".new-version, .target-version"
    RELEASE_NOTES = ".release-notes, .changelog, .update-notes"
    FILE_INFO = ".file-info, .firmware-info"

    # Options and checkboxes
    VERIFY_CHECKBOX = "input[type='checkbox'][name='verify'], input[name='verification'], .verify-option"
    BACKUP_CHECKBOX = "input[type='checkbox'][name='backup'], input[name='backup-firmware'], .backup-option"
    FORCE_UPDATE_CHECKBOX = "input[name='force'], .force-update"

    # Warnings and confirmations
    WARNING_MESSAGE = ".warning, .alert-warning, .caution"
    CONFIRMATION_DIALOG = ".confirmation, .confirm-update"

    @allure.step("Select firmware file: {file_path}")
    def select_firmware_file(self, file_path: str) -> None:
        """Select firmware file for upload."""
        self.page.set_input_files(self.FILE_INPUT, file_path)

    @allure.step("Drag and drop firmware file")
    def drag_drop_firmware(self, file_path: str) -> None:
        """Drag and drop firmware file to dropzone."""
        # Create file chooser
        with self.page.expect_file_chooser() as fc_info:
            self.page.locator(self.FILE_DROPZONE).click()
        file_chooser = fc_info.value
        file_chooser.set_files(file_path)

    @allure.step("Upload firmware")
    def upload_firmware(self, file_path: str) -> None:
        """Upload firmware file."""
        self.select_firmware_file(file_path)

        if self.is_visible(self.UPLOAD_BUTTON):
            self.click(self.UPLOAD_BUTTON)

        # Wait for upload
        self.wait_for_element(self.PROGRESS_BAR, timeout=10000)

    @allure.step("Start firmware update")
    def start_update(self, verify: bool = True, backup: bool = True) -> None:
        """Start firmware update process."""
        # Set verification option
        if self.is_visible(self.VERIFY_CHECKBOX):
            if verify:
                self.check(self.VERIFY_CHECKBOX)
            else:
                self.uncheck(self.VERIFY_CHECKBOX)

        # Set backup option
        if self.is_visible(self.BACKUP_CHECKBOX):
            if backup:
                self.check(self.BACKUP_CHECKBOX)
            else:
                self.uncheck(self.BACKUP_CHECKBOX)

        # Start update
        self.click(self.UPDATE_BUTTON)

    @allure.step("Get update progress")
    def get_progress(self) -> int:
        """Get firmware update progress percentage."""
        if not self.is_visible(self.PROGRESS_BAR):
            return 0

        progress = self.page.locator(self.PROGRESS_BAR)
        value = progress.get_attribute("aria-valuenow")
        return int(value) if value else 0

    @allure.step("Wait for update completion")
    def wait_for_update_complete(self, timeout: int = 300000) -> None:
        """Wait for firmware update to complete."""
        self.page.wait_for_function(
            """
            () => {
                const progress = document.querySelector('[role="progressbar"]');
                return !progress || progress.getAttribute('aria-valuenow') === '100';
            }
            """,
            timeout=timeout,
        )

    @allure.step("Get version info")
    def get_version_info(self) -> str:
        """Get firmware version information."""
        if self.is_visible(self.VERSION_INFO):
            return self.get_text(self.VERSION_INFO)
        return ""

    @allure.step("Get release notes")
    def get_release_notes(self) -> str:
        """Get firmware release notes."""
        if self.is_visible(self.RELEASE_NOTES):
            return self.get_text(self.RELEASE_NOTES)
        return ""
