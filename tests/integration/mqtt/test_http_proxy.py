import uuid

import allure
import pytest
import yaml


HTTP_METHODS = {"delete", "get", "patch", "post", "put"}
BLOCKED_RESPONSE = b"HTTP/1.1 422 Unprocessable Entity\r\n\r\n"
DISRUPTIVE_OPERATIONS = {
    ("POST", "/api/wifi/disconnect"): 1,
    ("DELETE", "/api/account"): 2,
}


def _local_only_operations(spec: dict) -> list[tuple[str, str]]:
    operations = []
    for path, path_item in spec.get("paths", {}).items():
        if not isinstance(path_item, dict):
            continue
        for method, operation in path_item.items():
            if (
                method.lower() in HTTP_METHODS
                and isinstance(operation, dict)
                and operation.get("x-local-only") is True
            ):
                operations.append((method.upper(), path))

    return sorted(
        operations,
        key=lambda operation: (
            DISRUPTIVE_OPERATIONS.get(operation, 0),
            operation,
        ),
    )


def _http_request(method: str, path: str) -> bytes:
    headers = [
        f"{method} {path} HTTP/1.1",
        "Host: 127.0.0.1",
        "Content-Length: 0",
    ]
    if path == "/api/status/ws":
        headers.extend(
            [
                "Connection: Upgrade",
                "Upgrade: websocket",
                "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==",
                "Sec-WebSocket-Version: 13",
            ]
        )
    return ("\r\n".join(headers) + "\r\n\r\n").encode("ascii")


def _http_status(response: bytes) -> int:
    status_line = response.split(b"\r\n", 1)[0]
    try:
        return int(status_line.split(b" ", 2)[1])
    except (IndexError, ValueError) as exc:
        raise AssertionError(
            f"Invalid HTTP response status line: {status_line!r}"
        ) from exc


@allure.feature("MQTT")
@allure.story("HTTP proxy")
@pytest.mark.api
@pytest.mark.mqtt
class TestMqttHttpProxy:
    @allure.title("OpenAPI x-local-only operations are rejected over MQTT")
    def test_local_only_operations_are_rejected(
        self,
        api_session,
        web_base_url,
        local_mqtt_client,
        local_linked_device_session,
    ):
        with allure.step(
            "Load x-local-only operations from the device OpenAPI spec"
        ):
            response = api_session.get(f"{web_base_url}/openapi.yaml")
            assert response.status_code == 200, (
                f"Expected OpenAPI HTTP 200, got {response.status_code}: "
                f"{response.text[:200]}"
            )
            operations = _local_only_operations(yaml.safe_load(response.text))
            assert operations, (
                "The device OpenAPI spec has no x-local-only operations"
            )

        response_topic_prefix = (
            f"{local_linked_device_session.up_topic}/http-response"
        )
        local_mqtt_client.subscribe(f"{response_topic_prefix}/#", qos=1)

        failures = []
        for method, path in operations:
            with allure.step(f"Verify {method} {path} is rejected"):
                request_id = uuid.uuid4().hex
                response_topic = f"{response_topic_prefix}/{request_id}"
                request = _http_request(method, path)

                local_mqtt_client.publish(
                    f"{local_linked_device_session.down_topic}/http-request",
                    request,
                    qos=1,
                    response_topic=response_topic,
                    correlation_data=request_id.encode("ascii"),
                )
                message = local_mqtt_client.wait_for(
                    response_topic,
                    lambda candidate: candidate.topic == response_topic,
                )
                allure.attach(
                    request.decode("ascii"),
                    name=f"{method} {path} request",
                    attachment_type=allure.attachment_type.TEXT,
                )
                allure.attach(
                    message.payload.decode("utf-8", errors="replace"),
                    name=f"{method} {path} response",
                    attachment_type=allure.attachment_type.TEXT,
                )

                if message.payload != BLOCKED_RESPONSE:
                    status = _http_status(message.payload)
                    failures.append(
                        f"{method} {path}: expected HTTP 422, "
                        f"got HTTP {status}"
                    )

        assert not failures, (
            "x-local-only operations accepted by the MQTT HTTP proxy: "
            + ", ".join(failures)
        )
