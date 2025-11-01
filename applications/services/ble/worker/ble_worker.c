#include "ble_worker.h"
#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "ble_config.h"
#include "ble_security.h"

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#include "ble_advertise.h"
#include "ble_worker_util.h"
#include "../service/ble_service_i.h"

#include <m-dict.h>

#define TAG "BleWorker"

#define BLE_DEFAULT_LOCAL_NAME "BUSY Bar"

#define BLE_WORKER_LOCAL_DEV_ADDR_LEN 18 // Length of the local device address
#define BLE_WORKER_MAX_MTU_SIZE       240
#define BLE_WORKER_ATTR_HEADER_SIZE   3

#define UUID_SIZE 16

#define BLE_WORKER_BT_HCI_COMMAND_DISALLOWED 0x4E0C

#define BLE_CCCD_NOTIFICATION_ENABLED(cccd_value) ((cccd_value & 0x01) != 0)
#define BLE_CCCD_INDICATION_ENABLED(cccd_value)   ((cccd_value & 0x02) != 0)

//! Configuration bitmap for attributes
#define ATT_REC_MAINTAIN_IN_HOST BIT(0) ///< Attribute record maintained in Host
#define SEC_MODE_1_LEVEL_1       BIT(1) ///< NO Auth and No Enc
#define SEC_MODE_1_LEVEL_2       BIT(2) ///< UnAUTH with Enc
#define SEC_MODE_1_LEVEL_3       BIT(3) ///< AUTH with Enc
#define SEC_MODE_1_LEVEL_4       BIT(4) ///< AUTH LE_SC Pairing with Enc
#define ON_BR_EDR_LINK_ONLY      BIT(5) ///< BR/EDR link-only mode
#define ON_LE_LINK_ONLY          BIT(6) ///< LE link-only mode
#define VARIABLE_ATT_CHAR_VAL    BIT(7) ///< Variable characteristic value length

#define RSI_BLE_ATT_CONFIG_BITMAP (SEC_MODE_1_LEVEL_4)

#ifdef RSI_BLE_SMP_IO_CAPABILITY
#undef RSI_BLE_SMP_IO_CAPABILITY
#define RSI_BLE_SMP_IO_CAPABILITY 0x03
#endif

#define MITM_REQ 1

//! application events list
typedef enum {
    BLEWorkerEvtExit = (1 << 0),
    BLEWorkerEvtAdvReport = (1 << 1),
    BLEWorkerEvtConnected = (1 << 2),
    BLEWorkerEvtDisconnected = (1 << 3),
    BLEWorkerEvtPhyUpdateComplete = (1 << 4),
    BLEWorkerEvtConnUpdate = (1 << 5),
    BLEWorkerEvtDataLengthChange = (1 << 6),

    BLEWorkerEvtReceveRemoteFeatures = (1 << 7),
    BLEWorkerEvtMoreDataReq = (1 << 8),

    BLEWorkerEvtWrite = (1 << 9),
    BLEWorkerEvtDataTransmit = (1 << 10),
    BLEWorkerEvtMtu = (1 << 11),
    BLEWorkerEvtIndicateConfirm = (1 << 12),

    BLEWorkerSmpResponse = (1 << 13),
    BLEWorkerSmpEncryptStarted = (1 << 14),
    BLEWorkerSmpLtkRequest = (1 << 15),
    BLEWorkerSmpSecurityKeys = (1 << 16),
} BLEWorkerEvt;

#define BLE_USART_ECHO_ALL_EVENTS                                                                \
    (BLEWorkerEvtExit | BLEWorkerEvtAdvReport | BLEWorkerEvtConnected |                          \
     BLEWorkerEvtDisconnected | BLEWorkerEvtPhyUpdateComplete | BLEWorkerEvtConnUpdate |         \
     BLEWorkerEvtDataLengthChange | BLEWorkerEvtReceveRemoteFeatures | BLEWorkerEvtMoreDataReq | \
     BLEWorkerEvtWrite | BLEWorkerEvtDataTransmit | BLEWorkerEvtMtu |                            \
     BLEWorkerEvtIndicateConfirm | BLEWorkerSmpResponse | BLEWorkerSmpLtkRequest |               \
     BLEWorkerSmpEncryptStarted | BLEWorkerSmpSecurityKeys)

typedef struct {
    ///TODO: for now this is ok, for future maybe it is worth to make each characteristic
    /// know its own service.
    BleServiceObject* service;
    uint16_t char_index;
} BleServiceEntry;

DICT_DEF2(BleServiceEntryDict, uint16_t, M_DEFAULT_OPLIST, BleServiceEntry, M_POD_OPLIST)

typedef enum {
    BleWorkerStateIdle,
    BleWorkerStateAdvertising,
    BleWorkerStateConnected,
    BleWorkerStateError,
} BleWorkerState;

typedef struct {
    FuriThread* thread;
    FuriSemaphore* indication_sem;
    FuriSemaphore* notification_sem;
    uint8_t pairing_info_available;
    ///TODO: this can be removed
    bool connected;
    BleWorkerState state;
    uint16_t max_payload_size;
    uint8_t device_found;
    uint8_t conn_params_updated;
    uint8_t remote_name[31];
    uint8_t remote_addr_type;
    uint8_t remote_dev_str_addr[18];
    uint8_t remote_dev_bd_addr[6];

    uint8_t str_remote_address[18];
    uint8_t remote_dev_address[6];

    rsi_ble_event_phy_update_t app_phy_update_complete;
    rsi_ble_event_data_length_update_t data_length_update;
    rsi_ble_event_conn_update_t event_conn_update_complete;

    rsi_ble_event_remote_features_t remote_dev_feature;

    rsi_ble_event_write_t app_ble_write_event;
    rsi_ble_event_mtu_t app_ble_mtu_event;

    rsi_bt_event_smp_resp_t rsi_bt_event_smp_resp;
    rsi_bt_event_le_ltk_request_t ble_ltk_req;
    BleSecurityData* security_data;
    BleAdvertiseContext* advertise;

    BleServiceEntryDict_t service_dict;
} BleWorker;

