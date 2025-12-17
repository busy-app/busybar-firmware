#include "../busy_i.h"

#include <m-array.h>

#include "../widgets/theme_picker.h"

#define THEME_NAME_LEN_MAX (64)

ARRAY_DEF(BusyThemeArray, BusyTheme*, BUSY_THEME_OPLIST);

typedef struct {
    ThemePicker* front_picker;
    ThemePicker* back_picker;
    BusyThemeArray_t themes;
} BusySceneSetupTheme;

static void busy_scene_setup_theme_picker_callback(uint32_t index, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    busy_send_custom_event(instance, index);
}

static void busy_scene_setup_theme_read_themes(BusyThemeArray_t data) {
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
                BusyThemeArray_push_back(data, theme);
            }
        }

    } while(false);

    busy_theme_free(theme);
    storage_file_free(themes_dir);

    furi_record_close(RECORD_STORAGE);
}

static void busy_scene_setup_theme_handle_theme_changed(BusyApp* instance, uint32_t theme_idx) {
    if(theme_idx == 0) {
        busy_theme_reset(instance->theme);

    } else {
        const BusySceneSetupTheme* data =
            scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTheme);
        busy_theme_set(instance->theme, *BusyThemeArray_cget(data->themes, theme_idx - 1));
    }
}

static void busy_scene_setup_theme_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTheme* data =
        scene_manager_get_scene_data(instance->scene_manager, BusyAppSceneIdSetupTheme);

    BusyThemeArray_init(data->themes);
    busy_scene_setup_theme_read_themes(data->themes);

    with_gui(instance->gui, {
        data->front_picker = theme_picker_alloc(instance->front_window);
        widget_set_align(theme_picker_get_base(data->front_picker), AlignCenter);

        data->back_picker = theme_picker_alloc(instance->back_window);
        widget_set_align(theme_picker_get_base(data->back_picker), AlignCenter);

        // Add default theme
        theme_picker_add_item(data->front_picker, BUSY_IMG_PATH("theme_preview_72x16.bin"));
        theme_picker_add_item(data->back_picker, BUSY_IMG_PATH("theme_preview_72x16.bin"));

        BusyThemeArray_it_ct it;
        for(BusyThemeArray_it(it, data->themes); !BusyThemeArray_end_p(it);
            BusyThemeArray_next(it)) {
            const BusyTheme* theme = *BusyThemeArray_cref(it);
            theme_picker_add_item(data->front_picker, busy_theme_get_preview_path(theme));
            theme_picker_add_item(data->back_picker, busy_theme_get_preview_path(theme));
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
        theme_picker_free(data->back_picker);
    });

    BusyThemeArray_clear(data->themes);
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
