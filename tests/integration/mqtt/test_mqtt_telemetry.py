from __future__ import annotations

import json
import time

import allure
import pytest

from clients.api import (
    AccountAPI,
    AccountBackend,
    AssetsAPI,
    BusyAPI,
    InputAPI,
    SettingsAPI,
    SystemAPI,
    TelemetryAPI,
)
from utils.busy_timer import (
    WORK_CARD_UUID,
    next_timestamp,
    wait_for_snapshot_type,
)
from utils.fetch_mtls_mqtt_broker import (
    PlainMQTTTestBroker,
    PublishedMQTTMessage,
)
from utils.wait import wait_for


PUSH_RATE_LIMIT_S = 5
CANVAS_TEST_APP = "telemetry_buffer_test"
CANVAS_TEST_ELEMENTS = [
    {
        "id": "telemetry-buffer-event",
        "type": "text",
        "text": "Telemetry",
        "x": 36,
        "y": 10,
        "align": "center",
        "font": "normal",
        "color": "#FFFFFFFF",
        "display": "front",
    }
]


def _device_telemetry_topic(system_api: SystemAPI) -> str:
    serial = system_api.get_device_info().serial_number
    return f"devices/{serial}/up/v1/telemetry"


def _decode_batch(message: PublishedMQTTMessage) -> dict:
    try:
        batch = json.loads(message.payload)
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise AssertionError(
            f"Telemetry payload is not valid JSON: {message.payload!r}"
        ) from exc
    assert isinstance(batch, dict), (
        f"Telemetry batch must be an object: {batch!r}"
    )
    return batch


def _has_timer_start_and_end(message: PublishedMQTTMessage) -> bool:
    try:
        batch = json.loads(message.payload)
        event_types = {event.get("t") for event in batch.get("events", [])}
    except (AttributeError, UnicodeDecodeError, json.JSONDecodeError):
        return False
    return {"timer.session.start", "timer.session.end"}.issubset(event_types)


def _has_event_types(*expected_types: str):
    def matches(message: PublishedMQTTMessage) -> bool:
        try:
            batch = json.loads(message.payload)
            event_types = {event.get("t") for event in batch.get("events", [])}
        except (AttributeError, UnicodeDecodeError, json.JSONDecodeError):
            return False
        return set(expected_types).issubset(event_types)

    return matches


def _has_timer_end_count(expected_count: int):
    def matches(message: PublishedMQTTMessage) -> bool:
        try:
            batch = json.loads(message.payload)
            actual_count = sum(
                event.get("t") == "timer.session.end"
                for event in batch.get("events", [])
            )
        except (AttributeError, UnicodeDecodeError, json.JSONDecodeError):
            return False
        return actual_count >= expected_count

    return matches


def _telemetry_publishes(
    broker: PlainMQTTTestBroker,
    topic: str,
    *,
    duration: float,
) -> list[PublishedMQTTMessage]:
    return [
        message
        for message in broker.collect_publishes(duration=duration)
        if message.topic == topic
    ]


def _canvas_cycle(assets_api: AssetsAPI) -> None:
    assets_api.draw(
        CANVAS_TEST_APP,
        CANVAS_TEST_ELEMENTS,
        priority=100,
    )
    assets_api.clear_display()


def _events_of_type(batch: dict, event_type: str) -> list[dict]:
    return [
        event
        for event in batch["events"]
        if event.get("t") == event_type
    ]


def _normalize_hex(value: str) -> str:
    return value.replace(":", "").replace("-", "").lower()


def _assert_event_envelope(event: dict, priority: int) -> None:
    assert set(event) == {"t", "ts", "p", "d"}, event
    assert isinstance(event["ts"], int) and event["ts"] > 0, event
    assert event["p"] == priority, event
    assert isinstance(event["d"], dict), event


def _assert_device_state(event: dict) -> None:
    assert event["p"] == 0, event
    assert set(event["d"]) == {
        "charge",
        "charging",
        "charge_limit",
        "matter_fabrics",
        "matter_commissioned",
        "account_linked",
        "dev_mode",
    }, event
    assert 0 <= event["d"]["charge"] <= 100, event
    assert 0 <= event["d"]["charge_limit"] <= 100, event
    for field in (
        "charging",
        "matter_commissioned",
        "account_linked",
        "dev_mode",
    ):
        assert isinstance(event["d"][field], bool), event
    assert isinstance(event["d"]["matter_fabrics"], int), event
    assert event["d"]["matter_fabrics"] >= 0, event


