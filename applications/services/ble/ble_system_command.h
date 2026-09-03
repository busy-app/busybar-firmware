/**
 * @file ble_system_command.h
 * @brief Available ble commands definition and common methods
 */
#pragma once

#include "ble.h"
#include "ble_intercom_types.h"

/** 
* @brief Enumeration of possible ble commands. 
*
* Some commands are used internally, others by public api. In general almost all commands are processed
* as request/response pair. If one side sends command as a request then other side performs command and
* sends back a response with a result.
*/
typedef enum {
    BleCommandUnknown,
    BleCommandInit, /**< Internal command used to init ble at device startup, sends automatically when all requirements are met*/
    BleCommandDeinit, /**< Internal command used to deinit ble if some serious failures occurred*/
    BleCommandEnable, /**< Enable ble. After this command device becomes visible for all scanning devices or for paired one only. Used in @ref ble_start*/
    BleCommandDisable, /**< Disable ble. Used by @ref ble_stop*/
    BleCommandGetStatus, /**< Get ble status. Used by @ref ble_get_state*/
    BleCommandSetStatus, /**< Internal command, used to update status from SiWG917 to U5*/
    BleCommandForgetPairing, /**< Forget currently paired remote device. Used by @ref ble_forget*/

    BleCommandCount /**< Commands count*/
} BleSystemCommand;

/**
 * @brief Command handler callback function type
 *
 * @param[in] frame Pointer to frame
 * @param[in] context Execution context
 * @return true when handling was fine, otherwise false
 */
typedef bool (*BleCommandHandler)(BleIntercomFrameGeneric* frame, void* context);

/**
 * @brief Struct for one command handlers
 */
typedef struct {
    BleCommandHandler request;
    BleCommandHandler response;
} BleCommandItem;

/**
 * @brief Send frame as a request to another side
 *
 * @param[in] frame pointer to frame to be sent
 * @param[in] context for frame processing
 * @returns true if send operation was success, otherwise false
 */
bool ble_command_request_process(BleIntercomFrameGeneric* frame, void* context);

/**
 * @brief Send frame as a response to another side
 *
 * @param[in] frame pointer to frame to be sent
 * @param[in] context for frame processing
 * @returns true if send operation was successful, otherwise false
 */
bool ble_command_response_process(BleIntercomFrameGeneric* frame, void* context);

/**
 * @brief Deinit all ble services in case of an error
 *
 * This function contains common logic for BleCommandDeinit shared by both sides 
 * and is called when some serious error happened and there is a need to shut down
 * all internal ble services.
 * 
 * @param[in] frame pointer to frame to be sent
 * @param[in] context for frame processing
 * @returns always false
 */
bool ble_command_deinit_process(BleIntercomFrameGeneric* frame, void* context);

/**
 * @brief Array with command handlers. 
 *
 * Each side has its own command handlers implementation which are placed in ble_system_command_u5.c
 * for U5 and and ble_system_command_917.c for 917. This array is used for command engine initialization
 */
extern const BleCommandItem ble_commands[];
