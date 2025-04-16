#include "ble_per_cli.h"
#include <intercom/intercom.h>
#include <furi_hal_cortex.h>
#include <args.h>
#include <strint.h>

#define TAG                   "BlePerCli"
#define CLI_BUFFER_SIZE       (1024U)
#define CLI_READ_TIMEOUT      (10U)
#define CLI_START_APP_TIMEOUT (5000000U) // 5 seconds

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    uint8_t rx_data[CLI_BUFFER_SIZE];
    FuriThread* thread;
    FuriString* rx_msg;
    BlePerTest* app_handle;
} CliCommandSlCli;

typedef enum {
    BlePerCliCmdTypeStartApp,
    BlePerCliCmdTypeEndApp,
    BlePerCliCmdTypeModeTx,
    BlePerCliCmdTypeModeRx,
    BlePerCliCmdTypeModeTxRxStop,
    BlePerCliCmdTypeSetChannel,
    BlePerCliCmdTypeSetPhyRate,
    BlePerCliCmdTypeSetPayloadLen,
    BlePerCliCmdTypeSetPayloadType,
    BlePerCliCmdTypeSetMode,
    BlePerCliCmdTypeSetHopping,
    BlePerCliCmdTypeSetTxPower,

    BlePerCliCmdTypeMax,
} BlePerCliCmdType;

typedef enum {
    BlePerCliThreadEventStop = (1 << 0),
    BlePerCliThreadEventRxData = (1 << 1),
} BlePerCliThreadEvent;

#define BLE_PER_CLI_THREAD_EVENT (BlePerCliThreadEventStop | BlePerCliThreadEventRxData)

/*
    BLE PER test>: Tests the BLE PER peripheral role.
    BLE PER test>: https://github.com/SiliconLabs/wiseconnect/tree/master/examples/featured/ble_per
    BLE PER test>: ?
    BLE PER test>: help
    BLE PER test>: tx Tx Start
    BLE PER test>: rx Rx Start
    BLE PER test>: stop Tx/Rx Stop
    BLE PER test>: channel <0..39> BLE channels 2402MHz to 2480MHz with 2MHz spacing
    BLE PER test>: phy_rate <0..4> PHY 0: 1Mbps, 1: 2Mbps, 2: 125Kbps, 3: 500Kbps
    BLE PER test>: payload_len <1..255> Payload length
    BLE PER test>: payload_type <0..7> Payload type 0: PRBS9, 1: 11110000, 2: 10101010, 3: PRBS15, 4: 11111111, 5: 00000000, 6: 00001111, 7: 01010101
    BLE PER test>: mode <0..3> Transmit mode 0: Burst, 1: Continuous 2: Cw
    BLE PER test>: hopping <0..2> Frequency hopping 0: No hopping, 1: Fixed Hopping, 2: Random Hopping
    BLE PER test>: tx_power <1..10 | 127> Transmit power 1..10: 1dBm..10dBm, 127: Max Power Supported by Country regio
*/

typedef struct {
    char* cmd;
} BlePerCliCmd;

static const BlePerCliCmd ble_per_cli_cmd[BlePerCliCmdTypeMax] = {
    [BlePerCliCmdTypeStartApp] = {"ble_per_test"},
    [BlePerCliCmdTypeEndApp] = {"^C"}, //Ctrl+C
    [BlePerCliCmdTypeModeTx] = {"tx"},
    [BlePerCliCmdTypeModeRx] = {"rx"},
    [BlePerCliCmdTypeModeTxRxStop] = {"stop"},
    [BlePerCliCmdTypeSetChannel] = {"channel"},
    [BlePerCliCmdTypeSetPhyRate] = {"phy_rate"},
    [BlePerCliCmdTypeSetPayloadLen] = {"payload_len"},
    [BlePerCliCmdTypeSetPayloadType] = {"payload_type"},
    [BlePerCliCmdTypeSetMode] = {"mode"},
    [BlePerCliCmdTypeSetHopping] = {"hopping"},
    [BlePerCliCmdTypeSetTxPower] = {"tx_power"},
};

typedef enum {
    BlePerCliStatsCmdTypeStartApp,
    BlePerCliStatsCmdTypeTxDones,
    BlePerCliStatsCmdTypeCrcFailCnt,
    BlePerCliStatsCmdTypeCrcPassCnt,
    BlePerCliStatsCmdTypeRssi,

    BlePerCliStatsCmdMax,
} BlePerCliStatsCmd;

