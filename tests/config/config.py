"""
Centralized configuration for BSB firmware tests.

Reads from environment variables with fallback defaults for local development.
In the GitHub runner container, these are set by busybar-docker-runner.py.
"""

import os
from pathlib import Path

from dotenv import load_dotenv

# Load .env file from the same directory as this config file
_config_dir = Path(__file__).parent
load_dotenv(_config_dir / ".env")


class Config:
    """Configuration settings from environment variables."""

    # Device settings
    BUSYBAR_IP: str = os.getenv("BUSYBAR_IP", "10.0.4.20")
    DEVICE_CHECK_PORT: int = int(os.getenv("DEVICE_CHECK_PORT", "80"))

    # DAPLink serial numbers (for flashing)
    DAPLINK_U5_ID: str = os.getenv("DAPLINK_U5_ID", "")
    DAPLINK_917_ID: str = os.getenv("DAPLINK_917_ID", "")

    # Paths
    BSB_FIRMWARE_PATH: str = os.path.expanduser(
        os.getenv("BSB_FIRMWARE_PATH", "/opt/bsb-firmware/")
    )
    PROJECT_WORKSPACE: str = os.getenv("PROJECT_WORKSPACE", "/_work")

    # Firmware-relative paths (within BSB_FIRMWARE_PATH)
    FIRMWARE_ELF: str = os.getenv("U5_ELF", "")
    PLATFORM_JSON: str = "scripts/debug/platforms/stm32u595.json"
    TOOLCHAIN_ENV: str = "scripts/toolchain/fbtenv.sh"
    OPENOCD_INTERFACE: str = "interface/cmsis-dap.cfg"
    OPENOCD_TARGET: str = "scripts/debug/platforms/stm32u5/stm32u5x.cfg"

    # Crash detection - ESSION_LOG_DIR (set by serial_logger)
    SESSION_LOG_DIR: str = os.getenv("SESSION_LOG_DIR", "")
    CRASH_FLAG_PATH: str = os.getenv(
        "CRASH_FLAG_PATH",
        os.path.join(SESSION_LOG_DIR, "crash_detected.flag") if SESSION_LOG_DIR else "/tmp/crash_detected.flag"
    )

    OPENOCD_LOCK_PATH: str = os.getenv(
        "OPENOCD_LOCK_FILE",
        os.path.join(BSB_FIRMWARE_PATH, ".openocd.lock") if BSB_FIRMWARE_PATH else "/tmp/.openocd.lock"
    )

    # Cloud settings
    CLOUD_BASE_URL: str = os.getenv("CLOUD_BASE_URL", "")
    CLOUD_EMAIL: str = os.getenv("CLOUD_EMAIL", "")
    CLOUD_PASSWORD: str = os.getenv("CLOUD_PASSWORD", "")
    CLOUD_BASIC_USER: str = os.getenv("CLOUD_BASIC_USER", "")
    CLOUD_BASIC_PASSWORD: str = os.getenv("CLOUD_BASIC_PASSWORD", "")

    @classmethod
    def validate_paths(cls) -> list[str]:
        """
        Validate that firmware-related files and directories exist.

        Returns:
            List of warning messages for missing paths (empty if all OK).
        """
        warnings: list[str] = []

        # Check base firmware directory
        firmware_base = Path(cls.BSB_FIRMWARE_PATH)
        if not firmware_base.is_dir():
            warnings.append(f"BSB_FIRMWARE_PATH not found: {firmware_base}")
            return warnings

        # Check firmware-relative paths (only if base exists)
        relative_paths = {
            "FIRMWARE_ELF": cls.FIRMWARE_ELF,
            "PLATFORM_JSON": cls.PLATFORM_JSON,
            "TOOLCHAIN_ENV": cls.TOOLCHAIN_ENV,
            "OPENOCD_TARGET": cls.OPENOCD_TARGET,
        }

        for name, rel_path in relative_paths.items():
            full_path = firmware_base / rel_path
            if not full_path.exists():
                warnings.append(f"{name}: {full_path}")

        return warnings

    @classmethod
    def device_reset_available(cls) -> bool:
        """Check if device reset via OpenOCD is available."""
        return len(cls.validate_paths()) == 0


config = Config()
