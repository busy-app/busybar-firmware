/**
 * @file ble_service_config_types.h
 * @brief Type definitions for describing services and characteristics
 * Each service must be described using this struct and added to service_config collection
 * under the corresponding index
 */
#pragma once

#include "../ble_log.h"
#include "../ble_callback_types.h"
#include "ble_service_index.h"

#if defined(BSB_MCU_SI917)
/**
 * @brief BLE union for both UUID types, used only on 917 side
 */
typedef union {
    uint16_t Char_UUID_16; /**< 16-bit UUID */
    uint8_t Char_UUID_128[16]; /**< 128-bit UUID */
} Char_UUID_t;
#endif

/**
 * @brief Type definition for init callback method.
 *
 * This method is called once during initialization process and
 * can be used in order to create necessary context for service logic
 * @param[in] instance pointer to BleServiceObject in a generic form
 * @return true when init is done, otherwise false
 */
typedef bool (*BleServiceInit)(void* instance);

/**
 * @brief Type definition for run callback method.
 *
 * This method is called every time when BleServiceCommandRun is enqueued for
 * the service. Run can be enqueued as a reaction to some logic event happened
 * inside of a service. For example: Battery service has got an event regarding 
 * battery charge level from Power subsystem.
 * @param[in] instance pointer to BleServiceObject in a generic form
 * @return true when run is done, otherwise false
 */
typedef bool (*BleServiceRun)(void* instance);

/**
 * @brief Characteristic configuration
 */
typedef struct {
    uint16_t intercom_index; /**< Index used to identify characteristic in intercom packets */
#if defined(BSB_MCU_SI917)
    Char_UUID_t uuid; /**< Ble UUID for characteristic */
    uint8_t uuid_size; /**< Size in bytes can be 2-bytes for short UUIDs or 16 for long UUIDS */
    uint8_t char_properties; /**< Defines properties read/write/indicate/notify 
                                  which affect on how characteristic is interpreted in BLE */
#endif
    uint8_t initial_data_size; /**< Set size of data buffer*/

    const char* name; /**< Name for identification and logging */
} BleCharacteristicConfig;

/**
 * @brief Service configuration
 */
typedef struct {
#if defined(BSB_MCU_SI917)
    Char_UUID_t uuid; /**< Ble UUID for service */
    uint8_t uuid_size; /**< Size in bytes can be 2-bytes for short UUIDs or 16 for long UUIDS */
#endif
    BleServiceIndex index; /**< Index used to identify service in intercom packets and api */
    uint8_t char_count; /**< Amount of characteristics inside this service */

    const BleCharacteristicConfig* char_configs; /**< Config per each characteristic */
    const char* name; /**< Name for identification and logging */

    BleServiceInit
        init; /**< Init callback, must return true in order to init service successfully */
    BleServiceRun run; /**< Run callback, must return true in order to run service successfully  */
} BleServiceConfig;
