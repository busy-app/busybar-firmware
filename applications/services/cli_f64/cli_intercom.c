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
} CliIntercom;

static CliIntercom* cli_intercom_handle = NULL;

static void cli_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(cli_intercom_handle);
    furi_check(data);
    UNUSED(context);

    const size_t rx_size =
        furi_stream_buffer_send(cli_intercom_handle->rx_buffer, data, data_size, FuriWaitForever);
    furi_assert(rx_size == data_size);
}

static void cli_intercom_init(void) {
    if(cli_intercom_handle == NULL) {
        cli_intercom_handle = malloc(sizeof(CliIntercom));
        cli_intercom_handle->rx_buffer = furi_stream_buffer_alloc(BUF_SIZE, 1);

        cli_intercom_handle->intercom = furi_record_open(RECORD_INTERCOM);
        intercom_set_rx_callback(
            cli_intercom_handle->intercom,
            IntercomChannelCli,
            cli_intercom_rx_callback,
            cli_intercom_handle);
    }
}

static void cli_intercom_deinit(void) {
    furi_check(cli_intercom_handle);

    furi_record_close(RECORD_INTERCOM);
    free(cli_intercom_handle);
}

static size_t cli_intercom_rx(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_check(cli_intercom_handle);

    return furi_stream_buffer_receive(cli_intercom_handle->rx_buffer, buffer, size, timeout);
}

static size_t cli_intercom_rx_stdin(uint8_t* data, size_t size, uint32_t timeout, void* context) {
    furi_check(cli_intercom_handle);
    UNUSED(context);

    return furi_stream_buffer_receive(cli_intercom_handle->rx_buffer, data, size, timeout);
}

static void cli_intercom_tx(const uint8_t* buffer, size_t size) {
    furi_check(cli_intercom_handle);

    intercom_tx(cli_intercom_handle->intercom, IntercomChannelCli, buffer, size, FuriWaitForever);
}

static void cli_intercom_tx_stdout(const char* data, size_t size, void* context) {
    furi_check(cli_intercom_handle);
    UNUSED(context);

    intercom_tx(cli_intercom_handle->intercom, IntercomChannelCli, data, size, FuriWaitForever);
}

static bool cli_intercom_is_connected(void) {
    return cli_intercom_handle != NULL;
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
