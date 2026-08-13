#include "ble_advertise.h"
#include "../../ble_log.h"
#include "rsi_ble_apis.h"
#include "rsi_bt_common_apis.h"

#define TAG "BleAdvertise"

#define BLE_ADVERTISE_PACKET_MAX_SIZE      (31U)
#define BLE_ADVERTISE_DATA_TYPE_LOCAL_NAME (0x09)

#define BLE_ADVERTISE_HANDLE                     0x00
#define BLE_ADVERTISE_FILTER_POLICY_ALLOW_ALL    0x00
#define BLE_ADVERTISE_FILTER_POLICY_ALLOW_PAIRED 0x03
#define BLE_ADVERTISE_TX_PWR                     0x7f
#define BLE_ADVERTISE_PRIMARY_PHY                0x01
#define BLE_ADVERTISE_SECONDARY_PHY              0x01
#define BLE_ADVERTISE_SECONDARY_MAX_SKIP         0x00
#define BLE_ADVERTISE_SID                        0x00
#define BLE_ADVERTISE_INTERVAL_MIN               0x100
#define BLE_ADVERTISE_INTERVAL_MAX               0x200
#define BLE_ADVERTISE_CHANNEL_MAP                0x07
#define BLE_SCAN_REQUEST_NOTIFICATION_DISABLE    0x00

#define BLE_ADVERTISE_CONNECTABLE        (1 << 0)
#define BLE_ADVERTISE_SCANNABLE          (1 << 1)
#define BLE_ADVERTISE_LOW_DUTY_DIR_CONN  (0 << 2)
#define BLE_ADVERTISE_HIGH_DUTY_DIR_CONN (0 << 3)
#define BLE_ADVERTISE_LEGACY             (1 << 4)
#define BLE_ADVERTISE_ANONYMOUS          (0 << 5)
#define BLE_ADVERTISE_TX_WR_PWR          (0 << 6)

#define BLE_ADVERTISE_CONNECTABLE_SCANNABLE_EVENT_PROP                                       \
    (BLE_ADVERTISE_CONNECTABLE | BLE_ADVERTISE_SCANNABLE | BLE_ADVERTISE_LOW_DUTY_DIR_CONN | \
     BLE_ADVERTISE_HIGH_DUTY_DIR_CONN | BLE_ADVERTISE_LEGACY | BLE_ADVERTISE_ANONYMOUS |     \
     BLE_ADVERTISE_TX_WR_PWR)

#define BLE_ADVERTISE_DATA_OPERATION_COMPLETE        0x03
#define BLE_ADVERTISE_DATA_FRAGMENTATION_NOT_ALLOWED 0x01

#define BLE_ADVERTISE_SCAN_TYPE_ACTIVE 0x01
#define PRI_PHY_LE_AE_SCAN_INTERVAL    0x100
#define PRI_PHY_LE_AE_SCAN_WINDOW      0x50
#define BLE_ADVERTISE_SCAN_PHY_1M      0x00

///TODO: Must be 0x01 or 0x03 for filtering by acceptlist, but it doesn't work on silabs side
#define BLE_ADVERTISE_SCAN_FILTER_POLICY 0x00

typedef enum {
    BleAdvertiseAeDataTypeAdvertise = AE_ADV_DATA,
    BleAdvertiseAeDataTypeScanResponse = 0x03
} BleAdvertiseAeDataType;

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
    BleAdvertiseServiceClassUUID service_class;
} BleAdvertiseConfigAnonymous;

typedef struct FURI_PACKED {
    BleAdvertiseConfigAnonymous anonymous;
    BleAdvertiseWordData appearance;
    BleAdvertiseWordData manufacturer;
} BleAdvertiseConfigPublic;

