"""Small MQTT v5 brokers used by hardware integration tests."""

from __future__ import annotations

import queue
import socket
import socketserver
import ssl
import threading
from dataclasses import dataclass
from pathlib import Path
from typing import Callable


def _read_exact(connection: socket.socket, size: int) -> bytes:
    data = bytearray()
    while len(data) < size:
        chunk = connection.recv(size - len(data))
        if not chunk:
            raise EOFError("MQTT connection closed")
        data.extend(chunk)
    return bytes(data)


def _read_remaining_length(connection: socket.socket) -> int:
    value = 0
    multiplier = 1
    for _ in range(4):
        encoded = _read_exact(connection, 1)[0]
        value += (encoded & 0x7F) * multiplier
        if encoded & 0x80 == 0:
            return value
        multiplier *= 128
    raise ValueError("malformed MQTT remaining length")


def _decode_variable_integer(data: bytes, offset: int) -> tuple[int, int]:
    value = 0
    multiplier = 1
    for _ in range(4):
        if offset >= len(data):
            raise ValueError("truncated MQTT variable integer")
        encoded = data[offset]
        offset += 1
        value += (encoded & 0x7F) * multiplier
        if encoded & 0x80 == 0:
            return value, offset
        multiplier *= 128
    raise ValueError("malformed MQTT variable integer")


def _encode_variable_integer(value: int) -> bytes:
    encoded = bytearray()
    while True:
        byte = value % 128
        value //= 128
        if value:
            byte |= 0x80
        encoded.append(byte)
        if not value:
            return bytes(encoded)


def _mqtt_connect_protocol_level(flags: int, payload: bytes) -> int:
    if flags != 0 or len(payload) < 7:
        raise ValueError("malformed MQTT CONNECT packet")
    protocol_name_length = int.from_bytes(payload[:2], "big")
    protocol_name_end = 2 + protocol_name_length
    if protocol_name_end >= len(payload):
        raise ValueError("truncated MQTT CONNECT protocol name")
    if payload[2:protocol_name_end] != b"MQTT":
        raise ValueError("unexpected MQTT CONNECT protocol name")
    protocol_level = payload[protocol_name_end]
    if protocol_level != 5:
        raise ValueError(
            f"expected MQTT v5, got protocol level {protocol_level}"
        )
    return protocol_level


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


@dataclass(frozen=True)
class BrokerMessage:
    topic: str
    payload: bytes
    qos: int
    properties: bytes


class _BrokerClient:
    def __init__(self, connection: socket.socket):
        self.connection = connection
        self.subscriptions: list[str] = []
        self._send_lock = threading.Lock()
        self._next_packet_id = 1

    def send(self, packet: bytes) -> None:
        with self._send_lock:
            self.connection.sendall(packet)

    def next_packet_id(self) -> bytes:
        with self._send_lock:
            packet_id = self._next_packet_id
            self._next_packet_id = 1 if packet_id == 65535 else packet_id + 1
        return packet_id.to_bytes(2, "big")


