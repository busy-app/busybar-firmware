#include "wifi_per_cli.h"
#include <intercom/intercom.h>
#include <furi_hal_cortex.h>
#include <args.h>
#include <strint.h>

#define TAG                   "WifiPerCli"
#define CLI_BUFFER_SIZE       (1024U)
#define CLI_READ_TIMEOUT      (10U)
#define CLI_START_APP_TIMEOUT (5000000U) // 5 seconds

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    uint8_t rx_data[CLI_BUFFER_SIZE];
    FuriThread* thread;
    FuriString* rx_msg;
    WifiPerTest* app_handle;
} CliCommandSlCli;

typedef enum {
    WifiPerCliCmdTypeStartApp,
    WifiPerCliCmdTypeEndApp,
    WifiPerCliCmdTypeModeRx,
    WifiPerCliCmdTypeModeTx,
    WifiPerCliCmdTypeModeTxStop,
    WifiPerCliCmdTypeSetTxPower,
    WifiPerCliCmdTypeSetPhyRate,
    WifiPerCliCmdTypeSetChannel,
    WifiPerCliCmdTypeSetMode,

    WifiPerCliCmdTypeMax,
} WifiPerCliCmdType;

typedef enum {
    WifiPerCliThreadEventStop = (1 << 0),
    WifiPerCliThreadEventRxData = (1 << 1),
} WifiPerCliThreadEvent;

#define BLE_PER_CLI_THREAD_EVENT (WifiPerCliThreadEventStop | WifiPerCliThreadEventRxData)

/*
    WiFi rf test>: Wi-Fi initialization successful
WiFi rf test>: WifiRfTestApp commands usage:
WiFi rf test>: ************************************************************************************************************************************************************
WiFi rf test>: Read the manual: https://github.com/SiliconLabs/wiseconnect/tree/master/examples/snippets/wlan/wlan_rf_test
WiFi rf test>: ?
WiFi rf test>: help
WiFi rf test>: rx [stats_count] Receive stats testing, Number of iterations for which the receive stats would be displayed
WiFi rf test>:  NOTE: Receive stats testing should be done in a controlled environment (RF shield box or chamber).
WiFi rf test>: tx_start Transmit spectrum start
WiFi rf test>: tx_stop Transmit spectrum stop
WiFi rf test>: set_power <2..18|127> Set transmit power.
WiFi rf test>: set_rate <1|2|5.5|11|6|9|12|18|24|36|48|54|MCS0|MCS1|MCS2|MCS3|MCS4|MCS5|MCS6|MCS7|MCS7_SG> Set transmit rate.
WiFi rf test>: set_channel <1..14> Set transmit channel.
WiFi rf test>: set_mode <burst|continuous|cw|cw_low|cw_high> Set transmit mode.
WiFi rf test>: ************************************************************************************************************************************************************
*/

typedef struct {
    char* cmd;
} WifiPerCliCmd;

static const WifiPerCliCmd wifi_per_cli_cmd[WifiPerCliCmdTypeMax] = {
    [WifiPerCliCmdTypeStartApp] = {"wifi_rf_test"},
    [WifiPerCliCmdTypeEndApp] = {"^C"}, //Ctrl+C
    [WifiPerCliCmdTypeModeRx] = {"rx"},
    [WifiPerCliCmdTypeModeTx] = {"tx_start"},
    [WifiPerCliCmdTypeModeTxStop] = {"tx_stop"},
    [WifiPerCliCmdTypeSetTxPower] = {"set_power"},
    [WifiPerCliCmdTypeSetPhyRate] = {"set_rate"},
    [WifiPerCliCmdTypeSetChannel] = {"set_channel"},
    [WifiPerCliCmdTypeSetMode] = {"set_mode"},

};

typedef enum {
    WifiPerCliStatsCmdTypeStartApp,
    WifiPerCliStatsCmdTypeTxDones,
    WifiPerCliStatsCmdTypeCrcFailCnt,
    WifiPerCliStatsCmdTypeCrcPassCnt,
    WifiPerCliStatsCmdTypeRssi,

    WifiPerCliStatsCmdMax,
} WifiPerCliStatsCmd;

typedef struct {
    char* cmd;
    int32_t value;
} WifiPerStats;

