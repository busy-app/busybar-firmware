"""
Display screenshot comparison tests.

Draws known content to the display, captures a screenshot via /api/screen,
and compares the raw pixel data against a saved reference frame.
"""

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
        "font": "medium",
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
        "font": "medium",
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
    pytest.param(
        BACK_DRAW_ELEMENTS, 1, "ref_back_screenshot_test.raw", BACK_FRAME_SIZE,
        id="back",
    ),
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
        assert ref_path.exists(), f"Reference frame not found: {ref_path}"
        reference = ref_path.read_bytes()
        assert len(reference) == expected_size, (
            f"Reference file size mismatch: {len(reference)} != {expected_size}"
        )

        try:
            with allure.step(f"Draw test content on display {display_id}"):
                assets_api.draw(SCREENSHOT_APP_ID, elements)
                sleep(RENDER_SETTLE_TIME)

            with allure.step("Capture and compare"):
                actual = streaming_api.get_screen_bytes(display=display_id)
                _assert_screenshot_matches(actual, reference, f"Display {display_id}")
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
        """Draw text with given alignment and compare against reference frame."""
        ref_path = ASSETS_DIR / f"ref_align_{align}.raw"
        assert ref_path.exists(), f"Reference not found: {ref_path}"
        reference = ref_path.read_bytes()

        try:
            with allure.step(f"Draw 'X' at ({ALIGN_ANCHOR_X},{ALIGN_ANCHOR_Y}) align={align}"):
                element = _make_front_text_element(
                    "X", x=ALIGN_ANCHOR_X, y=ALIGN_ANCHOR_Y, align=align,
                )
                actual = _draw_and_capture(
                    assets_api, streaming_api, "align_test", [element],
                )
            _assert_screenshot_matches(actual, reference, f"Alignment '{align}'")
        finally:
            assets_api.clear_display()

    @allure.title("Alignment affects text position")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_alignment_positions_differ(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
    ):
        """Verify that different alignments produce different pixel positions."""
        test_alignments = ["top_left", "center", "bottom_right"]
        bboxes = {}

        try:
            for align in test_alignments:
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

            with allure.step("Verify positions differ"):
                allure.attach(
                    "\n".join(f"{a}: x=[{b[0]},{b[1]}] y=[{b[2]},{b[3]}]"
                              for a, b in bboxes.items()),
                    name="Bounding boxes",
                    attachment_type=allure.attachment_type.TEXT,
                )

                # top_left anchor: text extends right and down from anchor
                # center anchor: text is centered on anchor
                # bottom_right anchor: text extends left and up from anchor
                assert bboxes["top_left"][0] > bboxes["center"][0], (
                    "top_left should be to the right of center"
                )
                assert bboxes["top_left"][2] > bboxes["center"][2], (
                    "top_left should be below center"
                )
                assert bboxes["bottom_right"][1] < bboxes["center"][1], (
                    "bottom_right should be to the left of center"
                )
                assert bboxes["bottom_right"][3] < bboxes["center"][3], (
                    "bottom_right should be above center"
                )
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

    @allure.title("Equal priority draw succeeds and replaces")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_equal_priority_replaces(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
    ):
        """Draw with a priority, then verify equal priority replaces it."""
        try:
            with allure.step("Draw with priority 8 (white)"):
                first_element = _make_front_text_element("FIRST", color="#FFFFFFFF")
                _draw_and_capture(
                    assets_api, streaming_api, "first", [first_element], priority=8,
                )

            with allure.step("Draw with same priority 8 (green)"):
                second_element = _make_front_text_element("SECOND", color="#00FF00FF")
                actual = _draw_and_capture(
                    assets_api, streaming_api, "second", [second_element], priority=8,
                )

            with allure.step("Verify display shows only second (green) content"):
                assert _has_dominant_channel(actual, channel=1), (
                    "Expected green pixels from second draw"
                )
                assert not _has_white(actual), (
                    "Should not have white pixels from first draw"
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
    ):
        """Draw with low priority, then verify higher priority replaces it."""
        try:
            with allure.step("Draw with priority 3 (white)"):
                low_element = _make_front_text_element("LOW", color="#FFFFFFFF")
                _draw_and_capture(
                    assets_api, streaming_api, "low", [low_element], priority=3,
                )

            with allure.step("Draw with priority 9 (green)"):
                high_element = _make_front_text_element("HIGH", color="#00FF00FF")
                actual = _draw_and_capture(
                    assets_api, streaming_api, "high", [high_element], priority=9,
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
