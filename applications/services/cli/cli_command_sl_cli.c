#include "cli_command_sl_cli.h"

#include <intercom/intercom.h>

#define CLI_BUFFER_SIZE  (1024U)
#define CLI_READ_TIMEOUT (10U)

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    uint8_t rx_data[CLI_BUFFER_SIZE];
} CliCommandSlCli;

static void cli_command_917_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);

    CliCommandSlCli* instance = context;

    furi_check(
        furi_stream_buffer_send(instance->rx_buffer, data, data_size, FuriWaitForever) ==
        data_size);
}

static CliCommandSlCli* cli_command_sl_cli_alloc(void) {
    CliCommandSlCli* instance = malloc(sizeof(CliCommandSlCli));

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->rx_buffer = furi_stream_buffer_alloc(CLI_BUFFER_SIZE, 1);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelCli, cli_command_917_rx_callback, instance);

    return instance;
}

static void cli_command_sl_cli_free(CliCommandSlCli* instance) {
    intercom_set_rx_callback(instance->intercom, IntercomChannelCli, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_stream_buffer_free(instance->rx_buffer);
    free(instance);
}

static void cli_command_sl_cli_send_command(Intercom* intercom, FuriString* cmd) {
    const size_t sz = furi_string_size(cmd);
    const size_t tx_size =
        intercom_tx(intercom, IntercomChannelCli, furi_string_get_cstr(cmd), sz, FuriWaitForever);
    furi_assert(tx_size == sz);
}

void cli_command_sl_cli_send_command_get_response(Cli* cli, const char* sl_cmd) {
    CliCommandSlCli* instance = cli_command_sl_cli_alloc();

    FuriString* buf = furi_string_alloc_printf("%s\r", sl_cmd);
    cli_command_sl_cli_send_command(instance->intercom, buf);

    while(true) {
        const size_t rx_size = furi_stream_buffer_receive(
            instance->rx_buffer, instance->rx_data, sizeof(instance->rx_data), 100);

        if(!rx_size) break;
        cli_write(cli, instance->rx_data, rx_size);
    }

    furi_string_printf(buf, "%c\r", CliSymbolAsciiETX);
    cli_command_sl_cli_send_command(instance->intercom, buf);

    cli_command_sl_cli_free(instance);
    furi_string_free(buf);
}

void cli_command_sl_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliCommandSlCli* instance = cli_command_sl_cli_alloc();

    printf("Starting Si917 cli...\r\n");
    printf("Press Ctrl+C to exit\r\n");

    while(true) {
        uint8_t ch = 0;

        if(cli_read_timeout(cli, &ch, sizeof(ch), CLI_READ_TIMEOUT)) {
            const size_t tx_size = intercom_tx(
                instance->intercom, IntercomChannelCli, &ch, sizeof(ch), FuriWaitForever);
            furi_assert(tx_size == sizeof(ch));

            if(ch == CliSymbolAsciiETX) {
                printf("\r\nEnd of Si917 cli session...\r\n\r\n");
                break;
            }
        }

        const size_t rx_size = furi_stream_buffer_receive(
            instance->rx_buffer, instance->rx_data, sizeof(instance->rx_data), 0);

        if(rx_size) {
            cli_write(cli, instance->rx_data, rx_size);
        }
    }

    cli_command_sl_cli_free(instance);
}

void cli_command_sl_echo(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(args);

    const uint32_t baud = 230400UL;

    printf("Starting 917 echo server on %ld\r\n", baud);
    FuriString* cmd = furi_string_alloc_printf("echo_server  %ld\r", baud);

    CliCommandSlCli* instance = cli_command_sl_cli_alloc();
    cli_command_sl_cli_send_command(instance->intercom, cmd);

    FuriHalSerialHandle* serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart6);
    furi_hal_serial_init(serial, baud);
    furi_hal_serial_clear(serial, FuriHalSerialDirectionTxRx);

    while(true) {
        uint8_t ch = cli_getc(cli);

        if(ch == CliSymbolAsciiETX) {
            break;
        } else if(ch == CliSymbolAsciiCR || ch == CliSymbolAsciiLF)
            continue;

        furi_hal_serial_tx(serial, &ch, 1);
        if(!furi_hal_serial_tx_wait_complete(serial, 100)) {
            break;
        }

        furi_delay_ms(10);

        while(furi_hal_serial_rx_available(serial)) {
            ch = furi_hal_serial_rx(serial);
            cli_putc(cli, ch);
        }
    }

    furi_hal_serial_control_release(serial);

    furi_string_printf(cmd, "%c\r", CliSymbolAsciiETX);
    cli_command_sl_cli_send_command(instance->intercom, cmd);

    furi_string_free(cmd);
    cli_command_sl_cli_free(instance);
}
