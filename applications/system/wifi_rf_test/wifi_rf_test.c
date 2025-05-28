#include "wifi_rf_test.h"
#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_net.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/shell/cli_shell.h>
#include <strint.h>

#define TAG "WifiRfTestApp"

#define WIFI_RF_TEST_RECEIVE_STATS_COUNT_DEFAULT 10
#define WIFI_RF_TEST_CHANNEL_DEFAULT             1
#define WIFI_RF_TEST_POWER_DEFAULT               127
#define WIFI_RF_TEST_RATE_DEFAULT                SL_WIFI_DATA_RATE_6
#define WIFI_RF_TEST_MODE_DEFAULT                SL_WIFI_TEST_BURST_MODE

typedef enum {
    WifiTestStateIdle,
    WifiTestStateReceive,
    WifiTestStateTransmit,
} WifiTestState;

typedef struct {
    const char* str_val;
    uint16_t int_val;
} WifiRfTestOption;

const WifiRfTestOption wifi_rf_test_rate_options[] = {
    {"1", 0},         {"2", 2},         {"5.5", 4},         {"11", 6},        {"6", 139},
    {"9", 143},       {"12", 138},      {"18", 142},        {"24", 137},      {"36", 141},
    {"48", 136},      {"54", 140},      {"MCS0", 256},      {"MCS1", 257},    {"MCS2", 258},
    {"MCS3", 259},    {"MCS4", 260},    {"MCS5", 261},      {"MCS6", 262},    {"MCS7", 263},
    {"MCS7_SG", 775}, {"802.11b_1", 0}, {"802.11b_5.5", 4}, {"802.11g", 139}, {"802.11n", 256},
    {NULL, 0},
};

const WifiRfTestOption wifi_rf_test_mode_options[] = {
    {"burst", 0},
    {"continuous", 1},
    {"cw", 2},
    {"cw_low", 3},
    {"cw_high", 4},
    {NULL, 0},
};

static const sl_wifi_device_configuration_t wifi_rf_test_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = WORLD_DOMAIN,
    .boot_config = {
        .oper_mode = SL_SI91X_TRANSMIT_TEST_MODE,
        .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
        .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
            (SL_SI91X_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_WPS_DISABLE),
#else
            (SL_SI91X_FEAT_SECURITY_OPEN),
#endif
        .tcp_ip_feature_bit_map =
            (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
        .custom_feature_bit_map = SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
        .ext_custom_feature_bit_map =
            (MEMORY_CONFIG
#ifdef SLI_SI917
             | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
             ),
        .bt_feature_bit_map = SL_SI91X_BT_RF_TYPE,
        .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
        .ble_feature_bit_map = 0,
        .ble_ext_feature_bit_map = 0,
        .config_feature_bit_map = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP}};

static const sl_si91x_request_tx_test_info_t tx_test_info_default = {
    .enable = 1,
    .power = WIFI_RF_TEST_POWER_DEFAULT,
    .rate = WIFI_RF_TEST_RATE_DEFAULT,
    .length = 100,
    .mode = WIFI_RF_TEST_MODE_DEFAULT,
    .channel = WIFI_RF_TEST_CHANNEL_DEFAULT,
    .aggr_enable = 0,
    .no_of_pkts = 0,
#ifdef SLI_SI917
    .enable_11ax = 0,
    .coding_type = 0,
    .nominal_pe = 0,
    .ul_dl = 0,
    .he_ppdu_type = 0,
    .beam_change = 0,
    .bw = 0,
    .stbc = 0,
    .tx_bf = 0,
    .gi_ltf = 0,
    .dcm = 0,
    .nsts_midamble = 0,
    .spatial_reuse = 0,
    .bss_color = 0,
    .he_siga2_reserved = 0,
    .ru_allocation = 0,
    .n_heltf_tot = 0,
    .sigb_dcm = 0,
    .sigb_mcs = 0,
    .user_sta_id = 0,
    .user_idx = 0,
    .sigb_compression_field = 0,
#endif
};

