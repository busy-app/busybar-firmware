"""
Display screenshot comparison tests.

Draws known content to the display, captures a screenshot via /api/screen,
and compares the raw pixel data against a saved reference frame.

Updating reference files
------------------------
When the font or render pipeline changes, regenerate stale reference data::

    UPDATE_REFS=1 pytest tests/integration/frontend/assets/test_display_screenshot.py \\
        -k "TestDisplayAlignment"

Each test saves the captured frame in place of the reference and is reported
as SKIPPED.  After the run, commit the updated .raw files.
"""

import os
from pathlib import Path
from time import sleep

import allure
import pytest

from clients.api import AssetsAPI, StreamingAPI
from clients.api.streaming import (
    BACK_DISPLAY_HEIGHT,
    BACK_DISPLAY_WIDTH,
    FRONT_DISPLAY_BPP,
    FRONT_DISPLAY_HEIGHT,
    FRONT_DISPLAY_WIDTH,
)

ASSETS_DIR = Path(__file__).resolve().parents[2] / "assets"

# Set UPDATE_REFS=1 to capture new reference frames instead of comparing.
UPDATE_REFS: bool = os.getenv("UPDATE_REFS", "").lower() in ("1", "true", "yes")

# Pre-computed frame sizes in bytes
FRONT_FRAME_SIZE = FRONT_DISPLAY_WIDTH * FRONT_DISPLAY_HEIGHT * FRONT_DISPLAY_BPP
BACK_FRAME_SIZE = BACK_DISPLAY_WIDTH * BACK_DISPLAY_HEIGHT // 2

# Time for the device to render a draw command before capturing
RENDER_SETTLE_TIME = 1

# Draw elements used for reference screenshots
FRONT_DRAW_ELEMENTS = [
    {
        "id": "1",
        "timeout": 30,
        "type": "text",
        "text": "BSB DISPLAY",
        "x": 36,
        "y": 3,
        "align": "center",
        "font": "small",
        "color": "#FFFFFFFF",
        "display": "front",
    },
    {
        "id": "2",
        "timeout": 30,
        "type": "text",
        "text": "TEST OK",
        "x": 36,
        "y": 11,
        "align": "center",
        "font": "small",
        "color": "#00FF00FF",
        "display": "front",
    },
]

BACK_DRAW_ELEMENTS = [
    {
        "id": "1",
        "timeout": 30,
        "type": "text",
        "text": "SCREENSHOT",
        "x": 80,
        "y": 20,
        "align": "center",
        "font": "normal",
        "color": "#FFFFFFFF",
        "display": "back",
    },
    {
        "id": "2",
        "timeout": 30,
        "type": "text",
        "text": "TEST",
        "x": 80,
        "y": 50,
        "align": "center",
        "font": "normal",
        "color": "#FFFFFFFF",
        "display": "back",
    },
]

SCREENSHOT_APP_ID = "screenshot_test"

# Anchor point for alignment tests (center of 72x16 display)
ALIGN_ANCHOR_X = 36
ALIGN_ANCHOR_Y = 8

ALL_ALIGNMENTS = [
    "top_left", "top_mid", "top_right",
    "mid_left", "center", "mid_right",
    "bottom_left", "bottom_mid", "bottom_right",
]

# --- Helpers ---


def _get_front_bbox(data: bytes) -> tuple[int, int, int, int] | None:
    """Return (min_x, max_x, min_y, max_y) of non-black pixels, or None if blank."""
    w, h = FRONT_DISPLAY_WIDTH, FRONT_DISPLAY_HEIGHT
    min_x, max_x, min_y, max_y = w, 0, h, 0
    for row in range(h):
        for col in range(w):
            off = (row * w + col) * 3
            if data[off] or data[off + 1] or data[off + 2]:
                min_x = min(min_x, col)
                max_x = max(max_x, col)
                min_y = min(min_y, row)
                max_y = max(max_y, row)
    if min_x > max_x:
        return None
    return min_x, max_x, min_y, max_y


def _has_dominant_channel(data: bytes, channel: int, threshold: int = 200) -> bool:
    """Check if frame contains any pixel where the given channel dominates.

    Args:
        data: Raw RGB pixel bytes.
        channel: 0=red, 1=green, 2=blue.
        threshold: Minimum value for the dominant channel.
    """
    for i in range(0, len(data), 3):
        if data[i + channel] >= threshold and all(
            data[i + c] < 50 for c in range(3) if c != channel
        ):
            return True
    return False


def _has_white(data: bytes, threshold: int = 200) -> bool:
    """Check if frame contains any near-white pixel (all channels above threshold)."""
    for i in range(0, len(data), 3):
        if data[i] >= threshold and data[i + 1] >= threshold and data[i + 2] >= threshold:
            return True
    return False


