#pragma once

#include "event/ble_incoming_nwp_event_processor.h"

#include "ble_config.h"
#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

void ble_nwp_core_config_callbacks(BleIncomingNwpEventProcessor* instance);
