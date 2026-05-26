from __future__ import annotations

import time

import pytest

from clients.api import AccountBackend, AccountAPI
from clients.mqtt_cloud import (
    CloudAccountClient,
    LinkedMqttSession,
    MqttCloudClient,
    MqttDependencyError,
)
from config.mqtt import MqttHarnessConfig


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