def _busy_settings(snapshot) -> dict:
    settings = snapshot.snapshot.get("busy_bar_settings")
    if settings:
        return settings
    if snapshot.busy_bar_settings:
        return snapshot.busy_bar_settings.model_dump()
    return {
        "theme": "busy",
        "show_work_phase_only": False,
        "trigger_smart_home": False,
    }


def _start_and_stop_http_timer(
    busy_api: BusyAPI,
    api_session,
    web_base_url: str,
    settings: dict,
) -> None:
    active = {
        "snapshot": {
            "type": "INFINITE",
            "card_id": WORK_CARD_UUID,
            "is_paused": True,
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    response = busy_api.set_snapshot_raw(active)
    assert response.status_code == 200, (
        "Failed to start telemetry timer scenario: "
        f"HTTP {response.status_code}, "
        f"body={response.text[:200]!r}"
    )
    wait_for_snapshot_type(api_session, web_base_url, "INFINITE")

    stopped = {
        "snapshot": {
            "type": "NOT_STARTED",
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    response = busy_api.set_snapshot_raw(stopped)
    assert response.status_code == 200, (
        "Failed to stop telemetry timer scenario: "
        f"HTTP {response.status_code}, "
        f"body={response.text[:200]!r}"
    )
    wait_for_snapshot_type(api_session, web_base_url, "NOT_STARTED")


def _ensure_http_timer_stopped(
    busy_api: BusyAPI,
    api_session,
    web_base_url: str,
    settings: dict,
) -> None:
    stopped = {
        "snapshot": {
            "type": "NOT_STARTED",
            "busy_bar_settings": settings,
        },
        "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
    }
    response = busy_api.set_snapshot_raw(stopped)
    assert response.status_code == 200, (
        "Failed to establish a stopped timer baseline: "
        f"HTTP {response.status_code}, body={response.text[:200]!r}"
    )
    wait_for_snapshot_type(api_session, web_base_url, "NOT_STARTED")


@allure.feature("MQTT")
@allure.story("Telemetry")
@pytest.mark.api
@pytest.mark.mqtt
class TestMqttTelemetry:
    @allure.title("Device reboot publishes the documented device.boot event")
    @pytest.mark.long_running
    def test_reboot_publishes_device_boot(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        persistent_cli_connection,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)

        with allure.step("Enable telemetry and establish a stopped timer"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )
            telemetry_api.set_enabled(True)
            local_mqtt_broker.drain_publishes()

        with allure.step(
            "Reboot the device while the local broker remains up"
        ):
            previous_connections = local_mqtt_broker.diagnostics()[
                "connections"
            ]
            recovered = persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url,
                timeout=90,
            )
            assert recovered, "Device did not recover after telemetry reboot"
            wait_for(
                "device MQTT reconnect after reboot",
                local_mqtt_broker.diagnostics,
                lambda value: value["connections"] > previous_connections,
                timeout=30,
                interval=0.5,
            )

        with allure.step("Capture and validate device.boot"):
            message = local_mqtt_broker.wait_for_publish(
                topic,
                _has_event_types("device.boot"),
                timeout=30,
            )
            batch = _decode_batch(message)
            boot_events = _events_of_type(batch, "device.boot")
            assert len(boot_events) == 1, batch
            event = boot_events[0]
            _assert_event_envelope(event, 2)

            expected_fields = {
                "serial",
                "fw_version",
                "fw_hash",
                "fw_branch",
                "fw_build_date",
                "fw_target",
                "fw_dirty",
                "usb_mac",
                "hw_version",
            }
            assert set(event["d"]) == expected_fields, event

            device = system_api.get_device_info()
            firmware = system_api.get_firmware_info()
            assert _normalize_hex(event["d"]["serial"]) == _normalize_hex(
                device.serial_number
            ), event
            assert _normalize_hex(event["d"]["usb_mac"]) == _normalize_hex(
                device.usb_mac
            ), event
            assert event["d"]["fw_version"] == firmware.version, event
            assert event["d"]["fw_hash"] == firmware.commit_hash, event
            assert event["d"]["fw_branch"] == firmware.branch, event
            assert event["d"]["fw_build_date"] == firmware.build_date, event
            assert event["d"]["fw_target"] == firmware.target, event
            assert isinstance(event["d"]["fw_dirty"], bool), event
            assert isinstance(event["d"]["hw_version"], str), event
            assert event["d"]["hw_version"], event

    @allure.title("HTTP timer lifecycle publishes a valid telemetry batch")
    def test_timer_lifecycle_publishes_telemetry_batch(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)

        with allure.step("Clear buffered telemetry and enable collection"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )
            local_mqtt_broker.drain_publishes()
            telemetry_api.set_enabled(True)
            time.sleep(PUSH_RATE_LIMIT_S + 0.2)

        with allure.step("Start and stop a BUSY timer through the HTTP API"):
            _start_and_stop_http_timer(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )

        with allure.step(
            "Wait for a telemetry batch containing the timer lifecycle"
        ):
            message = local_mqtt_broker.wait_for_publish(
                topic,
                _has_event_types("timer.session.end"),
                timeout=15,
            )
            batch = _decode_batch(message)

        with allure.step("Validate the MQTT and batch envelope contract"):
            assert message.topic == topic, (
                f"Unexpected telemetry topic: {message.topic!r}"
            )
            assert message.qos == 1, (
                f"Telemetry QoS is {message.qos}, expected 1"
            )
            assert message.retained is False, (
                "Telemetry messages must not be retained"
            )
            assert set(batch) == {"schema", "ts", "events"}, (
                f"Unexpected telemetry batch fields: {sorted(batch)}"
            )
            assert batch["schema"] == 1, (
                f"Unexpected telemetry schema: {batch['schema']!r}"
            )
            assert isinstance(batch["ts"], int) and batch["ts"] > 0, (
                f"Invalid telemetry batch timestamp: {batch['ts']!r}"
            )
            assert isinstance(batch["events"], list), (
                f"Telemetry events must be a list: {batch['events']!r}"
            )
            assert 2 <= len(batch["events"]) <= 34, (
                f"Unexpected telemetry batch size: {len(batch['events'])}"
            )

        events = {
            event["t"]: event
            for event in batch["events"]
            if event.get("t") in {"timer.session.start", "timer.session.end"}
        }
        with allure.step("Validate timer event wire fields and HTTP source"):
            assert set(events) == {
                "timer.session.start",
                "timer.session.end",
            }, f"Incomplete timer lifecycle: {events!r}"
            for event_type, priority in (
                ("timer.session.start", 1),
                ("timer.session.end", 2),
            ):
                event = events[event_type]
                _assert_event_envelope(event, priority)
                assert event["d"]["source"] == "http_api", event

            started = events["timer.session.start"]["d"]
            assert set(started) == {
                "source",
                "profile",
                "theme",
                "mode",
                "demo",
            }, started
            assert started["mode"] == "infinite", started
            assert isinstance(started["profile"], str), started
            assert started["profile"], started
            assert isinstance(started["theme"], str), started
            assert started["theme"], started
            assert isinstance(started["demo"], bool), started

            ended = events["timer.session.end"]["d"]
            assert set(ended) == {
                "outcome",
                "source",
                "duration_s",
                "cycles",
            }, ended
            assert ended["outcome"] == "stopped", ended
            assert isinstance(ended["duration_s"], int), ended
            assert ended["duration_s"] >= 0, ended
            assert isinstance(ended["cycles"], int), ended
            assert ended["cycles"] >= 0, ended

    @allure.title("Public APIs emit documented telemetry collector payloads")
    def test_public_api_collectors_publish_documented_payloads(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        telemetry_device_settings_guard,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        settings_api: SettingsAPI,
        assets_api: AssetsAPI,
        input_api: InputAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)
        timer_settings = _busy_settings(telemetry_busy_state_guard)
        current_brightness = settings_api.get_brightness().value
        new_brightness = 37 if current_brightness != "37" else 63
        current_volume = int(settings_api.get_volume().volume)
        new_volume = 23 if current_volume != 23 else 41

        with allure.step("Establish clean collector state"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                timer_settings,
            )
            assets_api.clear_display()
            local_mqtt_broker.drain_publishes()
            telemetry_api.set_enabled(True)
            time.sleep(PUSH_RATE_LIMIT_S + 0.2)

        try:
            with allure.step("Trigger externally controllable collectors"):
                settings_api.set_brightness(str(new_brightness))
                settings_api.set_volume(new_volume)
                switch_response = input_api.send_key("custom")
                assert switch_response.status_code == 200, switch_response.text
                assets_api.draw(
                    CANVAS_TEST_APP,
                    CANVAS_TEST_ELEMENTS,
                    priority=100,
                )
                assets_api.clear_display()
                _start_and_stop_http_timer(
                    busy_api,
                    api_session,
                    web_base_url,
                    timer_settings,
                )

            expected_types = {
                "setting.brightness",
                "setting.volume",
                "input.switch",
                "canvas.acquire",
                "canvas.release",
                "timer.session.start",
                "timer.session.end",
            }
            with allure.step("Capture the collector batch"):
                message = local_mqtt_broker.wait_for_publish(
                    topic,
                    _has_event_types(*expected_types),
                    timeout=15,
                )
                batch = _decode_batch(message)

            with allure.step("Validate setting and input payloads"):
                brightness = _events_of_type(
                    batch,
                    "setting.brightness",
                )[-1]
                _assert_event_envelope(brightness, 0)
                assert brightness["d"] == {
                    "value": new_brightness,
                    "mode": "manual",
                }, brightness

                volume = _events_of_type(batch, "setting.volume")[-1]
                _assert_event_envelope(volume, 0)
                assert set(volume["d"]) == {"volume"}, volume
                assert volume["d"]["volume"] == pytest.approx(
                    new_volume / 100,
                    abs=0.01,
                ), volume

                switch = _events_of_type(batch, "input.switch")[-1]
                _assert_event_envelope(switch, 0)
                assert switch["d"] == {"pos": "status"}, switch

            with allure.step("Validate canvas ownership payloads"):
                acquire = _events_of_type(batch, "canvas.acquire")[-1]
                release = _events_of_type(batch, "canvas.release")[-1]
                _assert_event_envelope(acquire, 1)
                _assert_event_envelope(release, 1)
                assert acquire["d"] == {
                    "app": CANVAS_TEST_APP,
                    "priority": 100,
                }, acquire
                assert release["d"] == {"app": CANVAS_TEST_APP}, release
        finally:
            assets_api.clear_display()

    @allure.title(
        "MQTT reconnect flushes buffered telemetry and offline events"
    )
    def test_reconnect_flushes_offline_backlog(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        account_api: AccountAPI,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)
        settings = _busy_settings(telemetry_busy_state_guard)

        with allure.step("Enable telemetry on a connected local MQTT session"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                settings,
            )
            local_mqtt_broker.drain_publishes()
            telemetry_api.set_enabled(True)

        with allure.step("Disconnect the device from MQTT"):
            account_api.set_backend(
                AccountBackend(
                    server_url="mqtt://192.0.2.1:1883",
                    client_cert_type="none",
                    ignore_server_cert=True,
                )
            )
            status = wait_for(
                "device MQTT session to become disconnected",
                account_api.get_status,
                lambda value: value.status in {"disconnected", "error"},
                timeout=15,
                interval=0.5,
            )
            assert status.status in {"disconnected", "error"}, status
            local_mqtt_broker.drain_publishes()

        with allure.step("Generate timer telemetry while MQTT is offline"):
            _start_and_stop_http_timer(
                busy_api,
                api_session,
                web_base_url,
                settings,
            )
            time.sleep(PUSH_RATE_LIMIT_S + 0.2)
            assert not _telemetry_publishes(
                local_mqtt_broker,
                topic,
                duration=0.2,
            ), "Device published to the local broker while configured offline"

        with allure.step("Reconnect the device to the local broker"):
            previous_connections = local_mqtt_broker.diagnostics()[
                "connections"
            ]
            account_api.set_backend(
                AccountBackend(
                    server_url=local_mqtt_broker.url,
                    client_cert_type="none",
                    ignore_server_cert=True,
                )
            )
            state = wait_for(
                "device MQTT reconnect to the local broker",
                lambda: (
                    account_api.get_status(),
                    local_mqtt_broker.diagnostics(),
                ),
                lambda value: (
                    value[0].status == "connected"
                    and value[1]["connections"] > previous_connections
                ),
                timeout=30,
                interval=0.5,
            )
            assert state[0].status == "connected", state

        expected_types = {
            "net.offline",
            "timer.session.start",
            "timer.session.end",
            "net.offline_duration",
            "net.online",
        }
        with allure.step("Verify reconnect flushes the complete backlog"):
            message = local_mqtt_broker.wait_for_publish(
                topic,
                _has_event_types(*expected_types),
                timeout=15,
            )
            batch = _decode_batch(message)
            actual_types = {event["t"] for event in batch["events"]}
            assert expected_types.issubset(actual_types), (
                f"Reconnect batch is missing events: "
                f"expected={sorted(expected_types)!r}, "
                f"actual={sorted(actual_types)!r}"
            )
            duration_event = _events_of_type(
                batch,
                "net.offline_duration",
            )[0]
            assert duration_event["p"] == 2, duration_event
            assert set(duration_event["d"]) == {"duration_ms"}, (
                duration_event
            )
            assert duration_event["d"]["duration_ms"] >= 5000, (
                duration_event
            )

    @allure.title("Push telemetry is limited to one MQTT batch per 5 seconds")
    def test_push_rate_limit_buffers_repeated_events(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)
        settings = _busy_settings(telemetry_busy_state_guard)

        with allure.step("Enable telemetry with an empty buffer"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                settings,
            )
            local_mqtt_broker.drain_publishes()
            telemetry_api.set_enabled(True)
            time.sleep(PUSH_RATE_LIMIT_S + 0.2)

        with allure.step("Publish the first push-priority timer batch"):
            _start_and_stop_http_timer(
                busy_api,
                api_session,
                web_base_url,
                settings,
            )
            first_message = local_mqtt_broker.wait_for_publish(
                topic,
                _has_timer_start_and_end,
                timeout=15,
            )
            first_received_at = time.monotonic()
            assert _decode_batch(first_message)["events"]

        with allure.step("Generate another p2 event inside the rate window"):
            _start_and_stop_http_timer(
                busy_api,
                api_session,
                web_base_url,
                settings,
            )
            observation_end = first_received_at + PUSH_RATE_LIMIT_S - 0.2
            messages = _telemetry_publishes(
                local_mqtt_broker,
                topic,
                duration=max(0.1, observation_end - time.monotonic()),
            )
            assert not messages, (
                "A second telemetry batch was published inside the 5-second "
                f"rate window: {[message.payload for message in messages]!r}"
            )

        with allure.step("Flush the buffered event after the rate window"):
            remaining = first_received_at + PUSH_RATE_LIMIT_S + 0.2
            time.sleep(max(0, remaining - time.monotonic()))
            _start_and_stop_http_timer(
                busy_api,
                api_session,
                web_base_url,
                settings,
            )
            message = local_mqtt_broker.wait_for_publish(
                topic,
                _has_timer_end_count(2),
                timeout=15,
            )
            batch = _decode_batch(message)
            assert len(_events_of_type(batch, "timer.session.end")) >= 2, (
                batch
            )

    @allure.title("A 32-event buffer publishes one batch with device state")
    def test_full_buffer_flushes_at_32_events(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        telemetry_fixed_brightness,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        assets_api: AssetsAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)

        with allure.step("Establish an empty canvas and telemetry buffer"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )
            assets_api.clear_display()
            local_mqtt_broker.drain_publishes()
            telemetry_api.set_enabled(True)

        try:
            with allure.step("Buffer 30 canvas lifecycle events"):
                for _ in range(15):
                    _canvas_cycle(assets_api)
                assert not _telemetry_publishes(
                    local_mqtt_broker,
                    topic,
                    duration=0.3,
                ), "Telemetry flushed before the documented 32-event limit"

            with allure.step("Buffer event 31 without flushing"):
                assets_api.draw(
                    CANVAS_TEST_APP,
                    CANVAS_TEST_ELEMENTS,
                    priority=100,
                )
                assert not _telemetry_publishes(
                    local_mqtt_broker,
                    topic,
                    duration=0.3,
                ), "Telemetry flushed at 31 buffered events"

            with allure.step("Add event 32 and capture the automatic flush"):
                assets_api.clear_display()
                message = local_mqtt_broker.wait_for_publish(
                    topic,
                    _has_event_types(
                        "canvas.acquire",
                        "canvas.release",
                        "device.state",
                    ),
                    timeout=15,
                )
                batch = _decode_batch(message)

            with allure.step("Validate the full-buffer batch"):
                assert len(batch["events"]) == 33, batch
                acquires = _events_of_type(batch, "canvas.acquire")
                releases = _events_of_type(batch, "canvas.release")
                assert len(acquires) == 16, batch
                assert len(releases) == 16, batch
                assert all(event["p"] == 1 for event in acquires), acquires
                assert all(event["p"] == 1 for event in releases), releases
                assert all(
                    event["d"]["app"] == CANVAS_TEST_APP
                    for event in acquires + releases
                ), batch
                assert all(
                    event["d"]["priority"] == 100 for event in acquires
                ), acquires
                state_events = _events_of_type(batch, "device.state")
                assert len(state_events) == 1, batch
                _assert_device_state(state_events[0])
                assert not _events_of_type(batch, "input.counts"), batch
        finally:
            assets_api.clear_display()

    @allure.title("A normal flush includes and resets aggregated input counts")
    def test_full_flush_includes_input_counts_composite(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        telemetry_fixed_brightness,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        assets_api: AssetsAPI,
        input_api: InputAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)

        with allure.step("Establish an empty canvas and telemetry buffer"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )
            assets_api.clear_display()
            local_mqtt_broker.drain_publishes()
            telemetry_api.set_enabled(True)

        try:
            with allure.step("Accumulate input counts for the flush window"):
                for key in ("up", "up", "down", "down", "down"):
                    response = input_api.send_key(key)
                    assert response.status_code == 200, (
                        f"Input API rejected {key!r}: "
                        f"HTTP {response.status_code}, body={response.text!r}"
                    )

            with allure.step(
                "Fill the event buffer to trigger a normal flush"
            ):
                for _ in range(16):
                    _canvas_cycle(assets_api)
                message = local_mqtt_broker.wait_for_publish(
                    topic,
                    _has_event_types(
                        "device.state",
                        "input.counts",
                    ),
                    timeout=15,
                )
                batch = _decode_batch(message)

            with allure.step("Validate both flush-time composite events"):
                assert len(batch["events"]) == 34, batch
                state_events = _events_of_type(batch, "device.state")
                counts_events = _events_of_type(batch, "input.counts")
                assert len(state_events) == 1, batch
                assert len(counts_events) == 1, batch
                _assert_device_state(state_events[0])

                counts = counts_events[0]
                assert counts["p"] == 0, counts
                assert set(counts["d"]) == {
                    "ok",
                    "back",
                    "start",
                    "wheel_up",
                    "wheel_down",
                }, counts
                assert counts["d"] == {
                    "ok": 0,
                    "back": 0,
                    "start": 0,
                    "wheel_up": 2,
                    "wheel_down": 3,
                }, counts

            with allure.step("Verify input counts reset after the flush"):
                for _ in range(16):
                    _canvas_cycle(assets_api)
                message = local_mqtt_broker.wait_for_publish(
                    topic,
                    _has_event_types("device.state"),
                    timeout=15,
                )
                next_batch = _decode_batch(message)
                assert not _events_of_type(next_batch, "input.counts"), (
                    next_batch
                )
        finally:
            assets_api.clear_display()

    @allure.title("QoS 1 telemetry survives a disconnect before PUBACK")
    def test_unacknowledged_publish_is_recovered_after_reconnect(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        telemetry_fixed_brightness,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        account_api: AccountAPI,
        assets_api: AssetsAPI,
        input_api: InputAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)

        with allure.step("Establish an empty connected telemetry buffer"):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )
            assets_api.clear_display()
            local_mqtt_broker.drain_publishes()
            telemetry_api.set_enabled(True)

        try:
            with allure.step("Prepare 31 buffered events and input counts"):
                for key in ("up", "down"):
                    response = input_api.send_key(key)
                    assert response.status_code == 200, response.text
                for _ in range(15):
                    _canvas_cycle(assets_api)
                assets_api.draw(
                    CANVAS_TEST_APP,
                    CANVAS_TEST_ELEMENTS,
                    priority=100,
                )
                assert not _telemetry_publishes(
                    local_mqtt_broker,
                    topic,
                    duration=0.3,
                ), "Telemetry flushed before the 32nd buffered event"

            with allure.step(
                "Drop the 32-event publish before sending PUBACK"
            ):
                previous_connections = local_mqtt_broker.diagnostics()[
                    "connections"
                ]
                local_mqtt_broker.drop_next_publish(topic)
                assets_api.clear_display()
                dropped = local_mqtt_broker.wait_for_dropped_publish(
                    timeout=15
                )
                assert dropped.topic == topic, dropped
                assert dropped.qos == 1, dropped
                assert dropped.duplicated is False, dropped
                dropped_batch = _decode_batch(dropped)
                assert _events_of_type(dropped_batch, "input.counts"), (
                    dropped_batch
                )

            with allure.step("Wait for the MQTT client to reconnect"):
                state = wait_for(
                    "device reconnect after missing PUBACK",
                    lambda: (
                        account_api.get_status(),
                        local_mqtt_broker.diagnostics(),
                    ),
                    lambda value: (
                        value[0].status == "connected"
                        and value[1]["connections"] > previous_connections
                    ),
                    timeout=30,
                    interval=0.5,
                )
                assert state[0].status == "connected", state

            with allure.step("Verify the unacknowledged batch is redelivered"):
                observed = _telemetry_publishes(
                    local_mqtt_broker,
                    topic,
                    duration=10,
                )
                recovered_messages = [
                    message
                    for message in observed
                    if message.payload == dropped.payload
                ]
                observed_event_types = []
                for message in observed:
                    try:
                        observed_batch = json.loads(message.payload)
                        observed_event_types.append(
                            [
                                event.get("t")
                                for event in observed_batch.get("events", [])
                            ]
                        )
                    except (
                        AttributeError,
                        UnicodeDecodeError,
                        json.JSONDecodeError,
                    ):
                        observed_event_types.append(["<invalid-json>"])
                assert recovered_messages, (
                    "QoS 1 batch was not redelivered after reconnect; "
                    f"post-reconnect event batches={observed_event_types!r}"
                )
                recovered = recovered_messages[0]
                assert recovered.qos == 1, recovered
                assert recovered.payload == dropped.payload, recovered
        finally:
            assets_api.clear_display()

    @allure.title("Opted-out device does not publish timer telemetry")
    def test_disabled_telemetry_does_not_publish(
        self,
        local_mqtt_broker: PlainMQTTTestBroker,
        system_api: SystemAPI,
        telemetry_api: TelemetryAPI,
        busy_api: BusyAPI,
        telemetry_busy_state_guard,
        api_session,
        web_base_url: str,
    ):
        topic = _device_telemetry_topic(system_api)

        with allure.step(
            "Disable telemetry and clear messages received during setup"
        ):
            _ensure_http_timer_stopped(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )
            local_mqtt_broker.drain_publishes()

        with allure.step("Generate a timer start and p2 stop event"):
            _start_and_stop_http_timer(
                busy_api,
                api_session,
                web_base_url,
                _busy_settings(telemetry_busy_state_guard),
            )

        with allure.step("Verify no telemetry message is published"):
            messages = [
                message
                for message in local_mqtt_broker.collect_publishes(
                    duration=2.0
                )
                if message.topic == topic
            ]
            assert not messages, (
                f"Opted-out device published {len(messages)} telemetry "
                "message(s): "
                f"{[message.payload[:200] for message in messages]!r}"
            )
