#include "ble_ibeacon.h"
#include <furi.h>
#include <args.h>
#include <cli_worker.h>
#include "helpers/ble_ibeacon_app.h"

void ble_ibeacon_command_start(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(args);

    CliWorker* worker = cli_worker_alloc("BLE iBeacon", cli);
    cli_worker_set_callback(
        worker, ble_ibeacon_app_start, ble_ibeacon_app_parse_msg, ble_ibeacon_app_stop);
    if(!cli_worker_start(worker)) {
        printf("Failed to start BLE iBeacon worker\r\n");
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
    printf("\r\nExit BLE iBeacon app\r\n");
}

static void ble_ibeacon_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("ble_ibeacon \"BLE iBeacon app\"\r\n");
}

static void ble_ibeacon_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            ble_ibeacon_command_start(cli, args, context);
            break;
        }

        ble_ibeacon_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void ble_ibeacon_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "ble_ibeacon", CliCommandFlagParallelSafe, ble_ibeacon_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(ble_ibeacon_command);
#endif
}
