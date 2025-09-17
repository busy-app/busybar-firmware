#include "../settings.h"

#include "../storage_macros.h"
#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <gui/modules/bar.h>

typedef enum {
    SceneCustomEventVolumeChanged = SettingsCustomEventSceneEventsStart,
    SceneCustomEventBackPressed
} SceneCustomEvent;

typedef struct {
    Label* label_status_back;
    Image* image_back;
    Bar* bar_back;
    Label* label_fw_name_download;
    Label* label_fw_current_version;

    Label* label_status_front;
    Bar* bar_front;

    uint8_t bar_volume;

} SettingsSceneFwUpdate;

static void settings_scene_fw_update_scene_update(SettingsApp* instance) {
    furi_assert(instance);
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);

    bar_set_value(data->bar_front, data->bar_volume);
    bar_set_value(data->bar_back, data->bar_volume);
    label_set_text_fmt(data->label_status_front, "Downloading      %d%%", data->bar_volume);
    label_set_text_fmt(data->label_status_back, "Downloading (%d%%)", data->bar_volume);
}

static bool settings_scene_fw_update_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);

    bool consumed = false;
    SceneCustomEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SceneCustomEventBackPressed;
            consumed = true;
            settings_send_custom_event(instance, custom_event);
            break;
        case InputKeyUp:
            data->bar_volume++;
            if(data->bar_volume > 99) {
                data->bar_volume = 99;
            }
            settings_scene_fw_update_scene_update(instance);
            consumed = true;
            break;
        case InputKeyDown:
            if(data->bar_volume > 0) {
                data->bar_volume--;
            }
            settings_scene_fw_update_scene_update(instance);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

static void settings_scene_fw_update_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, settings_scene_fw_update_input_callback, instance);

        // GuiDisplayIdBack
        Widget* root_back = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        data->label_status_back = label_alloc(root_back);
        label_set_text(data->label_status_back, "Downloading (27%)");
        widget_set_pos(label_get_base(data->label_status_back), 22, 23);

        data->bar_back = bar_alloc(root_back);
        widget_set_pos(bar_get_base(data->bar_back), 7, 40);
        bar_set_size(data->bar_back, 132, 5);

        data->label_fw_name_download = label_alloc(root_back);
        label_set_text_font_size(data->label_fw_name_download, LabelFontSizeSmall);
        label_set_text(data->label_fw_name_download, "Firmware Name....tar");
        widget_set_pos(label_get_base(data->label_fw_name_download), 7, 52);

        data->label_fw_current_version = label_alloc(root_back);
        label_set_text_font_size(data->label_fw_current_version, LabelFontSizeSmall);
        label_set_text(data->label_fw_current_version, "Current Version: 1.0.0");
        widget_set_pos(label_get_base(data->label_fw_current_version), 7, 69);

        data->image_back = image_alloc(root_back);
        image_set_source(data->image_back, SETTINGS_IMG_PATH("fw_update_12x12.bin"));
        widget_set_pos(image_get_base(data->image_back), 7, 23);

        // GuiDisplayIdFront
        Widget* root_front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        data->label_status_front = label_alloc(root_front);
        label_set_text_color(data->label_status_front, (Color){255, 127, 0});
        label_set_text(data->label_status_front, "Downloading      27%");
        widget_set_pos(label_get_base(data->label_status_front), 1, 0);

        data->bar_front = bar_alloc(root_front);
        widget_set_pos(bar_get_base(data->bar_front), 2, 10);
        bar_set_size(data->bar_front, 68, 5);
        bar_set_color(data->bar_front, (Color){0, 200, 30});
    });
}

static void settings_scene_fw_update_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, settings_scene_fw_update_input_callback);

        label_free(data->label_status_back);
        image_free(data->image_back);
        bar_free(data->bar_back);
        label_free(data->label_status_front);
        label_free(data->label_fw_name_download);
        label_free(data->label_fw_current_version);
        bar_free(data->bar_front);
    });
}

static bool settings_scene_fw_update_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneCustomEventVolumeChanged: {
            SettingsSceneFwUpdate* data =
                scene_manager_get_current_scene_data(instance->scene_manager);
            UNUSED(data);
            //settings_volume_set(instance, data->volume);
            audio_play_file(instance->audio, SETTINGS_SOUND_PATH("volume_change.snd"));

            consumed = true;
            break;
        }

        case SceneCustomEventBackPressed:
            scene_manager_handle_back_event(instance->scene_manager);
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(instance);
    }

    return consumed;
}

const Scene settings_scene_fw_update = {
    .enter_callback = settings_scene_fw_update_on_enter,
    .exit_callback = settings_scene_fw_update_on_exit,
    .event_callback = settings_scene_fw_update_on_event,
    .data_size = sizeof(SettingsSceneFwUpdate),
};
