from __future__ import annotations

import json
import queue
import ssl
import subprocess
import time
import uuid
from dataclasses import dataclass
from typing import Callable

import requests

from config.mqtt import MqttHarnessConfig


class MqttDependencyError(RuntimeError):
    pass


@dataclass
class MqttMessage:
    topic: str
    payload: bytes
    qos: int
    retained: bool
    received_at: float


@dataclass
class LinkedMqttSession:
    session_id: str
    device_id: str | None
    account_id: str | None
    up_topic: str
    down_topic: str


@dataclass
class CloudLinkResult:
    account_id: str | None
    device_id: str | None
    session_id: str | None


def _topic_matches(filter_: str, topic: str) -> bool:
    filter_parts = filter_.split("/")
    topic_parts = topic.split("/")
    for index, filter_part in enumerate(filter_parts):
        if filter_part == "#":
            return True
        if index >= len(topic_parts):
            return False
        if filter_part != "+" and filter_part != topic_parts[index]:
            return False
    return len(filter_parts) == len(topic_parts)


class MqttCloudClient:
    def __init__(self, config: MqttHarnessConfig):
        self.config = config
        self.messages: "queue.Queue[MqttMessage]" = queue.Queue()
        self.client = None

    def connect(self) -> None:
        if self.client is not None:
            return
        try:
            import paho.mqtt.client as mqtt
        except ImportError as exc:
            raise MqttDependencyError("Install paho-mqtt to run MQTT harness tests") from exc

        parsed = self.config.parsed_server_url
        if parsed.scheme not in {"mqtt", "mqtts"}:
            raise ValueError(f"Unsupported MQTT URL scheme: {parsed.scheme}")

        client_id = f"{self.config.client_id_prefix}-{uuid.uuid4().hex[:12]}"
        client = mqtt.Client(client_id=client_id)
        if self.config.username:
            client.username_pw_set(self.config.username, self.config.password)
        if parsed.scheme == "mqtts":
            client.tls_set(
                ca_certs=str(self.config.ca_path) if self.config.ca_path else None,
                certfile=str(self.config.client_cert_path)
                if self.config.client_cert_path
                else None,
                keyfile=str(self.config.client_key_path)
                if self.config.client_key_path
                else None,
                cert_reqs=ssl.CERT_NONE
                if self.config.ignore_server_cert
                else ssl.CERT_REQUIRED,
            )
            if self.config.ignore_server_cert:
                client.tls_insecure_set(True)

        def on_message(_client, _userdata, msg):
            self.messages.put(
                MqttMessage(
                    topic=msg.topic,
                    payload=msg.payload,
                    qos=msg.qos,
                    retained=msg.retain,
                    received_at=time.monotonic(),
                )
            )

        client.on_message = on_message
        client.connect(
            parsed.hostname,
            parsed.port or (8883 if parsed.scheme == "mqtts" else 1883),
            keepalive=30,
        )
        client.loop_start()
        self.client = client

    def disconnect(self) -> None:
        if self.client is None:
            return
        self.client.loop_stop()
        self.client.disconnect()
        self.client = None

    def subscribe(self, topic_filter: str, qos: int = 1) -> None:
        if self.client is None:
            raise RuntimeError("MQTT client is not connected")
        result, _mid = self.client.subscribe(topic_filter, qos=qos)
        if result:
            raise RuntimeError(f"MQTT subscribe failed with code {result}")

    def publish(
        self, topic: str, payload: bytes | str = b"", qos: int = 1, retain: bool = False
    ) -> None:
        if self.client is None:
            raise RuntimeError("MQTT client is not connected")
        payload_bytes = payload.encode("utf-8") if isinstance(payload, str) else payload
        result = self.client.publish(topic, payload=payload_bytes, qos=qos, retain=retain)
        result.wait_for_publish(timeout=self.config.message_timeout_s)
        if result.rc:
            raise RuntimeError(f"MQTT publish failed with code {result.rc}")

    def wait_for(
        self,
        topic_filter: str,
        predicate: Callable[[MqttMessage], bool],
        timeout: float | None = None,
    ) -> MqttMessage:
        deadline = time.monotonic() + (timeout or self.config.message_timeout_s)
        while time.monotonic() < deadline:
            try:
                message = self.messages.get(timeout=max(0.1, deadline - time.monotonic()))
            except queue.Empty:
                break
            if _topic_matches(topic_filter, message.topic) and predicate(message):
                return message
        raise AssertionError(f"No MQTT message matched {topic_filter}")

    def drain(self, topic_filter: str, duration: float) -> list[MqttMessage]:
        deadline = time.monotonic() + duration
        matched: list[MqttMessage] = []
        while time.monotonic() < deadline:
            try:
                message = self.messages.get(timeout=max(0.1, deadline - time.monotonic()))
            except queue.Empty:
                continue
            if _topic_matches(topic_filter, message.topic):
                matched.append(message)
        return matched


class CloudAccountClient:
    def __init__(self, config: MqttHarnessConfig):
        self.config = config
        self.session = requests.Session()

    def redeem_link_code(self, code: str) -> CloudLinkResult:
        if self.config.cloud_link_command:
            output = subprocess.check_output(
                self.config.cloud_link_command.format(code=code),
                shell=True,
                text=True,
                timeout=60,
            )
            data = json.loads(output)
            return CloudLinkResult(
                account_id=data.get("account_id"),
                device_id=data.get("device_id"),
                session_id=data.get("session_id"),
            )

        if not (
            self.config.cloud_test_api_url
            and self.config.cloud_test_user
            and self.config.cloud_test_password
        ):
            raise RuntimeError(
                "Configure BSB_CLOUD_LINK_COMMAND or BSB_CLOUD_TEST_API_URL/"
                "BSB_CLOUD_TEST_USER/BSB_CLOUD_TEST_PASSWORD to link test accounts"
            )

        base = self.config.cloud_test_api_url.rstrip("/")
        login = self.session.post(
            f"{base}/auth/login",
            json={
                "email": self.config.cloud_test_user,
                "password": self.config.cloud_test_password,
            },
            timeout=20,
        )
        login.raise_for_status()

        response = self.session.post(f"{base}/devices/link", json={"code": code}, timeout=20)
        response.raise_for_status()
        data = response.json()
        return CloudLinkResult(
            account_id=data.get("account_id"),
            device_id=data.get("device_id"),
            session_id=data.get("session_id"),
        )
