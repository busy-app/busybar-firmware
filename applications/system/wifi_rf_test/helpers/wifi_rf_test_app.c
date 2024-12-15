#include "wifi_rf_test_app.h"
#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_net.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#include <args.h>
#include <strint.h>

#define TAG "WifiRfTestApp"

#define WIFI_RF_TEST_RECEIVE_STATS_COUNT_DEFAULT 10
#define WIFI_RF_TEST_CHANNEL_DEFAULT             1
#define WIFI_RF_TEST_POWER_DEFAULT               127
#define WIFI_RF_TEST_RATE_DEFAULT                SL_WIFI_DATA_RATE_6
#define WIFI_RF_TEST_MODE_DEFAULT                SL_WIFI_TEST_BURST_MODE

typedef enum {
    WifiTestCmdTypeHelp,
    WifiTestCmdTypeHelpHelp,
    WifiTestCmdTypeReceive,
    WifiTestCmdTypeTransmitStart,
    WifiTestCmdTypeTransmitStop,
    WifiTestCmdTypeSetPower,
    WifiTestCmdTypeSetRate,
    WifiTestCmdTypeSetChannel,
    WifiTestCmdTypeSetMode,

    WifiTestCmdTypeMax,
} WifiTestCmdType;

typedef enum {
    WifiTestStateIdle,
    WifiTestStateReceive,
    WifiTestStateTransmit,
} WifiTestState;

typedef struct {
    char* cmd;
} WifiTestCmd;

const WifiTestCmd wifi_rf_test_cmd[WifiTestCmdTypeMax] = {
    {"?"},
    {"help"},
    {"rx"},
    {"tx_start"},
    {"tx_stop"},
    {"set_power"},
    {"set_rate"},
    {"set_channel"},
    {"set_mode"},

};

typedef struct {
    char* rate_cmd;
    uint16_t rate_value;
} WifiTestRateCmd;
#define WIFI_RF_TEST_RATE_CMD_MAX 21
const WifiTestRateCmd wifi_rf_test_rate_cmd[WIFI_RF_TEST_RATE_CMD_MAX] = {
    {"1", 0},      {"2", 2},      {"5.5", 4},       {"11", 6},     {"6", 139},    {"9", 143},
    {"12", 138},   {"18", 142},   {"24", 137},      {"36", 141},   {"48", 136},   {"54", 140},
    {"MCS0", 256}, {"MCS1", 257}, {"MCS2", 258},    {"MCS3", 259}, {"MCS4", 260}, {"MCS5", 261},
    {"MCS6", 262}, {"MCS7", 263}, {"MCS7_SG", 775},
};

typedef struct {
    char* mode_cmd;
    uint16_t mode_value;
} WifiTestModeCmd;
#define WIFI_RF_TEST_MODE_CMD_MAX 5
const WifiTestModeCmd wifi_rf_test_mode_cmd[WIFI_RF_TEST_MODE_CMD_MAX] = {
    {"burst", 0},
    {"continuous", 1},
    {"cw", 2},
    {"cw_low", 3},
    {"cw_high", 4},
};