typedef struct {
    FuriString* msg;
    CliShell* shell;
    WifiTestState state;

    float pass_avg;
    float fail_avg;
    uint8_t stats_count;
    uint8_t channel;
    bool exit;

    sl_status_t callback_status;
    uint16_t total_crc_pass;
    uint16_t total_crc_fail;
    uint16_t max_receive_stats_count;

    sl_si91x_request_tx_test_info_t tx_test_info;
} WifiRfTestApp;

void wifi_rf_test_app_stop(void* app_handle);
static sl_status_t wifi_rf_test_stats_receive_handler(
    sl_wifi_event_t event,
    void* reponse,
    uint32_t result_length,
    void* arg);

void* wifi_rf_test_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");

    WifiRfTestApp* instance = malloc(sizeof(WifiRfTestApp));
    instance->msg = furi_string_alloc();
    instance->shell = shell;
    instance->state = WifiTestStateIdle;

    instance->pass_avg = 0;
    instance->fail_avg = 0;
    instance->stats_count = 0;
    instance->channel = WIFI_RF_TEST_CHANNEL_DEFAULT;
    instance->max_receive_stats_count = WIFI_RF_TEST_RECEIVE_STATS_COUNT_DEFAULT;
    instance->exit = false;

    memcpy(
        &instance->tx_test_info, &tx_test_info_default, sizeof(sl_si91x_request_tx_test_info_t));

    sl_status_t status = SL_STATUS_FAIL;
    do {
        status =
            sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_rf_test_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, ANSI_FG_RED "Failed to start Wi-Fi client interface: 0x%lx" ANSI_RESET, status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        } else {
            furi_string_printf(instance->msg, "Wi-Fi initialization successful");
            cli_shell_notification_print(instance->shell, instance->msg);
        }
        // Register WLAN receive stats call back handler
        status = sl_wifi_set_stats_callback(wifi_rf_test_stats_receive_handler, instance);
        if(status != SL_STATUS_OK) {
            furi_string_printf(instance->msg, ANSI_FG_RED "Failed to set stats callback: 0x%lx" ANSI_RESET, status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        }
        status = sl_wifi_set_antenna(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, SL_WIFI_ANTENNA_INTERNAL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(instance->msg, ANSI_FG_RED "Failed to start set Antenna: 0x%lx" ANSI_RESET, status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        }

        furi_string_printf(instance->msg, "start_app: 1");
        cli_shell_notification_print(instance->shell, instance->msg);
    } while(0);

    if(status != SL_STATUS_OK) {
        wifi_rf_test_app_stop(instance);
        return NULL;
    }
    return (void*)instance;
}

void wifi_rf_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    WifiRfTestApp* instance = (WifiRfTestApp*)app_handle;

    if(instance) {
        instance->exit = true;
        furi_delay_ms(100);
        if(instance->state != WifiTestStateIdle) {
            if(instance->state == WifiTestStateReceive) {
                sl_wifi_stop_statistic_report(SL_WIFI_CLIENT_INTERFACE);
            }
            sl_si91x_transmit_test_stop();
        }
        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }

    sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
}

