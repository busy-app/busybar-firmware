#include "telemetry.h"

#include <containers/pipe.h>
#include <cli/args.h>
#include <cli/cli_command.h>
#include <furi.h>

static void cli_command_telemetry_print_usage(void) {
    printf("Usage:\r\n");
    printf("telemetry status - show whether telemetry collection is enabled\r\n");
    printf("telemetry on     - enable telemetry collection\r\n");
    printf("telemetry off    - disable telemetry collection\r\n");
}

void cli_command_telemetry(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    FuriString* cmd = furi_string_alloc();

    if(args_read_string_and_trim(args, cmd)) {
        Telemetry* instance = furi_record_open(RECORD_TELEMETRY);

        if(furi_string_equal(cmd, "status")) {
            printf("Telemetry collection is %s\r\n", telemetry_is_enabled(instance) ? "enabled" : "disabled");
        } else if(furi_string_equal(cmd, "on")) {
            telemetry_set_enabled(instance, true);
            printf("Telemetry collection enabled\r\n");
        } else if(furi_string_equal(cmd, "off")) {
            telemetry_set_enabled(instance, false);
            printf("Telemetry collection disabled\r\n");
        } else {
            cli_command_telemetry_print_usage();
        }

        furi_record_close(RECORD_TELEMETRY);
    } else {
        cli_command_telemetry_print_usage();
    }

    furi_string_free(cmd);
}
