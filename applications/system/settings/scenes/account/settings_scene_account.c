#include "../../settings.h"
#include "../settings_scenes.h"

#include <mqtt_client/mqtt_client.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneSubmenuIndexPairing,
} SceneSubmenuIndex;

typedef enum {
    SceneCustomEventMenuItemClicked = SettingsCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    MqttClientStatus status;

    Submenu* submenus[GuiDisplayIdMax];
    _Atomic size_t menu_idx;
} SettingsSceneAccount;

static void settings_scene_account_submenu_item_callback(uint32_t index, void* context) {
    furi_assert(context);

    SettingsApp* app = context;
    SettingsSceneAccount* scene = scene_manager_get_current_scene_data(app->scene_manager);

    scene->menu_idx = index;
    settings_send_custom_event(app, SceneCustomEventMenuItemClicked);
}

static void settings_scene_account_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneAccount* scene = scene_manager_get_current_scene_data(app->scene_manager);

    MqttClient* mqtt_client = furi_record_open(RECORD_MQTT);
    scene->status = mqtt_client_get_status(mqtt_client);
    furi_record_close(RECORD_MQTT);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            Widget* window = (display == GuiDisplayIdFront) ? app->front_scene_window :
                                                              app->back_scene_window;
            bool add_callback = display == GuiDisplayIdBack;
            if(scene->status == MqttClientStatusConnectedNotLinked) {
                scene->submenus[display] = submenu_alloc(window);
                submenu_add_item(
                    scene->submenus[display],
                    "Get pairing code",
                    SceneSubmenuIndexPairing,
                    add_callback ? settings_scene_account_submenu_item_callback : NULL,
                    app);
            } else if(scene->status == MqttClientStatusConnectedLinked) {
                submenu_add_item(
                    scene->submenus[display],
                    "Unlink",
                    SceneSubmenuIndexPairing,
                    add_callback ? settings_scene_account_submenu_item_callback : NULL,
                    app);
                submenu_add_item(
                    scene->submenus[display],
                    "Cancel",
                    SceneSubmenuIndexPairing,
                    add_callback ? settings_scene_account_submenu_item_callback : NULL,
                    app);
            }
        }
    });
}

static void settings_scene_account_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneAccount* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            submenu_free(scene->submenus[display]);
        }
    });
}

static bool settings_scene_account_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* app = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneCustomEventMenuItemClicked) {
            SettingsSceneAccount* scene = scene_manager_get_current_scene_data(app->scene_manager);

            if(scene->status == MqttClientStatusConnectedLinked) {
                if(scene->menu_idx == SceneSubmenuIndexPairing) {
                    // scene_manager_next_scene(app->scene_manager, SettingsAppSceneIdAccountPairing);
                }
            } else {
                furi_crash();
            }

            consumed = true;
        }

    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(app);
    }

    return consumed;
}

const Scene settings_scene_account = {
    .enter_callback = settings_scene_account_on_enter,
    .exit_callback = settings_scene_account_on_exit,
    .event_callback = settings_scene_account_on_event,
    .data_size = sizeof(SettingsSceneAccount),
};
