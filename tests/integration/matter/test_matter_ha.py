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

from utils.busy_timer import WORK_CARD_UUID, next_timestamp, wait_for_snapshot_type

pytestmark = [
    pytest.mark.matter,
    pytest.mark.regression,
    pytest.mark.uses_si917,
    pytest.mark.external_service,
]


def _wait_for(predicate, timeout: float = 20.0, interval: float = 0.3):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(interval)
    return predicate()


def _wait_for_state(getter, expected: str, timeout: float = 30.0):
    """Poll getter() until it returns expected; return (ok, samples)."""
    samples = []
    deadline = time.monotonic() + timeout
    t0 = time.monotonic()
    while time.monotonic() < deadline:
        value = getter()
        if not samples or samples[-1][1] != value:
            samples.append((round(time.monotonic() - t0, 1), value))
        if value == expected:
            return True, samples
        time.sleep(0.3)
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
def test_ha_sets_device_switch(ha, matter_smart_home_api, commissioned_device, state):
    """HA switch service call is reflected in the device switch state."""
    with allure.step(f"Turn the Home Assistant switch {'on' if state else 'off'}"):
        ha.set_switch(commissioned_device["switch_entity"], state)

    with allure.step("Wait for and verify the device switch state"):
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is state
        ), f"device switch did not become {state}"


def _busy_settings(trigger_smart_home: bool) -> dict:
    return {
        "theme": "busy",
        "show_work_phase_only": False,
        "trigger_smart_home": trigger_smart_home,
    }