//==========================================================
static BleWorker* ble_worker_instance;
/*==============================================*/
/**
 * @fn         ble_usart_echo_app_on_adv_report_event
 * @brief      invoked when advertise report event is received
 * @param[in]  adv_report, pointer to the received advertising report
 * @return     none.
 * @section description
 * This callback function updates the scanned remote devices list
 */
static void ble_worker_on_adv_report_event(rsi_ble_event_adv_report_t* adv_report) {
    BLE_LOG_W("ble_worker_on_adv_report_event");
    if(ble_worker_instance->device_found == 1) {
        return;
    }

    memset(&ble_worker_instance->remote_name, 0, sizeof(ble_worker_instance->remote_name));
    BT_LE_ADPacketExtract(
        ble_worker_instance->remote_name, adv_report->adv_data, adv_report->adv_data_len);

    ble_worker_instance->remote_addr_type = adv_report->dev_addr_type;
    rsi_6byte_dev_address_to_ascii(
        ble_worker_instance->remote_dev_str_addr, (uint8_t*)adv_report->dev_addr);
    memcpy((int8_t*)ble_worker_instance->remote_dev_bd_addr, (uint8_t*)adv_report->dev_addr, 6);

#if(CONNECT_OPTION == CONN_BY_NAME)
    if((ble_worker_instance->device_found == 0) &&
       ((strcmp((const char*)ble_worker_instance->remote_name, RSI_REMOTE_DEVICE_NAME)) == 0)) {
        ble_worker_instance->device_found = 1;

        furi_thread_flags_set(
            furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtAdvReport);
        return;
    }
#elif(CONNECT_OPTION == CONN_BY_ADDR)
    if((!strcmp(RSI_BLE_REMOTE_DEV_ADDR, (char*)ble_worker_instance->remote_dev_str_addr))) {
        ble_worker_instance->device_found = 1;
        furi_thread_flags_set(
            furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtAdvReport);
    }
#endif

    return;
}

/*==============================================*/
/**
 * @fn         ble_worker_on_connect_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void ble_worker_on_connect_event(rsi_ble_event_conn_status_t* resp_conn) {
    memcpy(ble_worker_instance->remote_dev_address, resp_conn->dev_addr, 6);
    rsi_6byte_dev_address_to_ascii(ble_worker_instance->str_remote_address, resp_conn->dev_addr);
    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtConnected);
}

/**
 * @fn         ble_worker_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void
    ble_worker_on_disconnect_event(rsi_ble_event_disconnect_t* resp_disconnect, uint16_t reason) {
    UNUSED(
        reason); //This statement is added only to resolve compilation warning, value is unchanged
    memcpy(ble_worker_instance->remote_dev_address, resp_disconnect->dev_addr, 6);
    rsi_6byte_dev_address_to_ascii(
        ble_worker_instance->str_remote_address, resp_disconnect->dev_addr);

    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtDisconnected);
}

/**
 * @fn         ble_worker_phy_update_complete_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
static void ble_worker_phy_update_complete_event(
    rsi_ble_event_phy_update_t* rsi_ble_event_phy_update_complete) {
    memcpy(
        &ble_worker_instance->app_phy_update_complete,
        rsi_ble_event_phy_update_complete,
        sizeof(rsi_ble_event_phy_update_t));
    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtPhyUpdateComplete);
}

/**
 * @fn         ble_worker_data_length_change_event
 * @brief      invoked when data length is set
 * @section description
 * This Callback function indicates data length is set
 */
static void ble_worker_data_length_change_event(
    rsi_ble_event_data_length_update_t* rsi_ble_data_length_update) {
    memcpy(
        &ble_worker_instance->data_length_update,
        rsi_ble_data_length_update,
        sizeof(rsi_ble_event_data_length_update_t));

    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtDataLengthChange);
}