def _get_back_pixel(data: bytes, x: int, y: int) -> int:
    """Return 0–15 grayscale nibble for pixel (x, y) in 4-bit nibble-packed back display."""
    idx = y * BACK_DISPLAY_WIDTH + x
    byte = data[idx // 2]
    return byte & 0x0F if idx % 2 == 0 else (byte >> 4) & 0x0F


def _count_bright_back_pixels(
    data: bytes, x1: int, y1: int, x2: int, y2: int, threshold: int = 10
) -> int:
    """Count pixels >= threshold (0–15) inside rectangle [x1,x2)×[y1,y2) on back display."""
    return sum(
        1
        for y in range(y1, y2)
        for x in range(x1, x2)
        if _get_back_pixel(data, x, y) >= threshold
    )


def _make_front_text_element(
    text: str,
    *,
    element_id: str = "1",
    color: str = "#FFFFFFFF",
    align: str = "center",
    x: int = 36,
    y: int = 5,
) -> dict:
    """Build a front display text element dict."""
    return {
        "id": element_id,
        "timeout": 30,
        "type": "text",
        "text": text,
        "x": x,
        "y": y,
        "align": align,
        "font": "small",
        "color": color,
        "display": "front",
    }


def _draw_and_capture(
    assets_api: AssetsAPI,
    streaming_api: StreamingAPI,
    app_id: str,
    elements: list[dict],
    priority: int | None = None,
    display: int = 0,
) -> bytes:
    """Draw elements, wait for render, capture screen, return raw bytes."""
    payload = {"application_name": app_id, "elements": elements}
    if priority is not None:
        payload["priority"] = priority
    response = assets_api.draw_raw(payload)
    assert response.status_code == 200, (
        f"Draw failed: {response.status_code} {response.text}"
    )
    sleep(RENDER_SETTLE_TIME)
    return streaming_api.get_screen_bytes(display=display)


def _assert_screenshot_matches(actual: bytes, reference: bytes, label: str):
    """Compare captured frame against reference with allure reporting."""
    assert len(actual) == len(reference), (
        f"{label}: size mismatch {len(actual)} != {len(reference)}"
    )
    assert actual == reference, f"{label}: pixel data differs from reference"


# --- Screenshot comparison tests ---


SCREENSHOT_PARAMS = [
    pytest.param(
        FRONT_DRAW_ELEMENTS, 0, "ref_front_screenshot_test.raw", FRONT_FRAME_SIZE,
        id="front",
    ),
    # Back display uses structural check in test_back_display_screenshot:
    # exact pixel comparison is not used because the back display shows
    # dynamic system UI (Wi-Fi indicator, time, etc.) in the background.
]

FORMAT_PARAMS = [
    pytest.param(0, FRONT_FRAME_SIZE, id="front"),
    pytest.param(1, BACK_FRAME_SIZE, id="back"),
]


@allure.feature("5. Web Frontend")
@allure.story("Display Screenshot")
class TestDisplayScreenshot:
    """Screenshot comparison tests for display rendering."""

    @allure.title("Display screenshot comparison ({display_id})")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "elements, display_id, ref_filename, expected_size", SCREENSHOT_PARAMS,
    )
    def test_display_screenshot(
        self,
        elements: list[dict],
        display_id: int,
        ref_filename: str,
        expected_size: int,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
    ):
        """Draw text on display and compare screenshot against reference."""
        ref_path = ASSETS_DIR / ref_filename

        try:
            with allure.step(f"Draw test content on display {display_id}"):
                assets_api.draw(SCREENSHOT_APP_ID, elements)
                sleep(RENDER_SETTLE_TIME)

            with allure.step("Capture and compare"):
                actual = streaming_api.get_screen_bytes(display=display_id)

                if UPDATE_REFS:
                    ref_path.write_bytes(actual)
                    pytest.skip(f"Reference updated: {ref_path.name}")

                assert ref_path.exists(), f"Reference frame not found: {ref_path}"
                reference = ref_path.read_bytes()
                assert len(reference) == expected_size, (
                    f"Reference file size mismatch: {len(reference)} != {expected_size}"
                )
                _assert_screenshot_matches(actual, reference, f"Display {display_id}")
        finally:
            assets_api.clear_display()

    @allure.title("Back display screenshot — structural content check")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_back_display_screenshot(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
    ):
        """Verify that drawing text on the back display produces visible content.

        Exact pixel comparison is not used because the back display renders dynamic
        system UI (Wi-Fi indicator, connection state, etc.) in the background, which
        differs between captures. Instead we verify:
          1. The frame changes after the draw command.
          2. The expected text regions contain bright (near-white) pixels.
        """
        try:
            with allure.step("Capture baseline before drawing"):
                baseline = streaming_api.get_screen_bytes(display=1)

            with allure.step("Draw test content on back display"):
                assets_api.draw(SCREENSHOT_APP_ID, BACK_DRAW_ELEMENTS)
                sleep(RENDER_SETTLE_TIME)

            with allure.step("Capture and verify content"):
                actual = streaming_api.get_screen_bytes(display=1)

                assert actual != baseline, (
                    "Back display did not change after draw — "
                    "draw command may have had no visible effect"
                )

                # "SCREENSHOT" at (80, 20) center-aligned, normal font
                top_bright = _count_bright_back_pixels(actual, 15, 12, 145, 30)
                assert top_bright > 0, (
                    f"No bright pixels in 'SCREENSHOT' text region "
                    f"(rows 12-30, cols 15-145); got {top_bright}"
                )

                # "TEST" at (80, 50) center-aligned, normal font
                bot_bright = _count_bright_back_pixels(actual, 50, 43, 110, 60)
                assert bot_bright > 0, (
                    f"No bright pixels in 'TEST' text region "
                    f"(rows 43-60, cols 50-110); got {bot_bright}"
                )
        finally:
            assets_api.clear_display()

    @allure.title("GET /api/screen returns valid display data ({display_id})")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("display_id, expected_size", FORMAT_PARAMS)
    def test_screen_api_format(
        self,
        display_id: int,
        expected_size: int,
        streaming_api: StreamingAPI,
    ):
        """Verify /api/screen returns correctly sized base64 data."""
        raw = streaming_api.get_screen_bytes(display=display_id)
        assert len(raw) == expected_size, (
            f"Display {display_id}: got {len(raw)} bytes, expected {expected_size}"
        )