class _MQTTHandler(socketserver.BaseRequestHandler):
    def handle(self):
        broker = self.server
        connection = self.request
        client = _BrokerClient(connection)
        peer_certificate = None
        if isinstance(connection, ssl.SSLSocket):
            peer_certificate = connection.getpeercert(binary_form=True)
        broker.record_connection(peer_certificate)
        broker.add_client(client)

        try:
            while True:
                first_byte = _read_exact(connection, 1)[0]
                payload = _read_exact(
                    connection, _read_remaining_length(connection)
                )
                packet_type = first_byte >> 4
                flags = first_byte & 0x0F
                broker.packet_types.put(packet_type)

                if packet_type == 1:  # CONNECT
                    broker.record_connect(
                        _mqtt_connect_protocol_level(flags, payload)
                    )
                    client.send(b"\x20\x03\x00\x00\x00")
                    broker.connected.set()
                elif packet_type == 3:  # PUBLISH
                    packet_id, message = broker.parse_publish(flags, payload)
                    broker.route_message(client, message)
                    if message.qos == 1 and packet_id is not None:
                        client.send(b"\x40\x02" + packet_id)
                    elif message.qos == 2 and packet_id is not None:
                        client.send(b"\x50\x02" + packet_id)
                elif packet_type == 6:  # PUBREL
                    if len(payload) < 2:
                        raise ValueError("malformed MQTT PUBREL packet")
                    client.send(b"\x70\x02" + payload[:2])
                elif packet_type == 8:  # SUBSCRIBE
                    packet_id, subscriptions = broker.parse_subscribe(payload)
                    client.subscriptions.extend(
                        topic for topic, _qos in subscriptions
                    )
                    reason_codes = bytes(qos for _topic, qos in subscriptions)
                    response = packet_id + b"\x00" + reason_codes
                    client.send(
                        b"\x90"
                        + _encode_variable_integer(len(response))
                        + response
                    )
                elif packet_type == 12:  # PINGREQ
                    client.send(b"\xD0\x00")
                elif packet_type == 14:  # DISCONNECT
                    break
        except (EOFError, OSError):
            pass
        except Exception as error:
            broker.record_error(error)
        finally:
            broker.remove_client(client)
            broker.record_disconnect()


