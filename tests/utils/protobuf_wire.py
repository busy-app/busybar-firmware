from __future__ import annotations


def read_varint(payload: bytes, offset: int) -> tuple[int, int]:
    value = 0
    shift = 0
    while offset < len(payload):
        byte = payload[offset]
        offset += 1
        value |= (byte & 0x7F) << shift
        if not (byte & 0x80):
            return value, offset
        shift += 7
        if shift > 70:
            break
    raise ValueError("Invalid protobuf varint")


def protobuf_fields(payload: bytes) -> list[tuple[int, int, int | bytes]]:
    fields: list[tuple[int, int, int | bytes]] = []
    offset = 0
    while offset < len(payload):
        key, offset = read_varint(payload, offset)
        field_number = key >> 3
        wire_type = key & 0x07

        if wire_type == 0:
            value, offset = read_varint(payload, offset)
        elif wire_type == 1:
            value = payload[offset : offset + 8]
            offset += 8
        elif wire_type == 2:
            length, offset = read_varint(payload, offset)
            value = payload[offset : offset + length]
            offset += length
        elif wire_type == 5:
            value = payload[offset : offset + 4]
            offset += 4
        else:
            raise ValueError(f"Unsupported protobuf wire type: {wire_type}")

        fields.append((field_number, wire_type, value))
    return fields
