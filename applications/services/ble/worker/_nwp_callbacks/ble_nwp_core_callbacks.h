/**
 * @file ble_nwp_core_callbacks.h
 * @brief Registers proper callback handlers for nwp.
 *
 * Translate nwp callback calls into events for further processing by @ref ble_incoming_nwp_event_processor.h
 * This module doesn't require instance allocation logic, and exists as a singleton
 */
#pragma once

#include "../event_processor/ble_incoming_nwp_event_processor.h"
#include "../transport/ble_transmitter.h"

#include "ble_nwp_headers.h"

/**
 * @brief Configures callbacks for further interaction with event processor and transport.
 *
 * Called once during initialization
 * 
 * @param[in] event_processor_instance instance of event processor where to forward events from callbacks
 * @param[in] transport_instance transport instance, required for transmission flow control
 */
void ble_nwp_core_config_callbacks(
    BleIncomingNwpEventProcessor* event_processor_instance,
    BleTransmitter* transport_instance);
