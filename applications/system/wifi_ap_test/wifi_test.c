#include "wifi_test.h"
#include <furi.h>
#include <args.h>
#include <cli_worker.h>
#include "helpers/wifi_test_app.h"

void wifi_test_command_start(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(args);

    CliWorker* worker = cli_worker_alloc("WiFi test", cli);
    cli_worker_set_callback(
        worker, wifi_test_app_start, wifi_test_app_parse_msg, wifi_test_app_stop);
    if(!cli_worker_start(worker)) {
        printf("Failed to start WiFi rf test worker\r\n");
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
    printf("\r\nExit WiFi test app\r\n");
}

static void wifi_test_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("wifi_test \"WiFi test app\"\r\n");
}

static void wifi_test_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            wifi_test_command_start(cli, args, context);
            break;
        }

        wifi_test_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void wifi_test_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "wifi_test", CliCommandFlagParallelSafe, wifi_test_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(wifi_test_command);
#endif
}
