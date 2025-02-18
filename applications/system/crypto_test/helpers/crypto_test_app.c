#include "crypto_test_app.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>
#include "sl_net.h"

#include "crypto_aes.h"
#include "crypto_sha.h"

#include <args.h>
#include <strint.h>

#define TAG "CryptoTestApp"

static const sl_wifi_device_configuration_t client_configuration_use_crypto = {
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
        .ext_custom_feature_bit_map =
            (MEMORY_CONFIG
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
    CryptoTestCmdTypeHelp,
    CryptoTestCmdTypeHelpHelp,
    CryptoTestCmdTypeAesTest,
    CryptoTestCmdTypeShaTest,

    CryptoTestCmdTypeMax,
} CryptoTestCmdType;

typedef enum {
    CryptoTestStateIdle,
    CryptoTestStateWifiInit,
} CryptoTestState;

typedef struct {
    char* cmd;
} CryptoTestCmd;

const CryptoTestCmd crypto_test_cmd[CryptoTestCmdTypeMax] = {
    {"?"},
    {"help"},
    {"aes_test"},
    {"sha_test"},
};

struct CryptoTestApp {
    FuriString* msg;
    CliWorker* worker;
    CryptoTestState state;

    //bool exit;
};

static CryptoTestApp* crypto_test_app_instance = NULL;

static void crypto_test_app_cmd_usage(CryptoTestApp* instance);

static void crypto_test_app_send_msg(CryptoTestApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

void crypto_test_app_send_text(CryptoTestApp* instance, FuriString* text) {
    cli_worker_add_rx_data(
        instance->worker, (uint8_t*)furi_string_get_cstr(text), furi_string_utf8_length(text));
}

static void crypto_test_app_send_msg_invalid_arg(CryptoTestApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    crypto_test_app_send_msg(instance);
}

void* crypto_test_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");
    sl_status_t status = SL_STATUS_FAIL;

    crypto_test_app_instance = malloc(sizeof(CryptoTestApp));
    crypto_test_app_instance->msg = furi_string_alloc();
    crypto_test_app_instance->worker = worker;

    //crypto_test_app_instance->exit = false;
    crypto_test_app_instance->state = CryptoTestStateIdle;

    do {
        status = sl_net_init(
            SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration_use_crypto, NULL, NULL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                crypto_test_app_instance->msg,
                "Failed to start Wi-Fi client interface: 0x%lx\r\n",
                status);
            crypto_test_app_send_msg(crypto_test_app_instance);
            break;
        } else {
            furi_string_printf(
                crypto_test_app_instance->msg, "Wi-Fi initialization successful\r\n");
            crypto_test_app_send_msg(crypto_test_app_instance);
        }
        FURI_LOG_D(TAG, "Wi-Fi initialization successful");
        crypto_test_app_instance->state = CryptoTestStateWifiInit;
    } while(false);

    if(status != SL_STATUS_OK) {
        crypto_test_app_stop(crypto_test_app_instance);
        return NULL;
    }
    crypto_test_app_cmd_usage(crypto_test_app_instance);
    return (void*)crypto_test_app_instance;
}

void crypto_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    CryptoTestApp* instance = (CryptoTestApp*)app_handle;

    if(crypto_test_app_instance->state == CryptoTestStateWifiInit) {
        sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
        FURI_LOG_D(TAG, "Wi-Fi deinitialization successful");
    }

    if(instance) {
        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
}

static sl_status_t crypto_test_app(CryptoTestApp* instance, uint8_t cmd_index, FuriString* args) {
    //sl_status_t status = SL_STATUS_FAIL;

    char* args_cstr = (char*)furi_string_get_cstr(args);
    UNUSED(args_cstr);
    FuriString* arg = furi_string_alloc();

    switch(cmd_index) {
    case CryptoTestCmdTypeHelp:
    case CryptoTestCmdTypeHelpHelp:
        crypto_test_app_cmd_usage(instance);
        break;
    case CryptoTestCmdTypeAesTest:
        crypto_aes_test(instance, instance->msg);
        break;
    case CryptoTestCmdTypeShaTest:
        crypto_sha_test(instance, instance->msg);
        break;

    default:
        crypto_test_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void crypto_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    CryptoTestApp* instance = (CryptoTestApp*)app_handle;
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

        for(i = 0; i < CryptoTestCmdTypeMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)crypto_test_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(crypto_test_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                crypto_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            crypto_test_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void crypto_test_app_cmd_usage(CryptoTestApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", TAG);
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");
    furi_string_cat_printf(instance->msg, "aes_test\r\n");
    furi_string_cat_printf(instance->msg, "sha_test\r\n");

    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    crypto_test_app_send_msg(instance);
}
