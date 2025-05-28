#include "ble_ibeacon.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#include <cli/args.h>
#include <cli/cli_command.h>
#include <cli/shell/cli_shell.h>
#include <strint.h>

#define TAG "BLEiBeaconApp"

#define BLE_IBEACON_APP_LOCAL_NAME "iBeaconBSB"

//! application events list
typedef enum {
    BLEiBeaconEvtConnected = (1 << 0),
    BLEiBeaconEvtDisconnected = (1 << 1),
    BLEiBeaconEvtExit = (1 << 2),
} BLEiBeaconEvt;

#define BLE_IBEACON_ALL_EVENTS \
    (BLEiBeaconEvtConnected | BLEiBeaconEvtDisconnected | BLEiBeaconEvtExit)

#define LOCAL_DEV_ADDR_LEN 18 // Length of the local device address

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
            (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
             | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
             | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
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
#if BLE_SIMPLE_GATT
             | SL_SI91X_BLE_GATT_INIT
#endif
             ),
        .config_feature_bit_map =
            (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_SI91X_ENABLE_ENHANCED_MAX_PSP)}};

typedef enum {
    BLETestStateIdle,
} BLETestState;

typedef struct {
    BLETestState state;

    FuriString* msg;
    CliShell* shell;
    FuriThread* thread;

    rsi_bt_resp_get_local_name_t rsi_app_resp_get_local_name;
    uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN];
    uint8_t rsi_app_resp_device_state;
    int8_t rsi_app_resp_rssi;
    rsi_ble_event_conn_status_t rsi_app_connected_device;
    rsi_ble_event_disconnect_t rsi_app_disconnected_device;
    uint8_t str_remote_address[18];
    bool exit;
} BLETiBeaconApp;

static BLETiBeaconApp* ble_ibeacon_app_instance = NULL;

/*==============================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_conn_status_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_simple_peripheral_on_conn_status_event(rsi_ble_event_conn_status_t* resp_conn) {
    memcpy(
        &ble_ibeacon_app_instance->rsi_app_connected_device,
        resp_conn,
        sizeof(rsi_ble_event_conn_status_t));
    furi_thread_flags_set(
        furi_thread_get_id(ble_ibeacon_app_instance->thread), BLEiBeaconEvtConnected);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
void rsi_ble_simple_peripheral_on_disconnect_event(
    rsi_ble_event_disconnect_t* resp_disconnect,
    uint16_t reason) {
    UNUSED(
        reason); //This statement is added only to resolve compilation warning, value is unchanged
    memcpy(
        &ble_ibeacon_app_instance->rsi_app_disconnected_device,
        resp_disconnect,
        sizeof(rsi_ble_event_disconnect_t));
    furi_thread_flags_set(
        furi_thread_get_id(ble_ibeacon_app_instance->thread), BLEiBeaconEvtDisconnected);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void rsi_ble_simple_peripheral_on_enhance_conn_status_event(
    rsi_ble_event_enhance_conn_status_t* resp_enh_conn) {
    ble_ibeacon_app_instance->rsi_app_connected_device.dev_addr_type =
        resp_enh_conn->dev_addr_type;
    memcpy(
        ble_ibeacon_app_instance->rsi_app_connected_device.dev_addr,
        resp_enh_conn->dev_addr,
        RSI_DEV_ADDR_LEN);
    ble_ibeacon_app_instance->rsi_app_connected_device.status = resp_enh_conn->status;
    furi_thread_flags_set(
        furi_thread_get_id(ble_ibeacon_app_instance->thread), BLEiBeaconEvtConnected);
}

/*==============================================*/
/**
 * @fn         ble_ibeacon
 * @brief      Tests the BLE GAP peripheral role.
 * @param[in]  none
  * @return    none.
 * @section description
 * ibeacon header:
 * ------------------------------------------------------------------------------------------------
 *| pre header: 9bytes | uuid: 16 bytes | major_num: 2bytes | minor_num: 2bytes | tx_power:  1byte |
 * ------------------------------------------------------------------------------------------------
 * This function is used to test the BLE peripheral role and simple GAP API's.
 */
