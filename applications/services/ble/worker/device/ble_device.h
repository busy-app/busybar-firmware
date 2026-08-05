/**
 * @file ble_device.h
 * @brief Logic representing BSB as a Ble device
 */
#pragma once

#include "ble_connection.h"
#include "ble_security.h"
#include "../transport/ble_transmitter.h"
#include "../../service/ble_service.h"

#include <furi.h>

/** @brief Enumeration of device states */
typedef enum {
    BleDeviceStateIdle, /**< Initial state means that device is ready */
    BleDeviceStateAdvertising, /**< Device currently advertises itself over ble and waiting for connections*/
    BleDeviceStateConnected, /**< Device is connected */
    BleDeviceStateStopping, /**< Device performs stop sequence, when it will be done it will move back to BleDeviceStateIdle */
    BleDeviceStateForgetting, /**< Device performs forget pairing sequence */
    BleDeviceStateError, /**< Error happened, more info in logs */
} BleDeviceState;

/**
 * @brief Opaque BleDevice type declaration.
 */
typedef struct BleDevice BleDevice;

/**
 * @brief Create ble device instance
 * Only one instance is created at device startup
 * @param[in] transmitter used internally to control data transmission
 * @return pointer to device instance
 */
BleDevice* ble_device_alloc(BleTransmitter* transmitter);

/**
 * @brief Free device instance
 * @param[in] instance of device
 */
void ble_device_free(BleDevice* instance);

/**
 * @brief Performs service registration in device registry and on nwp side
 *
 * This operation performed once per service during initialization 
 * @param[in] instance of device
 * @param[in] service to be registered
 * @return true if registration is done
 */
bool ble_device_register_service(BleDevice* instance, BleServiceObject* service);

/**
 * @brief Gets context created during connection establishment for further use
 *
 * @param[in] instance of device
 * @return connection context pointer
 */
BleConnectionContext* ble_device_get_connection_context(BleDevice* instance);

/**
 * @brief Process connection event from nwp
 *
 * This function is called when nwp sends event that remote device was connected.
 * It creates connection context and stores parameters.
 * @param[in] instance of device
 * @param[in] type of connected device address 
 * @param[in] peer_addr connected device address 
 * @return true if connection created successfully
 */
bool ble_device_connection_open(
    BleDevice* instance,
    BleDeviceAddressType type,
    const uint8_t* peer_addr);

/**
 * @brief Process disconnection event from nwp
 *
 * This function is called when nwp sends event that remote device was disconnected.
 * It destroys connection context, flushes all pending data, because they are not needed
 * anymore and switch device state to idle or advertising depending on state
 * @param[in] instance of device
 * @return true if connection closed successfully
 */
bool ble_device_connection_close(BleDevice* instance);

/**
 * @brief Start updating parameters after connection was established
 *
 * Calls ble_connection_start_update_parameters under the hood
 * @param[in] instance of device
 * @param[in] event_loop instance, through which processing is done
 * @param[in] update_done_cb done callback will be triggered when update done
 * @param[in] ctx done callback context
 */
void ble_device_connection_update(
    BleDevice* instance,
    FuriEventLoop* event_loop,
    BleConnectionUpdateParametersDoneCallback update_done_cb,
    void* ctx);

/**
 * @brief Force disconnect from the remote
 *
 * @param[in] instance of device
 * @return true if success, otherwise false
 */
bool ble_device_disconnect(BleDevice* instance);

/**
 * @brief Check if device connected
 *
 * @param[in] instance of device
 * @return true if connected, otherwise false
 */
bool ble_device_is_connected(BleDevice* instance);

/**
 * @brief Get device state
 *
 * @param[in] instance of device
 * @return state of the device
 */
BleDeviceState ble_device_get_state(BleDevice* instance);

/**
 * @brief Set device name
 *
 * @param[in] instance of device
 * @param[in] name for the device
 */
void ble_device_set_name(BleDevice* instance, const char* name);

