#include "js_app_launcher_i.h"

#include <desktop/desktop.h>
#include <loader/loader.h>

#include <js_app/js_app_common.h>

#define JS_APP_LAUNCHER_APP_ID "js_app_launcher"

#define APPS_MENU_JS_APP_ID_LEN_EXTRA (sizeof(JS_APP_LAUNCHER_ARG_SKIP_MENU))

bool js_app_launcher_start(const char* app_id, JsAppLauncherMode mode) {
    furi_check(app_id);
    furi_check(mode < JsAppLauncherModeMax);

    char args[JS_APP_ID_LEN_MAX + APPS_MENU_JS_APP_ID_LEN_EXTRA];
    strlcpy(args, app_id, sizeof(args) - strlen(JS_APP_LAUNCHER_ARG_SKIP_MENU));

    if(mode == JsAppLauncherModeSkipMenu) {
        strlcat(args, JS_APP_LAUNCHER_ARG_SKIP_MENU, sizeof(args));
    }

    Desktop* desktop = furi_record_open(RECORD_DESKTOP);
    const bool success = desktop_replace_current_app(desktop, JS_APP_LAUNCHER_APP_ID, args);
    furi_record_close(RECORD_DESKTOP);

    return success;
}

bool js_app_launcher_stop(void) {
    bool success = false;

    FuriString* app_id = furi_string_alloc();
    Loader* loader = furi_record_open(RECORD_LOADER);

    do {
        if(!loader_get_application_id(loader, app_id)) {
            break;
        }

        if(!furi_string_equal(app_id, JS_APP_LAUNCHER_APP_ID)) {
            break;
        }

        if(!loader_send_signal(loader, FuriSignalExit, JS_APP_LAUNCHER_ARG_FORGET)) {
            break;
        }

        success = true;
    } while(false);

    furi_record_close(RECORD_LOADER);
    furi_string_free(app_id);

    return success;
}