static int32_t ble_ibeacon_app_thread_callback(void* context) {
    BLETiBeaconApp* instance = (BLETiBeaconApp*)context;

    //int32_t temp_event_map = 0;
    uint8_t scan_data[31] = {2, 1, 6};
    uint8_t adv[31] = {0x02, 0x01, 0x02, 0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15}; //prefix(9bytes)
    uint8_t uuid[16] = {
        0xFB,
        0x0B,
        0x57,
        0xA2,
        0x82,
        0x28,
        0x44,
        0xCD,
        0x91,
        0x3A,
        0x94,
        0xA1,
        0x22,
        0xBA,
        0x12,
        0x06};
    uint8_t major_num[2] = {0x11, 0x22};
    uint8_t minor_num[2] = {0x33, 0x44};
    uint8_t tx_power = 0x33;
    sl_status_t status;
    sl_wifi_firmware_version_t version = {0};
    uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN] = {0};

    //! Wi-Fi initialization
    status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
    if(status != SL_STATUS_OK) {
        furi_string_printf(
            instance->msg, "Wi-Fi Initialization Failed, Error Code : 0x%lX", status);
        cli_shell_notification_print(instance->shell, instance->msg);
        furi_crash();
    }
    furi_string_printf(instance->msg, "Wi-Fi coex mode initialization is successful");
    cli_shell_notification_print(instance->shell, instance->msg);

    //! Firmware version Prints
    status = sl_wifi_get_firmware_version(&version);
    if(status != SL_STATUS_OK) {
        furi_string_printf(
            instance->msg, "Firmware version Failed, Error Code : 0x%lX", status);
        cli_shell_notification_print(instance->shell, instance->msg);
    } else {
        furi_string_printf(
            instance->msg,
            "Firmware version is: %x%x.%d.%d.%d.%d.%d.%d",
            version.chip_id,
            version.rom_id,
            version.major,
            version.minor,
            version.security_version,
            version.patch_num,
            version.customer_id,
            version.build_num);
        cli_shell_notification_print(instance->shell, instance->msg);
    }

    //! BLE register GAP callbacks
    rsi_ble_gap_register_callbacks(
        NULL,
        rsi_ble_simple_peripheral_on_conn_status_event,
        rsi_ble_simple_peripheral_on_disconnect_event,
        NULL,
        NULL,
        NULL,
        rsi_ble_simple_peripheral_on_enhance_conn_status_event,
        NULL,
        NULL,
        NULL);

    //! get the local device MAC address.
    status = rsi_bt_get_local_device_address(instance->rsi_app_resp_get_dev_addr);
    if(status != RSI_SUCCESS) {
        furi_string_printf(instance->msg, "Get local device address failed = %lx", status);
        cli_shell_notification_print(instance->shell, instance->msg);
        furi_crash();
    } else {
        rsi_6byte_dev_address_to_ascii(local_dev_addr, instance->rsi_app_resp_get_dev_addr);
        furi_string_printf(instance->msg, "Local device address %s", local_dev_addr);
        cli_shell_notification_print(instance->shell, instance->msg);
    }

    //! set the local device name
    status = rsi_bt_set_local_name((uint8_t*)BLE_IBEACON_APP_LOCAL_NAME);
    if(status != RSI_SUCCESS) {
        furi_string_printf(
            instance->msg, "Failed to set local name, error code : %lx", status);
        cli_shell_notification_print(instance->shell, instance->msg);
        furi_crash();
    }

    //! get the local device name
    status = rsi_bt_get_local_name(&instance->rsi_app_resp_get_local_name);
    if(status != RSI_SUCCESS) {
        furi_string_printf(
            instance->msg, "Failed to get local name, error code : %lx", status);
        cli_shell_notification_print(instance->shell, instance->msg);
        furi_crash();
    }

    furi_string_printf(
        instance->msg, "Local name set to: %s", instance->rsi_app_resp_get_local_name.name);
    cli_shell_notification_print(instance->shell, instance->msg);

    //! memcpy the uuid value
    memcpy(&adv[9], uuid, 16);
    //! memcpy the major_number value
    memcpy(&adv[9 + 16], major_num, 2);
    //! memcpy the minor_number value
    memcpy(&adv[9 + 16 + 2], minor_num, 2);
    //! memcpy the minor_number value
    adv[9 + 16 + 2 + 2] = tx_power;

    furi_string_printf(instance->msg, "Start advertising ...");
    cli_shell_notification_print(instance->shell, instance->msg);
    //! set advertise data
    rsi_ble_set_advertise_data(adv, 30);
    if(status != RSI_SUCCESS) {
        furi_string_printf(
            instance->msg, "Set Advertise Data Failed, error code : %lx", status);
        cli_shell_notification_print(instance->shell, instance->msg);
        furi_crash();
    } else {
        furi_string_printf(instance->msg, "Set Advertise Data Success");
        cli_shell_notification_print(instance->shell, instance->msg);
    }
    scan_data[3] = strlen(BLE_IBEACON_APP_LOCAL_NAME) + 1;
    scan_data[4] = 9;
    strcpy((char*)&scan_data[5], BLE_IBEACON_APP_LOCAL_NAME);
    status = rsi_ble_set_scan_response_data(scan_data, 31);
    if(status != RSI_SUCCESS) {
        furi_string_printf(
            instance->msg, "Set Scan Response Data Failed, error code : %lx", status);
        cli_shell_notification_print(instance->shell, instance->msg);
        furi_crash();
    } else {
        furi_string_printf(instance->msg, "Set Scan Response Data Success");
        cli_shell_notification_print(instance->shell, instance->msg);
    }

    //! start the advertising
    status = rsi_ble_start_advertising();
    if(status != RSI_SUCCESS) {
        furi_string_printf(
            instance->msg, "Failed to start advertising, error code : %lx", status);
        cli_shell_notification_print(instance->shell, instance->msg);
        furi_crash();
    } else {
        furi_string_printf(instance->msg, "Start advertising ...");
        cli_shell_notification_print(instance->shell, instance->msg);
    }

    FURI_LOG_D(TAG, "Worker Start");
    while(!instance->exit) {
        uint32_t events =
            furi_thread_flags_wait(BLE_IBEACON_ALL_EVENTS, FuriFlagWaitAny, FuriWaitForever);

        if(events & BLEiBeaconEvtConnected) {
            rsi_6byte_dev_address_to_ascii(
                instance->str_remote_address, instance->rsi_app_connected_device.dev_addr);
            //! get the RSSI value with connected remote device
            status = rsi_bt_get_rssi(
                (uint8_t*)instance->rsi_app_connected_device.dev_addr,
                &instance->rsi_app_resp_rssi);
            if(status != RSI_SUCCESS) {
                furi_crash();
            }

            furi_string_printf(
                instance->msg,
                "Connected to address : %s RSSI : %d",
                instance->str_remote_address,
                instance->rsi_app_resp_rssi);
            cli_shell_notification_print(instance->shell, instance->msg);
        }

        if(events & BLEiBeaconEvtDisconnected) {
            furi_string_printf(instance->msg, "Disconnected");
            cli_shell_notification_print(instance->shell, instance->msg);
            //! get the local device state.
            status = rsi_ble_get_device_state(&instance->rsi_app_resp_device_state);
            if(status != RSI_SUCCESS) {
                furi_crash();
            }

            //! set device in advertising mode.
            status = rsi_ble_start_advertising();
            while(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg, "Failed to start advertising, error code : %lx", status);
                cli_shell_notification_print(instance->shell, instance->msg);
                status = rsi_ble_start_advertising();
            };
            furi_string_printf(instance->msg, "Start advertising ...");
            cli_shell_notification_print(instance->shell, instance->msg);
        }

        if(events & BLEiBeaconEvtExit) {
            break;
        }
    }
    sl_wifi_deinit();
    FURI_LOG_D(TAG, "Worker Stop");
    return 0;
}

