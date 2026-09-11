"""Polling helpers for the input API."""

from __future__ import annotations

from clients.api import InputAPI, InputSwitchResponse
from utils.wait import wait_for


def wait_for_switch_position(
    input_api: InputAPI,
    position: str,
    timeout: float = 2.0,
) -> InputSwitchResponse:
    """Poll GET /api/input/switch until it reports the expected position."""
    return wait_for(
        f"switch position {position!r}",
        input_api.get_switch,
        lambda reply: reply.position == position,
        timeout=timeout,
        interval=0.1,
    )
