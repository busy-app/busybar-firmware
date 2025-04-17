#include "wifi_adaptive_cli.h"
#include <intercom/intercom.h>
#include <furi_hal_cortex.h>
#include <args.h>
#include <strint.h>

#define TAG                   "WifiAdaptiveCli"
#define CLI_BUFFER_SIZE       (1024U)
#define CLI_READ_TIMEOUT      (10U)
#define CLI_START_APP_TIMEOUT (15000000U) // 15 seconds

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    uint8_t rx_data[CLI_BUFFER_SIZE];
    FuriThread* thread;
    FuriString* rx_msg;
    WifiAdaptiveTest* app_handle;
    FuriString* sta_ip_addr_str;
} CliCommandSlCli;

typedef enum {
    WifiAdaptiveCliCmdTypeStartApp,
    WifiAdaptiveCliCmdTypeEndApp,
    WifiAdaptiveCliCmdTypeScan,
    WifiAdaptiveCliCmdTypeApUp,
    WifiAdaptiveCliCmdTypeApDown,
    WifiAdaptiveCliCmdTypeStaUp,
    WifiAdaptiveCliCmdTypeStaDown,
    WifiAdaptiveCliCmdTypeTestTcpRx,
    WifiAdaptiveCliCmdTypeTestTcpTx,
    WifiAdaptiveCliCmdTypeTestEcho,
    WifiAdaptiveCliCmdTypeTestUdpTx,
    WifiAdaptiveCliCmdTypeTestUdpTxStop,

    WifiAdaptiveCliCmdTypeMax,
} WifiAdaptiveCliCmdType;

typedef enum {
    WifiAdaptiveCliThreadEventStop = (1 << 0),
    WifiAdaptiveCliThreadEventRxData = (1 << 1),
} WifiAdaptiveCliThreadEvent;

#define BLE_PER_CLI_THREAD_EVENT \
    (WifiAdaptiveCliThreadEventStop | WifiAdaptiveCliThreadEventRxData)

/*
WiFi test>: Wi-Fi APSTA interface init
WiFi test>: WifiTestApp commands usage:
WiFi test>: ************************************************************************************************************************************************************
WiFi test>: ?
WiFi test>: help
WiFi test>: scan WiFi scan ap. Scanning is possible when the access point is not running
WiFi test>: ap_up Start AP.
WiFi test>: ap_down Stop AP.
WiFi test>: sta_up Start STA. SSID:Zyxel24 PASS:1qa2wszz
WiFi test>: sta_down Stop STA.
WiFi test>: test_tcp_tx [ip] Start TCP TX iPref test "iperf.exe -s -p 5000 -i 1". Default IP:192.168.10.2
WiFi test>: test_tcp_rx Start TCP RX iPref test "iperf.exe -c 192.168.11.10 -p 5005 -i 1 -b70M -t 30".
WiFi test>: test_echo Start TCP echo test port 5005. Work time 90 sec
WiFi test>: test_udp_tx [ip] Start UDP TX iPref test "iperf.exe -s -u -p 5001 -i 1". Default IP:192.168.10.2 Work time 5 min
WiFi test>: test_udp_tx_stop Stop UDP TX iPref test
WiFi test>: ************************************************************************************************************************************************************
*/

typedef struct {
    char* cmd;
} WifiAdaptiveCliCmd;

static const WifiAdaptiveCliCmd wifi_adaptive_cli_cmd[WifiAdaptiveCliCmdTypeMax] = {
    [WifiAdaptiveCliCmdTypeStartApp] = {"wifi_test"},
    [WifiAdaptiveCliCmdTypeEndApp] = {"^C"}, //Ctrl+C
    [WifiAdaptiveCliCmdTypeScan] = {"scan"},
    [WifiAdaptiveCliCmdTypeApUp] = {"ap_up"},
    [WifiAdaptiveCliCmdTypeApDown] = {"ap_down"},
    [WifiAdaptiveCliCmdTypeStaUp] = {"sta_up"},
    [WifiAdaptiveCliCmdTypeStaDown] = {"sta_down"},
    [WifiAdaptiveCliCmdTypeTestTcpTx] = {"test_tcp_tx"},
    [WifiAdaptiveCliCmdTypeTestTcpRx] = {"test_tcp_rx"},
    [WifiAdaptiveCliCmdTypeTestEcho] = {"test_echo"},
    [WifiAdaptiveCliCmdTypeTestUdpTx] = {"test_udp_tx"},
    [WifiAdaptiveCliCmdTypeTestUdpTxStop] = {"test_udp_tx_stop"},

};

