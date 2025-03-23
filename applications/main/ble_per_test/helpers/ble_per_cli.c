#include "ble_per_cli.h"
#include <intercom/intercom.h>

#define TAG "BlePerCli"
#define CLI_BUFFER_SIZE  (1024U)
#define CLI_READ_TIMEOUT (10U)

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    uint8_t rx_data[CLI_BUFFER_SIZE];

} CliCommandSlCli;

typedef enum{
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
    [BlePerCliCmdTypeEndApp] = {"^C"},  //Ctrl+C
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

CliCommandSlCli* ble_per_cli_instance = NULL;

static void cli_command_917_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);

    CliCommandSlCli* instance = context;
    UNUSED(instance);
    UNUSED(data_size);
    for(size_t i = 0; i < data_size; i++) {
        FURI_LOG_RAW_E("%c", ((uint8_t*) data)[i]);
    }
}

void ble_per_cli_data_tx(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size);

    const size_t tx_size = intercom_tx(
        ble_per_cli_instance->intercom, IntercomChannelCli, data, data_size, FuriWaitForever);
    furi_assert(tx_size == data_size);

}


void ble_per_cli_start(BlePerCliSettings settings) {
    UNUSED(settings);
    if(ble_per_cli_instance != NULL) {
        return;
    }

    ble_per_cli_instance = malloc(sizeof(CliCommandSlCli));

    ble_per_cli_instance->intercom = furi_record_open(RECORD_INTERCOM);
    ble_per_cli_instance->rx_buffer = furi_stream_buffer_alloc(CLI_BUFFER_SIZE, 1);

    intercom_set_rx_callback(
        ble_per_cli_instance->intercom, IntercomChannelCli, cli_command_917_rx_callback, ble_per_cli_instance);

    FuriString* msg = furi_string_alloc();
    //Start App
    furi_string_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeStartApp].cmd);
    ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));
    furi_delay_ms(3000); //wait for the app to start
    //set settings
    furi_string_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetChannel].cmd, settings.channel);
    furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetPhyRate].cmd, settings.rate);
    furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetPayloadType].cmd, settings.payload_type);
    furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetTxPower].cmd, settings.tx_power);
    
    switch (settings.mode_work)
    {
    case BLEPerCliSettingsModeWorkCarrier:
        furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetHopping].cmd, 0); //No hopping
        furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetMode].cmd, 2); //CW
        furi_string_cat_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeModeTx].cmd);
        break;
    case BLEPerCliSettingsModeWorkPacket:
        if(settings.mode == BLEPerCliSettingsModeTx) {
            furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetHopping].cmd, 0); //No hoppin
            furi_string_cat_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeModeTx].cmd);
        } else if(settings.mode == BLEPerCliSettingsModeRx) {
            furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetHopping].cmd, 0); //No hoppin
            furi_string_cat_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeModeRx].cmd);
        } else if(settings.mode == BLEPerCliSettingsModeHopping) {
            furi_string_cat_printf(msg, "%s %d\r\n", ble_per_cli_cmd[BlePerCliCmdTypeSetHopping].cmd, 2); //Random hopping
            furi_string_cat_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeModeTx].cmd);
        }
        break;
    
    default:
        furi_crash("Invalid mode work");
        break;
    }
    
    
    ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg));

    furi_string_free(msg);

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

    //furi_string_printf(msg, "%s\r\n", ble_per_cli_cmd[BlePerCliCmdTypeEndApp].cmd);
    furi_string_printf(msg, "%c\r\n", 0x03); //Ctrl+C
    ble_per_cli_data_tx((uint8_t*)furi_string_get_cstr(msg), furi_string_utf8_length(msg));
    FURI_LOG_D(TAG, "End APP");

    furi_string_free(msg);

    intercom_set_rx_callback(ble_per_cli_instance->intercom, IntercomChannelCli, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_stream_buffer_free(ble_per_cli_instance->rx_buffer);
    free(ble_per_cli_instance);
    ble_per_cli_instance = NULL;
}