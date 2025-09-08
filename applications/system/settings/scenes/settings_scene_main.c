#include "../settings.h"
#include "settings_scenes.h"
#include "../storage_macros.h"

#include <gui/modules/menu.h>

typedef enum {
    SettingsSceneMainMenuIndexSound,
    SettingsSceneMainMenuIndexBrightness,
    SettingsSceneMainMenuIndexDebugApps,

    SettingsSceneMainMenuIndexesCount,
} SettingsSceneMainMenuIndex;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    size_t menu_idx;
} SettingsSceneMain;

static void settings_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(context);
    furi_assert(index < SettingsSceneMainMenuIndexesCount);

    SettingsApp* instance = context;
    settings_send_custom_event(instance, index);
}

static void settings_scene_main_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    char volume_text[snprintf(NULL, 0, "%u%%", UINT8_MAX) + 1];
    uint8_t volume = roundf(100.f * audio_get_volume(instance->audio));
    sprintf(volume_text, "%u%%", volume);

    const char* brightness_text;
    uint8_t brightness = back_display_get_brightness(instance->back_display);
    if(brightness == BACK_DISPLAY_BRIGHTNESS_AUTO) {
        brightness_text = "Auto";
    } else {
        char* text = alloca(snprintf(NULL, 0, "%u%%", UINT8_MAX) + 1);
        sprintf(text, "%u%%", brightness);
        brightness_text = text;
    }

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_scene_window);

        menu_add_item(
            data->front_menu,
            "Sound",
            volume_text,
            SETTINGS_IMG_PATH("sound_front_7x7.bin"),
            SettingsSceneMainMenuIndexSound,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "Brightness",
            brightness_text,
            SETTINGS_IMG_PATH("brightness_front_7x7.bin"),
            SettingsSceneMainMenuIndexBrightness,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "Debug apps",
            NULL,
            SETTINGS_IMG_PATH("debug_front_7x7.bin"),
            SettingsSceneMainMenuIndexDebugApps,
            settings_scene_setup_menu_callback,
            instance);

        menu_set_selected_item_index(data->front_menu, data->menu_idx);

        data->back_menu = menu_alloc(instance->back_scene_window);

        menu_add_item(
            data->back_menu,
            "SOUND",
            volume_text,
            SETTINGS_IMG_PATH("sound_on_back_12x12.bin"),
            SettingsSceneMainMenuIndexSound,
            NULL,
            instance);
        menu_add_item(
            data->back_menu,
            "BRIGHTNESS",
            brightness_text,
            SETTINGS_IMG_PATH("brightness_back_12x12.bin"),
            SettingsSceneMainMenuIndexBrightness,
            NULL,
            instance);
        menu_add_item(
            data->back_menu,
            "DEBUG APPS",
            NULL,
            SETTINGS_IMG_PATH("debug_back_12x12.bin"),
            SettingsSceneMainMenuIndexDebugApps,
            NULL,
            instance);

        menu_set_selected_item_index(data->back_menu, data->menu_idx);
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);
    });
}

static void settings_scene_main_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        menu_free(data->front_menu);
        menu_free(data->back_menu);
    });
}

static bool settings_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        data->menu_idx = event->event;

        switch(event->event) {
        case SettingsSceneMainMenuIndexSound:
            settings_push_location(instance, "SOUND");
            scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdSound);
            break;

        case SettingsSceneMainMenuIndexBrightness:
            settings_push_location(instance, "BRIGHTNESS");
            scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdBrightness);
            break;

        case SettingsSceneMainMenuIndexDebugApps:
            settings_push_location(instance, "DEBUG APPS");
            scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdDebugApps);
            break;

        default:
            break;
        }

        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        data->menu_idx = 0;
    }

    return consumed;
}

const Scene settings_scene_main = {
    .enter_callback = settings_scene_main_on_enter,
    .exit_callback = settings_scene_main_on_exit,
    .event_callback = settings_scene_main_on_event,
    .data_size = sizeof(SettingsSceneMain),
};
