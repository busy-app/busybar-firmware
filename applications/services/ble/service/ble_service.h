/**
 * @file ble_service.h
 * @brief Public api for ble services
 */
#pragma once

#include "../ble_log.h"
#include "../ble_intercom_types.h"
#include "ble_service_config_types.h"

#include <furi.h>

/**
 * @brief Opaque BleServiceObject type declaration.
 */
typedef struct BleServiceObject BleServiceObject;

/**
 * @brief Allocate service instance using predefined config.
 *  
 * @param[in] service_config predefined config which describes service and its internals
 * @param[in] message_queue queue which all services use for processing events
 * @param[in] intercom_ch intercom channel for sending data to other side
 * @returns pointer to service instance
 */
BleServiceObject* ble_service_alloc(
    const BleServiceConfig* service_config,
    FuriMessageQueue* message_queue,
    IntercomChannel* intercom_ch);

/**
 * @brief Process events related to service.
 *  
 * This method is called from message queue handler and it can be called
 * in three cases: 
 * 1. Internal event happened inside and service enqueued run in order to be able to process this event
 * 2. Incoming frame for this service was received from the other side
 * 3. Incoming frame was emulated in order to send command (init) to this service
 * 
 * @param[in] instance pointer to the BleServiceObject instance
 * @returns true when processing was fine, otherwise false
 */
bool ble_service_process(BleServiceObject* instance);

/**
 * @brief Process intercom frames addressed to the service from other side.
 *  
 * This method if called from intercom_rx callback which must be as fast as possible, 
 * therefore it only copies frame payload to internal service frame buffer, enqueues further
 * service processing and returns
 * @param[in] instance pointer to the BleServiceObject instance
 * @param[in] input_frame pointer to frame received from other side
 */
void ble_service_process_mailbox(
    BleServiceObject* instance,
    const BleIntercomFrameGeneric* input_frame);

/**
 * @brief Returns service status.
 *  
 * Used during init in order to check that all services initialized
 * correctly and are ready to run
 * @param[in] instance pointer to the BleServiceObject instance
 * @returns true when service is ready, otherwise false
 */
bool ble_service_is_ready(BleServiceObject* instance);

/**
 * @brief Returns service name.
 *  
 * @param[in] instance pointer to the BleServiceObject instance
 * @returns service name string pointer
 */
const char* ble_service_get_name(BleServiceObject* instance);

/**
 * @brief Returns error message.
 *  
 * Called when service processing ends up with an error, error message
 * is then printed to logs
 * @param[in] instance pointer to the BleServiceObject instance
 * @param[out] error pre-allocated error string buffer
 */
void ble_service_get_error(BleServiceObject* instance, FuriString* error);

/**
 * @brief Enqueue initialize command during startup in order to init service.
 *  
 * Init command is emulated using pre-filled input frame buffer which after
 * that will be processed ble_service_process and result in BleServiceCommandInit
 * execution
 * @param[in] instance pointer to the BleServiceObject instance
 */
void ble_service_enqueue_init(BleServiceObject* instance);

/**
 * @brief Enqueue run command in order to call service specific logic.
 *  
 * @param[in] instance pointer to the BleServiceObject instance
 */
void ble_service_enqueue_run(BleServiceObject* instance);

/**
 * @brief Enqueue deinit command.
 *  
 * This is used on unrecoverable error handling in order to 
 * shut down service with all internals. 
 * @param[in] instance pointer to the BleServiceObject instance
 */
void ble_service_deinit(BleServiceObject* instance);

/**
 * @brief Write data to service.
 *  
 * Calling this method stores data inside of selected characteristic
 * and enqueues service processing, which then will send data the other 
 * side
 * @param[in] instance pointer to the BleServiceObject instance
 * @param[in] index characteristic index where data should be placed
 * @param[in] data payload
 * @param[in] data_size payload size
 */
void ble_service_write_data(
    BleServiceObject* instance,
    uint8_t index,
    const void* data,
    const size_t data_size);

/**
 * @brief Register update callback for selected characteristic inside of a service.
 *  
 * @param[in] instance pointer to the BleServiceObject instance
 * @param[in] index characteristic index where data should be placed
 * @param[in] cb pointer to update callback function
 * @param[in] ctx update callback context
 */
void ble_service_register_update_callback(
    BleServiceObject* instance,
    uint16_t index,
    BleDataUpdatedCallback cb,
    void* ctx);

/**
 * @brief Register transmission done callback for selected characteristic inside of a service.
 *  
 * @param[in] instance pointer to the BleServiceObject instance
 * @param[in] index characteristic index where data should be placed
 * @param[in] cb pointer to transmission done callback function
 * @param[in] ctx transmission done callback context
 */
void ble_service_register_transmission_done_callback(
    BleServiceObject* instance,
    uint16_t index,
    BleDataTransmitDoneCallback cb,
    void* ctx);