typedef struct {
    char* cmd;
    int32_t value;
} BlePerStats;

BlePerStats ble_per_cli_stats[BlePerCliStatsCmdMax] = {
    [BlePerCliStatsCmdTypeStartApp] = {"start_app:"},
    [BlePerCliStatsCmdTypeTxDones] = {"tx_dones:"},
    [BlePerCliStatsCmdTypeCrcFailCnt] = {"crc_fail_cnt:"},
    [BlePerCliStatsCmdTypeCrcPassCnt] = {"crc_pass_cnt:"},
    [BlePerCliStatsCmdTypeRssi] = {"rssi:"},
};

CliCommandSlCli* ble_per_cli_instance = NULL;

static void cli_command_917_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);

    CliCommandSlCli* instance = context;
    UNUSED(instance);
    UNUSED(data_size);
    for(size_t i = 0; i < data_size; i++) {
        FURI_LOG_RAW_E("%c", ((uint8_t*)data)[i]);
    }
    furi_check(
        furi_stream_buffer_send(instance->rx_buffer, data, data_size, FuriWaitForever) ==
        data_size);
    furi_thread_flags_set(
        furi_thread_get_id(ble_per_cli_instance->thread), BlePerCliThreadEventRxData);
}

void ble_per_cli_data_tx(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size);

    const size_t tx_size = intercom_tx(
        ble_per_cli_instance->intercom, IntercomChannelCli, data, data_size, FuriWaitForever);
    furi_assert(tx_size == data_size);
}

bool ble_per_cli_parse_msg(FuriString* args, const char* suffix) {
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
        for(uint32_t i = 0; i < BlePerCliStatsCmdMax; i++) {
            if(furi_string_cmp_str(arg, (char*)ble_per_cli_stats[i].cmd) == 0) {
                args_read_string_and_trim(args, arg);
                strint_to_int32(furi_string_get_cstr(arg), &args_cstr, &arg_int32, 10);
                if(i != BlePerCliStatsCmdTypeRssi) {
                    ble_per_cli_stats[i].value += arg_int32;
                } else {
                    //Rssi not needs to be savings
                    ble_per_cli_stats[i].value = arg_int32;
                }

                ret = true;
                break;
            }
        }

    } while(false);
    furi_string_free(arg);
    return ret;
}

static int32_t ble_per_cli_worker_thread(void* context) {
    CliCommandSlCli* instance = context;
    UNUSED(instance);
    FURI_LOG_I(TAG, "Start");
    while(1) {
        uint32_t events =
            furi_thread_flags_wait(BLE_PER_CLI_THREAD_EVENT, FuriFlagWaitAny, FuriWaitForever);

        if(events & BlePerCliThreadEventRxData) {
            uint8_t data[CLI_BUFFER_SIZE];
            const size_t rx_size =
                furi_stream_buffer_receive(instance->rx_buffer, data, sizeof(data), 0);
            for(size_t i = 0; i < rx_size; i++) {
                if(data[i] != '\n' && data[i] != '\r') {
                    furi_string_push_back(instance->rx_msg, data[i]);
                } else {
                    if(ble_per_cli_parse_msg(instance->rx_msg, ">: ")) {
                        ble_per_test_update(
                            instance->app_handle,
                            ble_per_cli_stats[BlePerCliStatsCmdTypeTxDones].value,
                            ble_per_cli_stats[BlePerCliStatsCmdTypeCrcFailCnt].value,
                            ble_per_cli_stats[BlePerCliStatsCmdTypeCrcPassCnt].value,
                            ble_per_cli_stats[BlePerCliStatsCmdTypeRssi].value);
                    }
                    furi_string_reset(instance->rx_msg);
                }
            }
        }

        if(events & BlePerCliThreadEventStop) {
            break;
        }
    }
    FURI_LOG_I(TAG, "Stop");

    return 0;
}

