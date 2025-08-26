#pragma once

#include "../ble_common.h"
#include "ble_service_index.h"

#if defined(SI917)
typedef union {
    /**
   * 16-bit UUID
   */
    uint16_t Char_UUID_16;
    /**
   * 128-bit UUID
   */
    uint8_t Char_UUID_128[16];
} Char_UUID_t;
#endif

typedef enum {
    BleServiceInitMethodLocal,
    BleServiceInitMethodRemote,
} BleServiceInitMethod;

typedef bool (*BleServiceInit)(void* instance);
typedef void (*BleServiceRead)(void* data, uint8_t data_size);
typedef void (*BleServiceWrite)(void* data, uint8_t data_size);
typedef void (*BleServiceNotify)(void* data, uint8_t data_size);

// typedef const uint8_t* (*BleCharacterGetData)(void* instance);

typedef struct {
    uint16_t intercom_index;
#if defined(SI917)
    Char_UUID_t uuid;
    uint8_t uuid_size;
    uint8_t char_properties;
#endif
    uint8_t initial_data_size;
    uint8_t security_permissions;
    const char* name;
    // BleCharacterGetData get_data;
} BleCharacteristicDescriptor;

typedef struct {
#if defined(SI917)
    Char_UUID_t uuid;
    uint8_t uuid_size;
#endif

    BleServiceIndex index;
    uint8_t char_count;
    BleServiceInitMethod init_method;
    const BleCharacteristicDescriptor* char_descriptors;
    const char* name;

    BleServiceInit init;
    BleServiceRead read;
    BleServiceWrite write;
    BleServiceNotify notify;
} BleServiceDescriptor;
