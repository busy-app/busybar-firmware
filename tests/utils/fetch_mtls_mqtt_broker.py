"""Minimal MQTT v5 brokers used by device integration tests."""

from __future__ import annotations

import queue
import socket
import socketserver
import ssl
import threading
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Callable


@dataclass(frozen=True)
class PublishedMQTTMessage:
    """MQTT PUBLISH packet captured from the device."""

    topic: str
    payload: bytes
    qos: int
    retained: bool
    duplicated: bool


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


def _decode_variable_byte_integer(data: bytes, offset: int) -> tuple[int, int]:
    value = 0
    multiplier = 1
    for _ in range(4):
        if offset >= len(data):
            raise ValueError("truncated MQTT variable byte integer")
        encoded = data[offset]
        offset += 1
        value += (encoded & 0x7F) * multiplier
        if encoded & 0x80 == 0:
            return value, offset
        multiplier *= 128
    raise ValueError("malformed MQTT variable byte integer")


def _parse_publish(
    flags: int,
    packet: bytes,
) -> tuple[PublishedMQTTMessage, bytes | None]:
    qos = (flags >> 1) & 0x03
    if qos == 3 or len(packet) < 2:
        raise ValueError("malformed MQTT PUBLISH packet")

    topic_length = int.from_bytes(packet[:2], "big")
    offset = 2
    topic_end = offset + topic_length
    if topic_end > len(packet):
        raise ValueError("truncated MQTT PUBLISH topic")
    try:
        topic = packet[offset:topic_end].decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError("MQTT PUBLISH topic is not UTF-8") from error
    offset = topic_end

    packet_id = None
    if qos > 0:
        packet_id_end = offset + 2
        if packet_id_end > len(packet):
            raise ValueError("truncated MQTT PUBLISH packet identifier")
        packet_id = packet[offset:packet_id_end]
        offset = packet_id_end

    properties_length, offset = _decode_variable_byte_integer(packet, offset)
    application_payload_offset = offset + properties_length
    if application_payload_offset > len(packet):
        raise ValueError("truncated MQTT PUBLISH properties")

    return (
        PublishedMQTTMessage(
            topic=topic,
            payload=packet[application_payload_offset:],
            qos=qos,
            retained=bool(flags & 0x01),
            duplicated=bool(flags & 0x08),
        ),
        packet_id,
    )


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


class _MQTTHandler(socketserver.BaseRequestHandler):
    def handle(self):
        broker = self.server
        connection = self.request
        getpeercert = getattr(connection, "getpeercert", None)
        peer_certificate = (
            getpeercert(binary_form=True) if getpeercert is not None else None
        )
        broker.record_connection(peer_certificate)

        try:
            while True:
                first_byte = _read_exact(connection, 1)[0]
                remaining_length = _read_remaining_length(connection)
                payload = _read_exact(connection, remaining_length)
                packet_type = first_byte >> 4
                flags = first_byte & 0x0F
                broker.packet_types.put(packet_type)

                if packet_type == 1:  # CONNECT
                    protocol_level = _mqtt_connect_protocol_level(
                        flags,
                        payload,
                    )
                    broker.record_connect(protocol_level)
                    connection.sendall(b"\x20\x03\x00\x00\x00")
                    broker.connected.set()
                elif packet_type == 3:  # PUBLISH
                    message, packet_id = _parse_publish(flags, payload)
                    if broker.should_drop_publish(message):
                        broker.record_dropped_publish(message)
                        connection.shutdown(socket.SHUT_RDWR)
                        break
                    broker.record_publish(message)
                    if packet_id is not None:
                        connection.sendall(b"\x40\x02" + packet_id)
                elif packet_type == 8:  # SUBSCRIBE
                    if len(payload) < 2:
                        raise ValueError("malformed MQTT SUBSCRIBE packet")
                    connection.sendall(b"\x90\x04" + payload[:2] + b"\x00\x00")
                elif packet_type == 12:  # PINGREQ
                    connection.sendall(b"\xD0\x00")
                elif packet_type == 14:  # DISCONNECT
                    break
        except (EOFError, OSError):
            pass
        except Exception as error:
            broker.record_error(error)
        finally:
            broker.record_disconnect()


