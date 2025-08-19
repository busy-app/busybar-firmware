#include "cli_command_rtc.h"
#include <furi/furi.h>

#include <cli/args.h>
#include <furi_hal_rtc.h>

// Close to ISO, `date +'%Y-%m-%d %H:%M:%S %u'`
#define CLI_DATE_FORMAT "%.4d-%.2d-%.2d %.2d:%.2d:%.2d %d"

void cli_command_rtc_date(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    DateTime datetime = {0};

    if(furi_string_size(args) > 0) {
        uint16_t hours, minutes, seconds, month, day, year, weekday;
        int ret = sscanf(
            furi_string_get_cstr(args),
            "%hu-%hu-%hu %hu:%hu:%hu %hu",
            &year,
            &month,
            &day,
            &hours,
            &minutes,
            &seconds,
            &weekday);

        // Some variables are going to discard upper byte
        // There will be some funky behaviour which is not breaking anything
        datetime.hour = hours;
        datetime.minute = minutes;
        datetime.second = seconds;
        datetime.weekday = weekday;
        datetime.month = month;
        datetime.day = day;
        datetime.year = year;

        if(ret != 7) {
            printf(
                "Invalid datetime format, use `%s`. sscanf %d %s",
                "%Y-%m-%d %H:%M:%S %u",
                ret,
                furi_string_get_cstr(args));
            return;
        }

        if(!datetime_validate_datetime(&datetime)) {
            printf("Invalid datetime data");
            return;
        }

        furi_hal_rtc_set_datetime(&datetime);
        // Verification
        furi_hal_rtc_get_datetime(&datetime);
        printf(
            "New datetime is: " CLI_DATE_FORMAT,
            datetime.year,
            datetime.month,
            datetime.day,
            datetime.hour,
            datetime.minute,
            datetime.second,
            datetime.weekday);
    } else {
        furi_hal_rtc_get_datetime(&datetime);
        printf(
            CLI_DATE_FORMAT,
            datetime.year,
            datetime.month,
            datetime.day,
            datetime.hour,
            datetime.minute,
            datetime.second,
            datetime.weekday);
    }
}
