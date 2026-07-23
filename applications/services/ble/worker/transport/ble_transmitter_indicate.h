/**
 * @file ble_transmitter_indicate.h
 * @brief Implements sending data over ble using indication
 */
#pragma once

#include "ble_transmitter_i.h"

/**
 * @brief Allocate indication transmitter instance
 * @return pointer to instance as a generic transmitter
 */
BleTransmitterGeneric* ble_transmitter_indicate_alloc();

/**
 * @brief Free indication transmitter instance
 * @param[in] transport to instance as a generic transmitter
 */
void ble_transmitter_indicate_free(BleTransmitterGeneric* transport);

/**
 * @brief Sends indication data using indication
 * @param[in] transport to instance as a generic transmitter
 * @param[in] dev_addr destination remote device address
 * @param[in] handle pointer to particular characteristic
 * @param[in] data_size size of payload to be send
 * @param[in] data payload
 */
bool ble_transmitter_indicate_chunk(
    BleTransmitterGeneric* transport,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data);

/**
 * @brief Unblocks thread waiting for indication data to be send
 * @param[in] transport transmitter instance
 */
void ble_transmitter_indicate_done(BleTransmitterGeneric* transport);
