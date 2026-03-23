#pragma once

#include <furi.h>

#define DEVICE_NAME_DEFAULT    "BUSY Bar"
#define DEVICE_NAME_MAX_LENGTH (20U)
#define DEVICE_NAME_MAX_SIZE   (DEVICE_NAME_MAX_LENGTH + 1U)

typedef enum {
    DeviceNameValidationStatusOk,
    DeviceNameValidationStatusEmpty,
    DeviceNameValidationStatusTooLong,
    DeviceNameValidationStatusDisallowedChar,
    DeviceNameValidationStatusOnlySpaces,
    DeviceNameValidationStatusMax,
} DeviceNameValidationStatus;

DeviceNameValidationStatus device_name_validate(const char* name);
