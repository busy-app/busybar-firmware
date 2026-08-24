"""Shared element builders, XPM2 source builders, and frame-wait helpers.

Framebuffer layout lives in clients.api.streaming (FrontFrame/BackFrame);
this module owns the draw-request vocabulary and render waits.
"""

from __future__ import annotations

import hashlib
import time
from collections.abc import Callable
from typing import TypeVar

from clients.api import StreamingAPI
from clients.api.streaming import (
    FRONT_DISPLAY_HEIGHT,
    FRONT_DISPLAY_WIDTH,
    BackFrame,
    FrontFrame,
)

_FRAME_TIMEOUT = 2.0
_FRAME_POLL_INTERVAL = 0.1

_FrameT = TypeVar("_FrameT", FrontFrame, BackFrame)

# Fill colors (#RRGGBBAA) and their readback forms: RGB_* as FrontFrame.pixel
# tuples, BGR_* as raw framebuffer byte triplets.
FILL_BLACK = "#000000FF"
FILL_RED = "#FF0000FF"
FILL_GREEN = "#00FF00FF"
FILL_BLUE = "#0000FFFF"
FILL_WHITE = "#FFFFFFFF"

RGB_BLACK = (0, 0, 0)
RGB_RED = (255, 0, 0)
RGB_GREEN = (0, 255, 0)
RGB_BLUE = (0, 0, 255)
RGB_WHITE = (255, 255, 255)

BGR_BLACK = b"\x00\x00\x00"
BGR_RED = b"\x00\x00\xff"
BGR_GREEN = b"\x00\xff\x00"
BGR_BLUE = b"\xff\x00\x00"

# XPM API limits mirrored from
# applications/services/web_server/http_api/api_display.c
XPM_API_MAX_COLORS = 32
XPM_API_MAX_CPP = 4

# Smallest real shared assets on the device. stock_path resolution: firmware
# takes the basename and looks it up in /ext/shared/images/ (image) or
# /ext/shared/animations/ (anim).
BUILTIN_IMAGE = "shared/checkmark_front_8x8.image"  # 28 bytes
BUILTIN_ANIM = "shared/spinner_front_8x8.anim"  # 2985 bytes


# ---------------------------------------------------------------------------
# Element builders
# ---------------------------------------------------------------------------


def text_element(
    text: str = "hello",
    *,
    element_id: str = "t1",
    font: str = "small",
    timeout: int = 5,
    **extra,
) -> dict:
    """Build a minimal valid text element."""
    base = {
        "id": element_id,
        "type": "text",
        "text": text,
        "font": font,
        "timeout": timeout,
    }
    base.update(extra)
    return base


def countdown_element(
    *,
    element_id: str = "cd1",
    timestamp: str = "1700000000",
    direction: str = "time_left",
    show_hours: str = "when_non_zero",
    timeout: int = 5,
    **extra,
) -> dict:
    """Build a minimal valid countdown element."""
    base = {
        "id": element_id,
        "type": "countdown",
        "timestamp": timestamp,
        "direction": direction,
        "show_hours": show_hours,
        "timeout": timeout,
    }
    base.update(extra)
    return base


def image_element(
    *,
    element_id: str = "img1",
    stock_path: str = BUILTIN_IMAGE,
    timeout: int = 5,
    **extra,
) -> dict:
    """Build a minimal valid image element using a real builtin image."""
    base = {
        "id": element_id,
        "type": "image",
        "stock_path": stock_path,
        "timeout": timeout,
    }
    base.update(extra)
    return base


def anim_element(
    *,
    element_id: str = "a1",
    stock_path: str = BUILTIN_ANIM,
    timeout: int = 5,
    **extra,
) -> dict:
    """Build a minimal valid anim element using a real builtin animation."""
    base = {
        "id": element_id,
        "type": "animation",
        "stock_path": stock_path,
        "timeout": timeout,
    }
    base.update(extra)
    return base


def solid_rectangle(
    element_id: str,
    color: str,
    *,
    x: int = 0,
    y: int = 0,
    width: int = FRONT_DISPLAY_WIDTH,
    height: int = FRONT_DISPLAY_HEIGHT,
    timeout: int = 0,
    align: str | None = None,
    z_index: int | str | float | None = None,
) -> dict:
    """Build a solid rectangle element with configurable geometry."""
    element = {
        "id": element_id,
        "type": "rectangle",
        "x": x,
        "y": y,
        "width": width,
        "height": height,
        "fill": "solid",
        "fill_colors": [color],
        "border_width": 0,
        "timeout": timeout,
    }
    if align is not None:
        element["align"] = align
    if z_index is not None:
        element["z_index"] = z_index
    return element


def xpm_element(
    data: str,
    *,
    element_id: str = "xpm1",
    align: str = "top_left",
    timeout: int = 5,
    **extra,
) -> dict:
    """Build a minimal XPM2 bitmap element."""
    base = {
        "id": element_id,
        "type": "xpmbitmap",
        "data": data,
        "align": align,
        "timeout": timeout,
    }
    base.update(extra)
    return base


# ---------------------------------------------------------------------------
# XPM2 source builders
# ---------------------------------------------------------------------------