/**
 * @fn         ble_worker_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void
    ble_worker_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t* resp_enh_conn) {
    memcpy(ble_worker_instance->remote_dev_address, resp_enh_conn->dev_addr, 6);
    rsi_6byte_dev_address_to_ascii(
        ble_worker_instance->str_remote_address, resp_enh_conn->dev_addr);
    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtConnected);
}

static void ble_worker_on_conn_update_complete_event(
    rsi_ble_event_conn_update_t* rsi_ble_event_conn_update_complete,
    uint16_t resp_status) {
    UNUSED(resp_status);
    memcpy(
        &ble_worker_instance->event_conn_update_complete,
        rsi_ble_event_conn_update_complete,
        sizeof(rsi_ble_event_conn_update_t));
    memcpy(
        ble_worker_instance->remote_dev_address, rsi_ble_event_conn_update_complete->dev_addr, 6);

    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtConnUpdate);
}

static void rsi_ble_on_directed_adv_report_event(
    rsi_ble_event_directedadv_report_t* rsi_ble_event_directed) {
    BLE_LOG_W("rsi_ble_on_directed_adv_report_event");
    UNUSED(rsi_ble_event_directed);
}
/*==============================================*/
/**
 * @fn         ble_worker_simple_peripheral_on_remote_features_event
 * @brief      invoked when LE remote features event is received.
 * @param[in] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void ble_worker_simple_peripheral_on_remote_features_event(
    rsi_ble_event_remote_features_t* rsi_ble_event_remote_features) {
    memcpy(
        &ble_worker_instance->remote_dev_feature,
        rsi_ble_event_remote_features,
        sizeof(rsi_ble_event_remote_features_t));
    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtReceveRemoteFeatures);
}

static void ble_worker_more_data_req_event(rsi_ble_event_le_dev_buf_ind_t* rsi_ble_more_data_evt) {
    UNUSED(rsi_ble_more_data_evt);

    //! set conn specific event
    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtMoreDataReq);

    return;
}

/*==============================================*/
/**
 * @fn         ble_worker_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void
    ble_worker_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t* rsi_ble_write) {
    UNUSED(event_id);

    memcpy(
        &ble_worker_instance->app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));
    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtWrite);
}

static void ble_worker_on_indicate_confirmation_event(
    uint16_t event_status,
    rsi_ble_set_att_resp_t* rsi_ble_event_set_att_rsp) {
    UNUSED(rsi_ble_event_set_att_rsp);

    if(event_status != 0) BLE_LOG_W("Indicate_CB: %d", event_status);

    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtIndicateConfirm);
}

/**
 * @fn         ble_worker_on_mtu_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void ble_worker_on_mtu_event(rsi_ble_event_mtu_t* rsi_ble_mtu) {
    memcpy(&ble_worker_instance->app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
    rsi_6byte_dev_address_to_ascii(
        ble_worker_instance->str_remote_address, ble_worker_instance->app_ble_mtu_event.dev_addr);

    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtMtu);
}

//===========================================================================================

static void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_W("rsi_ble_on_smp_request");
}

static void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t* resp) {
    BLE_LOG_D("rsi_ble_on_smp_response");
    memcpy(&ble_worker_instance->rsi_bt_event_smp_resp, resp, sizeof(rsi_bt_event_smp_resp_t));
    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpResponse);
}

static void rsi_ble_on_smp_passkey(rsi_bt_event_smp_passkey_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_W("rsi_ble_on_smp_passkey");
}

static void
    rsi_ble_on_smp_failed(uint16_t resp_status, rsi_bt_event_smp_failed_t* remote_dev_address) {
    UNUSED(resp_status);
    UNUSED(remote_dev_address);
    BLE_LOG_W("rsi_ble_on_smp_failed status: %X", resp_status);
}

static void rsi_ble_on_encrypt_started(
    uint16_t resp_status,
    rsi_bt_event_encryption_enabled_t* enc_enabled) {
    UNUSED(resp_status);
    BLE_LOG_D("rsi_ble_on_encrypt_started status: %X", resp_status);
    ble_security_set_pairing_data(ble_worker_instance->security_data, enc_enabled);

    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpEncryptStarted);
}

static void
    rsi_ble_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t* rsi_ble_event_le_ltk_request) {
    UNUSED(rsi_ble_event_le_ltk_request);

    BLE_LOG_D("rsi_ble_on_le_ltk_req_event");
    memcpy(
        &ble_worker_instance->ble_ltk_req,
        rsi_ble_event_le_ltk_request,
        sizeof(rsi_bt_event_le_ltk_request_t));
    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpLtkRequest);
}

static void
    rsi_ble_on_le_security_keys(rsi_bt_event_le_security_keys_t* rsi_ble_event_le_security_keys) {
    BLE_LOG_D("rsi_ble_on_le_security_keys");
    ble_security_set_rpa_data(ble_worker_instance->security_data, rsi_ble_event_le_security_keys);

    furi_thread_flags_set(
        furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpSecurityKeys);
}

static void ble_on_cli_smp_response_event(rsi_bt_event_smp_resp_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_W("ble_on_cli_smp_response_event");
}

static void rsi_ble_on_sc_method(rsi_bt_event_sc_method_t* scmethod) {
    UNUSED(scmethod);
    BLE_LOG_W("rsi_ble_on_sc_method");
}
//===========================================================================================
static bool ble_worker_start_advertising(
    bool rpa_enabled,
    const rsi_bt_event_le_security_keys_t* key,
    const BleAdvertiseContext* advertise) {
    rsi_ble_req_adv_t ble_adv = {0};
    ble_adv.status = RSI_BLE_START_ADV;

    ble_adv.adv_type = rpa_enabled ? DIR_CONN_LOW_DUTY_CYCLE : UNDIR_CONN;
    ble_adv.filter_type = RSI_BLE_ADV_FILTER_TYPE;
    if(rpa_enabled) {
        ble_adv.direct_addr_type = key->Identity_addr_type;
        memcpy(ble_adv.direct_addr, key->Identity_addr, RSI_DEV_ADDR_LEN);
    }

    ble_adv.adv_int_min = RSI_BLE_ADV_INT_MIN;
    ble_adv.adv_int_max = RSI_BLE_ADV_INT_MAX;
    ble_adv.adv_channel_map = RSI_BLE_ADV_CHANNEL_MAP;

    ble_adv.own_addr_type = rpa_enabled ? LE_RESOLVABLE_RANDOM_ADDRESS : LE_PUBLIC_ADDRESS;

    ble_advertise_refresh_data(advertise);

    sl_status_t status = rsi_ble_start_advertising_with_values(&ble_adv);

    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to start advertising, error code : 0x%08lx", status);
    } else {
        BLE_LOG_I("Start advertising...");
    }

    return status == RSI_SUCCESS;
}

static bool ble_worker_stop_advertising() {
    sl_status_t status = rsi_ble_stop_advertising();

    if(status != RSI_SUCCESS)
        BLE_LOG_W("Failed to stop advertising, error code : 0x%08lx", status);
    else
        BLE_LOG_I("Stop advertising...");

    return status == RSI_SUCCESS;
}

static void ble_hw_config() {
    sl_status_t status = 0;
    static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = {0};
    uint8_t local_dev_addr[BLE_WORKER_LOCAL_DEV_ADDR_LEN] = {0};

    //! get the local device MAC address.
    status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Get local device address failed = 0x%08lx", status);
        ///TODO: don't crash, return false instead
        furi_crash();
    } else {
        rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
        BLE_LOG_I("Local device address %s", local_dev_addr);
    }

    // //! registering the GAP callback functions
    rsi_ble_gap_register_callbacks(
        ble_worker_on_adv_report_event,
        ble_worker_on_connect_event,
        ble_worker_on_disconnect_event,
        NULL,
        ble_worker_phy_update_complete_event,
        ble_worker_data_length_change_event,
        ble_worker_on_enhance_conn_status_event,
        rsi_ble_on_directed_adv_report_event,
        ble_worker_on_conn_update_complete_event,
        NULL);

    rsi_ble_smp_register_callbacks(
        rsi_ble_on_smp_request,
        rsi_ble_on_smp_response,
        rsi_ble_on_smp_passkey,
        rsi_ble_on_smp_failed,
        rsi_ble_on_encrypt_started,
        NULL,
        NULL,
        rsi_ble_on_le_ltk_req_event,
        rsi_ble_on_le_security_keys,
        ble_on_cli_smp_response_event,
        rsi_ble_on_sc_method);

    rsi_ble_gap_extended_register_callbacks(
        ble_worker_simple_peripheral_on_remote_features_event, ble_worker_more_data_req_event);

    rsi_ble_gatt_register_callbacks(
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ble_worker_on_gatt_write_event,
        NULL,
        NULL,
        NULL,
        ble_worker_on_mtu_event,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ble_worker_on_indicate_confirmation_event,
        NULL);

    //! Set local name
    status = rsi_bt_set_local_name((const uint8_t*)BLE_DEFAULT_LOCAL_NAME);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set local name, error code : 0x%08lx", status);
        furi_crash();
    }

    ble_advertise_print_data(ble_worker_instance->advertise);

    // ble_adjust_gap_service_data();

    ble_worker_instance->pairing_info_available =
        ble_security_init(ble_worker_instance->security_data);

    status = rsi_ble_set_random_address_with_value(rsi_app_resp_get_dev_addr);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set address: %08lX", status);
    }
}

static int32_t ble_worker_thread_callback(void* context) {
    BleWorker* instance = context;

    sl_status_t status = 0;

    FURI_LOG_D(TAG, "Worker Thread Start");
    while(true) {
        uint32_t events =
            furi_thread_flags_wait(BLE_USART_ECHO_ALL_EVENTS, FuriFlagWaitAny, FuriWaitForever);

        if(events & BLEWorkerEvtConnected) {
            //! event invokes when connection was completed
            BLE_LOG_I("Connected, str_remote_address : %s", instance->str_remote_address);
            instance->state = BleWorkerStateConnected;
            //! Setting MTU Exchange event
            status =
                rsi_ble_mtu_exchange_event(instance->remote_dev_address, BLE_WORKER_MAX_MTU_SIZE);
            if(status != RSI_SUCCESS) {
                BLE_LOG_W("MTU request cmd failed with error code = 0x%08lx", status);
                furi_crash();
            } else {
                BLE_LOG_I("MTU sent");
            }

            if(!instance->conn_params_updated) {
                status = rsi_ble_conn_params_update(
                    instance->remote_dev_address,
                    CONN_INTERVAL_MIN,
                    CONN_INTERVAL_MAX,
                    CONN_LATENCY,
                    SUPERVISION_TIMEOUT);
                if(status != RSI_SUCCESS) {
                    BLE_LOG_W(
                        "Failed to update connection parameters, error code : 0x%08lx", status);
                    furi_crash();
                }
            }

            ble_worker_instance->connected = true;
        }

        if(events & BLEWorkerEvtDisconnected) {
            //! event invokes when disconnection was completed
            BLE_LOG_I("Disconnected, str_remote_address : %s", instance->str_remote_address);

            instance->device_found = 0;
            instance->conn_params_updated = 0;
            instance->connected = false;

            BleServiceEntryDict_it_t entry_iter;
            for(BleServiceEntryDict_it(entry_iter, instance->service_dict);
                !BleServiceEntryDict_end_p(entry_iter);
                BleServiceEntryDict_next(entry_iter)) {
                BleServiceEntryDict_itref_t* entry_ref = BleServiceEntryDict_ref(entry_iter);

                BleServiceEntry* entry = &entry_ref->value;
                BleServiceObject* service = entry->service;
                if(ble_service_lock(service)) {
                    BleCharacteristicObject* ch = service->chars[entry->char_index];
                    ble_characteristic_set_cccd_value(ch, 0);
                    ble_service_unlock(service);
                }
            }

            //! start advertising
            const rsi_bt_event_le_security_keys_t* rpa =
                ble_security_get_rpa_data(ble_worker_instance->security_data);
            instance->state = ble_worker_start_advertising(
                                  ble_worker_instance->pairing_info_available,
                                  rpa,
                                  ble_worker_instance->advertise) ?
                                  BleWorkerStateAdvertising :
                                  BleWorkerStateError;
        }

        if(events & BLEWorkerEvtReceveRemoteFeatures) {
            //! event invokes when remote features were received
            BLE_LOG_I(
                "Feature received is 0x%04X",
                *(uint16_t*)instance->remote_dev_feature.remote_features);

            if(instance->remote_dev_feature.remote_features[0] & 0x20) {
                status = rsi_ble_set_data_len(instance->remote_dev_address, TX_LEN, TX_TIME);
                if(status != RSI_SUCCESS) {
                    BLE_LOG_W("Failed to set data length, error code : 0x%08lx", status);

                    furi_thread_flags_set(
                        furi_thread_get_id(ble_worker_instance->thread),
                        BLEWorkerEvtReceveRemoteFeatures);
                }

            } else if(instance->remote_dev_feature.remote_features[1] & 0x01) {
                status = rsi_ble_setphy(
                    (int8_t*)instance->remote_dev_address,
                    TX_PHY_RATE,
                    RX_PHY_RATE,
                    CODDED_PHY_RATE);
                if(status != RSI_SUCCESS) {
                    if(status != BLE_WORKER_BT_HCI_COMMAND_DISALLOWED) {
                        //retry the same command
                        furi_thread_flags_set(
                            furi_thread_get_id(ble_worker_instance->thread),
                            BLEWorkerEvtDataLengthChange);
                    } else {
                        BLE_LOG_W("Failed to set phy, error code : 0x%08lx", status);
                    }
                }
            }
        }

        if(events & BLEWorkerEvtDataLengthChange) {
            BLE_LOG_I(
                "Max_tx_octets: %d\r\nMax_tx_time: %d\r\nMax_rx_octets: %d\r\nMax_rx_time: %d",
                instance->data_length_update.MaxTxOctets,
                instance->data_length_update.MaxTxTime,
                instance->data_length_update.MaxRxOctets,
                instance->data_length_update.MaxRxTime);

            if(instance->remote_dev_feature.remote_features[1] & 0x01) {
                osDelay(500);
                status = rsi_ble_setphy(
                    (int8_t*)instance->remote_dev_address,
                    TX_PHY_RATE,
                    RX_PHY_RATE,
                    CODDED_PHY_RATE);
                if(status != RSI_SUCCESS) {
                    if(status != BLE_WORKER_BT_HCI_COMMAND_DISALLOWED) {
                        //retry the same command
                        furi_thread_flags_set(
                            furi_thread_get_id(ble_worker_instance->thread),
                            BLEWorkerEvtDataLengthChange);
                    } else {
                        BLE_LOG_W("Failed to set phy, error code : 0x%08lx", status);
                    }
                }
            }
        }

        if(events & BLEWorkerEvtPhyUpdateComplete) {
            //! phy update complete event
            BLE_LOG_I(
                "Tx Phy rate = 0x%x  and Rx Phy rate = 0x%x",
                instance->app_phy_update_complete.TxPhy,
                instance->app_phy_update_complete.RxPhy);
        }

        if(events & BLEWorkerEvtConnUpdate) {
            BLE_LOG_I(
                "Connection parameters update completed \r\n Connection interval = %d, Latency = %d, Supervision Timeout = %d",
                instance->event_conn_update_complete.conn_interval,
                instance->event_conn_update_complete.conn_latency,
                instance->event_conn_update_complete.timeout);
        }

        if(events & BLEWorkerEvtMtu) {
            //! event invokes when write/notification events received
            BLE_LOG_I(
                "MTU size received from remote device(%s) is %u",
                instance->str_remote_address,
                instance->app_ble_mtu_event.mtu_size);

            ble_worker_instance->max_payload_size =
                instance->app_ble_mtu_event.mtu_size - BLE_WORKER_ATTR_HEADER_SIZE;
            BLE_LOG_I("Max payload size: %u", ble_worker_instance->max_payload_size);

            status = rsi_ble_set_wo_resp_notify_buf_info(
                instance->remote_dev_address, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
            if(status != RSI_SUCCESS) {
                BLE_LOG_W("Failed to set the buffer configuration mode, error: 0x%08lx", status);
            } else {
                BLE_LOG_I(
                    "Buffer configuration done for notify and set_att cmds buf mode = %d , max buff count =%d",
                    DLE_BUFFER_MODE,
                    DLE_BUFFER_COUNT);
            }
        }

        if(events & BLEWorkerEvtWrite) {
            BLE_LOG_D("Received packet type = %u", instance->app_ble_write_event.pkt_type);

            if(instance->app_ble_write_event.pkt_type == RSI_BLE_WRITE_REQUEST_EVENT) {
                const void* data = instance->app_ble_write_event.att_value;
                const size_t data_size = instance->app_ble_write_event.length;

                uint16_t handle = *(uint16_t*)instance->app_ble_write_event.handle;

                if(handle == 0x001D) BLE_LOG_W("Subscribed!");

                BleServiceEntry* entry =
                    BleServiceEntryDict_get(ble_worker_instance->service_dict, handle);

                if(entry) {
                    BLE_LOG_D("Entry present");
                    BleServiceObject* service = entry->service;
                    if(ble_service_lock(service)) {
                        BleCharacteristicObject* ch = service->chars[entry->char_index];

                        if(ble_characteristic_is_cccd_handle(ch, handle)) {
                            ble_characteristic_set_cccd_value(ch, *((uint8_t*)data));
                        } else {
                            ble_characteristic_set_data(ch, data, data_size);
                            ble_service_enqueue_run(service);
                        }

                        ble_service_unlock(service);
                    } else
                        furi_crash("FAIL!");
                } else {
                    BLE_LOG_W("Not found: %04X", handle);
                    status =
                        rsi_ble_gatt_write_response(ble_worker_instance->remote_dev_address, 0);
                }
            } else if(instance->app_ble_write_event.pkt_type == RSI_BLE_NOTIFICATION_EVENT) {
                BLE_LOG_W("Notification event");
            } else if(instance->app_ble_write_event.pkt_type == RSI_BLE_INDICATION_EVENT) {
                BLE_LOG_W("Indication event");
            }
        }

        if(events & BLEWorkerEvtMoreDataReq) {
            BLE_LOG_D("BLEWorkerEvtMoreDataReq");
            furi_semaphore_release(ble_worker_instance->notification_sem);
        }

        if(events & BLEWorkerEvtExit) {
            instance->state = ble_worker_stop_advertising() ? BleWorkerStateIdle :
                                                              BleWorkerStateError;
            break;
        }

        if(events & BLEWorkerEvtIndicateConfirm) {
            BLE_LOG_D("BLEWorkerEvtIndicateConfirm");
            furi_semaphore_release(ble_worker_instance->indication_sem);
        }

        if(events & BLEWorkerSmpResponse) {
            BLE_LOG_I("BLEWorkerSmpResponse");
            status = rsi_ble_smp_pair_response(
                ble_worker_instance->remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_REQ);

            if(status != SL_STATUS_OK) {
                BLE_LOG_W("Passkey Status: %lX", status);
            }
            ble_worker_instance->pairing_info_available = 0;
        }

        if(events & BLEWorkerSmpEncryptStarted) {
            BLE_LOG_I("BLEWorkerSmpEncryptStarted");
            if(ble_worker_instance->pairing_info_available == 0) {
                ble_worker_instance->pairing_info_available = 1;

                if(ble_security_save_data(ble_worker_instance->security_data))
                    BLE_LOG_I("Security data saved");
                else
                    BLE_LOG_W("Failed to save Security");
            }
        }

        if(events & BLEWorkerSmpLtkRequest) {
            BLE_LOG_I("BLEWorkerSmpLtkRequest");
            ///TODO: Move this logic to ble_security module
            if(ble_worker_instance->pairing_info_available) {
                const rsi_bt_event_encryption_enabled_t* encrypt_keys =
                    ble_security_get_pairing_data(ble_worker_instance->security_data);

                status = rsi_ble_ltk_req_reply(
                    ble_worker_instance->remote_dev_address,
                    (1 | encrypt_keys->enabled | (encrypt_keys->sc_enable << 7)),
                    encrypt_keys->localltk);
                if(status != RSI_SUCCESS) {
                    BLE_LOG_W("ltk req reply cmd failed with reason = %lx", status);
                }
                BLE_LOG_I("Paired device");
            } else {
                BLE_LOG_I("Not paired device");
                rsi_ble_ltk_req_reply(ble_worker_instance->remote_dev_address, 0, NULL);
                if(status != RSI_SUCCESS) {
                    BLE_LOG_W("ltk negative req reply cmd failed with reason = %lx \n", status);
                }
            }
        }

        if(events & BLEWorkerSmpSecurityKeys) {
            BLE_LOG_I("BLEWorkerSmpSecurityKeys");
            do {
                if(!ble_security_rpa_enable(ble_worker_instance->security_data)) break;

                if(!ble_security_save_data(ble_worker_instance->security_data)) {
                    BLE_LOG_W("Failed to save Security");
                    break;
                }
                BLE_LOG_I("Security data saved");
            } while(false);
        }
    }

    return 0;
}

/**
 * @fn         ble_usart_echo_app_prepare_128bit_uuid
 * @brief      this function is used to prepare the 128bit UUID
 * @param[in]  temp_service,received 128-bit service.
 * @param[out] temp_uuid,formed 128-bit service structure.
 * @return     none.
 * @section description
 * This function prepares the 128bit UUID
 */
