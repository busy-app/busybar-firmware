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

typedef enum {
    UartEvtStop = (1 << 0),
    UartEvtConnect = (1 << 1),
    UartEvtDisconnect = (1 << 2),
    UartEvtRx = (1 << 3),
    UartEvtStreamTx = (1 << 5),
} WorkerEvtFlags;

#define VCP_THREAD_FLAG_ALL \
    (UartEvtStop | UartEvtConnect | UartEvtDisconnect | UartEvtRx | UartEvtStreamTx)

typedef struct {
    FuriThread* thread;

    FuriStreamBuffer* tx_stream;
    FuriStreamBuffer* rx_stream;

    volatile bool connected;
    volatile bool running;
    FuriHalSerialHandle* handle_intercom;

    uint8_t data_buffer[BUF_SIZE];
} CliUart;

static int32_t cli_intercom_worker(void* context);

static CliUart* cli_intercom_handle = NULL;

static const uint8_t ascii_soh = 0x01;
static const uint8_t ascii_eot = 0x04;

static void cli_intercom_serial_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* ctx) {
    CliUart* context = ctx;

    if(event & (FuriHalSerialRxEventData | FuriHalSerialRxEventIdle)) {
        while(furi_hal_serial_rx_available(handle)) {
            const uint8_t c = furi_hal_serial_rx(handle);
            furi_check(furi_stream_buffer_send(context->rx_stream, &c, sizeof(c), 0) == sizeof(c));
        }
        furi_thread_flags_set(furi_thread_get_id(context->thread), UartEvtRx);
    } else {
        furi_crash();
    }
}

static void cli_intercom_init(void) {
    if(cli_intercom_handle == NULL) {
        cli_intercom_handle = malloc(sizeof(CliUart));
        cli_intercom_handle->tx_stream = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE_TX, 1);
        cli_intercom_handle->rx_stream = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE_TX, 1);
        cli_intercom_handle->handle_intercom = furi_hal_serial_control_acquire(HANDLE_UART);
        furi_check(cli_intercom_handle->handle_intercom);
        furi_hal_serial_init(cli_intercom_handle->handle_intercom, UART_BAUD_RATE);
        furi_hal_serial_set_callback(
            cli_intercom_handle->handle_intercom,
            NULL,
            cli_intercom_serial_rx_callback,
            cli_intercom_handle);
        furi_hal_serial_async_rx_start(cli_intercom_handle->handle_intercom, false);
    }
    furi_assert(cli_intercom_handle->thread == NULL);

    cli_intercom_handle->connected = false;

    cli_intercom_handle->thread =
        furi_thread_alloc_ex("CliUartWorker", 1024, cli_intercom_worker, NULL);
    furi_thread_start(cli_intercom_handle->thread);

    FURI_LOG_I(TAG, "Init OK");
}

static void cli_intercom_deinit(void) {
    furi_thread_flags_set(furi_thread_get_id(cli_intercom_handle->thread), UartEvtStop);
    furi_thread_join(cli_intercom_handle->thread);
    furi_thread_free(cli_intercom_handle->thread);
    cli_intercom_handle->thread = NULL;
}

static int32_t cli_intercom_worker(void* context) {
    UNUSED(context);
    //size_t missed_rx = 0;

    FURI_LOG_D(TAG, "Start");
    cli_intercom_handle->running = true;

    while(1) {
        uint32_t flags =
            furi_thread_flags_wait(VCP_THREAD_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        furi_assert(!(flags & FuriFlagError));

        // Uart session opened
        if((flags & UartEvtRx) && (cli_intercom_handle->connected == false)) {
            CLI_UART_DEBUG("Connect");

            if(cli_intercom_handle->connected == false) {
                cli_intercom_handle->connected = true;
                flags |= UartEvtRx;
                furi_stream_buffer_send(
                    cli_intercom_handle->rx_stream, &ascii_soh, 1, FuriWaitForever);
            }
        }

        // Uart write transfer done
        if(flags & UartEvtStreamTx) {
            size_t len = furi_stream_buffer_receive(
                cli_intercom_handle->tx_stream, cli_intercom_handle->data_buffer, BUF_SIZE, 0);

            CLI_UART_DEBUG("Tx %d", len);

            if(len > 0) { // Some data left in Tx buffer. Sending it now
                furi_hal_serial_tx(
                    cli_intercom_handle->handle_intercom,
                    (const uint8_t*)cli_intercom_handle->data_buffer,
                    len);
            }
        }

        if(flags & UartEvtStop) {
            cli_intercom_handle->connected = false;
            cli_intercom_handle->running = false;

            furi_stream_buffer_receive(
                cli_intercom_handle->tx_stream, cli_intercom_handle->data_buffer, BUF_SIZE, 0);
            furi_stream_buffer_send(
                cli_intercom_handle->rx_stream, &ascii_eot, 1, FuriWaitForever);

            break;
        }
    }
    furi_hal_serial_deinit(cli_intercom_handle->handle_intercom);
    furi_hal_serial_control_release(cli_intercom_handle->handle_intercom);
    FURI_LOG_D(TAG, "End");
    return 0;
}

static size_t cli_intercom_rx(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(cli_intercom_handle);
    furi_assert(buffer);

    if(cli_intercom_handle->running == false) {
        return 0;
    }

    CLI_UART_DEBUG("rx %u start", size);

    size_t rx_cnt = 0;
    while(size > 0) {
        size_t batch_size = size;
        if(batch_size > BUF_SIZE) batch_size = BUF_SIZE;

        size_t len = furi_stream_buffer_receive(
            cli_intercom_handle->rx_stream, buffer, batch_size, timeout);
        CLI_UART_DEBUG("rx %u ", batch_size);

        if(len == 0) break;
        if(cli_intercom_handle->running == false) {
            // EOT command is received after VCP session close
            rx_cnt += len;
            break;
        }
        size -= len;
        buffer += len;
        rx_cnt += len;
    }

    CLI_UART_DEBUG("rx %u end", size);
    return rx_cnt;
}

static size_t cli_intercom_rx_stdin(uint8_t* data, size_t size, uint32_t timeout, void* context) {
    UNUSED(context);
    return cli_intercom_rx(data, size, timeout);
}

static void cli_intercom_tx(const uint8_t* buffer, size_t size) {
    furi_assert(cli_intercom_handle);
    furi_assert(buffer);

    if(cli_intercom_handle->running == false) {
        return;
    }

    CLI_UART_DEBUG("tx %u start", size);

    while(size > 0 && cli_intercom_handle->connected) {
        size_t batch_size = size;
        if(batch_size > BUF_SIZE) batch_size = BUF_SIZE;

        furi_stream_buffer_send(
            cli_intercom_handle->tx_stream, buffer, batch_size, FuriWaitForever);
        furi_thread_flags_set(furi_thread_get_id(cli_intercom_handle->thread), UartEvtStreamTx);
        CLI_UART_DEBUG("tx %u", batch_size);

        size -= batch_size;
        buffer += batch_size;
    }

    CLI_UART_DEBUG("tx %u end", size);
}

static void cli_intercom_tx_stdout(const char* data, size_t size, void* context) {
    UNUSED(context);
    cli_intercom_tx((const uint8_t*)data, size);
}

static bool cli_intercom_is_connected(void) {
    furi_assert(cli_intercom_handle);
    return cli_intercom_handle->connected;
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