static const BleAdvertiseConfigPublic advertise_config_template = {
    .anonymous =
        {
            .flags =
                {
                    .header = {.length = 2, .type = 1},
                    .data = 6,
                },
            .service_class =
                {
                    .header = {.type = 0x02, .length = 3},
                    .data = 0x308A,
                },
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
};

static_assert(sizeof(advertise_config_template) <= BLE_ADVERTISE_PACKET_MAX_SIZE);
//==========================================================

struct BleAdvertiseContext {
    FuriMutex* lock;
    BleAdvertiseLocalName* local_name;
};

BleAdvertiseContext* ble_advertise_alloc() {
    BleAdvertiseContext* instance = malloc(sizeof(BleAdvertiseContext));

    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->local_name = malloc(BLE_ADVERTISE_PACKET_MAX_SIZE);
    instance->local_name->header.type = BLE_ADVERTISE_DATA_TYPE_LOCAL_NAME;
    return instance;
}

void ble_advertise_free(BleAdvertiseContext* instance) {
    furi_assert(instance);
    furi_mutex_free(instance->lock);
    free(instance->local_name);
    free(instance);
}

void ble_advertise_set_name(BleAdvertiseContext* instance, const char* new_name) {
    furi_assert(instance);
    furi_assert(new_name);

    furi_mutex_acquire(instance->lock, FuriWaitForever);

    FuriString* name = furi_string_alloc_set_str(new_name);
    const size_t free_space = BLE_ADVERTISE_PACKET_MAX_SIZE - sizeof(BleAdvertiseHeader);

    if(furi_string_size(name) > free_space) {
        furi_string_left(name, free_space - 3);
        furi_string_cat_printf(name, "...");
    }

    const size_t name_size = furi_string_size(name);
    BLE_LOG_I("New device name: %s, size: %d", furi_string_get_cstr(name), name_size);

    instance->local_name->header.length = name_size + 1;
    memset(instance->local_name->data, 0, free_space);
    memcpy(instance->local_name->data, furi_string_get_cstr(name), name_size);

    furi_string_free(name);

    furi_mutex_release(instance->lock);
}

static void ble_advertise_set_ae_data(
    const BleAdvertiseAeDataType type,
    const uint8_t size,
    const void* data) {
    rsi_ble_ae_data_t ble_ae_data = {0};

    ble_ae_data.type = type;
    ble_ae_data.adv_handle = BLE_ADVERTISE_HANDLE;
    ble_ae_data.operation = BLE_ADVERTISE_DATA_OPERATION_COMPLETE;
    ble_ae_data.frag_pref = BLE_ADVERTISE_DATA_FRAGMENTATION_NOT_ALLOWED;
    ble_ae_data.data_len = size;
    if(size > 0) {
        memcpy(ble_ae_data.data, data, ble_ae_data.data_len);
    }

    sl_status_t status = rsi_ble_set_ae_data(&ble_ae_data);
    if(status != RSI_SUCCESS) {
        const char* type_name = type == BleAdvertiseAeDataTypeScanResponse ? "scan response" :
                                                                             "advertise";
        BLE_LOG_W("set %s data failed with 0x%lX", type_name, status);
    }
}

static void ble_advertise_refresh_data(const BleAdvertiseContext* instance, bool paired) {
    furi_assert(instance);
    furi_mutex_acquire(instance->lock, FuriWaitForever);

    const size_t adv_size = !paired ? sizeof(BleAdvertiseConfigPublic) :
                                      sizeof(BleAdvertiseConfigAnonymous);
    ble_advertise_set_ae_data(
        BleAdvertiseAeDataTypeAdvertise, adv_size, &advertise_config_template);

    const size_t scan_size =
        !paired ? (sizeof(BleAdvertiseHeader) + instance->local_name->header.length) : 0;
    ble_advertise_set_ae_data(BleAdvertiseAeDataTypeScanResponse, scan_size, instance->local_name);

    furi_mutex_release(instance->lock);
}

static void ble_advertise_set_scan_params() {
    rsi_ble_ae_set_scan_params_t ae_set_scan_params = {0};

    ae_set_scan_params.own_addr_type = LE_PUBLIC_ADDRESS;
    ae_set_scan_params.scanning_filter_policy = BLE_ADVERTISE_SCAN_FILTER_POLICY;
    ae_set_scan_params.scanning_phys = BLE_ADVERTISE_SCAN_PHY_1M;
    ae_set_scan_params.ScanParams[0].ScanType = BLE_ADVERTISE_SCAN_TYPE_ACTIVE;
    ae_set_scan_params.ScanParams[0].ScanInterval = PRI_PHY_LE_AE_SCAN_INTERVAL;
    ae_set_scan_params.ScanParams[0].ScanWindow = PRI_PHY_LE_AE_SCAN_WINDOW;

    sl_status_t status = rsi_ble_ae_set_scan_params(&ae_set_scan_params);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("set ae scan params failed with 0x%lX", status);
    }
}

