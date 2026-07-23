/**
 * @file ble_transmitter_set.h
 * @brief Implements sending data over ble using notification
 */
#pragma once

#include "ble_transmitter_i.h"

/**
 * @brief Allocate notification transmitter instance
 * @return pointer to instance as a generic transmitter
 */
BleTransmitterGeneric* ble_transmitter_set_alloc();

/**
 * @brief Free notification transmitter instance
 * @param[in] transport to instance as a generic transmitter
 */
void ble_transmitter_set_free(BleTransmitterGeneric* transport);

/**
 * @brief Enables sending data
 *
 * Otherwise all data put by ble_transmitter_set_chunk will be ignored
 * @param[in] transport to instance as a generic transmitter
 */
void ble_transmitter_set_enable(BleTransmitterGeneric* transport);

/**
 * @brief Send data as notification
 * @param[in] transport to instance as a generic transmitter
 * @param[in] dev_addr address of remote device to which data will be sent
 * @param[in] handle pointer to particular characteristic
 * @param[in] data_size size of payload to be sent
 * @param[in] data payload
 * @return true if data send operation was successful, otherwise false
 */
bool ble_transmitter_set_chunk(
    BleTransmitterGeneric* transport,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data);

/**
 * @brief Allows sending more notification data to nwp
 * @param[in] transport transmitter instance
 */
void ble_transmitter_set_more_data(BleTransmitterGeneric* transport);

/**
 * @brief Cleanup all pending data without actual send
 * @param[in] transport transmitter instance
 */
void ble_transmitter_set_reset(BleTransmitterGeneric* transport);

/**
 * @brief Subscribes transmitter queues for processing via event_loop.
 * @param[in] transport transmitter instance
 * @param[in] event_loop used for all events happening inside
 * @param[in] context context for further usage
 */
void ble_transmitter_set_subscribe(
    BleTransmitterGeneric* transport,
    FuriEventLoop* event_loop,
    void* context);

/**
 * @brief Unsubscribe transmitter queues from event loop on stop
 *
 * @param[in] transport transmitter instance
 * @param[in] event_loop which needs to be unsubscribed from any transmitter queues and primitives
 */
void ble_transmitter_set_unsubscribe(BleTransmitterGeneric* transport, FuriEventLoop* event_loop);
