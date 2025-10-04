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
    SettingsSceneMainMenuIndexLanguage,
    SettingsSceneMainMenuIndexDebugApps,

    SettingsSceneMainMenuIndexesCount,
} SettingsSceneMainMenuIndex;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    _Atomic SettingsSceneMainMenuIndex menu_idx;
} SettingsSceneMain;

typedef struct {
    L10nKey nav_bar_key;
    SettingsAppSceneId scene_id;
} NextSceneParameters;

static const NextSceneParameters next_scenes_parameters[] = {
    [SettingsSceneMainMenuIndexSound] =
        {
            .nav_bar_key = L10N_KEY_SETTINGS_MAIN_SOUND_BACK,
            .scene_id = SettingsAppSceneIdSound,
        },
    [SettingsSceneMainMenuIndexBrightness] =
        {
            .nav_bar_key = L10N_KEY_SETTINGS_MAIN_BRIGHTNESS_BACK,
            .scene_id = SettingsAppSceneIdBrightness,
        },
    [SettingsSceneMainMenuIndexLanguage] =
        {
            .nav_bar_key = L10N_KEY_SETTINGS_MAIN_LANGUAGE_BACK,
            .scene_id = SettingsAppSceneIdLanguage,
        },
    [SettingsSceneMainMenuIndexDebugApps] =
        {
            .nav_bar_key = L10N_KEY_SETTINGS_MAIN_DEBUG_APPS_BACK,
            .scene_id = SettingsAppSceneIdDebugApps,
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

    uint8_t volume = settings_volume_get(instance);
    char volume_text[snprintf(NULL, 0, "%u%%", UINT8_MAX) + 1];
    sprintf(volume_text, "%u%%", volume);

    char brightness_text[32];
    if(settings_brightness_get_mode(instance) == SettingsBrightnessModeAuto) {
        strcpy(brightness_text, l10n_get(instance->l10n, L10N_KEY_SETTINGS_BRIGHTNESS_MODE_AUTO));
    } else {
        uint8_t brightness = settings_brightness_get(instance);
        snprintf(brightness_text, sizeof(brightness_text), "%u%%", brightness);
    }

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_scene_window);

        menu_add_item(
            data->front_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_SOUND_FRONT),
            volume_text,
            SETTINGS_IMG_PATH("sound_front_7x7.bin"),
            SettingsSceneMainMenuIndexSound,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_BRIGHTNESS_FRONT),
            brightness_text,
            SETTINGS_IMG_PATH("brightness_front_7x7.bin"),
            SettingsSceneMainMenuIndexBrightness,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_LANGUAGE_FRONT),
            NULL,
            SETTINGS_IMG_PATH("language_front_7x7.bin"),
            SettingsSceneMainMenuIndexLanguage,
            settings_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->front_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_DEBUG_APPS_FRONT),
            NULL,
            SETTINGS_IMG_PATH("debug_front_7x7.bin"),
            SettingsSceneMainMenuIndexDebugApps,
            settings_scene_setup_menu_callback,
            instance);

        menu_set_selected_item_index(data->front_menu, data->menu_idx);

        data->back_menu = menu_alloc(instance->back_scene_window);

        menu_add_item(
            data->back_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_SOUND_BACK),
            volume_text,
            SETTINGS_IMG_PATH("sound_on_back_12x12.bin"),
            SettingsSceneMainMenuIndexSound,
            NULL,
            instance);
        menu_add_item(
            data->back_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_BRIGHTNESS_BACK),
            brightness_text,
            SETTINGS_IMG_PATH("brightness_back_12x12.bin"),
            SettingsSceneMainMenuIndexBrightness,
            NULL,
            instance);
        menu_add_item(
            data->back_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_LANGUAGE_BACK),
            NULL,
            SETTINGS_IMG_PATH("language_back_12x12.bin"),
            SettingsSceneMainMenuIndexLanguage,
            NULL,
            instance);
        menu_add_item(
            data->back_menu,
            l10n_get(instance->l10n, L10N_KEY_SETTINGS_MAIN_DEBUG_APPS_BACK),
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
        if(event->event == SceneCustomEventMenuItemClicked) {
            SettingsSceneMain* data =
                scene_manager_get_current_scene_data(instance->scene_manager);
            const NextSceneParameters* next_scene_parameters =
                &next_scenes_parameters[data->menu_idx];

            const char* nav_bar = l10n_get(instance->l10n, next_scene_parameters->nav_bar_key);
            settings_push_location(instance, nav_bar);
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
