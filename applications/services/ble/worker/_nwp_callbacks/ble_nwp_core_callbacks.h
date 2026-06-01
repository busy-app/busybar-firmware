#pragma once

#include "../event/ble_incoming_nwp_event_processor.h"

#include "ble_nwp_headers.h"

void ble_nwp_core_config_callbacks(
    BleIncomingNwpEventProcessor* instance,
    FuriSemaphore* transmit_sem,
    FuriSemaphore* indicate_sem);
