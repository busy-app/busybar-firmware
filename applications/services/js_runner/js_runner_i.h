/**
 * @file js_runner_i.h
 *
 * @brief Javascript app runner - private declarations
 */
#pragma once
#include "js_runner.h"
#include "js_util.h"
#include <furi/furi.h>
#include <storage/storage.h>
#include <path.h>
#include <fetch/fetch.h>

#include <m-dict.h>
#include <m-array.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <jerryscript.h>
#pragma GCC diagnostic pop

// #define JS_DEBUG

#if defined(JS_DEBUG)
#define JS_TRACE(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define JS_TRACE(...)
#endif

#define MIN_INTERVAL_DELAY_MS 10.0f
#define MAX_FETCH_MESSAGES    32
#define MAX_COMMAND_MESSAGES  4

#define PTR_HASH(p) ((size_t)(p))

typedef enum ChildStatus {
    ChildStatusNotYet,
    ChildStatusRunning,
    ChildStatusDone
} ChildStatus;

#define JS_RUNNER_MAX_SCRIPT_SIZE (250 * 1024)

typedef struct IntervalContext {
    bool once;
    FuriEventLoopTimer* timer;
    jerry_value_t callback;
} IntervalContext;

M_DICT_DEF2(IntervalDict, uint32_t, M_DEFAULT_OPLIST, IntervalContext, M_POD_OPLIST);
ARRAY_DEF(ByteArray, uint8_t);

typedef struct JsRunnerAppConsole {
    JsRunnerConsoleOutCallback callback;
    void* callback_context;
} JsRunnerAppConsole;

typedef struct JsRunnerAppInterval {
    IntervalDict_t intervals;
    uint32_t last_id;
} JsRunnerAppInterval;

typedef struct JsFetch JsFetch;
ARRAY_DEF(FetchArray, JsFetch*, M_PTR_OPLIST);
typedef struct JsRunnerAppFetch {
    FetchArray_t fetches;
    FuriMessageQueue* event_queue;
} JsRunnerAppFetch;

typedef enum JsRunnerAppCommandType {
    JsRunnerAppCommandTypeInvalid,
    JsRunnerAppCommandTypeAbort,

    JsRunnerAppCommandTypeMax,
} JsRunnerAppCommandType;

typedef struct JsRunnerApp {
    FuriString* app_id;

    size_t heap_size;
    void* jrs_context;
    FuriEventLoop* event_loop;
    FuriString* root_path;
    _Atomic bool should_terminate;
    FuriMessageQueue* command_queue;

    JsRunnerAppConsole console;
    JsRunnerAppInterval interval;
    JsRunnerAppFetch fetch;
} JsRunnerApp;

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

#define WITH_JS_RUNNER_APP(APP, BLOCK)                                                           \
    do {                                                                                         \
        JsRunner* __instance = furi_record_open(RECORD_JS_RUNNER);                               \
        furi_check(furi_mutex_acquire(__instance->apps_mutex, FuriWaitForever) == FuriStatusOk); \
        FuriThread* current_thread = furi_thread_get_current();                                  \
        JsRunnerApp* APP = *AppDict_get(__instance->apps, current_thread);                       \
        if(APP) {                                                                                \
            BLOCK                                                                                \
        } else {                                                                                 \
            FURI_LOG_E(TAG, "No JS app handle for current thread");                              \
            furi_crash();                                                                        \
        }                                                                                        \
        furi_check(furi_mutex_release(__instance->apps_mutex) == FuriStatusOk);                  \
        furi_record_close(RECORD_JS_RUNNER);                                                     \
    } while(false)

#define JS_ARG(n) (args_count > (n) ? args[(n)] : jerry_undefined())

void js_runner_app_stop_if_done(JsRunnerApp* app);
void js_run_jobs(void);

/** @brief Allocate Jerryscript context for current thread. This function is used by jerryscript glue.
 *
 * Jerryscript uses global context. JsRunner keeps track of jerryscript context, allocating a separate context instance for each thread.
 */
size_t js_runner_thread_context_alloc(size_t context_size);

/** @brief Free Jerryscript context for current thread. This function is used by jerryscript glue.
 *
 * Jerryscript uses global context. JsRunner keeps track of jerryscript context, allocating a separate context instance for each thread.
 * */
void js_runner_thread_context_free(void);

/** @brief Get Jerryscript context for current thread. This function is used by jerryscript glue.
 *
 * Jerryscript uses global context. JsRunner keeps track of jerryscript context, allocating a separate context instance for each thread.
 * */
void* js_runner_thread_context_get(void);

/** @brief Get root path of the current JS app (folder containg entry point).
 * This function is used by jerryscript glue. */
void js_runner_get_root_path(FuriString* path);

void js_runner_add_fetch_thread(JsRunnerApp* app, JsFetch* fetch);
void js_runner_del_fetch_thread(JsRunnerApp* app, JsFetch* fetch);

const char* js_runner_app_get_id(const JsRunnerApp* app);
