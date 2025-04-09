#include "wifi_sta_cli.h"
#include <intercom/intercom.h>
#include <furi_hal_cortex.h>
#include <args.h>
#include <strint.h>

#define TAG                   "WifiStaCli"
#define CLI_BUFFER_SIZE       (1024U)
#define CLI_READ_TIMEOUT      (10U)
#define CLI_START_APP_TIMEOUT (15000000U) // 15 seconds

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    uint8_t rx_data[CLI_BUFFER_SIZE];
    FuriThread* thread;
    FuriString* rx_msg;
    WifiStaTest* app_handle;
    FuriString* sta_ip_addr_str;
} CliCommandSlCli;

typedef enum {
    WifiStaCliCmdTypeStartApp,
    WifiStaCliCmdTypeEndApp,
    WifiStaCliCmdTypeScan,
    WifiStaCliCmdTypeApUp,
    WifiStaCliCmdTypeApDown,
    WifiStaCliCmdTypeStaUp,
    WifiStaCliCmdTypeStaDown,
    WifiStaCliCmdTypeTestTcpRx,
    WifiStaCliCmdTypeTestTcpTx,
    WifiStaCliCmdTypeTestEcho,

    WifiStaCliCmdTypeMax,
} WifiStaCliCmdType;

typedef enum {
    WifiStaCliThreadEventStop = (1 << 0),
    WifiStaCliThreadEventRxData = (1 << 1),
} WifiStaCliThreadEvent;

#define BLE_PER_CLI_THREAD_EVENT (WifiStaCliThreadEventStop | WifiStaCliThreadEventRxData)

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
WiFi test>: ************************************************************************************************************************************************************
*/

typedef struct {
    char* cmd;
} WifiStaCliCmd;

static const WifiStaCliCmd wifi_sta_cli_cmd[WifiStaCliCmdTypeMax] = {
    [WifiStaCliCmdTypeStartApp] = {"wifi_test"},
    [WifiStaCliCmdTypeEndApp] = {"^C"}, //Ctrl+C
    [WifiStaCliCmdTypeScan] = {"scan"},
    [WifiStaCliCmdTypeApUp] = {"ap_up"},
    [WifiStaCliCmdTypeApDown] = {"ap_down"},
    [WifiStaCliCmdTypeStaUp] = {"sta_up"},
    [WifiStaCliCmdTypeStaDown] = {"sta_down"},
    [WifiStaCliCmdTypeTestTcpTx] = {"test_tcp_tx"},
    [WifiStaCliCmdTypeTestTcpRx] = {"test_tcp_rx"},
    [WifiStaCliCmdTypeTestEcho] = {"test_echo"},

};

typedef enum {
    WifiStaCliStatsCmdTypeStartApp,
    WifiStaCliStatsCmdTypeIP,

    WifiStaCliStatsCmdMax,
} WifiStaCliStatsCmd;

typedef struct {
    char* cmd;
    int32_t value;
} WifiPerStats;

WifiPerStats wifi_sta_cli_stats[WifiStaCliStatsCmdMax] = {
    [WifiStaCliStatsCmdTypeStartApp] = {"start_app:"},
    [WifiStaCliStatsCmdTypeIP] = {"ip_address_of_client:"},
};

CliCommandSlCli* wifi_sta_cli_instance = NULL;

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
        furi_thread_get_id(wifi_sta_cli_instance->thread), WifiStaCliThreadEventRxData);
}

void wifi_sta_cli_data_tx(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size);

    const size_t tx_size = intercom_tx(
        wifi_sta_cli_instance->intercom, IntercomChannelCli, data, data_size, FuriWaitForever);
    furi_assert(tx_size == data_size);
}

bool wifi_sta_cli_parse_msg(FuriString* args, const char* suffix) {
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
        for(uint32_t i = 0; i < WifiStaCliStatsCmdMax; i++) {
            if(furi_string_cmp_str(arg, (char*)wifi_sta_cli_stats[i].cmd) == 0) {
                args_read_string_and_trim(args, arg);
                if(i == WifiStaCliStatsCmdTypeIP) {
                    furi_string_set(wifi_sta_cli_instance->sta_ip_addr_str, arg);
                    wifi_sta_cli_stats[i].value = 1;
                } else {
                    strint_to_int32(furi_string_get_cstr(arg), &args_cstr, &arg_int32, 10);
                    wifi_sta_cli_stats[i].value += arg_int32;
                }
                ret = true;
                break;
            }
        }

    } while(false);
    furi_string_free(arg);
    return ret;
}

static int32_t wifi_sta_cli_worker_thread(void* context) {
    CliCommandSlCli* instance = context;
    UNUSED(instance);
    FURI_LOG_I(TAG, "Start");
    while(1) {
        uint32_t events =
            furi_thread_flags_wait(BLE_PER_CLI_THREAD_EVENT, FuriFlagWaitAny, FuriWaitForever);

        if(events & WifiStaCliThreadEventRxData) {
            uint8_t data[CLI_BUFFER_SIZE];
            const size_t rx_size =
                furi_stream_buffer_receive(instance->rx_buffer, data, sizeof(data), 0);
            for(size_t i = 0; i < rx_size; i++) {
                if(data[i] != '\n' && data[i] != '\r') {
                    furi_string_push_back(instance->rx_msg, data[i]);
                } else {
                    if(wifi_sta_cli_parse_msg(instance->rx_msg, ">: ")) {
                    }

                    furi_string_reset(instance->rx_msg);
                }
            }
        }

        if(events & WifiStaCliThreadEventStop) {
            break;
        }
    }
    FURI_LOG_I(TAG, "Stop");

    return 0;
}

