#include "ble_advertise.h"
#include "../ble_common.h"
#include "rsi_ble_apis.h"
#include "rsi_bt_common_apis.h"

#define TAG "BleAdvertise"

#define BLE_ADVERTISE_PACKET_MAX_SIZE (31)

typedef struct FURI_PACKED {
    uint8_t length;
    uint8_t type;
} BleAdvertiseHeader;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    uint8_t data;
} BleAdvertiseByteData;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    uint16_t data;
} BleAdvertiseWordData;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    char data[];
} BleAdvertiseLocalName;

typedef struct FURI_PACKED {
    BleAdvertiseHeader header;
    uint16_t data;
} BleAdvertiseServiceClassUUID;

typedef struct FURI_PACKED {
    BleAdvertiseByteData flags;
    BleAdvertiseWordData appearance;
    BleAdvertiseWordData manufacturer;
    BleAdvertiseServiceClassUUID service_class;
    BleAdvertiseLocalName local_name;
} BleAdvertiseConfig;

// extern const BleAdvertiseConfig advertise_config;

static const BleAdvertiseConfig advertise_config_template = {
    .flags =
        {
            .header = {.length = 2, .type = 1},
            .data = 6,
        },
    .appearance =
        {
            .header = {.type = 0x19, .length = 3},
            .data = 0x0880, //0x00C0,
        },
    .manufacturer =
        {
            .header = {.type = 0xFF, .length = 3},
            .data = 0x0E29,
        },
    .service_class =
        {
            .header = {.type = 0x02, .length = 3},
            .data = 0x308A,
        },
    .local_name =
        {
            .header = {.type = 0x9, .length = 0},
            // .data = BLE_LOCAL_NAME,
        },
};

static_assert(sizeof(advertise_config_template) <= BLE_ADVERTISE_PACKET_MAX_SIZE);
//==========================================================

struct BleAdvertiseContext {
    FuriMutex* lock;
    size_t actual_size;
    void* advertise_config;
};

BleAdvertiseContext* ble_advertise_alloc() {
    BleAdvertiseContext* instance = malloc(sizeof(BleAdvertiseContext));

    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->actual_size = sizeof(advertise_config_template);
    instance->advertise_config = malloc(BLE_ADVERTISE_PACKET_MAX_SIZE);
    memcpy(
        instance->advertise_config, &advertise_config_template, sizeof(advertise_config_template));
    return instance;
}

void ble_advertise_free(BleAdvertiseContext* instance) {
    furi_assert(instance);
    furi_mutex_free(instance->lock);
    free(instance->advertise_config);
    free(instance);
}

void ble_advertise_set_name(BleAdvertiseContext* instance, const char* new_name) {
    furi_assert(instance);
    furi_assert(new_name);

    furi_mutex_acquire(instance->lock, FuriWaitForever);

    FuriString* name = furi_string_alloc_set_str(new_name);
    const size_t free_space = BLE_ADVERTISE_PACKET_MAX_SIZE - sizeof(advertise_config_template);

    if(furi_string_size(name) > free_space) {
        furi_string_left(name, free_space - 3);
        BLE_LOG_I("Trimmed name: %s", furi_string_get_cstr(name));
        furi_string_cat_printf(name, "...");
    }

    BLE_LOG_I("New device name: %s", furi_string_get_cstr(name));
    const size_t name_size = furi_string_size(name);

    BleAdvertiseConfig* const config = instance->advertise_config;
    config->local_name.header.length = name_size + 1;
    memset(config->local_name.data, 0, free_space);
    memcpy(config->local_name.data, furi_string_get_cstr(name), name_size);

    instance->actual_size = sizeof(advertise_config_template) + name_size;
    furi_string_free(name);

    furi_mutex_release(instance->lock);
}

void ble_advertise_refresh_data(const BleAdvertiseContext* instance) {
    furi_assert(instance);

    furi_mutex_acquire(instance->lock, FuriWaitForever);

    BLE_LOG_I("Size: %d", instance->actual_size);
    if(rsi_ble_set_advertise_data(instance->advertise_config, instance->actual_size) !=
       RSI_SUCCESS)
        BLE_LOG_W("Failed to set advertise data");

    furi_mutex_release(instance->lock);
}

void ble_advertise_print_data(const BleAdvertiseContext* instance) {
    furi_assert(instance);

    furi_mutex_acquire(instance->lock, FuriWaitForever);

    const BleAdvertiseConfig* const config = instance->advertise_config;

    BLE_LOG_I("Flags: %d", config->flags.data);
    BLE_LOG_I("Appearance: %04X", config->appearance.data);
    BLE_LOG_I("Manufacturer: %04X", config->manufacturer.data);
    if(config->local_name.header.length > 0) {
        BLE_LOG_I(
            "Local name: %s, size: %d", config->local_name.data, config->local_name.header.length);
    } else
        BLE_LOG_I("Free name space %d", BLE_ADVERTISE_PACKET_MAX_SIZE - instance->actual_size);

    furi_mutex_release(instance->lock);
}