typedef struct {
    char* channel_cmd;
    uint16_t channel_value;
} WifiTestChannelCmd;
#define WIFI_RF_TEST_CHANNEL_CMD_MAX 14
const WifiTestChannelCmd wifi_rf_test_channel_cmd[WIFI_RF_TEST_CHANNEL_CMD_MAX] = {
    {"1", 2412},
    {"2", 2417},
    {"3", 2422},
    {"4", 2427},
    {"5", 2432},
    {"6", 2437},
    {"7", 2442},
    {"8", 2447},
    {"9", 2452},
    {"10", 2457},
    {"11", 2462},
    {"12", 2467},
    {"13", 2472},
    {"14", 2484},
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

struct WifiRfTestApp {
    FuriString* msg;
    CliWorker* worker;
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
};

static void wifi_rf_test_app_cmd_usage(WifiRfTestApp* instance);
static sl_status_t wifi_rf_test_stats_receive_handler(
    sl_wifi_event_t event,
    void* reponse,
    uint32_t result_length,
    void* arg);

static void wifi_rf_test_app_send_msg(WifiRfTestApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

static void wifi_rf_test_app_send_msg_invalid_arg(WifiRfTestApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    wifi_rf_test_app_send_msg(instance);
}

void* wifi_rf_test_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");

    WifiRfTestApp* instance = malloc(sizeof(WifiRfTestApp));
    instance->msg = furi_string_alloc();
    instance->worker = worker;
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
                instance->msg, "Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
            wifi_rf_test_app_send_msg(instance);
            break;
        } else {
            furi_string_printf(instance->msg, "Wi-Fi initialization successful\r\n");
            wifi_rf_test_app_send_msg(instance);
        }
        // Register WLAN receive stats call back handler
        status = sl_wifi_set_stats_callback(wifi_rf_test_stats_receive_handler, instance);
        if(status != SL_STATUS_OK) {
            furi_string_printf(instance->msg, "Failed to set stats callback: 0x%lx\r\n", status);
            wifi_rf_test_app_send_msg(instance);
            break;
        }
        status = sl_wifi_set_antenna(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, SL_WIFI_ANTENNA_INTERNAL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(instance->msg, "Failed to start set Antenna: 0x%lx\r\n", status);
            wifi_rf_test_app_send_msg(instance);
            break;
        }

        wifi_rf_test_app_cmd_usage(instance);
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

    if(event == SL_WIFI_STATS_AYSNC_EVENT) {
        sl_si91x_async_stats_response_t* result = (sl_si91x_async_stats_response_t*)reponse;

        furi_string_printf(
            instance->msg, "WIFI STATS Recieved packet# %d\r\n", instance->stats_count);

        furi_string_cat_printf(
            instance->msg,
            "stats : crc_pass %d, crc_fail %d, cal_rssi :%d\r\n",
            result->crc_pass,
            result->crc_fail,
            result->cal_rssi);
        wifi_rf_test_app_send_msg(instance);

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
                "Total : total_crc_pass %d, total_crc_fail %d\r\n",
                instance->total_crc_pass,
                instance->total_crc_fail);
            wifi_rf_test_app_send_msg(instance);

            instance->pass_avg = 0;
            instance->fail_avg = 0;
        }
        instance->stats_count++;
        instance->callback_status = SL_STATUS_OK;
    }
    return SL_STATUS_OK;
}

