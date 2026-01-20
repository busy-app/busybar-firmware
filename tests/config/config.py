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
        os.getenv("BSB_FIRMWARE_PATH", "~/Projects/bsb-firmware/")
    )
    PROJECT_WORKSPACE: str = os.getenv("PROJECT_WORKSPACE", "/_work")

    # Firmware-relative paths (within BSB_FIRMWARE_PATH)
    FIRMWARE_ELF: str = "dist/f21-D/busybar-f21-firmware-local.elf"
    PLATFORM_JSON: str = "scripts/debug/platforms/stm32u595.json"
    TOOLCHAIN_ENV: str = "scripts/toolchain/fbtenv.sh"
    OPENOCD_INTERFACE: str = "interface/cmsis-dap.cfg"
    OPENOCD_TARGET: str = "scripts/debug/platforms/stm32u5/stm32u5x.cfg"

    # Crash detection
    CRASH_FLAG_PATH: str = os.getenv("CRASH_FLAG_PATH", "/tmp/crash_detected.flag")


# Singleton instance for easy import
config = Config()
