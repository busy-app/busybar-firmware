#include "sound_settings.h"
#include "scenes/sound_scenes.h"
#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>

#define TAG "SoundSettings"

// TODO: FW-1091 Refactor Audio service and settings to be deadlock-proof
#define EVENT_QUEUE_TIMEOUT_MS (10)

static bool sound_settings_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    SoundSettings* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        sound_settings_send_custom_event(instance, AppEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void sound_settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SoundSettings* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void sound_settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SoundSettings* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool sound_settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SoundSettings* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            consumed = true;
        }
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(instance->input_queue, event, FuriWaitForever) == FuriStatusOk);
    }

    return consumed;
}

static SoundSettings* sound_settings_alloc(void) {
    SoundSettings* instance = malloc(sizeof(SoundSettings));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(sound_settings_scenes, COUNT_OF(sound_settings_scenes), instance);

    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->audio = furi_record_open(RECORD_AUDIO);

    instance->model = volume_model_alloc();

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, sound_settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_set_header_text(instance->back_nav_bar, "SETTINGS");
        nav_bar_push_location(instance->back_nav_bar, "SOUND");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), SETTINGS_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 1, 0, 0, 2);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        sound_settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        sound_settings_event_queue_callback,
        instance);

    scene_manager_next_scene(instance->scene_manager, SceneIdMain);

    return instance;
}

static void sound_settings_free(SoundSettings* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, sound_settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    volume_model_free(instance->model);

    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t sound_settings_entry(void* arg) {
    if(arg) {
        VolumeModel* model = volume_model_alloc();
        SettingsAppDescriptor* descriptor = arg;

        furi_string_set_str(descriptor->front_title, "Sound");
        furi_string_set_str(descriptor->back_title, "Sound");
        volume_model_format(model, descriptor->menu_extra);
        furi_string_set_str(descriptor->front_icon, IMG_PATH("speaker_front_8x8.image"));
        furi_string_set_str(descriptor->back_icon, IMG_PATH("speaker_back_100_11x11.image"));

        volume_model_free(model);
        return 0;
    }

    SoundSettings* instance = sound_settings_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, sound_settings_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    sound_settings_free(instance);

    return 0;
}

void sound_settings_send_custom_event(SoundSettings* instance, uint32_t event) {
    furi_assert(instance);

    const FuriStatus status = furi_message_queue_put(
        instance->event_queue, &event, furi_ms_to_ticks(EVENT_QUEUE_TIMEOUT_MS));

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        FURI_LOG_W(TAG, "Deadlock avoided, see FW-1091");
    }
}