sl_status_t wifi_rf_test_app(WifiRfTestApp* instance, uint8_t cmd_index, FuriString* args) {
    sl_status_t status = SL_STATUS_FAIL;

    char* args_cstr = (char*)furi_string_get_cstr(args);
    FuriString* arg = furi_string_alloc();
    StrintParseError parse_err = StrintParseNoError;
    uint8_t i = 0;

    switch(cmd_index) {
    case WifiTestCmdTypeHelp:
    case WifiTestCmdTypeHelpHelp:
        wifi_rf_test_app_cmd_usage(instance);
        break;

    case WifiTestCmdTypeReceive:
        if(instance->state == WifiTestStateTransmit) {
            sl_si91x_transmit_test_stop();
            furi_string_printf(instance->msg, "Transmit test stop Success\r\n");
            wifi_rf_test_app_send_msg(instance);
            instance->state = WifiTestStateIdle;
        }

        instance->state = WifiTestStateReceive;

        if(furi_string_size(args)) {
            parse_err |=
                strint_to_uint16(args_cstr, &args_cstr, &instance->max_receive_stats_count, 10);
            if(parse_err != StrintParseNoError) {
                wifi_rf_test_app_send_msg_invalid_arg(instance);
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
            status = sl_wifi_start_statistic_report(SL_WIFI_CLIENT_INTERFACE, channel);

            if(SL_STATUS_IN_PROGRESS == status) {
                instance->callback_status = SL_STATUS_IN_PROGRESS;

                furi_string_printf(instance->msg, "Receive Statistics...\r\n");
                wifi_rf_test_app_send_msg(instance);

                do {
                    while(instance->stats_count <= instance->max_receive_stats_count &&
                          !instance->exit) {
                        furi_thread_yield();
                        if(instance->stats_count == instance->max_receive_stats_count &&
                           instance->callback_status != SL_STATUS_IN_PROGRESS) {
                            furi_string_printf(instance->msg, "Stop Statistics Report\r\n");
                            wifi_rf_test_app_send_msg(instance);

                            sl_wifi_stop_statistic_report(SL_WIFI_CLIENT_INTERFACE);

                            furi_string_printf(
                                instance->msg, "Start Statistic Report Success\r\n");
                            wifi_rf_test_app_send_msg(instance);
                            instance->state = WifiTestStateIdle;
                            break;
                        }
                    }
                } while(0);
                status = instance->callback_status;
            }
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg,
                    "Start Statistic Report Failed, Error Code : 0x%lX\r\n",
                    status);
                wifi_rf_test_app_send_msg(instance);
            }
            instance->stats_count = 0;
            instance->callback_status = SL_STATUS_OK;
            instance->total_crc_pass = 0;
            instance->total_crc_fail = 0;
            instance->pass_avg = 0;
            instance->fail_avg = 0;
        }
        break;
    case WifiTestCmdTypeTransmitStart:
        if(instance->state == WifiTestStateReceive) {
            sl_wifi_stop_statistic_report(SL_WIFI_CLIENT_INTERFACE);
            instance->state = WifiTestStateIdle;
        }

        instance->state = WifiTestStateTransmit;
        do {
            if(instance->tx_test_info.mode != SL_WIFI_TEST_BURST_MODE) {
                uint16_t mode_temp = instance->tx_test_info.mode;
                instance->tx_test_info.mode = SL_WIFI_TEST_CONTINOUS_MODE;
                status = sl_si91x_transmit_test_start(&instance->tx_test_info);
                if(status != SL_STATUS_OK) {
                    furi_string_printf(
                        instance->msg, "Transmit test start failed 0x%lX\r\n", status);
                    wifi_rf_test_app_send_msg(instance);
                    instance->tx_test_info.mode = mode_temp;
                    break;
                }
                status = sl_si91x_transmit_test_stop();
                if(status != SL_STATUS_OK) {
                    furi_string_printf(instance->msg, "Transmit failed to stop %lx\r\n", status);
                    wifi_rf_test_app_send_msg(instance);
                    instance->tx_test_info.mode = mode_temp;
                    break;
                }
                instance->tx_test_info.mode = mode_temp;
            }

            status = sl_si91x_transmit_test_start(&instance->tx_test_info);
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Transmit test start failed 0x%lX\r\n", status);
                wifi_rf_test_app_send_msg(instance);
                break;
            } else {
                furi_string_printf(instance->msg, "Transmit test start Success\r\n");
                wifi_rf_test_app_send_msg(instance);
            }
        } while(0);
        break;
    case WifiTestCmdTypeTransmitStop:
        if(instance->state == WifiTestStateTransmit) {
            status = sl_si91x_transmit_test_stop();
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Transmit test stop failed 0x%lX\r\n", status);
                wifi_rf_test_app_send_msg(instance);
            } else {
                furi_string_printf(instance->msg, "Transmit test stop Success\r\n");
                wifi_rf_test_app_send_msg(instance);
            }
            instance->state = WifiTestStateIdle;
        } else {
            furi_string_printf(instance->msg, "Transmit test not started\r\n");
            wifi_rf_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeSetPower:
        if(instance->state == WifiTestStateIdle) {
            if(furi_string_size(args)) {
                uint16_t power_temp = 0;
                parse_err |= strint_to_uint16(args_cstr, &args_cstr, &power_temp, 10);

                if(parse_err == StrintParseNoError) {
                    if((power_temp >= 2 && power_temp <= 18) || (power_temp == 127)) {
                        furi_string_printf(instance->msg, "Power set to %d\r\n", power_temp);
                        wifi_rf_test_app_send_msg(instance);
                        instance->tx_test_info.power = power_temp;
                    } else {
                        wifi_rf_test_app_send_msg_invalid_arg(instance);
                    }
                } else {
                    wifi_rf_test_app_send_msg_invalid_arg(instance);
                }
            }
        } else {
            furi_string_printf(instance->msg, "Transmit test is running, stop it first\r\n");
            wifi_rf_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeSetRate:
        if(instance->state == WifiTestStateIdle) {
            do {
                if(!args_read_string_and_trim(args, arg)) {
                    wifi_rf_test_app_send_msg_invalid_arg(instance);
                    break;
                }

                for(i = 0; i < WIFI_RF_TEST_RATE_CMD_MAX; i++) {
                    if(furi_string_cmp_str(arg, (char*)wifi_rf_test_rate_cmd[i].rate_cmd) == 0) {
                        instance->tx_test_info.rate = wifi_rf_test_rate_cmd[i].rate_value;
                        furi_string_printf(
                            instance->msg,
                            "Rate set to %s\r\n",
                            wifi_rf_test_rate_cmd[i].rate_cmd);
                        wifi_rf_test_app_send_msg(instance);
                        break;
                    }
                }

                if(i == WIFI_RF_TEST_RATE_CMD_MAX) {
                    furi_string_printf(instance->msg, "Unknown rate\r\n");
                    wifi_rf_test_app_send_msg(instance);
                }
            } while(false);
        } else {
            furi_string_printf(instance->msg, "Transmit test is running, stop it first\r\n");
            wifi_rf_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeSetChannel:
        if(instance->state == WifiTestStateIdle) {
            do {
                if(!args_read_string_and_trim(args, arg)) {
                    wifi_rf_test_app_send_msg_invalid_arg(instance);
                    break;
                }

                for(i = 0; i < WIFI_RF_TEST_CHANNEL_CMD_MAX; i++) {
                    if(furi_string_cmp_str(arg, (char*)wifi_rf_test_channel_cmd[i].channel_cmd) ==
                       0) {
                        instance->tx_test_info.channel = i + 1;
                        furi_string_printf(
                            instance->msg,
                            "Channel set to %d frequency %d Mhz\r\n",
                            instance->tx_test_info.channel,
                            wifi_rf_test_channel_cmd[i].channel_value);
                        wifi_rf_test_app_send_msg(instance);
                        break;
                    }
                }

                if(i == WIFI_RF_TEST_CHANNEL_CMD_MAX) {
                    furi_string_printf(instance->msg, "Unknown channel\r\n");
                    wifi_rf_test_app_send_msg(instance);
                }
            } while(false);
        } else {
            furi_string_printf(instance->msg, "Transmit test is running, stop it first\r\n");
            wifi_rf_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeSetMode:
        if(instance->state == WifiTestStateIdle) {
            do {
                if(!args_read_string_and_trim(args, arg)) {
                    wifi_rf_test_app_send_msg_invalid_arg(instance);
                    break;
                }

                for(i = 0; i < WIFI_RF_TEST_MODE_CMD_MAX; i++) {
                    if(furi_string_cmp_str(arg, (char*)wifi_rf_test_mode_cmd[i].mode_cmd) == 0) {
                        instance->tx_test_info.mode = wifi_rf_test_mode_cmd[i].mode_value;
                        furi_string_printf(
                            instance->msg,
                            "Mode set to %s\r\n",
                            wifi_rf_test_mode_cmd[i].mode_cmd);
                        wifi_rf_test_app_send_msg(instance);
                        break;
                    }
                }

                if(i == WIFI_RF_TEST_MODE_CMD_MAX) {
                    furi_string_printf(instance->msg, "Unknown mode\r\n");
                    wifi_rf_test_app_send_msg(instance);
                }
            } while(false);
        } else {
            furi_string_printf(instance->msg, "Transmit test is running, stop it first\r\n");
            wifi_rf_test_app_send_msg(instance);
        }
        break;
    default:
        wifi_rf_test_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void wifi_rf_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    WifiRfTestApp* instance = (WifiRfTestApp*)app_handle;
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

        for(i = 0; i < WifiTestCmdTypeMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)wifi_rf_test_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(wifi_rf_test_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                wifi_rf_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            wifi_rf_test_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void wifi_rf_test_app_cmd_usage(WifiRfTestApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", TAG);
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "***********************************************\r\n");
    furi_string_cat_printf(
        instance->msg,
        "Read the manual: https://github.com/SiliconLabs/wiseconnect/tree/master/examples/snippets/wlan/wlan_rf_test\r\n");
    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");
    furi_string_cat_printf(
        instance->msg,
        "rx [stats_count] Receive stats testing, Number of iterations for which the receive stats would be displayed\r\n");
    furi_string_cat_printf(
        instance->msg,
        "\tNOTE: Receive stats testing should be done in a controlled environment (RF shield box or chamber).\r\n");
    furi_string_cat_printf(instance->msg, "tx_start Transmit spectrum start\r\n");
    furi_string_cat_printf(instance->msg, "tx_stop Transmit spectrum stop\r\n");
    furi_string_cat_printf(instance->msg, "set_power <2..18|127> Set transmit power.\r\n");
    furi_string_cat_printf(
        instance->msg,
        "set_rate <1|2|5.5|11|6|9|12|18|24|36|48|54|MCS0|MCS1|MCS2|MCS3|MCS4|MCS5|MCS6|MCS7|MCS7_SG> Set transmit rate.\r\n");
    furi_string_cat_printf(instance->msg, "set_channel <1..14> Set transmit channel.\r\n");
    furi_string_cat_printf(
        instance->msg, "set_mode <burst|continuous|cw|cw_low|cw_high> Set transmit mode.\r\n");

    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "***********************************************\r\n");
    wifi_rf_test_app_send_msg(instance);
}
