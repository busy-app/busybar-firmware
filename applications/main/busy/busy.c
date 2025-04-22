#include "busy.h"

static void busy_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    furi_assert(instance->input_queue == object);

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                if(!scene_manager_handle_back_event(instance->scene_manager)) {
                    furi_event_loop_stop(instance->event_loop);
                    break;
                }
            }
        }
    }
}

static void busy_event_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    furi_assert(instance->event_queue == object);

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool busy_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    BusyApp* instance = context;

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

static BusyApp* busy_alloc(void) {
    BusyApp* instance = malloc(sizeof(BusyApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager = scene_manager_alloc(busy_scenes, BusyAppSceneIdMax, instance);
    instance->busy_timer = busy_timer_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, busy_gui_input_callback, instance);

        Widget* root;

        root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_window = widget_alloc(root);

        root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_window = widget_alloc(root);
        // TODO: Return a root widget of appropriate size to accomodate the status bar
        widget_set_size(instance->back_window, widget_get_width(root) - 12, widget_get_height(root));
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        busy_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        busy_event_queue_callback,
        instance);

    scene_manager_switch_to_scene(instance->scene_manager, BusyAppSceneIdStart);

    return instance;
}

static void busy_free(BusyApp* instance) {
    busy_timer_free(instance->busy_timer);
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, busy_gui_input_callback);

        widget_free(instance->front_window);
        widget_free(instance->back_window);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t busy_app(void* arg) {
    UNUSED(arg);

    BusyApp* instance = busy_alloc();
    furi_event_loop_run(instance->event_loop);
    busy_free(instance);

    return 0;
}

void busy_send_custom_event(BusyApp* instance, uint32_t custom_event) {
    furi_assert(instance);
    furi_check(
        furi_message_queue_put(instance->event_queue, &custom_event, FuriWaitForever) ==
        FuriStatusOk);
}
