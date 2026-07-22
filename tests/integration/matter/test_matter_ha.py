"""
Matter integration tests against an existing Home Assistant instance.

The device is commissioned into HA over Matter; the "lamp" is the BUSY Bar
OnOff switch, asserted from both sides:
- HA switch service → device /api/smart_home/switch state,
- device /api/smart_home/switch POST → HA entity state.

See conftest.py for required env and README.md for topology.
"""

from __future__ import annotations

import time

import allure
import pytest

from utils.busy_timer import WORK_CARD_UUID, next_timestamp

pytestmark = [
    pytest.mark.matter,
    pytest.mark.regression,
    pytest.mark.uses_si917,
    pytest.mark.external_service,
]


def _wait_for_switch_state(
    ha,
    matter_smart_home_api,
    entity: str,
    expected: bool,
    timeout: float = 30.0,
    interval: float = 0.3,
):
    """Wait until HA and the device report the same expected switch state."""
    deadline = time.monotonic() + timeout
    t0 = time.monotonic()
    samples = []
    expected_ha = "on" if expected else "off"

    while time.monotonic() < deadline:
        observed = (
            ha.get_state(entity),
            matter_smart_home_api.get_switch_state().state,
        )
        if not samples or samples[-1][1] != observed:
            samples.append((round(time.monotonic() - t0, 1), observed))
        if observed == (expected_ha, expected):
            return True, samples
        time.sleep(interval)
    return False, samples


def _assert_switch_state(
    ha, matter_smart_home_api, entity: str, expected: bool
) -> None:
    ok, samples = _wait_for_switch_state(ha, matter_smart_home_api, entity, expected)
    expected_name = "on" if expected else "off"
    assert ok, (
        f"switch did not become {expected_name} in HA and on the device; "
        f"observed (t, (ha, device)): {samples}"
    )


def _set_ha_switch(ha, matter_smart_home_api, entity: str, state: bool) -> None:
    ha.set_switch(entity, state)
    _assert_switch_state(ha, matter_smart_home_api, entity, state)


def _set_device_switch(ha, matter_smart_home_api, entity: str, state: bool) -> None:
    resp = matter_smart_home_api.set_switch_state(state)
    assert resp.status_code == 200, resp.text
    _assert_switch_state(ha, matter_smart_home_api, entity, state)


def _wait_for_timer_type(
    busy_api,
    expected_type: str,
    *,
    equal: bool = True,
    timeout: float = 20.0,
    interval: float = 0.3,
):
    deadline = time.monotonic() + timeout
    t0 = time.monotonic()
    samples = []

    while time.monotonic() < deadline:
        snapshot_type = busy_api.get_snapshot().snapshot.get("type")
        if not samples or samples[-1][1] != snapshot_type:
            samples.append((round(time.monotonic() - t0, 1), snapshot_type))
        if (snapshot_type == expected_type) is equal:
            return True, samples
        time.sleep(interval)
    return False, samples


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Device is commissioned into Home Assistant")
@pytest.mark.timeout(240)
def test_commissioning(ha, matter_smart_home_api, commissioned_device):
    """Device commissions into the Home Assistant Matter fabric."""
    with allure.step("Read Matter commissioning status"):
        pairing = matter_smart_home_api.get_pairing()

    with allure.step("Verify the device is commissioned and exposed in Home Assistant"):
        assert pairing.fabric_count >= 1
        assert pairing.latest_pairing_status.value == "completed_successfully"
        assert commissioned_device["switch_entity"].startswith(("light.", "switch."))


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Home Assistant turns the device switch {param_id}")
@pytest.mark.timeout(120)
@pytest.mark.parametrize("state", [True, False], ids=["on", "off"])
def test_ha_sets_device_switch(ha, matter_smart_home_api, idle_lamp_off, state):
    """HA switch service call is reflected in the device switch state."""
    entity = idle_lamp_off

    with allure.step("Establish the opposite device-side switch state"):
        _set_device_switch(ha, matter_smart_home_api, entity, not state)

    with allure.step(f"Turn the Home Assistant switch {'on' if state else 'off'}"):
        ha.set_switch(entity, state)

    with allure.step("Wait for and verify the device switch state"):
        _assert_switch_state(ha, matter_smart_home_api, entity, state)


def _busy_settings(trigger_smart_home: bool) -> dict:
    return {
        "theme": "busy",
        "show_work_phase_only": False,
        "trigger_smart_home": trigger_smart_home,
    }


