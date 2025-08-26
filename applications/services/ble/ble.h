#pragma once

#include <furi.h>

/**
 * @brief BLE FURI record identifier.
 */
#define RECORD_BLE "ble"

typedef struct Ble Ble;

bool ble_start(Ble* ble);

bool ble_stop(Ble* ble);
