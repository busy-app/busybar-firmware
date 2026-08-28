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
#include <toolbox/api_lock.h>

#include <m-dict.h>
#include <m-array.h>

#include <stdatomic.h>

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

#define APP_THREAD_STACK_SIZE 2048

#define JS_RUNNER_APP_FLAG_IDLE 1

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
    JsRunnerAppCommandTypeRunFile,
    JsRunnerAppCommandTypeQuit,
    JsRunnerAppCommandTypeRunSnippet,

    JsRunnerAppCommandTypeMax,
} JsRunnerAppCommandType;

typedef struct JsRunnerAppCommandRun {
    const char* path;
    JsRunnerContextHandle* context_handle;
} JsRunnerAppCommandRunFile;

typedef struct JsRunnerAppCommandRunSnippet {
    const char* code;
    bool print_result;
    JsRunnerContextHandle* context_handle;
} JsRunnerAppCommandRunSnippet;

typedef struct JsRunnerAppCommand {
    JsRunnerAppCommandType type;
    FuriApiLock lock;
    JsRunnerError* result;

    union {
        JsRunnerAppCommandRunFile run_file;
        JsRunnerAppCommandRunSnippet run_snippet;
    };
} JsRunnerAppCommand;

typedef struct JsRunnerApp {
    const FuriString* app_id;
    FuriThread* thread;

    size_t heap_size;
    void* jrs_context;
    FuriString* root_path;

    FuriEventFlag* is_idle; ///< This flag is set if a script is being run

    FuriEventLoop* event_loop;
    _Atomic bool should_terminate; ///< Flag to terminate JS busy loops
    FuriMessageQueue* command_queue;
    atomic_flag is_execution_handle_taken;

    JsRunnerAppConsole console;
    JsRunnerAppInterval interval;
    JsRunnerAppFetch fetch;
} JsRunnerApp;

typedef struct JsRunner {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
} JsRunner;

typedef struct JsRunnerContextHandle {
    JsRunner* instance;
    JsRunnerApp* app;
    FuriThread* thread;
    FuriMessageQueue* command_queue;
} JsRunnerContextHandle;

typedef struct JsRunnerExecutionHandle {
    JsRunnerContextHandle* context_handle;
    JsRunnerApp* app;
} JsRunnerExecutionHandle;

typedef struct JsRunnerStaticContext {
    atomic_flag is_running;
    JsRunnerApp* volatile app;
} JsRunnerStaticContext;

extern JsRunnerStaticContext js_runner_static_context;

#define WITH_JS_RUNNER_APP(APP, BLOCK)                         \
    do {                                                       \
        JsRunnerApp* APP = js_runner_static_context.app;       \
        furi_assert(APP->thread == furi_thread_get_current()); \
        BLOCK                                                  \
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
 * This function is used by jerryscript glue.
 *
 * @return true if root path (and file operations) is available, false otherwise */
bool js_runner_get_root_path(FuriString* path);

void js_runner_add_fetch_thread(JsRunnerApp* app, JsFetch* fetch);
void js_runner_del_fetch_thread(JsRunnerApp* app, JsFetch* fetch);

const char* js_runner_app_get_id(const JsRunnerApp* app);
