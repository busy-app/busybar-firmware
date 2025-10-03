"""Screenshot utilities for test automation."""

import io
from datetime import datetime
from pathlib import Path
from typing import Optional, Tuple

import allure
from PIL import Image, ImageDraw, ImageFont
from playwright.sync_api import Page


class ScreenshotHelper:
    """Helper class for screenshot operations."""

    def __init__(self, screenshot_dir: str = "artifacts/screenshots"):
        self.screenshot_dir = Path(screenshot_dir)
        self.screenshot_dir.mkdir(parents=True, exist_ok=True)

    def take_screenshot(
        self,
        page: Page,
        name: str = None,
        full_page: bool = True,
        attach_to_allure: bool = True,
    ) -> Path:
        """
        Take a screenshot and optionally attach to Allure report.

        Args:
            page: Playwright page object
            name: Screenshot name (auto-generated if None)
            full_page: Capture full page or viewport only
            attach_to_allure: Attach to Allure report

        Returns:
            Path to saved screenshot
        """
        if not name:
            name = f"screenshot_{datetime.now():%Y%m%d_%H%M%S}"

        # Ensure .png extension
        if not name.endswith(".png"):
            name += ".png"

        screenshot_path = self.screenshot_dir / name

        # Take screenshot
        screenshot_bytes = page.screenshot(
            path=str(screenshot_path), full_page=full_page
        )

        # Attach to Allure if requested
        if attach_to_allure:
            allure.attach(
                screenshot_bytes, name=name, attachment_type=allure.attachment_type.PNG
            )

        return screenshot_path

    def take_element_screenshot(
        self, page: Page, selector: str, name: str = None, attach_to_allure: bool = True
    ) -> Optional[Path]:
        """
        Take screenshot of specific element.

        Args:
            page: Playwright page object
            selector: Element selector
            name: Screenshot name
            attach_to_allure: Attach to Allure report

        Returns:
            Path to screenshot or None if element not found
        """
        element = page.locator(selector)

        if not element.is_visible():
            return None

        if not name:
            name = f"element_{datetime.now():%Y%m%d_%H%M%S}"

        if not name.endswith(".png"):
            name += ".png"

        screenshot_path = self.screenshot_dir / name

        # Take element screenshot
        screenshot_bytes = element.screenshot(path=str(screenshot_path))

        if attach_to_allure:
            allure.attach(
                screenshot_bytes, name=name, attachment_type=allure.attachment_type.PNG
            )

        return screenshot_path

    def highlight_element(
        self, page: Page, selector: str, color: str = "red", duration: int = 2000
    ) -> None:
        """
        Highlight element on page before taking screenshot.

        Args:
            page: Playwright page object
            selector: Element selector
            color: Highlight color
            duration: Highlight duration in ms
        """
        page.evaluate(
            """
            ([selector, color, duration]) => {
                const element = document.querySelector(selector);
                if (element) {
                    const originalStyle = element.style.cssText;
                    element.style.cssText = `
                        ${originalStyle}
                        border: 3px solid ${color} !important;
                        box-shadow: 0 0 10px ${color} !important;
                    `;
                    setTimeout(() => {
                        element.style.cssText = originalStyle;
                    }, duration);
                }
            }
            """,
            [selector, color, duration],
        )

    def take_screenshot_with_highlight(
        self, page: Page, selector: str, name: str = None, color: str = "red"
    ) -> Path:
        """
        Take screenshot with highlighted element.

        Args:
            page: Playwright page object
            selector: Element to highlight
            name: Screenshot name
            color: Highlight color

        Returns:
            Path to screenshot
        """
        # Highlight element
        self.highlight_element(page, selector, color)

        # Wait a bit for highlight to appear
        page.wait_for_timeout(100)

        # Take screenshot
        return self.take_screenshot(page, name)

    def annotate_screenshot(
        self,
        screenshot_path: Path,
        text: str,
        position: Tuple[int, int] = (10, 10),
        color: str = "red",
        font_size: int = 20,
    ) -> Path:
        """
        Add text annotation to screenshot.

        Args:
            screenshot_path: Path to screenshot
            text: Text to add
            position: Text position (x, y)
            color: Text color
            font_size: Font size

        Returns:
            Path to annotated screenshot
        """
        # Open image
        img = Image.open(screenshot_path)
        draw = ImageDraw.Draw(img)

        # Try to use a font, fall back to default if not available
        try:
            font = ImageFont.truetype("arial.ttf", font_size)
        except:
            font = ImageFont.load_default()

        # Add text
        draw.text(position, text, fill=color, font=font)

        # Save annotated image
        annotated_path = screenshot_path.parent / f"annotated_{screenshot_path.name}"
        img.save(annotated_path)

        return annotated_path

    def compare_screenshots(
        self, screenshot1: Path, screenshot2: Path, threshold: float = 0.95
    ) -> Tuple[bool, float]:
        """
        Compare two screenshots for similarity.

        Args:
            screenshot1: First screenshot path
            screenshot2: Second screenshot path
            threshold: Similarity threshold (0-1)

        Returns:
            Tuple of (is_similar, similarity_score)
        """
        img1 = Image.open(screenshot1)
        img2 = Image.open(screenshot2)

        # Resize images to same size if different
        if img1.size != img2.size:
            img2 = img2.resize(img1.size)

        # Convert to grayscale for comparison
        img1_gray = img1.convert("L")
        img2_gray = img2.convert("L")

        # Calculate difference
        diff = Image.blend(img1_gray, img2_gray, 0.5)

        # Calculate similarity score (simplified)
        # In production, use more sophisticated algorithms
        pixels1 = list(img1_gray.getdata())
        pixels2 = list(img2_gray.getdata())

        matching_pixels = sum(
            1 for p1, p2 in zip(pixels1, pixels2) if abs(p1 - p2) < 10  # Tolerance
        )

        similarity = matching_pixels / len(pixels1)

        return similarity >= threshold, similarity

    def create_screenshot_diff(
        self, screenshot1: Path, screenshot2: Path, output_path: Path = None
    ) -> Path:
        """
        Create a visual diff of two screenshots.

        Args:
            screenshot1: First screenshot
            screenshot2: Second screenshot
            output_path: Output path for diff image

        Returns:
            Path to diff image
        """
        img1 = Image.open(screenshot1)
        img2 = Image.open(screenshot2)

        # Ensure same size
        if img1.size != img2.size:
            img2 = img2.resize(img1.size)

        # Create diff image
        diff = Image.new("RGB", img1.size)

        for x in range(img1.width):
            for y in range(img1.height):
                pixel1 = img1.getpixel((x, y))
                pixel2 = img2.getpixel((x, y))

                if pixel1 != pixel2:
                    # Highlight differences in red
                    diff.putpixel((x, y), (255, 0, 0))
                else:
                    # Keep original pixel
                    diff.putpixel((x, y), pixel1)

        if not output_path:
            output_path = (
                self.screenshot_dir / f"diff_{datetime.now():%Y%m%d_%H%M%S}.png"
            )

        diff.save(output_path)
        return output_path

    def cleanup_old_screenshots(self, days: int = 7) -> int:
        """
        Clean up screenshots older than specified days.

        Args:
            days: Number of days to keep screenshots

        Returns:
            Number of files deleted
        """
        cutoff_time = datetime.now() - timedelta(days=days)
        deleted_count = 0

        for screenshot in self.screenshot_dir.glob("*.png"):
            if datetime.fromtimestamp(screenshot.stat().st_mtime) < cutoff_time:
                screenshot.unlink()
                deleted_count += 1

        return deleted_count


# Convenience instance
screenshot_helper = ScreenshotHelper()
