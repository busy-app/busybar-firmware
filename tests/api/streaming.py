"""
Streaming API client and Pydantic models.

Endpoints:
- GET /api/screen
"""

from __future__ import annotations

import allure
import requests

from .base import BaseAPI


# === API Client ===


class StreamingAPI(BaseAPI):
    """
    Streaming API client.

    Endpoints:
    - GET /api/screen - Get display frame as BMP image
    """

    def get_screen(self, display: int = 0) -> requests.Response:
        """
        Get display frame as BMP image.

        Args:
            display: Display number (0 = front, 1 = back)
        """
        return self.get_raw("/api/screen", params={"display": display})

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
