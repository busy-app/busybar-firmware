#include "../settings.h"
#include "../storage_macros.h"

#include <gui/modules/menu.h>
#include <gui/modules/submenu.h>

#include <power/power_service/power.h>

#define LOCALE_IMAGE(name) SETTINGS_IMG_PATH("locale_" name "_front_10x7.bin")

static const char* const LOCALE_IMAGES[L10nLocaleCOUNT] = {
    [L10nLocaleEnUs] = LOCALE_IMAGE("en_us"),
    [L10nLocaleRuRu] = LOCALE_IMAGE("ru_ru"),
};

typedef struct {
    Menu* front;
    Submenu* back;
} SettingsSceneLanguage;

static void settings_scene_language_submenu_callback(uint32_t index, void* context) {
    furi_assert(context);
    SettingsApp* instance = context;
    settings_send_custom_event(instance, index);
}

static void settings_scene_language_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneLanguage* scene = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        scene->front = menu_alloc(instance->front_scene_window);
        scene->back = submenu_alloc(instance->back_scene_window);

        for(L10nLocale locale = 0; locale < L10nLocaleCOUNT; locale++) {
            const L10nLocaleInfo* info = l10n_locale_info(locale);
            const char* front_image = LOCALE_IMAGES[locale];

            menu_add_item(
                scene->front,
                info->self_name,
                NULL,
                front_image,
                locale,
                settings_scene_language_submenu_callback,
                instance);

            submenu_add_item(scene->back, info->self_name, locale, NULL, NULL);
        }
    });
}

static void settings_scene_language_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneLanguage* scene = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        menu_free(scene->front);
        submenu_free(scene->back);
    });
}

static bool settings_scene_language_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        L10nLocale locale = event->event;
        l10n_set_global_locale(instance->l10n_service, locale);

        // TODO: display a "reboot needed" screen
        Power* power = furi_record_open(RECORD_POWER);
        power_reboot(power, PowerRebootNormal);

        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(instance);
    }

    return consumed;
}

const Scene settings_scene_language = {
    .enter_callback = settings_scene_language_on_enter,
    .exit_callback = settings_scene_language_on_exit,
    .event_callback = settings_scene_language_on_event,
    .data_size = sizeof(SettingsSceneLanguage),
};
