"""
Streaming API client and Pydantic models.

Endpoints:
- GET /api/screen
"""

from __future__ import annotations

import base64

import allure
import requests

from .base import BaseAPI

# Display specifications
FRONT_DISPLAY_WIDTH = 72
FRONT_DISPLAY_HEIGHT = 16
FRONT_DISPLAY_BPP = 3  # RGB, 3 bytes per pixel

BACK_DISPLAY_WIDTH = 160
BACK_DISPLAY_HEIGHT = 80
BACK_DISPLAY_BPP_NIBBLE = True  # 4-bit grayscale, nibble-packed


# === API Client ===


class StreamingAPI(BaseAPI):
    """
    Streaming API client.

    Endpoints:
    - GET /api/screen - Get display frame (base64-encoded raw pixel data)
    """

    def get_screen(self, display: int = 0) -> requests.Response:
        """
        Get display frame response.

        Args:
            display: Display number (0 = front, 1 = back)
        """
        return self.get_raw("/api/screen", params={"display": display})

    def get_screen_bytes(self, display: int = 0) -> bytes:
        """
        Get display frame as raw pixel bytes (base64-decoded).

        Args:
            display: Display number (0 = front, 1 = back)

        Returns:
            Raw pixel data bytes.
            Front (display=0): 3456 bytes (72x16x3 RGB)
            Back (display=1): 6400 bytes (160x80/2, nibble-packed 4-bit grayscale)
        """
        response = self.get_screen(display)
        response.raise_for_status()
        return base64.b64decode(response.content)

    def get_front_display(self) -> requests.Response:
        """Get front display frame."""
        return self.get_screen(display=0)

    def get_back_display(self) -> requests.Response:
        """Get back display frame."""
        return self.get_screen(display=1)

    def attach_screen(self, display: int = 0, name: str = "Display Frame") -> requests.Response:
        """
        Get display frame and attach to Allure report.

        Args:
            display: Display number (0 = front, 1 = back)
            name: Attachment name
        """
        response = self.get_screen(display)
        if response.ok:
            allure.attach(
                response.content,
                name=name,
                attachment_type=allure.attachment_type.BMP,
            )
        return response
