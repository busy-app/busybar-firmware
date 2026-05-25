from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path
from urllib.parse import urlparse


@dataclass(frozen=True)
class MqttHarnessConfig:
    server_url: str
    topic_prefix: str
    client_id_prefix: str
    ca_path: Path | None
    client_cert_path: Path | None
    client_key_path: Path | None
    username: str | None
    password: str | None
    ignore_server_cert: bool
    connect_timeout_s: float
    message_timeout_s: float
    cloud_test_api_url: str | None
    cloud_test_user: str | None
    cloud_test_password: str | None
    cloud_link_command: str | None

    @classmethod
    def from_env(cls) -> "MqttHarnessConfig | None":
        server_url = os.getenv("BSB_MQTT_TEST_URL")
        if not server_url:
            return None

        def optional_path(name: str) -> Path | None:
            value = os.getenv(name)
            return Path(value).expanduser() if value else None

        return cls(
            server_url=server_url,
            topic_prefix=os.getenv("BSB_MQTT_TOPIC_PREFIX", "").strip("/"),
            client_id_prefix=os.getenv("BSB_MQTT_CLIENT_ID_PREFIX", "bsb-tests"),
            ca_path=optional_path("BSB_MQTT_CA_PATH"),
            client_cert_path=optional_path("BSB_MQTT_CLIENT_CERT_PATH"),
            client_key_path=optional_path("BSB_MQTT_CLIENT_KEY_PATH"),
            username=os.getenv("BSB_MQTT_USERNAME"),
            password=os.getenv("BSB_MQTT_PASSWORD"),
            ignore_server_cert=os.getenv("BSB_MQTT_IGNORE_SERVER_CERT", "").lower()
            in {"1", "true", "yes"},
            connect_timeout_s=float(os.getenv("BSB_MQTT_CONNECT_TIMEOUT_S", "10")),
            message_timeout_s=float(os.getenv("BSB_MQTT_MESSAGE_TIMEOUT_S", "15")),
            cloud_test_api_url=os.getenv("BSB_CLOUD_TEST_API_URL"),
            cloud_test_user=os.getenv("BSB_CLOUD_TEST_USER"),
            cloud_test_password=os.getenv("BSB_CLOUD_TEST_PASSWORD"),
            cloud_link_command=os.getenv("BSB_CLOUD_LINK_COMMAND"),
        )

    @property
    def parsed_server_url(self):
        return urlparse(self.server_url)

    @property
    def device_backend_url(self) -> str:
        return self.server_url

    def topic(self, *parts: str) -> str:
        clean_parts = [part.strip("/") for part in parts if part.strip("/")]
        if self.topic_prefix:
            clean_parts.insert(0, self.topic_prefix)
        return "/".join(clean_parts)