static sl_status_t wifi_rf_test_stats_receive_handler(
    sl_wifi_event_t event,
    void* reponse,
    uint32_t result_length,
    void* arg) {
    UNUSED_PARAMETER(result_length);

    WifiRfTestApp* instance = (WifiRfTestApp*)arg;

    if(SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
        instance->callback_status = *(sl_status_t*)reponse;
        return SL_STATUS_FAIL;
    }

    if(event == SL_WIFI_STATS_ASYNC_EVENT) {
        sl_si91x_async_stats_response_t* result = (sl_si91x_async_stats_response_t*)reponse;

        furi_string_printf(
            instance->msg, "WIFI STATS Recieved packet# %d", instance->stats_count);

        furi_string_cat_printf(
            instance->msg,
            "Rx Stats\r\n"
            "  crc_fail_cnt: %d\r\n"
            "  crc_pass_cnt: %d\r\n"
            "  rssi: -%d",
            result->crc_fail,
            result->crc_pass,
            result->cal_rssi);
        cli_shell_notification_print(instance->shell, instance->msg);

        float p = result->crc_pass;
        float f = result->crc_fail;
        float t = p + f;

        float per_pass = (p * 100 / t);
        float per_fail = (f * 100 / t);

        instance->pass_avg += per_pass;
        instance->fail_avg += per_fail;

        instance->total_crc_pass += result->crc_pass;
        instance->total_crc_fail += result->crc_fail;

        if(instance->stats_count == instance->max_receive_stats_count - 1) {
            furi_string_printf(
                instance->msg,
                "CRC Average pass%% = %.6f,         CRC Average fail%% = %.6f\r\n",
                (double)instance->pass_avg / instance->max_receive_stats_count,
                (double)instance->fail_avg / instance->max_receive_stats_count);

            furi_string_cat_printf(
                instance->msg,
                "Total : total_crc_pass %d, total_crc_fail %d",
                instance->total_crc_pass,
                instance->total_crc_fail);
            cli_shell_notification_print(instance->shell, instance->msg);

            instance->pass_avg = 0;
            instance->fail_avg = 0;
        }
        instance->stats_count++;
        instance->callback_status = SL_STATUS_OK;
    }
    return SL_STATUS_OK;
}

static bool wifi_rf_test_generic_command_guard(WifiRfTestApp* instance, FuriString* args, WifiTestState expected_state, uint8_t min_arg, uint8_t max_arg, uint8_t* arg_out) {
    if(instance->state != expected_state) {
        printf(ANSI_FG_RED "invalid state %d; expected to be in state %d\r\n" ANSI_RESET, instance->state, expected_state);
        return false;
    }

    int arg = 0;
    if(!args_read_int_and_trim(args, &arg)) {
        printf(ANSI_FG_RED "expected numeric argument\r\n" ANSI_RESET);
        return false;
    }

    if(arg < (int)min_arg || arg > (int)max_arg) {
        printf(ANSI_FG_RED "argument out of bounds; expected >= %d, <= %d\r\n" ANSI_RESET, min_arg, max_arg);
        return false;
    }

    *arg_out = arg;
    return true;
}

static bool wifi_rf_test_generic_enum_arg_command_guard(WifiRfTestApp* instance, FuriString* args, WifiTestState expected_state, const WifiRfTestOption* options, uint16_t* arg_out) {
    if(instance->state != expected_state) {
        printf(ANSI_FG_RED "invalid state %d; expected to be in state %d\r\n" ANSI_RESET, instance->state, expected_state);
        return false;
    }

    FuriString* arg = furi_string_alloc();
    bool ret_val = false;
    do {
        if(!args_read_string_and_trim(args, arg)) {
            printf(ANSI_FG_RED "expected string argument\r\n" ANSI_RESET);
            break;
        }

        const WifiRfTestOption* iter = options;
        while(iter->str_val) {
            if(furi_string_cmp_str(arg, iter->str_val) == 0) {
                *arg_out = iter->int_val;
                break;
            }
            iter++;
        }

        if(iter->str_val) {
            ret_val = true;
        } else {
            printf(ANSI_FG_RED "unrecognized variant\r\n" ANSI_RESET);
        }
    } while(0);
    furi_string_free(arg);

    if(!ret_val) {
        printf("available variants:\r\n");

        const WifiRfTestOption* iter = options;
        while(iter->str_val) {
            printf("  ");
            printf(iter->str_val);
            iter++;
        }

        printf("\r\n");
    }

    return ret_val;
}

