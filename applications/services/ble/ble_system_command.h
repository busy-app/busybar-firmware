#pragma once

#include "ble_i.h"

typedef enum {
    BleCommandUnknown,
    BleCommandInit,
    BleCommandDeinit,
    BleCommandEnable,
    BleCommandDisable,
    BleCommandGetStatus,
    BleCommandSetStatus,
    BleCommandForgetPairing,
    BleCommandSetDeviceName,
    BleCommandDisconnect,

    BleCommandCount
} BleSystemCommand;

BleIntercomFrameGeneric*
    ble_command_extract_frame(Ble* instance, BleCommandEngineExtractFrameSource source);

bool ble_command_request_process(BleIntercomFrameGeneric* frame, void* context);
bool ble_command_response_process(BleIntercomFrameGeneric* frame, void* context);
bool ble_command_deinit_process(BleIntercomFrameGeneric* frame, void* context);

void ble_command_unblock_with_result(Ble* instance, bool result);

extern const BleCommandItem ble_commands[];

void ble_invoke_retry_command_on_internal_event(
    Ble* instance,
    BleSystemCommand command,
    BleEventType retry_event,
    uint32_t retry_timeout);
