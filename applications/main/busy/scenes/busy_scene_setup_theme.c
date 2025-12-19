#include "../busy_i.h"

#include <m-array.h>

#include "../widgets/theme_picker.h"

#define THEME_NAME_LEN_MAX (64)

typedef struct {
    ThemePickerModel* picker_model;
    ThemePicker* front_picker;
    // TODO: Solve storage issue
    // ThemePicker* back_picker;
} BusySceneSetupTheme;

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
    busy_theme_set(instance->theme, theme_picker_model_get_item(data->picker_model, theme_idx));
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
        data->front_picker = theme_picker_alloc(instance->front_window);
        theme_picker_set_model(data->front_picker, data->picker_model);
        widget_set_align(theme_picker_get_base(data->front_picker), AlignCenter);

        // data->back_picker = theme_picker_alloc(instance->back_window);
        // theme_picker_set_model(data->back_picker, data->picker_model);
        // widget_set_align(theme_picker_get_base(data->back_picker), AlignCenter);

        if(selected_theme_index != THEME_PICKER_MODEL_INVALID_INDEX) {
            theme_picker_set_current_item(data->front_picker, selected_theme_index);
            // theme_picker_set_current_item(data->back_picker, selected_theme_index);
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

    with_gui(instance->gui, {
        theme_picker_free(data->front_picker);
        // theme_picker_free(data->back_picker);
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
