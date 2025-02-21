#include "loader_i.h"

#include <furi_hal.h>

#include <applications.h>
#include <storage/storage.h>

#define TAG                       "Loader"
#define LOADER_MAGIC_THREAD_VALUE 0xDEADBEEF

// internal

void furi_hal_power_insomnia_enter(void) {
}

void furi_hal_power_insomnia_exit(void) {
}

// API

LoaderStatus
    loader_start(Loader* loader, const char* name, const char* args, FuriString* error_message) {
    furi_check(loader);
    furi_check(name);

    LoaderMessage message;
    LoaderMessageLoaderStatusResult result;

    message.type = LoaderMessageTypeStartByName;
    message.start.name = name;
    message.start.args = args;
    message.start.error_message = error_message;
    message.api_lock = api_lock_alloc_locked();
    message.status_value = &result;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result.value;
}

bool loader_lock(Loader* loader) {
    furi_check(loader);

    LoaderMessage message;
    LoaderMessageBoolResult result;
    message.type = LoaderMessageTypeLock;
    message.api_lock = api_lock_alloc_locked();
    message.bool_value = &result;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result.value;
}

void loader_unlock(Loader* loader) {
    furi_check(loader);

    LoaderMessage message;
    message.type = LoaderMessageTypeUnlock;

    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
}

bool loader_is_locked(Loader* loader) {
    furi_check(loader);

    LoaderMessage message;
    LoaderMessageBoolResult result;
    message.type = LoaderMessageTypeIsLocked;
    message.api_lock = api_lock_alloc_locked();
    message.bool_value = &result;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result.value;
}

FuriPubSub* loader_get_pubsub(Loader* loader) {
    furi_check(loader);
    // it's safe to return pubsub without locking
    // because it's never freed and loader is never exited
    // also the loader instance cannot be obtained until the pubsub is created
    return loader->pubsub;
}

static void
    loader_thread_state_callback(FuriThread* thread, FuriThreadState thread_state, void* context) {
    UNUSED(thread);
    furi_assert(context);

    if(thread_state == FuriThreadStateStopped) {
        Loader* loader = context;

        LoaderMessage message;
        message.type = LoaderMessageTypeAppClosed;
        furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    }
}

// implementation

static Loader* loader_alloc(void) {
    Loader* loader = malloc(sizeof(Loader));
    loader->pubsub = furi_pubsub_alloc();
    loader->queue = furi_message_queue_alloc(1, sizeof(LoaderMessage));
    loader->app.args = NULL;
    loader->app.thread = NULL;
    loader->app.insomniac = false;
    return loader;
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
    const struct {
        const FlipperInternalApplication* list;
        const uint32_t count;
    } lists[] = {
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
    LoaderEvent event;
    event.type = LoaderEventTypeApplicationBeforeLoad;
    furi_pubsub_publish(loader->pubsub, &event);

    // store args
    furi_assert(loader->app.args == NULL);
    if(args && strlen(args) > 0) {
        loader->app.args = strdup(args);
    }

    loader->app.thread =
        furi_thread_alloc_ex(app->name, app->stack_size, app->app, loader->app.args);
    furi_thread_set_appid(loader->app.thread, app->appid);

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

static bool loader_do_is_locked(Loader* loader) {
    return loader->app.thread != NULL;
}

static LoaderStatus loader_do_start_by_name(
    Loader* loader,
    const char* name,
    const char* args,
    FuriString* error_message) {
    LoaderStatus status;
    do {
        // check lock
        if(loader_do_is_locked(loader)) {
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
            LoaderStatusErrorUnknownApp, error_message, "Application \"%s\" not found", name);
    } while(false);

    return status;
}

static bool loader_do_lock(Loader* loader) {
    if(loader->app.thread) {
        return false;
    }

    loader->app.thread = (FuriThread*)LOADER_MAGIC_THREAD_VALUE;
    return true;
}

static void loader_do_unlock(Loader* loader) {
    furi_check(loader->app.thread == (FuriThread*)LOADER_MAGIC_THREAD_VALUE);
    loader->app.thread = NULL;
}

static void loader_do_app_closed(Loader* loader) {
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

    FURI_LOG_I(TAG, "Application stopped. Free heap: %zu", memmgr_get_free_heap());

    LoaderEvent event;
    event.type = LoaderEventTypeApplicationStopped;
    furi_pubsub_publish(loader->pubsub, &event);
}

// app

int32_t loader_srv(void* p) {
    UNUSED(p);
    Loader* loader = loader_alloc();
    furi_record_create(RECORD_LOADER, loader);

    FURI_LOG_I(TAG, "Executing system start hooks");
    for(size_t i = 0; i < FLIPPER_ON_SYSTEM_START_COUNT; i++) {
        FLIPPER_ON_SYSTEM_START[i]();
    }

    if(FLIPPER_AUTORUN_APP_NAME && strlen(FLIPPER_AUTORUN_APP_NAME)) {
        FURI_LOG_I(TAG, "Starting autorun app: %s", FLIPPER_AUTORUN_APP_NAME);
        loader_do_start_by_name(loader, FLIPPER_AUTORUN_APP_NAME, NULL, NULL);
    }

    LoaderMessage message;
    while(true) {
        if(furi_message_queue_get(loader->queue, &message, FuriWaitForever) == FuriStatusOk) {
            switch(message.type) {
            case LoaderMessageTypeStartByName:
                message.status_value->value = loader_do_start_by_name(
                    loader, message.start.name, message.start.args, message.start.error_message);
                api_lock_unlock(message.api_lock);
                break;
            case LoaderMessageTypeIsLocked:
                message.bool_value->value = loader_do_is_locked(loader);
                api_lock_unlock(message.api_lock);
                break;
            case LoaderMessageTypeAppClosed:
                loader_do_app_closed(loader);
                break;
            case LoaderMessageTypeLock:
                message.bool_value->value = loader_do_lock(loader);
                api_lock_unlock(message.api_lock);
                break;
            case LoaderMessageTypeUnlock:
                loader_do_unlock(loader);
                break;
            }
        }
    }

    return 0;
}
