"""Integration tests for XPM2 elements in POST /api/display/draw."""

from __future__ import annotations

import time

import allure
import pytest

from clients.api import AssetsAPI, StreamingAPI
from clients.api.assets import DEFAULT_ELEMENT_PRIORITY
from clients.api.streaming import (
    BACK_DISPLAY_HEIGHT,
    BACK_DISPLAY_WIDTH,
    FRONT_DISPLAY_HEIGHT,
    FRONT_DISPLAY_WIDTH,
)

_APP = "xpm_test_app"
_RENDER_SETTLE = 0.5


def _draw(api: AssetsAPI, elements: list[dict]):
    """Post an XPM draw request and return the raw response."""
    return api.draw_response(_APP, elements, priority=DEFAULT_ELEMENT_PRIORITY)


def _xpm(data: str, overrides: dict | None = None, **extra) -> dict:
    """Build a minimal XPM2 bitmap element."""
    base = {
        "id": "xpm1",
        "type": "xpmbitmap",
        "data": data,
        "align": "top_left",
        "timeout": 5,
    }
    if overrides:
        base.update(overrides)
    base.update(extra)
    return base


def _solid_xpm(width: int, height: int, key: str = "X") -> str:
    """Build a valid single-color XPM2 image of the requested dimensions."""
    rows = "\n".join(key * width for _ in range(height))
    return (
        f"! XPM2\n{width} {height} 1 {len(key)}\n"
        f"{key} c #FFFFFF\n{rows}"
    )


def _max_colors_xpm() -> str:
    """Build a valid XPM2 image with the API maximum of 32 colors."""
    keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef"
    colors = "\n".join(f"{key} c #FFFFFF" for key in keys)
    return f"! XPM2\n1 1 {len(keys)} 1\n{colors}\n{keys[0]}"


def _rectangle(element_id: str, color: str, **extra) -> dict:
    """Build a minimal solid rectangle display element."""
    base = {
        "id": element_id,
        "type": "rectangle",
        "width": 1,
        "height": 1,
        "fill": "solid",
        "fill_colors": [color],
        "border_width": 0,
        "align": "top_left",
        "timeout": 5,
    }
    base.update(extra)
    return base


def _front_pixel(frame: bytes, x: int, y: int) -> tuple[int, int, int]:
    """Return one RGB pixel from the front display's raw BGR byte layout."""
    offset = (y * FRONT_DISPLAY_WIDTH + x) * 3
    return frame[offset + 2], frame[offset + 1], frame[offset]


