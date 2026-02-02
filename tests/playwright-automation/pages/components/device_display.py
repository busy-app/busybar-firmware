"""Device display component for BUSY Bar."""

try:
    import allure
except ImportError:
    # Mock allure if not available
    class allure:
        @staticmethod
        def step(title):
            def decorator(func):
                return func

            return decorator


import base64
from typing import Optional, Tuple

from playwright.sync_api import Locator, Page


class DeviceDisplayComponent:
    """Component for interacting with device display canvas."""

    # Selectors - Updated for new DOM structure
    DISPLAY_CONTAINER = ".screen-stream-container, div[data-v-ca2a5bb9], .display-container, .device-screen, main"
    DEVICE_IMAGE = ".device-image img, img[data-savepage-src*='busybar-device'], img[src*='busybar-device'], .device img, nav img"
    CANVAS = "canvas"
    PIXEL_GRID_OVERLAY = "img[src*='pixel-grid'], .pixel-grid, .grid-overlay"

    # Additional display elements
    DISPLAY_WRAPPER = ".display-wrapper, .canvas-wrapper, div[data-v-ca2a5bb9]"
    SCREEN_AREA = ".screen-area, .display-area"
    DEVICE_FRAME_IMAGE = "img[src*='busybar-device'], .device-image img"

    def __init__(self, page: Page):
        self.page = page

    @allure.step("Check if display is visible")
    def is_display_visible(self) -> bool:
        """Check if device display is visible (canvas or image)."""
        return (
            self.page.locator(self.CANVAS).is_visible()
            or self.page.locator(self.DEVICE_IMAGE).is_visible()
            or self.page.locator(self.DISPLAY_CONTAINER).is_visible()
        )

    @allure.step("Get canvas element")
    def get_canvas(self) -> Locator:
        """Get canvas element."""
        return self.page.locator(self.CANVAS)

    @allure.step("Get canvas dimensions")
    def get_canvas_dimensions(self) -> Tuple[int, int]:
        """Get canvas width and height."""
        canvas = self.get_canvas()
        width = canvas.get_attribute("width")
        height = canvas.get_attribute("height")
        return int(width), int(height)

    @allure.step("Capture display screenshot")
    def capture_display(self) -> bytes:
        """Capture screenshot of device display."""
        canvas = self.get_canvas()
        # Get bounding box of canvas
        bbox = canvas.bounding_box()
        if bbox:
            return self.page.screenshot(
                clip={
                    "x": bbox["x"],
                    "y": bbox["y"],
                    "width": bbox["width"],
                    "height": bbox["height"],
                }
            )
        return self.page.screenshot()

    @allure.step("Get canvas content as data URL")
    def get_canvas_data_url(self) -> str:
        """Get canvas content as base64 data URL."""
        return self.page.evaluate(
            """
            () => {
                const canvas = document.querySelector('canvas');
                return canvas ? canvas.toDataURL() : null;
            }
        """
        )

    @allure.step("Check if display is streaming")
    def is_streaming(self) -> bool:
        """Check if display is actively streaming content."""
        # Check if canvas context is being updated
        initial_data = self.get_canvas_data_url()
        self.page.wait_for_timeout(1000)  # Wait 1 second
        current_data = self.get_canvas_data_url()
        return initial_data != current_data

    @allure.step("Wait for display to start streaming")
    def wait_for_streaming(self, timeout: int = 10000) -> None:
        """Wait for display to start streaming."""
        self.page.wait_for_function(
            """
            () => {
                const canvas = document.querySelector('canvas');
                if (!canvas) return false;
                const ctx = canvas.getContext('2d');
                const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
                return imageData.data.some(pixel => pixel !== 0);
            }
            """,
            timeout=timeout,
        )

    @allure.step("Get pixel at position")
    def get_pixel_color(self, x: int, y: int) -> Tuple[int, int, int, int]:
        """Get RGBA color of pixel at specific position."""
        color = self.page.evaluate(
            """
            ([x, y]) => {
                const canvas = document.querySelector('canvas');
                if (!canvas) return null;
                const ctx = canvas.getContext('2d');
                const pixel = ctx.getImageData(x, y, 1, 1).data;
                return [pixel[0], pixel[1], pixel[2], pixel[3]];
            }
            """,
            [x, y],
        )
        return tuple(color) if color else (0, 0, 0, 0)

    @allure.step("Check if display shows content")
    def has_content(self) -> bool:
        """Check if canvas has any non-black content or device image is present."""
        # First check if device image is present (static image case)
        if self.page.locator(self.DEVICE_IMAGE).is_visible():
            return True

        # Then check canvas content for dynamic display
        return self.page.evaluate(
            """
            () => {
                const canvas = document.querySelector('canvas');
                if (!canvas) return false;
                const ctx = canvas.getContext('2d');
                const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
                // Check if any pixel is not black
                for (let i = 0; i < imageData.data.length; i += 4) {
                    if (imageData.data[i] !== 0 || 
                        imageData.data[i+1] !== 0 || 
                        imageData.data[i+2] !== 0) {
                        return true;
                    }
                }
                return false;
            }
            """
        )

    @allure.step("Compare display with reference image")
    def compare_with_reference(
        self, reference_path: str, threshold: float = 0.95
    ) -> bool:
        """Compare current display with reference image."""
        # This would require image comparison library like Pillow or OpenCV
        # Placeholder for now
        current = self.capture_display()
        # TODO: Implement actual image comparison
        return NotImplemented

    @allure.step("Wait for specific content on display")
    def wait_for_content(self, text_or_pattern: str, timeout: int = 10000) -> None:
        """Wait for specific content to appear on display."""
        # This would require OCR or pattern matching
        # For now, just wait for any content
        self.wait_for_streaming(timeout)

    @allure.step("Check if device frame image is visible")
    def is_device_frame_visible(self) -> bool:
        """Check if device frame image is visible."""
        return self.page.locator(self.DEVICE_FRAME_IMAGE).is_visible()

    @allure.step("Get device frame image source")
    def get_device_frame_src(self) -> str:
        """Get device frame image source URL."""
        if self.is_device_frame_visible():
            return self.page.locator(self.DEVICE_FRAME_IMAGE).get_attribute("src") or ""
        return ""
