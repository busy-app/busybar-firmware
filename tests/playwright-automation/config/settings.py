"""Configuration management for BUSY Bar tests."""

import os
from typing import Literal, Optional

from pydantic import Field, validator
from pydantic_settings import BaseSettings, SettingsConfigDict


class TestSettings(BaseSettings):
    """Test configuration settings with validation."""

    model_config = SettingsConfigDict(
        env_file=".env", env_file_encoding="utf-8", case_sensitive=False
    )

    # Application URLs
    base_url: str = Field(
        default="http://busybar.local", description="Base URL for BUSY Bar device"
    )
    cloud_url: str = Field(
        default="https://cloud.busy.app", description="BUSY Cloud URL"
    )

    # Browser Configuration
    browser: Literal["chromium", "firefox", "webkit"] = Field(
        default="chromium", description="Browser to use for testing"
    )
    headless: bool = Field(default=False, description="Run browser in headless mode")
    slow_mo: int = Field(default=0, description="Slow down operations by milliseconds")

    # Timeouts (in seconds)
    default_timeout: int = Field(
        default=5, description="Default timeout for page operations"
    )
    navigation_timeout: int = Field(
        default=10, description="Timeout for page navigation"
    )
    element_timeout: int = Field(
        default=2, description="Timeout for element operations (fast fail)"
    )
    modal_timeout: int = Field(
        default=3, description="Timeout for modal dialogs to appear"
    )
    long_timeout: int = Field(
        default=15, description="Extended timeout for slow operations"
    )

    # Device Configuration
    device_ip: Optional[str] = Field(
        default=None, description="Specific IP if not using busybar.local"
    )
    device_port: int = Field(default=80, description="Device web server port")

    # Test Execution
    screenshot_on_failure: bool = Field(
        default=True, description="Take screenshot on test failure"
    )
    video_on_failure: bool = Field(
        default=False, description="Record video for failed tests"
    )
    trace_on_failure: bool = Field(default=True, description="Capture trace on failure")

    # Paths
    screenshot_dir: str = Field(
        default="screenshots", description="Directory for screenshots"
    )
    video_dir: str = Field(default="videos", description="Directory for videos")
    trace_dir: str = Field(default="traces", description="Directory for traces")

    # Environment
    environment: Literal["local", "ci", "staging"] = Field(
        default="local", description="Test environment"
    )

    # Retry Configuration
    retry_count: int = Field(
        default=3, description="Number of retries for flaky operations"
    )
    retry_delay: int = Field(default=1, description="Delay between retries in seconds")

    # CI/CD specific
    ci_mode: bool = Field(default=False, description="Running in CI mode")

    @validator("headless", pre=True)
    def set_headless_for_ci(cls, v, values):
        """Automatically set headless mode for CI."""
        if values.get("ci_mode") or values.get("environment") == "ci":
            return True
        return v

    @validator("base_url", pre=True)
    def construct_base_url(cls, v, values):
        """Construct base URL from device_ip if provided."""
        device_ip = values.get("device_ip")
        if device_ip:
            port = values.get("device_port", 80)
            return f"http://{device_ip}:{port}" if port != 80 else f"http://{device_ip}"
        return v

    @property
    def is_ci(self) -> bool:
        """Check if running in CI environment."""
        return (
            self.ci_mode
            or self.environment == "ci"
            or os.getenv("CI", "").lower() == "true"
        )

    @property
    def browser_args(self) -> list:
        """Get browser launch arguments."""
        args = []
        if self.browser == "chromium":
            args.extend(
                [
                    "--disable-dev-shm-usage",
                    "--disable-blink-features=AutomationControlled",
                    "--no-sandbox",
                ]
            )
            if self.is_ci:
                args.extend(
                    [
                        "--disable-gpu",
                        "--disable-software-rasterizer",
                    ]
                )
        return args

    def get_viewport(self) -> dict:
        """Get viewport configuration."""
        return (
            {"width": 1920, "height": 1080}
            if not self.is_ci
            else {"width": 1280, "height": 720}
        )


# Singleton instance
settings = TestSettings()
