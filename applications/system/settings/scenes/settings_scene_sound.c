#include "../settings.h"
#include "../widgets/slider_view.h"
#include "../storage_macros.h"

#define FRONT_SLIDER_GRADIENT_START ((Color)COLOR_MAKE_HEX(0x104224))
#define FRONT_SLIDER_GRADIENT_STOP  ((Color)COLOR_MAKE_HEX(0x16A34A))

typedef struct {
    SliderView* front_slider;
    SliderView* back_slider;

    _Atomic uint8_t volume;
} SettingsSceneSound;

typedef enum {
    SettingsSoundCustomEventShortPressed,
    SettingsSoundCustomEventVolumeChanged,
} SettingsSoundCustomEvent;

static bool settings_scene_sound_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    SettingsSoundCustomEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SettingsSoundCustomEventShortPressed;
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
static void settings_scene_sound_slider_view_callback(int32_t value, void* context) {
    furi_assert(context);
    furi_assert(value <= 100);

    SettingsApp* instance = context;
    SettingsSceneSound* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->volume = value;
    settings_send_custom_event(instance, SettingsSoundCustomEventVolumeChanged);
}

static void settings_scene_sound_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneSound* data = scene_manager_get_current_scene_data(instance->scene_manager);

    uint8_t volume = roundf(100.f * audio_get_volume(instance->audio));

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, settings_scene_sound_input_callback, instance);

        data->front_slider = slider_view_alloc(instance->front_scene_window);
        slider_view_set_range(data->front_slider, 0, 100);
        slider_view_set_step(data->front_slider, 5);
        slider_view_set_value(data->front_slider, volume);
        slider_view_set_suffix(data->front_slider, "%");
        slider_view_set_bar_gradient(
            data->front_slider, FRONT_SLIDER_GRADIENT_START, FRONT_SLIDER_GRADIENT_STOP);
        slider_view_add_level_image(
            data->front_slider, 1, SETTINGS_IMG_PATH("sound_on_front_7x7.bin"));
        slider_view_add_level_image(
            data->front_slider, 0, SETTINGS_IMG_PATH("sound_off_front_7x7.bin"));
        slider_view_set_callback(
            data->front_slider, settings_scene_sound_slider_view_callback, instance);

        data->back_slider = slider_view_alloc(instance->back_scene_window);
        slider_view_set_range(data->back_slider, 0, 100);
        slider_view_set_step(data->back_slider, 5);
        slider_view_set_value(data->back_slider, volume);
        slider_view_set_suffix(data->back_slider, "%");
        slider_view_add_level_image(
            data->back_slider, 1, SETTINGS_IMG_PATH("sound_on_back_12x12.bin"));
        slider_view_add_level_image(
            data->back_slider, 0, SETTINGS_IMG_PATH("sound_off_back_12x12.bin"));
    });
}

static void settings_scene_sound_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneSound* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, settings_scene_sound_input_callback);

        slider_view_free(data->front_slider);
        slider_view_free(data->back_slider);
    });
}

static bool settings_scene_sound_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SettingsSoundCustomEventShortPressed:
            scene_manager_handle_back_event(instance->scene_manager);
            consumed = true;
            break;

        case SettingsSoundCustomEventVolumeChanged: {
            SettingsSceneSound* data =
                scene_manager_get_current_scene_data(instance->scene_manager);

            audio_set_volume(instance->audio, .01f * data->volume);
            audio_play_file(instance->audio, SETTINGS_SOUND_PATH("volume_change.snd"));
            consumed = true;
            break;
        }

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(instance);
    }

    return consumed;
}

const Scene settings_scene_sound = {
    .enter_callback = settings_scene_sound_on_enter,
    .exit_callback = settings_scene_sound_on_exit,
    .event_callback = settings_scene_sound_on_event,
    .data_size = sizeof(SettingsSceneSound),
};