class _MQTTTestBrokerBase(socketserver.ThreadingTCPServer):
    """Shared MQTT v5 protocol state for plain and mTLS test brokers."""

    allow_reuse_address = True
    daemon_threads = True

    def __init__(self, server_address):
        self.connected = threading.Event()
        self.peer_certificates = queue.Queue()
        self.packet_types = queue.Queue()
        self.connect_protocol_levels = queue.Queue()
        self.published_messages = queue.Queue()
        self.dropped_publishes = queue.Queue()
        self._state_lock = threading.Lock()
        self._errors = []
        self._drop_next_publish_topic = None
        self.connection_count = 0
        self.disconnect_count = 0
        super().__init__(server_address, _MQTTHandler)

    def record_connection(self, peer_certificate: bytes | None) -> None:
        with self._state_lock:
            self.connection_count += 1
        self.peer_certificates.put(peer_certificate)

    def record_connect(self, protocol_level: int) -> None:
        self.connect_protocol_levels.put(protocol_level)

    def record_publish(self, message: PublishedMQTTMessage) -> None:
        self.published_messages.put(message)

    def drop_next_publish(self, topic: str) -> None:
        """Close the client connection before acknowledging a publish."""
        with self._state_lock:
            self._drop_next_publish_topic = topic

    def should_drop_publish(self, message: PublishedMQTTMessage) -> bool:
        with self._state_lock:
            if self._drop_next_publish_topic != message.topic:
                return False
            self._drop_next_publish_topic = None
            return True

    def record_dropped_publish(self, message: PublishedMQTTMessage) -> None:
        self.dropped_publishes.put(message)

    def wait_for_dropped_publish(
        self,
        *,
        timeout: float,
    ) -> PublishedMQTTMessage:
        """Return a publish whose connection was dropped before PUBACK."""
        try:
            return self.dropped_publishes.get(timeout=timeout)
        except queue.Empty as error:
            raise AssertionError(
                "Timed out waiting for fault-injected MQTT PUBLISH; "
                f"broker={self.diagnostics()!r}"
            ) from error

    def record_disconnect(self) -> None:
        with self._state_lock:
            self.disconnect_count += 1

    def record_error(self, error: Exception) -> None:
        with self._state_lock:
            self._errors.append(repr(error))

    def wait_for_publish(
        self,
        topic: str,
        predicate: Callable[[PublishedMQTTMessage], bool],
        *,
        timeout: float,
    ) -> PublishedMQTTMessage:
        """Wait for the first captured publish matching topic and predicate."""
        deadline = time.monotonic() + timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise AssertionError(
                    f"Timed out waiting for MQTT PUBLISH on {topic!r}; "
                    f"broker={self.diagnostics()!r}"
                )
            try:
                message = self.published_messages.get(timeout=remaining)
            except queue.Empty as error:
                raise AssertionError(
                    f"Timed out waiting for MQTT PUBLISH on {topic!r}; "
                    f"broker={self.diagnostics()!r}"
                ) from error
            if message.topic == topic and predicate(message):
                return message

    def collect_publishes(
        self,
        *,
        duration: float,
    ) -> list[PublishedMQTTMessage]:
        """Collect captured publishes until the observation window closes."""
        messages = []
        deadline = time.monotonic() + duration
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return messages
            try:
                messages.append(self.published_messages.get(timeout=remaining))
            except queue.Empty:
                return messages

    def drain_publishes(self) -> list[PublishedMQTTMessage]:
        """Remove and return all publishes already captured by the broker."""
        messages = []
        while True:
            try:
                messages.append(self.published_messages.get_nowait())
            except queue.Empty:
                return messages

    def diagnostics(self) -> dict:
        """Return connection state suitable for a pytest timeout message."""
        with self._state_lock:
            return {
                "connections": self.connection_count,
                "disconnects": self.disconnect_count,
                "errors": list(self._errors),
            }


class PlainMQTTTestBroker(_MQTTTestBrokerBase):
    """Plain TCP broker that captures MQTT v5 publishes from the device."""

    def process_request_thread(self, request, client_address):
        request.settimeout(15)
        super().process_request_thread(request, client_address)

    @property
    def url(self) -> str:
        host, port = self.server_address[:2]
        return f"mqtt://{host}:{port}"


class FetchMTLSMQTTBroker(_MQTTTestBrokerBase):
    """Small TLS broker that accepts enough MQTT v5 for device connectivity."""

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
            server_certificate_path,
            server_private_key_path,
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
                request,
                server_side=True,
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