def xpm_source(
    width: int,
    height: int,
    colors: dict[str, str],
    rows: list[str],
) -> str:
    """Compose an XPM2 source from a color table and pixel rows.

    ``colors`` maps each pixel key to its color spec including the visual
    type, e.g. ``{"R": "c #FF0000", "G": "g #808080"}``. All keys must share
    one length (chars per pixel).
    """
    cpp = len(next(iter(colors)))
    lines = ["! XPM2", f"{width} {height} {len(colors)} {cpp}"]
    lines += [f"{key} {spec}" for key, spec in colors.items()]
    lines += rows
    return "\n".join(lines)


def solid_xpm(
    width: int, height: int, key: str = "X", color: str = "#FFFFFF"
) -> str:
    """Build a valid single-color XPM2 image of the requested dimensions."""
    return xpm_source(width, height, {key: f"c {color}"}, [key * width] * height)


def pixel_xpm(
    color: str = "#FFFFFF", *, visual: str = "c", key: str = "X"
) -> str:
    """Build a one-pixel XPM2 image from a single color definition.

    ``visual`` is the XPM color-line visual type: c (color), g (grayscale),
    m (monochrome), or s (symbolic).
    """
    return xpm_source(1, 1, {key: f"{visual} {color}"}, [key])


def colors_xpm(count: int) -> str:
    """Build a valid one-pixel XPM2 image with the requested color count."""
    keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    assert count <= len(keys)
    keys = keys[:count]
    return xpm_source(1, 1, {key: "c #FFFFFF" for key in keys}, [keys[0]])


# ---------------------------------------------------------------------------
# Frame capture and waiting
# ---------------------------------------------------------------------------


def frame_digest(frame: bytes) -> str:
    """Return a compact identity for raw-frame assertion messages."""
    return hashlib.sha256(frame).hexdigest()


def _poll_for_frame(
    capture: Callable[[], _FrameT],
    predicate: Callable[[_FrameT], bool],
) -> tuple[bool, _FrameT]:
    deadline = time.monotonic() + _FRAME_TIMEOUT
    last_frame = capture()

    while time.monotonic() < deadline:
        if predicate(last_frame):
            return True, last_frame
        time.sleep(_FRAME_POLL_INTERVAL)
        last_frame = capture()

    return predicate(last_frame), last_frame


def _wait_for_frame(
    capture: Callable[[], _FrameT],
    predicate: Callable[[_FrameT], bool],
    description: str,
) -> _FrameT:
    matched, last_frame = _poll_for_frame(capture, predicate)
    if matched:
        return last_frame

    last_frame.attach(f"Timeout: {description}")
    raise AssertionError(
        f"Timed out waiting for {description}; "
        f"last_frame_sha256={last_frame.digest()}"
    )


def wait_for_front_frame(
    streaming_api: StreamingAPI,
    predicate: Callable[[FrontFrame], bool],
    description: str,
) -> FrontFrame:
    """Poll until a front frame matches; attach the last frame on timeout."""
    return _wait_for_frame(streaming_api.front_frame, predicate, description)


def wait_for_back_frame(
    streaming_api: StreamingAPI,
    predicate: Callable[[BackFrame], bool],
    description: str,
) -> BackFrame:
    """Poll until a back frame matches; attach the last frame on timeout."""
    return _wait_for_frame(streaming_api.back_frame, predicate, description)


def capture_stable_front_frame(streaming_api: StreamingAPI) -> FrontFrame:
    """Wait for two consecutive identical front-display frames."""
    previous = streaming_api.front_frame()

    def frame_is_stable(current: FrontFrame) -> bool:
        nonlocal previous
        stable = current.raw == previous.raw
        previous = current
        return stable

    return wait_for_front_frame(
        streaming_api,
        frame_is_stable,
        "two consecutive identical front-display frames",
    )


def assert_front_pixels(
    streaming_api: StreamingAPI,
    expected: dict[tuple[int, int], tuple[int, int, int]],
    attach_as: str,
) -> FrontFrame:
    """Wait until every (x, y) front pixel matches its RGB value.

    Attaches the last captured frame to Allure under ``attach_as`` and
    returns it; on timeout raises with the expected-vs-actual pixel diff.
    """
    def matches(frame: FrontFrame) -> bool:
        return all(frame.pixel(x, y) == rgb for (x, y), rgb in expected.items())

    matched, frame = _poll_for_frame(streaming_api.front_frame, matches)
    frame.attach(attach_as if matched else f"Timeout: {attach_as}")
    actual = {(x, y): frame.pixel(x, y) for (x, y) in expected}
    assert actual == expected, (
        f"Timed out waiting for front pixels: expected {expected!r}, "
        f"got {actual!r}; last_frame_sha256={frame.digest()}"
    )
    return frame


def assert_back_pixels(
    streaming_api: StreamingAPI,
    expected: dict[tuple[int, int], int],
    attach_as: str,
) -> BackFrame:
    """Wait until every (x, y) back pixel matches its 4-bit luma value.

    Attaches the last captured frame to Allure under ``attach_as`` and
    returns it; on timeout raises with the expected-vs-actual pixel diff.
    """
    def matches(frame: BackFrame) -> bool:
        return all(
            frame.pixel(x, y) == luma for (x, y), luma in expected.items()
        )

    matched, frame = _poll_for_frame(streaming_api.back_frame, matches)
    frame.attach(attach_as if matched else f"Timeout: {attach_as}")
    actual = {(x, y): frame.pixel(x, y) for (x, y) in expected}
    assert actual == expected, (
        f"Timed out waiting for back pixels: expected {expected!r}, "
        f"got {actual!r}; last_frame_sha256={frame.digest()}"
    )
    return frame

