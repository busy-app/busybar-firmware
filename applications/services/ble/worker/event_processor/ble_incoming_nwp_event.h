/**
 * @file ble_incoming_nwp_event.h
 * @brief Logic of creation of incoming nwp events used by @ref "ble_incoming_nwp_event_processor.h"
 * when spawning new event on nwp callback calls
 */
#pragma once

#include "ble_incoming_nwp_event_type_enum.h"

#include <furi.h>

typedef struct {
    BleIncomingNwpEventType type;
    size_t data_size;
    void* data;
} BleIncomingNwpEvent;

/**
 * @brief Allocates incoming event with mentioned parameters
 * @param[in] type one of possible types, described in @ref "ble_incoming_nwp_event_type_enum.h"
 * @param[in] data_size size of incoming data received from nwp
 * @param[in] data payload received from nwp
 * @return pointer to event for future use
 */
BleIncomingNwpEvent*
    ble_incoming_nwp_event_alloc(BleIncomingNwpEventType type, size_t data_size, void* data);

/**
 * @brief Free event
 * @param[in] instance pointer to event instance to be freed
 */
void ble_incoming_nwp_event_free(BleIncomingNwpEvent* instance);