def _set_timer(busy_api, snapshot: dict) -> None:
    body = {
        "snapshot": snapshot,
        "snapshot_timestamp_ms": next_timestamp(busy_api.session, busy_api.base_url),
    }
    resp = busy_api.set_snapshot_raw(body)
    assert resp.status_code == 200, resp.text
    wait_for_snapshot_type(busy_api.session, busy_api.base_url, snapshot["type"])


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


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Smart-home timer controls the Home Assistant lamp")
@pytest.mark.timeout(180)
def test_timer_with_smart_home_lights_lamp(
    ha, matter_smart_home_api, commissioned_device, busy_api, request
):
    """Running work timer with 'trigger smart home' ON turns the lamp on;
    stopping the timer turns it off."""
    request.addfinalizer(lambda: _set_timer(busy_api, _stopped_timer_snapshot(True)))
    entity = commissioned_device["switch_entity"]

    with allure.step("Establish a stopped timer and lamp-off baseline"):
        _set_timer(busy_api, _stopped_timer_snapshot(True))
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is False
        )

    with allure.step("Start the timer and verify the lamp turns on"):
        _set_timer(busy_api, _running_timer_snapshot(True))
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is True
        ), "running work timer did not turn the device switch on"
        ok, samples = _wait_for_state(lambda: ha.get_state(entity), "on")
        assert ok, f"HA lamp did not turn on with the timer; observed: {samples}"

    with allure.step("Stop the timer and verify the lamp turns off"):
        _set_timer(busy_api, _stopped_timer_snapshot(True))
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is False
        ), "stopping the timer did not turn the device switch off"
        ok, samples = _wait_for_state(lambda: ha.get_state(entity), "off")
        assert ok, f"HA lamp did not turn off after timer stop; observed: {samples}"


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title(
    "Timer leaves the Home Assistant lamp off when smart-home control is disabled"
)
@pytest.mark.timeout(180)
def test_timer_without_smart_home_keeps_lamp_off(
    ha, matter_smart_home_api, commissioned_device, busy_api, request
):
    """Running timer with 'trigger smart home' OFF must not touch the lamp."""
    request.addfinalizer(lambda: _set_timer(busy_api, _stopped_timer_snapshot(False)))
    entity = commissioned_device["switch_entity"]

    with allure.step("Establish a stopped timer and lamp-off baseline"):
        _set_timer(busy_api, _stopped_timer_snapshot(False))
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is False
        )
        ok, _ = _wait_for_state(lambda: ha.get_state(entity), "off")
        assert ok, "baseline: HA lamp is not off"

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
def test_timer_pause_toggles_lamp(
    ha, matter_smart_home_api, commissioned_device, busy_api, request
):
    """Pausing a running work timer turns the lamp off; resuming turns it on."""
    request.addfinalizer(lambda: _set_timer(busy_api, _stopped_timer_snapshot(True)))
    entity = commissioned_device["switch_entity"]

    with allure.step("Establish a running timer and lamp-on baseline"):
        _set_timer(busy_api, _running_timer_snapshot(True))
        assert _wait_for(lambda: matter_smart_home_api.get_switch_state().state is True)

    with allure.step("Pause the timer and verify the lamp turns off"):
        paused = dict(_running_timer_snapshot(True), is_paused=True)
        _set_timer(busy_api, paused)
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is False
        ), "pausing the timer did not turn the device switch off"
        ok, samples = _wait_for_state(lambda: ha.get_state(entity), "off")
        assert ok, f"HA lamp did not turn off on pause; observed: {samples}"

    with allure.step("Resume the timer and verify the lamp turns on"):
        _set_timer(busy_api, _running_timer_snapshot(True))
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is True
        ), "resuming the timer did not turn the device switch back on"
        ok, samples = _wait_for_state(lambda: ha.get_state(entity), "on")
        assert ok, f"HA lamp did not turn back on after resume; observed: {samples}"


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Work and rest intervals toggle the Home Assistant lamp")
@pytest.mark.timeout(180)
def test_timer_rest_interval_turns_lamp_off(
    ha, matter_smart_home_api, commissioned_device, busy_api, request
):
    """In INTERVAL mode the lamp is on during WORK and off during REST."""
    request.addfinalizer(lambda: _set_timer(busy_api, _stopped_timer_snapshot(True)))
    entity = commissioned_device["switch_entity"]

    with allure.step("Start a work interval and verify the lamp turns on"):
        _set_timer(busy_api, _interval_timer_snapshot(0, True))  # WORK
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is True
        ), "WORK interval did not turn the device switch on"
        ok, samples = _wait_for_state(lambda: ha.get_state(entity), "on")
        assert ok, f"HA lamp is not on during WORK interval; observed: {samples}"

    with allure.step("Start a rest interval and verify the lamp turns off"):
        _set_timer(busy_api, _interval_timer_snapshot(1, True))  # REST
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is False
        ), "REST interval did not turn the device switch off"
        ok, samples = _wait_for_state(lambda: ha.get_state(entity), "off")
        assert ok, f"HA lamp is not off during REST interval; observed: {samples}"


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Home Assistant lamp starts and stops the device timer")
@pytest.mark.timeout(180)
def test_ha_lamp_starts_and_stops_timer(
    ha, matter_smart_home_api, commissioned_device, busy_api, request
):
    """Reverse direction: HA lamp ON starts the busy timer on the device,
    lamp OFF stops it."""
    request.addfinalizer(lambda: _set_timer(busy_api, _stopped_timer_snapshot(True)))
    entity = commissioned_device["switch_entity"]

    with allure.step("Establish a stopped timer and lamp-off baseline"):
        _set_timer(busy_api, _stopped_timer_snapshot(True))
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is False
        )

    with allure.step("Turn on the Home Assistant lamp and verify the timer starts"):
        ha.set_switch(entity, True)
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is True
        ), "HA lamp ON did not turn the device switch on"
        assert _wait_for(
            lambda: busy_api.get_snapshot().snapshot.get("type") != "NOT_STARTED"
        ), "HA lamp ON did not start the busy timer (snapshot stayed NOT_STARTED)"

    with allure.step("Turn off the Home Assistant lamp and verify the timer stops"):
        ha.set_switch(entity, False)
        assert _wait_for(
            lambda: matter_smart_home_api.get_switch_state().state is False
        ), "HA lamp OFF did not turn the device switch off"
        assert _wait_for(
            lambda: busy_api.get_snapshot().snapshot.get("type") == "NOT_STARTED"
        ), "HA lamp OFF did not stop the busy timer"


@allure.feature("Matter")
@allure.story("Home Assistant integration")
@allure.title("Device switch state is visible in Home Assistant as {param_id}")
@pytest.mark.timeout(120)
@pytest.mark.parametrize("state", [True, False], ids=["on", "off"])
def test_device_switch_visible_in_ha(
    ha, matter_smart_home_api, commissioned_device, state
):
    """Device-side switch change is visible as the HA entity state."""
    with allure.step(f"Turn the device switch {'on' if state else 'off'}"):
        resp = matter_smart_home_api.set_switch_state(state)
        assert resp.status_code == 200, resp.text

    with allure.step("Wait for and verify the Home Assistant entity state"):
        expected = "on" if state else "off"
        ok, samples = _wait_for_state(
            lambda: (
                f"ha={ha.get_state(commissioned_device['switch_entity'])}"
                f"/dev={matter_smart_home_api.get_switch_state().state}"
            ),
            f"ha={expected}/dev={state}",
        )
        assert (
            ok
        ), f"HA entity did not become {expected}; observed (t, ha/device): {samples}"
