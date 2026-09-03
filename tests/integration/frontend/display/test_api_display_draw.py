"""
Integration tests for the Draw/Canvas HTTP API (POST/DELETE /api/display/draw)
and Brightness API (GET/POST /api/display/brightness).

These tests verify parameter handling, required/optional fields, enum values,
type-specific element validation, response codes, and the delete/brightness
endpoints.

Priority-specific behaviour is tested in test_api_display_priority.py.

Endpoints under test
--------------------
POST   /api/display/draw       – draw elements on display
DELETE /api/display/draw       – clear elements
GET    /api/display/brightness – read current brightness
POST   /api/display/brightness – set brightness

Firmware implementation: api_display.c
OpenAPI schema:          openapi.yaml  (DisplayElements, DisplayElement, ...)
"""

from __future__ import annotations

import time

import allure
import pytest
import requests

from clients.api.assets import (
    AssetsAPI,
    DEFAULT_ELEMENT_PRIORITY,
)
from clients.api import StreamingAPI

_RENDER_SETTLE = 0.5  # seconds to wait after draw before capturing screenshot
_MISSING_IMAGE = "nonexistent/does_not_exist.image"
_MISSING_ANIM = "nonexistent/does_not_exist.anim"
_VALID_TEXT_FONTS = [
    "tiny",
    "small",
    "normal",
    "condensed",
    "bold",
    "large",
    "extra_large",
    "global",
]


def _capture_after_draw(
    assets_api: AssetsAPI,
    streaming_api: StreamingAPI,
    elements: list[dict],
    display: int = 0,
) -> bytes:
    """Draw elements, wait for render, return display screenshot bytes."""
    _draw(assets_api, elements)
    time.sleep(_RENDER_SETTLE)
    return streaming_api.get_screen_bytes(display=display)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

_APP = "draw_test_app"
_PRI = DEFAULT_ELEMENT_PRIORITY  # 50 – always passes when idle (loader prio=10)


def _draw(api: AssetsAPI, elements: list[dict], **kw) -> requests.Response:
    """Post a draw request and return the raw response."""
    return api.draw_response(_APP, elements, priority=_PRI, **kw)


def _draw_raw(api: AssetsAPI, body: dict) -> requests.Response:
    """Post an arbitrary JSON body and return the raw response."""
    return api.draw_raw(body)


def _text(overrides: dict | None = None, **extra) -> dict:
    """Build a minimal valid text element, applying *overrides*."""
    base = {"id": "t1", "type": "text", "text": "hello", "font": "small", "timeout": 5}
    if overrides:
        base.update(overrides)
    base.update(extra)
    return base


def _countdown(overrides: dict | None = None, **extra) -> dict:
    """Build a minimal valid countdown element."""
    base = {
        "id": "cd1",
        "type": "countdown",
        "timestamp": "1700000000",
        "direction": "time_left",
        "show_hours": "when_non_zero",
        "timeout": 5,
    }
    if overrides:
        base.update(overrides)
    base.update(extra)
    return base


# Smallest real shared assets on the device (from /ext/shared/images and /ext/shared/animations).
# stock_path resolution: firmware takes the filename after the last "/" and
# looks it up in /ext/shared/images/ (image) or /ext/shared/animations/ (anim).
_BUILTIN_IMAGE = "shared/checkmark_front_8x8.image"  # 28 bytes
_BUILTIN_ANIM = "shared/spinner_front_8x8.anim"  # 2985 bytes


def _image(overrides: dict | None = None, **extra) -> dict:
    """Build a minimal valid image element using a real builtin image."""
    base = {
        "id": "img1",
        "type": "image",
        "stock_path": _BUILTIN_IMAGE,
        "timeout": 5,
    }
    if overrides:
        base.update(overrides)
    base.update(extra)
    return base


def _anim(overrides: dict | None = None, **extra) -> dict:
    """Build a minimal valid anim element using a real builtin animation."""
    base = {"id": "a1", "type": "animation", "stock_path": _BUILTIN_ANIM, "timeout": 5}
    if overrides:
        base.update(overrides)
    base.update(extra)
    return base


