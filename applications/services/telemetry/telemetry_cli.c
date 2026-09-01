#include "telemetry.h"

#include <containers/pipe.h>
#include <cli/args.h>
#include <cli/cli_command.h>
#include <furi.h>
#include <furi_hal_nvm.h>

static void cli_command_telemetry_print_usage(void) {
    printf("Usage:\r\n");
    printf("telemetry status - show whether telemetry collection is enabled\r\n");
    printf("telemetry on     - enable telemetry collection\r\n");
    printf("telemetry off    - disable telemetry collection\r\n");
    if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        printf("telemetry stats  - show telemetry buffer status and event counters\r\n");
    }
}

static void cli_command_telemetry_stats(Telemetry* instance) {
    TelemetryStats stats;
    telemetry_get_stats(instance, &stats);

    printf("Telemetry stats:\r\n");
    printf("  enabled: %s\r\n", stats.is_enabled ? "true" : "false");
    printf("  connected: %s\r\n", stats.is_connected ? "true" : "false");
    printf("  buffered events: %lu\r\n", (unsigned long)stats.buffered_events);
    printf("  batches sent: %lu\r\n", (unsigned long)stats.batches_sent);
    printf("  events sent: %lu\r\n", (unsigned long)stats.events_sent);
    printf("  events dropped: %lu\r\n", (unsigned long)stats.events_dropped);
    printf("  events by type:\r\n");
    for(TelemetryEventType type = 0; type < TelemetryEventMax; ++type) {
        printf(
            "    %-24s %lu\r\n",
            telemetry_event_type_name(type),
            (unsigned long)stats.events_by_type[type]);
    }
}

void cli_command_telemetry(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    FuriString* cmd = furi_string_alloc();

    if(args_read_string_and_trim(args, cmd)) {
        Telemetry* instance = furi_record_open(RECORD_TELEMETRY);

        if(furi_string_equal(cmd, "status")) {
            printf(
                "Telemetry collection is %s\r\n",
                telemetry_is_enabled(instance) ? "enabled" : "disabled");
        } else if(furi_string_equal(cmd, "on")) {
            telemetry_set_enabled(instance, true);
            printf("Telemetry collection enabled\r\n");
        } else if(furi_string_equal(cmd, "off")) {
            telemetry_set_enabled(instance, false);
            printf("Telemetry collection disabled\r\n");
        } else if(furi_string_equal(cmd, "stats")) {
            if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
                cli_command_telemetry_stats(instance);
            } else {
                printf("Not available: debug mode is disabled\r\n");
            }
        } else {
            cli_command_telemetry_print_usage();
        }

        furi_record_close(RECORD_TELEMETRY);
    } else {
        cli_command_telemetry_print_usage();
    }

    furi_string_free(cmd);
}
