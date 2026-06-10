from __future__ import annotations

from clients.mqtt_cloud import LinkedMqttSession, MqttCloudClient
from clients.state_publisher.decoder import decode_state_frame
from clients.state_publisher.models import StateFrame


class MqttStateTransport:
    name = "mqtt"

    def __init__(self, mqtt_client: MqttCloudClient, session: LinkedMqttSession):
        self.mqtt_client = mqtt_client
        self.session = session

    def connect(self) -> None:
        self.mqtt_client.connect()
        self.mqtt_client.subscribe(f"{self.session.up_topic}/#")

    def enable(self) -> None:
        # MQTT state publishing is device/session driven; no HTTP-style enable
        # command is known for this transport.
        return None

    def read_frame(self, timeout: float = 6.0) -> bytes:
        message = self.mqtt_client.wait_for(
            f"{self.session.up_topic}/#",
            lambda item: item.payload,
            timeout=timeout,
        )
        return message.payload

    def read_state(self, timeout: float = 6.0) -> StateFrame:
        return decode_state_frame(self.read_frame(timeout=timeout))

    def close(self) -> None:
        self.mqtt_client.disconnect()
