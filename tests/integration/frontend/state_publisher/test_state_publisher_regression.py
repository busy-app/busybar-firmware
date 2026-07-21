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


@allure.feature("State Publisher")
@allure.story("Published events")
@allure.title("Device name change publishes a StateUpdate")
def test_device_name_change_publishes(state_publisher_ws, settings_api):
    """Setting a new device name emits a ``device_name`` StateUpdate."""
    original = settings_api.get_name().name
    new_name = f"reg-{int(time.time()) % 100000}"
    if new_name == original:
        new_name = f"{new_name}x"

    try:
        state_publisher_ws.drain()
        with allure.step(f"Change the device name to {new_name!r}"):
            settings_api.set_name(new_name)

        with allure.step("Wait for the device-name StateUpdate"):
            update = state_publisher_ws.wait_for(
                lambda u: u.WhichOneof("state") == "device_name"
                and u.device_name.name == new_name,
                timeout=5.0,
            )

        with allure.step("Verify the published device name"):
            assert update.WhichOneof("state") == "device_name"
            assert update.device_name.name == new_name
    finally:
        try:
            settings_api.set_name(original)
        except Exception:
            pass


@allure.feature("State Publisher")
@allure.story("Published events")
@allure.title("Audio volume change publishes a StateUpdate")
def test_audio_volume_change_publishes(state_publisher_ws, settings_api):
    """Setting a new volume emits an ``audio_volume`` StateUpdate."""
    current = int(settings_api.get_volume().volume)
    candidates = [v for v in (10, 50, 90) if v != current]
    new_volume = candidates[0]

    try:
        state_publisher_ws.drain()
        with allure.step(f"Change the audio volume to {new_volume}"):
            settings_api.set_volume(new_volume)

        with allure.step("Wait for the audio-volume StateUpdate"):
            update = state_publisher_ws.wait_for(
                lambda u: u.WhichOneof("state") == "audio_volume"
                and u.audio_volume.volume == new_volume,
                timeout=5.0,
            )

        with allure.step("Verify the published audio volume"):
            assert update.WhichOneof("state") == "audio_volume"
            assert update.audio_volume.volume == new_volume
    finally:
        try:
            settings_api.set_volume(current)
        except Exception:
            pass


def _brightness_mode_for(value: str) -> str:
    return "automatic" if value.lower() == "auto" else "manual"


@allure.feature("State Publisher")
@allure.story("Published events")
@allure.title("Brightness change publishes a StateUpdate")
def test_brightness_change_publishes(state_publisher_ws, settings_api):
    """Changing brightness emits a ``brightness`` StateUpdate.

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
        with allure.step(f"Change the display brightness to {new_value}"):
            settings_api.set_brightness(new_value)

        with allure.step("Wait for the brightness StateUpdate"):
            update = state_publisher_ws.wait_for(
                lambda u: u.WhichOneof("state") == "brightness"
                and u.brightness.WhichOneof("setting") == expected_mode,
                timeout=5.0,
            )

        with allure.step("Verify the published brightness mode"):
            assert update.WhichOneof("state") == "brightness"
            assert update.brightness.WhichOneof("setting") == expected_mode
    finally:
        try:
            settings_api.set_brightness(current)
        except Exception:
            pass


@allure.feature("State Publisher")
@allure.story("Published events")
@allure.title("Timezone change publishes a StateUpdate")
def test_timezone_change_publishes(state_publisher_ws, system_api):
    """Setting a new timezone emits a ``timezone`` StateUpdate."""
    current = system_api.get_timezone().name
    tz_list = [item.name for item in system_api.get_timezone_list().list]
    candidates = [tz for tz in tz_list if tz != current]
    if not candidates:
        pytest.skip("Device reports no alternative timezone")
    new_tz = candidates[0]

    try:
        state_publisher_ws.drain()
        with allure.step(f"Change the device timezone to {new_tz}"):
            system_api.set_timezone(new_tz)

        with allure.step("Wait for the timezone StateUpdate"):
            update = state_publisher_ws.wait_for(
                lambda u: u.WhichOneof("state") == "timezone"
                and u.timezone.name == new_tz,
                timeout=5.0,
            )

        with allure.step("Verify the published timezone"):
            assert update.WhichOneof("state") == "timezone"
            assert update.timezone.name == new_tz
    finally:
        try:
            system_api.set_timezone(current)
        except Exception:
            pass


@allure.feature("State Publisher")
@allure.story("Published events")
@allure.title("BLE enable and disable transitions publish StateUpdates")
def test_ble_enable_disable_publishes(state_publisher_ws, ble_api):
    """Each BLE enable/disable transition emits a ``ble`` StateUpdate."""
    initial = ble_api.get_status().status
    started_enabled = initial not in ("disabled", "reset", "initialization")

    def _toggle_off():
        state_publisher_ws.drain()
        with allure.step("Disable BLE"):
            ble_api.disable()
        with allure.step("Wait for the BLE-disabled StateUpdate"):
            update = state_publisher_ws.wait_for(
                lambda u: u.WhichOneof("state") == "ble", timeout=5.0
            )
        with allure.step("Verify the BLE-disabled update discriminator"):
            assert update.WhichOneof("state") == "ble"

    def _toggle_on():
        state_publisher_ws.drain()
        with allure.step("Enable BLE"):
            ble_api.enable()
        with allure.step("Wait for the BLE-enabled StateUpdate"):
            update = state_publisher_ws.wait_for(
                lambda u: u.WhichOneof("state") == "ble", timeout=5.0
            )
        with allure.step("Verify the BLE-enabled update discriminator"):
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


@allure.feature("State Publisher")
@allure.story("Published events")
@allure.title("Busy timer change publishes a StateUpdate")
def test_busy_timer_change_publishes(
    state_publisher_ws,
    api_session,
    web_base_url,
    busy_state_guard,
):
    """Mutating the busy-timer snapshot emits a ``timer`` StateUpdate.

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
    with allure.step("Start an active INFINITE busy timer"):
        set_snapshot(api_session, web_base_url, body)

    with allure.step("Wait for the timer StateUpdate"):
        update = state_publisher_ws.wait_for(
            lambda u: u.WhichOneof("state") == "timer", timeout=5.0
        )

    with allure.step("Verify the published timer update discriminator"):
        assert update.WhichOneof("state") == "timer"
