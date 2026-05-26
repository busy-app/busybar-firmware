import time

import allure
import pytest

from clients.api import AccountBackend


@allure.feature("MQTT")
@allure.story("Connectivity")
@pytest.mark.api
@pytest.mark.mqtt
@pytest.mark.external_service
@pytest.mark.regression
class TestMqttConnectivity:
    @allure.title("Unreachable MQTT backend does not wedge HTTP")
    def test_unreachable_mqtt_backend_does_not_wedge_http(
        self, account_api, system_api, mqtt_config, account_backend_guard
    ):
        account_api.set_backend(
            AccountBackend(
                server_url="mqtt://192.0.2.1:1883",
                client_cert_type="none",
                ignore_server_cert=True,
            )
        )

        deadline = time.monotonic() + 10
        status = account_api.get_status()
        while time.monotonic() < deadline and status.status == "connected":
            time.sleep(1)
            status = account_api.get_status()

        assert status.status in {"error", "disconnected"}
        assert system_api.get_version().api_semver
