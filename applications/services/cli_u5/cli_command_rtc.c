#include "cli_command_rtc.h"
#include <furi/furi.h>

#include <cli/args.h>
#include <furi_hal_rtc.h>
#include <sntp/sntp.h>

void cli_command_rtc_date(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(furi_string_size(args) > 0) {
        DateTime dt;
        bool ok = datetime_parse_timestamp(furi_string_get_cstr(args), &dt);

        if(!ok) {
            printf("Invalid datetime format, use ISO 8601 timestamp");
            return;
        }

        furi_hal_rtc_set_datetime(&(DateTimeMs){.dt = dt, .millis = 0});
        // Verification

        Sntp* sntp = furi_record_open(RECORD_SNTP);
        LocalTime local_time = sntp_get_local_time(sntp);
        furi_record_close(RECORD_SNTP);

        char ts_buf[DATETIME_TIMESTAMP_STR_LEN + 1];
        datetime_format_timestamp(&local_time, ts_buf);
        printf("New datetime is: %s", ts_buf);
    } else {
        Sntp* sntp = furi_record_open(RECORD_SNTP);
        LocalTime local_time = sntp_get_local_time(sntp);
        furi_record_close(RECORD_SNTP);

        char ts_buf[DATETIME_TIMESTAMP_STR_LEN + 1];
        datetime_format_timestamp(&local_time, ts_buf);

        printf("%s", ts_buf);
    }
}

void cli_command_rtc_timezone(PipeSide* pipe, FuriString* args, void* context) {
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

        Sntp* sntp = furi_record_open(RECORD_SNTP);
        SntpSettings settings;
        sntp_get_settings(sntp, &settings);
        settings.timezone = zone;
        bool ok = sntp_set_settings(sntp, &settings);
        furi_record_close(RECORD_SNTP);

        if(ok) {
            printf("Timezone set OK, new timezone: %s", zone.name);
            return;
        } else {
            printf("Error saving timezone");
        }
    } else {
        Sntp* sntp = furi_record_open(RECORD_SNTP);
        SntpSettings settings;
        sntp_get_settings(sntp, &settings);
        furi_record_close(RECORD_SNTP);

        printf("%s", settings.timezone.name);
    }
}
