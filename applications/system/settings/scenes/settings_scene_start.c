#include "../settings.h"
#include "settings_scenes.h"
#include "../storage_macros.h"

#include <gui/modules/app_title_card.h>

typedef struct {
    AppTitleCard* front_card;
    AppTitleCard* back_card;
} SettingsSceneStart;

typedef enum {
    SettingsStartCustomEventShortPressed
} SettingsStartCustomEvent;

static bool settings_scene_start_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    SettingsStartCustomEvent custom_event;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SettingsStartCustomEventShortPressed;
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(consumed) {
        settings_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void settings_scene_start_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, settings_scene_start_input_callback, instance);

        data->front_card = app_title_card_alloc(instance->front_scene_window);
        app_title_card_set_text(data->front_card, "SETTINGS");
        app_title_card_set_image(data->front_card, SETTINGS_IMG_PATH("settings_front_13x13.bin"));

        data->back_card = app_title_card_alloc(instance->back_scene_window);
        app_title_card_set_text(data->back_card, "SETTINGS");
        app_title_card_set_image(data->back_card, SETTINGS_IMG_PATH("settings_back_18x18.bin"));

        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), false);
    });
}

static void settings_scene_start_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, settings_scene_start_input_callback);

        app_title_card_free(data->front_card);
        app_title_card_free(data->back_card);
    });
}

static bool settings_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    SettingsApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SettingsStartCustomEventShortPressed) {
            scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdMain);
        }

        consumed = true;
    }

    return consumed;
}

const Scene settings_scene_start = {
    .enter_callback = settings_scene_start_on_enter,
    .exit_callback = settings_scene_start_on_exit,
    .event_callback = settings_scene_start_on_event,
    .data_size = sizeof(SettingsSceneStart),
};