def _back_pixel(frame: bytes, x: int, y: int) -> int:
    """Return one 4-bit luma pixel from a nibble-packed back-display frame."""
    pixel_index = y * BACK_DISPLAY_WIDTH + x
    packed = frame[pixel_index // 2]
    return (packed & 0x0F) if pixel_index % 2 == 0 else (packed >> 4)


@allure.feature("5. Web Frontend")
@allure.story("Draw API – XPM Bitmap Element")
class TestXpmBitmapElement:
    """XPM2 data is validated, decoded, and rendered as a raw canvas image."""

    _FRONT_DATA = (
        "! XPM2\n"
        "2 2 4 1\n"
        "R c #FF0000\n"
        "G c #00FF00\n"
        "B c #0000FF\n"
        "W c #FFFFFF\n"
        "RG\n"
        "BW"
    )
    _BACK_DATA = (
        "! XPM2\n"
        "2 1 2 1\n"
        "K c #000000\n"
        "W c #FFFFFF\n"
        "KW"
    )

    @allure.title("XPM bitmap renders exact colors on the front display")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_front_rendering(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 3, 4

        with allure.step("Draw a 2x2 four-color XPM bitmap"):
            response = _draw(
                assets_api,
                [_xpm(self._FRONT_DATA, display="front", x=x, y=y)],
            )
            assets_api.assert_status(response, 200)
            time.sleep(_RENDER_SETTLE)

        with allure.step("Verify the four rendered RGB pixels"):
            frame = streaming_api.get_screen_bytes(display=0)
            actual = {
                "red": _front_pixel(frame, x, y),
                "green": _front_pixel(frame, x + 1, y),
                "blue": _front_pixel(frame, x, y + 1),
                "white": _front_pixel(frame, x + 1, y + 1),
            }
            expected = {
                "red": (255, 0, 0),
                "green": (0, 255, 0),
                "blue": (0, 0, 255),
                "white": (255, 255, 255),
            }
            assert actual == expected, (
                f"Unexpected front XPM pixels: {actual!r}"
            )

    @allure.title("XPM bitmap renders luma on the back display")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_back_rendering(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 20, 30

        with allure.step(
            "Draw adjacent black and white pixels on the back display"
        ):
            response = _draw(
                assets_api,
                [_xpm(self._BACK_DATA, display="back", x=x, y=y)],
            )
            assets_api.assert_status(response, 200)
            time.sleep(_RENDER_SETTLE)

        with allure.step("Verify the nibble-packed luma values"):
            frame = streaming_api.get_screen_bytes(display=1)
            actual = (_back_pixel(frame, x, y), _back_pixel(frame, x + 1, y))
            assert actual == (0x0, 0xF), (
                f"Unexpected back XPM pixels: {actual!r}"
            )

    @allure.title("XPM bitmap requires data")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_data(self, assets_api: AssetsAPI, busy_timer_stopped):
        element = _xpm(self._FRONT_DATA)
        del element["data"]

        response = _draw(assets_api, [element])
        assets_api.assert_status(response, 400)

    @allure.title("Malformed XPM data is rejected: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "data"),
        [
            pytest.param(
                "missing signature",
                "1 1 1 1\n. c #FFFFFF\n.",
                id="signature",
            ),
            pytest.param("invalid header", "! XPM2\n1 1 0 1", id="header"),
            pytest.param(
                "invalid color",
                "! XPM2\n1 1 1 1\n. c not-a-color\n.",
                id="color",
            ),
            pytest.param(
                "unknown pixel key",
                "! XPM2\n1 1 1 1\n. c #FFFFFF\nX",
                id="pixels",
            ),
        ],
    )
    def test_malformed_data_rejected(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
        case: str,
        data: str,
    ):
        response = _draw(assets_api, [_xpm(data)])
        assets_api.assert_status(response, 400)

    @allure.title("XPM API limit is enforced: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "data", "display"),
        [
            pytest.param(
                "33 colors", "! XPM2\n1 1 33 1", "front", id="colors"
            ),
            pytest.param("cpp 5", "! XPM2\n1 1 1 5", "front", id="cpp"),
            pytest.param(
                "front width 73",
                "! XPM2\n73 1 1 1",
                "front",
                id="front-width",
            ),
            pytest.param(
                "back width 161",
                "! XPM2\n161 1 1 1",
                "back",
                id="back-width",
            ),
            pytest.param(
                "front height 17",
                "! XPM2\n1 17 1 1",
                "front",
                id="front-height",
            ),
            pytest.param(
                "back height 81",
                "! XPM2\n1 81 1 1",
                "back",
                id="back-height",
            ),
        ],
    )
    def test_api_limits_rejected(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
        case: str,
        data: str,
        display: str,
    ):
        response = _draw(assets_api, [_xpm(data, display=display)])
        assets_api.assert_status(response, 400)

    @allure.title("XPM API boundary is accepted: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "data", "display"),
        [
            pytest.param(
                "32 colors", _max_colors_xpm(), "front", id="colors"
            ),
            pytest.param(
                "cpp 4", _solid_xpm(1, 1, key="ABCD"), "front", id="cpp"
            ),
            pytest.param(
                "front dimensions",
                _solid_xpm(FRONT_DISPLAY_WIDTH, FRONT_DISPLAY_HEIGHT),
                "front",
                id="front-dimensions",
            ),
            pytest.param(
                "back dimensions",
                _solid_xpm(BACK_DISPLAY_WIDTH, BACK_DISPLAY_HEIGHT),
                "back",
                id="back-dimensions",
            ),
        ],
    )
    def test_api_boundaries_accepted(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
        case: str,
        data: str,
        display: str,
    ):
        response = _draw(assets_api, [_xpm(data, display=display)])
        assets_api.assert_status(response, 200)

    @allure.title("XPM opacity outside 0..100 is rejected: {opacity}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("opacity", [-1, 101])
    def test_invalid_opacity_rejected(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
        opacity: int,
    ):
        response = _draw(assets_api, [_xpm(self._FRONT_DATA, opacity=opacity)])
        assets_api.assert_status(response, 400)

    @allure.title("XPM with zero opacity does not cover existing content")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_zero_opacity_rendering(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 3, 4
        blue_data = "! XPM2\n1 1 1 1\nB c #0000FF\nB"
        elements = [
            _rectangle("background", "#FF0000FF", x=x, y=y),
            _xpm(blue_data, x=x, y=y, opacity=0),
        ]

        response = _draw(assets_api, elements)
        assets_api.assert_status(response, 200)
        time.sleep(_RENDER_SETTLE)

        actual = _front_pixel(
            streaming_api.get_screen_bytes(display=0), x, y
        )
        assert actual == (255, 0, 0), (
            f"Zero-opacity XPM covered the background: {actual!r}"
        )

    @allure.title("XPM opacity changes the rendered front pixel")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_opacity_rendering(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        red_data = "! XPM2\n1 1 1 1\nR c #FF0000\nR"
        x, y = 3, 4

        with allure.step("Render the control pixel at full opacity"):
            response = _draw(
                assets_api, [_xpm(red_data, x=x, y=y, opacity=100)]
            )
            assets_api.assert_status(response, 200)
            time.sleep(_RENDER_SETTLE)
            full = _front_pixel(
                streaming_api.get_screen_bytes(display=0), x, y
            )

        with allure.step("Render the same pixel at half opacity"):
            response = _draw(
                assets_api, [_xpm(red_data, x=x, y=y, opacity=50)]
            )
            assets_api.assert_status(response, 200)
            time.sleep(_RENDER_SETTLE)
            half = _front_pixel(
                streaming_api.get_screen_bytes(display=0), x, y
            )

        with allure.step("Verify opacity reduced only the red channel"):
            assert full == (255, 0, 0), (
                f"Unexpected full-opacity pixel: {full!r}"
            )
assert abs(half[0] - full[0] * 0.5) <= 2, (
    f"Unexpected half-opacity red value: {half!r}"
)
assert half[1:] == (0, 0), (
    f"Unexpected half-opacity channels: {half!r}"
)

    @allure.title("Transparent XPM pixels preserve underlying content")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_transparent_color_rendering(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 3, 4
        data = (
            "! XPM2\n"
            "2 1 2 1\n"
            "T c none\n"
            "G c #00FF00\n"
            "TG"
        )
        elements = [
            _rectangle("background", "#FF0000FF", x=x, y=y, width=2),
            _xpm(data, x=x, y=y),
        ]

        response = _draw(assets_api, elements)
        assets_api.assert_status(response, 200)
        time.sleep(_RENDER_SETTLE)

        frame = streaming_api.get_screen_bytes(display=0)
        actual = (
            _front_pixel(frame, x, y),
            _front_pixel(frame, x + 1, y),
        )
        assert actual == ((255, 0, 0), (0, 255, 0)), (
            f"Unexpected transparent XPM composition: {actual!r}"
        )

    @allure.title("XPM renders alongside another element in one request")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_mixed_element_request(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 3, 4
        red_data = "! XPM2\n1 1 1 1\nR c #FF0000\nR"
        elements = [
            _xpm(red_data, x=x, y=y),
            _rectangle("companion", "#00FF00FF", x=x + 2, y=y),
        ]

        response = _draw(assets_api, elements)
        assets_api.assert_status(response, 200)
        time.sleep(_RENDER_SETTLE)

        frame = streaming_api.get_screen_bytes(display=0)
        actual = (
            _front_pixel(frame, x, y),
            _front_pixel(frame, x + 2, y),
        )
        assert actual == ((255, 0, 0), (0, 255, 0)), (
            f"Unexpected mixed-element pixels: {actual!r}"
        )

    @allure.title("XPM can be replaced, cleared, and drawn again")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_redraw_lifecycle(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 3, 4

        def draw_color(color: str) -> tuple[int, int, int]:
            data = f"! XPM2\n1 1 1 1\nX c {color}\nX"
            response = _draw(assets_api, [_xpm(data, x=x, y=y)])
            assets_api.assert_status(response, 200)
            time.sleep(_RENDER_SETTLE)
            return _front_pixel(
                streaming_api.get_screen_bytes(display=0), x, y
            )

        with allure.step("Draw and then replace an existing raw image"):
            red = draw_color("#FF0000")
            green = draw_color("#00FF00")

        with allure.step("Clear the raw image and draw a new one"):
            assets_api.clear_display()
            time.sleep(_RENDER_SETTLE)
            blue = draw_color("#0000FF")

        with allure.step("Verify every raw image reached the framebuffer"):
            actual = {"red": red, "green": green, "blue": blue}
            expected = {
                "red": (255, 0, 0),
                "green": (0, 255, 0),
                "blue": (0, 0, 255),
            }
            assert actual == expected, (
                f"Unexpected XPM lifecycle pixels: {actual!r}"
            )
