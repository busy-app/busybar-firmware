#include "ble_usart_echo_app.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#include <args.h>
#include <strint.h>

#define TAG "BLE_USART_Echo_App"

#define BLE_USART_ECHO_APP_LOCAL_NAME        "BSB_Usart_Echo"
#define BLE_USART_ECHO_APP_MAX_MTU_SIZE      240
#define BLE_USART_ECHO_APP_MAX_SEND_DATA_LEN 232

#define BLE_USART_ECHO_APP_UUID_TYPE 2
#if(BLE_USART_ECHO_APP_UUID_TYPE == 1)
// Silabs Bluetooth UUIDs
// UART_SERVICE_UUID = "0000aabb-0000-1000-8000-0026bb765291"
// UART_TX_CHAR_UUID = "00001bb1-0000-1000-8000-00805f9b34fb"
// UART_RX_CHAR_UUID = "00001aa1-0000-1000-8000-00805f9b34fb"
#define UART_SERVICE_UUID \
    {0x00, 0x00, 0xAA, 0xBB, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x26, 0xBB, 0x76, 0x52, 0x91}
#define UART_TX_CHAR_UUID \
    {0x00, 0x00, 0x1B, 0xB1, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}
#define UART_RX_CHAR_UUID \
    {0x00, 0x00, 0x1A, 0xA1, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}

#elif(BLE_USART_ECHO_APP_UUID_TYPE == 2)
// Nordic UART Service UUIDs
// UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
// UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
// UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_SERVICE_UUID \
    {0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}
#define UART_RX_CHAR_UUID \
    {0x6E, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}
#define UART_TX_CHAR_UUID \
    {0x6E, 0x40, 0x00, 0x03, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E}

#elif(BLE_USART_ECHO_APP_UUID_TYPE == 3)
// HM-10 Bluetooth UUIDs
// UART_SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb"
// UART_TX_CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
// UART_RX_CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
#define UART_SERVICE_UUID \
    {0x00, 0x00, 0xFF, 0xE0, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}
#define UART_TX_CHAR_UUID \
    {0x00, 0x00, 0xFF, 0xE1, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}
#define UART_RX_CHAR_UUID \
    {0x00, 0x00, 0xFF, 0xE1, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}
#else
#error "Invalid BLE_USART_ECHO_APP_UUID_TYPE"
#endif

#define UUID_SIZE                                    16
//! error code
#define BLE_USART_ECHO_APP_BT_HCI_COMMAND_DISALLOWED 0x4E0C
#define BLE_USART_ECHO_APP_LOCAL_DEV_ADDR_LEN        18 // Length of the local device address

//! Configuration bitmap for attributes
#define RSI_BLE_ATT_MAINTAIN_IN_HOST BIT(0)
#define RSI_BLE_ATT_SECURITY_ENABLE  BIT(1)

#define RSI_BLE_ATT_CONFIG_BITMAP (RSI_BLE_ATT_MAINTAIN_IN_HOST)

//! application events list
typedef enum {
    BLEUsartEchoEvtExit = (1 << 0),
    BLEUsartEchoEvtAdvReport = (1 << 1),
    BLEUsartEchoEvtConnected = (1 << 2),
    BLEUsartEchoEvtDisconnected = (1 << 3),
    BLEUsartEchoEvtPhyUpdateComplete = (1 << 4),
    BLEUsartEchoEvtConnUpdate = (1 << 5),
    BLEUsartEchoEvtDataLengthChange = (1 << 6),

    BLEUsartEchoEvtReceveRemoteFeatures = (1 << 7),
    BLEUsartEchoEvtMoreDataReq = (1 << 8),

    BLEUsartEchoEvtWrite = (1 << 9),
    BLEUsartEchoEvtDataTransmit = (1 << 10),
    BLEUsartEchoEvtMtu = (1 << 11),

} BLEUsartEchoEvt;

#define BLE_USART_ECHO_ALL_EVENTS                                                                 \
    (BLEUsartEchoEvtExit | BLEUsartEchoEvtAdvReport | BLEUsartEchoEvtConnected |                  \
     BLEUsartEchoEvtDisconnected | BLEUsartEchoEvtPhyUpdateComplete | BLEUsartEchoEvtConnUpdate | \
     BLEUsartEchoEvtDataLengthChange | BLEUsartEchoEvtReceveRemoteFeatures |                      \
     BLEUsartEchoEvtMoreDataReq | BLEUsartEchoEvtWrite | BLEUsartEchoEvtDataTransmit |            \
     BLEUsartEchoEvtMtu)