static void
    ble_worker_prepare_128bit_uuid(const uint8_t temp_service[UUID_SIZE], uuid_t* temp_uuid) {
    temp_uuid->val.val128.data1 =
        ((temp_service[0] << 24) | (temp_service[1] << 16) | (temp_service[2] << 8) |
         (temp_service[3]));
    temp_uuid->val.val128.data2 = ((temp_service[5]) | (temp_service[4] << 8));
    temp_uuid->val.val128.data3 = ((temp_service[7]) | (temp_service[6] << 8));
    temp_uuid->val.val128.data4[0] = temp_service[9];
    temp_uuid->val.val128.data4[1] = temp_service[8];
    temp_uuid->val.val128.data4[2] = temp_service[11];
    temp_uuid->val.val128.data4[3] = temp_service[10];
    temp_uuid->val.val128.data4[4] = temp_service[15];
    temp_uuid->val.val128.data4[5] = temp_service[14];
    temp_uuid->val.val128.data4[6] = temp_service[13];
    temp_uuid->val.val128.data4[7] = temp_service[12];
}

/**
 * @fn         ble_usart_echo_app_add_char_serv_att
 * @brief      this function is used to add characteristic service attribute..
 * @param[in]  serv_handler, service handler.
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  att_val_handle, characteristic value handle
 * @param[in]  att_val_uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used at application to add characteristic attribute
 */
