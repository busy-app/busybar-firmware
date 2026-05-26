from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Literal


StateUpdateKind = Literal[
    "device_name",
    "power",
    "brightness",
    "audio_volume",
    "wifi",
    "update_state",
    "update_check",
    "timezone",
    "matter",
    "frame",
    "input",
    "timer",
    "ble",
    "auto_update_state",
    "timer_profiles",
]


@dataclass
class StateUpdate:
    kind: str
    payload: Any
    raw: bytes


@dataclass
class StateFrame:
    timestamp: int | None
    updates: list[StateUpdate]
    errors: list[Any]
    raw: bytes

    @property
    def update_kinds(self) -> set[str]:
        return {update.kind for update in self.updates if update.kind}
