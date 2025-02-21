#include "ble_usart_echo.h"
#include <furi.h>
#include <args.h>
#include <cli_worker.h>
#include "helpers/ble_usart_echo_app.h"

void ble_usart_echo_command_start(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(args);

    CliWorker* worker = cli_worker_alloc("BLE USART Echo", cli);
    cli_worker_set_callback(
        worker, ble_usart_echo_app_start, ble_usart_echo_app_parse_msg, ble_usart_echo_app_stop);
    if(!cli_worker_start(worker)) {
        printf("Failed to start BLE USART Echo worker\r\n");
        if(cli_worker_is_running(worker)) {
            cli_worker_stop(worker);
            cli_worker_free(worker);
            return;
        }
    }

    cli_worker_run(worker);

    if(cli_worker_is_running(worker)) {
        cli_worker_stop(worker);
        cli_worker_free(worker);
    }
    printf("\r\nExit BLE USART Echo app\r\n");
}

static void ble_usart_echo_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("ble_usart_echo \"BLE USART Echo app\"\r\n");
}

static void ble_usart_echo_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            ble_usart_echo_command_start(cli, args, context);
            break;
        }

        ble_usart_echo_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void ble_usart_echo_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(
        cli, "ble_usart_echo", CliCommandFlagParallelSafe, ble_usart_echo_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(ble_usart_echo_command);
#endif
}
