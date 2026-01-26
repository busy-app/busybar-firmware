#include "system_settings.h"
#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>

static bool system_settings_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    SystemSettings* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        system_settings_send_custom_event(instance, AppEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void system_settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SystemSettings* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void system_settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SystemSettings* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool system_settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SystemSettings* instance = context;

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

static SystemSettings* system_settings_alloc() {
    SystemSettings* instance = malloc(sizeof(SystemSettings));
    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(4, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(4, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(system_settings_scenes, COUNT_OF(system_settings_scenes), instance);

    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->power = furi_record_open(RECORD_POWER);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, system_settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_push_location(instance->back_nav_bar, "SYSTEM");
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
        system_settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        system_settings_event_queue_callback,
        instance);

    scene_manager_next_scene(instance->scene_manager, SceneIdMain);
    return instance;
}

static void system_settings_free(SystemSettings* instance) {
    furi_assert(instance);
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, system_settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        widget_free(instance->back_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_POWER);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t system_settings_entry(void* arg) {
    if(arg) {
        SettingsAppDescriptor* descriptor = arg;
        furi_string_set_str(descriptor->front_title, "System");
        furi_string_set_str(descriptor->back_title, "System");

        furi_string_set_str(descriptor->front_icon, IMG_PATH("system_settings_front_8x8.bin"));
        furi_string_set_str(descriptor->back_icon, IMG_PATH("system_settings_back_11x11.bin"));
        return 0;
    }

    SystemSettings* instance = system_settings_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, system_settings_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    system_settings_free(instance);

    return 0;
}

void system_settings_send_custom_event(SystemSettings* instance, uint32_t event) {
    furi_assert(instance);

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}
