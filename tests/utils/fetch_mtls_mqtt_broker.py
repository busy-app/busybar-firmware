"""Minimal MQTT v5 broker used by the Fetch/mTLS integration tests."""

from __future__ import annotations

import queue
import socket
import socketserver
import ssl
import threading
from pathlib import Path


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


def _publish_packet_id(flags: int, payload: bytes) -> bytes | None:
    qos = (flags >> 1) & 0x03
    if qos == 0 or len(payload) < 2:
        return None

    topic_length = int.from_bytes(payload[:2], "big")
    packet_id_offset = 2 + topic_length
    packet_id_end = packet_id_offset + 2
    if packet_id_end > len(payload):
        raise ValueError("malformed MQTT PUBLISH packet")
    return payload[packet_id_offset:packet_id_end]


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
        raise ValueError(f"expected MQTT v5, got protocol level {protocol_level}")
    return protocol_level


class _MQTTHandler(socketserver.BaseRequestHandler):
    def handle(self):
        broker = self.server
        connection = self.request
        broker.record_connection(connection.getpeercert(binary_form=True))

        try:
            while True:
                first_byte = _read_exact(connection, 1)[0]
                payload = _read_exact(connection, _read_remaining_length(connection))
                packet_type = first_byte >> 4
                flags = first_byte & 0x0F
                broker.packet_types.put(packet_type)

                if packet_type == 1:  # CONNECT
                    broker.record_connect(_mqtt_connect_protocol_level(flags, payload))
                    connection.sendall(b"\x20\x03\x00\x00\x00")
                    broker.connected.set()
                elif packet_type == 3:  # PUBLISH
                    packet_id = _publish_packet_id(flags, payload)
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


class FetchMTLSMQTTBroker(socketserver.ThreadingTCPServer):
    """Small TLS broker that accepts enough MQTT v5 for device connectivity."""

    allow_reuse_address = True
    daemon_threads = True

    def __init__(
        self,
        server_address,
        *,
        server_certificate_path: Path,
        server_private_key_path: Path,
        client_ca_pem: str,
        allow_partial_chain: bool = False,
    ):
        self.connected = threading.Event()
        self.peer_certificates = queue.Queue()
        self.packet_types = queue.Queue()
        self.connect_protocol_levels = queue.Queue()
        self._state_lock = threading.Lock()
        self._errors = []
        self.connection_count = 0
        self.disconnect_count = 0

        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.minimum_version = ssl.TLSVersion.TLSv1_2
        context.load_cert_chain(server_certificate_path, server_private_key_path)
        context.load_verify_locations(cadata=client_ca_pem)
        context.verify_mode = ssl.CERT_REQUIRED
        if allow_partial_chain:
            context.verify_flags |= ssl.VERIFY_X509_PARTIAL_CHAIN
        self._tls_context = context

        super().__init__(server_address, _MQTTHandler)

    def process_request_thread(self, request, client_address):
        """Perform each TLS handshake outside the broker accept loop."""
        request.settimeout(10)
        try:
            tls_request = self._tls_context.wrap_socket(request, server_side=True)
        except Exception as error:
            self.record_error(error)
            request.close()
            return

        tls_request.settimeout(15)
        super().process_request_thread(tls_request, client_address)

    def record_connection(self, peer_certificate: bytes) -> None:
        with self._state_lock:
            self.connection_count += 1
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
        return f"mqtts://{host}:{port}"
