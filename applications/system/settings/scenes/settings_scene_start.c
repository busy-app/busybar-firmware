#include "../settings.h"
#include "settings_scenes.h"
#include "../storage_macros.h"

#include <gui/modules/app_title_card.h>

#define ENTER_IMAGE_ANIM_START 0
#define ENTER_IMAGE_ANIM_END   59

#define ENTER_TEXT_ANIM_START       -8
#define ENTER_TEXT_ANIM_END         0
#define ENTER_TEXT_ANIM_DURATION_MS 165

#define EXIT_IMAGE_ANIM_START 60
#define EXIT_IMAGE_ANIM_END   67

#define EXIT_TEXT_ANIM_START       0
#define EXIT_TEXT_ANIM_END         -8
#define EXIT_TEXT_ANIM_DURATION_MS 135

typedef struct {
    AppTitleCard* front_card;
    AppTitleCard* back_card;
} SettingsSceneStart;

static bool settings_scene_start_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    SettingsCustomEvent custom_event;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SettingsCustomEventShortPressed;
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

        data->front_card = app_title_card_alloc(instance->front_scene_window, true);
        app_title_card_set_text(data->front_card, "SETTINGS");
        app_title_card_set_image(
            data->front_card, SETTINGS_ANIM_PATH("settings_front_13x13.anim"));
        app_title_card_run_text_anim(
            data->front_card,
            ENTER_TEXT_ANIM_START,
            ENTER_TEXT_ANIM_END,
            ENTER_TEXT_ANIM_DURATION_MS);
        app_title_card_run_image_anim(
            data->front_card, ENTER_IMAGE_ANIM_START, ENTER_IMAGE_ANIM_END);

        data->back_card = app_title_card_alloc(instance->back_scene_window, false);
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

static void settings_scene_start_run_exit_animations(SettingsApp* instance) {
    SettingsSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);
    with_gui(instance->gui, {
        app_title_card_run_text_anim(
            data->front_card, EXIT_TEXT_ANIM_START, EXIT_TEXT_ANIM_END, EXIT_TEXT_ANIM_DURATION_MS);
        app_title_card_run_image_anim(
            data->front_card, EXIT_IMAGE_ANIM_START, EXIT_IMAGE_ANIM_END);
    });
}

static bool settings_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    SettingsApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SettingsCustomEventShortPressed:
            scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdMain);
            consumed = true;
            break;

        case SettingsCustomEventAboutToExit:
            settings_scene_start_run_exit_animations(instance);
            consumed = true;
            break;

        default:
            break;
        }
    }

    return consumed;
}

const Scene settings_scene_start = {
    .enter_callback = settings_scene_start_on_enter,
    .exit_callback = settings_scene_start_on_exit,
    .event_callback = settings_scene_start_on_event,
    .data_size = sizeof(SettingsSceneStart),
};