static const sl_wifi_device_configuration_t config = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = US,
    .boot_config = {
        .oper_mode = SL_SI91X_CLIENT_MODE,
        .coex_mode = SL_SI91X_WLAN_BLE_MODE,
        .feature_bit_map =
            (SL_SI91X_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE |
             SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
        .tcp_ip_feature_bit_map =
            (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
        .custom_feature_bit_map = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),

        .ext_custom_feature_bit_map =
            (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG |
             SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0 |
             SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
        .bt_feature_bit_map = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
        .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
        //!ENABLE_BLE_PROTOCOL in bt_feature_bit_map
        .ble_feature_bit_map =
            ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS) |
              SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS) |
              SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV) |
              SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)) |
             SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID |
             SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX) |
             SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) |
             SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
             | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
             ),
        .ble_ext_feature_bit_map =
            ((SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS) |
              SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES))
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
             | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST //indication response from app
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
             | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST //MTU Exchange request initiation from app
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
             | (SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST) //Set SCAN Resp Data from app
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
             | (SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST) //Disable Coded PHY from app
#endif
#if RSI_BLE_GATT_INIT
             | SL_SI91X_BLE_GATT_INIT
#endif
             ),
        .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP)}};

typedef enum {
    BLEUsartEchoCmdTypeHelp,
    BLEUsartEchoCmdTypeHelpHelp,

    BLEUsartEchoCmdTypeMax,
} BLEUsartEchoCmdType;

typedef enum {
    BLEUsartEchoStateIdle,
} BLEUsartEchoState;

typedef struct {
    char* cmd;
} BLEUsartEchoCmd;

const BLEUsartEchoCmd ble_usart_echo_cmd[BLEUsartEchoCmdTypeMax] = {
    {"?"},
    {"help"},
};

struct BLEUsartEchoApp {
    FuriString* msg;
    CliWorker* worker;
    BLEUsartEchoState state;

    FuriThread* thread;

    uint16_t ble_att1_val_hndl;
    uint16_t ble_att2_val_hndl;

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

    bool exit;
};

static BLEUsartEchoApp* ble_usart_echo_app_instance = NULL;

static void ble_usart_echo_app_cmd_usage(BLEUsartEchoApp* instance);

static void ble_usart_echo_app_send_msg(BLEUsartEchoApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

void ble_usart_echo_app_send_text(BLEUsartEchoApp* instance, FuriString* text) {
    cli_worker_add_rx_data(
        instance->worker, (uint8_t*)furi_string_get_cstr(text), furi_string_utf8_length(text));
}

static void ble_usart_echo_app_send_msg_invalid_arg(BLEUsartEchoApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    ble_usart_echo_app_send_msg(instance);
}

/*==============================================*/

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
static void ble_usart_echo_app_add_char_serv_att(
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
        new_att.data[0] = val_prop;
        rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
        rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);
    }

    //! Add attribute to the service
    rsi_ble_add_attribute(&new_att);

    return;
}

