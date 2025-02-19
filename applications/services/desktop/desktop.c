#include <furi.h>

#include <input/input.h>
#include <input/input_common.h>

#include <loader/loader.h>
#include <gui_lvgl/gui_lvgl.h>

#define TAG "Desktop"

#define DEBOUNCE_DELAY_MS (300)

typedef struct {
    FuriEventLoop* event_loop;
    FuriSemaphore* exit_semaphore;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* debounce_timer;
    Loader* loader;
    InputSwitchPosition prev_pos;
    InputSwitchPosition current_pos;
} Desktop;

typedef struct {
    const char* name;
    const char* args;
} DesktopAppDesc;

static const DesktopAppDesc desktop_apps[];

static void desktop_input_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const InputEvent* event = message;
    Desktop* instance = context;

    if(event->type == InputTypePress) {
        const InputKey key = event->key;

        if(key >= InputKeyBusy && key < InputKeyMAX) {
            const InputSwitchPosition pos = key - InputKeyBusy;

            furi_check(
                furi_message_queue_put(instance->input_queue, &pos, FuriWaitForever) ==
                FuriStatusOk);
        }
    }
}

static void desktop_loader_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const LoaderEvent* event = message;
    Desktop* instance = context;

    const LoaderEventType event_type = event->type;

    if(event_type == LoaderEventTypeApplicationStopped) {
        furi_check(
            furi_semaphore_acquire(instance->exit_semaphore, FuriWaitForever) == FuriStatusOk);
    }
}

static bool desktop_start_current_app(Desktop* instance) {
    furi_assert(instance->current_pos < InputSwitchPositionMAX);

    FURI_LOG_I(TAG, "Starting application with id: %d", instance->current_pos);

    FuriString* error_message = furi_string_alloc();

    const DesktopAppDesc* desc = &desktop_apps[instance->current_pos];
    const LoaderStatus status =
        loader_start(instance->loader, desc->name, desc->args, error_message);

    if(status != LoaderStatusOk) {
        FURI_LOG_E(
            TAG, "Failed to load app %s: %s", desc->name, furi_string_get_cstr(error_message));
        return false;
    } else {
        return true;
    }
}

static void desktop_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->input_queue == object);
    furi_check(furi_message_queue_get_count(instance->input_queue) > 0);

    if(!furi_event_loop_timer_is_running(instance->debounce_timer)) {
        instance->prev_pos = instance->current_pos;
        // TODO: Hide current app, play animation on the top layer
        FURI_LOG_D(TAG, "Switch interaction start");
    }

    while(furi_message_queue_get(instance->input_queue, &instance->current_pos, 0) ==
          FuriStatusOk) {
        // TODO: Play some animation in response to user interactions
    }

    furi_event_loop_timer_start(instance->debounce_timer, DEBOUNCE_DELAY_MS);
}

static void desktop_exit_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->exit_semaphore == object);

    if(!desktop_start_current_app(instance)) {
        FURI_LOG_E(TAG, "Failed to start application");
    }

    furi_semaphore_release(instance->exit_semaphore);
}

static void desktop_debounce_timer_callback(void* context) {
    furi_assert(context);
    Desktop* instance = context;

    const LoaderStatus status = loader_stop(instance->loader);

    if(status == LoaderStatusErrorAppNotRunning) {
        if(!desktop_start_current_app(instance)) {
            FURI_LOG_E(TAG, "Failed to start application");
        }

    } else if(status == LoaderStatusErrorInternal) {
        furi_crash("App doesn't support signals, fix it");
    } else {
        // Load App asynchronously
        // Do not hide the animation yet
        return;
    }

    // TODO: Hide the animation overlay
}

static Desktop* desktop_alloc(void) {
    Desktop* instance = malloc(sizeof(Desktop));

    instance->event_loop = furi_event_loop_alloc();
    instance->exit_semaphore = furi_semaphore_alloc(1, 1);
    instance->input_queue = furi_message_queue_alloc(16, sizeof(InputSwitchPosition));
    instance->debounce_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        desktop_debounce_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->loader = furi_record_open(RECORD_LOADER);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        desktop_input_queue_callback,
        instance);

    furi_event_loop_subscribe_semaphore(
        instance->event_loop,
        instance->exit_semaphore,
        FuriEventLoopEventOut,
        desktop_exit_semaphore_callback,
        instance);

    FuriPubSub* loader_events = loader_get_pubsub(instance->loader);
    furi_pubsub_subscribe(loader_events, desktop_loader_pubsub_callback, instance);

    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_subscribe(input_events, desktop_input_pubsub_callback, instance);

    return instance;
}

int32_t desktop_srv(void* arg) {
    UNUSED(arg);

    Desktop* instance = desktop_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const DesktopAppDesc desktop_apps[] = {
    [InputSwitchPositionBusy] = {"busy", NULL},
    [InputSwitchPositionStatus] = {"dummy", "Status"},
    [InputSwitchPositionOff] = {"dummy", "Off"},
    [InputSwitchPositionApps] = {"dummy", "Apps"},
    [InputSwitchPositionSettings] = {"dummy", "Settings"},
};
