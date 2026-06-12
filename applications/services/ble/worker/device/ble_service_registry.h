#pragma once

#include "../../service/ble_service.h"
#include "../../service/ble_characteristic.h"

typedef struct BleServiceRegistry BleServiceRegistry;

typedef struct {
    BleServiceObject* service;
    uint16_t char_index;
} BleServiceRegistryEntry;

BleServiceRegistry* ble_service_registry_alloc();
void ble_service_registry_free(BleServiceRegistry* instance);

bool ble_service_registry_add_service_entry(
    BleServiceRegistry* instance,
    BleServiceObject* service);

const BleServiceRegistryEntry*
    ble_service_registry_get_service_entry(BleServiceRegistry* instance, const uint16_t handle);

void ble_service_registry_reset_cccds(BleServiceRegistry* instance);
