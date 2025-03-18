#include "cli_i.h" // IWYU pragma: keep

#include <furi.h>
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#define TAG "CliIntercom"

#define BUF_SIZE       (256UL)
#define UART_BAUD_RATE (230400UL)
#define HANDLE_UART    FuriHalSerialIdUart1

#define STREAM_BUFFER_SIZE_TX (BUF_SIZE)
#define STREAM_BUFFER_SIZE_RX (BUF_SIZE)

#ifdef CLI_UART_DEBUG
#define CLI_UART_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define CLI_UART_DEBUG(...)
#endif

static void cli_intercom_init(void) {
}

static void cli_intercom_deinit(void) {
}

static size_t cli_intercom_rx(uint8_t* buffer, size_t size, uint32_t timeout) {
    UNUSED(buffer);
    UNUSED(size);
    UNUSED(timeout);

    return 0;
}

static size_t cli_intercom_rx_stdin(uint8_t* data, size_t size, uint32_t timeout, void* context) {
    UNUSED(data);
    UNUSED(size);
    UNUSED(timeout);
    UNUSED(context);

    return 10;
}

static void cli_intercom_tx(const uint8_t* buffer, size_t size) {
    UNUSED(buffer);
    UNUSED(size);
}

static void cli_intercom_tx_stdout(const char* data, size_t size, void* context) {
    UNUSED(data);
    UNUSED(size);
    UNUSED(context);
}

static bool cli_intercom_is_connected(void) {
    return false;
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