# --- Alignment tests ---


@allure.feature("5. Web Frontend")
@allure.story("Display Alignment")
class TestDisplayAlignment:
    """Test text alignment anchor points on the front display."""

    @allure.title("Text alignment: {align}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("align", ALL_ALIGNMENTS)
    def test_text_alignment(
        self,
        align: str,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
    ):
        """Draw text with given alignment and compare against reference frame.

        Run with UPDATE_REFS=1 to refresh stale reference files after a font change.
        """
        ref_path = ASSETS_DIR / f"ref_align_{align}.raw"

        try:
            with allure.step(f"Draw 'X' at ({ALIGN_ANCHOR_X},{ALIGN_ANCHOR_Y}) align={align}"):
                element = _make_front_text_element(
                    "X", x=ALIGN_ANCHOR_X, y=ALIGN_ANCHOR_Y, align=align,
                )
                actual = _draw_and_capture(
                    assets_api, streaming_api, "align_test", [element],
                )

            if UPDATE_REFS:
                ref_path.write_bytes(actual)
                pytest.skip(f"Reference updated: {ref_path.name}")

            assert ref_path.exists(), f"Reference not found: {ref_path}"
            _assert_screenshot_matches(actual, ref_path.read_bytes(), f"Alignment '{align}'")
        finally:
            assets_api.clear_display()

    @allure.title("Alignment grid: positions are consistent and ordered")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_alignment_positions_differ(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
    ):
        """Verify the full 3×3 alignment grid using all 9 alignment values.

        Checks two structural properties that hold regardless of font size:

        Consistency — alignments in the same row/column share an edge coordinate:
          - left column  → same min_x  (anchor is left edge for all)
          - right column → same max_x  (anchor is right edge for all)
          - mid column   → same min_x  (anchor is horizontal center for all)
          - top row      → same min_y  (anchor is top edge for all)
          - bottom row   → same max_y  (anchor is bottom edge for all)
          - mid row      → same min_y  (anchor is vertical center for all)

        Ordering — position shifts predictably across columns/rows:
          - left min_x > mid min_x > right min_x  (text moves left as anchor moves right)
          - top min_y  > mid min_y  > bottom min_y (text moves up as anchor moves down)
        """
        bboxes: dict[str, tuple[int, int, int, int]] = {}

        try:
            for align in ALL_ALIGNMENTS:
                assets_api.clear_display()
                sleep(RENDER_SETTLE_TIME)
                element = _make_front_text_element(
                    "X", x=ALIGN_ANCHOR_X, y=ALIGN_ANCHOR_Y, align=align,
                )
                actual = _draw_and_capture(
                    assets_api, streaming_api, "align_test", [element],
                )
                bbox = _get_front_bbox(actual)
                assert bbox is not None, f"No pixels drawn for align={align}"
                bboxes[align] = bbox

            with allure.step("Verify alignment grid"):
                allure.attach(
                    "\n".join(
                        f"{a:12s}  x=[{b[0]:2d},{b[1]:2d}]  y=[{b[2]:2d},{b[3]:2d}]"
                        for a, b in bboxes.items()
                    ),
                    name="Bounding boxes",
                    attachment_type=allure.attachment_type.TEXT,
                )

                # --- Column consistency (same anchor edge → same coordinate) ---
                assert bboxes["top_left"][0] == bboxes["mid_left"][0] == bboxes["bottom_left"][0], \
                    "left column: all min_x values should be equal"
                assert bboxes["top_right"][1] == bboxes["mid_right"][1] == bboxes["bottom_right"][1], \
                    "right column: all max_x values should be equal"
                assert bboxes["top_mid"][0] == bboxes["center"][0] == bboxes["bottom_mid"][0], \
                    "mid column: all min_x values should be equal"

                # --- Row consistency ---
                assert bboxes["top_left"][2] == bboxes["top_mid"][2] == bboxes["top_right"][2], \
                    "top row: all min_y values should be equal"
                assert bboxes["bottom_left"][3] == bboxes["bottom_mid"][3] == bboxes["bottom_right"][3], \
                    "bottom row: all max_y values should be equal"
                assert bboxes["mid_left"][2] == bboxes["center"][2] == bboxes["mid_right"][2], \
                    "mid row: all min_y values should be equal"

                # --- Horizontal ordering (left starts right of mid, mid right of right) ---
                for row in ("top", "mid", "bottom"):
                    left_key = row + "_left" if row != "mid" else "mid_left"
                    mid_key = row + "_mid" if row != "mid" else "center"
                    right_key = row + "_right" if row != "mid" else "mid_right"
                    assert bboxes[left_key][0] > bboxes[mid_key][0] > bboxes[right_key][0], \
                        f"{row} row: left min_x > mid min_x > right min_x"

                # --- Vertical ordering (top starts below mid, mid below bottom) ---
                for col in ("left", "mid", "right"):
                    top_key = "top_" + col
                    mid_key = "center" if col == "mid" else "mid_" + col
                    bot_key = "bottom_" + col
                    assert bboxes[top_key][2] > bboxes[mid_key][2] > bboxes[bot_key][2], \
                        f"{col} column: top min_y > mid min_y > bottom min_y"
        finally:
            assets_api.clear_display()


# --- Priority tests ---


@allure.feature("5. Web Frontend")
@allure.story("Display Priority")
class TestDisplayPriority:
    """Test draw priority behavior on the front display."""

    @allure.title("Lower priority draw is rejected (409)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_low_priority_rejected(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
    ):
        """Draw with high priority, then verify lower priority is rejected."""
        try:
            with allure.step("Draw with high priority (10)"):
                high_element = _make_front_text_element("HIGH", color="#FFFFFFFF")
                _draw_and_capture(
                    assets_api, streaming_api, "high_prio", [high_element], priority=10,
                )

            with allure.step("Attempt draw with low priority (1)"):
                low_element = _make_front_text_element("LOW", color="#FF0000FF")
                response = assets_api.draw_raw({
                    "application_name": "low_prio",
                    "priority": 1,
                    "elements": [low_element],
                })
                assert response.status_code == 409

            with allure.step("Verify display still shows high-priority content"):
                actual = streaming_api.get_screen_bytes(display=0)
                assert _has_white(actual), (
                    "Expected white pixels from HIGH priority draw"
                )
                assert not _has_dominant_channel(actual, channel=0), (
                    "Should not have red pixels from LOW priority draw"
                )
        finally:
            assets_api.clear_display()

    @allure.title("Higher priority draw replaces lower")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_higher_priority_replaces(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        """Draw with low priority, then verify higher priority replaces it.

        Uses busy_timer_stopped to ensure the loader priority is at the
        idle baseline (10) before the test, so draws at 20 and 40 are
        both above any background interference.
        """
        try:
            with allure.step("Draw with priority 20 (white)"):
                low_element = _make_front_text_element("LOW", color="#FFFFFFFF")
                _draw_and_capture(
                    assets_api, streaming_api, "low", [low_element], priority=20,
                )

            with allure.step("Draw with priority 40 (green)"):
                high_element = _make_front_text_element("HIGH", color="#00FF00FF")
                actual = _draw_and_capture(
                    assets_api, streaming_api, "high", [high_element], priority=40,
                )

            with allure.step("Verify display shows only high-priority (green) content"):
                assert _has_dominant_channel(actual, channel=1), (
                    "Expected green pixels from high priority draw"
                )
                assert not _has_white(actual), (
                    "Should not have white pixels from low priority draw"
                )
        finally:
            assets_api.clear_display()
