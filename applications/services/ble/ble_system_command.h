#pragma once

#include "ble_i.h"

typedef enum {
    BleCommandUnknown,
    BleCommandInit,
    BleCommandEnable,
    BleCommandDisable,
    BleCommandGetState,
    BleCommandForgetPairing,
    BleCommandGetPairing,
    BleCommandSetDeviceName,
    BleCommandConnectionUpdated,

    BleCommandCount
} BleSystemCommand;

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events);

bool ble_command_request_process(BleIntercomFrameGeneric* frame, void* context);
bool ble_command_response_process(BleIntercomFrameGeneric* frame, void* context);

extern const BleCommandItem ble_commands[];

void ble_invoke_retry_command_on_internal_event(
    Ble* instance,
    BleSystemCommand command,
    BleEventType retry_event,
    uint32_t retry_timeout);
