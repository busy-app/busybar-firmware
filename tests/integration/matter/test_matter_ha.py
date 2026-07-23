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

from integration.matter.helpers import (
    assert_switch_state,
    assert_timer_type,
    interval_timer_snapshot,
    running_timer_snapshot,
    set_device_switch,
    set_ha_switch,
    set_timer,
    stopped_timer_snapshot,
)

pytestmark = [
    pytest.mark.matter,
    pytest.mark.regression,
    pytest.mark.uses_si917,
    pytest.mark.external_service,
]


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
        set_device_switch(ha, matter_smart_home_api, entity, not state)

    with allure.step(f"Turn the Home Assistant switch {'on' if state else 'off'}"):
        ha.set_switch(entity, state)

    with allure.step("Wait for and verify the device switch state"):
        assert_switch_state(ha, matter_smart_home_api, entity, state)


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
        set_timer(busy_api, running_timer_snapshot(True))
        assert_switch_state(ha, matter_smart_home_api, entity, True)

    with allure.step("Stop the timer and verify the lamp turns off"):
        set_timer(busy_api, stopped_timer_snapshot(True))
        assert_switch_state(ha, matter_smart_home_api, entity, False)


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
        set_timer(busy_api, stopped_timer_snapshot(False))
        assert_switch_state(ha, matter_smart_home_api, entity, False)

    with allure.step("Run the timer with smart-home control disabled"):
        set_timer(busy_api, running_timer_snapshot(False))

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
        set_timer(busy_api, running_timer_snapshot(True))
        assert_switch_state(ha, matter_smart_home_api, entity, True)

    with allure.step("Pause the timer and verify the lamp turns off"):
        paused = dict(running_timer_snapshot(True), is_paused=True)
        set_timer(busy_api, paused)
        assert_switch_state(ha, matter_smart_home_api, entity, False)

    with allure.step("Resume the timer and verify the lamp turns on"):
        set_timer(busy_api, running_timer_snapshot(True))
        assert_switch_state(ha, matter_smart_home_api, entity, True)


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
        set_timer(busy_api, interval_timer_snapshot(0, True))  # WORK
        assert_switch_state(ha, matter_smart_home_api, entity, True)

    with allure.step("Start a rest interval and verify the lamp turns off"):
        set_timer(busy_api, interval_timer_snapshot(1, True))  # REST
        assert_switch_state(ha, matter_smart_home_api, entity, False)


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
        set_ha_switch(ha, matter_smart_home_api, entity, True)
        assert_timer_type(
            busy_api,
            "NOT_STARTED",
            equal=False,
            message="HA lamp ON did not start the busy timer",
        )

    with allure.step("Turn off the Home Assistant lamp and verify the timer stops"):
        set_ha_switch(ha, matter_smart_home_api, entity, False)
        assert_timer_type(
            busy_api,
            "NOT_STARTED",
            message="HA lamp OFF did not stop the busy timer",
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
        set_ha_switch(ha, matter_smart_home_api, entity, not state)

    with allure.step(f"Turn the device switch {'on' if state else 'off'}"):
        resp = matter_smart_home_api.set_switch_state(state)
        assert resp.status_code == 200, resp.text

    with allure.step("Wait for and verify the Home Assistant entity state"):
        assert_switch_state(ha, matter_smart_home_api, entity, state)
