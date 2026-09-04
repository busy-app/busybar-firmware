"""Integration tests for XPM2 elements in POST /api/display/draw."""

from __future__ import annotations

import allure
import pytest

from clients.api import AssetsAPI, StreamingAPI
from clients.api.streaming import (
    BACK_DISPLAY_HEIGHT,
    BACK_DISPLAY_WIDTH,
    FRONT_DISPLAY_HEIGHT,
    FRONT_DISPLAY_WIDTH,
    FrontFrame,
)
from clients.api.assets import DEFAULT_ELEMENT_PRIORITY
from .display_helpers import (
    FILL_BLACK,
    FILL_GREEN,
    FILL_RED,
    RGB_BLACK,
    RGB_BLUE,
    RGB_GREEN,
    RGB_RED,
    RGB_WHITE,
    assert_back_pixels,
    assert_front_pixels,
    colors_xpm,
    pixel_xpm,
    solid_rectangle,
    solid_xpm,
    wait_for_front_frame,
    xpm_element,
    xpm_source,
)

_APP = "xpm_test_app"


def _draw(api: AssetsAPI, elements: list[dict]):
    """Post an XPM draw request and return the raw response."""
    return api.draw_response(_APP, elements, priority=DEFAULT_ELEMENT_PRIORITY)


@allure.feature("5. Web Frontend")
@allure.story("Draw API – XPM Bitmap Element")
class TestXpmBitmapElement:
    """XPM2 data is validated, decoded, and rendered as a raw canvas image."""

    _FRONT_DATA = xpm_source(
        2,
        2,
        {
            "R": "c #FF0000",
            "G": "c #00FF00",
            "B": "c #0000FF",
            "W": "c #FFFFFF",
        },
        ["RG", "BW"],
    )
    _BACK_DATA = xpm_source(
        3,
        1,
        {"K": "c #000000", "G": "c #808080", "W": "c #FFFFFF"},
        ["KGW"],
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
                [xpm_element(self._FRONT_DATA, display="front", x=x, y=y)],
            )
            assets_api.assert_status(response, 200)

        with allure.step("Verify the four rendered RGB pixels"):
            assert_front_pixels(
                streaming_api,
                {
                    (x, y): RGB_RED,
                    (x + 1, y): RGB_GREEN,
                    (x, y + 1): RGB_BLUE,
                    (x + 1, y + 1): RGB_WHITE,
                },
                "Front XPM colors",
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
            "Draw adjacent black, mid-gray, and white pixels on the back "
            "display"
        ):
            response = _draw(
                assets_api,
                [xpm_element(self._BACK_DATA, display="back", x=x, y=y)],
            )
            assets_api.assert_status(response, 200)

        with allure.step("Verify the nibble-packed luma values"):
            # mid-gray pins the luma conversion; black and white alone can
            # match the canvas background
            assert_back_pixels(
                streaming_api,
                {(x, y): 0x0, (x + 1, y): 0x8, (x + 2, y): 0xF},
                "Back XPM luma",
            )

    @allure.title("XPM bitmap requires data")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_data(self, assets_api: AssetsAPI, busy_timer_stopped):
        with allure.step("Submit an XPM element without data"):
            element = xpm_element(self._FRONT_DATA)
            del element["data"]
            response = _draw(assets_api, [element])

        with allure.step("Verify the missing-data error"):
            assets_api.assert_status(response, 400)
            assert "elements[0].data is required" in response.text, (
                f"Unexpected missing-data response: {response.text!r}"
            )

    @allure.title("Malformed XPM data is rejected: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "data", "expected_error"),
        [
            pytest.param(
                "missing signature",
                "1 1 1 1\n. c #FFFFFF\n.",
                "elements[0].data value is invalid; must have valid XPM header",
                id="signature",
            ),
            pytest.param(
                "invalid header",
                "! XPM2\n1 1 1\n. c #FFFFFF\n.",
                "elements[0].data value is invalid; must have valid XPM header",
                id="header",
            ),
            pytest.param(
                "invalid color",
                "! XPM2\n1 1 1 1\n. c not-a-color\n.",
                "elements[0].data value is invalid; must have valid XPM color table",
                id="color",
            ),
            pytest.param(
                "unknown pixel key",
                "! XPM2\n1 1 1 1\n. c #FFFFFF\nX",
                "elements[0].data value is invalid; must have valid XPM pixel data",
                id="pixels",
            ),
            pytest.param(
                "pixel row shorter than width",
                xpm_source(3, 1, {".": "c #FFFFFF"}, [".."]),
                "elements[0].data value is invalid; must have valid XPM pixel data",
                id="short-row",
            ),
            pytest.param(
                "empty data",
                "",
                "elements[0].data value is invalid; must have valid XPM header",
                id="empty",
            ),
        ],
    )
    def test_malformed_data_rejected(
        self,
        assets_api: AssetsAPI,
        busy_timer_stopped,
        case: str,
        data: str,
        expected_error: str,
    ):
        with allure.step(f"Submit malformed XPM data: {case}"):
            response = _draw(assets_api, [xpm_element(data)])

        with allure.step(f"Verify the XPM parser error: {expected_error}"):
            assets_api.assert_status(response, 400)
            assert expected_error in response.text, (
                f"Expected {expected_error!r}, got {response.text!r}"
            )

    @allure.title("XPM API limit is enforced: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "data", "display", "expected_error"),
        [
            pytest.param(
                "33 colors",
                colors_xpm(33),
                "front",
                "elements[0].data value is invalid; must have <= 32 colors and <= 4 chars per pixel",
                id="colors",
            ),
            pytest.param(
                "cpp 5",
                solid_xpm(1, 1, key="ABCDE"),
                "front",
                "elements[0].data value is invalid; must have <= 32 colors and <= 4 chars per pixel",
                id="cpp",
            ),
            pytest.param(
                "front width 73",
                solid_xpm(FRONT_DISPLAY_WIDTH + 1, 1),
                "front",
                f"elements[0].data value is invalid; must be smaller than the display ({FRONT_DISPLAY_WIDTH}x{FRONT_DISPLAY_HEIGHT})",
                id="front-width",
            ),
            pytest.param(
                "back width 161",
                solid_xpm(BACK_DISPLAY_WIDTH + 1, 1),
                "back",
                f"elements[0].data value is invalid; must be smaller than the display ({BACK_DISPLAY_WIDTH}x{BACK_DISPLAY_HEIGHT})",
                id="back-width",
            ),
            pytest.param(
                "front height 17",
                solid_xpm(1, FRONT_DISPLAY_HEIGHT + 1),
                "front",
                f"elements[0].data value is invalid; must be smaller than the display ({FRONT_DISPLAY_WIDTH}x{FRONT_DISPLAY_HEIGHT})",
                id="front-height",
            ),
            pytest.param(
                "back height 81",
                solid_xpm(1, BACK_DISPLAY_HEIGHT + 1),
                "back",
                f"elements[0].data value is invalid; must be smaller than the display ({BACK_DISPLAY_WIDTH}x{BACK_DISPLAY_HEIGHT})",
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
        expected_error: str,
    ):
        with allure.step(f"Submit otherwise valid over-limit XPM: {case}"):
            response = _draw(assets_api, [xpm_element(data, display=display)])

        with allure.step(f"Verify the API limit error: {expected_error}"):
            assets_api.assert_status(response, 400)
            assert expected_error in response.text, (
                f"Expected {expected_error!r}, got {response.text!r}"
            )

    @allure.title("XPM API boundary is accepted: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "data", "display"),
        [
            pytest.param(
                "32 colors", colors_xpm(32), "front", id="colors"
            ),
            pytest.param(
                "cpp 4", solid_xpm(1, 1, key="ABCD"), "front", id="cpp"
            ),
            pytest.param(
                "front dimensions",
                solid_xpm(FRONT_DISPLAY_WIDTH, FRONT_DISPLAY_HEIGHT),
                "front",
                id="front-dimensions",
            ),
            pytest.param(
                "back dimensions",
                solid_xpm(BACK_DISPLAY_WIDTH, BACK_DISPLAY_HEIGHT),
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
        with allure.step(f"Submit XPM at the supported boundary: {case}"):
            response = _draw(assets_api, [xpm_element(data, display=display)])

        with allure.step("Verify the boundary value is accepted"):
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
        with allure.step(f"Submit XPM with invalid opacity {opacity}"):
            response = _draw(
                assets_api, [xpm_element(self._FRONT_DATA, opacity=opacity)]
            )

        with allure.step("Verify the opacity validation error"):
            assets_api.assert_status(response, 400)
            assert "elements[0].opacity value is invalid; must be in range [0; 100]" in response.text, (
                f"Unexpected invalid-opacity response: {response.text!r}"
            )

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
        blue_data = pixel_xpm("#0000FF")
        elements = [
            solid_rectangle(
                "background",
                FILL_RED,
                x=x,
                y=y,
                width=1,
                height=1,
                timeout=5,
                align="top_left",
                z_index=0,
            ),
            xpm_element(blue_data, x=x, y=y, opacity=0, z_index=1),
        ]

        with allure.step("Draw an invisible blue XPM over a red rectangle"):
            response = _draw(assets_api, elements)
            assets_api.assert_status(response, 200)

        with allure.step("Verify the red background remains visible"):
            assert_front_pixels(
                streaming_api, {(x, y): RGB_RED}, "Zero-opacity XPM"
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
        red_data = pixel_xpm("#FF0000")
        x, y = 3, 4
        background = solid_rectangle(
            "background",
            FILL_BLACK,
            x=x,
            y=y,
            width=1,
            height=1,
            timeout=5,
            align="top_left",
            z_index=0,
        )

        with allure.step(
            "Render the control pixel over black at full opacity"
        ):
            response = _draw(
                assets_api,
                [
                    background,
                    xpm_element(red_data, x=x, y=y, opacity=100, z_index=1),
                ],
            )
            assets_api.assert_status(response, 200)
            full_frame = assert_front_pixels(
                streaming_api, {(x, y): RGB_RED}, "Full-opacity XPM"
            )
            full = full_frame.pixel(x, y)

        with allure.step("Render the same pixel over black at half opacity"):
            response = _draw(
                assets_api,
                [
                    background,
                    xpm_element(red_data, x=x, y=y, opacity=50, z_index=1),
                ],
            )
            assets_api.assert_status(response, 200)
            expected_half_red = full[0] * 50 // 100

            def is_half_red(current: FrontFrame) -> bool:
                pixel = current.pixel(x, y)
                return (
                    abs(pixel[0] - expected_half_red) <= 2
                    and pixel[1:] == (0, 0)
                )

            half_frame = wait_for_front_frame(
                streaming_api, is_half_red, "half-opacity red XPM pixel"
            )
            half = half_frame.pixel(x, y)
            half_frame.attach("Half-opacity XPM")

        with allure.step("Verify opacity reduced only the red channel"):
            assert full == RGB_RED, (
                f"Unexpected full-opacity pixel: {full!r}"
            )
            assert abs(half[0] - expected_half_red) <= 2, (
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
        data = xpm_source(
            2, 1, {"T": "c none", "G": "c #00FF00"}, ["TG"]
        )
        elements = [
            solid_rectangle(
                "background",
                FILL_RED,
                x=x,
                y=y,
                width=2,
                height=1,
                timeout=5,
                align="top_left",
                z_index=0,
            ),
            xpm_element(data, x=x, y=y, z_index=1),
        ]

        with allure.step("Draw transparent and green XPM pixels over red"):
            response = _draw(assets_api, elements)
            assets_api.assert_status(response, 200)

        with allure.step("Verify transparent composition"):
            assert_front_pixels(
                streaming_api,
                {(x, y): RGB_RED, (x + 1, y): RGB_GREEN},
                "Transparent XPM composition",
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
        red_data = pixel_xpm("#FF0000")
        elements = [
            xpm_element(red_data, x=x, y=y),
            solid_rectangle(
                "companion",
                FILL_GREEN,
                x=x + 2,
                y=y,
                width=1,
                height=1,
                timeout=5,
                align="top_left",
            ),
        ]

        with allure.step("Draw XPM and rectangle elements together"):
            response = _draw(assets_api, elements)
            assets_api.assert_status(response, 200)

        with allure.step("Verify both element types reached the framebuffer"):
            assert_front_pixels(
                streaming_api,
                {(x, y): RGB_RED, (x + 2, y): RGB_GREEN},
                "Mixed XPM and rectangle",
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
        background = solid_rectangle(
            "background",
            FILL_BLACK,
            x=x,
            y=y,
            width=1,
            height=1,
            timeout=5,
            align="top_left",
            z_index=0,
        )

        with allure.step("Draw and verify a red XPM image"):
            red_data = pixel_xpm("#FF0000")
            response = _draw(
                assets_api,
                [background, xpm_element(red_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            assert_front_pixels(
                streaming_api, {(x, y): RGB_RED}, "XPM lifecycle: red"
            )

        with allure.step("Replace it with and verify a green XPM image"):
            green_data = pixel_xpm("#00FF00")
            response = _draw(
                assets_api,
                [background, xpm_element(green_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            assert_front_pixels(
                streaming_api, {(x, y): RGB_GREEN}, "XPM lifecycle: green"
            )

        with allure.step("Remove the XPM image and verify its background"):
            assets_api.clear_display_elements(["xpm1"], app_name=_APP)
            assert_front_pixels(
                streaming_api, {(x, y): RGB_BLACK}, "XPM lifecycle: cleared"
            )

        with allure.step("Draw a blue XPM after clearing"):
            blue_data = pixel_xpm("#0000FF")
            response = _draw(
                assets_api,
                [background, xpm_element(blue_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            assert_front_pixels(
                streaming_api, {(x, y): RGB_BLUE}, "XPM redraw lifecycle"
            )

    @allure.title("Shrinking an XPM element clears its previous bounds")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_shrinking_xpm_clears_previous_bounds(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 3, 4
        background = solid_rectangle(
            "background",
            FILL_BLACK,
            x=x,
            y=y,
            width=2,
            height=1,
            align="top_left",
            z_index=0,
        )
        red_data = solid_xpm(2, 1, color="#FF0000")
        green_data = pixel_xpm("#00FF00")

        with allure.step("Draw and verify a red 2x1 XPM image"):
            response = _draw(
                assets_api,
                [
                    background,
                    xpm_element(red_data, x=x, y=y, timeout=0, z_index=1),
                ],
            )
            assets_api.assert_status(response, 200)
            assert_front_pixels(
                streaming_api,
                {(x, y): RGB_RED, (x + 1, y): RGB_RED},
                "XPM before geometry shrink",
            )

        with allure.step("Replace it with a green 1x1 XPM using the same ID"):
            response = _draw(
                assets_api,
                [xpm_element(green_data, x=x, y=y, timeout=0, z_index=1)],
            )
            assets_api.assert_status(response, 200)

        with allure.step(
            "Verify the retired pixel returns to the black background"
        ):
            assert_front_pixels(
                streaming_api,
                {(x, y): RGB_GREEN, (x + 1, y): RGB_BLACK},
                "XPM after geometry shrink",
            )

    @allure.title("XPM clipped at a display edge renders the visible part: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "x", "visible_x"),
        [
            pytest.param("left edge", -1, 0, id="left-edge"),
            pytest.param(
                "right edge",
                FRONT_DISPLAY_WIDTH - 1,
                FRONT_DISPLAY_WIDTH - 1,
                id="right-edge",
            ),
        ],
    )
    def test_clipped_rendering(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
        case: str,
        x: int,
        visible_x: int,
    ):
        y = 4
        red_data = solid_xpm(2, 1, color="#FF0000")

        with allure.step(f"Draw a 2x1 XPM partially off-screen: {case}"):
            response = _draw(
                assets_api, [xpm_element(red_data, x=x, y=y)]
            )
            assets_api.assert_status(response, 200)

        with allure.step("Verify the on-screen pixel renders"):
            assert_front_pixels(
                streaming_api, {(visible_x, y): RGB_RED}, f"Clipped XPM ({case})"
            )

    @allure.title("XPM color spec variants render: {visual} {color}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("color", "visual", "expected"),
        [
            pytest.param("red", "c", RGB_RED, id="named-red"),
            pytest.param("white", "c", RGB_WHITE, id="named-white"),
            pytest.param("#FFFFFF", "g", RGB_WHITE, id="visual-g"),
            pytest.param("#FFFFFF", "m", RGB_WHITE, id="visual-m"),
        ],
    )
    def test_color_spec_variants(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
        color: str,
        visual: str,
        expected: tuple[int, int, int],
    ):
        x, y = 3, 4
        spec = f"{visual} {color}"
        data = pixel_xpm(color, visual=visual)

        with allure.step(f"Draw a one-pixel XPM colored {spec!r}"):
            response = _draw(assets_api, [xpm_element(data, x=x, y=y)])
            assets_api.assert_status(response, 200)

        with allure.step(f"Verify the pixel renders as {expected}"):
            assert_front_pixels(
                streaming_api, {(x, y): expected}, f"XPM color spec ({spec})"
            )

    @allure.title("Rectangle above XPM wins the overlapping pixel by z_index")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_rectangle_over_xpm_z_order(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
    ):
        x, y = 3, 4
        red_data = pixel_xpm("#FF0000")
        elements = [
            xpm_element(red_data, x=x, y=y, z_index=1),
            solid_rectangle(
                "cover",
                FILL_GREEN,
                x=x,
                y=y,
                width=1,
                height=1,
                timeout=5,
                align="top_left",
                z_index=2,
            ),
        ]

        with allure.step("Draw a green rectangle above a red XPM"):
            response = _draw(assets_api, elements)
            assets_api.assert_status(response, 200)

        with allure.step("Verify the rectangle covers the XPM pixel"):
            assert_front_pixels(
                streaming_api, {(x, y): RGB_GREEN}, "Rectangle over XPM"
            )
