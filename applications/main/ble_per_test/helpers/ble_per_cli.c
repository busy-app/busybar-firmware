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
    //FURI_LOG_RAW_E("\r\n");
    
    // furi_check(
    //     furi_stream_buffer_send(instance->rx_buffer, data, data_size, FuriWaitForever) ==
    //     data_size);
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

    uint8_t data[] = {"ble_per_test\r\n"};
    ble_per_cli_data_tx(data, sizeof(data));

    uint8_t data1[] = {"channel 3\r\n"};
    ble_per_cli_data_tx(data1, sizeof(data1));

    //furi_delay_ms(5000);
    uint8_t data2[] = {"tx\r\n"};
    ble_per_cli_data_tx(data2, sizeof(data2));

}


void ble_per_cli_stop(void) {
    if(ble_per_cli_instance == NULL) {
        return;
    }
    uint8_t data5[] = {"stop\r\n"};
    ble_per_cli_data_tx(data5, sizeof(data5));

    furi_delay_ms(1000);
    uint8_t data6[] = {0x03 , 0x0D};
    ble_per_cli_data_tx(data6, sizeof(data6));

    intercom_set_rx_callback(ble_per_cli_instance->intercom, IntercomChannelCli, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_stream_buffer_free(ble_per_cli_instance->rx_buffer);
    free(ble_per_cli_instance);
    ble_per_cli_instance = NULL;
}