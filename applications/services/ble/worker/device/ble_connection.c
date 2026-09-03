#include "ble_connection.h"
#include "../_nwp_callbacks/ble_nwp_headers.h"

#include "../../ble_log.h"

#define TAG "BleConnection"

#define BLE_ADJUST_CONNECTION_PARAMETERS_RETRY_COUNT (10)
#define BLE_ADJUST_CONNECTION_PARAMETERS_TIMEOUT     (500)

typedef enum {
    BleConnectionCommandUpdatePhy,
    BleConnectionCommandUpdateDataLength,
    BleConnectionCommandEnableNwpDLE,
    BleConnectionCommandCount
} BleConnectionCommand;

typedef union {
    struct {
        bool phy_2m_update_done : 1;
        bool length_update_done : 1;
        bool dle_done           : 1;
    } feature;
    uint8_t value;
} BleConnectionUpdateStatus;

typedef void (*BleConnectionCommandHandler)(BleConnectionContext* instance);

struct BleConnectionContext {
    BleConnectionTimings timings;
    BleConnectionDataLength data_length_params;
    BlePhy TxPhy;
    BlePhy RxPhy;

    BleDeviceBase* peer;

    BleConnectionUpdateStatus current_status;
    BleConnectionUpdateStatus expected_status;
    BleConnectionCommand next_update_command;
    uint8_t update_param_retry_count;
    FuriEventLoopTimer* update_param_timer;
    BleConnectionUpdateParametersDoneCallback done_cb;
    void* done_ctx;
};

static void connection_update_callback(void* context);

BleConnectionContext*
    ble_connection_alloc(BleDeviceAddressType type, const uint8_t* const peer_address) {
    furi_assert(peer_address);

    BleConnectionContext* instance = malloc(sizeof(BleConnectionContext));
    instance->peer = ble_device_base_alloc(BleDeviceRoleCentral);
    ble_device_base_set_address(instance->peer, type, peer_address);

    return instance;
}

void ble_connection_free(BleConnectionContext* instance) {
    furi_assert(instance);

    if(instance->update_param_timer) {
        furi_event_loop_timer_stop(instance->update_param_timer);
        furi_event_loop_timer_free(instance->update_param_timer);
    }

    ble_device_base_free(instance->peer);
    free(instance);
}

BleDeviceBase* ble_connection_get_peer(BleConnectionContext* instance) {
    furi_assert(instance);
    return instance->peer;
}

const BleConnectionTimings* ble_connection_get_timings(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->timings;
}

void ble_connection_set_timings(
    BleConnectionContext* instance,
    const BleConnectionTimings* const timings) {
    furi_assert(instance);
    furi_assert(timings);
    memcpy(&instance->timings, timings, sizeof(BleConnectionTimings));
}

const BleConnectionDataLength* ble_connection_get_data_length(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->data_length_params;
}

void ble_connection_set_data_length(
    BleConnectionContext* instance,
    const BleConnectionDataLength* const data_length) {
    furi_assert(instance);
    furi_assert(data_length);
    memcpy(&instance->data_length_params, data_length, sizeof(BleConnectionDataLength));
    BLE_LOG_I("Length set done");
    instance->current_status.feature.length_update_done = true;
}

const BlePhy* ble_connection_get_tx_phy(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->TxPhy;
}

const BlePhy* ble_connection_get_rx_phy(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->RxPhy;
}

void ble_connection_set_phy(
    BleConnectionContext* instance,
    const uint8_t tx_phy,
    const uint8_t rx_phy) {
    furi_assert(instance);
    instance->TxPhy.value = tx_phy;
    instance->RxPhy.value = rx_phy;

    BLE_LOG_I("PHY set done");
    instance->current_status.feature.phy_2m_update_done = instance->TxPhy.flags.phy_le_2m;
}

static void ble_connection_command_update_phy(BleConnectionContext* instance) {
    BleDeviceBase* peer = instance->peer;
    do {
        if(!ble_device_base_is_feature_supported(peer, BleDeviceFeaturesLE2MPhy)) {
            BLE_LOG_W("2MPhy not supported, skip");
            instance->next_update_command = BleConnectionCommandUpdateDataLength;
            break;
        }

        if(instance->current_status.feature.phy_2m_update_done) {
            BLE_LOG_I("2MPhy already, skip");
            instance->next_update_command = BleConnectionCommandUpdateDataLength;
            break;
        }

        const uint8_t* addr = ble_device_base_get_address(peer, BleDeviceAddressTypeOrigin);
        sl_status_t status =
            rsi_ble_setphy((const int8_t*)addr, TX_PHY_RATE, RX_PHY_RATE, CODDED_PHY_RATE);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("Failed to set phy, error code : 0x%08lx", status);
            break;
        }

        instance->next_update_command = BleConnectionCommandUpdateDataLength;
    } while(false);
}