static void wifi_rf_test_set_power_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    WifiRfTestApp* instance = context;
    uint8_t arg;
    if(wifi_rf_test_generic_command_guard(instance, args, WifiTestStateIdle, 2, 127, &arg)) {
        if(arg <= 18 || arg == 127) {
            printf("Power set to %d\r\n", arg);
            instance->tx_test_info.power = arg;
        } else {
            printf(ANSI_FG_RED "argument out of bounds; expected >= 2, <= 18, or 127\r\n" ANSI_RESET);
        }
    }
}

static void wifi_rf_test_set_rate_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    WifiRfTestApp* instance = context;
    uint16_t arg;
    if(wifi_rf_test_generic_enum_arg_command_guard(instance, args, WifiTestStateIdle, wifi_rf_test_rate_options, &arg)) {
        instance->tx_test_info.rate = arg;
        printf("Rate set\r\n");
    }
}

static void wifi_rf_test_set_channel_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    WifiRfTestApp* instance = context;
    uint8_t arg;
    if(wifi_rf_test_generic_command_guard(instance, args, WifiTestStateIdle, 1, 14, &arg)) {
        size_t frequency = 2407 + (5 * (size_t)arg);
        instance->tx_test_info.rate = arg;
        printf("Channel set: %zu MHz\r\n", frequency);
    }
}

static void wifi_rf_test_set_mode_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    WifiRfTestApp* instance = context;
    uint16_t arg;
    if(wifi_rf_test_generic_enum_arg_command_guard(instance, args, WifiTestStateIdle, wifi_rf_test_mode_options, &arg)) {
        instance->tx_test_info.mode = arg;
        printf("Mode set\r\n");
    }
}

static void wifi_rf_test_rx_command(PipeSide* pipe, FuriString* args, void* context) {
    char* args_cstr = (char*)furi_string_get_cstr(args);
    WifiRfTestApp* instance = context;

    if(instance->state == WifiTestStateTransmit) {
        sl_si91x_transmit_test_stop();
        printf("Transmit test stop Success\r\n");
        instance->state = WifiTestStateIdle;
    }

    instance->state = WifiTestStateReceive;
    StrintParseError parse_err = StrintParseNoError;
    if(furi_string_size(args)) {
        parse_err =
            strint_to_uint16(args_cstr, &args_cstr, &instance->max_receive_stats_count, 10);
        if(parse_err != StrintParseNoError) {
            printf(ANSI_FG_RED "invalid argument\r\n" ANSI_RESET);
        }
    } else {
        instance->max_receive_stats_count = WIFI_RF_TEST_RECEIVE_STATS_COUNT_DEFAULT;
    }

    if(parse_err == StrintParseNoError) {
        ////////////////////////////////////////
        // Transmit data/TX from the peer//////
        ////////////////////////////////////////

        // Start/Receive publishing RX stats
        sl_wifi_channel_t channel = {0};
        channel.channel = instance->channel;
        sl_status_t status = sl_wifi_start_statistic_report(SL_WIFI_CLIENT_INTERFACE, channel);

        if(SL_STATUS_IN_PROGRESS == status) {
            instance->callback_status = SL_STATUS_IN_PROGRESS;

            printf("Receive Statistics...\r\n");

            do {
                while(instance->stats_count <= instance->max_receive_stats_count &&
                      !instance->exit && !cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
                    furi_thread_yield();
                    if(instance->stats_count == instance->max_receive_stats_count &&
                       instance->callback_status != SL_STATUS_IN_PROGRESS) {
                        printf("Stop Statistics Report\r\n");

                        sl_wifi_stop_statistic_report(SL_WIFI_CLIENT_INTERFACE);

                        printf("Start Statistic Report Success\r\n");
                        instance->state = WifiTestStateIdle;
                        break;
                    }
                }
            } while(0);
            status = instance->callback_status;
        }
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Start Statistic Report Failed, Error Code : 0x%lX\r\n" ANSI_RESET,
                status);
        }
        instance->stats_count = 0;
        instance->callback_status = SL_STATUS_OK;
        instance->total_crc_pass = 0;
        instance->total_crc_fail = 0;
        instance->pass_avg = 0;
        instance->fail_avg = 0;
    }

    printf("NOTE: Receive stats testing should be done in a controlled environment (RF shield box or chamber).");
}