WifiPerStats wifi_per_cli_stats[WifiPerCliStatsCmdMax] = {
    [WifiPerCliStatsCmdTypeStartApp] = {"start_app:"},
    [WifiPerCliStatsCmdTypeTxDones] = {"tx_dones:"},
    [WifiPerCliStatsCmdTypeCrcFailCnt] = {"crc_fail_cnt:"},
    [WifiPerCliStatsCmdTypeCrcPassCnt] = {"crc_pass_cnt:"},
    [WifiPerCliStatsCmdTypeRssi] = {"rssi:"},
};

CliCommandSlCli* wifi_per_cli_instance = NULL;

static void cli_command_917_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);

    CliCommandSlCli* instance = context;
    for(size_t i = 0; i < data_size; i++) {
        FURI_LOG_RAW_E("%c", ((uint8_t*)data)[i]);
    }
    furi_check(
        furi_stream_buffer_send(instance->rx_buffer, data, data_size, FuriWaitForever) ==
        data_size);
    furi_thread_flags_set(
        furi_thread_get_id(wifi_per_cli_instance->thread), WifiPerCliThreadEventRxData);
}

void wifi_per_cli_data_tx(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size);

    const size_t tx_size = intercom_tx(
        wifi_per_cli_instance->intercom, IntercomChannelCli, data, data_size, FuriWaitForever);
    furi_assert(tx_size == data_size);
}

bool wifi_per_cli_parse_msg(FuriString* args, const char* suffix) {
    furi_check(args);
    furi_check(suffix);
    FuriString* arg = furi_string_alloc();
    int32_t arg_int32 = 0;
    char* args_cstr = (char*)furi_string_get_cstr(args);
    bool ret = false;

    furi_string_right(args, furi_string_search_str(args, suffix, 0) + strlen(suffix));

    do {
        if(!args_read_string_and_trim(args, arg)) {
            break;
        }
        //update stats
        for(uint32_t i = 0; i < WifiPerCliStatsCmdMax; i++) {
            if(furi_string_cmp_str(arg, (char*)wifi_per_cli_stats[i].cmd) == 0) {
                args_read_string_and_trim(args, arg);
                strint_to_int32(furi_string_get_cstr(arg), &args_cstr, &arg_int32, 10);
                if(i != WifiPerCliStatsCmdTypeRssi) {
                    wifi_per_cli_stats[i].value += arg_int32;
                } else {
                    //Rssi not needs to be savings
                    wifi_per_cli_stats[i].value = arg_int32;
                }

                ret = true;
                break;
            }
        }

    } while(false);
    furi_string_free(arg);
    return ret;
}

static int32_t wifi_per_cli_worker_thread(void* context) {
    CliCommandSlCli* instance = context;
    UNUSED(instance);
    FURI_LOG_I(TAG, "Start");
    while(1) {
        uint32_t events =
            furi_thread_flags_wait(BLE_PER_CLI_THREAD_EVENT, FuriFlagWaitAny, FuriWaitForever);

        if(events & WifiPerCliThreadEventRxData) {
            uint8_t data[CLI_BUFFER_SIZE];
            const size_t rx_size =
                furi_stream_buffer_receive(instance->rx_buffer, data, sizeof(data), 0);
            for(size_t i = 0; i < rx_size; i++) {
                if(data[i] != '\n' && data[i] != '\r') {
                    furi_string_push_back(instance->rx_msg, data[i]);
                } else {
                    if(wifi_per_cli_parse_msg(instance->rx_msg, ">: ")) {
                        wifi_per_test_update(
                            instance->app_handle,
                            wifi_per_cli_stats[WifiPerCliStatsCmdTypeTxDones].value,
                            wifi_per_cli_stats[WifiPerCliStatsCmdTypeCrcFailCnt].value,
                            wifi_per_cli_stats[WifiPerCliStatsCmdTypeCrcPassCnt].value,
                            wifi_per_cli_stats[WifiPerCliStatsCmdTypeRssi].value);
                    }

                    furi_string_reset(instance->rx_msg);
                }
            }
        }

        if(events & WifiPerCliThreadEventStop) {
            break;
        }
    }
    FURI_LOG_I(TAG, "Stop");

    return 0;
}