static void* ble_ibeacon_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");

    ble_ibeacon_app_instance = malloc(sizeof(BLETiBeaconApp));
    ble_ibeacon_app_instance->msg = furi_string_alloc();
    ble_ibeacon_app_instance->shell = shell;

    ble_ibeacon_app_instance->exit = false;
    ble_ibeacon_app_instance->thread = furi_thread_alloc_ex(
        "BLEiBeaconAppWorker", 2048, ble_ibeacon_app_thread_callback, ble_ibeacon_app_instance);
    furi_thread_start(ble_ibeacon_app_instance->thread);

    ble_ibeacon_app_instance->state = BLETestStateIdle;

    return (void*)ble_ibeacon_app_instance;
}

static void ble_ibeacon_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    BLETiBeaconApp* instance = (BLETiBeaconApp*)app_handle;

    if(instance) {
        instance->exit = true;
        furi_thread_flags_set(furi_thread_get_id(instance->thread), BLEiBeaconEvtExit);
        furi_thread_join(instance->thread);
        furi_thread_free(instance->thread);

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
}

static void ble_ibeacon_motd(void* context) {
    UNUSED(context);
    printf("\r\n+-------------------------------+\r\n");
    printf("| Welcome to BLE iBeacon shell! |\r\n");
    printf("+-------------------------------+\r\n\r\n");
    printf("This application emulates an iBeacon beacon named \"" BLE_IBEACON_APP_LOCAL_NAME "\" with the ability to connect to it\r\n");
}

void ble_ibeacon_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliRegistry* registry = cli_registry_alloc();
    CliShell* shell = cli_shell_alloc(ble_ibeacon_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "ibeacon");

    cli_shell_start(shell);
    BLETiBeaconApp* ble_ibeacon_app = ble_ibeacon_app_start(shell);
    cli_shell_join(shell);
    ble_ibeacon_app_stop(ble_ibeacon_app);

    cli_shell_free(shell);
    cli_registry_free(registry);
}