static uint16_t ble_worker_add_char_serv_att(
    void* serv_handler,
    uint16_t handle,
    uint8_t val_prop,
    uint16_t att_val_handle,
    uuid_t att_val_uuid) {
    rsi_ble_req_add_att_t new_att = {0};

    //! preparing the attribute service structure
    new_att.serv_handler = serv_handler;
    new_att.handle = handle;
    new_att.att_uuid.size = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
    new_att.property = RSI_BLE_ATT_PROPERTY_READ;

    //! preparing the characteristic attribute value
    if(att_val_uuid.size == UUID_SIZE) {
        new_att.data_len = 4 + att_val_uuid.size;
        new_att.data[0] = val_prop;
        rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
        memcpy(&new_att.data[4], &att_val_uuid.val.val128, sizeof(att_val_uuid.val.val128));
    } else {
        new_att.data_len = 6;
        rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
        new_att.data[0] = val_prop;
        rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);
    }

    //! Add attribute to the service
    sl_status_t status = rsi_ble_add_attribute(&new_att);
    if(status != SL_STATUS_OK) {
        BLE_LOG_W("Status: %04lX", status);
    }

    return handle;
}

static uint16_t ble_worker_add_char_val_att(
    void* serv_handler,
    uint16_t handle,
    uuid_t att_type_uuid,
    uint8_t val_prop,
    const uint8_t* data,
    uint8_t data_len,
    uint8_t auth_read) {
    rsi_ble_req_add_att_t new_att = {0};

    //! preparing the attributes
    new_att.serv_handler = serv_handler;
    new_att.handle = handle;
    new_att.config_bitmap = auth_read;
    memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
    new_att.property = val_prop;

    //! preparing the attribute value
    new_att.data_len = RSI_MIN(sizeof(new_att.data), data_len);

    if(data != NULL) memcpy(new_att.data, data, new_att.data_len);

    //! add attribute to the service
    sl_status_t status = rsi_ble_add_attribute(&new_att);

    if(status != SL_STATUS_OK) {
        BLE_LOG_W("Status: %04lX", status);
    }

    //! check the attribute property with notification/Indication
    if((val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) || (val_prop & RSI_BLE_ATT_PROPERTY_INDICATE)) {
        //! if notification/indication property supports then we need to add client characteristic service.
        handle += 1;
        //! preparing the client characteristic attribute & values
        memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
        new_att.serv_handler = serv_handler;
        new_att.handle = handle;
        new_att.att_uuid.size = 2;
        new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
        new_att.property = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
        new_att.data_len = 2;
        new_att.config_bitmap = auth_read;

        //! add attribute to the service
        int32_t ret = rsi_ble_add_attribute(&new_att);
        BLE_LOG_D("Add CCCD handle: %04X, Ret: %lX", handle, ret);
        UNUSED(ret);
    }
    return handle;
}

