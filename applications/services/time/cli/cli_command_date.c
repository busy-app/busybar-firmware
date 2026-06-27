#include "cli_command_date.h"
#include <furi/furi.h>

#include <cli/args.h>
#include <furi_hal_rtc.h>
#include <time/time.h>

void cli_command_date(PipeSide* pipe, FuriString* args, void* context) {
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

        Time* time = furi_record_open(RECORD_TIME);
        LocalTime local_time = time_get_local_time(time);
        furi_record_close(RECORD_TIME);

        char ts_buf[DATETIME_TIMESTAMP_STR_LEN + 1];
        datetime_format_timestamp(&local_time, ts_buf);
        printf("New datetime is: %s", ts_buf);
    } else {
        Time* time = furi_record_open(RECORD_TIME);
        LocalTime local_time = time_get_local_time(time);
        furi_record_close(RECORD_TIME);

        char ts_buf[DATETIME_TIMESTAMP_STR_LEN + 1];
        datetime_format_timestamp(&local_time, ts_buf);

        printf("%s", ts_buf);
    }
}
