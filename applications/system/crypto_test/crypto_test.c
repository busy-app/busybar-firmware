#include "crypto_test.h"
#include <furi.h>
#include <args.h>
#include <cli_worker.h>
#include "helpers/crypto_test_app.h"

void crypto_test_command_start(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(args);

    CliWorker* worker = cli_worker_alloc("Crypto test", cli);
    cli_worker_set_callback(
        worker, crypto_test_app_start, crypto_test_app_parse_msg, crypto_test_app_stop);
    if(!cli_worker_start(worker)) {
        printf("Failed to start Crypto test worker\r\n");
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
    printf("\r\nExit Crypto test app\r\n");
}

static void crypto_test_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("crypto_test \"Crypto test app\"\r\n");
}

static void crypto_test_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            crypto_test_command_start(cli, args, context);
            break;
        }

        crypto_test_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void crypto_test_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "crypto_test", CliCommandFlagParallelSafe, crypto_test_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(crypto_test_command);
#endif
}
