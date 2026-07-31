/**
 * @file js_runner_i.h
 *
 * @brief Javascript app runner - private declarations
 */
#pragma once
#include "js_runner.h"
#include <furi/furi.h>
#include <storage/storage.h>
#include <path.h>

#include <m-dict.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <jerryscript.h>
#pragma GCC diagnostic pop

#define MIN_INTERVAL_DELAY_MS 10.0f

#define JS_RUNNER_MAX_SCRIPT_SIZE (250 * 1024)

typedef struct IntervalContext {
    bool once;
    FuriEventLoopTimer* timer;
    jerry_value_t callback;
} IntervalContext;

M_DICT_DEF2(IntervalDict, uint32_t, M_DEFAULT_OPLIST, IntervalContext, M_POD_OPLIST);

typedef struct JsRunnerApp {
    size_t heap_size;
    void* jrs_context;
    FuriEventLoop* event_loop;
    JsRunnerConsoleOutCallback console_callback;
    void* console_callback_context;
    FuriString* root_path;
    IntervalDict_t intervals;
    uint32_t last_interval_id;
} JsRunnerApp;

#define PTR_HASH(p) ((size_t)(p))

M_DICT_DEF2(
    AppDict,
    FuriThread*,
    M_OPEXTEND(M_PTR_OPLIST, HASH(PTR_HASH)),
    JsRunnerApp*,
    M_PTR_OPLIST);

typedef struct JsRunner {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    FuriMutex* apps_mutex;
    AppDict_t apps;
} JsRunner;

#define WITH_JS_RUNNER_APP(APP, BLOCK)                                   \
    do {                                                                 \
        furi_mutex_acquire(instance->apps_mutex, FuriWaitForever);       \
        FuriThread* current_thread = furi_thread_get_current();          \
        JsRunnerApp* APP = *AppDict_get(instance->apps, current_thread); \
        if(APP) {                                                        \
            BLOCK                                                        \
        } else {                                                         \
            FURI_LOG_E(TAG, "No JS app handle for current thread");      \
            furi_crash();                                                \
        }                                                                \
        furi_mutex_release(instance->apps_mutex);                        \
    } while(false)

JsRunner* js_runner_get_instance(void);
void js_runner_check_and_free(jerry_value_t val);
void js_runner_check_event_loop(JsRunnerApp* app);

void js_runner_setup_interval_methods(void);
void js_runner_setup_console(JsRunnerApp* app);

/** @brief Allocate Jerryscript context for current thread. This function is used by jerryscript glue. */
size_t js_runner_context_alloc(JsRunner* instance, size_t context_size);

/** @brief Free Jerryscript context for current thread. This function is used by jerryscript glue. */
void js_runner_context_free(JsRunner* instance);

/** @brief Get Jerryscript context for current thread. This function is used by jerryscript glue. */
void* js_runner_context_get(JsRunner* instance);

/** @brief Get root path of the current JS app (folder containg entry point).
 * This function is used by jerryscript glue. */
void js_runner_get_root_path(JsRunner* instance, FuriString* path);

/** @brief Create a string out of a JS exception.
 *
 * @param exception JS exception. This value is not freed.
 * @return exception string or NULL if conversion failed.
 */
FuriString* js_runner_get_exception_string(jerry_value_t exception);
