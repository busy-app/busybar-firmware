"""
Streaming API client and Pydantic models.

Endpoints:
- GET /api/screen
"""

from __future__ import annotations

import base64
from io import BytesIO

import allure
import requests
from PIL import Image

from .base import BaseAPI

# Display specifications
FRONT_DISPLAY_WIDTH = 72
FRONT_DISPLAY_HEIGHT = 16
FRONT_DISPLAY_BPP = 3  # RGB, 3 bytes per pixel

BACK_DISPLAY_WIDTH = 160
BACK_DISPLAY_HEIGHT = 80
BACK_DISPLAY_BPP_NIBBLE = True  # 4-bit grayscale, nibble-packed

SCREENSHOT_SCALE = 4  # upscale factor for PNG screenshots


def raw_to_png(raw_pixels: bytes, display: int) -> bytes:
    """Convert raw display pixel data to PNG bytes.

    Args:
        raw_pixels: Raw pixel data from GET /api/screen (base64-decoded).
        display: 0 = front (72x16 RGB), 1 = back (160x80 4-bit grayscale).

    Returns:
        PNG image bytes, upscaled for readability.
    """
    if display == 0:
        img = Image.frombytes(
            "RGB",
            (FRONT_DISPLAY_WIDTH, FRONT_DISPLAY_HEIGHT),
            raw_pixels,
        )
    else:
        w, h = BACK_DISPLAY_WIDTH, BACK_DISPLAY_HEIGHT
        pixels = bytearray(w * h)
        for i, byte in enumerate(raw_pixels):
            lo = byte & 0x0F
            hi = (byte >> 4) & 0x0F
            pixels[i * 2] = lo * 17      # 0x0F -> 0xFF
            pixels[i * 2 + 1] = hi * 17
        img = Image.frombytes("L", (w, h), bytes(pixels))

    img = img.resize(
        (img.width * SCREENSHOT_SCALE, img.height * SCREENSHOT_SCALE),
        Image.NEAREST,
    )
    buf = BytesIO()
    img.save(buf, format="PNG")
    return buf.getvalue()


def attach_failure_screenshots(base_url: str) -> None:
    """Capture screenshots from both displays and attach as PNG to Allure.

    Intended for use in pytest hooks where no StreamingAPI instance is available.
    Silently does nothing if the device cannot be reached.
    """
    for display, name in (
        (0, "Front Display (on failure)"),
        (1, "Back Display (on failure)"),
    ):
        try:
            resp = requests.get(
                f"{base_url}/api/screen",
                params={"display": display},
                timeout=3,
            )
            if resp.ok and resp.content:
                png_bytes = raw_to_png(base64.b64decode(resp.content), display)
                allure.attach(
                    png_bytes,
                    name=name,
                    attachment_type=allure.attachment_type.PNG,
                )
        except Exception:
            pass  # device unreachable or decode error — skip


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

    def get_screen_png(self, display: int = 0) -> bytes:
        """Get display frame as PNG bytes (upscaled for readability)."""
        return raw_to_png(self.get_screen_bytes(display), display)

    def attach_screen(self, display: int = 0, name: str = "Display Frame") -> None:
        """Get display frame and attach as PNG to Allure report."""
        try:
            png_bytes = self.get_screen_png(display)
            allure.attach(png_bytes, name=name, attachment_type=allure.attachment_type.PNG)
        except Exception:
            pass
