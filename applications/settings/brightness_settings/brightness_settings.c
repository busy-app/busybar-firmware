#include "brightness_settings.h"
#include "scenes/brightness_scenes.h"
#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>

static bool brightness_settings_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    BrightnessSettings* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        brightness_settings_send_custom_event(instance, AppEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void brightness_settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    BrightnessSettings* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void brightness_settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    BrightnessSettings* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool brightness_settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BrightnessSettings* instance = context;

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

static BrightnessSettings* brightness_settings_alloc(void) {
    BrightnessSettings* instance = malloc(sizeof(BrightnessSettings));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager = scene_manager_alloc(
        brightness_settings_scenes, COUNT_OF(brightness_settings_scenes), instance);

    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);

    instance->model = brightness_model_alloc();

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, brightness_settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_set_header_text(instance->back_nav_bar, "SETTINGS");
        nav_bar_push_location(instance->back_nav_bar, "BRIGHTNESS");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), SETTINGS_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 2, 2, 0, 0);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        brightness_settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        brightness_settings_event_queue_callback,
        instance);

    scene_manager_next_scene(instance->scene_manager, SceneIdMain);

    return instance;
}

static void brightness_settings_free(BrightnessSettings* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, brightness_settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    brightness_model_free(instance->model);

    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_STATUS_LIGHTS);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t brightness_settings_entry(void* arg) {
    if(arg) {
        BrightnessModel* model = brightness_model_alloc();
        SettingsAppDescriptor* descriptor = arg;

        furi_string_set_str(descriptor->front_title, "Brightness");
        furi_string_set_str(descriptor->back_title, "BRIGHTNESS");
        brightness_model_format(model, descriptor->menu_extra);
        furi_string_set_str(descriptor->front_icon, IMG_PATH("sun_front_7x7.bin"));
        furi_string_set_str(descriptor->back_icon, IMG_PATH("sun_back_12x12.bin"));

        brightness_model_free(model);
        return 0;
    }

    BrightnessSettings* instance = brightness_settings_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, brightness_settings_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    brightness_settings_free(instance);

    return 0;
}

void brightness_settings_send_custom_event(BrightnessSettings* instance, uint32_t event) {
    furi_assert(instance);

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}
