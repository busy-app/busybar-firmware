#pragma once

#include "ble_intercom_types.h"

typedef void (*BleEngineCommandPreProcess)(BleIntercomFrameGeneric* frame, void* context);
typedef void (*BleEngineCommandPostProcess)(BleIntercomFrameGeneric* frame, void* context);

typedef bool (*BleRequestCommandHandler)(BleIntercomFrameGeneric* frame, void* context);
typedef bool (*BleResponseCommandHandler)(BleIntercomFrameGeneric* frame, void* context);

typedef struct {
    BleRequestCommandHandler request;
    BleResponseCommandHandler response;
} BleCommandItem;

typedef struct BleCommandEngine BleCommandEngine;

BleCommandEngine* ble_command_engine_alloc(
    const BleCommandItem* commands,
    uint8_t commands_count,
    BleEngineCommandPreProcess pre_process,
    BleEngineCommandPostProcess post_process);

bool ble_command_engine_run(
    BleCommandEngine* instance,
    BleIntercomFrameGeneric* frame,
    void* context);
