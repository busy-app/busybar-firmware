"""Shared element builders and framebuffer helpers for display tests."""

from __future__ import annotations

import hashlib
import time
from collections.abc import Callable

import allure

from clients.api import StreamingAPI
from clients.api.streaming import (
    BACK_DISPLAY_WIDTH,
    FRONT_DISPLAY_HEIGHT,
    FRONT_DISPLAY_WIDTH,
    raw_to_png,
)

_FRAME_TIMEOUT = 2.0
_FRAME_POLL_INTERVAL = 0.1


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


def frame_digest(frame: bytes) -> str:
    """Return a compact identity for framebuffer assertion messages."""
    return hashlib.sha256(frame).hexdigest()


def front_pixel(frame: bytes, x: int, y: int) -> tuple[int, int, int]:
    """Return one RGB pixel from the front display's raw BGR byte layout."""
    offset = (y * FRONT_DISPLAY_WIDTH + x) * 3
    return frame[offset + 2], frame[offset + 1], frame[offset]


def back_pixel(frame: bytes, x: int, y: int) -> int:
    """Return one 4-bit luma pixel from a nibble-packed back-display frame."""
    pixel_index = y * BACK_DISPLAY_WIDTH + x
    packed = frame[pixel_index // 2]
    return (packed & 0x0F) if pixel_index % 2 == 0 else (packed >> 4)


def _wait_for_frame(
    streaming_api: StreamingAPI,
    display: int,
    predicate: Callable[[bytes], bool],
    description: str,
) -> bytes:
    deadline = time.monotonic() + _FRAME_TIMEOUT
    last_frame = streaming_api.get_screen_bytes(display=display)

    while time.monotonic() < deadline:
        if predicate(last_frame):
            return last_frame
        time.sleep(_FRAME_POLL_INTERVAL)
        last_frame = streaming_api.get_screen_bytes(display=display)

    assert predicate(last_frame), (
        f"Timed out waiting for {description}; "
        f"last_frame_sha256={frame_digest(last_frame)}"
    )
    return last_frame


def wait_for_front_frame(
    streaming_api: StreamingAPI,
    predicate: Callable[[bytes], bool],
    description: str,
) -> bytes:
    """Poll until a front framebuffer matches the requested condition."""
    return _wait_for_frame(streaming_api, 0, predicate, description)


def wait_for_back_frame(
    streaming_api: StreamingAPI,
    predicate: Callable[[bytes], bool],
    description: str,
) -> bytes:
    """Poll until a back framebuffer matches the requested condition."""
    return _wait_for_frame(streaming_api, 1, predicate, description)


def capture_stable_front_frame(streaming_api: StreamingAPI) -> bytes:
    """Wait for two consecutive identical front-display frames."""
    previous = streaming_api.get_screen_bytes(display=0)

    def frame_is_stable(current: bytes) -> bool:
        nonlocal previous
        stable = current == previous
        previous = current
        return stable

    return wait_for_front_frame(
        streaming_api,
        frame_is_stable,
        "two consecutive identical front-display frames",
    )


def attach_frame(frame: bytes, display: int, name: str) -> None:
    """Attach a captured framebuffer to the Allure report as a PNG."""
    png_frame = frame
    if display == 0:
        rgb_frame = bytearray(frame)
        rgb_frame[0::3] = frame[2::3]
        rgb_frame[2::3] = frame[0::3]
        png_frame = bytes(rgb_frame)

    allure.attach(
        raw_to_png(png_frame, display),
        name=name,
        attachment_type=allure.attachment_type.PNG,
    )
