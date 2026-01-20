"""
Device firmware flasher utility for BSB firmware tests.

Handles flashing firmware and waiting for device recovery.
"""

import logging
import socket
import subprocess
import time

import allure

from config.config import config

logger = logging.getLogger("bsb_automation.device_flasher")

# Keep as module constants for backward compatibility, but source from config
DEVICE_IP = config.BUSYBAR_IP
DEVICE_CHECK_PORT = config.DEVICE_CHECK_PORT
FIRMWARE_PATH = config.FIRMWARE_ELF
PLATFORM_JSON = config.PLATFORM_JSON
TOOLCHAIN_ENV = config.TOOLCHAIN_ENV
FIRMWARE_DIR = config.BSB_FIRMWARE_PATH
OPENOCD_INTERFACE = config.OPENOCD_INTERFACE
OPENOCD_TARGET = config.OPENOCD_TARGET


class DeviceFlasher:
    """
    Handles firmware flashing and device recovery.

    Usage:
        flasher = DeviceFlasher()
        flasher.flash_and_wait()
    """

    def __init__(
        self,
        device_ip: str = None,
        firmware_path: str = None,
        platform_json: str = None,
        firmware_dir: str = None,
        serial: str = None,
    ):
        self.device_ip = device_ip or config.BUSYBAR_IP
        self.firmware_path = firmware_path or config.FIRMWARE_ELF
        self.platform_json = platform_json or config.PLATFORM_JSON
        self.firmware_dir = firmware_dir or config.BSB_FIRMWARE_PATH
        self.serial = serial or config.DAPLINK_U5_ID or "auto"

    def reset_and_wait(
        self,
        wait_timeout: float = 60.0,
        reset_interval: float = 5.0,
    ) -> bool:
        """
        Reset device and wait for it to recover, retrying reset periodically.

        Args:
            wait_timeout: Maximum time to wait for device recovery.
            reset_interval: Send reset every N seconds until device responds.

        Returns:
            True if device recovered successfully, False otherwise.
        """
        logger.info(f"Resetting device and waiting (reset every {reset_interval}s)...")
        start_time = time.time()
        last_reset_time = 0.0

        while time.time() - start_time < wait_timeout:
            if time.time() - last_reset_time >= reset_interval:
                self.reset_device()
                last_reset_time = time.time()

            # Check if device is available
            if self._check_device_available():
                elapsed = time.time() - start_time
                logger.info(f"Device is available after {elapsed:.1f}s")
                allure.attach(
                    f"Device recovered and available after {elapsed:.1f} seconds",
                    name="Device Recovery",
                    attachment_type=allure.attachment_type.TEXT,
                )
                return True

            time.sleep(0.5)

        logger.error(f"Device did not become available within {wait_timeout}s")
        allure.attach(
            f"Device at {self.device_ip} did not become available within {wait_timeout} seconds",
            name="Device Recovery Timeout",
            attachment_type=allure.attachment_type.TEXT,
        )
        return False

    def reset_device(self) -> bool:
        """
        Reset the device without flashing firmware.
        Uses OpenOCD to send a reset command via the debug probe.

        Returns:
            True if reset succeeded, False otherwise.
        """
        logger.info("Resetting device via debug probe...")

        reset_cmd = (
            f"cd {self.firmware_dir} && "
            f"source {TOOLCHAIN_ENV} && "
            f"openocd "
            f"-f {OPENOCD_INTERFACE} "
            f'-c "transport select swd" '
            f'-c "adapter serial {self.serial}" '
            f"-f {OPENOCD_TARGET} "
            f'-c "init" -c "reset run" -c "exit"'
        )

        try:
            result = subprocess.run(
                reset_cmd,
                shell=True,
                executable="/bin/bash",
                capture_output=True,
                text=True,
                timeout=30,
                cwd=self.firmware_dir,
            )

            # OpenOCD outputs to stderr even on success
            if "reset run" in result.stderr or result.returncode == 0:
                logger.info("Device reset completed successfully")
                return True
            else:
                logger.error(f"Device reset failed: {result.stderr}")
                allure.attach(
                    f"STDOUT:\n{result.stdout}\n\nSTDERR:\n{result.stderr}",
                    name="Device Reset Failed",
                    attachment_type=allure.attachment_type.TEXT,
                )
                return False

        except subprocess.TimeoutExpired:
            logger.error("Device reset timed out")
            return False
        except Exception as e:
            logger.error(f"Device reset error: {e}")
            return False

    def wait_for_device(
        self,
        timeout: float = 60.0,
        check_interval: float = 2.0,
    ) -> bool:
        """
        Wait for device to become available.

        Args:
            timeout: Maximum time to wait in seconds.
            check_interval: Time between availability checks.

        Returns:
            True if device became available, False if timeout.
        """
        logger.info(f"Waiting for device at {self.device_ip} to become available...")
        start_time = time.time()

        while time.time() - start_time < timeout:
            if self._check_device_available():
                elapsed = time.time() - start_time
                logger.info(f"Device is available after {elapsed:.1f}s")
                allure.attach(
                    f"Device recovered and available after {elapsed:.1f} seconds",
                    name="Device Recovery",
                    attachment_type=allure.attachment_type.TEXT,
                )
                return True
            time.sleep(check_interval)

        logger.error(f"Device did not become available within {timeout}s")
        allure.attach(
            f"Device at {self.device_ip} did not become available within {timeout} seconds",
            name="Device Recovery Timeout",
            attachment_type=allure.attachment_type.TEXT,
        )
        return False

    def _check_device_available(self) -> bool:
        """Check if device is reachable via TCP connection."""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(2.0)
            result = sock.connect_ex((self.device_ip, DEVICE_CHECK_PORT))
            sock.close()
            return result == 0
        except socket.error:
            return False