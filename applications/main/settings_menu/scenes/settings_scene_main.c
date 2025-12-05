#include "../settings.h"
#include "settings_scenes.h"
#include <applications.h>
#include <settings_helpers/app_desc.h>

#include <gui/modules/menu.h>

typedef enum {
    SceneCustomEventMenuItemClicked = SettingsCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;
} SettingsSceneMain;

static void settings_scene_setup_menu_callback(uint32_t index, void* context) {
    UNUSED(index);
    furi_assert(context);
    SettingsApp* instance = context;
    settings_send_custom_event(instance, SceneCustomEventMenuItemClicked);
}

static void settings_scene_main_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SettingsAppSceneIdMain);

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_scene_window);
        data->back_menu = menu_alloc(instance->back_scene_window);

        uint32_t passed_index = 0;
        const char* passed_app_name = instance->launching_subapp;
        SettingsAppDescriptor* descriptor = settings_app_descriptor_alloc();

        for(size_t i = 0; i < FLIPPER_SETTINGS_APPS_COUNT; i++) {
            settings_app_descriptor_reset(descriptor);
            const FlipperInternalApplication* app = &FLIPPER_SETTINGS_APPS[i];
            app->app(descriptor);

            menu_add_item(
                data->front_menu,
                furi_string_get_cstr(descriptor->front_title),
                furi_string_get_cstr(descriptor->menu_extra),
                furi_string_get_cstr(descriptor->front_icon),
                i,
                settings_scene_setup_menu_callback,
                instance);
            menu_add_item(
                data->back_menu,
                furi_string_get_cstr(descriptor->back_title),
                furi_string_get_cstr(descriptor->menu_extra),
                furi_string_get_cstr(descriptor->back_icon),
                i,
                NULL,
                NULL);

            if(passed_app_name) {
                if(strcmp(passed_app_name, app->appid) == 0) {
                    passed_index = i;
                }
            }
        }

        settings_app_descriptor_free(descriptor);

        menu_set_selected_item_index(data->front_menu, passed_index);
        menu_set_selected_item_index(data->back_menu, passed_index);

        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);
    });

    instance->launching_subapp = NULL;
}

static void settings_scene_main_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SettingsAppSceneIdMain);

    with_gui(instance->gui, {
        menu_free(data->front_menu);
        menu_free(data->back_menu);
    });
}

static bool settings_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, SettingsAppSceneIdMain);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneCustomEventMenuItemClicked) {
            uint32_t index = menu_get_selected_item_index(data->front_menu);
            const FlipperInternalApplication* app = &FLIPPER_SETTINGS_APPS[index];
            desktop_replace_current_app(instance->desktop, app->name, NULL);

            consumed = true;
        }
    }

    return consumed;
}

const Scene settings_scene_main = {
    .enter_callback = settings_scene_main_on_enter,
    .exit_callback = settings_scene_main_on_exit,
    .event_callback = settings_scene_main_on_event,
    .data_size = sizeof(SettingsSceneMain),
};
