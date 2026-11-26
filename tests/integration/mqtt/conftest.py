from __future__ import annotations

import time
import threading

import allure
import pytest

from clients.api import (
    AccountBackend,
    AccountAPI,
    BusyAPI,
    SettingsAPI,
    TelemetryAPI,
)
from clients.mqtt_cloud import (
    CloudAccountClient,
    LinkedMqttSession,
    MqttCloudClient,
    MqttDependencyError,
)
from config.mqtt import MqttHarnessConfig
from utils.busy_timer import next_timestamp, wait_for_snapshot_type
from utils.fetch_mtls_mqtt_broker import PlainMQTTTestBroker
from utils.wait import wait_for


@pytest.fixture(scope="session")
def mqtt_config() -> MqttHarnessConfig:
    config = MqttHarnessConfig.from_env()
    if config is None:
        pytest.skip("BSB_MQTT_TEST_URL is not configured")
    return config


@pytest.fixture
def mqtt_client(mqtt_config: MqttHarnessConfig):
    client = MqttCloudClient(mqtt_config)
    try:
        client.connect()
    except MqttDependencyError as exc:
        pytest.skip(str(exc))
    try:
        yield client
    finally:
        client.disconnect()


@pytest.fixture
def account_backend_guard(account_api: AccountAPI):
    original_backend = account_api.get_backend()
    yield
    try:
        account_api.unlink()
    except Exception:
        pass
    try:
        account_api.set_backend(original_backend)
    except Exception:
        pass


@pytest.fixture
def linked_device_session(
    account_api: AccountAPI,
    mqtt_config: MqttHarnessConfig,
    account_backend_guard,
) -> LinkedMqttSession:
    account_api.set_backend(
        AccountBackend(
            server_url=mqtt_config.device_backend_url,
            client_cert_type="default",
            ignore_server_cert=mqtt_config.ignore_server_cert,
        )
    )

    link = account_api.link()
    try:
        result = CloudAccountClient(mqtt_config).redeem_link_code(link.code)
    except Exception as exc:
        pytest.skip(f"Cloud account link is not configured or failed: {exc}")

    deadline = time.monotonic() + 30
    while time.monotonic() < deadline:
        if account_api.get_info().linked:
            break
        time.sleep(1)
    else:
        pytest.fail("Device did not become linked after cloud link flow")

    session_id = result.session_id
    if not session_id:
        pytest.skip("Cloud link result did not include session_id")

    base = mqtt_config.topic("sessions", session_id)
    return LinkedMqttSession(
        session_id=session_id,
        device_id=result.device_id,
        account_id=result.account_id,
        up_topic=f"{base}/up/v1",
        down_topic=f"{base}/down/v1",
    )


@pytest.fixture
def local_mqtt_broker(
    persistent_cli_connection,
    account_api: AccountAPI,
    telemetry_api: TelemetryAPI,
    telemetry_state_guard,
) -> PlainMQTTTestBroker:
    """Route device MQTT traffic to a broker controlled by the test process."""
    original_backend = account_api.get_backend()
    telemetry_api.set_enabled(False)

    host_ip = persistent_cli_connection.tn.sock.getsockname()[0]
    broker = PlainMQTTTestBroker((host_ip, 0))
    server_thread = threading.Thread(
        target=broker.serve_forever,
        name="local-mqtt-test-broker",
        daemon=True,
    )
    server_thread.start()

    try:
        account_api.set_backend(
            AccountBackend(
                server_url=broker.url,
                client_cert_type="none",
                ignore_server_cert=True,
            )
        )
        state = wait_for(
            "device MQTT session to connect to the local broker",
            lambda: (account_api.get_status(), broker.diagnostics()),
            lambda value: (
                value[0].status == "connected" and broker.connected.is_set()
            ),
            timeout=30,
            interval=0.5,
        )
        assert state[0].status == "connected", state
        broker.drain_publishes()
        yield broker
    finally:
        try:
            telemetry_api.set_enabled(False)
        finally:
            try:
                account_api.set_backend(original_backend)
            finally:
                broker.shutdown()
                broker.server_close()
                server_thread.join(timeout=5)


@pytest.fixture
def telemetry_fixed_brightness(
    local_mqtt_broker: PlainMQTTTestBroker,
    settings_api: SettingsAPI,
    telemetry_api: TelemetryAPI,
):
    """Prevent ambient-light changes from adding p0 events to exact batches."""
    original_brightness = settings_api.get_brightness().value
    settings_api.set_brightness("50")
    yield
    telemetry_api.set_enabled(False)
    settings_api.set_brightness(original_brightness)


@pytest.fixture
def telemetry_device_settings_guard(
    local_mqtt_broker: PlainMQTTTestBroker,
    settings_api: SettingsAPI,
    telemetry_api: TelemetryAPI,
):
    """Restore brightness and volume changed by telemetry collector tests."""
    original_brightness = settings_api.get_brightness().value
    original_volume = int(settings_api.get_volume().volume)
    yield
    telemetry_api.set_enabled(False)
    settings_api.set_brightness(original_brightness)
    settings_api.set_volume(original_volume)


@pytest.fixture
def telemetry_busy_state_guard(
    busy_api: BusyAPI,
    api_session,
    web_base_url: str,
):
    """Restore the BUSY timer state after telemetry event scenarios."""
    original = busy_api.get_snapshot()
    yield original

    try:
        restore = {
            "snapshot": original.snapshot,
            "snapshot_timestamp_ms": next_timestamp(api_session, web_base_url),
        }
        response = busy_api.set_snapshot_raw(restore)
        with allure.step("Restore BUSY timer state"):
            assert response.status_code == 200, (
                f"Failed to restore BUSY state: HTTP {response.status_code}, "
                f"body={response.text[:200]!r}"
            )
        wait_for_snapshot_type(
            api_session,
            web_base_url,
            original.snapshot["type"],
        )
    except Exception:
        pass