bool ble_per_cli_start(BlePerTest* app_handle, BlePerCliSettings settings) {
    UNUSED(settings);

    bool ret = false;
    if(ble_per_cli_instance != NULL) {
        return ret;
    }

    FuriHalCortexTimer wait = furi_hal_cortex_timer_get(CLI_START_APP_TIMEOUT);
    FuriString* msg = furi_string_alloc();

    ble_per_cli_instance = malloc(sizeof(CliCommandSlCli));
    ble_per_cli_instance->app_handle = app_handle;
    for(size_t i = 0; i < BlePerCliStatsCmdMax; i++) {
        ble_per_cli_stats[i].value = 0;
    }
    ble_per_cli_instance->intercom = furi_record_open(RECORD_INTERCOM);

    intercom_set_rx_callback(
        ble_per_cli_instance->intercom,
        IntercomChannelCli,
        cli_command_917_rx_callback,
        ble_per_cli_instance);

    ble_per_cli_instance->rx_buffer = furi_stream_buffer_alloc(CLI_BUFFER_SIZE, 1);
    ble_per_cli_instance->rx_msg = furi_string_alloc();
    ble_per_cli_instance->thread = furi_thread_alloc_ex(
        "BLE_CLI_Worker", CLI_BUFFER_SIZE * 2, ble_per_cli_worker_thread, ble_per_cli_instance);
    furi_thread_start(ble_per_cli_instance->thread);

    //Start App
    furi_string_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeStartApp].cmd);
    ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    //wait for the app to start
    while(!furi_hal_cortex_timer_is_expired(wait)) {
        furi_thread_yield();
        if(ble_per_cli_stats[BlePerCliStatsCmdTypeStartApp].value) {
            break;
        }
    };

    if(ble_per_cli_stats[BlePerCliStatsCmdTypeStartApp].value) {
        //set settings
        furi_string_printf(
            msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetChannel].cmd, settings.channel);
        furi_string_cat_printf(
            msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetPhyRate].cmd, settings.rate);
        furi_string_cat_printf(
            msg,
            "%s %d\r\n",
            ble_per_cli_cmd[BlePerCliCmdTypeSetPayloadLen].cmd,
            settings.payload_len);
        furi_string_cat_printf(
            msg,
            "%s %d\r\n",
            ble_per_cli_cmd[BlePerCliCmdTypeSetPayloadType].cmd,
            settings.payload_type);
        furi_string_cat_printf(
            msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetMode].cmd, settings.mode_work);
        furi_string_cat_printf(
            msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetHopping].cmd, settings.hopping);
        furi_string_cat_printf(
            msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetTxPower].cmd, settings.tx_power);

        if(settings.mode == BLEPerCliSettingsModeTx) {
            furi_string_cat_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeModeTx].cmd);
        } else if(settings.mode == BLEPerCliSettingsModeRx) {
            furi_string_cat_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeModeRx].cmd);
        } else {
            furi_crash("Invalid mode");
        }

        ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
        FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

        ble_per_test_update(
            app_handle,
            ble_per_cli_stats[BlePerCliStatsCmdTypeTxDones].value,
            ble_per_cli_stats[BlePerCliStatsCmdTypeCrcFailCnt].value,
            ble_per_cli_stats[BlePerCliStatsCmdTypeCrcPassCnt].value,
            ble_per_cli_stats[BlePerCliStatsCmdTypeRssi].value);

        ret = true;
    }

    furi_string_free(msg);
    return ret;
}

void ble_per_cli_stop(void) {
    if(ble_per_cli_instance == NULL) {
        return;
    }

    FuriString* msg = furi_string_alloc();
    furi_string_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeModeTxRxStop].cmd);
    ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    furi_delay_ms(1000); //wait for the app to stop

    furi_string_printf(msg, "%c\r\n", 0x03); //Ctrl+C
    ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    furi_delay_ms(1000); //wait for the app to stop
    FURI_LOG_D(TAG, "End APP");

    furi_string_free(msg);

    intercom_set_rx_callback(ble_per_cli_instance->intercom, IntercomChannelCli, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_thread_flags_set(
        furi_thread_get_id(ble_per_cli_instance->thread), BlePerCliThreadEventStop);
    furi_thread_join(ble_per_cli_instance->thread);
    furi_thread_free(ble_per_cli_instance->thread);

    furi_string_free(ble_per_cli_instance->rx_msg);

    furi_stream_buffer_free(ble_per_cli_instance->rx_buffer);
    free(ble_per_cli_instance);
    ble_per_cli_instance = NULL;
}
