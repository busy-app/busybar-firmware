#include "../busy_i.h"

#include <m-array.h>

#include "../widgets/theme_picker.h"

#define THEME_NAME_LEN_MAX (64)

typedef struct {
    ThemePickerModel* picker_model;
    ThemePicker* front_picker;
    ThemePicker* back_picker;
} BusySceneSetupTheme;

static bool busy_scene_setup_theme_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    bool consumed = false;
    BusyCustomEvent custom_event;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            custom_event = BusyCustomEventOkShortPressed;
            consumed = true;

        } else if(event->key == InputKeyStart) {
            custom_event = BusyCustomEventStartShortPressed;
            consumed = true;
        }
    }

    if(consumed) {
        busy_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void busy_scene_setup_theme_picker_callback(uint32_t index, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    busy_send_custom_event(instance, index);
}

static void busy_scene_setup_theme_read_extra_themes(ThemePickerModel* model) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    File* themes_dir = storage_file_alloc(storage);
    BusyTheme* theme = busy_theme_alloc();

    do {
        if(!storage_dir_open(themes_dir, BUSY_THEMES_DIR)) {
            break;
        }

        char file_name[THEME_NAME_LEN_MAX];

        while(storage_dir_read(themes_dir, NULL, file_name, THEME_NAME_LEN_MAX)) {
            if(busy_theme_read(theme, file_name)) {
                theme_picker_model_add_item(model, theme);
            }
        }

    } while(false);

    busy_theme_free(theme);
    storage_file_free(themes_dir);

    furi_record_close(RECORD_STORAGE);
}

static void busy_scene_setup_theme_handle_theme_changed(BusyApp* instance, uint32_t theme_idx) {
    const BusySceneSetupTheme* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTheme);

    const BusyTheme* selected_theme = theme_picker_model_get_item(data->picker_model, theme_idx);
    busy_theme_set(instance->theme, selected_theme);

    BusySettings* settings = &instance->settings;
    strlcpy(
        settings->theme_name, busy_theme_get_name(selected_theme), sizeof(settings->theme_name));
}

static void busy_scene_setup_theme_handle_theme_accepted(BusyApp* instance) {
    busy_pop_location(instance);
    scene_manager_previous_scene(instance->scene_manager);
}

static void busy_scene_setup_theme_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTheme* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTheme);

    data->picker_model = theme_picker_model_alloc();

    BusyTheme* default_theme = busy_theme_alloc_default();
    theme_picker_model_add_item(data->picker_model, default_theme);
    busy_theme_free(default_theme);

    busy_scene_setup_theme_read_extra_themes(data->picker_model);

    const uint32_t selected_theme_index =
        theme_picker_model_get_item_index(data->picker_model, instance->theme);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_scene_setup_theme_input_callback, instance);

        data->front_picker = theme_picker_alloc(instance->front_window);
        theme_picker_set_model(data->front_picker, data->picker_model);
        widget_set_align(theme_picker_get_base(data->front_picker), AlignCenter);

        data->back_picker = theme_picker_alloc(instance->back_window);
        theme_picker_set_model(data->back_picker, data->picker_model);
        widget_set_align(theme_picker_get_base(data->back_picker), AlignCenter);

        if(selected_theme_index != THEME_PICKER_MODEL_INVALID_INDEX) {
            theme_picker_set_current_item(data->front_picker, selected_theme_index);
            theme_picker_set_current_item(data->back_picker, selected_theme_index);
        }

        // Needed only once
        theme_picker_set_callback(
            data->front_picker, busy_scene_setup_theme_picker_callback, instance);
    });
}

static void busy_scene_setup_theme_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTheme* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTheme);

    busy_settings_save(&instance->settings);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_scene_setup_theme_input_callback);

        theme_picker_free(data->front_picker);
        theme_picker_free(data->back_picker);
        theme_picker_model_free(data->picker_model);
    });
}

static bool busy_scene_setup_theme_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    BusyApp* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event < BusyCustomEventIndexMax) {
            busy_scene_setup_theme_handle_theme_changed(instance, event->event);

        } else if(event->event == BusyCustomEventStartShortPressed) {
            busy_scene_setup_theme_handle_theme_accepted(instance);

        } else if(event->event == BusyCustomEventOkShortPressed) {
            busy_scene_setup_theme_handle_theme_accepted(instance);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        busy_pop_location(instance);
    }

    return consumed;
}

const Scene busy_scene_setup_theme = {
    .enter_callback = busy_scene_setup_theme_on_enter,
    .exit_callback = busy_scene_setup_theme_on_exit,
    .event_callback = busy_scene_setup_theme_on_event,
    .data_size = sizeof(BusySceneSetupTheme),
};
