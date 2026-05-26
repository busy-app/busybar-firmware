#include "power_on_i.h"

#include <storage/storage.h>

#define POWER_ON_DONE_PATH APP_DATA_PATH("done.txt")

bool power_on_is_done_flag_present(PowerOnApp* instance) {
    return storage_file_exists(instance->storage, POWER_ON_DONE_PATH);
}

void power_on_done_flag_create(PowerOnApp* instance) {
    File* file = storage_file_alloc(instance->storage);

    if(!storage_file_open(file, POWER_ON_DONE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_W(TAG, "Failed to create file");
    }

    storage_file_close(file);
    storage_file_free(file);
}

static bool power_on_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    PowerOnApp* instance = context;

    switch(signal) {
    case FuriSignalExit:
        // Desktop has received the initial switch state and wants to close us
        power_on_send_custom_event(instance, PowerOnAppEventStarted);
        return true;

    default:
        return false;
    }
}

static void power_on_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    PowerOnApp* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void power_on_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    PowerOnApp* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool power_on_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    PowerOnApp* instance = context;

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

static PowerOnApp* power_on_alloc() {
    PowerOnApp* instance = malloc(sizeof(PowerOnApp));
    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(4, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(power_on_scenes, COUNT_OF(power_on_scenes), instance);

    instance->gui = furi_record_open(RECORD_GUI);
    instance->power = furi_record_open(RECORD_POWER);
    instance->storage = furi_record_open(RECORD_STORAGE);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, power_on_gui_input_callback, instance);

        instance->front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        power_on_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        power_on_event_queue_callback,
        instance);

    scene_manager_next_scene(instance->scene_manager, SceneIdStarting);

    return instance;
}

static void power_on_free(PowerOnApp* instance) {
    furi_assert(instance);
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, power_on_gui_input_callback);
    });

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t power_on_app(void* arg) {
    UNUSED(arg);

    PowerOnApp* instance = power_on_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, power_on_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    power_on_free(instance);

    return 0;
}

void power_on_send_custom_event(PowerOnApp* instance, uint32_t event) {
    furi_assert(instance);

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

bool power_on_handle_generic_input(PowerOnApp* instance, const InputEvent* event) {
    furi_assert(instance);
    furi_assert(event);

    bool consumed = false;
    PowerOnAppEvent app_event;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyOk:
        case InputKeyStart:
            app_event = PowerOnAppEventUserInteracted;
            consumed = true;
            break;
        default:
            break;
        }
    }
    if(event->type == InputTypePress) {
        switch(event->key) {
        case InputKeyBusy:
        case InputKeyCustom:
        case InputKeyOff:
        case InputKeyApps:
        case InputKeySettings:
            app_event = PowerOnAppEventUserInteracted;
            consumed = true;
            break;
        default:
            break;
        }
    }

    if(consumed) {
        power_on_send_custom_event(instance, app_event);
    }

    return consumed;
}
