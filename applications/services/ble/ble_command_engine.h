/**
 * @file ble_command_engine.h
 * @brief System command engine
 */
#pragma once

#include "ble_intercom_types.h"
#include "ble_system_command.h"

/**
 * @brief Opaque command engine handle
 */
typedef struct BleCommandEngine BleCommandEngine;

/**
 * @brief Allocates command engine instance with command array provided from the outside
 *
 * @param[in] ble Pointer to ble instance
 * @param[in] commands Predefined command array
 * @param[in] commands_count Amount of commands in array
 * @param[in] event_loop Event loop used for queue processing
 * @return Pointer to command engine instance
 */
BleCommandEngine* ble_command_engine_alloc(
    Ble* ble,
    const BleCommandItem* commands,
    uint8_t commands_count,
    FuriEventLoop* event_loop);

/**
 * @brief Perform command processing, command frame can be extracted from command 
 * buffer or from intercom for U5 and only from intercom for 917
 *
 * @param[in] instance Pointer to engine instance
 * @param[in] frame frame with data to be processed
 * @return true when command was processed successfully, otherwise false
 */
bool ble_command_engine_run(BleCommandEngine* instance, BleIntercomFrameGeneric* frame);

/**
 * @brief Put command into queue and wait until it is processed by ble thread.
 *
 * Data and data_size fields are used as input/output fields depending on command. 
 * In case when command requires input data, they must be provided in data, and size must be data_size.
 * In case when command returns data, a buffer of appropriate size must be provided, then it will be filled
 * with data.
 * Currently there is no any commands which require both input and output data, in such case this function
 * will return output data only if its size is equal to data_size provided by caller, otherwise it will return false, 
 * despite the actual command result.
 * 
 * @param[in] instance Pointer to engine instance
 * @param[in] code Command code from enum
 * @param data Data required for command processing, must be NULL if command doesn't require input data and doesn't return any data
 * @param data_size Data size, must be 0 if command doesn't require input data and doesn't return any data
 * @return true if command processed successfully, otherwise false
 */
bool ble_command_engine_put_command(
    BleCommandEngine* instance,
    BleSystemCommand code,
    void* data,
    size_t data_size);

/**
 * @brief Put command into queue and return.
 *
 * There are some commands (BleCommandInit, BleCommandSetStatus for example) which are not exposed to api. 
 * They perform internally as a reaction to some events or state change.
 * This function enqueues those commands and they will be performed non-blocking way.
 * 
 * @param[in] instance Pointer to engine instance
 * @param[in] code Command code from enum
 * @param data Data required for command processing, must be NULL if command doesn't require input data and doesn't return any data
 * @param data_size Data size, must be 0 if command doesn't require input data and doesn't return any data
 */
void ble_command_engine_put_command_no_wait(
    BleCommandEngine* instance,
    BleSystemCommand code,
    void* data,
    size_t data_size);

/**
 * @brief Unblocks command engine when command is done
 *
 * @param[in] instance pointer to the ble instance
 * @param[in] data data which command can provide
 * @param[in] data_size size of the data buffer
 * @param[in] result result of command processing, which will be transferred to public api
 * call and become its return value
 */
void ble_command_engine_unblock_with_result(
    BleCommandEngine* instance,
    const void* const data,
    const size_t data_size,
    bool result);
