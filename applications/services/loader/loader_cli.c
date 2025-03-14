#include "loader.h"

#include <furi.h>
#include <cli/cli.h>
#include <desktop/desktop.h>

#include <lib/toolbox/args.h>
#include <lib/toolbox/strint.h>

static void loader_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("loader <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\topen <Application Name:string>\t - Open application by name\r\n");
    printf("\tkill\t - Kill the current application\r\n");
}

static void loader_cli_kill(Loader* loader) {
    const LoaderStatus status = loader_stop(loader);
    if(status == LoaderStatusOk) {
        printf("App stopped successfully\r\n");
    } else if(status == LoaderStatusErrorAppNotRunning) {
        printf("No app running\r\n");
    } else if(status == LoaderStatusErrorInternal) {
        printf("Failed to stop: update app to support signals\r\n");
    } else {
        printf("Failed to stop: unexpected loader status");
    }
}

static void loader_cli_open(FuriString* args, Desktop* desktop) {
    FuriString* app_name = furi_string_alloc();

    do {
        if(!args_read_probably_quoted_string_and_trim(args, app_name)) {
            printf("No application provided\r\n");
            break;
        }
        furi_string_trim(args);

        const char* args_str = furi_string_get_cstr(args);
        if(strlen(args_str) == 0) {
            args_str = NULL;
        }

        const char* app_name_str = furi_string_get_cstr(app_name);

        if(!desktop_replace_current_app(desktop, app_name_str, (void*)args_str)) {
            printf("Failed to launch %s", app_name_str);
        }
    } while(false);

    furi_string_free(app_name);
}

static void loader_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    Loader* loader = furi_record_open(RECORD_LOADER);
    Desktop* desktop = furi_record_open(RECORD_DESKTOP);

    FuriString* cmd;
    cmd = furi_string_alloc();

    if(!args_read_string_and_trim(args, cmd)) {
        loader_cli_print_usage();
    } else if(furi_string_equal(cmd, "open")) {
        loader_cli_open(args, desktop);
    } else if(furi_string_equal(cmd, "kill")) {
        loader_cli_kill(loader);
    } else {
        loader_cli_print_usage();
    }

    furi_string_free(cmd);
    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_LOADER);
}

void loader_on_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, RECORD_LOADER, CliCommandFlagParallelSafe, loader_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(loader_cli);
#endif
}
