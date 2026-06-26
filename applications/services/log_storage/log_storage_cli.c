#include "log_storage.h"

#include <cli/cli_command.h>
#include <cli/args.h>

typedef struct {
    FuriString* path;
    bool help;
} LogStorageCliArguments;

static const LogStorageCliArguments log_storage_cli_default_arguments = {
    .path = NULL,
    .help = false,
};

static void log_storage_cli_print_usage(void) {
    printf("Usage: log_dump [-h] [path]\r\n");
    printf("Dump captured logs to <path>, or " LOG_STORAGE_DUMP_DEFAULT_FILE_PATH
           " if omitted.\r\n");
    printf("  -h, --help    Show this help message\r\n");
}

static void
    log_storage_cli_parse_arguments(FuriString* args_string, LogStorageCliArguments* args) {
    *args = log_storage_cli_default_arguments;

    FuriString* arg = furi_string_alloc();
    while(args_read_string_and_trim(args_string, arg)) {
        if(furi_string_equal_str(arg, "-h") || furi_string_equal_str(arg, "--help")) {
            args->help = true;
            break;
        } else if(args->path == NULL) {
            args->path = furi_string_alloc_set(arg);
        } else {
            printf("Unexpected argument: %s\r\n", furi_string_get_cstr(arg));
            args->help = true;
            break;
        }
    }

    furi_string_free(arg);
}

void log_storage_cli_command_entry(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    LogStorageCliArguments parsed;
    log_storage_cli_parse_arguments(args, &parsed);

    if(parsed.help) {
        log_storage_cli_print_usage();
    } else {
        const char* path = parsed.path ? furi_string_get_cstr(parsed.path) : NULL;
        LogStorage* log_storage = furi_record_open(RECORD_LOG_STORAGE);
        bool is_successful = log_storage_dump(log_storage, path);
        furi_record_close(RECORD_LOG_STORAGE);

        if(is_successful) {
            printf(
                "Log successfully saved to %s.\r\n", path ?: LOG_STORAGE_DUMP_DEFAULT_FILE_PATH);
        } else {
            printf("Failed to save log.\r\n");
        }
    }

    if(parsed.path) furi_string_free(parsed.path);
}
