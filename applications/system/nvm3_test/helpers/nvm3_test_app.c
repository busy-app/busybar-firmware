#include "nvm3_test_app.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_net.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include <nvm3_default.h>
#include <ecode.h>

#include <args.h>
#include <strint.h>

#define TAG "NVM3 Test"

const sl_wifi_device_configuration_t client_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = US,
    .boot_config = {
        .oper_mode = SL_SI91X_CLIENT_MODE,
        .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
        .feature_bit_map =
            (SL_SI91X_FEAT_SECURITY_PSK | SL_SI91X_FEAT_AGGREGATION
#ifdef SLI_SI91X_MCU_INTERFACE
             | SL_SI91X_FEAT_WPS_DISABLE
#endif
             ),
        .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
        .custom_feature_bit_map = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),
        .ext_custom_feature_bit_map = (
#ifdef SLI_SI91X_MCU_INTERFACE
            SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
#else
            SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE
#endif
#if defined(SLI_SI917) || defined(SLI_SI915)
            | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
            ),
        .bt_feature_bit_map = 0,
        .ext_tcp_ip_feature_bit_map = 0,
        .ble_feature_bit_map = 0,
        .ble_ext_feature_bit_map = 0,
        .config_feature_bit_map = 0}};

typedef enum {
    NVM3TestCmdTypeHelp,
    NVM3TestCmdTypeHelpHelp,

    NVM3TestCmdTypeMax,
} NVM3TestCmdType;

typedef enum {
    NVM3TestStateIdle,
    NVM3TestStateInit,
} NVM3TestState;

typedef struct {
    char* cmd;
} NVM3TestCmd;

const NVM3TestCmd nvm3_test_cmd[NVM3TestCmdTypeMax] = {
    {"?"},
    {"help"},
};

struct NVM3TestApp {
    FuriString* msg;
    CliWorker* worker;
    NVM3TestState state;

    bool exit;
};

static NVM3TestApp* nvm3_test_app_instance = NULL;

static void nvm3_test_app_cmd_usage(NVM3TestApp* instance);

static void nvm3_test_app_send_msg(NVM3TestApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

void nvm3_test_app_send_text(NVM3TestApp* instance, FuriString* text) {
    cli_worker_add_rx_data(
        instance->worker, (uint8_t*)furi_string_get_cstr(text), furi_string_utf8_length(text));
}

static void nvm3_test_app_send_msg_invalid_arg(NVM3TestApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    nvm3_test_app_send_msg(instance);
}

void* nvm3_test_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");

    nvm3_test_app_instance = malloc(sizeof(NVM3TestApp));
    nvm3_test_app_instance->msg = furi_string_alloc();
    nvm3_test_app_instance->worker = worker;
    nvm3_test_app_instance->state = NVM3TestStateIdle;

    nvm3_test_app_instance->exit = false;
    sl_status_t status = SL_STATUS_FAIL;
    do {
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                nvm3_test_app_instance->msg,
                "Failed to start Wi-Fi client interface: 0x%lx\r\n",
                status);
            nvm3_test_app_send_msg(nvm3_test_app_instance);
            break;
        }

        Ecode_t err=ECODE_OK;
        nvm3_initDefault();
        if(err != ECODE_OK) {
            furi_string_printf(nvm3_test_app_instance->msg, "Failed to init NVM3: 0x%lx\r\n", err);
            nvm3_test_app_send_msg(nvm3_test_app_instance);
            break;
        }

        furi_string_printf(nvm3_test_app_instance->msg, "Wi-Fi APSTA interface init\r\n");
        nvm3_test_app_send_msg(nvm3_test_app_instance);

        nvm3_test_app_cmd_usage(nvm3_test_app_instance);
        nvm3_test_app_instance->state = NVM3TestStateInit;
    } while(0);

    if(status != SL_STATUS_OK) {
        nvm3_test_app_stop(nvm3_test_app_instance);
        return NULL;
    }
    return (void*)nvm3_test_app_instance;
}

void nvm3_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    NVM3TestApp* instance = (NVM3TestApp*)app_handle;

    if(instance->state == NVM3TestStateInit) {
        sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    }

    if(instance) {
        instance->exit = true;

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
}

static sl_status_t nvm3_test_app(NVM3TestApp* instance, uint8_t cmd_index, FuriString* args) {
    //sl_status_t status = SL_STATUS_FAIL;

    char* args_cstr = (char*)furi_string_get_cstr(args);
    UNUSED(args_cstr);
    FuriString* arg = furi_string_alloc();

    switch(cmd_index) {
    case NVM3TestCmdTypeHelp:
    case NVM3TestCmdTypeHelpHelp:
        nvm3_test_app_cmd_usage(instance);
        break;

    default:
        nvm3_test_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void nvm3_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    NVM3TestApp* instance = (NVM3TestApp*)app_handle;
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

        for(i = 0; i < NVM3TestCmdTypeMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)nvm3_test_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(nvm3_test_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                nvm3_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            nvm3_test_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void nvm3_test_app_cmd_usage(NVM3TestApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", "BLE iBeacon");
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");

    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    nvm3_test_app_send_msg(instance);
}
