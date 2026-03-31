#include "busy_timer_i.h"

#include <busy/busy.h>
#include <desktop/desktop.h>

void busy_timer_start_app(const BusyAppConfig* app_config) {
    if(!furi_record_exists(RECORD_BUSY_APP)) {
        Desktop* desktop = furi_record_open(RECORD_DESKTOP);
        while(!desktop_replace_current_app(desktop, "busy", BUSY_APP_TIMER_MODE)) {
            furi_thread_yield();
        }
        furi_record_close(RECORD_DESKTOP);
    }

    BusyApp* busy_app = furi_record_open(RECORD_BUSY_APP);

    busy_set_config(busy_app, app_config);
    busy_show_timer(busy_app);

    furi_record_close(RECORD_BUSY_APP);
}

void busy_timer_exit_app(void) {
    if(furi_record_exists(RECORD_BUSY_APP)) {
        BusyApp* busy_app = furi_record_open(RECORD_BUSY_APP);
        busy_request_exit(busy_app);
        furi_record_close(RECORD_BUSY_APP);
    }
}