static void ble_advertise_set_extended_params(
    const rsi_bt_event_le_security_keys_t* key,
    bool advertise_to_paired_only) {
    rsi_ble_clear_acceptlist();

    //! set ae adv params
    rsi_ble_ae_adv_params_t ble_ae_params = {0};

    ble_ae_params.adv_handle = BLE_ADVERTISE_HANDLE;
    ble_ae_params.adv_event_prop = BLE_ADVERTISE_CONNECTABLE_SCANNABLE_EVENT_PROP;
    ble_ae_params.primary_adv_intterval_min = BLE_ADVERTISE_INTERVAL_MIN;
    ble_ae_params.primary_adv_intterval_max = BLE_ADVERTISE_INTERVAL_MAX;
    ble_ae_params.primary_adv_chnl_map = BLE_ADVERTISE_CHANNEL_MAP;
    ble_ae_params.adv_tx_power = BLE_ADVERTISE_TX_PWR;
    ble_ae_params.primary_adv_phy = BLE_ADVERTISE_PRIMARY_PHY;
    ble_ae_params.sec_adv_phy = BLE_ADVERTISE_SECONDARY_PHY;
    ble_ae_params.sec_adv_max_skip = BLE_ADVERTISE_SECONDARY_MAX_SKIP;
    ble_ae_params.adv_sid = BLE_ADVERTISE_SID;
    ble_ae_params.scan_req_notify_enable = BLE_SCAN_REQUEST_NOTIFICATION_DISABLE;

    if(!advertise_to_paired_only) {
        ble_ae_params.own_addr_type = LE_PUBLIC_ADDRESS;
        ble_ae_params.peer_addr_type = LE_PUBLIC_ADDRESS;
        ble_ae_params.adv_filter_policy = BLE_ADVERTISE_FILTER_POLICY_ALLOW_ALL;
    } else {
        rsi_ble_addto_acceptlist((int8_t*)key->Identity_addr, key->Identity_addr_type);
        ble_ae_params.own_addr_type = LE_RESOLVABLE_PUBLIC_ADDRESS;
        ble_ae_params.peer_addr_type = key->Identity_addr_type;
        memcpy(ble_ae_params.peer_dev_addr, key->Identity_addr, RSI_DEV_ADDR_LEN);
        ble_ae_params.adv_filter_policy = BLE_ADVERTISE_FILTER_POLICY_ALLOW_PAIRED;
    }

    int8_t rsi_app_resp_tx_power = 0;
    sl_status_t status = rsi_ble_set_ae_params(&ble_ae_params, &rsi_app_resp_tx_power);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("set ae params failed with 0x%lX", status);
    } else {
        BLE_LOG_I(
            "Setting AE params successful and selected TX Power is %d dbm", rsi_app_resp_tx_power);
    }
}

static sl_status_t ble_advertise_nwp_start_stop(bool start) {
    rsi_ble_ae_adv_enable_t ble_ae_adv = {0};

    ble_ae_adv.enable = start ? RSI_BLE_START_ADV : RSI_BLE_STOP_ADV;
    ble_ae_adv.no_of_sets = 1;
    ble_ae_adv.adv_handle = BLE_ADVERTISE_HANDLE;
    ble_ae_adv.duration = 0;
    ble_ae_adv.max_ae_events = 0;

    return rsi_ble_start_ae_advertising(&ble_ae_adv);
}

bool ble_advertise_start(
    BleAdvertiseContext* instance,
    bool advertise_to_paired_only,
    const rsi_bt_event_le_security_keys_t* key) {
    furi_assert(instance);
    furi_assert(key);

    ble_advertise_set_extended_params(key, advertise_to_paired_only);
    ble_advertise_set_scan_params();

    ble_advertise_refresh_data(instance, advertise_to_paired_only);

    sl_status_t status = ble_advertise_nwp_start_stop(true);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to start advertising: 0x%08lX", status);
    } else {
        BLE_LOG_I("Start advertising...");
    }

    return status == RSI_SUCCESS;
}

bool ble_advertise_stop(BleAdvertiseContext* instance) {
    furi_assert(instance);
    UNUSED(instance);
    sl_status_t status = ble_advertise_nwp_start_stop(false);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to stop advertising: 0x%08lX", status);
    }
    return status == RSI_SUCCESS;
}

void ble_advertise_print_data(const BleAdvertiseContext* instance) {
    furi_assert(instance);

    furi_mutex_acquire(instance->lock, FuriWaitForever);

    const BleAdvertiseConfigPublic* const config = &advertise_config_template;

    BLE_LOG_I("Flags: %d", config->anonymous.flags.data);
    BLE_LOG_I("Appearance: %04X", config->appearance.data);
    BLE_LOG_I("Manufacturer: %04X", config->manufacturer.data);
    BLE_LOG_I(
        "Local name: %s, size: %d",
        instance->local_name->data,
        instance->local_name->header.length);

    furi_mutex_release(instance->lock);
}