/**
 * @brief Starts device advertising
 *
 * @param[in] instance of device
 * @return true if success, otherwise false
 */
bool ble_device_start(BleDevice* instance);

/**
 * @brief Stop device advertising
 *
 * If device was connected, it will perform disconnect first
 *
 * @param[in] instance of device
 * @return true if success, otherwise false
 */
bool ble_device_stop(BleDevice* instance);

/**
 * @brief Save mtu value received from nwp and calculate max payload size
 *
 * @param[in] instance of device
 * @param[in] mtu value from nwp
 */
void ble_device_set_mtu(BleDevice* instance, uint16_t mtu);

//---------------------------------------------------------------------------
/**
 * @brief Process all write requests using service registry
 *
 * @param[in] instance of device
 * @param[in] remote_addr address of the remote sent this request
 * @param[in] handle handle of characteristic
 * @param[in] data_size size of data nwp wants to write into characteristic
 * @param[in] data data nwp wants to write into characteristic
 * @return true if characteristic was found and processing was done
 */
bool ble_device_process_write_request(
    BleDevice* instance,
    const uint8_t* remote_addr,
    const uint16_t handle,
    const size_t data_size,
    const void* data);

/**
 * @brief Process read requests using service registry
 *
 * @param[in] instance of device
 * @param[in] addr address of the remote sent this request
 * @param[in] type read request type from nwp, should be always 0
 * @param[in] handle handle of characteristic
 * @param[in] offset byte offset of requested data
 * @return true if characteristic was found and processing was done
 */
bool ble_device_process_read_request(
    BleDevice* instance,
    uint8_t* addr,
    uint8_t type,
    uint16_t handle,
    uint16_t offset);

/**
 * @brief Send confirmation that write request is done
 *
 * @param[in] instance device instance
 * @param[in] handle handle of characteristic
 * @param[in] cccd_value property flags for characteristic
 */
void ble_device_receive_confirm(BleDevice* instance, uint16_t handle, uint8_t cccd_value);

/**
 * @brief Send data over ble
 *
 * This function also splits huge chunks using max payload size if needed
 * @param[in] instance of device
 * @param[in] handle handle of characteristic
 * @param[in] data_size size of data nwp wants to write into characteristic
 * @param[in] data data nwp wants to write into characteristic
 * @param[in] cccd_value property flags for characteristic
 */
void ble_device_send_data(
    BleDevice* instance,
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value);

//----------------------------------------------------------------------------
//PAIRING SMP HANDLERS

/** 
 * @brief Get pointer to security instance
 * @param[in] instance of device
 * @return pointer to security instance
 */
BleSecurityData* ble_device_get_security_data(BleDevice* instance);

/**
 * @brief Check if BSB is paired or not
 * @param[in] instance of device
 * @return true if paired, otherwise false
 */
bool ble_device_is_paired(BleDevice* instance);

/**
 * @brief Send respose with pairing capabilities on event from nwp
 * @param[in] instance of device
 */
void ble_device_response_pairing_capabilities(BleDevice* instance);

/**
 * @brief Attempt to request pairing
 * Used in attempt to avoid some pairing corner cases when BSB has
 * pairing but other side does not
 * @param[in] instance of device
 */
void ble_device_request_pairing(BleDevice* instance);

/**
 * @brief Send pairing keys data as a respose in format required by nwp
 * @param[in] instance of device
 * @return true if success, otherwise false
 */
bool ble_device_send_encryption_response(BleDevice* instance);

/**
 * @brief Performs first pairing steps on event from nwp
 * @param[in] instance of device
 * @param[in] encryption_data from nwp
 */
void ble_device_handle_encryption_start(
    BleDevice* instance,
    rsi_bt_event_encryption_enabled_t* encryption_data);

/**
 * @brief Delete currently stored pairing data
 * @param[in] instance of device
 * @return true if success, otherwise false
 */
bool ble_device_forget_paired(BleDevice* instance);
