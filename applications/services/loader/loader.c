#include "loader_i.h"

#include <furi_hal.h>

#include <applications.h>
#include <storage/storage.h>

#define TAG                       "Loader"
#define LOADER_MAGIC_THREAD_VALUE 0xDEADBEEF

static const LoaderMessageHandler loader_handlers[];

// ==========
// Public API
// ==========

static void loader_synchronous_request(Loader* loader, LoaderMessage* message) {
    furi_assert(loader);
    furi_assert(message);

    message->caller = furi_thread_get_current_id();
    message->api_lock = api_lock_alloc_locked();

    furi_check(furi_message_queue_put(loader->queue, message, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(message->api_lock);
}

LoaderStatus
    loader_start(Loader* loader, const char* name, const char* args, FuriString* error_message) {
    furi_check(loader);
    furi_check(name);

    LoaderMessageLoaderStatusResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeStart,
        .start.name = name,
        .start.args = args,
        .start.error_message = error_message,
        .status_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value;
}

LoaderStatus loader_stop(Loader* loader) {
    furi_check(loader);

    LoaderMessageLoaderStatusResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeStop,
        .status_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value;
}

bool loader_lock(Loader* loader) {
    furi_check(loader);

    LoaderMessageBoolResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeLock,
        .bool_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value;
}

void loader_unlock(Loader* loader) {
    furi_check(loader);

    LoaderMessage message = {
        .type = LoaderMessageTypeUnlock,
    };

    loader_synchronous_request(loader, &message);
}

bool loader_is_locked(Loader* loader) {
    furi_check(loader);

    LoaderMessageBoolResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeIsLocked,
        .bool_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value;
}

FuriPubSub* loader_get_pubsub(Loader* loader) {
    furi_check(loader);
    // it's safe to return pubsub without locking
    // because it's never freed and loader is never exited
    // also the loader instance cannot be obtained until the pubsub is created
    return loader->pubsub;
}

bool loader_get_application_name(Loader* loader, FuriString* name) {
    furi_check(loader);
    furi_check(name);

    LoaderMessageBoolResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeGetApplicationName,
        .application_name = name,
        .bool_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value;
}

bool loader_send_signal(Loader* loader, uint32_t signal, void* arg) {
    furi_check(loader);

    LoaderMessageBoolResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeSendCustomSignal,
        .custom_signal = {.signal = signal, .arg = arg},
        .bool_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value;
}

bool loader_set_priority(Loader* loader, size_t priority) {
    furi_check(loader);

    LoaderMessageBoolResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeSetPriority,
        .priority = &priority,
        .bool_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value;
}

size_t loader_get_priority(Loader* loader) {
    furi_check(loader);

    LoaderMessageBoolResult result;
    size_t priority = 0;

    LoaderMessage message = {
        .type = LoaderMessageTypeGetPriority,
        .priority = &priority,
        .bool_value = &result,
    };

    loader_synchronous_request(loader, &message);
    return result.value ? priority : 0;
}

// ==============
// Implementation
// ==============

static bool loader_is_locked_internal(const Loader* loader) {
    return loader->app.thread != NULL;
}

static void
    loader_thread_state_callback(FuriThread* thread, FuriThreadState thread_state, void* context) {
    UNUSED(thread);
    furi_assert(context);

    if(thread_state == FuriThreadStateStopped) {
        Loader* loader = context;

        LoaderMessage message = {
            .type = LoaderMessageTypeAppClosed,
        };

        loader_synchronous_request(loader, &message);
    }
}

static FlipperInternalApplication const* loader_find_application_by_name_in_list(
    const char* name,
    const FlipperInternalApplication* list,
    const uint32_t n_apps) {
    for(size_t i = 0; i < n_apps; i++) {
        if((strcmp(name, list[i].name) == 0) || (strcmp(name, list[i].appid) == 0)) {
            return &list[i];
        }
    }
    return NULL;
}

static const FlipperInternalApplication* loader_find_application_by_name(const char* name) {
    typedef struct {
        const FlipperInternalApplication* list;
        const uint32_t count;
    } FlipperInternalApplicationList;

    const FlipperInternalApplicationList lists[] = {
        {FLIPPER_SETTINGS_APPS, FLIPPER_SETTINGS_APPS_COUNT},
        {FLIPPER_SYSTEM_APPS, FLIPPER_SYSTEM_APPS_COUNT},
        {FLIPPER_DEBUG_APPS, FLIPPER_DEBUG_APPS_COUNT},
        {FLIPPER_APPS, FLIPPER_APPS_COUNT},
    };

    for(size_t i = 0; i < COUNT_OF(lists); i++) {
        const FlipperInternalApplication* application =
            loader_find_application_by_name_in_list(name, lists[i].list, lists[i].count);
        if(application) {
            return application;
        }
    }

    return NULL;
}

static void loader_start_app_thread(Loader* loader, FlipperInternalApplicationFlag flags) {
    // setup heap trace
    FuriHalMemoryHeapTrackMode mode = furi_hal_memory_get_heap_track_mode();
    if(mode > FuriHalMemoryHeapTrackModeNone) {
        furi_thread_enable_heap_trace(loader->app.thread);
    } else {
        furi_thread_disable_heap_trace(loader->app.thread);
    }

    // setup insomnia
    if(!(flags & FlipperInternalApplicationFlagInsomniaSafe)) {
        furi_hal_power_insomnia_enter();
        loader->app.insomniac = true;
    } else {
        loader->app.insomniac = false;
    }

    // setup thread state callbacks
    furi_thread_set_state_context(loader->app.thread, loader);
    furi_thread_set_state_callback(loader->app.thread, loader_thread_state_callback);

    // start app thread
    furi_thread_start(loader->app.thread);
}

static void loader_start_internal_app(
    Loader* loader,
    const FlipperInternalApplication* app,
    const char* args) {
    FURI_LOG_I(TAG, "Starting %s", app->name);
    LoaderEvent event = {
        .type = LoaderEventTypeApplicationBeforeLoad,
        .appid = app->appid,
    };
    furi_pubsub_publish(loader->pubsub, &event);

    // store args
    furi_assert(loader->app.args == NULL);
    if(args && strlen(args) > 0) {
        loader->app.args = strdup(args);
    }

    loader->app.appid = app->appid;
    loader->app.thread =
        furi_thread_alloc_ex(app->name, app->stack_size, app->app, loader->app.args);
    loader->app.priority = LOADER_DEFAULT_APP_PRIORITY;
    furi_thread_set_appid(loader->app.thread, app->appid);

    LoaderEvent prio_event = {
        .type = LoaderEventTypePriorityChanged,
        .priority = loader->app.priority,
    };
    furi_pubsub_publish(loader->pubsub, &prio_event);

    loader_start_app_thread(loader, app->flags);
}

static void loader_log_status_error(
    LoaderStatus status,
    FuriString* error_message,
    const char* format,
    va_list args) {
    if(error_message) {
        furi_string_vprintf(error_message, format, args);
        FURI_LOG_E(TAG, "Status [%d]: %s", status, furi_string_get_cstr(error_message));
    } else {
        FURI_LOG_E(TAG, "Status [%d]", status);
    }
}

static LoaderStatus loader_make_status_error(
    LoaderStatus status,
    FuriString* error_message,
    const char* format,
    ...) {
    va_list args;
    va_start(args, format);
    loader_log_status_error(status, error_message, format, args);
    va_end(args);
    return status;
}

static LoaderStatus loader_make_success_status(FuriString* error_message) {
    if(error_message) {
        furi_string_set(error_message, "App started");
    }

    return LoaderStatusOk;
}

static LoaderStatus loader_start_internal(
    Loader* loader,
    const char* name,
    const char* args,
    FuriString* error_message) {
    LoaderStatus status;

    do {
        // check lock
        if(loader_is_locked_internal(loader)) {
            const char* current_thread_name =
                furi_thread_get_name(furi_thread_get_id(loader->app.thread));
            status = loader_make_status_error(
                LoaderStatusErrorAppStarted,
                error_message,
                "Loader is locked, please close the \"%s\" first",
                current_thread_name);
            break;
        }

        // check internal apps
        {
            const FlipperInternalApplication* app = loader_find_application_by_name(name);
            if(app) {
                loader_start_internal_app(loader, app, args);
                status = loader_make_success_status(error_message);
                break;
            }
        }

        status = loader_make_status_error(
            LoaderStatusErrorUnknownApp, error_message, "App not found \"%s\"", name);

    } while(false);

    return status;
}

// Message handlers

static void loader_start_handler(Loader* loader, const LoaderMessage* message) {
    const LoaderMessageStartByName* start = &message->start;
    message->status_value->value =
        loader_start_internal(loader, start->name, start->args, start->error_message);
}

static void loader_stop_handler(Loader* loader, const LoaderMessage* message) {
    LoaderStatus status;

    do {
        if(!loader_is_locked_internal(loader)) {
            status = LoaderStatusErrorAppNotRunning;
            break;
        }

        if(!furi_thread_signal(loader->app.thread, FuriSignalExit, NULL)) {
            status = LoaderStatusErrorNoSignalHandler;
            break;
        }

        status = LoaderStatusOk;

    } while(false);

    message->status_value->value = status;
}

static void loader_app_closed_handler(Loader* loader, const LoaderMessage* message) {
    UNUSED(message);
    furi_assert(loader->app.thread);

    furi_thread_join(loader->app.thread);
    FURI_LOG_I(TAG, "App returned: %li", furi_thread_get_return_code(loader->app.thread));

    if(loader->app.args) {
        free(loader->app.args);
        loader->app.args = NULL;
    }

    if(loader->app.insomniac) {
        furi_hal_power_insomnia_exit();
    }

    furi_thread_free(loader->app.thread);
    loader->app.thread = NULL;
    loader->app.priority = 0;

    FURI_LOG_I(TAG, "Application stopped. Free heap: %zu", memmgr_get_free_heap());

    LoaderEvent prio_event = {
        .type = LoaderEventTypePriorityChanged,
        .priority = 0,
    };
    furi_pubsub_publish(loader->pubsub, &prio_event);

    LoaderEvent event = {
        .type = LoaderEventTypeApplicationStopped,
        .appid = loader->app.appid,
    };

    furi_pubsub_publish(loader->pubsub, &event);
    loader->app.appid = NULL;
}

static void loader_lock_handler(Loader* loader, const LoaderMessage* message) {
    bool success = false;

    if(!loader_is_locked_internal(loader)) {
        loader->app.thread = (FuriThread*)LOADER_MAGIC_THREAD_VALUE;
        success = true;
    }

    message->bool_value->value = success;
}

static void loader_unlock_handler(Loader* loader, const LoaderMessage* message) {
    UNUSED(message);

    furi_check(loader->app.thread == (FuriThread*)LOADER_MAGIC_THREAD_VALUE);
    loader->app.thread = NULL;
}

static void loader_is_locked_handler(Loader* loader, const LoaderMessage* message) {
    message->bool_value->value = loader_is_locked_internal(loader);
}

static bool loader_is_application_running(Loader* loader) {
    FuriThread* app_thread = loader->app.thread;
    return app_thread && (app_thread != (FuriThread*)LOADER_MAGIC_THREAD_VALUE);
}

static void loader_do_get_application_name(Loader* loader, const LoaderMessage* message) {
    message->bool_value->value = false;
    if(loader_is_application_running(loader)) {
        furi_string_set(
            message->application_name,
            furi_thread_get_name(furi_thread_get_id(loader->app.thread)));
        message->bool_value->value = true;
    }
}

static void loader_do_send_custom_signal(Loader* loader, const LoaderMessage* message) {
    message->bool_value->value = false;

    if(loader_is_application_running(loader)) {
        message->bool_value->value = furi_thread_signal(
            loader->app.thread, message->custom_signal.signal, message->custom_signal.arg);
    }
}

static void loader_do_set_priority(Loader* loader, const LoaderMessage* message) {
    message->bool_value->value = false;

    if(!loader_is_application_running(loader)) return;

    const char* running_app = furi_thread_get_appid(furi_thread_get_id(loader->app.thread));
    const char* caller_app = furi_thread_get_appid(message->caller);
    if(strcmp(running_app, caller_app) != 0) return;

    message->bool_value->value = true;
    furi_assert(message->priority);
    loader->app.priority = *message->priority;

    LoaderEvent event = {
        .type = LoaderEventTypePriorityChanged,
        .priority = loader->app.priority,
    };
    furi_pubsub_publish(loader->pubsub, &event);
}

static void loader_do_get_priority(Loader* loader, const LoaderMessage* message) {
    message->bool_value->value = false;

    if(!loader_is_application_running(loader)) return;

    message->bool_value->value = true;
    furi_assert(message->priority);
    *message->priority = loader->app.priority;
}

static void loader_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Loader* loader = context;
    furi_assert(object == loader->queue);

    LoaderMessage message;
    furi_check(furi_message_queue_get(loader->queue, &message, FuriWaitForever) == FuriStatusOk);
    furi_assert(message.type < LoaderMessageTypeMax);

    loader_handlers[message.type](loader, &message);

    furi_assert(message.api_lock);
    api_lock_unlock(message.api_lock);
}

static Loader* loader_alloc(void) {
    Loader* loader = malloc(sizeof(Loader));

    loader->event_loop = furi_event_loop_alloc();
    loader->queue = furi_message_queue_alloc(1, sizeof(LoaderMessage));
    loader->pubsub = furi_pubsub_alloc();

    furi_event_loop_subscribe_message_queue(
        loader->event_loop,
        loader->queue,
        FuriEventLoopEventIn,
        loader_message_queue_callback,
        loader);

    furi_record_create(RECORD_LOADER, loader);

    return loader;
}

static void loader_do_autorun(Loader* loader) {
    UNUSED(loader);

    if(FLIPPER_AUTORUN_APP_NAME && strlen(FLIPPER_AUTORUN_APP_NAME)) {
        FURI_LOG_I(TAG, "Starting autorun app: %s", FLIPPER_AUTORUN_APP_NAME);

        loader_start_internal(loader, FLIPPER_AUTORUN_APP_NAME, NULL, NULL);
    }
}

// app

int32_t loader_srv(void* p) {
    UNUSED(p);

    Loader* loader = loader_alloc();

    loader_do_autorun(loader);

    furi_event_loop_run(loader->event_loop);

    return 0;
}

static const LoaderMessageHandler loader_handlers[LoaderMessageTypeMax] = {
    [LoaderMessageTypeStart] = loader_start_handler,
    [LoaderMessageTypeStop] = loader_stop_handler,
    [LoaderMessageTypeAppClosed] = loader_app_closed_handler,
    [LoaderMessageTypeLock] = loader_lock_handler,
    [LoaderMessageTypeUnlock] = loader_unlock_handler,
    [LoaderMessageTypeIsLocked] = loader_is_locked_handler,
    [LoaderMessageTypeGetApplicationName] = loader_do_get_application_name,
    [LoaderMessageTypeSendCustomSignal] = loader_do_send_custom_signal,
    [LoaderMessageTypeSetPriority] = loader_do_set_priority,
    [LoaderMessageTypeGetPriority] = loader_do_get_priority,
};
