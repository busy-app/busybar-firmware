#include "busy_timer_i.h"

#include <busy/busy.h>
#include <desktop/desktop.h>

#define BUSY_APP_WAIT_TIME_TICKS (500)

static bool busy_timer_app_show_timer(BusyApp* busy_app, const BusyAppConfig* app_config) {
    bool success = false;

    do {
        if(busy_set_config(busy_app, app_config) != BusyStatusOk) {
            break;
        }

        if(busy_show_timer(busy_app) != BusyStatusOk) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static void busy_timer_app_launch(Desktop* desktop) {
    while(!desktop_replace_current_app(desktop, "busy", BUSY_APP_TIMER_MODE)) {
        furi_delay_tick(2);
    }
}

void busy_timer_start_app(const BusyAppConfig* app_config) {
    // NOTE: Waiting for Desktop before attempting to look for the app record
    Desktop* desktop = furi_record_open(RECORD_DESKTOP);

    bool success = false;

    do {
        BusyApp* busy_app = furi_record_open_ex(RECORD_BUSY_APP, BUSY_APP_WAIT_TIME_TICKS);

        if(busy_app == NULL) {
            busy_timer_app_launch(desktop);
            continue;
        }

        success = busy_timer_app_show_timer(busy_app, app_config);

        furi_record_close(RECORD_BUSY_APP);

    } while(!success);

    furi_record_close(RECORD_DESKTOP);
}

void busy_timer_exit_app(void) {
    BusyApp* busy_app = furi_record_open_ex(RECORD_BUSY_APP, 0);

    if(busy_app != NULL) {
        busy_request_exit(busy_app);
        furi_record_close(RECORD_BUSY_APP);
    }
}