# ───────────────────────────────────────────────────────────────────────────
# TestDrawRequestValidation — top-level request body validation
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – Request Validation")
class TestDrawRequestValidation:
    """
    Validation of the top-level POST /api/display/draw request body:
    app_id (required), elements (required, non-empty array), priority range.
    """

    @allure.title("Missing app_id → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_app_id(self, assets_api: AssetsAPI):
        resp = _draw_raw(assets_api, {"elements": [_text()]})
        assets_api.assert_status(resp, 400)

    @allure.title("Missing elements → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_elements(self, assets_api: AssetsAPI):
        resp = _draw_raw(assets_api, {"application_name": _APP, "priority": _PRI})
        assets_api.assert_status(resp, 400)

    @allure.title("Elements not an array → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_elements_not_array(self, assets_api: AssetsAPI):
        resp = _draw_raw(
            assets_api, {"application_name": _APP, "elements": "oops", "priority": _PRI}
        )
        assets_api.assert_status(resp, 400)

    @allure.title("Empty elements array → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_empty_elements(self, assets_api: AssetsAPI):
        resp = _draw_raw(assets_api, {"application_name": _APP, "elements": [], "priority": _PRI})
        assets_api.assert_status(resp, 400)

    @allure.title("Empty JSON body → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_empty_body(self, assets_api: AssetsAPI):
        resp = _draw_raw(assets_api, {})
        assets_api.assert_status(resp, 400)

    @allure.title("Priority boundary 1 (min valid) → accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_min_valid(self, assets_api: AssetsAPI, busy_timer_stopped):
        """priority=1 is the minimum accepted value; with loader prio=10 it may
        be blocked by priority gating (409), but it must NOT return 400."""
        resp = assets_api.draw_response(_APP, [_text()], priority=1)
        assert resp.status_code in (200, 409)

    @allure.title("Priority boundary 100 (max valid) → 200")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_max_valid(self, assets_api: AssetsAPI, busy_timer_stopped):
        resp = assets_api.draw_response(_APP, [_text()], priority=100)
        assets_api.assert_status(resp, 200)

    @allure.title("Priority 0 → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_zero(self, assets_api: AssetsAPI):
        try:
            resp = assets_api.draw_response(_APP, [_text()], priority=0)
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass  # firmware may reject without response

    @allure.title("Priority 101 → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_above_max(self, assets_api: AssetsAPI):
        try:
            resp = assets_api.draw_response(_APP, [_text()], priority=101)
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass

    @allure.title("Negative priority → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_priority_negative(self, assets_api: AssetsAPI):
        try:
            resp = assets_api.draw_response(_APP, [_text()], priority=-5)
            assets_api.assert_status(resp, 400)
        except requests.exceptions.ReadTimeout:
            pass


# ───────────────────────────────────────────────────────────────────────────
# TestCommonElementFields — fields shared by all element types
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – Common Element Fields")
class TestCommonElementFields:
    """
    Every element must have ``id`` (string) and ``type`` (enum). Optional
    common fields: timeout, display_until (mutually exclusive), x, y, align,
    display.
    """

    @allure.title("Missing id → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_id(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text()
        del elem["id"]
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Missing type → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_type(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text()
        del elem["type"]
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Unknown type → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_unknown_type(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(type="sparkline")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("timeout > 0 and display_until > 0 are mutually exclusive → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_timeout_and_display_until_mutex(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        elem = _text(timeout=10, display_until="1900000000")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("timeout=0 with display_until is accepted (0 ≡ unset)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_timeout_zero_with_display_until(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        """timeout defaults to 0 when omitted. Providing display_until alongside
        timeout=0 must not trigger the exclusivity check."""
        elem = _text(timeout=0, display_until="1900000000")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("display_until without timeout is accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_display_until_only(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text()
        del elem["timeout"]
        elem["display_until"] = "1900000000"
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Valid align values are accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "align",
        [
            "top_left",
            "top_mid",
            "top_right",
            "mid_left",
            "center",
            "mid_right",
            "bottom_left",
            "bottom_mid",
            "bottom_right",
        ],
    )
    def test_valid_align_values(
        self, assets_api: AssetsAPI, busy_timer_stopped, align: str
    ):
        elem = _text(align=align)
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Invalid align → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_invalid_align(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(align="diagonal")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("display=front accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_display_front(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(display="front")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("display=back accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_display_back(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(display="back")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Invalid display value → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_invalid_display(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(display="side")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("x and y accept integers")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_x_and_y(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(x=10, y=20)
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)


# ───────────────────────────────────────────────────────────────────────────
# TestTextElement — text-element-specific fields
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – Text Element")
class TestTextElement:
    """
    Text element requires ``text`` (string) and ``font`` (enum).
    Optional: color (#RRGGBBAA), width (>0), scroll_rate (>=0).
    """

    @allure.title("Valid text element with all fields")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_full_text_element(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(
            x=10,
            y=5,
            align="center",
            color="#FF0000FF",
            width=72,
            scroll_rate=1000,
            display="front",
        )
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Missing text field → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_text_field(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text()
        del elem["text"]
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Missing font field → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_font(self, assets_api: AssetsAPI, busy_timer_stopped):
        """Font is required by the firmware parser (null font → parse break)."""
        elem = _text()
        del elem["font"]
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Valid font enum values are accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("font", _VALID_TEXT_FONTS)
    def test_valid_font_values(
        self, assets_api: AssetsAPI, busy_timer_stopped, font: str
    ):
        elem = _text(font=font)
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Invalid font value → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_invalid_font(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(font="comic_sans")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Valid color (#RRGGBBAA) accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_valid_color(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(color="#AABBCCDD")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Invalid color string → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_invalid_color(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(color="red")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("width=0 → 400 (must be > 0)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_width_zero_rejected(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(width=0)
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Positive width accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_positive_width(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(width=72)
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("scroll_rate=0 accepted (no scrolling)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_scroll_rate_zero(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(scroll_rate=0, width=72)
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Negative scroll_rate → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_negative_scroll_rate(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text(scroll_rate=-1, width=72)
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Color field is optional (omitted → default white)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_color_optional(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _text()
        assert "color" not in elem
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)


# ───────────────────────────────────────────────────────────────────────────
# TestCountdownElement
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – Countdown Element")
class TestCountdownElement:
    """
    Countdown requires ``timestamp`` (string), ``direction`` (enum),
    ``show_hours`` (enum). Optional: color.
    """

    @allure.title("Valid countdown element")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_valid_countdown(self, assets_api: AssetsAPI, busy_timer_stopped):
        resp = _draw(assets_api, [_countdown()])
        assets_api.assert_status(resp, 200)

    @allure.title("Missing timestamp → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_timestamp(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _countdown()
        del elem["timestamp"]
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Missing direction → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_direction(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _countdown()
        del elem["direction"]
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Missing show_hours → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_missing_show_hours(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _countdown()
        del elem["show_hours"]
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Unrecognised direction → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_unrecognised_direction_rejected(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        elem = _countdown(direction="backward")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("direction=time_since accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_direction_time_since(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _countdown(direction="time_since")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Unrecognised show_hours → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_unrecognised_show_hours_rejected(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        elem = _countdown(show_hours="never")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("show_hours=always accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_show_hours_always(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _countdown(show_hours="always")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Countdown with color accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_countdown_color(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _countdown(color="#00FF00FF")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 200)

    @allure.title("Countdown with invalid color → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_countdown_invalid_color(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _countdown(color="green")
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)


# ───────────────────────────────────────────────────────────────────────────
# TestImageElement
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – Image Element")
class TestImageElement:
    """
    Image requires exactly one of ``path`` or ``stock_path``.
    Providing both or neither is rejected.
    """

    @allure.title("Image with path accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_image_with_path(self, assets_api: AssetsAPI, busy_timer_stopped):
        resp = _draw(assets_api, [_image()])
        # 200 if the canvas app can process it; never 400 for valid schema
        assert resp.status_code != 400, f"Unexpected 400: {resp.text}"

    @allure.title("Image with stock_path renders; missing asset is rejected with 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_image_with_builtin(
        self, assets_api: AssetsAPI, streaming_api: StreamingAPI, busy_timer_stopped
    ):
        valid_elem = {"id": "bi1", "type": "image", "stock_path": _BUILTIN_IMAGE, "timeout": 10}
        missing_elem = {"id": "bi1", "type": "image", "stock_path": _MISSING_IMAGE, "timeout": 10}

        # A missing image asset fails decoder validation and is rejected with 400.
        assets_api.assert_status(_draw(assets_api, [missing_elem]), 400)

        # The builtin image is accepted and renders non-blank pixels (vs a cleared display).
        assets_api.clear_display()
        time.sleep(_RENDER_SETTLE)
        blank_screen = streaming_api.get_screen_bytes(display=0)
        valid_screen = _capture_after_draw(assets_api, streaming_api, [valid_elem])

        assert valid_screen != blank_screen, (
            f"stock_path {_BUILTIN_IMAGE!r} rendered nothing — "
            "file may not exist on the device"
        )

    @allure.title("Image with both path and stock_path \u2192 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_image_both_path_and_builtin(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        elem = {
            "id": "bad1",
            "type": "image",
            "path": "nonexistent.png",
            "stock_path": _BUILTIN_IMAGE,
            "timeout": 5,
        }
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Image with neither path nor stock_path → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_image_no_source(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = {"id": "bad2", "type": "image", "timeout": 5}
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)


# ───────────────────────────────────────────────────────────────────────────
# TestAnimElement
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – Anim Element")
class TestAnimElement:
    """
    Animation requires exactly one of ``path`` or ``stock_path``.
    Optional: section (string), loop (bool), await_previous_end (bool).
    """

    @allure.title("Anim with path accepted")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_anim_with_path(self, assets_api: AssetsAPI, busy_timer_stopped):
        resp = _draw(assets_api, [_anim()])
        assert resp.status_code != 400, f"Unexpected 400: {resp.text}"

    @allure.title("Anim with stock_path renders different pixels than missing asset")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_anim_with_builtin(
        self, assets_api: AssetsAPI, streaming_api: StreamingAPI, busy_timer_stopped
    ):
        valid_elem = {"id": "ba1", "type": "animation", "stock_path": _BUILTIN_ANIM, "timeout": 10}
        missing_elem = {"id": "ba1", "type": "animation", "stock_path": _MISSING_ANIM, "timeout": 10}

        valid_screen = _capture_after_draw(assets_api, streaming_api, [valid_elem])
        missing_screen = _capture_after_draw(assets_api, streaming_api, [missing_elem])

        assert valid_screen != missing_screen, (
            f"stock_path {_BUILTIN_ANIM!r} rendered same pixels as a missing asset — "
            "file may not exist on the device"
        )

    @allure.title("Anim with both path and stock_path \u2192 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_anim_both_path_and_builtin(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        elem = {
            "id": "bad1",
            "type": "animation",
            "path": "nonexistent.anim",
            "stock_path": _BUILTIN_ANIM,
            "timeout": 5,
        }
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Anim with neither path nor stock_path \u2192 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_anim_no_source(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = {"id": "bad2", "type": "animation", "timeout": 5}
        resp = _draw(assets_api, [elem])
        assets_api.assert_status(resp, 400)

    @allure.title("Anim with optional section, loop, await_previous_end")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_anim_optional_fields(self, assets_api: AssetsAPI, busy_timer_stopped):
        elem = _anim(section="intro", loop=True, await_previous_end=True)
        resp = _draw(assets_api, [elem])
        assert resp.status_code != 400, f"Unexpected 400: {resp.text}"


# ───────────────────────────────────────────────────────────────────────────
# TestDrawMultipleElements — mixed payloads
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – Multiple Elements")
class TestDrawMultipleElements:
    """
    Draw requests may contain multiple elements of different types.
    If ANY element fails validation the whole request is rejected.
    """

    @allure.title("Multiple text elements in one request")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_multiple_text_elements(self, assets_api: AssetsAPI, busy_timer_stopped):
        elems = [
            _text(id="t1", text="first", x=0, y=0),
            _text(id="t2", text="second", x=0, y=20),
        ]
        resp = _draw(assets_api, elems)
        assets_api.assert_status(resp, 200)

    @allure.title("Mixed text + countdown in one request")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_mixed_text_countdown(self, assets_api: AssetsAPI, busy_timer_stopped):
        elems = [_text(), _countdown(id="cd2")]
        resp = _draw(assets_api, elems)
        assets_api.assert_status(resp, 200)

    @allure.title("One bad element in a batch rejects the whole request")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_one_bad_element_rejects_batch(
        self, assets_api: AssetsAPI, busy_timer_stopped
    ):
        good = _text(id="good")
        bad = {"id": "bad", "type": "text", "timeout": 5}  # missing text & font
        resp = _draw(assets_api, [good, bad])
        assets_api.assert_status(resp, 400)

    @allure.title("Later valid element doesn't rescue earlier invalid one")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_bad_first_element(self, assets_api: AssetsAPI, busy_timer_stopped):
        bad = {"id": "bad", "type": "text", "timeout": 5}
        good = _text(id="good")
        resp = _draw(assets_api, [bad, good])
        assets_api.assert_status(resp, 400)


# ───────────────────────────────────────────────────────────────────────────
# TestDeleteDisplay
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Draw API – DELETE")
class TestDeleteDisplay:
    """
    DELETE /api/display/draw clears canvas elements.
    ``application_name`` is an optional ownership check passed in the query.
    ``element_ids`` in the optional JSON body selects individual elements.
    """

    @allure.title("DELETE without application_name → 200")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_all(self, assets_api: AssetsAPI):
        resp = assets_api.clear_display()
        assert resp.result  # Pydantic validates non-empty "result"

    @allure.title("DELETE with matching application_name → 200")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_by_app_id(self, assets_api: AssetsAPI, busy_timer_stopped):
        draw_response = assets_api.draw_response(
            "some_app",
            [_text(id="owned_element")],
            priority=_PRI,
        )
        assets_api.assert_status(draw_response, 200)
        resp = assets_api.clear_display_by_app("some_app")
        assets_api.assert_status(resp, 200)

    @allure.title("DELETE after draw clears canvas, re-draw succeeds")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_then_redraw(self, assets_api: AssetsAPI, busy_timer_stopped):
        _draw(assets_api, [_text()])
        assets_api.clear_display()
        resp = _draw(assets_api, [_text(text="after clear")])
        assets_api.assert_status(resp, 200)

    @allure.title("DELETE for a specific application allows another app to draw")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_app_isolation(self, assets_api: AssetsAPI, busy_timer_stopped):
        """Draw as app A, clear app A, draw as app B — must succeed."""
        a_elem = [_text(id="a1", text="A draw")]
        b_elem = [_text(id="b1", text="B draw")]
        assets_api.draw_response("app_a", a_elem, priority=_PRI)
        assets_api.clear_display_by_app("app_a")
        resp = assets_api.draw_response("app_b", b_elem, priority=_PRI)
        assets_api.assert_status(resp, 200)

    @allure.title("DELETE is idempotent (double delete → 200)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_delete_idempotent(self, assets_api: AssetsAPI):
        assets_api.clear_display()
        resp = assets_api.clear_display_by_app("nonexistent")
        assets_api.assert_status(resp, 200)


# ───────────────────────────────────────────────────────────────────────────
# TestBrightnessGet — GET /api/display/brightness
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Brightness API – GET")
class TestBrightnessGet:
    """
    GET /api/display/brightness returns ``{"value": "<number>"|"auto"}``.
    """

    @allure.title("GET brightness returns 200 with value field")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_get_brightness_ok(self, api_session: requests.Session, web_base_url: str):
        resp = api_session.get(f"{web_base_url}/api/display/brightness", timeout=10)
        assert resp.status_code == 200, f"Expected 200, got {resp.status_code}"
        data = resp.json()
        assert "value" in data, f"Response missing 'value' field: {data}"

    @allure.title("GET brightness value is 'auto' or numeric string")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_get_brightness_value_format(
        self, api_session: requests.Session, web_base_url: str
    ):
        resp = api_session.get(f"{web_base_url}/api/display/brightness", timeout=10)
        resp.raise_for_status()
        value = resp.json()["value"]
        if value != "auto":
            assert value.isdigit(), f"Expected numeric string or 'auto', got '{value}'"
            num = int(value)
            assert 0 <= num <= 100, f"Brightness {num} out of [0,100]"


# ───────────────────────────────────────────────────────────────────────────
# TestBrightnessSet — POST /api/display/brightness
# ───────────────────────────────────────────────────────────────────────────


@allure.feature("5. Web Frontend")
@allure.story("Brightness API – POST")
class TestBrightnessSet:
    """
    POST /api/display/brightness?value=<n|auto>
    """

    @pytest.fixture(autouse=True)
    def _restore_brightness(self, api_session: requests.Session, web_base_url: str):
        """Capture current brightness and restore it after the test."""
        url = f"{web_base_url}/api/display/brightness"
        resp = api_session.get(url, timeout=10)
        resp.raise_for_status()
        original_value = resp.json().get("value", "auto")
        yield
        api_session.post(url, params={"value": original_value}, timeout=10)
        # Allow async brightness controller to process the restore message
        time.sleep(0.3)

    @allure.title("Set brightness to numeric value → 200")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_set_numeric(self, api_session: requests.Session, web_base_url: str):
        resp = api_session.post(
            f"{web_base_url}/api/display/brightness",
            params={"value": "50"},
            timeout=10,
        )
        assert resp.status_code == 200

    @allure.title("Set brightness to 'auto' → 200")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_set_auto(self, api_session: requests.Session, web_base_url: str):
        resp = api_session.post(
            f"{web_base_url}/api/display/brightness",
            params={"value": "auto"},
            timeout=10,
        )
        assert resp.status_code == 200

    @allure.title("Set brightness to 0 (min) → 200")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_set_min(self, api_session: requests.Session, web_base_url: str):
        resp = api_session.post(
            f"{web_base_url}/api/display/brightness",
            params={"value": "0"},
            timeout=10,
        )
        assert resp.status_code == 200

    @allure.title("Set brightness to 100 (max) → 200")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_set_max(self, api_session: requests.Session, web_base_url: str):
        resp = api_session.post(
            f"{web_base_url}/api/display/brightness",
            params={"value": "100"},
            timeout=10,
        )
        assert resp.status_code == 200

    @allure.title("Set brightness without value param → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_set_missing_value(self, api_session: requests.Session, web_base_url: str):
        resp = api_session.post(
            f"{web_base_url}/api/display/brightness",
            timeout=10,
        )
        assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"

    @allure.title("Set brightness to non-numeric, non-auto → 400")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_set_invalid_string(self, api_session: requests.Session, web_base_url: str):
        resp = api_session.post(
            f"{web_base_url}/api/display/brightness",
            params={"value": "xyz"},
            timeout=10,
        )
        assert resp.status_code == 400, f"Expected 400, got {resp.status_code}"

    @allure.title("Set brightness verifies round-trip (set → get shows manual mode)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_brightness_round_trip(
        self, api_session: requests.Session, web_base_url: str
    ):
        """Set a manual brightness and read it back.

        Only multiples of 10 survive the internal quantisation
        (user → internal = val*10/100, internal → user = val*100/10),
        so we pick 50 for an exact round-trip.

        A short sleep is needed because the brightness controller
        processes set requests asynchronously via a message queue.
        """
        url = f"{web_base_url}/api/display/brightness"
        set_resp = api_session.post(url, params={"value": "50"}, timeout=10)
        assert set_resp.status_code == 200

        time.sleep(0.3)  # wait for async state update

        get_resp = api_session.get(url, timeout=10)
        get_resp.raise_for_status()
        assert get_resp.json()["value"] == "50"
