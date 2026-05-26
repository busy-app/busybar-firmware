import time
import uuid

import allure
import pytest


@allure.feature("MQTT")
@allure.story("Account unlink")
@pytest.mark.api
@pytest.mark.mqtt
@pytest.mark.uses_cloud
@pytest.mark.external_service
@pytest.mark.regression
class TestMqttAccountUnlink:
    @allure.title("Cloud gone event unlinks the device")
    def test_cloud_gone_unlinks_device(self, account_api, mqtt_client, linked_device_session):
        mqtt_client.publish(f"{linked_device_session.down_topic}/gone")

        deadline = time.monotonic() + 20
        while time.monotonic() < deadline:
            if not account_api.get_info().linked:
                break
            time.sleep(1)

        assert account_api.get_info().linked is False

    @allure.title("Local account unlink publishes MQTT unlink event")
    def test_local_unlink_publishes_up_unlink(
        self, account_api, mqtt_client, linked_device_session
    ):
        mqtt_client.subscribe(f"{linked_device_session.up_topic}/#")

        response = account_api.delete_raw("/api/account")
        assert response.status_code == 200

        message = mqtt_client.wait_for(
            f"{linked_device_session.up_topic}/unlink",
            lambda _message: True,
            timeout=20,
        )
        assert message.topic == f"{linked_device_session.up_topic}/unlink"
        assert account_api.get_info().linked is False

    @allure.title("Gone event for another session is ignored")
    def test_gone_for_other_session_is_ignored(
        self, account_api, mqtt_client, linked_device_session
    ):
        other_session_id = uuid.uuid4().hex
        mqtt_client.publish(f"sessions/{other_session_id}/down/v1/gone")
        time.sleep(3)

        assert account_api.get_info().linked is True
