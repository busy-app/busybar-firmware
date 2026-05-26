"""
Synchronous WebSocket client for the firmware ``/api/status/ws`` state
publisher endpoint.

Usage::

    with StatePublisherClient(base_url) as ws:
        ws.drain()
        settings_api.set_name("new")
        update = ws.wait_for(
            lambda u: u.WhichOneof("state") == "device_name"
                      and u.device_name.name == "new",
            timeout=5.0,
        )

The client opens the socket, sends ``{"enable": true}`` to opt in to the
delta stream, then spawns a daemon reader thread that decodes each binary
frame as ``BSB_State.State`` and enqueues each contained ``StateUpdate``.

Heartbeat frames (``State.updates`` empty) and any text frames are
silently ignored.
"""

from __future__ import annotations

import json
import queue
import threading
import time
from typing import Callable, List, Optional
from urllib.parse import urlencode, urlparse, urlunparse

from websocket import (  # type: ignore
    ABNF,
    WebSocketException,
    WebSocketTimeoutException,
    create_connection,
)

# Importing clients.state_pb triggers the on-demand protoc generation and
# wires up sys.path so ``state_pb2`` resolves below.
from clients import state_pb  # noqa: F401  (import for its side effects)
from clients.state_pb import state_pb2


class StatePublisherClient:
    """Sync, context-managed wrapper around websocket-client.

    The reader thread is daemonised so that a stuck test can never wedge
    pytest shutdown; callers that care about clean exit should let the
    context manager close the socket and join the reader.
    """

    WS_PATH = "/api/status/ws"
    ENABLE_FRAME = json.dumps({"enable": True})

    def __init__(self, base_url: str, api_key: Optional[str] = None) -> None:
        self.base_url = base_url
        self.api_key = api_key
        self.ws = None  # type: ignore[assignment]
        self._queue: "queue.Queue" = queue.Queue()
        self._closed = threading.Event()
        self._reader: Optional[threading.Thread] = None
        # Updates skipped during the most recent ``wait_for`` call. Useful
        # for diagnostics on timeout.
        self.seen: List = []

    # ------------------------------------------------------------------ helpers

    def _build_url(self) -> str:
        parsed = urlparse(self.base_url)
        scheme = {"http": "ws", "https": "wss"}.get(parsed.scheme, parsed.scheme)
        query = urlencode({"key": self.api_key}) if self.api_key else ""
        return urlunparse((scheme, parsed.netloc, self.WS_PATH, "", query, ""))

    # ------------------------------------------------------------------ context

    def __enter__(self) -> "StatePublisherClient":
        url = self._build_url()
        self.ws = create_connection(url, enable_multithread=True, timeout=5)
        self.ws.send(self.ENABLE_FRAME)
        self._closed.clear()
        self._reader = threading.Thread(
            target=self._reader_loop, name="state-publisher-reader", daemon=True
        )
        self._reader.start()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self._closed.set()
        try:
            if self.ws is not None:
                self.ws.close()
        except Exception:
            pass
        if self._reader is not None:
            self._reader.join(timeout=2.0)
        self._reader = None
        self.ws = None

    # ------------------------------------------------------------------ reader

    def _reader_loop(self) -> None:
        ws = self.ws
        assert ws is not None
        while not self._closed.is_set():
            try:
                opcode, data = ws.recv_data(control_frame=False)
            except WebSocketTimeoutException:
                continue
            except WebSocketException:
                break
            except OSError:
                break
            except Exception:
                # Any other recv failure ends the reader; the next wait_for
                # will time out cleanly.
                break

            if opcode == ABNF.OPCODE_BINARY:
                if not data:
                    continue
                try:
                    state = state_pb2.State()  # type: ignore[union-attr]
                    state.ParseFromString(data)
                except Exception:
                    continue
                if not state.updates:
                    # Heartbeat / empty delta — ignore.
                    continue
                for update in state.updates:
                    self._queue.put(update)
            else:
                # Text / ping / pong / close: ignore.
                continue

    # ------------------------------------------------------------------ public

    def drain(self) -> None:
        """Discard any updates currently buffered in the queue."""
        try:
            while True:
                self._queue.get_nowait()
        except queue.Empty:
            pass
        self.seen = []

    def wait_for(
        self,
        predicate: Callable[[object], bool],
        timeout: float = 5.0,
    ):
        """Block until ``predicate(update)`` returns truthy or ``timeout`` elapses.

        Skipped (non-matching) updates are appended to ``self.seen`` so the
        caller — or the allure step attached on timeout — can inspect what
        actually arrived.
        """
        self.seen = []
        deadline = time.monotonic() + timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                self._attach_seen_to_allure("timeout (deadline reached)")
                raise TimeoutError(
                    f"wait_for timed out after {timeout:.2f}s; "
                    f"seen kinds: {self._kinds(self.seen)}"
                )
            try:
                update = self._queue.get(timeout=remaining)
            except queue.Empty:
                self._attach_seen_to_allure("timeout (queue empty)")
                raise TimeoutError(
                    f"wait_for timed out after {timeout:.2f}s; "
                    f"seen kinds: {self._kinds(self.seen)}"
                )
            if predicate(update):
                return update
            self.seen.append(update)

    # ------------------------------------------------------------------ diag

    @staticmethod
    def _kinds(updates) -> List[str]:
        out: List[str] = []
        for u in updates:
            try:
                out.append(u.WhichOneof("state") or "<none>")
            except Exception:
                out.append("<unknown>")
        return out

    def _attach_seen_to_allure(self, label: str) -> None:
        try:
            import allure  # type: ignore
        except Exception:
            return
        try:
            body = "\n".join(
                f"{i}: {kind}" for i, kind in enumerate(self._kinds(self.seen))
            ) or "(no updates seen)"
            allure.attach(
                body,
                name=f"state_publisher_ws.wait_for {label}",
                attachment_type=allure.attachment_type.TEXT,
            )
        except Exception:
            pass
