#pragma once

#include "js_runner.h"
#include <m-dict.h>
#include <m-array.h>
#include <stdatomic.h>
#include <furi/furi.h>
#include <toolbox/api_lock.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <jerryscript.h>
#pragma GCC diagnostic pop

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

    FuriEventFlag* is_idle; ///< This flag is set if no script is being run

    FuriEventLoop* event_loop;
    _Atomic bool should_terminate; ///< Flag to terminate JS busy loops
    FuriMessageQueue* command_queue;
    atomic_flag is_execution_handle_taken;
    JsRunnerExecutionHandle* execution_handle;

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
    JsRunnerTerminationCallback termination_callback;
    void* termination_callback_context;
} JsRunnerExecutionHandle;

typedef struct JsRunnerStaticContext {
    atomic_flag is_running;
    JsRunnerApp* volatile app;
} JsRunnerStaticContext;

typedef void (*JsRunnerExternalDataDestructor)(void* object, void* user_p);

typedef struct JsRunnerByteArrayDestructor {
    JsRunnerExternalDataDestructor destructor;
    ByteArray_t* byte_array;
} JsRunnerByteArrayDestructor;
