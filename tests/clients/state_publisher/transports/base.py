from __future__ import annotations

from typing import Protocol

from clients.state_publisher.models import StateFrame


class StatePublisherTransport(Protocol):
    name: str

    def connect(self) -> None:
        ...

    def enable(self) -> None:
        ...

    def read_frame(self, timeout: float = 6.0) -> bytes:
        ...

    def read_state(self, timeout: float = 6.0) -> StateFrame:
        ...

    def close(self) -> None:
        ...