static void ble_connection_command_update_data_length(BleConnectionContext* instance) {
    BleDeviceBase* peer = instance->peer;
    do {
        if(!ble_device_base_is_feature_supported(
               peer, BleDeviceFeaturesLEDataPacketLengthExtension)) {
            instance->next_update_command = BleConnectionCommandEnableNwpDLE;
            break;
        }

        if(instance->current_status.feature.length_update_done) {
            BLE_LOG_I("Length already set, skip");
            instance->next_update_command = BleConnectionCommandEnableNwpDLE;
            break;
        }

        const uint8_t* addr = ble_device_base_get_address(peer, BleDeviceAddressTypeOrigin);
        sl_status_t status = rsi_ble_set_data_len((uint8_t*)addr, TX_LEN, TX_TIME);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("\n Set data length cmd failed with error code = %lx \n", status);
            break;
        }

        instance->next_update_command = BleConnectionCommandEnableNwpDLE;
    } while(false);
}

static void ble_connection_command_enable_nwp_dle(BleConnectionContext* instance) {
    BleDeviceBase* peer = instance->peer;

    if(instance->current_status.feature.dle_done) {
        BLE_LOG_I("Dle already set, goto start");
        instance->next_update_command = BleConnectionCommandUpdatePhy;
        return;
    }

    const uint8_t* addr = ble_device_base_get_address(peer, BleDeviceAddressTypeOrigin);
    sl_status_t status =
        rsi_ble_set_wo_resp_notify_buf_info(addr, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set the buffer configuration mode, error: 0x%08lx", status);
    } else {
        BLE_LOG_I(
            "Buffer configuration done for notify and set_att cmds buf mode = %d , max buff count =%d",
            DLE_BUFFER_MODE,
            DLE_BUFFER_COUNT);
        instance->current_status.feature.dle_done = true;
        instance->next_update_command = BleConnectionCommandUpdatePhy;
    }
}

static const BleConnectionCommandHandler commands[BleConnectionCommandCount] = {
    [BleConnectionCommandUpdatePhy] = ble_connection_command_update_phy,
    [BleConnectionCommandUpdateDataLength] = ble_connection_command_update_data_length,
    [BleConnectionCommandEnableNwpDLE] = ble_connection_command_enable_nwp_dle,
};

static void connection_update_callback(void* context) {
    BleConnectionContext* instance = context;
    BLE_LOG_I("%s", __func__);

    BleConnectionCommandHandler handler = commands[instance->next_update_command];
    handler(instance);

    bool all_updates_done = instance->current_status.value == instance->expected_status.value;
    if(all_updates_done) {
        instance->done_cb(instance->done_ctx);
    } else {
        instance->update_param_retry_count += 1;

        if(instance->update_param_retry_count == BLE_ADJUST_CONNECTION_PARAMETERS_RETRY_COUNT) {
            BLE_LOG_W("Max retry count exceeded");
            instance->done_cb(instance->done_ctx);
        } else {
            furi_event_loop_timer_start(
                instance->update_param_timer, BLE_ADJUST_CONNECTION_PARAMETERS_TIMEOUT);
        }
    }
}

static void ble_connection_get_update_expected_status(BleConnectionContext* instance) {
    instance->expected_status.feature.phy_2m_update_done =
        ble_device_base_is_feature_supported(instance->peer, BleDeviceFeaturesLE2MPhy);

    instance->expected_status.feature.length_update_done = ble_device_base_is_feature_supported(
        instance->peer, BleDeviceFeaturesLEDataPacketLengthExtension);

    instance->expected_status.feature.dle_done = true;
}

void ble_connection_start_update_parameters(
    BleConnectionContext* instance,
    FuriEventLoop* event_loop,
    BleConnectionUpdateParametersDoneCallback done_cb,
    void* context) {
    furi_assert(instance);
    furi_assert(event_loop);
    furi_assert(done_cb);
    furi_assert(context);

    if(instance->update_param_timer == NULL) {
        instance->update_param_timer = furi_event_loop_timer_alloc(
            event_loop, connection_update_callback, FuriEventLoopTimerTypeOnce, instance);
        instance->done_cb = done_cb;
        instance->done_ctx = context;
        instance->next_update_command = BleConnectionCommandUpdatePhy;

        ble_connection_get_update_expected_status(instance);

        connection_update_callback(instance);
    }
}
