#include "cli_i.h" // IWYU pragma: keep

#include <furi.h>
#include <intercom/intercom.h>

#define TAG "CliIntercom"

#define BUF_SIZE (1024)

#ifdef CLI_UART_DEBUG
#define CLI_UART_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define CLI_UART_DEBUG(...)
#endif

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    uint8_t data[BUF_SIZE];
} CliIntercomContext;

static CliIntercomContext* instance = NULL;

static void cli_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    UNUSED(context);
    furi_check(instance);

    if(instance) {
        furi_check(
            furi_stream_buffer_send(instance->rx_buffer, data, data_size, FuriWaitForever) ==
            data_size);
    }
}

static void cli_intercom_init(void) {
    if(instance == NULL) {
        instance = malloc(sizeof(CliIntercomContext));
        instance->rx_buffer = furi_stream_buffer_alloc(BUF_SIZE, 1);

        instance->intercom = furi_record_open(RECORD_INTERCOM);
        intercom_set_rx_callback(
            instance->intercom, IntercomChannelControl, cli_intercom_rx_callback, instance);
    }
}

static void cli_intercom_deinit(void) {
    furi_check(instance);

    furi_record_close(RECORD_INTERCOM);
    free(instance);
}

static size_t cli_intercom_rx(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_check(instance);

    size_t rx_bytes = furi_stream_buffer_receive(instance->rx_buffer, buffer, size, timeout);

    if(rx_bytes) {
        FURI_LOG_W("INTERCOM", "Rx bytes %d: %c", rx_bytes, buffer[0]);
    }

    return rx_bytes;
}

static size_t cli_intercom_rx_stdin(uint8_t* data, size_t size, uint32_t timeout, void* context) {
    UNUSED(context);
    furi_check(instance);

    size_t rx_bytes = furi_stream_buffer_receive(instance->rx_buffer, data, size, timeout);

    if(rx_bytes) {
        FURI_LOG_W("INTERCOM", "Rx bytes %d: %c", rx_bytes, data[0]);
    }

    return rx_bytes;
}

static void cli_intercom_tx(const uint8_t* buffer, size_t size) {
    furi_check(instance);

    FURI_LOG_I("INTERCOM", "TX %d bytes: %c", size, buffer[0]);

    intercom_tx(instance->intercom, IntercomChannelControl, buffer, size, FuriWaitForever);
}

static void cli_intercom_tx_stdout(const char* data, size_t size, void* context) {
    UNUSED(context);
    furi_check(instance);

    intercom_tx(instance->intercom, IntercomChannelControl, data, size, FuriWaitForever);
}

static bool cli_intercom_is_connected(void) {
    return instance != NULL;
}

CliSession cli_intercom = {
    cli_intercom_init,
    cli_intercom_deinit,
    cli_intercom_rx,
    cli_intercom_rx_stdin,
    cli_intercom_tx,
    cli_intercom_tx_stdout,
    cli_intercom_is_connected,
};
