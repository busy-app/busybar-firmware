/**
 * @file ble_event_handlers.h
 * @brief Declarations of handlers for all possible events from nwp
 */
#pragma once

#include <furi.h>

/** @name Commands
 * Commands are sent internally in order to change state. Currently only
 * one command is needed
 */
/**@{*/
/**
 * @brief Stops device instance and event loop responsible for processing
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_cmd_exit(size_t data_size, void* data, void* context);

/**
 * @brief Forget paired device command initiated by ble_worker_forget_pairing
 * It can also be initiated a second time from the disconnect event to guarantee
 * that the device forgets its pairing only after disconnecting from the remote
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_cmd_forget_paired(size_t data_size, void* data, void* context);

/**@}*/

/** @name GAP handlers */
/**@{*/
/**
 * @brief Handles disconnection event from nwp
 *
 * On disconnect device instance closes connection also if exit command was initiated
 * previously then it will be initiated again in order to stop only after disconnect 
 * actually happened
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return true on success, otherwise false
 */
bool ble_event_handler_gap_disconnected(size_t data_size, void* data, void* context);

/**
 * @brief Handles connection event from nwp
 *
 * Makes device to instance create a new connection instance and store parameters
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return true on success, otherwise false
 */
bool ble_event_handler_gap_connected(size_t data_size, void* data, void* context);

/**
 * @brief Handles phy event from nwp
 *
 * Nwp returns Phy parameters of connection the connection agreed between both sides
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_gap_phy_update_complete(size_t data_size, void* data, void* context);

/**
 * @brief Handles update connection event from nwp
 *
 * Nwp returns connection parameters which are then stored into connection
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_gap_connection_update(size_t data_size, void* data, void* context);

/**
 * @brief Handles length response event from nwp
 *
 * Nwp returns data length set for current connection
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_gap_length_change(size_t data_size, void* data, void* context);

/**
 * @brief Handles remote features response event from nwp
 *
 * Nwp returns array of supported features of the remote peer, which 
 * then will be stored into peer instance inside the connection
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_gap_receive_remote_features(size_t data_size, void* data, void* context);
/**@}*/

/** @name GATT handlers */
/**@{*/
/**
 * @brief Handles mtu response event from nwp
 *
 * Nwp returns mtu, which was agreed between both peers
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_gatt_mtu(size_t data_size, void* data, void* context);

/**
 * @brief Handles write requests from nwp
 *
 * Nwp returns several type of ble events, but currently only write request event is used. 
 * Each request has a handle to particular characteristic where data should be written
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return true if write request was processed, otherwise false
 */
bool ble_event_handler_gatt_write_event(size_t data_size, void* data, void* context);

/**
 * @brief Handles read requests from nwp
 *
 * Nwp requests data from characteristic marked by handle
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return true if read request was processed, otherwise false
 */
bool ble_event_handler_gatt_read_request_event(size_t data_size, void* data, void* context);
/**@}*/

/** @name SMP handlers */
/**@{*/
/**
 * @brief Handles SMP pairing request by sending capability response
 *
 * When BSB has no pairing, a response will be sent, otherwise if 
 * some remote peer attempts to pair with BSB which already has pairing,
 * then BSB will disconnect from such remote peer, and continue to do it
 * until pairing on BSB will not be removed.
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_smp_response(size_t data_size, void* data, void* context);

/**
 * @brief Handles SMP encryption started event from NWP
 *
 * This event indicated, that pairing process with remote was finished successfully
 * and also it returns pairing keys, which need to be stored in order to keep pairing
 * after device reboot
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_smp_encrypt_started(size_t data_size, void* data, void* context);

/**
 * @brief SMP request for restoring pairing with previously paired peer
 *
 * NWP requires a response with proper pairing keys for connected peer.
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_smp_ltk_request(size_t data_size, void* data, void* context);

/**
 * @brief SMP response with IRK key used for RPA
 *
 * NWP returns an instance of keys for remote device, which
 * after that can be used in order to resolve private random address (RPA)
 * into original device address. This function provide those keys to security
 * module for future saving
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return always true
 */
bool ble_event_handler_smp_security_keys(size_t data_size, void* data, void* context);

/**
 * @brief SMP response when pairing process was failed
 *
 * Fail is considered when user press 'Cancel' on phone during
 * pairing instead of 'Ok', then pairing process is terminated
 * and nwp spawns such event. BSB from its side will disconnect
 *
 * @param[in] data_size payload size
 * @param[in] data payload data
 * @param[in] context used for call
 * @return true if disconnect operation was initiated successfully, otherwise false
 */
bool ble_event_handler_smp_pairing_failed(size_t data_size, void* data, void* context);
/**@}*/
