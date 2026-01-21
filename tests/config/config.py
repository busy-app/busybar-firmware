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
    FIRMWARE_ELF: str = "busybar-f21-firmware.elf"
    PLATFORM_JSON: str = "scripts/debug/platforms/stm32u595.json"
    TOOLCHAIN_ENV: str = "scripts/toolchain/fbtenv.sh"
    OPENOCD_INTERFACE: str = "interface/cmsis-dap.cfg"
    OPENOCD_TARGET: str = "scripts/debug/platforms/stm32u5/stm32u5x.cfg"

    # Crash detection
    CRASH_FLAG_PATH: str = os.getenv("CRASH_FLAG_PATH", "/tmp/crash_detected.flag")

    @classmethod
    def validate_paths(cls) -> None:
        """
        Validate that all required files and directories exist.

        Raises:
            FileNotFoundError: If any required path does not exist.
        """
        missing_paths: list[str] = []

        # Check base firmware directory
        firmware_base = Path(cls.BSB_FIRMWARE_PATH)
        if not firmware_base.is_dir():
            missing_paths.append(f"BSB_FIRMWARE_PATH: {firmware_base}")

        # Check firmware-relative paths (only if base exists)
        if firmware_base.is_dir():
            relative_paths = {
                "FIRMWARE_ELF": cls.FIRMWARE_ELF,
                "PLATFORM_JSON": cls.PLATFORM_JSON,
                "TOOLCHAIN_ENV": cls.TOOLCHAIN_ENV,
                "OPENOCD_TARGET": cls.OPENOCD_TARGET,
            }

            for name, rel_path in relative_paths.items():
                full_path = firmware_base / rel_path
                if not full_path.exists():
                    missing_paths.append(f"{name}: {full_path}")

        if missing_paths:
            error_msg = "Required paths not found:\n  - " + "\n  - ".join(missing_paths)
            raise FileNotFoundError(error_msg)


config = Config()
