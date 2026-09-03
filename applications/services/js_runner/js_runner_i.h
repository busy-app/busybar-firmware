/**
 * @file js_runner_i.h
 *
 * @brief Javascript app runner - private declarations
 */
#pragma once
#include "js_runner.h"
#include "js_util.h"
#include "js_runner_types.h"
#include <furi/furi.h>
#include <storage/storage.h>
#include <path.h>
#include <fetch/fetch.h>

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

#define APP_THREAD_STACK_SIZE (8 * 1024)

#define JS_RUNNER_APP_FLAG_IDLE 1

#define PTR_HASH(p) ((size_t)(p))

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

void js_runner_byte_array_destructor(void* object, void* user_p);
void js_runner_heap_destructor(void* object, void* user_p);
