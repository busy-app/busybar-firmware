#pragma once

#include "ble_intercom_types.h"
#include "ble.h"

typedef enum {
    BleCommandEngineExtractFrameSourceCommandBuffer,
    BleCommandEngineExtractFrameSourceIntercomBuffer,
} BleCommandEngineExtractFrameSource;

typedef BleIntercomFrameGeneric* (
    *BleCommandEngineExtractFrame)(Ble* instance, BleCommandEngineExtractFrameSource source);

typedef bool (*BleRequestCommandHandler)(BleIntercomFrameGeneric* frame, void* context);
typedef bool (*BleResponseCommandHandler)(BleIntercomFrameGeneric* frame, void* context);

typedef struct {
    BleRequestCommandHandler request;
    BleResponseCommandHandler response;
} BleCommandItem;

typedef struct BleCommandEngine BleCommandEngine;

BleCommandEngine* ble_command_engine_alloc(
    Ble* ble,
    const BleCommandItem* commands,
    uint8_t commands_count,
    BleCommandEngineExtractFrame extract_frame);

bool ble_command_engine_run(BleCommandEngine* instance, BleCommandEngineExtractFrameSource source);
