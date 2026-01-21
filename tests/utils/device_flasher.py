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
        reset_interval: float = 10.0,
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

        with allure.step("Resetting device and waiting for recovery"):
            while time.time() - start_time < wait_timeout:
                if time.time() - last_reset_time >= reset_interval:
                    self.reset_device()
                    last_reset_time = time.time()

                if self.check_device_available():
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
            f"source {config.TOOLCHAIN_ENV} && "
            f"openocd "
            f"-f {config.OPENOCD_INTERFACE} "
            f'-c "transport select swd" '
            f'-c "adapter serial {self.serial}" '
            f"-f {config.OPENOCD_TARGET} "
            f'-c "init" -c "reset run" -c "exit"'
        )
        logger.debug(f"Reset command: {reset_cmd}")

        try:
            with allure.step(f"Executing device reset command: {reset_cmd}"):
                result = subprocess.run(
                    reset_cmd,
                    shell=True,
                    executable="/bin/bash",
                    capture_output=True,
                    text=True,
                    timeout=30,
                    cwd=self.firmware_dir,
                )
                logger.debug(f"Reset command result: {result}")
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

    def check_device_available(self) -> bool:
        """Check if device is reachable via TCP connection."""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(2.0)
            result = sock.connect_ex((self.device_ip, 80))
            sock.close()
            return result == 0
        except socket.error:
            return False