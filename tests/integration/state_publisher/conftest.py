from __future__ import annotations

import os
import time
from dataclasses import dataclass
from typing import Callable

import pytest

from clients.state_publisher.models import StateFrame
from clients.state_publisher.transports.ble import BleStateTransport
from clients.state_publisher.transports.mqtt import MqttStateTransport
from clients.state_publisher.transports.ws import WsStateTransport
from utils.busy_timer import WORK_CARD_UUID, next_timestamp
from utils.busy_timer import get_snapshot, set_snapshot, wait_for_snapshot_type


def _requested_transports() -> list[str]:
    raw = os.getenv("BSB_STATE_PUBLISHER_TRANSPORTS", "ws")
    return [item.strip() for item in raw.split(",") if item.strip()]


@pytest.fixture
def busy_state_guard(api_session, web_base_url):
    original = get_snapshot(api_session, web_base_url)
    yield original
    try:
        restore = dict(original)
        restore["snapshot_timestamp_ms"] = next_timestamp(api_session, web_base_url)
        set_snapshot(api_session, web_base_url, restore)
        original_type = original.get("snapshot", {}).get("type")
        if original_type:
            wait_for_snapshot_type(api_session, web_base_url, original_type)
    except Exception:
        pass


@pytest.fixture(params=_requested_transports())
def state_transport(request, web_base_url):
    name = request.param
    if name == "ws":
        transport = WsStateTransport(web_base_url)
    elif name == "mqtt":
        mqtt_client = request.getfixturevalue("mqtt_client")
        linked_session = request.getfixturevalue("linked_device_session")
        transport = MqttStateTransport(mqtt_client, linked_session)
    elif name == "ble":
        transport = BleStateTransport()
    else:
        pytest.fail(f"Unknown state publisher transport: {name}")

    transport.connect()
    try:
        yield transport
    finally:
        transport.close()


@pytest.fixture
def state_publisher(state_transport):
    state_transport.enable()
    return StatePublisherProbe(state_transport)


class StatePublisherProbe:
    def __init__(self, transport):
        self.transport = transport

    def read_state(self, timeout: float = 6.0) -> StateFrame:
        return self.transport.read_state(timeout=timeout)

    def wait_for_update(self, kind: str, timeout: float = 8.0) -> StateFrame:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            frame = self.read_state(timeout=max(0.1, deadline - time.monotonic()))
            if kind in frame.update_kinds:
                return frame
        raise AssertionError(f"No {kind!r} state update received over {self.transport.name}")

    def drain(self, duration: float = 0.5) -> list[StateFrame]:
        deadline = time.monotonic() + duration
        frames: list[StateFrame] = []
        while time.monotonic() < deadline:
            try:
                frames.append(self.read_state(timeout=max(0.1, deadline - time.monotonic())))
            except Exception:
                break
        return frames


@dataclass
class StatePublisherEvent:
    name: str
    expected_kind: str
    trigger: Callable[[], None]


@pytest.fixture
def state_event_driver(
    request,
    api_session,
    web_base_url,
    input_api,
    busy_api,
    settings_api,
    system_api,
    update_api,
    busy_state_guard,
):
    def restore_brightness(value: str) -> None:
        try:
            settings_api.set_brightness(value)
        except Exception:
            pass

    def restore_timezone(value: str) -> None:
        try:
            system_api.set_timezone(value)
        except Exception:
            pass

    def restore_autoupdate(original) -> None:
        try:
            update_api.set_autoupdate(
                {
                    "is_enabled": original.is_enabled,
                    "interval_start": original.interval_start,
                    "interval_end": original.interval_end,
                }
            )
        except Exception:
            pass

    def input_event() -> None:
        response = input_api.send_key("back")
        assert response.status_code == 200

    def timer_event() -> None:
        settings = (
            busy_state_guard.get("snapshot", {}).get("busy_bar_settings")
            or busy_state_guard.get("busy_bar_settings")
            or {
                "theme": "busy",
                "show_work_phase_only": False,
                "trigger_smart_home": False,
            }
        )
        response = busy_api.set_snapshot_raw(
            {
                "snapshot": {
                    "type": "SIMPLE",
                    "card_id": WORK_CARD_UUID,
                    "is_paused": True,
                    "time_left_ms": 180000,
                    "busy_bar_settings": settings,
                },
                "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
            }
        )
        assert response.status_code == 200
        wait_for_snapshot_type(api_session, web_base_url, "SIMPLE")

    def brightness_event() -> None:
        original = settings_api.get_brightness().value
        request.addfinalizer(lambda: restore_brightness(original))
        new_value = "10" if str(original) != "10" else "20"
        settings_api.set_brightness(new_value)

    def timezone_event() -> None:
        original = system_api.get_timezone().name
        request.addfinalizer(lambda: restore_timezone(original))
        candidates = [item.name for item in system_api.get_timezone_list().list]
        new_value = next((item for item in candidates if item != original), None)
        if not new_value:
            pytest.skip("No alternate timezone available")
        system_api.set_timezone(new_value)

    def autoupdate_event() -> None:
        original = update_api.get_autoupdate()
        request.addfinalizer(lambda: restore_autoupdate(original))
        update_api.set_autoupdate(
            {
                "is_enabled": not original.is_enabled,
                "interval_start": "02:00",
                "interval_end": "05:00",
            }
        )

    return {
        "input": StatePublisherEvent("input", "input", input_event),
        "timer": StatePublisherEvent("timer", "timer", timer_event),
        "brightness": StatePublisherEvent("brightness", "brightness", brightness_event),
        "timezone": StatePublisherEvent("timezone", "timezone", timezone_event),
        "autoupdate": StatePublisherEvent(
            "autoupdate", "auto_update_state", autoupdate_event
        ),
    }
