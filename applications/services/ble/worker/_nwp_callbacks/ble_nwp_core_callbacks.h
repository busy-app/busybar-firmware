#pragma once

#include "../event_processor/ble_incoming_nwp_event_processor.h"
#include "../transport/ble_transmitter.h"

#include "ble_nwp_headers.h"

void ble_nwp_core_config_callbacks(
    BleIncomingNwpEventProcessor* event_processor_instance,
    BleTransmitter* transport_instance);
