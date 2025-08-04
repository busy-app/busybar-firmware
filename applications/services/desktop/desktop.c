#include "desktop.h"
#include "desktop_overlay.h"

#include <furi.h>

#include <input/input.h>

#include <loader/loader.h>
#include <gui/gui.h>

#define TAG "Desktop"

// Time to wait for the rotary switch steady state
#define SWITCH_DELAY_MS   (300)
// Maximum and initial counts for synchronisation primitives
#define INPUT_QUEUE_COUNT (8)
#define START_QUEUE_COUNT (3)
#define EXIT_SEMAPH_COUNT (1)
#define EXIT_SEMAPH_INIT  (1)

typedef struct {
    const char* name;
    const char* args;
} DesktopDefaultApp;

typedef struct {
    FuriString* name;
    FuriString* args;
} DesktopStartRequest;

struct Desktop {
    FuriEventLoop* event_loop;
    FuriSemaphore* exit_semaphore;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* start_queue;
    FuriEventLoopTimer* switch_timer;
    FuriEventLoopTimer* start_timer;
    FuriString* error_message;
    Loader* loader;
    DesktopOverlay* overlay;
    DesktopStartRequest* current_request;
    InputSwitchPosition switch_pos;
    bool pin_current_app;
};

static const DesktopDefaultApp desktop_default_apps[];

// Called by the Input service thread when the user interacts with the rotary switch/buttons
static void desktop_input_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const InputEvent* event = message;
    Desktop* instance = context;

    if(!instance->pin_current_app) {
        if(event->type == InputTypePress) {
            const InputKey key = event->key;
            // Only react to rotary switch events
            if(key >= InputKeyBusy && key < InputKeyMAX) {
                const InputSwitchPosition pos = key - InputKeyBusy;

                furi_check(
                    furi_message_queue_put(instance->input_queue, &pos, FuriWaitForever) ==
                    FuriStatusOk);
            }
        }
    }
}

// Called by the Loader service thread when an event has occurred
static void desktop_loader_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const LoaderEvent* event = message;
    Desktop* instance = context;

    const LoaderEventType event_type = event->type;
    // Only react to the ApplicationStopped events
    if(event_type == LoaderEventTypeApplicationStopped) {
        // Wait until the Desktop thread acks the previous app exit
        furi_check(
            furi_semaphore_acquire(instance->exit_semaphore, FuriWaitForever) == FuriStatusOk);
    }
}

// DesktopStartRequest is a non-trivial type, so it gets its own class
static DesktopStartRequest* desktop_start_request_alloc(void) {
    DesktopStartRequest* request = malloc(sizeof(DesktopStartRequest));

    request->name = furi_string_alloc();
    request->args = furi_string_alloc();

    return request;
}

static void desktop_start_request_free(DesktopStartRequest* request) {
    furi_string_free(request->name);
    furi_string_free(request->args);
    free(request);
}

static void desktop_start_request_set_name(DesktopStartRequest* request, const char* name) {
    furi_string_set(request->name, name);
}

static void desktop_start_request_set_args(DesktopStartRequest* request, const char* args) {
    if(args) {
        furi_string_set(request->args, args);
    } else {
        furi_string_reset(request->args);
    }
}

static const char* desktop_start_request_get_name(const DesktopStartRequest* request) {
    return furi_string_get_cstr(request->name);
}

static const char* desktop_start_request_get_args(const DesktopStartRequest* request) {
    return furi_string_empty(request->args) ? NULL : furi_string_get_cstr(request->args);
}

// Schedule an app to be started (can be called from any thread)
static bool desktop_enqueue_start_request(Desktop* instance, const char* name, const char* args) {
    DesktopStartRequest* request = desktop_start_request_alloc();
    desktop_start_request_set_name(request, name);
    desktop_start_request_set_args(request, args);

    // No waiting to avoid complex deadlock situations, excess requests are dropped
    const bool success = furi_message_queue_put(instance->start_queue, &request, 0) ==
                         FuriStatusOk;
    if(!success) {
        desktop_start_request_free(request);
    }

    return success;
}

