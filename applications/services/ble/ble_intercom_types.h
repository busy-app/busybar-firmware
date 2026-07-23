/**
 * @file ble_intercom_types.h
 * @brief Typedefs used for data exchange via intercom between ble services on both chips
 */
#pragma once

#include <intercom/intercom.h>
#include <intercom/intercom_frame.h>
#include <furi.h>

/**
 * @brief Maximum intercom timeout, exceeding this timeout results in error
 */
#define BLE_INTERCOM_TX_TIMEOUT_MS (1000)

/** Enumeration of intercom frame sources. Allows to differ ble commands data from ble services data. */
typedef enum {
    BleIntercomFrameSourceUnknown, /**< Stub in order to avoid empty frames with zeros from becoming valid*/
    BleIntercomFrameSourceSystem, /**< This frame contains ble command data and will be processed by ble*/
    BleIntercomFrameSourceService, /**< This frame contains ble service data and will be provided to it for further processing*/
} BleIntercomFrameSource;

/** Enumeration of intercom frame types. There are to frame types, request and response */
typedef enum {
    BleIntercomFrameTypeUnknown, /**< Stub in order to avoid empty frames with zeros from becoming valid*/
    BleIntercomFrameTypeRequest, /**< Request frame, which contains command or data to be processed by this side */
    BleIntercomFrameTypeResponse, /**< Response frame contains the result from other side */
} BleIntercomFrameType;

/**
 * @brief Command code type, which allows placing codes from BleSystemCommand and BleServiceCommandEnum
 */
typedef uint8_t BleCommandCode;

/**
 * @brief Frame header with fields which help control frame exchange
 *
 * Contains all data required to send packets via intercom
 */
typedef struct FURI_PACKED {
    bool result; /**< Returns the result of the operation which was initiated by request frame. 
    * Usually is checked in response processing. Also this result is transferred as a result to 
    * public api function, which returns it as a result of performed operation
    */

    BleCommandCode command; /**< Command code for entire ble or internal service */
    uint16_t service_index; /**< Internal service index, used only for service frame type */
    uint32_t num; /**< Frame sequence number for consistency checks */
    BleIntercomFrameSource source; /**< Frame source */
    BleIntercomFrameType frame_type; /**< Frame type */
    uint32_t data_size; /**< Size of payload data in frame after header */
} BleIntercomFrameHeader;

/**
 * @brief Maximum possible ble payload size per one frame. Is less than maximum intercom frame by size of @ref BleIntercomFrameHeader
 */
#define MAX_BLE_INTERCOM_FRAME_SIZE (INTERCOM_FRAME_DATA_SIZE - sizeof(BleIntercomFrameHeader))

/**
 * @brief Frame transmitted over ble from one side to another
 */
typedef struct FURI_PACKED {
    BleIntercomFrameHeader header; /**< Frame header of type @ref BleIntercomFrameHeader */
    uint8_t data[MAX_BLE_INTERCOM_FRAME_SIZE]; /**< Frame payload, can contain command data or 
        * some further structs related to internal services and characteristics 
        */
} BleIntercomFrameGeneric;

/**
 * @brief Header for inner service specific frames.
 */
typedef struct FURI_PACKED {
    uint16_t index; /**< Characteristic index */
    uint16_t data_size; /**< Payload data size */
    BleIntercomFrameType frame_type; /**< Frame type */
    uint32_t seq_num; /**< Characteristic sequence number */
} BleCharacteristicDataHeader;

/**
 * @brief Used for service characteristics data transmissions.
 */
typedef struct FURI_PACKED {
    BleCharacteristicDataHeader header; /**< Characteristic frame header */
    uint8_t data[]; /**< Characteristic data */
} BleCharacteristicData;

/**
 * @brief Amount of characteristics in one @ref BleIntercomServiceData frame.
 */
typedef uint32_t BleCharacteristicCountType;

/**
 * @brief Inner service data frame type
 *
 * This struct is send as a data in @ref BleIntercomFrameGeneric
 */
typedef struct FURI_PACKED {
    BleCharacteristicCountType char_count; /**< Amount of characteristics per frame */
    BleCharacteristicData chars_config[]; /**< Array of @ref BleCharacteristicData frames */
} BleIntercomServiceData;

//=============================================