def _set_timer(busy_api, snapshot: dict) -> None:
    timestamp = next_timestamp(busy_api.session, busy_api.base_url)
    body = {
        "snapshot": snapshot,
        "snapshot_timestamp_ms": timestamp,
    }
    resp = busy_api.set_snapshot_raw(body)
    assert resp.status_code == 200, resp.text

    deadline = time.monotonic() + 5.0
    samples = []
    while time.monotonic() < deadline:
        current = busy_api.get_snapshot()
        observed = (current.snapshot_timestamp_ms, current.snapshot.get("type"))
        if not samples or samples[-1] != observed:
            samples.append(observed)
        if observed == (timestamp, snapshot["type"]):
            return
        time.sleep(0.1)

    raise AssertionError(
        f"timer snapshot was not applied: expected timestamp/type "
        f"{timestamp}/{snapshot['type']}, observed: {samples}"
    )


def _running_timer_snapshot(trigger_smart_home: bool) -> dict:
    return {
        "type": "SIMPLE",
        "card_id": WORK_CARD_UUID,
        "is_paused": False,
        "time_left_ms": 180_000,
        "busy_bar_settings": _busy_settings(trigger_smart_home),
    }


def _stopped_timer_snapshot(trigger_smart_home: bool) -> dict:
    return {
        "type": "NOT_STARTED",
        "busy_bar_settings": _busy_settings(trigger_smart_home),
    }


def _interval_timer_snapshot(interval_index: int, trigger_smart_home: bool) -> dict:
    """INTERVAL-mode snapshot; even interval_index = WORK, odd = REST."""
    # firmware validation: work/rest intervals must be >= 5 minutes
    interval_ms = 5 * 60 * 1000
    return {
        "type": "INTERVAL",
        "card_id": WORK_CARD_UUID,
        "is_paused": False,
        "current_interval": interval_index,
        "current_interval_time_total_ms": interval_ms,
        "current_interval_time_left_ms": interval_ms - 20_000,
        "interval_settings": {
            "type": "INTERVAL",
            "interval_work_ms": interval_ms,
            "interval_rest_ms": interval_ms,
            "interval_work_cycles_count": 4,
            "is_autostart_enabled": False,
        },
        "busy_bar_settings": _busy_settings(trigger_smart_home),
    }


@pytest.fixture()
def busy_api(matter_module_api_factory):
    from clients.api import BusyAPI

    return matter_module_api_factory(BusyAPI)


def _establish_idle_lamp_off(ha, matter_smart_home_api, busy_api, entity: str) -> None:
    _set_timer(busy_api, _stopped_timer_snapshot(True))
    _set_ha_switch(ha, matter_smart_home_api, entity, False)


