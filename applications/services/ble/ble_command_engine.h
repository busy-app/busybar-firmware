#pragma once

#include "ble_intercom_types.h"
#include "ble.h"

/**
 * @brief Enumeration of possible frame source
 */
typedef enum {
    BleCommandEngineExtractFrameSourceCommandBuffer, /**< Extract frame from command buffer */
    BleCommandEngineExtractFrameSourceIntercomBuffer, /**< Extract frame from intercom buffer */
} BleCommandEngineExtractFrameSource;

/**
 * @brief Extract frame callback function type
 *
 * @param[in] instance Pointer to ble instance
 * @param[in] source Source from where frame should be taken
 * @param[out] BleIntercomFrameGeneric Pointer to frame struct
 */
typedef BleIntercomFrameGeneric* (
    *BleCommandEngineExtractFrame)(Ble* instance, BleCommandEngineExtractFrameSource source);

/**
 * @brief Command handler callback function type
 *
 * @param[in] frame Pointer to frame
 * @param[in] context Execution context
 * @param[out] true when handling was fine, otherwise false
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
 * @brief Opaque command engine handle
 */
typedef struct BleCommandEngine BleCommandEngine;

/**
 * @brief Allocates command engine instance with command array provided from the outside
 *
 * @param[in] ble Pointer to ble instance
 * @param[in] commands Predefined command array
 * @param[in] commands_count Amount of commands in array
 * @param[in] extract_frame Frame extraction callback, separate for each chip
 * @param[out] BleCommandEngine* Pointer to command engine instance
 */
BleCommandEngine* ble_command_engine_alloc(
    Ble* ble,
    const BleCommandItem* commands,
    uint8_t commands_count,
    BleCommandEngineExtractFrame extract_frame);

/**
 * @brief Perform command processing, command frame can be extracted from command 
 * buffer or from intercom for U5 and only from intercom for 917
 *
 * @param[in] instance Pointer to engine instance
 * @param[in] source Source from where frame should be taken
 * @param[out] true when command was processed successfully, otherwise false
 */
bool ble_command_engine_run(BleCommandEngine* instance, BleCommandEngineExtractFrameSource source);
