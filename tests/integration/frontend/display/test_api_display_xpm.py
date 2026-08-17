"""Integration tests for XPM2 elements in POST /api/display/draw."""

from __future__ import annotations

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
from .display_helpers import (
    attach_frame,
    back_pixel,
    front_pixel,
    solid_rectangle,
    wait_for_back_frame,
    wait_for_front_frame,
)

_APP = "xpm_test_app"


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


def _colors_xpm(count: int) -> str:
    """Build a valid one-pixel XPM2 image with the requested color count."""
    keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    assert count <= len(keys)
    keys = keys[:count]
    colors = "\n".join(f"{key} c #FFFFFF" for key in keys)
    return f"! XPM2\n1 1 {len(keys)} 1\n{colors}\n{keys[0]}"


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

        with allure.step("Verify the four rendered RGB pixels"):
            expected = {
                "red": (255, 0, 0),
                "green": (0, 255, 0),
                "blue": (0, 0, 255),
                "white": (255, 255, 255),
            }

            def rendered_pixels(
                frame: bytes,
            ) -> dict[str, tuple[int, int, int]]:
                return {
                    "red": front_pixel(frame, x, y),
                    "green": front_pixel(frame, x + 1, y),
                    "blue": front_pixel(frame, x, y + 1),
                    "white": front_pixel(frame, x + 1, y + 1),
                }

            frame = wait_for_front_frame(
                streaming_api,
                lambda current: rendered_pixels(current) == expected,
                "four rendered XPM colors",
            )
            attach_frame(frame, 0, "Front XPM colors")
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

        with allure.step(
            "Draw adjacent black and white pixels on the back display"
        ):
            response = _draw(
                assets_api,
                [_xpm(self._BACK_DATA, display="back", x=x, y=y)],
            )
            assets_api.assert_status(response, 200)

        with allure.step("Verify the nibble-packed luma values"):
            def rendered_pixels(frame: bytes) -> tuple[int, int]:
                return back_pixel(frame, x, y), back_pixel(frame, x + 1, y)

            frame = wait_for_back_frame(
                streaming_api,
                lambda current: rendered_pixels(current) == (0x0, 0xF),
                "black and white XPM luma pixels",
            )
            attach_frame(frame, 1, "Back XPM luma")
            actual = rendered_pixels(frame)
            assert actual == (0x0, 0xF), (
                f"Unexpected back XPM pixels: {actual!r}"
            )

    @allure.title("XPM bitmap requires data")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_data(self, assets_api: AssetsAPI, busy_timer_stopped):
        with allure.step("Submit an XPM element without data"):
            element = _xpm(self._FRONT_DATA)
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
            response = _draw(assets_api, [_xpm(data)])

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
                _colors_xpm(33),
                "front",
                "XPM header values exceed limits",
                id="colors",
            ),
            pytest.param(
                "cpp 5",
                _solid_xpm(1, 1, key="ABCDE"),
                "front",
                "XPM header values exceed limits",
                id="cpp",
            ),
            pytest.param(
                "front width 73",
                _solid_xpm(FRONT_DISPLAY_WIDTH + 1, 1),
                "front",
                "XPM image exceeds display dimensions",
                id="front-width",
            ),
            pytest.param(
                "back width 161",
                _solid_xpm(BACK_DISPLAY_WIDTH + 1, 1),
                "back",
                "XPM image exceeds display dimensions",
                id="back-width",
            ),
            pytest.param(
                "front height 17",
                _solid_xpm(1, FRONT_DISPLAY_HEIGHT + 1),
                "front",
                "XPM image exceeds display dimensions",
                id="front-height",
            ),
            pytest.param(
                "back height 81",
                _solid_xpm(1, BACK_DISPLAY_HEIGHT + 1),
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
            response = _draw(assets_api, [_xpm(data, display=display)])

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
                "32 colors", _colors_xpm(32), "front", id="colors"
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
        with allure.step(f"Submit XPM at the supported boundary: {case}"):
            response = _draw(assets_api, [_xpm(data, display=display)])

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
                assets_api, [_xpm(self._FRONT_DATA, opacity=opacity)]
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
        blue_data = "! XPM2\n1 1 1 1\nB c #0000FF\nB"
        elements = [
            solid_rectangle(
                "background",
                "#FF0000FF",
                x=x,
                y=y,
                width=1,
                height=1,
                timeout=5,
                align="top_left",
                z_index=0,
            ),
            _xpm(blue_data, x=x, y=y, opacity=0, z_index=1),
        ]

        with allure.step("Draw an invisible blue XPM over a red rectangle"):
            response = _draw(assets_api, elements)
            assets_api.assert_status(response, 200)

        with allure.step("Verify the red background remains visible"):
            frame = wait_for_front_frame(
                streaming_api,
                lambda current: front_pixel(current, x, y) == (255, 0, 0),
                "red background below zero-opacity XPM",
            )
            attach_frame(frame, 0, "Zero-opacity XPM")
            actual = front_pixel(frame, x, y)
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
        background = solid_rectangle(
            "background",
            "#000000FF",
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
                    _xpm(
                        red_data,
                        x=x,
                        y=y,
                        opacity=100,
                        z_index=1,
                    ),
                ],
            )
            assets_api.assert_status(response, 200)
            full_frame = wait_for_front_frame(
                streaming_api,
                lambda current: front_pixel(current, x, y) == (255, 0, 0),
                "full-opacity red XPM pixel",
            )
            full = front_pixel(full_frame, x, y)

        with allure.step("Render the same pixel over black at half opacity"):
            response = _draw(
                assets_api,
                [
                    background,
                    _xpm(
                        red_data,
                        x=x,
                        y=y,
                        opacity=50,
                        z_index=1,
                    ),
                ],
            )
            assets_api.assert_status(response, 200)
            expected_half_red = full[0] * 50 // 100
            half_frame = wait_for_front_frame(
                streaming_api,
                lambda current: (
                    abs(front_pixel(current, x, y)[0] - expected_half_red) <= 2
                    and front_pixel(current, x, y)[1:] == (0, 0)
                ),
                "half-opacity red XPM pixel",
            )
            half = front_pixel(half_frame, x, y)
            attach_frame(half_frame, 0, "Half-opacity XPM")

        with allure.step("Verify opacity reduced only the red channel"):
            assert full == (255, 0, 0), (
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
        data = (
            "! XPM2\n"
            "2 1 2 1\n"
            "T c none\n"
            "G c #00FF00\n"
            "TG"
        )
        elements = [
            solid_rectangle(
                "background",
                "#FF0000FF",
                x=x,
                y=y,
                width=2,
                height=1,
                timeout=5,
                align="top_left",
                z_index=0,
            ),
            _xpm(data, x=x, y=y, z_index=1),
        ]

        with allure.step("Draw transparent and green XPM pixels over red"):
            response = _draw(assets_api, elements)
            assets_api.assert_status(response, 200)

        with allure.step("Verify transparent composition"):
            expected = ((255, 0, 0), (0, 255, 0))

            def pixels(frame: bytes) -> tuple[tuple[int, int, int], ...]:
                return (
                    front_pixel(frame, x, y),
                    front_pixel(frame, x + 1, y),
                )

            frame = wait_for_front_frame(
                streaming_api,
                lambda current: pixels(current) == expected,
                "transparent XPM composition",
            )
            attach_frame(frame, 0, "Transparent XPM composition")
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
        red_data = "! XPM2\n1 1 1 1\nR c #FF0000\nR"
        elements = [
            _xpm(red_data, x=x, y=y),
            solid_rectangle(
                "companion",
                "#00FF00FF",
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
            expected = ((255, 0, 0), (0, 255, 0))

            def pixels(frame: bytes) -> tuple[tuple[int, int, int], ...]:
                return (
                    front_pixel(frame, x, y),
                    front_pixel(frame, x + 2, y),
                )

            frame = wait_for_front_frame(
                streaming_api,
                lambda current: pixels(current) == expected,
                "mixed XPM and rectangle pixels",
            )
            attach_frame(frame, 0, "Mixed XPM and rectangle")
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
            "#000000FF",
            x=x,
            y=y,
            width=1,
            height=1,
            timeout=5,
            align="top_left",
            z_index=0,
        )

        with allure.step("Draw and verify a red XPM image"):
            red_data = "! XPM2\n1 1 1 1\nX c #FF0000\nX"
            response = _draw(
                assets_api,
                [background, _xpm(red_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            red_frame = wait_for_front_frame(
                streaming_api,
                lambda current: front_pixel(current, x, y) == (255, 0, 0),
                "red XPM pixel before replacement",
            )
            red = front_pixel(red_frame, x, y)
            assert red == (255, 0, 0), (
                f"Initial XPM was not rendered: {red!r}"
            )

        with allure.step("Replace it with and verify a green XPM image"):
            green_data = "! XPM2\n1 1 1 1\nX c #00FF00\nX"
            response = _draw(
                assets_api,
                [background, _xpm(green_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            green_frame = wait_for_front_frame(
                streaming_api,
                lambda current: front_pixel(current, x, y) == (0, 255, 0),
                "green replacement XPM pixel",
            )
            green = front_pixel(green_frame, x, y)
            assert green == (0, 255, 0), (
                f"XPM replacement was not rendered: {green!r}"
            )

        with allure.step("Remove the XPM image and verify its background"):
            assets_api.clear_display_elements(["xpm1"], app_name=_APP)
            cleared_frame = wait_for_front_frame(
                streaming_api,
                lambda current: front_pixel(current, x, y) == (0, 0, 0),
                "black background after removing XPM",
            )
            cleared = front_pixel(cleared_frame, x, y)
            assert cleared == (0, 0, 0), (
                f"XPM pixels remained after clear: {cleared!r}"
            )

        with allure.step("Draw a blue XPM after clearing"):
            blue_data = "! XPM2\n1 1 1 1\nX c #0000FF\nX"
            response = _draw(
                assets_api,
                [background, _xpm(blue_data, x=x, y=y, z_index=1)],
            )
            assets_api.assert_status(response, 200)
            blue_frame = wait_for_front_frame(
                streaming_api,
                lambda current: front_pixel(current, x, y) == (0, 0, 255),
                "blue XPM after clearing",
            )
            attach_frame(blue_frame, 0, "XPM redraw lifecycle")
            blue = front_pixel(blue_frame, x, y)
            assert blue == (0, 0, 255), (
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
            "#000000FF",
            x=x,
            y=y,
            width=2,
            height=1,
            align="top_left",
            z_index=0,
        )
        red_data = "! XPM2\n2 1 1 1\nX c #FF0000\nXX"
        green_data = "! XPM2\n1 1 1 1\nX c #00FF00\nX"

        def pixels(frame: bytes) -> tuple[tuple[int, int, int], ...]:
            return (
                front_pixel(frame, x, y),
                front_pixel(frame, x + 1, y),
            )

        with allure.step("Draw and verify a red 2x1 XPM image"):
            response = _draw(
                assets_api,
                [
                    background,
                    _xpm(red_data, x=x, y=y, timeout=0, z_index=1),
                ],
            )
            assets_api.assert_status(response, 200)
            initial_frame = wait_for_front_frame(
                streaming_api,
                lambda current: pixels(current)
                == ((255, 0, 0), (255, 0, 0)),
                "two red pixels before shrinking the XPM",
            )
            attach_frame(initial_frame, 0, "XPM before geometry shrink")

        with allure.step("Replace it with a green 1x1 XPM using the same ID"):
            response = _draw(
                assets_api,
                [_xpm(green_data, x=x, y=y, timeout=0, z_index=1)],
            )
            assets_api.assert_status(response, 200)

        with allure.step(
            "Verify the retired pixel returns to the black background"
        ):
            expected = ((0, 255, 0), (0, 0, 0))
            last_frame: bytes | None = None

            def resized_pixels_match(current: bytes) -> bool:
                nonlocal last_frame
                last_frame = current
                return pixels(current) == expected

            try:
                resized_frame = wait_for_front_frame(
                    streaming_api,
                    resized_pixels_match,
                    "green 1x1 XPM followed by its black background",
                )
            except AssertionError as error:
                assert last_frame is not None
                attach_frame(last_frame, 0, "Failed XPM geometry shrink")
                actual = pixels(last_frame)
                raise AssertionError(
                    f"Shrinking XPM left stale pixels: "
                    f"expected={expected!r}, actual={actual!r}"
                ) from error

            attach_frame(resized_frame, 0, "XPM after geometry shrink")
            actual = pixels(resized_frame)
            assert actual == expected, (
                f"Unexpected pixels after shrinking XPM: {actual!r}"
            )
