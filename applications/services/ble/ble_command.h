#pragma once

#include "ble_i.h"

BleIntercomFrameGeneric* ble_command_preprocess(Ble* instance, uint32_t events);

void ble_command_postprocess(Ble* instance, uint32_t events, bool result);

void ble_command_handler_enable(Ble* instance, BleIntercomFrameGeneric* frame);

void ble_command_handler_disable(Ble* instance, BleIntercomFrameGeneric* frame);

void ble_command_handler_get_status(Ble* instance, BleIntercomFrameStatus* frame);
