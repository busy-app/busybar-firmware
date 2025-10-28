#include "../settings.h"
#include "../models/brightness.h"
#include "../models/volume.h"
#include "settings_scenes.h"
#include "../storage_macros.h"

#include <gui/modules/menu.h>

typedef enum {
    SceneCustomEventMenuItemClicked = SettingsCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef enum {
    SettingsSceneMainMenuIndexSound,
    SettingsSceneMainMenuIndexBrightness,
    SettingsSceneMainMenuIndexDebugApps,
    SettingsSceneMainMenuIndexFwUpdate,
    SettingsSceneMainMenuIndexMatter,

    SettingsSceneMainMenuIndexesCount,
} SettingsSceneMainMenuIndex;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    _Atomic SettingsSceneMainMenuIndex menu_idx;
} SettingsSceneMain;

typedef struct {
    const char* nav_bar_entry;
    SettingsAppSceneId scene_id;
} NextSceneParameters;

static const NextSceneParameters next_scenes_parameters[] = {
    [SettingsSceneMainMenuIndexSound] =
        {
            .nav_bar_entry = "SOUND",
            .scene_id = SettingsAppSceneIdSound,
        },
    [SettingsSceneMainMenuIndexBrightness] =
        {
            .nav_bar_entry = "BRIGHTNESS",
            .scene_id = SettingsAppSceneIdBrightness,
        },
    [SettingsSceneMainMenuIndexFwUpdate] =
        {
            .nav_bar_entry = "FW UPDATE",
            .scene_id = SettingsAppSceneIdFwUpdate,
        },
    [SettingsSceneMainMenuIndexDebugApps] =
        {
            .nav_bar_entry = "DEBUG",
            .scene_id = SettingsAppSceneIdDebugApps,
        },
    [SettingsSceneMainMenuIndexMatter] =
        {
            .nav_bar_entry = "SMART HOME",
            .scene_id = SettingsAppSceneIdMatter,
        },
};

static_assert(COUNT_OF(next_scenes_parameters) == SettingsSceneMainMenuIndexesCount);

static void settings_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->menu_idx = index;
    settings_send_custom_event(instance, SceneCustomEventMenuItemClicked);
}

static void settings_scene_main_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneMain* data = scene_manager_get_current_scene_data(instance->scene_manager);

    FuriString* volume_text = furi_string_alloc_printf("%u%%", settings_volume_get(instance));
    FuriString* brightness_text =
        (settings_brightness_get_mode(instance) == SettingsBrightnessModeAuto) ?
            furi_string_alloc_set("Auto") :
            furi_string_alloc_printf("%u%%", settings_brightness_get(instance));

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_scene_window);

        menu_add_item(
            data->front_menu,
            "Sound",
            furi_string_get_cstr(volume_text),
            SETTINGS_IMG_PATH("sound_front_7x7.bin"),
            SettingsSceneMainMenuIndexSound,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "Brightness",
            furi_string_get_cstr(brightness_text),
            SETTINGS_IMG_PATH("brightness_front_7x7.bin"),
            SettingsSceneMainMenuIndexBrightness,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "FW Update",
            "",
            SETTINGS_IMG_PATH("fw_update_8x8.bin"),
            SettingsSceneMainMenuIndexFwUpdate,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "Debug apps",
            "",
            SETTINGS_IMG_PATH("debug_front_7x7.bin"),
            SettingsSceneMainMenuIndexDebugApps,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            "Smart home",
            "",
            SETTINGS_IMG_PATH("house_front_7x7.bin"),
            SettingsSceneMainMenuIndexMatter,
            settings_scene_setup_menu_callback,
            instance);

        menu_set_selected_item_index(data->front_menu, data->menu_idx);

        data->back_menu = menu_alloc(instance->back_scene_window);

        menu_add_item(
            data->back_menu,
            "SOUND",
            furi_string_get_cstr(volume_text),
            SETTINGS_IMG_PATH("sound_on_back_12x12.bin"),
            SettingsSceneMainMenuIndexSound,
            NULL,
            instance);
        menu_add_item(
            data->back_menu,
            "BRIGHTNESS",
            furi_string_get_cstr(brightness_text),
            SETTINGS_IMG_PATH("brightness_back_12x12.bin"),
            SettingsSceneMainMenuIndexBrightness,
            NULL,
            instance);
        menu_add_item(
            data->back_menu,
            "FW UPDATE",
            NULL,
            SETTINGS_IMG_PATH("fw_update_12x12.bin"),
            SettingsSceneMainMenuIndexFwUpdate,
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
        menu_add_item(
            data->back_menu,
            "SMART HOME",
            NULL,
            SETTINGS_IMG_PATH("house_back_12x12.bin"),
            SettingsSceneMainMenuIndexMatter,
            NULL,
            instance);

        menu_set_selected_item_index(data->back_menu, data->menu_idx);
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);
    });

    furi_string_free(volume_text);
    furi_string_free(brightness_text);
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
        if(event->event == SceneCustomEventMenuItemClicked) {
            SettingsSceneMain* data =
                scene_manager_get_current_scene_data(instance->scene_manager);
            const NextSceneParameters* next_scene_parameters =
                &next_scenes_parameters[data->menu_idx];

            settings_push_location(instance, next_scene_parameters->nav_bar_entry);
            scene_manager_next_scene(instance->scene_manager, next_scene_parameters->scene_id);

            consumed = true;
        }
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
