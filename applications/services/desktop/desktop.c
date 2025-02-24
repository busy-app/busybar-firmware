#include "desktop.h"
#include "desktop_overlay.h"

#include <furi.h>

#include <input/input.h>
#include <input/input_common.h>

#include <loader/loader.h>
#include <gui_lvgl/gui_lvgl.h>

#define TAG "Desktop"

#define DEBOUNCE_DELAY_MS (300)

typedef struct {
    const char* name;
    const char* args;
} DesktopAppDesc;

typedef struct {
    FuriString* name;
    const char* arg;
} DesktopStartAppDesc;

struct Desktop {
    FuriEventLoop* event_loop;
    FuriSemaphore* exit_semaphore;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* start_queue;
    FuriEventLoopTimer* debounce_timer;
    FuriString* error_message;
    Loader* loader;
    DesktopOverlay* overlay;
    DesktopStartAppDesc start_app;
    InputSwitchPosition switch_pos;
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

static bool desktop_should_handle_switch_start(Desktop* instance) {
    return !desktop_overlay_show_requested(instance->overlay);
}

static bool desktop_enqueue_app_start(Desktop* instance, const char* name, const char* arg) {
    const DesktopStartAppDesc desc = {
        .name = furi_string_alloc_set(name),
        .arg = arg,
    };

    const bool success = furi_message_queue_put(instance->start_queue, &desc, 0) == FuriStatusOk;

    if(!success) {
        furi_string_free(desc.name);
    }

    return success;
}

static void desktop_prepare_default_app(Desktop* instance) {
    const DesktopAppDesc* default_app = &desktop_apps[instance->switch_pos];
    furi_string_set(instance->start_app.name, default_app->name);
    instance->start_app.arg = default_app->args;
}

static void desktop_start_current_app(Desktop* instance) {
    furi_assert(!furi_string_empty(instance->start_app.name));

    const char* name = furi_string_get_cstr(instance->start_app.name);
    const char* arg = instance->start_app.arg;

    FURI_LOG_I(TAG, "Starting application '%s' with arg 0x%p", name, arg);

    if(loader_start(instance->loader, name, arg, instance->error_message) == LoaderStatusOk) {
        desktop_handle_switch_finished(instance);
        FURI_LOG_I(TAG, "App %s started", name);

    } else {
        // TODO: Show error screen
        FURI_LOG_E(
            TAG, "Failed to load app %s: %s", name, furi_string_get_cstr(instance->error_message));
    }

    desktop_prepare_default_app(instance);
}

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

    furi_event_loop_timer_start(instance->debounce_timer, DEBOUNCE_DELAY_MS);
}

static void desktop_debounce_timer_callback(void* context) {
    furi_assert(context);
    Desktop* instance = context;

    furi_assert(instance->switch_pos < InputSwitchPositionMAX);
    const DesktopAppDesc* default_app = &desktop_apps[instance->switch_pos];

    desktop_enqueue_app_start(instance, default_app->name, default_app->args);
}

static void desktop_app_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->start_queue == object);

    DesktopStartAppDesc desc = {0};

    while(furi_message_queue_get(instance->start_queue, &desc, 0) == FuriStatusOk) {
        furi_string_move(instance->start_app.name, desc.name);
        instance->start_app.arg = desc.arg;
    }

    if(desktop_should_handle_switch_start(instance)) {
        desktop_handle_switch_start(instance);
        furi_delay_ms(DEBOUNCE_DELAY_MS);
    }

    const LoaderStatus status = loader_stop(instance->loader);

    if(status == LoaderStatusOk) {
        /* App will be started asynchronously after
         * the currently running one will have stopped */
    } else if(status == LoaderStatusErrorAppNotRunning) {
        desktop_start_current_app(instance);
    } else if(status == LoaderStatusErrorInternal) {
        furi_crash("Update app to support signals");
    } else {
        furi_crash("Unexpected loader status");
    }
}

static void desktop_exit_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Desktop* instance = context;
    furi_assert(instance->exit_semaphore == object);

    if(desktop_should_handle_switch_start(instance)) {
        desktop_handle_switch_start(instance);
        furi_delay_ms(DEBOUNCE_DELAY_MS);
    }

    desktop_start_current_app(instance);
    furi_semaphore_release(instance->exit_semaphore);
}

static Desktop* desktop_alloc(void) {
    Desktop* instance = malloc(sizeof(Desktop));

    instance->event_loop = furi_event_loop_alloc();
    instance->exit_semaphore = furi_semaphore_alloc(1, 1);
    instance->input_queue = furi_message_queue_alloc(16, sizeof(InputSwitchPosition));
    instance->start_queue = furi_message_queue_alloc(3, sizeof(DesktopStartAppDesc));
    instance->debounce_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        desktop_debounce_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->error_message = furi_string_alloc();
    instance->loader = furi_record_open(RECORD_LOADER);

    GuiLvgl* gui = furi_record_open(RECORD_GUI_LVGL);
    instance->overlay = desktop_overlay_alloc(gui);
    instance->start_app.name = furi_string_alloc();

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

    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_subscribe(input_events, desktop_input_pubsub_callback, instance);

    furi_record_create(RECORD_DESKTOP, instance);
    return instance;
}

bool desktop_replace_current_app(Desktop* instance, const char* name, void* arg) {
    furi_check(instance);
    furi_check(name);

    return desktop_enqueue_app_start(instance, name, arg);
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