bool wifi_per_cli_start(WifiPerTest* app_handle, WifiPerCliSettings settings) {
    UNUSED(settings);

    bool ret = false;
    if(wifi_per_cli_instance != NULL) {
        return ret;
    }

    FuriHalCortexTimer wait = furi_hal_cortex_timer_get(CLI_START_APP_TIMEOUT);
    FuriString* msg = furi_string_alloc();
    wifi_per_cli_instance = malloc(sizeof(CliCommandSlCli));
    wifi_per_cli_instance->app_handle = app_handle;
    for(size_t i = 0; i < WifiPerCliStatsCmdMax; i++) {
        wifi_per_cli_stats[i].value = 0;
    }
    wifi_per_cli_instance->intercom = furi_record_open(RECORD_INTERCOM);

    intercom_set_rx_callback(
        wifi_per_cli_instance->intercom,
        IntercomChannelCli,
        cli_command_917_rx_callback,
        wifi_per_cli_instance);

    wifi_per_cli_instance->rx_buffer = furi_stream_buffer_alloc(CLI_BUFFER_SIZE, 1);
    wifi_per_cli_instance->rx_msg = furi_string_alloc();
    wifi_per_cli_instance->thread = furi_thread_alloc_ex(
        "WiFi_CLI_Worker", CLI_BUFFER_SIZE * 2, wifi_per_cli_worker_thread, wifi_per_cli_instance);
    furi_thread_start(wifi_per_cli_instance->thread);

    wifi_per_test_update(wifi_per_cli_instance->app_handle, 0, 0, 0, 0);

    //Start App
    furi_string_printf(msg, "%s\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeStartApp].cmd);
    wifi_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    //wait for the app to start
    while(!furi_hal_cortex_timer_is_expired(wait)) {
        furi_thread_yield();
        if(wifi_per_cli_stats[WifiPerCliStatsCmdTypeStartApp].value) {
            break;
        }
    };

    if(wifi_per_cli_stats[WifiPerCliStatsCmdTypeStartApp].value) {
        //set settings
        furi_string_cat_printf(
            msg, "%s %d\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeSetTxPower].cmd, settings.tx_power);
        furi_string_cat_printf(
            msg, "%s %s\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeSetPhyRate].cmd, settings.rate);
        furi_string_printf(
            msg, "%s %d\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeSetChannel].cmd, settings.channel);
        furi_string_cat_printf(
            msg, "%s %s\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeSetMode].cmd, settings.mode_work);

        if(settings.mode == BLEPerCliSettingsModeTx) {
            furi_string_cat_printf(msg, "%s\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeModeTx].cmd);
        } else if(settings.mode == BLEPerCliSettingsModeRx) {
            furi_string_cat_printf(
                msg, "%s 1000\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeModeRx].cmd);
        } else {
            furi_crash("Invalid mode");
        }

        wifi_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
        FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

        wifi_per_test_update(
            app_handle,
            wifi_per_cli_stats[WifiPerCliStatsCmdTypeTxDones].value,
            wifi_per_cli_stats[WifiPerCliStatsCmdTypeCrcFailCnt].value,
            wifi_per_cli_stats[WifiPerCliStatsCmdTypeCrcPassCnt].value,
            wifi_per_cli_stats[WifiPerCliStatsCmdTypeRssi].value);

        ret = true;
    }
    furi_string_free(msg);
    return ret;
}

void wifi_per_cli_stop(void) {
    if(wifi_per_cli_instance == NULL) {
        return;
    }

    FuriString* msg = furi_string_alloc();
    furi_string_printf(msg, "%s\r\n", wifi_per_cli_cmd[WifiPerCliCmdTypeModeTxStop].cmd);
    wifi_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    furi_delay_ms(1000); //wait for the app to stop

    furi_string_printf(msg, "%c\r\n", 0x03); //Ctrl+C
    wifi_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "End APP");

    furi_string_free(msg);

    intercom_set_rx_callback(wifi_per_cli_instance->intercom, IntercomChannelCli, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_thread_flags_set(
        furi_thread_get_id(wifi_per_cli_instance->thread), WifiPerCliThreadEventStop);
    furi_thread_join(wifi_per_cli_instance->thread);
    furi_thread_free(wifi_per_cli_instance->thread);

    furi_string_free(wifi_per_cli_instance->rx_msg);

    furi_stream_buffer_free(wifi_per_cli_instance->rx_buffer);
    free(wifi_per_cli_instance);
    wifi_per_cli_instance = NULL;
}