// Called before unloading the current application, e.g. when the user interacted with the rotary switch
static void desktop_handle_switch_start(Desktop* instance) {
    FURI_LOG_D(TAG, "Switch interaction started");

    desktop_overlay_show(instance->overlay);
}

// Called on each new position of the rotary switch if steady state has not been reached
static void desktop_handle_switch_update(Desktop* instance) {
    UNUSED(instance);

    FURI_LOG_D(TAG, "Switch position updated");
}

// Called after the app has been started, due to rotary switch interaction or programmatically
static void desktop_handle_switch_finished(Desktop* instance) {
    FURI_LOG_D(TAG, "Switch interaction finished");

    desktop_overlay_hide(instance->overlay);
}

// Check if desktop_handle_switch_start() should be called
static bool desktop_should_handle_switch_start(Desktop* instance) {
    return !desktop_overlay_show_requested(instance->overlay);
}

// Called if the requested app failed to start (Shows error message via the Message app)
static void desktop_handle_error(Desktop* instance) {
    FURI_LOG_D(TAG, "Error starting app: %s", furi_string_get_cstr(instance->error_message));

    desktop_enqueue_start_request(
        instance, "message", (void*)furi_string_get_cstr(instance->error_message));
}

// Reset the pending app to a default one (w/ respect the the switch position)
static void desktop_prepare_default_app(Desktop* instance) {
    const DesktopDefaultApp* default_app = &desktop_default_apps[instance->switch_pos];
    desktop_start_request_set_name(instance->current_request, default_app->name);
    desktop_start_request_set_args(instance->current_request, default_app->args);
}

// Start the pending app immediately (the previous app MUST have exited at this point)
static void desktop_start_current_app(Desktop* instance) {
    furi_assert(!furi_string_empty(instance->current_request->name));

    const char* app_name = desktop_start_request_get_name(instance->current_request);
    const char* app_args = desktop_start_request_get_args(instance->current_request);

    if(app_args) {
        FURI_LOG_D(TAG, "Starting application '%s' with args '%s'", app_name, app_args);
    } else {
        FURI_LOG_D(TAG, "Starting application '%s' with no args", app_name);
    }

    if(loader_start(instance->loader, app_name, app_args, instance->error_message) ==
       LoaderStatusOk) {
        desktop_handle_switch_finished(instance);
    } else {
        desktop_handle_error(instance);
    }
}

// Start the pending app using two strategies depending on the loader state
static void desktop_do_replace_current_app(Desktop* instance) {
    const LoaderStatus status = loader_stop(instance->loader);

    if(status == LoaderStatusOk) {
        // App will be started asynchronously after
        // the currently running one will have stopped
    } else if(status == LoaderStatusErrorAppNotRunning) {
        // App will be started immediately
        desktop_start_current_app(instance);
        desktop_prepare_default_app(instance);
    } else if(status == LoaderStatusErrorInternal) {
        furi_crash("Update app to support signals");
    } else {
        furi_crash("Unexpected loader status");
    }
}

// Called in the Desktop thread when there are input events to process
static void desktop_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->input_queue == object);

    if(desktop_should_handle_switch_start(instance)) {
        desktop_handle_switch_start(instance);
    }

    while(furi_message_queue_get(instance->input_queue, &instance->switch_pos, 0) ==
          FuriStatusOk) {
        desktop_handle_switch_update(instance);
    }

    furi_event_loop_timer_start(instance->switch_timer, SWITCH_DELAY_MS);
}

// Called in the Desktop thread when the switch steady state has been reached
static void desktop_switch_timer_callback(void* context) {
    furi_assert(context);
    Desktop* instance = context;

    furi_assert(instance->switch_pos < InputSwitchPositionMAX);
    const DesktopDefaultApp* default_app = &desktop_default_apps[instance->switch_pos];

    desktop_enqueue_start_request(instance, default_app->name, default_app->args);
}

// Called in the Desktop thread when the pending app is ready to be started programmatically
static void desktop_start_timer_callback(void* context) {
    furi_assert(context);
    Desktop* instance = context;

    desktop_do_replace_current_app(instance);
}