static void wifi_rf_test_tx_start_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiRfTestApp* instance = context;

    if(instance->state == WifiTestStateReceive) {
        sl_wifi_stop_statistic_report(SL_WIFI_CLIENT_INTERFACE);
        instance->state = WifiTestStateIdle;
    }

    instance->state = WifiTestStateTransmit;
    do {
        if(instance->tx_test_info.mode != SL_WIFI_TEST_BURST_MODE) {
            uint16_t mode_temp = instance->tx_test_info.mode;
            instance->tx_test_info.mode = SL_WIFI_TEST_CONTINOUS_MODE;
            sl_status_t status = sl_si91x_transmit_test_start(&instance->tx_test_info);
            if(status != SL_STATUS_OK) {
                printf(ANSI_FG_RED "Transmit test start failed 0x%lX\r\n" ANSI_RESET, status);
                instance->tx_test_info.mode = mode_temp;
                break;
            }
            furi_delay_ms(200);
            status = sl_si91x_transmit_test_stop();
            if(status != SL_STATUS_OK) {
                printf(ANSI_FG_RED "Transmit failed to stop %lx\r\n" ANSI_RESET, status);
                instance->tx_test_info.mode = mode_temp;
                break;
            }
            instance->tx_test_info.mode = mode_temp;
        }

        sl_status_t status = sl_si91x_transmit_test_start(&instance->tx_test_info);
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Transmit test start failed 0x%lX\r\n" ANSI_RESET, status);
            break;
        } else {
            printf("Transmit test start Success\r\n");
        }
    } while(0);
}

static void wifi_rf_test_tx_stop_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiRfTestApp* instance = context;
    if(instance->state == WifiTestStateTransmit) {
        sl_status_t status = sl_si91x_transmit_test_stop();
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Transmit test stop failed 0x%lX\r\n" ANSI_RESET, status);
            cli_shell_notification_print(instance->shell, instance->msg);
        } else {
            printf("Transmit test stop Success\r\n");
        }
        instance->state = WifiTestStateIdle;
    } else {
        printf("Transmit test not started\r\n");
    }
}

static void wifi_rf_test_motd(void* context) {
    UNUSED(context);
    printf("\r\n+---------------------------------+\r\n");
    printf("| Welcome to Wi-Fi RF test shell! |\r\n");
    printf("+---------------------------------+\r\n\r\n");
    printf("Read the manual: https://github.com/SiliconLabs/wiseconnect/tree/master/examples/snippets/wlan/wlan_rf_test\r\n");
}

void wifi_rf_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliRegistry* registry = cli_registry_alloc();
    CliShell* shell = cli_shell_alloc(wifi_rf_test_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "wifi_rf_test");
    cli_shell_start(shell);

    WifiRfTestApp* app = wifi_rf_test_app_start(shell);
    cli_registry_add_command(registry, "rx", CliCommandFlagDefault, wifi_rf_test_rx_command, app);
    cli_registry_add_command(registry, "tx_start", CliCommandFlagDefault, wifi_rf_test_tx_start_command, app);
    cli_registry_add_command(registry, "tx_stop", CliCommandFlagDefault, wifi_rf_test_tx_stop_command, app);
    cli_registry_add_command(registry, "set_power", CliCommandFlagDefault, wifi_rf_test_set_power_command, app);
    cli_registry_add_command(registry, "set_channel", CliCommandFlagDefault, wifi_rf_test_set_channel_command, app);
    cli_registry_add_command(registry, "set_mode", CliCommandFlagDefault, wifi_rf_test_set_mode_command, app);
    cli_registry_add_command(registry, "set_rate", CliCommandFlagDefault, wifi_rf_test_set_rate_command, app);

    cli_shell_join(shell);
    wifi_rf_test_app_stop(app);

    cli_shell_free(shell);
    cli_registry_free(registry);
}
