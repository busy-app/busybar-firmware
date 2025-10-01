#include "../../settings.h"
#include "../../storage_macros.h"
#include "../settings_scenes.h"
#include "matter_scenes_common.h"

#include "../../widgets/matter_code_view.h"
#include "../../widgets/status_view.h"

#include <matter/matter.h>

typedef struct {
    bool ui_initialized;

    StatusView* front_prompt;
    MatterCodeView* back_codes;
} SettingsSceneMatterPairing;

typedef enum {
    SettingsSceneEventSwitchToConnectWifi = SettingsCustomEventSceneEventsStart,
} SettingsSceneEvent;

static void settings_scene_matter_pairing_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneMatterPairing* scene = scene_manager_get_current_scene_data(app->scene_manager);

    scene->ui_initialized = false;

    if(!(wifi_poller_get_state(app->wifi) & WifiPollerStateLinkUp)) {
        settings_send_custom_event(app, SettingsSceneEventSwitchToConnectWifi);
        return;
    }

    FuriString* qr_code = furi_string_alloc();
    FuriString* man_code = furi_string_alloc();

    size_t window_secs = matter_enable_commissioning(app->matter, qr_code, man_code);
    UNUSED(window_secs);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        scene->front_prompt = status_view_alloc(app->front_scene_window);
        status_view_set_icon(scene->front_prompt, SETTINGS_IMG_PATH("info_front_7x7.bin"));
        status_view_set_header(scene->front_prompt, "Look at back\nscreen");

        scene->back_codes = matter_code_view_alloc(app->back_scene_window);
        matter_code_view_set_logo_path(
            scene->back_codes, SETTINGS_IMG_PATH("matter_back_14x14.bin"));
        matter_code_view_set_codes(
            scene->back_codes, furi_string_get_cstr(qr_code), furi_string_get_cstr(man_code));
    });

    scene->ui_initialized = true;

    furi_string_free(qr_code);
    furi_string_free(man_code);
}

static void settings_scene_matter_pairing_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneMatterPairing* scene = scene_manager_get_current_scene_data(app->scene_manager);

    if(!scene->ui_initialized) return;

    with_gui(app->gui, {
        status_view_free(scene->front_prompt);
        matter_code_view_free(scene->back_codes);
    });
}

static bool settings_scene_matter_pairing_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    SettingsApp* app = context;

    bool consumed = false;

    do {
        if(event->type == SceneManagerEventTypeCustom) {
            consumed = matter_scene_replace_current(app, event->event);
            if(consumed) break;

            if(event->event == SettingsSceneEventSwitchToConnectWifi) {
                scene_manager_next_scene(app->scene_manager, SettingsAppSceneIdConnectWifi);
                consumed = true;
            }
        }
    } while(0);

    return consumed;
}

const Scene settings_scene_matter_pairing = {
    .enter_callback = settings_scene_matter_pairing_on_enter,
    .exit_callback = settings_scene_matter_pairing_on_exit,
    .event_callback = settings_scene_matter_pairing_on_event,
    .data_size = sizeof(SettingsSceneMatterPairing),
};