static void ble_prepare_uuid(const Char_UUID_t* temp, const uint8_t size, uuid_t* uuid) {
    uuid->size = size;
    if(size == 2)
        uuid->val.val16 = temp->Char_UUID_16;
    else if(size == 16)
        ble_worker_prepare_128bit_uuid(temp->Char_UUID_128, uuid);
}

void ble_worker_init() {
    ble_worker_instance = malloc(sizeof(BleWorker));
    ble_worker_instance->state = BleWorkerStateIdle;
    ble_worker_instance->thread =
        furi_thread_alloc_ex("BleWorker", 2048, ble_worker_thread_callback, ble_worker_instance);

    ble_worker_instance->indication_sem = furi_semaphore_alloc(1, 0);
    ble_worker_instance->notification_sem = furi_semaphore_alloc(1, 1);
    ble_worker_instance->max_payload_size = BLE_WORKER_MAX_MTU_SIZE - BLE_WORKER_ATTR_HEADER_SIZE;
    ble_worker_instance->security_data = ble_security_alloc();
    ble_worker_instance->advertise = ble_advertise_alloc();
    ble_advertise_set_name(ble_worker_instance->advertise, BLE_DEFAULT_LOCAL_NAME);

    BleServiceEntryDict_init(ble_worker_instance->service_dict);

    ble_hw_config();

    //Appearance adjustment
    uuid_t uuid = {0};
    uuid.size = 2;
    uuid.val.val16 = 0x2A01;
    uint16_t value_handle = 0;
    if(ble_find_characteristic_value_handle_by_uiid(&uuid, 0x001E, &value_handle)) {
        uint16_t data = 0x00C0;
        BLE_LOG_D("Handle found: %04X", value_handle);
        sl_status_t status = rsi_ble_set_local_att_value(value_handle, 2, (uint8_t*)&data);
        UNUSED(status);
        BLE_LOG_D("Status: %lX", status);
    }
    //---------------------------------------
}

