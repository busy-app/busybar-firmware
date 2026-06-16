#include "busy_timer_i.h"

#include <busy/busy.h>
#include <desktop/desktop.h>

#define BUSY_APP_WAIT_TIME_TICKS (500)

static bool busy_timer_app_record_exists(void) {
    bool record_exists = false;

    for(const uint32_t start_tick = furi_get_tick();
        furi_get_tick() - start_tick < BUSY_APP_WAIT_TIME_TICKS;
        furi_delay_tick(2)) {
        if(furi_record_exists(RECORD_BUSY_APP)) {
            record_exists = true;
            break;
        }
    }

    return record_exists;
}

static bool busy_timer_app_show_timer(const BusyAppConfig* app_config) {
    bool success = false;

    BusyApp* busy_app = furi_record_open(RECORD_BUSY_APP);

    do {
        if(busy_set_config(busy_app, app_config) != BusyStatusOk) {
            break;
        }

        if(busy_show_timer(busy_app) != BusyStatusOk) {
            break;
        }

        success = true;
    } while(false);

    furi_record_close(RECORD_BUSY_APP);

    return success;
}

void busy_timer_start_app(const BusyAppConfig* app_config) {
    // NOTE: Waiting for Desktop before attempting to look for the app record
    Desktop* desktop = furi_record_open(RECORD_DESKTOP);

    for(;;) {
        if(!busy_timer_app_record_exists()) {
            while(!desktop_replace_current_app(desktop, "busy", BUSY_APP_TIMER_MODE)) {
                furi_delay_tick(2);
            }
        }

        if(busy_timer_app_show_timer(app_config)) {
            break;
        }
    }

    furi_record_close(RECORD_DESKTOP);
}

void busy_timer_exit_app(void) {
    if(busy_timer_app_record_exists()) {
        BusyApp* busy_app = furi_record_open(RECORD_BUSY_APP);
        busy_request_exit(busy_app);
        furi_record_close(RECORD_BUSY_APP);
    }
}