typedef enum {
    WifiAdaptiveCliStatsCmdTypeStartApp,
    WifiAdaptiveCliStatsCmdTypeIP,

    WifiAdaptiveCliStatsCmdMax,
} WifiAdaptiveCliStatsCmd;

typedef struct {
    char* cmd;
    int32_t value;
} WifiPerStats;

WifiPerStats wifi_adaptive_cli_stats[WifiAdaptiveCliStatsCmdMax] = {
    [WifiAdaptiveCliStatsCmdTypeStartApp] = {"start_app:"},
    [WifiAdaptiveCliStatsCmdTypeIP] = {"ip_address_of_client:"},
};

CliCommandSlCli* wifi_adaptive_cli_instance = NULL;

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
        furi_thread_get_id(wifi_adaptive_cli_instance->thread), WifiAdaptiveCliThreadEventRxData);
}

void wifi_adaptive_cli_data_tx(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size);

    const size_t tx_size = intercom_tx(
        wifi_adaptive_cli_instance->intercom, IntercomChannelCli, data, data_size, FuriWaitForever);
    furi_assert(tx_size == data_size);
}

bool wifi_adaptive_cli_parse_msg(FuriString* args, const char* suffix) {
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
        for(uint32_t i = 0; i < WifiAdaptiveCliStatsCmdMax; i++) {
            if(furi_string_cmp_str(arg, (char*)wifi_adaptive_cli_stats[i].cmd) == 0) {
                args_read_string_and_trim(args, arg);
                if(i == WifiAdaptiveCliStatsCmdTypeIP) {
                    furi_string_set(wifi_adaptive_cli_instance->sta_ip_addr_str, arg);
                    wifi_adaptive_cli_stats[i].value = 1;
                } else {
                    strint_to_int32(furi_string_get_cstr(arg), &args_cstr, &arg_int32, 10);
                    wifi_adaptive_cli_stats[i].value += arg_int32;
                }
                ret = true;
                break;
            }
        }

    } while(false);
    furi_string_free(arg);
    return ret;
}

static int32_t wifi_adaptive_cli_worker_thread(void* context) {
    CliCommandSlCli* instance = context;
    UNUSED(instance);
    FURI_LOG_I(TAG, "Start");
    while(1) {
        uint32_t events =
            furi_thread_flags_wait(BLE_PER_CLI_THREAD_EVENT, FuriFlagWaitAny, FuriWaitForever);

        if(events & WifiAdaptiveCliThreadEventRxData) {
            uint8_t data[CLI_BUFFER_SIZE];
            const size_t rx_size =
                furi_stream_buffer_receive(instance->rx_buffer, data, sizeof(data), 0);
            for(size_t i = 0; i < rx_size; i++) {
                if(data[i] != '\n' && data[i] != '\r') {
                    furi_string_push_back(instance->rx_msg, data[i]);
                } else {
                    if(wifi_adaptive_cli_parse_msg(instance->rx_msg, ">: ")) {
                    }

                    furi_string_reset(instance->rx_msg);
                }
            }
        }

        if(events & WifiAdaptiveCliThreadEventStop) {
            break;
        }
    }
    FURI_LOG_I(TAG, "Stop");

    return 0;
}