bool ble_worker_register_service(BleServiceObject* service) {
    uuid_t rsi_uiid = {0};
    rsi_ble_resp_add_serv_t new_serv_resp = {0};

    ble_prepare_uuid(&service->config->uuid, service->config->uuid_size, &rsi_uiid);
    sl_status_t status = rsi_ble_add_service(rsi_uiid, &new_serv_resp);

    bool result = false;
    if(status == RSI_SUCCESS) {
        ///TODO: it's not very good that worker knows inner kitchen of ble_service object
        /// Possibly need to close all of this parts behind some methods
        service->service_handler = new_serv_resp.serv_handler;
        service->handle = new_serv_resp.start_handle;

        uint16_t handle = new_serv_resp.start_handle;
        BLE_LOG_D("Register servive: 0x%04X", new_serv_resp.start_handle);
        for(uint8_t i = 0; i < service->config->char_count; i++) {
            BleCharacteristicObject* ch = service->chars[i];
            const BleCharacteristicDescriptor* ch_config = ble_characteristic_get_config(ch);

            memset(&rsi_uiid, 0, sizeof(uuid_t));
            ble_prepare_uuid(&ch_config->uuid, ch_config->uuid_size, &rsi_uiid);

            BLE_LOG_D("Add char %s att handle: %04X", ch_config->name, handle + 1);
            ble_worker_add_char_serv_att(
                service->service_handler,
                handle + 1,
                ch_config->char_properties,
                handle + 2,
                rsi_uiid);

            uint16_t value_handle = handle + 2;
            BLE_LOG_D("Add char %s val att handle: %04X", ch_config->name, value_handle);
            ble_characteristic_set_handle(ch, value_handle);
            handle = ble_worker_add_char_val_att(
                service->service_handler,
                value_handle,
                rsi_uiid,
                ch_config->char_properties,
                ble_characteristic_get_data(ch),
                ble_characteristic_get_data_size(ch),
                RSI_BLE_ATT_CONFIG_BITMAP);
            BleServiceEntry entry = {.service = service, .char_index = ch_config->intercom_index};
            BleServiceEntryDict_set_at(ble_worker_instance->service_dict, value_handle, entry);

            if((ch_config->char_properties & RSI_BLE_ATT_PROPERTY_NOTIFY) ||
               (ch_config->char_properties & RSI_BLE_ATT_PROPERTY_INDICATE)) {
                ble_characteristic_set_cccd_handle(ch, handle);
                BleServiceEntry entry = {
                    .service = service, .char_index = ch_config->intercom_index};
                BleServiceEntryDict_set_at(ble_worker_instance->service_dict, handle, entry);
            }
        }

        result = true;
    }

    return result;
}