bool wifi_sta_cli_start(WifiStaTest* app_handle, WifiStaCliSettings settings) {
    UNUSED(settings);

    bool ret = false;
    if(wifi_sta_cli_instance != NULL) {
        return ret;
    }

    FuriHalCortexTimer wait = furi_hal_cortex_timer_get(CLI_START_APP_TIMEOUT);
    FuriString* msg = furi_string_alloc();

    wifi_sta_cli_instance = malloc(sizeof(CliCommandSlCli));
    wifi_sta_cli_instance->app_handle = app_handle;
    for(size_t i = 0; i < WifiStaCliStatsCmdMax; i++) {
        wifi_sta_cli_stats[i].value = 0;
    }
    wifi_sta_cli_instance->intercom = furi_record_open(RECORD_INTERCOM);

    intercom_set_rx_callback(
        wifi_sta_cli_instance->intercom,
        IntercomChannelCli,
        cli_command_917_rx_callback,
        wifi_sta_cli_instance);

    wifi_sta_cli_instance->rx_buffer = furi_stream_buffer_alloc(CLI_BUFFER_SIZE, 1);
    wifi_sta_cli_instance->rx_msg = furi_string_alloc();
    wifi_sta_cli_instance->sta_ip_addr_str = furi_string_alloc();
    wifi_sta_cli_instance->thread = furi_thread_alloc_ex(
        "WiFi_CLI_Worker", CLI_BUFFER_SIZE * 2, wifi_sta_cli_worker_thread, wifi_sta_cli_instance);
    furi_thread_start(wifi_sta_cli_instance->thread);

    wifi_sta_test_update(wifi_sta_cli_instance->app_handle, WifiStaTestStatusConnecting, NULL);

    //Start App
    furi_string_printf(msg, "%s\r\n", wifi_sta_cli_cmd[WifiStaCliCmdTypeStartApp].cmd);
    wifi_sta_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_RAW_D("\r\n ");
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    //wait for the app to start
    while(!furi_hal_cortex_timer_is_expired(wait)) {
        furi_thread_yield();
        if(wifi_sta_cli_stats[WifiStaCliStatsCmdTypeStartApp].value) {
            break;
        }
    };

    if(wifi_sta_cli_stats[WifiStaCliStatsCmdTypeStartApp].value) {
        furi_string_printf(msg, "%s\r\n", wifi_sta_cli_cmd[WifiStaCliCmdTypeStaUp].cmd);

        wifi_sta_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
        FURI_LOG_RAW_D("\r\n ");
        FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

        //wait for the app to start
        while(!furi_hal_cortex_timer_is_expired(wait)) {
            furi_thread_yield();

            if(wifi_sta_cli_stats[WifiStaCliStatsCmdTypeIP].value) {
                break;
            }
        };
        if(wifi_sta_cli_stats[WifiStaCliStatsCmdTypeIP].value) {
            FURI_LOG_RAW_D("\r\n ");
            FURI_LOG_D(TAG, "%s", furi_string_get_cstr(wifi_sta_cli_instance->sta_ip_addr_str));
            wifi_sta_test_update(
                wifi_sta_cli_instance->app_handle,
                WifiStaTestStatusConnected,
                wifi_sta_cli_instance->sta_ip_addr_str);

            ret = true;
        }
    }
    furi_string_free(msg);
    return ret;
}

void wifi_sta_cli_stop(void) {
    if(wifi_sta_cli_instance == NULL) {
        return;
    }

    FuriString* msg = furi_string_alloc();
    furi_string_printf(msg, "%s\r\n", wifi_sta_cli_cmd[WifiStaCliCmdTypeStaDown].cmd);
    wifi_sta_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    wifi_sta_test_update(wifi_sta_cli_instance->app_handle, WifiStaTestStatusDisconnected, NULL);
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    furi_delay_ms(1000); //wait for the app to stop

    furi_string_printf(msg, "%c\r\n", 0x03); //Ctrl+C
    wifi_sta_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    furi_delay_ms(1000); //wait for the app to stop
    FURI_LOG_D(TAG, "End APP");

    furi_string_free(msg);

    intercom_set_rx_callback(wifi_sta_cli_instance->intercom, IntercomChannelCli, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_thread_flags_set(
        furi_thread_get_id(wifi_sta_cli_instance->thread), WifiStaCliThreadEventStop);
    furi_thread_join(wifi_sta_cli_instance->thread);
    furi_thread_free(wifi_sta_cli_instance->thread);

    furi_string_free(wifi_sta_cli_instance->rx_msg);
    furi_string_free(wifi_sta_cli_instance->sta_ip_addr_str);

    furi_stream_buffer_free(wifi_sta_cli_instance->rx_buffer);
    free(wifi_sta_cli_instance);
    wifi_sta_cli_instance = NULL;
}