bool wifi_adaptive_cli_start(WifiAdaptiveTest* app_handle, WifiAdaptiveCliSettings settings) {
    UNUSED(settings);

    bool ret = false;
    if(wifi_adaptive_cli_instance != NULL) {
        return ret;
    }

    FuriHalCortexTimer wait = furi_hal_cortex_timer_get(CLI_START_APP_TIMEOUT);
    FuriString* msg = furi_string_alloc();

    wifi_adaptive_cli_instance = malloc(sizeof(CliCommandSlCli));
    wifi_adaptive_cli_instance->app_handle = app_handle;
    for(size_t i = 0; i < WifiAdaptiveCliStatsCmdMax; i++) {
        wifi_adaptive_cli_stats[i].value = 0;
    }
    wifi_adaptive_cli_instance->intercom = furi_record_open(RECORD_INTERCOM);

    intercom_set_rx_callback(
        wifi_adaptive_cli_instance->intercom,
        IntercomChannelCli,
        cli_command_917_rx_callback,
        wifi_adaptive_cli_instance);

    wifi_adaptive_cli_instance->rx_buffer = furi_stream_buffer_alloc(CLI_BUFFER_SIZE, 1);
    wifi_adaptive_cli_instance->rx_msg = furi_string_alloc();
    wifi_adaptive_cli_instance->sta_ip_addr_str = furi_string_alloc();
    wifi_adaptive_cli_instance->thread = furi_thread_alloc_ex(
        "WiFi_CLI_Worker",
        CLI_BUFFER_SIZE * 2,
        wifi_adaptive_cli_worker_thread,
        wifi_adaptive_cli_instance);
    furi_thread_start(wifi_adaptive_cli_instance->thread);

    wifi_adaptive_test_update(
        wifi_adaptive_cli_instance->app_handle, WifiAdaptiveTestStatusConnecting, NULL);

    //Start App
    furi_string_printf(msg, "%s\r\n", wifi_adaptive_cli_cmd[WifiAdaptiveCliCmdTypeStartApp].cmd);
    wifi_adaptive_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_RAW_D("\r\n ");
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    //wait for the app to start
    while(!furi_hal_cortex_timer_is_expired(wait)) {
        furi_thread_yield();
        if(wifi_adaptive_cli_stats[WifiAdaptiveCliStatsCmdTypeStartApp].value) {
            break;
        }
    };

    if(wifi_adaptive_cli_stats[WifiAdaptiveCliStatsCmdTypeStartApp].value) {
        furi_string_printf(msg, "%s\r\n", wifi_adaptive_cli_cmd[WifiAdaptiveCliCmdTypeStaUp].cmd);

        wifi_adaptive_cli_data_tx(
            (uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
        FURI_LOG_RAW_D("\r\n ");
        FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

        //wait for the app to start
        while(!furi_hal_cortex_timer_is_expired(wait)) {
            furi_thread_yield();

            if(wifi_adaptive_cli_stats[WifiAdaptiveCliStatsCmdTypeIP].value) {
                break;
            }
        };
        if(wifi_adaptive_cli_stats[WifiAdaptiveCliStatsCmdTypeIP].value) {
            FURI_LOG_RAW_D("\r\n ");
            FURI_LOG_D(
                TAG, "%s", furi_string_get_cstr(wifi_adaptive_cli_instance->sta_ip_addr_str));
            wifi_adaptive_test_update(
                wifi_adaptive_cli_instance->app_handle,
                WifiAdaptiveTestStatusConnected,
                wifi_adaptive_cli_instance->sta_ip_addr_str);

            ret = true;
        }
    }

    furi_string_printf(
        msg, "%s %s\r\n", wifi_adaptive_cli_cmd[WifiAdaptiveCliCmdTypeTestUdpTx].cmd, settings.ip);
    wifi_adaptive_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));

    furi_string_free(msg);
    return ret;
}

void wifi_adaptive_cli_stop(void) {
    if(wifi_adaptive_cli_instance == NULL) {
        return;
    }

    FuriString* msg = furi_string_alloc();
    furi_string_printf(
        msg, "%s\r\n", wifi_adaptive_cli_cmd[WifiAdaptiveCliCmdTypeTestUdpTxStop].cmd);
    furi_string_cat_printf(
        msg, "%s\r\n", wifi_adaptive_cli_cmd[WifiAdaptiveCliCmdTypeStaDown].cmd);
    wifi_adaptive_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    wifi_adaptive_test_update(
        wifi_adaptive_cli_instance->app_handle, WifiAdaptiveTestStatusDisconnected, NULL);
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    furi_delay_ms(1000); //wait for the app to stop

    furi_string_printf(msg, "%c\r\n", 0x03); //Ctrl+C
    wifi_adaptive_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    furi_delay_ms(1000); //wait for the app to stop
    FURI_LOG_D(TAG, "End APP");

    furi_string_free(msg);

    intercom_set_rx_callback(wifi_adaptive_cli_instance->intercom, IntercomChannelCli, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_thread_flags_set(
        furi_thread_get_id(wifi_adaptive_cli_instance->thread), WifiAdaptiveCliThreadEventStop);
    furi_thread_join(wifi_adaptive_cli_instance->thread);
    furi_thread_free(wifi_adaptive_cli_instance->thread);

    furi_string_free(wifi_adaptive_cli_instance->rx_msg);
    furi_string_free(wifi_adaptive_cli_instance->sta_ip_addr_str);

    furi_stream_buffer_free(wifi_adaptive_cli_instance->rx_buffer);
    free(wifi_adaptive_cli_instance);
    wifi_adaptive_cli_instance = NULL;
}