class MQTTBroker(socketserver.ThreadingTCPServer):
    """Plain local MQTT v5 broker with subscription-based message routing."""

    allow_reuse_address = True
    daemon_threads = True

    def __init__(
        self,
        server_address,
        *,
        message_callback: Callable[[BrokerMessage], None] | None = None,
    ):
        self.connected = threading.Event()
        self.peer_certificates = queue.Queue()
        self.packet_types = queue.Queue()
        self.connect_protocol_levels = queue.Queue()
        self.messages = queue.Queue()
        self.message_callback = message_callback
        self._state_lock = threading.Lock()
        self._clients: list[_BrokerClient] = []
        self._errors = []
        self.connection_count = 0
        self.disconnect_count = 0
        super().__init__(server_address, _MQTTHandler)

    @staticmethod
    def parse_publish(
        flags: int, payload: bytes
    ) -> tuple[bytes | None, BrokerMessage]:
        if len(payload) < 3:
            raise ValueError("malformed MQTT PUBLISH packet")
        topic_length = int.from_bytes(payload[:2], "big")
        topic_end = 2 + topic_length
        if topic_end > len(payload):
            raise ValueError("truncated MQTT PUBLISH topic")
        topic = payload[2:topic_end].decode("utf-8")
        qos = (flags >> 1) & 0x03
        cursor = topic_end
        packet_id = None
        if qos:
            if cursor + 2 > len(payload):
                raise ValueError("truncated MQTT PUBLISH packet id")
            packet_id = payload[cursor:cursor + 2]
            cursor += 2

        properties_start = cursor
        properties_length, cursor = _decode_variable_integer(payload, cursor)
        message_start = cursor + properties_length
        if message_start > len(payload):
            raise ValueError("truncated MQTT PUBLISH properties")

        return packet_id, BrokerMessage(
            topic=topic,
            payload=payload[message_start:],
            qos=qos,
            properties=payload[properties_start:message_start],
        )

    @staticmethod
    def parse_subscribe(payload: bytes) -> tuple[bytes, list[tuple[str, int]]]:
        if len(payload) < 4:
            raise ValueError("malformed MQTT SUBSCRIBE packet")
        packet_id = payload[:2]
        properties_length, cursor = _decode_variable_integer(payload, 2)
        cursor += properties_length
        subscriptions = []
        while cursor < len(payload):
            if cursor + 2 > len(payload):
                raise ValueError("truncated MQTT SUBSCRIBE topic length")
            topic_length = int.from_bytes(payload[cursor:cursor + 2], "big")
            cursor += 2
            topic_end = cursor + topic_length
            if topic_end + 1 > len(payload):
                raise ValueError("truncated MQTT SUBSCRIBE topic")
            topic = payload[cursor:topic_end].decode("utf-8")
            cursor = topic_end
            qos = min(payload[cursor] & 0x03, 2)
            cursor += 1
            subscriptions.append((topic, qos))
        if not subscriptions:
            raise ValueError("MQTT SUBSCRIBE packet has no topics")
        return packet_id, subscriptions

    def add_client(self, client: _BrokerClient) -> None:
        with self._state_lock:
            self._clients.append(client)

    def remove_client(self, client: _BrokerClient) -> None:
        with self._state_lock:
            if client in self._clients:
                self._clients.remove(client)

    def route_message(
        self,
        sender: _BrokerClient | None,
        message: BrokerMessage,
    ) -> None:
        self.messages.put(message)
        if self.message_callback is not None:
            self.message_callback(message)

        with self._state_lock:
            recipients = [
                client
                for client in self._clients
                if client is not sender
                and any(
                    _topic_matches(filter_, message.topic)
                    for filter_ in client.subscriptions
                )
            ]

        topic = message.topic.encode("utf-8")
        for recipient in recipients:
            variable_header = len(topic).to_bytes(2, "big") + topic
            if message.qos:
                variable_header += recipient.next_packet_id()
            body = variable_header + message.properties + message.payload
            flags = message.qos << 1
            recipient.send(
                bytes([0x30 | flags])
                + _encode_variable_integer(len(body))
                + body
            )

    def publish(
        self,
        topic: str,
        payload: bytes | str = b"",
        *,
        qos: int = 0,
    ) -> None:
        if qos not in {0, 1, 2}:
            raise ValueError(f"invalid MQTT QoS: {qos}")
        data = payload.encode("utf-8") if isinstance(payload, str) else payload
        self.route_message(
            None,
            BrokerMessage(
                topic=topic,
                payload=data,
                qos=qos,
                properties=b"\x00",
            ),
        )

    def record_connection(self, peer_certificate: bytes | None) -> None:
        with self._state_lock:
            self.connection_count += 1
        if peer_certificate is not None:
            self.peer_certificates.put(peer_certificate)

    def record_connect(self, protocol_level: int) -> None:
        self.connect_protocol_levels.put(protocol_level)

    def record_disconnect(self) -> None:
        with self._state_lock:
            self.disconnect_count += 1

    def record_error(self, error: Exception) -> None:
        with self._state_lock:
            self._errors.append(repr(error))

    def diagnostics(self) -> dict:
        """Return connection state suitable for a pytest timeout message."""
        with self._state_lock:
            return {
                "connections": self.connection_count,
                "disconnects": self.disconnect_count,
                "errors": list(self._errors),
            }

    @property
    def url(self) -> str:
        host, port = self.server_address[:2]
        return f"mqtt://{host}:{port}"


class FetchMTLSMQTTBroker(MQTTBroker):
    """MQTT v5 broker that requires the device mTLS certificate."""

    def __init__(
        self,
        server_address,
        *,
        server_certificate_path: Path,
        server_private_key_path: Path,
        client_ca_pem: str,
        allow_partial_chain: bool = False,
    ):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.minimum_version = ssl.TLSVersion.TLSv1_2
        context.load_cert_chain(
            server_certificate_path, server_private_key_path
        )
        context.load_verify_locations(cadata=client_ca_pem)
        context.verify_mode = ssl.CERT_REQUIRED
        if allow_partial_chain:
            context.verify_flags |= ssl.VERIFY_X509_PARTIAL_CHAIN
        self._tls_context = context
        super().__init__(server_address)

    def process_request_thread(self, request, client_address):
        """Perform each TLS handshake outside the broker accept loop."""
        request.settimeout(10)
        try:
            tls_request = self._tls_context.wrap_socket(
                request, server_side=True
            )
        except Exception as error:
            self.record_error(error)
            request.close()
            return

        tls_request.settimeout(15)
        super().process_request_thread(tls_request, client_address)

    @property
    def url(self) -> str:
        host, port = self.server_address[:2]
        return f"mqtts://{host}:{port}"
