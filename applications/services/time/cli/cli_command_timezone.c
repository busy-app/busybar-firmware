#include "cli_command_timezone.h"
#include <furi/furi.h>

#include <cli/args.h>
#include <furi_hal_rtc.h>
#include <time/time.h>

void cli_command_timezone(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(furi_string_size(args) > 0) {
        utz_zone_t zone;
        if(!utz_get_zone_by_name(furi_string_get_cstr(args), &zone)) {
            printf("Invalid timezone. Possible values:\n");
            for(const char* item = utz_zone_names; item; item = utz_next_zone_name(item)) {
                printf("%s\n", item);
            }
            return;
        }

        Time* time = furi_record_open(RECORD_TIME);
        TimeSettings settings;
        time_get_settings(time, &settings);
        settings.timezone = zone;
        bool ok = time_set_settings(time, &settings);
        furi_record_close(RECORD_TIME);

        if(ok) {
            printf("Timezone set OK, new timezone: %s", zone.name);
            return;
        } else {
            printf("Error saving timezone");
        }
    } else {
        Time* time = furi_record_open(RECORD_TIME);
        TimeSettings settings;
        time_get_settings(time, &settings);
        furi_record_close(RECORD_TIME);

        printf("%s", settings.timezone.name);
    }
}
