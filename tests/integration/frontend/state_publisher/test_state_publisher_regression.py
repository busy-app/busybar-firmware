"""
Regression coverage for the firmware state publisher WebSocket.

Each test mutates state through the existing REST surface and asserts that
the matching ``StateUpdate`` arrives on ``/api/status/ws`` within a
bounded timeout. All tests carry the ``regression`` and ``api`` markers so
they only execute under the dedicated ``regression-tests`` CI job (fired
by ``-rc`` tags); PR / dev runs never collect them.
"""

from __future__ import annotations

import time

import allure
import pytest

from utils.busy_timer import (
    WORK_CARD_UUID,
    next_timestamp,
    set_snapshot,
)


pytestmark = [pytest.mark.regression, pytest.mark.api]


# --------------------------------------------------------------------- AC3
def test_device_name_change_publishes(state_publisher_ws, settings_api):
    """AC3: setting a new device name emits a ``device_name`` StateUpdate."""
    original = settings_api.get_name().name
    new_name = f"reg-{int(time.time()) % 100000}"
    if new_name == original:
        new_name = f"{new_name}x"

    try:
        state_publisher_ws.drain()
        with allure.step(f"POST /api/name -> {new_name!r}"):
            settings_api.set_name(new_name)

        update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "device_name"
            and u.device_name.name == new_name,
            timeout=5.0,
        )
        assert update.WhichOneof("state") == "device_name"
        assert update.device_name.name == new_name
    finally:
        try:
            settings_api.set_name(original)
        except Exception:
            pass


# --------------------------------------------------------------------- AC4
def test_audio_volume_change_publishes(state_publisher_ws, settings_api):
    """AC4: setting a new volume emits an ``audio_volume`` StateUpdate."""
    current = int(settings_api.get_volume().volume)
    candidates = [v for v in (10, 50, 90) if v != current]
    new_volume = candidates[0]

    try:
        state_publisher_ws.drain()
        with allure.step(f"POST /api/audio/volume?volume={new_volume}"):
            settings_api.set_volume(new_volume)

        update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "audio_volume"
            and u.audio_volume.volume == new_volume,
            timeout=5.0,
        )
        assert update.WhichOneof("state") == "audio_volume"
        assert update.audio_volume.volume == new_volume
    finally:
        try:
            settings_api.set_volume(current)
        except Exception:
            pass


# --------------------------------------------------------------------- AC5
def _brightness_mode_for(value: str) -> str:
    return "automatic" if value.lower() == "auto" else "manual"


def test_brightness_change_publishes(state_publisher_ws, settings_api):
    """AC5: changing brightness emits a ``brightness`` StateUpdate.

    We only assert on the oneof discriminator (``automatic`` vs ``manual``)
    because the numeric ``actual_brightness`` depends on hardware and the
    sensor's averaging window.
    """
    current = settings_api.get_brightness().value
    candidates = [v for v in ("auto", "20", "80") if v != current]
    new_value = candidates[0]
    expected_mode = _brightness_mode_for(new_value)

    try:
        state_publisher_ws.drain()
        with allure.step(f"POST /api/display/brightness?value={new_value}"):
            settings_api.set_brightness(new_value)

        update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "brightness"
            and u.brightness.WhichOneof("setting") == expected_mode,
            timeout=5.0,
        )
        assert update.WhichOneof("state") == "brightness"
        assert update.brightness.WhichOneof("setting") == expected_mode
    finally:
        try:
            settings_api.set_brightness(current)
        except Exception:
            pass


# --------------------------------------------------------------------- AC6
def test_timezone_change_publishes(state_publisher_ws, system_api):
    """AC6: setting a new timezone emits a ``timezone`` StateUpdate."""
    current = system_api.get_timezone().name
    tz_list = [item.name for item in system_api.get_timezone_list().list]
    candidates = [tz for tz in tz_list if tz != current]
    if not candidates:
        pytest.skip("Device reports no alternative timezone")
    new_tz = candidates[0]

    try:
        state_publisher_ws.drain()
        with allure.step(f"POST /api/time/timezone?timezone={new_tz}"):
            system_api.set_timezone(new_tz)

        update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "timezone"
            and u.timezone.name == new_tz,
            timeout=5.0,
        )
        assert update.WhichOneof("state") == "timezone"
        assert update.timezone.name == new_tz
    finally:
        try:
            system_api.set_timezone(current)
        except Exception:
            pass


# --------------------------------------------------------------------- AC7
def test_ble_enable_disable_publishes(state_publisher_ws, ble_api):
    """AC7: each BLE enable/disable transition emits a ``ble`` StateUpdate."""
    initial = ble_api.get_status().status
    started_enabled = initial not in ("disabled", "reset", "initialization")

    def _toggle_off():
        state_publisher_ws.drain()
        with allure.step("POST /api/ble/disable"):
            ble_api.disable()
        update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "ble", timeout=5.0
        )
        assert update.WhichOneof("state") == "ble"

    def _toggle_on():
        state_publisher_ws.drain()
        with allure.step("POST /api/ble/enable"):
            ble_api.enable()
        update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "ble", timeout=5.0
        )
        assert update.WhichOneof("state") == "ble"

    try:
        if started_enabled:
            _toggle_off()
            _toggle_on()
        else:
            _toggle_on()
            _toggle_off()
    finally:
        try:
            if started_enabled:
                ble_api.enable()
            else:
                ble_api.disable()
        except Exception:
            pass


# --------------------------------------------------------------------- AC8
def test_busy_timer_change_publishes(
    state_publisher_ws,
    api_session,
    web_base_url,
    busy_state_guard,
):
    """AC8: mutating the busy-timer snapshot emits a ``timer`` StateUpdate.

    The payload (``BSB_Util.Json``) is opaque to this test — we only assert
    that some ``timer`` update arrives. ``busy_state_guard`` autorestores
    the snapshot on teardown.
    """
    current = busy_state_guard
    settings = current.get("snapshot", {}).get("busy_bar_settings", {})
    body = {
        "snapshot": {
            "type": "INFINITE",
            "card_id": WORK_CARD_UUID,
            "is_paused": False,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }

    state_publisher_ws.drain()
    with allure.step("PUT /api/busy/snapshot (INFINITE, active)"):
        set_snapshot(api_session, web_base_url, body)

    update = state_publisher_ws.wait_for(
        lambda u: u.WhichOneof("state") == "timer", timeout=5.0
    )
    assert update.WhichOneof("state") == "timer"