static void ble_usart_echo_app_add_char_val_att(
    void* serv_handler,
    uint16_t handle,
    uuid_t att_type_uuid,
    uint8_t val_prop,
    uint8_t* data,
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
    if(data != NULL) memcpy(new_att.data, data, sizeof(new_att.data));

    new_att.data_len = RSI_MIN(sizeof(new_att.data), data_len);

    //! add attribute to the service
    rsi_ble_add_attribute(&new_att);

    //! check the attribute property with notification/Indication
    if((val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) || (val_prop & RSI_BLE_ATT_PROPERTY_INDICATE)) {
        //! if notification/indication property supports then we need to add client characteristic service.

        //! preparing the client characteristic attribute & values
        memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
        new_att.serv_handler = serv_handler;
        new_att.handle = handle + 1;
        new_att.att_uuid.size = 2;
        new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
        new_att.property = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
        new_att.data_len = 2;

        //! add attribute to the service
        rsi_ble_add_attribute(&new_att);
    }

    return;
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
    ble_usart_echo_app_prepare_128bit_uuid(uint8_t temp_service[UUID_SIZE], uuid_t* temp_uuid) {
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
 * @fn         ble_usart_echo_app_add_simple_chat_serv
 * @brief      this function is used to add new servcie i.e.., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */
static uint32_t ble_usart_echo_app_add_simple_chat_serv(BLEUsartEchoApp* instance) {
    uuid_t new_uuid = {0};
    rsi_ble_resp_add_serv_t new_serv_resp = {0};
    uint8_t data[RSI_BLE_MAX_DATA_LEN] = {"bsb_ble_sampletest"};

    //! adding new service
    uint8_t ble_serv[UUID_SIZE] = UART_SERVICE_UUID;
    new_uuid.size = UUID_SIZE;
    ble_usart_echo_app_prepare_128bit_uuid(ble_serv, &new_uuid);
    rsi_ble_add_service(new_uuid, &new_serv_resp);

    //! adding characteristic service attribute to the service
    uint8_t ble_rx_att[UUID_SIZE] = UART_RX_CHAR_UUID;
    new_uuid.size = UUID_SIZE;
    ble_usart_echo_app_prepare_128bit_uuid(ble_rx_att, &new_uuid);
    ble_usart_echo_app_add_char_serv_att(
        new_serv_resp.serv_handler,
        new_serv_resp.start_handle + 1,
        RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
        new_serv_resp.start_handle + 2,
        new_uuid);

    //! adding characteristic value attribute to the service
    ble_usart_echo_app_add_char_val_att(
        new_serv_resp.serv_handler,
        new_serv_resp.start_handle + 2,
        new_uuid,
        RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE,
        data,
        sizeof(data),
        RSI_BLE_ATT_CONFIG_BITMAP);

    instance->ble_att1_val_hndl = new_serv_resp.start_handle + 2;

    //! adding characteristic service attribute to the service
    uint8_t ble_tx_att[UUID_SIZE] = UART_TX_CHAR_UUID;
    new_uuid.size = UUID_SIZE;
    ble_usart_echo_app_prepare_128bit_uuid(ble_tx_att, &new_uuid);
    ble_usart_echo_app_add_char_serv_att(
        new_serv_resp.serv_handler,
        new_serv_resp.start_handle + 3,
        RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
        new_serv_resp.start_handle + 4,
        new_uuid);

    //! adding characteristic value attribute to the service
    ble_usart_echo_app_add_char_val_att(
        new_serv_resp.serv_handler,
        new_serv_resp.start_handle + 4,
        new_uuid,
        RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
        data,
        sizeof(data),
        0);

    instance->ble_att2_val_hndl = new_serv_resp.start_handle + 4;
    return 0;
}

/*==============================================*/
/**
 * @fn         ble_usart_echo_app_on_adv_report_event
 * @brief      invoked when advertise report event is received
 * @param[in]  adv_report, pointer to the received advertising report
 * @return     none.
 * @section description
 * This callback function updates the scanned remote devices list
 */
void ble_usart_echo_app_on_adv_report_event(rsi_ble_event_adv_report_t* adv_report) {
    if(ble_usart_echo_app_instance->device_found == 1) {
        return;
    }

    memset(
        &ble_usart_echo_app_instance->remote_name,
        0,
        sizeof(ble_usart_echo_app_instance->remote_name));
    BT_LE_ADPacketExtract(
        ble_usart_echo_app_instance->remote_name, adv_report->adv_data, adv_report->adv_data_len);

    ble_usart_echo_app_instance->remote_addr_type = adv_report->dev_addr_type;
    rsi_6byte_dev_address_to_ascii(
        ble_usart_echo_app_instance->remote_dev_str_addr, (uint8_t*)adv_report->dev_addr);
    memcpy(
        (int8_t*)ble_usart_echo_app_instance->remote_dev_bd_addr,
        (uint8_t*)adv_report->dev_addr,
        6);

#if(CONNECT_OPTION == CONN_BY_NAME)
    if((ble_usart_echo_app_instance->device_found == 0) &&
       ((strcmp((const char*)ble_usart_echo_app_instance->remote_name, RSI_REMOTE_DEVICE_NAME)) ==
        0)) {
        ble_usart_echo_app_instance->device_found = 1;

        furi_thread_flags_set(
            furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtAdvReport);
        return;
    }
#elif(CONNECT_OPTION == CONN_BY_ADDR)
    if((!strcmp(
           RSI_BLE_REMOTE_DEV_ADDR, (char*)ble_usart_echo_app_instance->remote_dev_str_addr))) {
        ble_usart_echo_app_instance->device_found = 1;
        furi_thread_flags_set(
            furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtAdvReport);
    }
#endif

    return;
}

/*==============================================*/
/**
 * @fn         ble_usart_echo_app_on_connect_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void ble_usart_echo_app_on_connect_event(rsi_ble_event_conn_status_t* resp_conn) {
    memcpy(ble_usart_echo_app_instance->remote_dev_address, resp_conn->dev_addr, 6);
    rsi_6byte_dev_address_to_ascii(
        ble_usart_echo_app_instance->str_remote_address, resp_conn->dev_addr);
    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtConnected);
}

/**
 * @fn         ble_usart_echo_app_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void ble_usart_echo_app_on_disconnect_event(
    rsi_ble_event_disconnect_t* resp_disconnect,
    uint16_t reason) {
    UNUSED(
        reason); //This statement is added only to resolve compilation warning, value is unchanged
    memcpy(ble_usart_echo_app_instance->remote_dev_address, resp_disconnect->dev_addr, 6);
    rsi_6byte_dev_address_to_ascii(
        ble_usart_echo_app_instance->str_remote_address, resp_disconnect->dev_addr);

    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtDisconnected);
}

/**
 * @fn         ble_usart_echo_app_phy_update_complete_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
void ble_usart_echo_app_phy_update_complete_event(
    rsi_ble_event_phy_update_t* rsi_ble_event_phy_update_complete) {
    memcpy(
        &ble_usart_echo_app_instance->app_phy_update_complete,
        rsi_ble_event_phy_update_complete,
        sizeof(rsi_ble_event_phy_update_t));
    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtPhyUpdateComplete);
}

/**
 * @fn         ble_usart_echo_app_data_length_change_event
 * @brief      invoked when data length is set
 * @section description
 * This Callback function indicates data length is set
 */
void ble_usart_echo_app_data_length_change_event(
    rsi_ble_event_data_length_update_t* rsi_ble_data_length_update) {
    memcpy(
        &ble_usart_echo_app_instance->data_length_update,
        rsi_ble_data_length_update,
        sizeof(rsi_ble_event_data_length_update_t));

    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtDataLengthChange);
}

/**
 * @fn         ble_usart_echo_app_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void ble_usart_echo_app_on_enhance_conn_status_event(
    rsi_ble_event_enhance_conn_status_t* resp_enh_conn) {
    memcpy(ble_usart_echo_app_instance->remote_dev_address, resp_enh_conn->dev_addr, 6);
    rsi_6byte_dev_address_to_ascii(
        ble_usart_echo_app_instance->str_remote_address, resp_enh_conn->dev_addr);
    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtConnected);
}

void ble_usart_echo_app_on_conn_update_complete_event(
    rsi_ble_event_conn_update_t* rsi_ble_event_conn_update_complete,
    uint16_t resp_status) {
    UNUSED(resp_status);
    memcpy(
        &ble_usart_echo_app_instance->event_conn_update_complete,
        rsi_ble_event_conn_update_complete,
        sizeof(rsi_ble_event_conn_update_t));
    memcpy(
        ble_usart_echo_app_instance->remote_dev_address,
        rsi_ble_event_conn_update_complete->dev_addr,
        6);

    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtConnUpdate);
}
/*==============================================*/
/**
 * @fn         ble_usart_echo_app_simple_peripheral_on_remote_features_event
 * @brief      invoked when LE remote features event is received.
 * @param[in] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void ble_usart_echo_app_simple_peripheral_on_remote_features_event(
    rsi_ble_event_remote_features_t* rsi_ble_event_remote_features) {
    memcpy(
        &ble_usart_echo_app_instance->remote_dev_feature,
        rsi_ble_event_remote_features,
        sizeof(rsi_ble_event_remote_features_t));
    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread),
        BLEUsartEchoEvtReceveRemoteFeatures);
}

static void
    ble_usart_echo_app_more_data_req_event(rsi_ble_event_le_dev_buf_ind_t* rsi_ble_more_data_evt) {
    UNUSED(rsi_ble_more_data_evt);

    //! set conn specific event
    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtMoreDataReq);

    return;
}

/*==============================================*/
/**
 * @fn         ble_usart_echo_app_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void ble_usart_echo_app_on_gatt_write_event(
    uint16_t event_id,
    rsi_ble_event_write_t* rsi_ble_write) {
    UNUSED(event_id);

    memcpy(
        &ble_usart_echo_app_instance->app_ble_write_event,
        rsi_ble_write,
        sizeof(rsi_ble_event_write_t));
    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtWrite);
}

/**
 * @fn         ble_usart_echo_app_on_mtu_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void ble_usart_echo_app_on_mtu_event(rsi_ble_event_mtu_t* rsi_ble_mtu) {
    memcpy(
        &ble_usart_echo_app_instance->app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
    rsi_6byte_dev_address_to_ascii(
        ble_usart_echo_app_instance->str_remote_address,
        ble_usart_echo_app_instance->app_ble_mtu_event.dev_addr);

    furi_thread_flags_set(
        furi_thread_get_id(ble_usart_echo_app_instance->thread), BLEUsartEchoEvtMtu);
}

/*==============================================*/

static int32_t ble_usart_echo_app_thread_callback(void* context) {
    BLEUsartEchoApp* instance = (BLEUsartEchoApp*)context;

    int32_t status = 0;
    uint8_t adv[31] = {2, 1, 6};
    sl_wifi_firmware_version_t version = {0};
    static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = {0};
    uint8_t local_dev_addr[BLE_USART_ECHO_APP_LOCAL_DEV_ADDR_LEN] = {0};

    //! Wi-Fi initialization
    status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
    if(status != SL_STATUS_OK) {
        furi_string_printf(
            instance->msg, "Wi-Fi Initialization Failed, Error Code : 0x0x%08lx\r\n", status);
        ble_usart_echo_app_send_msg(instance);
        furi_crash();
    }
    furi_string_printf(instance->msg, "Wireless Initialization Success\r\n");
    ble_usart_echo_app_send_msg(instance);

    //! Firmware version Prints
    status = sl_wifi_get_firmware_version(&version);
    if(status != SL_STATUS_OK) {
        furi_string_printf(
            instance->msg, "Firmware version Failed, Error Code : 0x0x%08lx\r\n", status);
        ble_usart_echo_app_send_msg(instance);
    } else {
        furi_string_printf(
            instance->msg,
            "Firmware version is: %x%x.%d.%d.%d.%d.%d.%d\r\n",
            version.chip_id,
            version.rom_id,
            version.major,
            version.minor,
            version.security_version,
            version.patch_num,
            version.customer_id,
            version.build_num);
        ble_usart_echo_app_send_msg(instance);
    }

    //! get the local device MAC address.
    status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
    if(status != RSI_SUCCESS) {
        furi_string_printf(instance->msg, "Get local device address failed = 0x%08lx\r\n", status);
        ble_usart_echo_app_send_msg(instance);
        furi_crash();
    } else {
        rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
        furi_string_printf(instance->msg, "Local device address %s \r\n", local_dev_addr);
        ble_usart_echo_app_send_msg(instance);
    }

    //! registering the GAP callback functions
    rsi_ble_gap_register_callbacks(
        ble_usart_echo_app_on_adv_report_event,
        ble_usart_echo_app_on_connect_event,
        ble_usart_echo_app_on_disconnect_event,
        NULL,
        ble_usart_echo_app_phy_update_complete_event,
        ble_usart_echo_app_data_length_change_event,
        ble_usart_echo_app_on_enhance_conn_status_event,
        NULL,
        ble_usart_echo_app_on_conn_update_complete_event,
        NULL);

    rsi_ble_gap_extended_register_callbacks(
        ble_usart_echo_app_simple_peripheral_on_remote_features_event,
        ble_usart_echo_app_more_data_req_event);

    rsi_ble_gatt_register_callbacks(
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ble_usart_echo_app_on_gatt_write_event,
        NULL,
        NULL,
        NULL,
        ble_usart_echo_app_on_mtu_event,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    ble_usart_echo_app_add_simple_chat_serv(instance);

    //! Set local name
    status = rsi_bt_set_local_name((uint8_t*)BLE_USART_ECHO_APP_LOCAL_NAME);
    if(status != RSI_SUCCESS) {
        furi_string_printf(
            instance->msg, "Failed to set local name, error code : 0x%08lx\r\n", status);
        ble_usart_echo_app_send_msg(instance);
        furi_crash();
    }

    //! prepare advertise data //local/device name
    adv[3] = strlen(BLE_USART_ECHO_APP_LOCAL_NAME) + 1;
    adv[4] = 9;
    strcpy((char*)&adv[5], BLE_USART_ECHO_APP_LOCAL_NAME);

    //! set advertise data
    rsi_ble_set_advertise_data(adv, strlen(BLE_USART_ECHO_APP_LOCAL_NAME) + 5);

    //! start advertising
    status = rsi_ble_start_advertising();
    if(status != RSI_SUCCESS) {
        furi_string_printf(
            instance->msg, "Failed to start advertising, error code : 0x%08lx\r\n", status);
        ble_usart_echo_app_send_msg(instance);
    } else {
        furi_string_printf(
            instance->msg,
            "Start advertising ... Local Name : %s\r\n",
            BLE_USART_ECHO_APP_LOCAL_NAME);
        ble_usart_echo_app_send_msg(instance);
    }

    FURI_LOG_D(TAG, "Worker Start");
    while(!instance->exit) {
        uint32_t events =
            furi_thread_flags_wait(BLE_USART_ECHO_ALL_EVENTS, FuriFlagWaitAny, FuriWaitForever);

        if(events & BLEUsartEchoEvtConnected) {
            //! event invokes when connection was completed
            furi_string_printf(
                instance->msg,
                "Connected, str_remote_address : %s\r\n",
                instance->str_remote_address);
            ble_usart_echo_app_send_msg(instance);

            //! Setting MTU Exchange event
            status = rsi_ble_mtu_exchange_event(
                instance->remote_dev_address, BLE_USART_ECHO_APP_MAX_MTU_SIZE);
            if(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg,
                    "MTU request cmd failed with error code = 0x%08lx \r\n",
                    status);
                ble_usart_echo_app_send_msg(instance);
            }

            if(!instance->conn_params_updated) {
                status = rsi_ble_conn_params_update(
                    instance->remote_dev_address,
                    CONN_INTERVAL_MIN,
                    CONN_INTERVAL_MAX,
                    CONN_LATENCY,
                    SUPERVISION_TIMEOUT);
                if(status != RSI_SUCCESS) {
                    furi_string_printf(
                        instance->msg,
                        "Failed to update connection parameters, error code : 0x%08lx\r\n",
                        status);
                    ble_usart_echo_app_send_msg(instance);
                    furi_crash();
                }
            }
        }

        if(events & BLEUsartEchoEvtDisconnected) {
            //! event invokes when disconnection was completed
            furi_string_printf(
                instance->msg,
                "Disconnected, str_remote_address : %s\r\n",
                instance->str_remote_address);
            ble_usart_echo_app_send_msg(instance);

            instance->device_found = 0;
            instance->conn_params_updated = 0;

            //! start advertising
            status = rsi_ble_start_advertising();
            if(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg,
                    "Failed to start advertising, error code : 0x%08lx\r\n",
                    status);
                ble_usart_echo_app_send_msg(instance);
            } else {
                furi_string_printf(
                    instance->msg,
                    "Start advertising ... Local Name : %s\r\n",
                    BLE_USART_ECHO_APP_LOCAL_NAME);
                ble_usart_echo_app_send_msg(instance);
            }
        }

        if(events & BLEUsartEchoEvtReceveRemoteFeatures) {
            //! event invokes when remote features were received
            furi_string_printf(
                instance->msg,
                "Feature received is 0x%x \r\n",
                *(uint8_t*)instance->remote_dev_feature.remote_features);
            ble_usart_echo_app_send_msg(instance);

            if(instance->remote_dev_feature.remote_features[0] & 0x20) {
                status = rsi_ble_set_data_len(instance->remote_dev_address, TX_LEN, TX_TIME);
                if(status != RSI_SUCCESS) {
                    furi_string_printf(
                        instance->msg,
                        "Failed to set data length, error code : 0x%08lx\r\n",
                        status);
                    ble_usart_echo_app_send_msg(instance);

                    furi_thread_flags_set(
                        furi_thread_get_id(ble_usart_echo_app_instance->thread),
                        BLEUsartEchoEvtReceveRemoteFeatures);
                }

            } else if(instance->remote_dev_feature.remote_features[1] & 0x01) {
                status = rsi_ble_setphy(
                    (int8_t*)instance->remote_dev_address,
                    TX_PHY_RATE,
                    RX_PHY_RATE,
                    CODDED_PHY_RATE);
                if(status != RSI_SUCCESS) {
                    if(status != BLE_USART_ECHO_APP_BT_HCI_COMMAND_DISALLOWED) {
                        //retry the same command
                        furi_thread_flags_set(
                            furi_thread_get_id(ble_usart_echo_app_instance->thread),
                            BLEUsartEchoEvtDataLengthChange);
                    } else {
                        furi_string_printf(
                            instance->msg, "Failed to set phy, error code : 0x%08lx\r\n", status);
                        ble_usart_echo_app_send_msg(instance);
                    }
                }
            }
        }

        if(events & BLEUsartEchoEvtDataLengthChange) {
            furi_string_printf(
                instance->msg, "Max_tx_octets: %d \r\n", instance->data_length_update.MaxTxOctets);
            furi_string_cat_printf(
                instance->msg, "Max_tx_time: %d \r\n", instance->data_length_update.MaxTxTime);
            furi_string_cat_printf(
                instance->msg, "Max_rx_octets: %d \r\n", instance->data_length_update.MaxRxOctets);
            furi_string_cat_printf(
                instance->msg, "Max_rx_time: %d \r\n", instance->data_length_update.MaxRxTime);
            ble_usart_echo_app_send_msg(instance);

            if(instance->remote_dev_feature.remote_features[1] & 0x01) {
                osDelay(500);
                status = rsi_ble_setphy(
                    (int8_t*)instance->remote_dev_address,
                    TX_PHY_RATE,
                    RX_PHY_RATE,
                    CODDED_PHY_RATE);
                if(status != RSI_SUCCESS) {
                    if(status != BLE_USART_ECHO_APP_BT_HCI_COMMAND_DISALLOWED) {
                        //retry the same command
                        furi_thread_flags_set(
                            furi_thread_get_id(ble_usart_echo_app_instance->thread),
                            BLEUsartEchoEvtDataLengthChange);
                    } else {
                        furi_string_printf(
                            instance->msg, "Failed to set phy, error code : 0x%08lx\r\n", status);
                        ble_usart_echo_app_send_msg(instance);
                    }
                }
            }
        }

        if(events & BLEUsartEchoEvtPhyUpdateComplete) {
            //! phy update complete event
            furi_string_printf(
                instance->msg,
                "Tx Phy rate = 0x%x  and Rx Phy rate = 0x%x \r\n",
                instance->app_phy_update_complete.TxPhy,
                instance->app_phy_update_complete.RxPhy);
            ble_usart_echo_app_send_msg(instance);
        }

        if(events & BLEUsartEchoEvtConnUpdate) {
            furi_string_printf(
                instance->msg,
                "Connection parameters update completed \r\n Connection interval = %d, Latency = %d, Supervision Timeout = %d \r\n",
                instance->event_conn_update_complete.conn_interval,
                instance->event_conn_update_complete.conn_latency,
                instance->event_conn_update_complete.timeout);
            ble_usart_echo_app_send_msg(instance);
        }

        if(events & BLEUsartEchoEvtMtu) {
            //! event invokes when write/notification events received
            furi_string_printf(
                instance->msg,
                "MTU size received from remote device(%s) is %u\r\n",
                instance->str_remote_address,
                instance->app_ble_mtu_event.mtu_size);
            ble_usart_echo_app_send_msg(instance);

            status = rsi_ble_set_wo_resp_notify_buf_info(
                instance->remote_dev_address, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
            if(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg,
                    "Failed to set the buffer configuration mode, error: 0x%08lx \r\n",
                    status);
                ble_usart_echo_app_send_msg(instance);
            } else {
                furi_string_printf(
                    instance->msg,
                    "Buffer configuration done for notify and set_att cmds buf mode = %d , max buff count =%d \r\n",
                    DLE_BUFFER_MODE,
                    DLE_BUFFER_COUNT);
                ble_usart_echo_app_send_msg(instance);
            }
        }

        if(events & BLEUsartEchoEvtWrite) {
            //! event invokes when write/notification events receive
            furi_string_printf(
                instance->msg,
                "Received packet type = %u\r\n",
                instance->app_ble_write_event.pkt_type);
            ble_usart_echo_app_send_msg(instance);

            //TO DO: send ERR or write response
            if((*(uint16_t*)instance->app_ble_write_event.handle) == instance->ble_att1_val_hndl) {
                furi_string_printf(
                    instance->msg,
                    "Received data: %s\r\n",
                    instance->app_ble_write_event.att_value);
                ble_usart_echo_app_send_msg(instance);

                rsi_ble_gatt_write_response(instance->remote_dev_address, 0);

                //Todo: if need send Error response
                //rsi_ble_att_error_response(instance->remote_dev_address,
                //    *(uint16_t *)instance->app_ble_write_event.handle,
                //    opcode,
                //    err);

                // Send notification to remote device
                rsi_ble_notify_value(
                    instance->remote_dev_address,
                    instance->ble_att2_val_hndl,
                    instance->app_ble_write_event.length,
                    instance->app_ble_write_event.att_value);
            } else {
                rsi_ble_gatt_write_response(instance->remote_dev_address, 0);
            }
        }

        if(events & BLEUsartEchoEvtExit) {
            break;
        }
    }
    sl_wifi_deinit();
    FURI_LOG_D(TAG, "Worker Stop");
    return 0;
}