void ble_worker_start() {
    do {
        if(ble_worker_instance->state != BleWorkerStateIdle) {
            BLE_LOG_W("BLE not in Idle state, skip advertise start");
            break;
        }

        const rsi_bt_event_le_security_keys_t* rpa =
            ble_security_get_rpa_data(ble_worker_instance->security_data);
        ble_worker_start_advertising(
            ble_worker_instance->pairing_info_available, rpa, ble_worker_instance->advertise);

        ble_worker_instance->state = BleWorkerStateAdvertising;
        furi_thread_start(ble_worker_instance->thread);
    } while(false);
}

void ble_worker_stop() {
    furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtExit);
    furi_thread_join(ble_worker_instance->thread);
    BLE_LOG_I("BLE Stopped");
}

void ble_worker_test_after_init() {
    ble_print_service_hierarchy(0x0023);
}

static void ble_worker_send_chunk(
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value) {
    sl_status_t status;
    BLE_LOG_D("Data_size: %d", data_size);

    if(ble_worker_instance->connected && BLE_CCCD_INDICATION_ENABLED(cccd_value)) {
        status = rsi_ble_indicate_value(
            ble_worker_instance->remote_dev_address, handle, data_size, data);

        if(furi_semaphore_acquire(ble_worker_instance->indication_sem, 1000) != FuriStatusOk) {
            furi_crash("Indication failed");
        }
    } else if(ble_worker_instance->connected && BLE_CCCD_NOTIFICATION_ENABLED(cccd_value)) {
        if(furi_semaphore_acquire(ble_worker_instance->notification_sem, 1000) != FuriStatusOk) {
            //furi_crash("Notification failed");
            BLE_LOG_W("Notification failed for %04X", handle);
        }
        status =
            rsi_ble_notify_value(ble_worker_instance->remote_dev_address, handle, data_size, data);
    } else {
        status = rsi_ble_set_local_att_value(handle, data_size, data);
    }

    if(status != 0) BLE_LOG_W("Send fail %08lX", status);
}

void ble_worker_send(uint16_t handle, uint16_t data_size, const uint8_t* data, uint16_t cccd_value) {
    size_t index = 0;
    size_t total_size = data_size;

    while(total_size) {
        size_t send_size = total_size > ble_worker_instance->max_payload_size ?
                               ble_worker_instance->max_payload_size :
                               total_size;
        ble_worker_send_chunk(handle, send_size, &data[index], cccd_value);
        index += send_size;
        total_size -= send_size;
    }
}

void ble_worker_receive_confirm(uint16_t handle, uint8_t cccd_value) {
    UNUSED(handle);
    sl_status_t status;
    if(ble_worker_instance->connected && BLE_CCCD_INDICATION_ENABLED(cccd_value)) {
        status = rsi_ble_indicate_confirm(ble_worker_instance->remote_dev_address);
    } else {
        status = rsi_ble_gatt_write_response(ble_worker_instance->remote_dev_address, 0);
    }

    if(status != 0) BLE_LOG_W("Recv fail %08lX", status);
}

bool ble_worker_forget_pairing() {
    if(ble_worker_instance->state == BleWorkerStateAdvertising) {
        ble_worker_stop_advertising();
    }

    ble_security_rpa_disable();

    bool result = ble_security_delete_data(ble_worker_instance->security_data);

    ble_worker_instance->pairing_info_available = 0;

    if(ble_worker_instance->state == BleWorkerStateAdvertising) {
        ble_worker_start_advertising(false, NULL, ble_worker_instance->advertise);
    }

    if(result) BLE_LOG_I("Security data removed");
    return result;
}
