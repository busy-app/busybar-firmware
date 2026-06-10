from __future__ import annotations

import sys
from pathlib import Path
from typing import Any

from utils.protobuf_wire import protobuf_fields

from .models import StateFrame, StateUpdate


STATE_UPDATE_FIELDS = {
    1: "device_name",
    2: "power",
    3: "brightness",
    4: "audio_volume",
    5: "wifi",
    6: "update_state",
    7: "update_check",
    8: "timezone",
    9: "matter",
    10: "frame",
    11: "input",
    12: "timer",
    13: "ble",
    14: "auto_update_state",
    15: "timer_profiles",
}


def _load_generated_state_model():
    generated_dir = Path(__file__).resolve().parents[1] / "state_pb" / "_generated"
    if generated_dir.exists() and str(generated_dir) not in sys.path:
        sys.path.insert(0, str(generated_dir))
    try:
        from clients.state_pb._generated import state_pb2

        return state_pb2.State
    except Exception:
        return None


GeneratedState = _load_generated_state_model()


def _decode_generated_state(payload: bytes) -> StateFrame | None:
    if GeneratedState is None:
        return None

    message = GeneratedState()
    message.ParseFromString(payload)
    updates: list[StateUpdate] = []
    for update in message.updates:
        kind = update.WhichOneof("state")
        if not kind:
            continue
        updates.append(StateUpdate(kind=kind, payload=getattr(update, kind), raw=update.SerializeToString()))

    errors: list[Any] = []
    try:
        if message.HasField("error"):
            errors.append(message.error)
    except ValueError:
        pass

    timestamp = message.timestamp if message.timestamp else None
    return StateFrame(timestamp=timestamp, updates=updates, errors=errors, raw=payload)


def _decode_wire_state(payload: bytes) -> StateFrame:
    fields = protobuf_fields(payload)
    timestamps = [value for number, wire, value in fields if number == 1 and wire == 0]
    raw_updates = [value for number, wire, value in fields if number == 2 and wire == 2]
    raw_errors = [value for number, wire, value in fields if number == 3 and wire == 2]

    updates: list[StateUpdate] = []
    for raw_update in raw_updates:
        assert isinstance(raw_update, bytes)
        for kind in state_update_kinds(raw_update):
            updates.append(StateUpdate(kind=kind, payload=None, raw=raw_update))

    return StateFrame(
        timestamp=timestamps[-1] if timestamps else None,
        updates=updates,
        errors=raw_errors,
        raw=payload,
    )


def decode_state_frame(payload: bytes) -> StateFrame:
    generated = _decode_generated_state(payload)
    if generated is not None:
        return generated
    return _decode_wire_state(payload)


def state_update_kinds(update: StateUpdate | bytes) -> set[str]:
    if isinstance(update, StateUpdate):
        return {update.kind} if update.kind else set()

    return {
        STATE_UPDATE_FIELDS[number]
        for number, wire, _value in protobuf_fields(update)
        if wire == 2 and number in STATE_UPDATE_FIELDS
    }
