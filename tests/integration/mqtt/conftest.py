from __future__ import annotations

import json
import os
import threading
import time
import uuid

import pytest

from clients.api import AccountBackend, AccountAPI
from clients.mqtt_cloud import (
    CloudAccountClient,
    LinkedMqttSession,
    MqttCloudClient,
    MqttDependencyError,
)
from config.mqtt import MqttHarnessConfig
from utils.fetch_mtls_mqtt_broker import BrokerMessage, MQTTBroker
from utils.wait import wait_for


LOCAL_LINK_CODE = "1234"
LOCAL_LINK_EMAIL = "mqtt-integration@busy.local"


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
def local_mqtt_broker(persistent_cli_connection):
    """Start the repository MQTT mock on the device USB network."""
    host_ip = os.getenv("BSB_LOCAL_MQTT_BIND_HOST") or (
        persistent_cli_connection.tn.sock.getsockname()[0]
    )
    host_port = int(os.getenv("BSB_LOCAL_MQTT_BIND_PORT", "0"))
    link_request = threading.Event()
    link_device_id = {"value": None}
    broker = MQTTBroker((host_ip, host_port))

    def handle_message(message: BrokerMessage) -> None:
        parts = message.topic.split("/")
        if (
            len(parts) == 6
            and parts[0] == "devices"
            and parts[2:] == ["up", "v1", "link", "request"]
        ):
            device_id = parts[1]
            link_device_id["value"] = device_id
            broker.publish(
                f"devices/{device_id}/down/v1/link/otp",
                json.dumps(
                    {
                        "code": LOCAL_LINK_CODE,
                        "expires_at": int(time.time()) + 60,
                    }
                ),
            )
            link_request.set()

    broker.message_callback = handle_message
    broker.link_request = link_request
    broker.link_device_id = link_device_id
    thread = threading.Thread(target=broker.serve_forever, daemon=True)
    thread.start()
    try:
        yield broker
    finally:
        broker.shutdown()
        thread.join(timeout=2)
        broker.server_close()


@pytest.fixture
def local_mqtt_config(local_mqtt_broker) -> MqttHarnessConfig:
    return MqttHarnessConfig(
        server_url=local_mqtt_broker.url,
        topic_prefix="",
        client_id_prefix="bsb-local-tests",
        ca_path=None,
        client_cert_path=None,
        client_key_path=None,
        username=None,
        password=None,
        ignore_server_cert=False,
        connect_timeout_s=10,
        message_timeout_s=15,
        cloud_test_api_url=None,
        cloud_test_user=None,
        cloud_test_password=None,
        cloud_link_command=None,
    )


@pytest.fixture
def local_mqtt_client(local_mqtt_config: MqttHarnessConfig):
    client = MqttCloudClient(local_mqtt_config)
    try:
        client.connect()
    except MqttDependencyError as exc:
        pytest.skip(str(exc))
    try:
        yield client
    finally:
        client.disconnect()


@pytest.fixture
def local_linked_device_session(
    account_api: AccountAPI,
    local_mqtt_broker,
) -> LinkedMqttSession:
    """Connect the device to the mock and create a local session if needed."""
    original_backend = account_api.get_backend()
    original_status = account_api.get_status().status
    created_local_session = False

    try:
        account_api.set_backend(
            AccountBackend(
                server_url=os.getenv(
                    "BSB_LOCAL_MQTT_DEVICE_URL", local_mqtt_broker.url
                ),
                client_cert_type="default",
                ignore_server_cert=False,
            )
        )
        assert local_mqtt_broker.device_connected.wait(timeout=30), (
            "Device did not subscribe on the local MQTT mock: "
            f"{local_mqtt_broker.diagnostics()}"
        )

        account_info = account_api.get_info()
        if account_info.linked:
            session_id = account_info.id
            device_id = None
            account_id = account_info.user_id
        else:
            link = account_api.link()
            assert link.code == LOCAL_LINK_CODE, (
                f"Expected local link code {LOCAL_LINK_CODE}, got {link.code}"
            )
            assert local_mqtt_broker.link_request.wait(timeout=1), (
                "MQTT mock did not receive the device link request"
            )
            device_id = local_mqtt_broker.link_device_id["value"]
            assert device_id, "MQTT mock could not determine the device id"

            session_id = str(uuid.uuid4())
            account_id = str(uuid.uuid4())
            local_mqtt_broker.publish(
                f"devices/{device_id}/down/v1/link/token",
                json.dumps(
                    {
                        "session_id": session_id,
                        "token": uuid.uuid4().hex,
                        "email": LOCAL_LINK_EMAIL,
                        "user_id": account_id,
                    }
                ),
            )
            wait_for(
                "device to accept the local MQTT session",
                account_api.get_info,
                lambda info: info.linked and info.id == session_id,
                timeout=30,
                interval=0.5,
            )
            created_local_session = True

        assert local_mqtt_broker.http_proxy_ready.wait(timeout=30), (
            "Device HTTP proxy did not subscribe on the local MQTT mock: "
            f"{local_mqtt_broker.diagnostics()}"
        )
        assert session_id, "Linked device did not expose a session id"
        base = f"sessions/{session_id}"
        yield LinkedMqttSession(
            session_id=session_id,
            device_id=device_id,
            account_id=account_id,
            up_topic=f"{base}/up/v1",
            down_topic=f"{base}/down/v1",
        )
    finally:
        if created_local_session:
            try:
                if account_api.get_info().linked:
                    account_api.unlink()
            except Exception:
                pass
        account_api.set_backend(original_backend)
        wait_for(
            "device to disconnect from the local MQTT mock",
            local_mqtt_broker.device_connected.is_set,
            lambda connected: not connected,
            timeout=30,
            interval=0.1,
        )
        if original_status == "connected":
            wait_for(
                "device to reconnect to the original MQTT backend",
                account_api.get_status,
                lambda status: status.status == "connected",
                timeout=30,
                interval=0.5,
            )
