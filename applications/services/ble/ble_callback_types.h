#pragma once

#include <furi.h>

typedef void (*BleDataUpdatedCallback)(size_t data_size, void* data, void* context);
typedef void (*BleDataTransmitDoneCallback)(void* context);
