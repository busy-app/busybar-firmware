/**
 * @file ble_connection.h
 * @brief Represents connection parameters between bsb and remote
 */
#pragma once

#include "ble_device_base.h"

/**
 * @brief Type definition for callback to be triggered when connection parameters updated
 * @param[in] ctx context for callback call
 */
typedef void (*BleConnectionUpdateParametersDoneCallback)(void* ctx);

/**
 * @brief Connection timings
 */
typedef struct {
    uint16_t interval;
    uint16_t latency;
    uint16_t timeout;
} BleConnectionTimings;

/**
 * @brief Connection data length
 */
typedef struct {
    uint16_t MaxTxOctets; /**< Maximum TX Octets to be transmitted*/
    uint16_t MaxTxTime; /**< Maximum TX time to transmit the MaxTxOctets*/
    uint16_t MaxRxOctets; /**< Maximum Rx Octets to be received*/
    uint16_t MaxRxTime; /**< Maximum Rx time to receive the MaxRxOctets*/
} BleConnectionDataLength;

/**
 * @brief Ble PHY representation
 */
typedef union {
    struct {
        bool phy_le_1m    : 1;
        bool phy_le_2m    : 1;
        bool phy_le_coded : 1;
        uint8_t reserved  : 5;
    } flags;
    uint8_t value;
} BlePhy;

/**
 * @brief Opaque BleConnectionContext type declaration.
 */
typedef struct BleConnectionContext BleConnectionContext;

/**
 * @brief Creates connection instance
 *
 * New connection instance is created by @ref "ble_device.h" everytime when nwp sends
 * a connection event
 * 
 * @param[in] type remote device address type
 * @param[in] peer_address remote device address
 * @return pointer to connection instance
 */
BleConnectionContext*
    ble_connection_alloc(BleDeviceAddressType type, const uint8_t* const peer_address);

/**
 * @brief Free connection instance when disconnect happened
 * @param[in] instance to connection to be deleted
 */
void ble_connection_free(BleConnectionContext* instance);

/**
 * @brief Get pointer to connected remote peer instance for future use
 * @param[in] instance to connection containing information about remote peer
 * @return Opaque instance to remote peer
 */
BleDeviceBase* ble_connection_get_peer(BleConnectionContext* instance);

/**
 * @brief Getter for accessing @ref BleConnectionTimings from the connection
 * @param[in] instance to connection
 * @return pointer to @ref BleConnectionTimings struct
 */
const BleConnectionTimings* ble_connection_get_timings(BleConnectionContext* instance);

/**
 * @brief Set connection timing data from outside
 * @param[in] instance to connection
 * @param[in] timings struct filled with data to set into the connection
 */
void ble_connection_set_timings(
    BleConnectionContext* instance,
    const BleConnectionTimings* const timings);

/**
 * @brief Getter for accessing @ref BleConnectionDataLength from the connection
 * @param[in] instance to connection
 * @return pointer to @ref BleConnectionDataLength struct
 */
const BleConnectionDataLength* ble_connection_get_data_length(BleConnectionContext* instance);

/**
 * @brief Set data length from outside
 * @param[in] instance to connection
 * @param[in] data_length struct filled with data to set into the connection
 */
void ble_connection_set_data_length(
    BleConnectionContext* instance,
    const BleConnectionDataLength* const data_length);

/**
 * @brief Get Tx PHY info from the connection
 * @param[in] instance to connection
 * @return pointer to @ref BlePhy struct
 */
const BlePhy* ble_connection_get_tx_phy(BleConnectionContext* instance);

/**
 * @brief Get Rx PHY info from the connection
 * @param[in] instance to connection
 * @return pointer to @ref BlePhy struct
 */
const BlePhy* ble_connection_get_rx_phy(BleConnectionContext* instance);

/**
 * @brief Set both Rx and Tx PHY infos to the connection
 * @param[in] instance to connection
 * @param[in] tx_phy value of Tx PHY
 * @param[in] rx_phy value of Rx PHY
 */
void ble_connection_set_phy(
    BleConnectionContext* instance,
    const uint8_t tx_phy,
    const uint8_t rx_phy);

/**
 * @brief Start process of updating connection parameters
 * 
 * This starts a series of request/responses to the remote device
 * in order to finalize connection parameters. When all operations
 * are completed a done callback will be called
 *
 * @param[in] instance to connection
 * @param[in] event_loop instance, through which processing is done
 * @param[in] done_cb callback which will be triggered once all parameters will be agreed
 * @param[in] ctx callback context
 */
void ble_connection_start_update_parameters(
    BleConnectionContext* instance,
    FuriEventLoop* event_loop,
    BleConnectionUpdateParametersDoneCallback done_cb,
    void* ctx);
