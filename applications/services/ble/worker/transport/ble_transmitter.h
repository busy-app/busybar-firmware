/**
 * @file ble_transmitter.h
 * @brief Responsible for sending data over ble
 */
#pragma once

#include <furi.h>

/**
 * @brief Opaque BleTransmitter type declaration.
 */
typedef struct BleTransmitter BleTransmitter;

/**
 * @brief Allocate transmitter instance
 * @return pointer to created transmitter instance
 */
BleTransmitter* ble_transmitter_alloc();

/**
 * @brief Free transmitter instance
 * @param[in] instance transmitter instance to be destroyed
 */
void ble_transmitter_free(BleTransmitter* instance);

/**
 * @brief Enables sending data for characteristic which are notifiable
 *
 * This function is called when all ble connection update procedures are done,
 * before that any data send over notification will be ignored
 *
 * @param[in] instance transmitter instance
 */
void ble_transmitter_enable_notifications(BleTransmitter* instance);

/**
 * @brief Send data over ble using indication or notification methods
 * 
 * Sending method is choosen under the hood using cccd_value.
 * @param[in] instance transmitter instance
 * @param[in] dev_addr address of remote device to which data will be sent
 * @param[in] handle pointer to particular characteristic
 * @param[in] data_size size of payload to be send
 * @param[in] data payload
 * @param[in] cccd_value special field with properties which each characteristic in ble has
 * @return true if data send was successful, otherwise false
 */
bool ble_transmitter_send_chunk(
    BleTransmitter* instance,
    const uint8_t* dev_addr,
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value);

/**
 * @brief Cleanup all pending data without actual send
 * @param[in] instance transmitter instance
 */
void ble_transmitter_reset(BleTransmitter* instance);

/**
 * @brief Unblocks thread waiting for indication data to be send
 * 
 * Used when rsi_ble_gatt_on_event_indicate_confirmation_event is triggered by nwp
 * @param[in] instance transmitter instance
 */
void ble_transmitter_indication_done(BleTransmitter* instance);

/**
 * @brief Allows sending more notification data to nwp
 *
 * Used when rsi_ble_gap_ext_on_le_more_data_request_event is triggered by nwp
 * @param[in] instance transmitter instance
 */
void ble_transmitter_need_more_data(BleTransmitter* instance);

/**
 * @brief Subscribes inner transmitter queues for processing via event_loop.
 *
 * This is called when worker thread spawns and creates event_loop
 * 
 * @param[in] instance transmitter instance
 * @param[in] event_loop used for all events happening inside
 * @param[in] context context for further usage
 */
void ble_transmitter_subscribe(BleTransmitter* instance, FuriEventLoop* event_loop, void* context);

/**
 * @brief Unsubscribe transmitter queues from event loop on stop
 *
 * @param[in] instance transmitter instance
 * @param[in] event_loop which needs to be unsubscribed from any transmitter queues and primitives
 */
void ble_transmitter_unsubscribe(BleTransmitter* instance, FuriEventLoop* event_loop);
