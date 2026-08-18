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
    colors_xpm,
    solid_rectangle,
    solid_xpm,
    wait_for_back_frame,
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
            expected = {
                "red": RGB_RED,
                "green": RGB_GREEN,
                "blue": RGB_BLUE,
                "white": RGB_WHITE,
            }

            def rendered_pixels(
                frame: FrontFrame,
            ) -> dict[str, tuple[int, int, int]]:
                return {
                    "red": frame.pixel(x, y),
                    "green": frame.pixel(x + 1, y),
                    "blue": frame.pixel(x, y + 1),
                    "white": frame.pixel(x + 1, y + 1),
                }

            frame = wait_for_front_frame(
                streaming_api,
                lambda current: rendered_pixels(current) == expected,
                "four rendered XPM colors",
            )
            frame.attach("Front XPM colors")
            actual = rendered_pixels(frame)
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
        # 0x8 pins the luma conversion itself: unlike black (canvas
        # background) and white, mid-gray cannot be satisfied by accident.
        expected = (0x0, 0x8, 0xF)

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
            def rendered_pixels(frame) -> tuple[int, int, int]:
                return (
                    frame.pixel(x, y),
                    frame.pixel(x + 1, y),
                    frame.pixel(x + 2, y),
                )

            frame = wait_for_back_frame(
                streaming_api,
                lambda current: rendered_pixels(current) == expected,
                "black, gray, and white XPM luma pixels",
            )
            frame.attach("Back XPM luma")
            actual = rendered_pixels(frame)
            assert actual == expected, (
                f"Unexpected back XPM pixels: {actual!r}"
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
            assert "Bad Request" in response.text, (
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
                "Failed to parse XPM header.",
                id="signature",
            ),
            pytest.param(
                "invalid header",
                "! XPM2\n1 1 1\n. c #FFFFFF\n.",
                "Failed to parse XPM header",
                id="header",
            ),
            pytest.param(
                "invalid color",
                "! XPM2\n1 1 1 1\n. c not-a-color\n.",
                "Failed to parse XPM color table",
                id="color",
            ),
            pytest.param(
                "unknown pixel key",
                "! XPM2\n1 1 1 1\n. c #FFFFFF\nX",
                "Failed to decode XPM pixel data",
                id="pixels",
            ),
            pytest.param(
                "pixel row shorter than width",
                xpm_source(3, 1, {".": "c #FFFFFF"}, [".."]),
                "Failed to decode XPM pixel data",
                id="short-row",
            ),
            pytest.param(
                "empty data",
                "",
                "Failed to parse XPM header",
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
                "XPM header values exceed limits",
                id="colors",
            ),
            pytest.param(
                "cpp 5",
                solid_xpm(1, 1, key="ABCDE"),
                "front",
                "XPM header values exceed limits",
                id="cpp",
            ),
            pytest.param(
                "front width 73",
                solid_xpm(FRONT_DISPLAY_WIDTH + 1, 1),
                "front",
                "XPM image exceeds display dimensions",
                id="front-width",
            ),
            pytest.param(
                "back width 161",
                solid_xpm(BACK_DISPLAY_WIDTH + 1, 1),
                "back",
                "XPM image exceeds display dimensions",
                id="back-width",
            ),
            pytest.param(
                "front height 17",
                solid_xpm(1, FRONT_DISPLAY_HEIGHT + 1),
                "front",
                "XPM image exceeds display dimensions",
                id="front-height",
            ),
            pytest.param(
                "back height 81",
                solid_xpm(1, BACK_DISPLAY_HEIGHT + 1),
                "back",
                "XPM image exceeds display dimensions",
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
            assert "Bad Request" in response.text, (
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
        blue_data = solid_xpm(1, 1, key="B", color="#0000FF")
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
            frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == RGB_RED,
                "red background below zero-opacity XPM",
            )
            frame.attach("Zero-opacity XPM")
            actual = frame.pixel(x, y)
            assert actual == RGB_RED, (
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
        red_data = solid_xpm(1, 1, key="R", color="#FF0000")
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
            full_frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == RGB_RED,
                "full-opacity red XPM pixel",
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
            expected = (RGB_RED, RGB_GREEN)

            def pixels(frame: FrontFrame) -> tuple[tuple[int, int, int], ...]:
                return frame.pixel(x, y), frame.pixel(x + 1, y)

            frame = wait_for_front_frame(
                streaming_api,
                lambda current: pixels(current) == expected,
                "transparent XPM composition",
            )
            frame.attach("Transparent XPM composition")
            actual = pixels(frame)
            assert actual == expected, (
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
        red_data = solid_xpm(1, 1, key="R", color="#FF0000")
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
            expected = (RGB_RED, RGB_GREEN)

            def pixels(frame: FrontFrame) -> tuple[tuple[int, int, int], ...]:
                return frame.pixel(x, y), frame.pixel(x + 2, y)

            frame = wait_for_front_frame(
                streaming_api,
                lambda current: pixels(current) == expected,
                "mixed XPM and rectangle pixels",
            )
            frame.attach("Mixed XPM and rectangle")
            actual = pixels(frame)
            assert actual == expected, (
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
            red_data = solid_xpm(1, 1, color="#FF0000")
            response = _draw(
                assets_api,
                [background, xpm_element(red_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            red_frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == RGB_RED,
                "red XPM pixel before replacement",
            )
            red = red_frame.pixel(x, y)
            assert red == RGB_RED, (
                f"Initial XPM was not rendered: {red!r}"
            )

        with allure.step("Replace it with and verify a green XPM image"):
            green_data = solid_xpm(1, 1, color="#00FF00")
            response = _draw(
                assets_api,
                [background, xpm_element(green_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            green_frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == RGB_GREEN,
                "green replacement XPM pixel",
            )
            green = green_frame.pixel(x, y)
            assert green == RGB_GREEN, (
                f"XPM replacement was not rendered: {green!r}"
            )

        with allure.step("Remove the XPM image and verify its background"):
            assets_api.clear_display_elements(["xpm1"], app_name=_APP)
            cleared_frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == RGB_BLACK,
                "black background after removing XPM",
            )
            cleared = cleared_frame.pixel(x, y)
            assert cleared == RGB_BLACK, (
                f"XPM pixels remained after clear: {cleared!r}"
            )

        with allure.step("Draw a blue XPM after clearing"):
            blue_data = solid_xpm(1, 1, color="#0000FF")
            response = _draw(
                assets_api,
                [background, xpm_element(blue_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            blue_frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == RGB_BLUE,
                "blue XPM after clearing",
            )
            blue_frame.attach("XPM redraw lifecycle")
            blue = blue_frame.pixel(x, y)
            assert blue == RGB_BLUE, (
                f"XPM redraw after clear failed: {blue!r}"
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
        green_data = solid_xpm(1, 1, color="#00FF00")

        def pixels(frame: FrontFrame) -> tuple[tuple[int, int, int], ...]:
            return frame.pixel(x, y), frame.pixel(x + 1, y)

        with allure.step("Draw and verify a red 2x1 XPM image"):
            response = _draw(
                assets_api,
                [
                    background,
                    xpm_element(red_data, x=x, y=y, timeout=0, z_index=1),
                ],
            )
            assets_api.assert_status(response, 200)
            initial_frame = wait_for_front_frame(
                streaming_api,
                lambda current: pixels(current) == (RGB_RED, RGB_RED),
                "two red pixels before shrinking the XPM",
            )
            initial_frame.attach("XPM before geometry shrink")

        with allure.step("Replace it with a green 1x1 XPM using the same ID"):
            response = _draw(
                assets_api,
                [xpm_element(green_data, x=x, y=y, timeout=0, z_index=1)],
            )
            assets_api.assert_status(response, 200)

        with allure.step(
            "Verify the retired pixel returns to the black background"
        ):
            expected = (RGB_GREEN, RGB_BLACK)
            resized_frame = wait_for_front_frame(
                streaming_api,
                lambda current: pixels(current) == expected,
                "green 1x1 XPM followed by its black background",
            )
            resized_frame.attach("XPM after geometry shrink")
            actual = pixels(resized_frame)
            assert actual == expected, (
                f"Unexpected pixels after shrinking XPM: {actual!r}"
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
            frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(visible_x, y) == RGB_RED,
                f"clipped XPM pixel at x={visible_x}",
            )
            frame.attach(f"Clipped XPM ({case})")
            actual = frame.pixel(visible_x, y)
            assert actual == RGB_RED, (
                f"Clipped XPM did not render its visible pixel: {actual!r}"
            )

    @allure.title("XPM color spec variants render: {case}")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        ("case", "data", "expected"),
        [
            pytest.param(
                "named color red",
                xpm_source(1, 1, {"R": "c red"}, ["R"]),
                RGB_RED,
                id="named-red",
            ),
            pytest.param(
                "named color white",
                xpm_source(1, 1, {"W": "c white"}, ["W"]),
                RGB_WHITE,
                id="named-white",
            ),
            pytest.param(
                "grayscale visual",
                xpm_source(1, 1, {"G": "g #FFFFFF"}, ["G"]),
                RGB_WHITE,
                id="visual-g",
            ),
            pytest.param(
                "monochrome visual",
                xpm_source(1, 1, {"M": "m #FFFFFF"}, ["M"]),
                RGB_WHITE,
                id="visual-m",
            ),
        ],
    )
    def test_color_spec_variants(
        self,
        assets_api: AssetsAPI,
        streaming_api: StreamingAPI,
        busy_timer_stopped,
        case: str,
        data: str,
        expected: tuple[int, int, int],
    ):
        x, y = 3, 4

        with allure.step(f"Draw a one-pixel XPM using a {case}"):
            response = _draw(assets_api, [xpm_element(data, x=x, y=y)])
            assets_api.assert_status(response, 200)

        with allure.step(f"Verify the pixel renders as {expected}"):
            frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == expected,
                f"{case} XPM pixel",
            )
            frame.attach(f"XPM color spec ({case})")
            actual = frame.pixel(x, y)
            assert actual == expected, (
                f"Unexpected {case} pixel: {actual!r}"
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
        red_data = solid_xpm(1, 1, color="#FF0000")
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
            frame = wait_for_front_frame(
                streaming_api,
                lambda current: current.pixel(x, y) == RGB_GREEN,
                "rectangle covering the XPM pixel",
            )
            frame.attach("Rectangle over XPM")
            actual = frame.pixel(x, y)
            assert actual == RGB_GREEN, (
                f"Rectangle above XPM did not win the pixel: {actual!r}"
            )
