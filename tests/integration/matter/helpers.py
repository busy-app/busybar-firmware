"""Shared helpers for the Matter/Home Assistant suite.

Switch-state assertions poll both sides (HA entity + device API) until they
agree; timer helpers drive /api/busy snapshots and wait for propagation.
"""

from __future__ import annotations

import time

from utils.busy_timer import WORK_CARD_UUID, next_timestamp
from utils.wait import wait_for


def _sampling_observer(observe):
    """Wrap a getter to record a (t, value) timeline of distinct observations."""
    samples = []
    t0 = time.monotonic()

    def observer():
        observed = observe()
        if not samples or samples[-1][1] != observed:
            samples.append((round(time.monotonic() - t0, 1), observed))
        return observed

    return observer, samples


def assert_switch_state(ha, smart_home_api, entity: str, expected: bool) -> None:
    """Wait until HA and the device report the same expected switch state."""
    expected_name = "on" if expected else "off"
    observer, samples = _sampling_observer(
        lambda: (ha.get_state(entity), smart_home_api.get_switch_state().state)
    )
    try:
        wait_for(
            f"switch to become {expected_name} in HA and on the device",
            observer,
            lambda observed: observed == (expected_name, expected),
            timeout=30.0,
            interval=0.3,
        )
    except AssertionError:
        raise AssertionError(
            f"switch did not become {expected_name} in HA and on the device; "
            f"observed (t, (ha, device)): {samples}"
        ) from None


def set_ha_switch(ha, smart_home_api, entity: str, state: bool) -> None:
    ha.set_switch(entity, state)
    assert_switch_state(ha, smart_home_api, entity, state)


def set_device_switch(ha, smart_home_api, entity: str, state: bool) -> None:
    resp = smart_home_api.set_switch_state(state)
    assert resp.status_code == 200, resp.text
    assert_switch_state(ha, smart_home_api, entity, state)


def assert_timer_type(
    busy_api,
    expected_type: str,
    *,
    equal: bool = True,
    message: str,
    timeout: float = 20.0,
):
    observer, samples = _sampling_observer(
        lambda: busy_api.get_snapshot().snapshot.get("type")
    )
    try:
        wait_for(
            f"busy timer snapshot type {'==' if equal else '!='} {expected_type}",
            observer,
            lambda snapshot_type: (snapshot_type == expected_type) is equal,
            timeout=timeout,
            interval=0.3,
        )
    except AssertionError:
        raise AssertionError(
            f"{message}; observed (t, type): {samples}"
        ) from None


def set_timer(busy_api, snapshot: dict) -> None:
    timestamp = next_timestamp(busy_api.session, busy_api.base_url)
    body = {
        "snapshot": snapshot,
        "snapshot_timestamp_ms": timestamp,
    }
    resp = busy_api.set_snapshot_raw(body)
    assert resp.status_code == 200, resp.text

    def observe():
        current = busy_api.get_snapshot()
        return (current.snapshot_timestamp_ms, current.snapshot.get("type"))

    wait_for(
        f"timer snapshot {timestamp}/{snapshot['type']} to apply",
        observe,
        lambda observed: observed == (timestamp, snapshot["type"]),
        timeout=5.0,
        interval=0.1,
    )


def _busy_settings(trigger_smart_home: bool) -> dict:
    return {
        "theme": "busy",
        "show_work_phase_only": False,
        "trigger_smart_home": trigger_smart_home,
    }


def running_timer_snapshot(trigger_smart_home: bool) -> dict:
    return {
        "type": "SIMPLE",
        "card_id": WORK_CARD_UUID,
        "is_paused": False,
        "time_left_ms": 180_000,
        "busy_bar_settings": _busy_settings(trigger_smart_home),
    }


def stopped_timer_snapshot(trigger_smart_home: bool) -> dict:
    return {
        "type": "NOT_STARTED",
        "busy_bar_settings": _busy_settings(trigger_smart_home),
    }


def interval_timer_snapshot(interval_index: int, trigger_smart_home: bool) -> dict:
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


def establish_idle_lamp_off(ha, smart_home_api, busy_api, entity: str) -> None:
    set_timer(busy_api, stopped_timer_snapshot(True))
    set_ha_switch(ha, smart_home_api, entity, False)
