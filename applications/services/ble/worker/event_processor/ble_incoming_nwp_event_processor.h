/**
 * @file ble_incoming_nwp_event_processor.h
 * @brief Spawns events on nwp callbacks and processes them in queue
 */
#pragma once

#include "ble_incoming_nwp_event_type_enum.h"
#include <furi.h>

/**
 * @brief Opaque BleIncomingNwpEventProcessor type declaration.
 */
typedef struct BleIncomingNwpEventProcessor BleIncomingNwpEventProcessor;

/**
 * @brief Allocates processor instance during ble initialization
 * @param[in] context context for future use in spawned events
 * @return pointer to event processor instance
 */
BleIncomingNwpEventProcessor* ble_incoming_nwp_event_processor_alloc(void* context);

/**
 * @brief Subscribes inner queue to event_loop.
 *
 * This is called when worker thread spawns and creates event_loop
 * 
 * @param[in] instance processor instance
 * @param[in] event_loop used for all events happening inside
 */
void ble_incoming_nwp_event_processor_subscribe(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop);

/**
 * @brief Unsubscribe queue from event loop on stop
 *
 * @param[in] instance processor instance
 * @param[in] event_loop which needs to be unsubscribed
 */
void ble_incoming_nwp_event_processor_unsubscribe(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop);

/**
 * @brief Spawns event of desired type and puts it into queue for further processing
 *
 * @param[in] instance processor instance
 * @param[in] type event type from @ref "ble_incoming_nwp_event_type_enum.h"
 * @param[in] data_size size of incoming data received from nwp
 * @param[in] data payload received from nwp
 */
void ble_incoming_nwp_event_processor_spawn_event(
    BleIncomingNwpEventProcessor* instance,
    BleIncomingNwpEventType type,
    size_t data_size,
    void* data);
