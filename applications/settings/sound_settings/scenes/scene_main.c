#include "../sound_settings.h"
#include "../models/volume.h"
#include "../widgets/slider_view.h"
#include <settings_helpers/gui_params.h>

#define FRONT_SLIDER_GRADIENT_START ((Color)COLOR_MAKE_HEX(0x104224))
#define FRONT_SLIDER_GRADIENT_STOP  ((Color)COLOR_MAKE_HEX(0x16A34A))

typedef enum {
    SceneEventVolumeChanged = AppEventSceneEventsStart,
    SceneEventBackPressed
} SceneEvent;

typedef struct {
    SliderView* front_slider;
    SliderView* back_slider;

    _Atomic uint8_t volume;
} SettingsSceneSound;

static bool scene_main_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SoundSettings* instance = context;

    bool consumed = false;
    SceneEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SceneEventBackPressed;
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(consumed) {
        sound_settings_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void scene_main_slider_view_callback(int32_t value, void* context) {
    furi_assert(context);

    SoundSettings* instance = context;
    SettingsSceneSound* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->volume = value;
    sound_settings_send_custom_event(instance, SceneEventVolumeChanged);
}

static void scene_main_on_enter(void* context) {
    furi_assert(context);

    SoundSettings* instance = context;
    SettingsSceneSound* data = scene_manager_get_current_scene_data(instance->scene_manager);

    uint8_t volume = volume_model_get(instance->model);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, scene_main_input_callback, instance);

        data->front_slider = slider_view_alloc(instance->front_scene_window);
        slider_view_set_range(
            data->front_slider, SETTINGS_VOLUME_RANGE_MIN, SETTINGS_VOLUME_RANGE_MAX);
        slider_view_set_step(data->front_slider, SETTINGS_VOLUME_STEP);
        slider_view_set_value(data->front_slider, volume);
        slider_view_set_suffix(data->front_slider, "%");
        slider_view_set_bar_gradient(
            data->front_slider, FRONT_SLIDER_GRADIENT_START, FRONT_SLIDER_GRADIENT_STOP);
        slider_view_add_level_image(data->front_slider, 1, IMG_PATH("speaker_on_front_7x7.bin"));
        slider_view_add_level_image(data->front_slider, 0, IMG_PATH("speaker_off_front_7x7.bin"));
        slider_view_set_callback(data->front_slider, scene_main_slider_view_callback, instance);

        data->back_slider = slider_view_alloc(instance->back_scene_window);
        slider_view_set_range(
            data->back_slider, SETTINGS_VOLUME_RANGE_MIN, SETTINGS_VOLUME_RANGE_MAX);
        slider_view_set_step(data->back_slider, SETTINGS_VOLUME_STEP);
        slider_view_set_value(data->back_slider, volume);
        slider_view_set_suffix(data->back_slider, "%");
        slider_view_add_level_image(data->back_slider, 1, IMG_PATH("speaker_on_back_12x12.bin"));
        slider_view_add_level_image(data->back_slider, 0, IMG_PATH("speaker_off_back_12x12.bin"));
    });
}

static void scene_main_on_exit(void* context) {
    furi_assert(context);

    SoundSettings* instance = context;
    SettingsSceneSound* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, scene_main_input_callback);

        slider_view_free(data->front_slider);
        slider_view_free(data->back_slider);
    });
}

static bool scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SoundSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventVolumeChanged: {
            SettingsSceneSound* data =
                scene_manager_get_current_scene_data(instance->scene_manager);

            volume_model_set(instance->model, data->volume);
            audio_play_file(instance->audio, SOUND_PATH("volume_change.snd"));

            consumed = true;
            break;
        }

        case SceneEventBackPressed:
            scene_manager_handle_back_event(instance->scene_manager);
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene sound_scene_main = {
    .enter_callback = scene_main_on_enter,
    .exit_callback = scene_main_on_exit,
    .event_callback = scene_main_on_event,
    .data_size = sizeof(SettingsSceneSound),
};