// Called in the Desktop thread when one or more apps have been scheduled for start using desktop_enqueue_start_request()
static void desktop_app_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->start_queue == object);

    DesktopStartRequest* request;

    // Only process the last request in the queue
    while(furi_message_queue_get(instance->start_queue, &request, 0) == FuriStatusOk) {
        desktop_start_request_free(instance->current_request);
        instance->current_request = request;
    }
    // Determine whether the animation should be played
    if(desktop_should_handle_switch_start(instance)) {
        desktop_handle_switch_start(instance);
        furi_event_loop_timer_start(instance->start_timer, SWITCH_DELAY_MS);

    } else {
        desktop_do_replace_current_app(instance);
    }
}

// Called in the Desktop thread when the Loader service signaled that the current app has stopped
static void desktop_exit_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->exit_semaphore == object);
    // Determine whether the animation should be played
    if(desktop_should_handle_switch_start(instance)) {
        desktop_handle_switch_start(instance);
        furi_event_loop_timer_start(instance->start_timer, SWITCH_DELAY_MS);

    } else {
        desktop_do_replace_current_app(instance);
    }
    // Acknowledge the event for the Loader
    furi_semaphore_release(instance->exit_semaphore);
}

static Desktop* desktop_alloc(void) {
    Desktop* instance = malloc(sizeof(Desktop));

    instance->event_loop = furi_event_loop_alloc();
    instance->exit_semaphore = furi_semaphore_alloc(EXIT_SEMAPH_COUNT, EXIT_SEMAPH_INIT);
    instance->input_queue =
        furi_message_queue_alloc(INPUT_QUEUE_COUNT, sizeof(InputSwitchPosition));
    instance->start_queue =
        furi_message_queue_alloc(START_QUEUE_COUNT, sizeof(DesktopStartRequest*));
    instance->switch_timer = furi_event_loop_timer_alloc(
        instance->event_loop, desktop_switch_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->start_timer = furi_event_loop_timer_alloc(
        instance->event_loop, desktop_start_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->error_message = furi_string_alloc();
    instance->loader = furi_record_open(RECORD_LOADER);

    Gui* gui = furi_record_open(RECORD_GUI);
    instance->overlay = desktop_overlay_alloc(gui);

    Input* input = furi_record_open(RECORD_INPUT);
    instance->current_request = desktop_start_request_alloc();
    instance->switch_pos = input_get_absolute_state(input).switch_position;

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        desktop_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->start_queue,
        FuriEventLoopEventIn,
        desktop_app_queue_callback,
        instance);

    furi_event_loop_subscribe_semaphore(
        instance->event_loop,
        instance->exit_semaphore,
        FuriEventLoopEventOut,
        desktop_exit_semaphore_callback,
        instance);

    FuriPubSub* loader_events = loader_get_pubsub(instance->loader);
    furi_pubsub_subscribe(loader_events, desktop_loader_pubsub_callback, instance);

#if defined(SRV_INPUT)
    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_subscribe(input_events, desktop_input_pubsub_callback, instance);
#else
    UNUSED(desktop_input_pubsub_callback);
#endif

    desktop_prepare_default_app(instance);
    if(!loader_is_locked(instance->loader)) {
        desktop_start_current_app(instance);
    }

    furi_record_create(RECORD_DESKTOP, instance);
    return instance;
}

bool desktop_replace_current_app(Desktop* instance, const char* name, const char* args) {
    furi_check(instance);
    furi_check(name);

    return desktop_enqueue_start_request(instance, name, args);
}

void desktop_pin_current_app(Desktop* instance, bool pin) {
    furi_check(instance);

    instance->pin_current_app = pin;
}

int32_t desktop_srv(void* arg) {
    UNUSED(arg);

    Desktop* instance = desktop_alloc();

    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const DesktopDefaultApp desktop_default_apps[] = {
    [InputSwitchPositionBusy] = {"busy", NULL},
    [InputSwitchPositionStatus] = {"dummy", "Status"},
    [InputSwitchPositionOff] = {"dummy", "Off"},
    [InputSwitchPositionApps] = {"apps_menu", NULL},
    [InputSwitchPositionSettings] = {"dummy", "Settings"},
};
