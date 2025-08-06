#include "ble_per_cli.h"

#include <cli_intercom/cli_intercom.h>
#include <containers/pipe_util.h>

#include <furi_hal_cortex.h>
#include <cli/args.h>
#include <strint.h>

#define TAG                   "BlePerCli"
#define CLI_BUFFER_SIZE       (1024U)
#define CLI_READ_TIMEOUT      (10U)
#define CLI_START_APP_TIMEOUT (5000000U) // 5 seconds
#define TRANSFER_BATCH_SIZE   512UL

typedef struct {
    FuriStreamBuffer* rx_buffer;
    uint8_t rx_data[CLI_BUFFER_SIZE];
    FuriThread* thread;
    FuriThread* thread_event_loop;
    FuriString* rx_msg;
    BlePerTest* app_handle;

    CliIntercom* cli_intercom;
    PipeSide* own_pipe;
    PipeSide* shell_pipe;
    FuriEventLoop* event_loop;
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
    [BlePerCliCmdTypeEndApp] = {"exit"},
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

static void cli_uart_data_from_pipe(PipeSide* pipe, void* context) {
    CliCommandSlCli* instance = context;

    size_t bytes_in_pipe = pipe_bytes_available(pipe);
    size_t to_transfer = MIN(bytes_in_pipe, TRANSFER_BATCH_SIZE);

    uint8_t buffer[to_transfer];
    furi_check(pipe_receive(pipe, buffer, to_transfer) == to_transfer);
    furi_check(
        furi_stream_buffer_send(instance->rx_buffer, buffer, sizeof(buffer), FuriWaitForever) ==
        sizeof(buffer));
    furi_thread_flags_set(furi_thread_get_id(instance->thread), BlePerCliThreadEventRxData);
}

void ble_per_cli_data_tx(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size);

    furi_check(pipe_send(ble_per_cli_instance->own_pipe, data, data_size) == data_size);
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
                if(data[i] == '\r') {
                } else if(data[i] != '\n' && data[i] != 0x1B) {
                    furi_string_push_back(instance->rx_msg, data[i]);
                } else {
                    FURI_LOG_I(TAG, "Parsed args: %s", furi_string_get_cstr(instance->rx_msg));
                    if(ble_per_cli_parse_msg(instance->rx_msg, " ")) {
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
            furi_event_loop_stop(instance->event_loop);
            break;
        }
    }
    FURI_LOG_I(TAG, "Stop");

    return 0;
}

static void ble_per_cli_thread_event_loop_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        FURI_LOG_D(TAG, "Stop event loop");
    }
}

static int32_t ble_per_cli_thread_event_loop_callback(void* context) {
    furi_assert(context);
    CliCommandSlCli* instance = context;

    instance->event_loop = furi_event_loop_alloc();
    PipeSideBundle temp_bundle = pipe_alloc(CLI_BUFFER_SIZE, 1);
    instance->own_pipe = temp_bundle.alices_side;
    instance->shell_pipe = temp_bundle.bobs_side;

    CliIntercom* cli_intercom = furi_record_open(RECORD_CLI_INTERCOM);
    furi_check(cli_intercom_spawn(cli_intercom, instance->shell_pipe) == CliIntercomSpawnStatusOk);

    pipe_attach_to_event_loop(instance->own_pipe, instance->event_loop);
    pipe_set_callback_context(instance->own_pipe, instance);
    pipe_set_data_arrived_callback(instance->own_pipe, cli_uart_data_from_pipe, 0);

    FURI_LOG_D(TAG, "Start event loop");
    furi_event_loop_run(instance->event_loop);
    FURI_LOG_D(TAG, "Stopping event loop");

    pipe_set_data_arrived_callback(instance->own_pipe, NULL, 0);
    pipe_set_broken_callback(instance->own_pipe, NULL, 0);
    pipe_detach_from_event_loop(instance->own_pipe);
    pipe_free(instance->own_pipe);
    cli_intercom_join(cli_intercom);
    pipe_free(instance->shell_pipe);

    furi_event_loop_free(ble_per_cli_instance->event_loop);
    furi_record_close(RECORD_CLI_INTERCOM);
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

    ble_per_cli_instance->rx_buffer = furi_stream_buffer_alloc(CLI_BUFFER_SIZE, 1);
    ble_per_cli_instance->rx_msg = furi_string_alloc();
    ble_per_cli_instance->thread = furi_thread_alloc_ex(
        "BLE_CLI_Worker", CLI_BUFFER_SIZE * 2, ble_per_cli_worker_thread, ble_per_cli_instance);
    furi_thread_start(ble_per_cli_instance->thread);

    // Start event loop thread
    ble_per_cli_instance->thread_event_loop = furi_thread_alloc_ex(
        "BLE_CLI_event_loop",
        CLI_BUFFER_SIZE * 2,
        ble_per_cli_thread_event_loop_callback,
        ble_per_cli_instance);
    furi_thread_set_state_callback(
        ble_per_cli_instance->thread_event_loop, ble_per_cli_thread_event_loop_state_callback);
    furi_thread_start(ble_per_cli_instance->thread_event_loop);

    furi_delay_ms(100); // wait for the thread to start

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

    furi_delay_ms(500); //wait for the app to stop

    furi_string_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeEndApp].cmd);
    ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    furi_delay_ms(500); //wait for the app to stop
    FURI_LOG_D(TAG, "End APP");

    furi_string_free(msg);

    furi_thread_flags_set(
        furi_thread_get_id(ble_per_cli_instance->thread), BlePerCliThreadEventStop);
    furi_thread_join(ble_per_cli_instance->thread);
    furi_thread_free(ble_per_cli_instance->thread);

    furi_string_free(ble_per_cli_instance->rx_msg);

    furi_stream_buffer_free(ble_per_cli_instance->rx_buffer);
    free(ble_per_cli_instance);
    ble_per_cli_instance = NULL;
}