@pytest.fixture()
def idle_lamp_off(ha, matter_smart_home_api, commissioned_device, busy_api):
    """Give each stateful test an idle timer and synchronized OFF switch."""
    entity = commissioned_device["switch_entity"]

    with allure.step("Establish an idle timer and lamp-off precondition"):
        _establish_idle_lamp_off(ha, matter_smart_home_api, busy_api, entity)

    yield entity

    with allure.step("Restore the idle timer and lamp-off state"):
        _establish_idle_lamp_off(ha, matter_smart_home_api, busy_api, entity)


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Smart-home timer controls the Home Assistant lamp")
@pytest.mark.timeout(180)
def test_timer_with_smart_home_lights_lamp(
    ha, matter_smart_home_api, busy_api, idle_lamp_off
):
    """Running work timer with 'trigger smart home' ON turns the lamp on;
    stopping the timer turns it off."""
    entity = idle_lamp_off

    with allure.step("Start the timer and verify the lamp turns on"):
        _set_timer(busy_api, _running_timer_snapshot(True))
        _assert_switch_state(ha, matter_smart_home_api, entity, True)

    with allure.step("Stop the timer and verify the lamp turns off"):
        _set_timer(busy_api, _stopped_timer_snapshot(True))
        _assert_switch_state(ha, matter_smart_home_api, entity, False)


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title(
    "Timer leaves the Home Assistant lamp off when smart-home control is disabled"
)
@pytest.mark.timeout(180)
def test_timer_without_smart_home_keeps_lamp_off(
    ha, matter_smart_home_api, busy_api, idle_lamp_off
):
    """Running timer with 'trigger smart home' OFF must not touch the lamp."""
    entity = idle_lamp_off

    with allure.step("Disable smart-home control while keeping the lamp off"):
        _set_timer(busy_api, _stopped_timer_snapshot(False))
        _assert_switch_state(ha, matter_smart_home_api, entity, False)

    with allure.step("Run the timer with smart-home control disabled"):
        _set_timer(busy_api, _running_timer_snapshot(False))

    with allure.step("Verify the device switch and Home Assistant lamp remain off"):
        # No positive signal to wait for — hold and verify nothing flips
        # (propagation is ~1s, so a 5s watch leaves ample margin).
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            assert (
                matter_smart_home_api.get_switch_state().state is False
            ), "device switch turned on despite trigger_smart_home=False"
            assert (
                ha.get_state(entity) == "off"
            ), "HA lamp turned on despite trigger_smart_home=False"
            time.sleep(0.5)


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Pausing and resuming the timer toggles the Home Assistant lamp")
@pytest.mark.timeout(180)
def test_timer_pause_toggles_lamp(ha, matter_smart_home_api, busy_api, idle_lamp_off):
    """Pausing a running work timer turns the lamp off; resuming turns it on."""
    entity = idle_lamp_off

    with allure.step("Establish a running timer and lamp-on baseline"):
        _set_timer(busy_api, _running_timer_snapshot(True))
        _assert_switch_state(ha, matter_smart_home_api, entity, True)

    with allure.step("Pause the timer and verify the lamp turns off"):
        paused = dict(_running_timer_snapshot(True), is_paused=True)
        _set_timer(busy_api, paused)
        _assert_switch_state(ha, matter_smart_home_api, entity, False)

    with allure.step("Resume the timer and verify the lamp turns on"):
        _set_timer(busy_api, _running_timer_snapshot(True))
        _assert_switch_state(ha, matter_smart_home_api, entity, True)


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Work and rest intervals toggle the Home Assistant lamp")
@pytest.mark.timeout(180)
def test_timer_rest_interval_turns_lamp_off(
    ha, matter_smart_home_api, busy_api, idle_lamp_off
):
    """In INTERVAL mode the lamp is on during WORK and off during REST."""
    entity = idle_lamp_off

    with allure.step("Start a work interval and verify the lamp turns on"):
        _set_timer(busy_api, _interval_timer_snapshot(0, True))  # WORK
        _assert_switch_state(ha, matter_smart_home_api, entity, True)

    with allure.step("Start a rest interval and verify the lamp turns off"):
        _set_timer(busy_api, _interval_timer_snapshot(1, True))  # REST
        _assert_switch_state(ha, matter_smart_home_api, entity, False)


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Home Assistant lamp starts and stops the device timer")
@pytest.mark.timeout(180)
def test_ha_lamp_starts_and_stops_timer(
    ha, matter_smart_home_api, busy_api, idle_lamp_off
):
    """Reverse direction: HA lamp ON starts the busy timer on the device,
    lamp OFF stops it."""
    entity = idle_lamp_off

    with allure.step("Turn on the Home Assistant lamp and verify the timer starts"):
        _set_ha_switch(ha, matter_smart_home_api, entity, True)
        ok, samples = _wait_for_timer_type(busy_api, "NOT_STARTED", equal=False)
        assert ok, (
            "HA lamp ON did not start the busy timer; "
            f"observed snapshot types: {samples}"
        )

    with allure.step("Turn off the Home Assistant lamp and verify the timer stops"):
        _set_ha_switch(ha, matter_smart_home_api, entity, False)
        ok, samples = _wait_for_timer_type(busy_api, "NOT_STARTED")
        assert ok, (
            "HA lamp OFF did not stop the busy timer; "
            f"observed snapshot types: {samples}"
        )


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Device switch state is visible in Home Assistant as {param_id}")
@pytest.mark.timeout(120)
@pytest.mark.parametrize("state", [True, False], ids=["on", "off"])
def test_device_switch_visible_in_ha(ha, matter_smart_home_api, idle_lamp_off, state):
    """Device-side switch change is visible as the HA entity state."""
    entity = idle_lamp_off

    with allure.step("Establish the opposite Home Assistant switch state"):
        _set_ha_switch(ha, matter_smart_home_api, entity, not state)

    with allure.step(f"Turn the device switch {'on' if state else 'off'}"):
        resp = matter_smart_home_api.set_switch_state(state)
        assert resp.status_code == 200, resp.text

    with allure.step("Wait for and verify the Home Assistant entity state"):
        _assert_switch_state(ha, matter_smart_home_api, entity, state)
