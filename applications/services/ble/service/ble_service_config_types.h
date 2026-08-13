#pragma once

#include "../ble_log.h"
#include "../ble_callback_types.h"
#include "ble_service_index.h"

#if defined(BSB_MCU_SI917)
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
typedef bool (*BleServiceRun)(void* instance);

typedef struct {
    uint16_t intercom_index;
#if defined(BSB_MCU_SI917)
    Char_UUID_t uuid;
    uint8_t uuid_size;
    uint8_t char_properties;
#endif
    uint8_t initial_data_size;
    uint8_t security_permissions;
    const char* name;
} BleCharacteristicDescriptor;

typedef struct {
#if defined(BSB_MCU_SI917)
    Char_UUID_t uuid;
    uint8_t uuid_size;
#endif
    BleServiceIndex index;
    uint8_t char_count;
    BleServiceInitMethod init_method;
    const BleCharacteristicDescriptor* char_descriptors;
    const char* name;

    BleServiceInit init;
    BleServiceRun run;
} BleServiceDescriptor;