void* ble_usart_echo_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");

    ble_usart_echo_app_instance = malloc(sizeof(BLEUsartEchoApp));
    ble_usart_echo_app_instance->msg = furi_string_alloc();
    ble_usart_echo_app_instance->worker = worker;

    ble_usart_echo_app_instance->exit = false;
    ble_usart_echo_app_instance->thread = furi_thread_alloc_ex(
        "BLEUssartEchoAppWorker",
        2048,
        ble_usart_echo_app_thread_callback,
        ble_usart_echo_app_instance);
    furi_thread_start(ble_usart_echo_app_instance->thread);

    ble_usart_echo_app_instance->state = BLEUsartEchoStateIdle;

    ble_usart_echo_app_cmd_usage(ble_usart_echo_app_instance);
    return (void*)ble_usart_echo_app_instance;
}

void ble_usart_echo_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    BLEUsartEchoApp* instance = (BLEUsartEchoApp*)app_handle;

    if(instance) {
        instance->exit = true;
        furi_thread_flags_set(furi_thread_get_id(instance->thread), BLEUsartEchoEvtExit);
        furi_thread_join(instance->thread);
        furi_thread_free(instance->thread);

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
}

static sl_status_t
    ble_usart_echo_app(BLEUsartEchoApp* instance, uint8_t cmd_index, FuriString* args) {
    char* args_cstr = (char*)furi_string_get_cstr(args);
    UNUSED(args_cstr);
    FuriString* arg = furi_string_alloc();

    switch(cmd_index) {
    case BLEUsartEchoCmdTypeHelp:
    case BLEUsartEchoCmdTypeHelpHelp:
        ble_usart_echo_app_cmd_usage(instance);
        break;

    default:
        ble_usart_echo_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void ble_usart_echo_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    BLEUsartEchoApp* instance = (BLEUsartEchoApp*)app_handle;
    uint8_t i = 0;
    uint8_t cmd_index = 0;
    bool cmd_valid = false;

    FuriString* args = furi_string_alloc();
    furi_string_set_strn(args, (const char*)data, size);
    FuriString* cmd = furi_string_alloc();

    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(args));

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            break;
        }

        for(i = 0; i < BLEUsartEchoCmdTypeMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)ble_usart_echo_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(ble_usart_echo_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                ble_usart_echo_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            ble_usart_echo_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void ble_usart_echo_app_cmd_usage(BLEUsartEchoApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", "BLE USART Echo");
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");

    furi_string_cat_printf(
        instance->msg,
        "This application emulates an BLE USART named \"%s\" with the ability to connect to it\r\n",
        BLE_USART_ECHO_APP_LOCAL_NAME);
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    ble_usart_echo_app_send_msg(instance);
}
