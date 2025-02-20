#include "desktop.h"
#include "desktop_overlay.h"

#include <furi.h>
#include <api_lock.h>

#include <input/input.h>
#include <input/input_common.h>

#include <loader/loader.h>
#include <gui_lvgl/gui_lvgl.h>

#define TAG "Desktop"

#define DEBOUNCE_DELAY_MS (300)

typedef enum {
    DesktopStateIdle,
    DesktopStateTransition,
    DesktopStateAppStopping,
    DesktopStateAppRunning,
    DesktopStateError,
} DesktopState;

typedef struct {
    const char* name;
    const char* args;
} DesktopAppDesc;

typedef struct {
    DesktopAppDesc desc;
    FuriApiLock lock;
    bool* result;
} DesktopMessage;

struct Desktop {
    FuriEventLoop* event_loop;
    FuriSemaphore* exit_semaphore;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* message_queue;
    FuriEventLoopTimer* debounce_timer;
    FuriString* error_message;
    Loader* loader;
    const DesktopAppDesc* app_desc;
    DesktopOverlay* overlay;
    DesktopState state;
    InputSwitchPosition prev_pos;
    InputSwitchPosition current_pos;
};

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
    furi_assert(instance->app_desc);

    const char* app_name = instance->app_desc->name;
    const char* app_args = instance->app_desc->args;

    FURI_LOG_I(
        TAG, "Starting application '%s' with args '%s'", app_name, app_args ? app_args : "NULL");

    const LoaderStatus status =
        loader_start(instance->loader, app_name, app_args, instance->error_message);

    const bool success = (status == LoaderStatusOk);

    if(success) {
        FURI_LOG_I(TAG, "App %s started", app_name);
    } else {
        FURI_LOG_E(
            TAG,
            "Failed to load app %s: %s",
            app_name,
            furi_string_get_cstr(instance->error_message));
    }

    return success;
}

static void desktop_handle_switch_start(Desktop* instance) {
    FURI_LOG_D(TAG, "Switch interaction started");

    desktop_overlay_show(instance->overlay);
}

static void desktop_handle_switch_update(Desktop* instance) {
    UNUSED(instance);

    FURI_LOG_D(TAG, "Switch position updated");
}

static void desktop_handle_switch_finished(Desktop* instance) {
    FURI_LOG_D(TAG, "Switch interaction finished");

    desktop_overlay_hide(instance->overlay);
}

static void desktop_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->input_queue == object);

    if(instance->state == DesktopStateAppStopping) {
        // Defer the input until the desktop becomes ready
        return;
    }

    if(instance->state != DesktopStateTransition) {
        desktop_handle_switch_start(instance);

        instance->state = DesktopStateTransition;
        instance->prev_pos = instance->current_pos;
    }

    while(furi_message_queue_get(instance->input_queue, &instance->current_pos, 0) ==
          FuriStatusOk) {
        desktop_handle_switch_update(instance);
    }

    furi_event_loop_timer_start(instance->debounce_timer, DEBOUNCE_DELAY_MS);
}

static void desktop_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->message_queue == object);

    if(instance->state == DesktopStateTransition || instance->state == DesktopStateAppStopping) {
        // Defer the request until the desktop becomes ready
        return;
    }

    DesktopMessage msg;
    furi_check(furi_message_queue_get(instance->message_queue, &msg, 0) == FuriStatusOk);

    // TODO: Actually launch the app
    FURI_LOG_D(TAG, "Launching app: %s", msg.desc.name);
    instance->state = DesktopStateAppRunning;

    *msg.result = false;
    api_lock_unlock(msg.lock);
}

static void desktop_exit_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->exit_semaphore == object);
    furi_assert(instance->state == DesktopStateAppStopping);

    if(!desktop_start_current_app(instance)) {
        // TODO: Show error message on screen
        instance->state = DesktopStateError;
    } else {
        instance->state = DesktopStateAppRunning;
    }

    desktop_handle_switch_finished(instance);

    furi_check(furi_semaphore_release(instance->exit_semaphore) == FuriStatusOk);
}

static void desktop_debounce_timer_callback(void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->state == DesktopStateTransition);

    furi_assert(instance->current_pos < InputSwitchPositionMAX);
    instance->app_desc = &desktop_apps[instance->current_pos];

    const LoaderStatus status = loader_stop(instance->loader);

    if(status == LoaderStatusOk) {
        /* App will be started asynchronously after
         * the currently running one will have stopped */
        instance->state = DesktopStateAppStopping;

    } else if(status == LoaderStatusErrorAppNotRunning) {
        if(!desktop_start_current_app(instance)) {
            // TODO: Show error message on screen
            instance->state = DesktopStateError;
        } else {
            instance->state = DesktopStateAppRunning;
        }

        desktop_handle_switch_finished(instance);

    } else if(status == LoaderStatusErrorInternal) {
        furi_crash("Update app to support signals");
    } else {
        furi_crash("Unexpected loader status");
    }
}

static Desktop* desktop_alloc(void) {
    Desktop* instance = malloc(sizeof(Desktop));

    instance->event_loop = furi_event_loop_alloc();
    instance->exit_semaphore = furi_semaphore_alloc(1, 1);
    instance->input_queue = furi_message_queue_alloc(16, sizeof(InputSwitchPosition));
    instance->message_queue = furi_message_queue_alloc(1, sizeof(DesktopMessage));
    instance->debounce_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        desktop_debounce_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->error_message = furi_string_alloc();
    instance->loader = furi_record_open(RECORD_LOADER);

    GuiLvgl* gui = furi_record_open(RECORD_GUI_LVGL);
    instance->overlay = desktop_overlay_alloc(gui);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        desktop_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        desktop_message_queue_callback,
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

    furi_record_create(RECORD_DESKTOP, instance);
    return instance;
}

bool desktop_replace_current_app(Desktop* instance, const char* name, const void* arg) {
    furi_check(instance);
    furi_check(name);

    bool result;

    const DesktopMessage msg = {
        .desc =
            {
                .name = name,
                .args = arg,
            },
        .lock = api_lock_alloc_locked(),
        .result = &result,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
    return result;
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
    [InputSwitchPositionApps] = {"apps_menu", NULL},
    [InputSwitchPositionSettings] = {"dummy", "Settings"},
};
