#pragma once

#include "intercom.h"
#include "intercom_frame.h"

#include <furi.h>

#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>
#include <furi_hal_resources.h>

void intercom_sync_request(const GpioPin* gpio);
bool intercom_sync_serial(FuriHalSerialHandle* serial);
