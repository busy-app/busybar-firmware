#include "crypto_test.h"

#include <furi.h>

#include <furi_hal_nwp.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>
#include "sl_net.h"

#include "helpers/crypto_aes.h"
#include "helpers/crypto_sha.h"
#include "helpers/crypto_ecdsa.h"
#include "helpers/crypto_hmac.h"
#include "helpers/crypto_mbedtls_edsa.h"

#include <cli/args.h>
#include <cli/cli_command.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>
#include <strint.h>

#define TAG "CryptoTestApp"

// static const sl_wifi_device_configuration_t client_configuration_use_crypto = {
//     .boot_option = LOAD_NWP_FW,
//     .mac_address = NULL,
//     .band = SL_SI91X_WIFI_BAND_2_4GHZ,
//     .region_code = US,
//     .boot_config = {
//         .oper_mode = SL_SI91X_CLIENT_MODE,
//         .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
//         .feature_bit_map =
//             (SL_SI91X_FEAT_SECURITY_PSK | SL_SI91X_FEAT_AGGREGATION
// #ifdef SLI_SI91X_MCU_INTERFACE
//              | SL_SI91X_FEAT_WPS_DISABLE
// #endif
//              ),
//         .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
//         .custom_feature_bit_map = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),
//         .ext_custom_feature_bit_map =
//             (MEMORY_CONFIG
// #if defined(SLI_SI917) || defined(SLI_SI915)
//              | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
// #endif
//              ),
//         .bt_feature_bit_map = 0,
//         .ext_tcp_ip_feature_bit_map = 0,
//         .ble_feature_bit_map = 0,
//         .ble_ext_feature_bit_map = 0,
//         .config_feature_bit_map = 0}};

typedef enum {
    CryptoTestStateIdle,
    CryptoTestStateWifiInit,
} CryptoTestState;

struct CryptoTestApp {
    FuriString* msg;
    CliShell* shell;
    CryptoTestState state;

    //bool exit;
};

void crypto_test_app_stop(void* app_handle);

static CryptoTestApp* crypto_test_app_instance = NULL;

void* crypto_test_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");
    //sl_status_t status = SL_STATUS_FAIL;

    crypto_test_app_instance = malloc(sizeof(CryptoTestApp));
    crypto_test_app_instance->msg = furi_string_alloc();
    crypto_test_app_instance->shell = shell;

    //crypto_test_app_instance->exit = false;
    crypto_test_app_instance->state = CryptoTestStateIdle;

    // do {
    //     status = sl_net_init(
    //         SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration_use_crypto, NULL, NULL);
    //     if(status != SL_STATUS_OK) {
    //         printf(
    //             ANSI_FG_RED "Failed to start Wi-Fi client interface: 0x%08lx\r\n" ANSI_RESET,
    //             status);
    //         break;
    //     }
    //     FURI_LOG_D(TAG, "Wi-Fi initialization successful");
    //     crypto_test_app_instance->state = CryptoTestStateWifiInit;
    // } while(false);
    if(furi_hal_nwp_init()){
        crypto_test_app_instance->state = CryptoTestStateWifiInit;
    } else {
        crypto_test_app_stop(crypto_test_app_instance);
        return NULL;
    }

    // if(status != SL_STATUS_OK) {
        
    // }
    return (void*)crypto_test_app_instance;
}

void crypto_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    CryptoTestApp* instance = (CryptoTestApp*)app_handle;

    if(crypto_test_app_instance->state == CryptoTestStateWifiInit) {
        //sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
        //FURI_LOG_D(TAG, "Wi-Fi deinitialization successful");
        furi_hal_nwp_deinit();
    }

    if(instance) {
        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
}

static void crypto_test_motd(void* context) {
    UNUSED(context);
    printf("\r\n+-------------------------------+\r\n");
    printf("| Welcome to crypto test shell! |\r\n");
    printf("+-------------------------------+\r\n\r\n");
}

void crypto_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliRegistry* registry = cli_registry_alloc();
    cli_registry_add_command(registry, "aes", CliCommandFlagDefault, crypto_aes_command, NULL);
    cli_registry_add_command(registry, "ecdsa", CliCommandFlagDefault, crypto_ecdsa_command, NULL);
    cli_registry_add_command(registry, "hmac", CliCommandFlagDefault, crypto_hmac_command, NULL);
    cli_registry_add_command(registry, "sha", CliCommandFlagDefault, crypto_sha_command, NULL);
    cli_registry_add_command(registry, "mbedtls_edsa", CliCommandFlagDefault, crypto_mbedtls_edsa_command, NULL);

    CliShell* shell = cli_shell_alloc(crypto_test_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "crypto_test");
    CryptoTestApp* app = crypto_test_app_start(shell);
    if(app) {
        cli_shell_start(shell);
        cli_shell_join(shell);
        crypto_test_app_stop(app);
    } else {
        printf(ANSI_FG_RED "Failed to start crypto test app" ANSI_RESET "\r\n");
    }

    cli_shell_free(shell);
    cli_registry_free(registry);
}
