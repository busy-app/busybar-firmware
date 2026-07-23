/**
 * @file ble_service_registry.h
 * @brief Registry used for service registration
 */
#pragma once

#include "../../service/ble_service.h"
#include "../../service/ble_characteristic.h"

/**
 * @brief Opaque BleServiceRegistry type declaration.
 */
typedef struct BleServiceRegistry BleServiceRegistry;

/**
 * @brief Entry associated with each characteristic
 */
typedef struct {
    BleServiceObject* service; /**< Service which contains this characteristic */
    uint16_t char_index; /**< Characteristic index within service */
} BleServiceRegistryEntry;

/**
 * @brief Create registry at statup
 * @return pointer to registry instance
 */
BleServiceRegistry* ble_service_registry_alloc();

/**
 * @brief Free registry instance
 * @param[in] instance to registry
 */
void ble_service_registry_free(BleServiceRegistry* instance);

/**
 * @brief Register service and all inner characteristics in nwp and save it in dictionary
 * Dictionary uses handle got from nwp during registration as a key 
 * @param[in] instance to registry
 * @param[in] service inner ble service which needs to be registered
 * @return true when registration completed successfully
 */
bool ble_service_registry_add_service_entry(
    BleServiceRegistry* instance,
    BleServiceObject* service);

/**
 * @brief Get entry from registry using handle as a key
 * 
 * @param[in] instance to registry
 * @param[in] handle value used as a key to find entry
 * @return pointer to registered entry
 */
const BleServiceRegistryEntry*
    ble_service_registry_get_service_entry(BleServiceRegistry* instance, const uint16_t handle);

/**
 * @brief Reset all notification flags 
 *
 * These flags can be previously set by remote device during interaction,
 * so on disconnect all those flags should be reset to default
 * 
 * @param[in] instance to registry
 */
void ble_service_registry_reset_cccds(BleServiceRegistry* instance);
