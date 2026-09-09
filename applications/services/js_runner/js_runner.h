/**
 * @file js_runner.h
 *
 * @brief Javascript app runner
 */
#pragma once
#include <stddef.h>
#include <furi/core/string.h>
#include <furi/core/thread.h>

#define RECORD_JS_RUNNER "js_runner"

typedef struct JsRunner JsRunner;

typedef enum JsRunnerError {
    JsRunnerErrorNone = 0,
    JsRunnerErrorCannotOpenFile,
    JsRunnerErrorInvalidFileSize,
    JsRunnerErrorCannotReadFile,
    JsRunnerErrorParseException,
    JsRunnerErrorInvalidAppId,
    JsRunnerErrorResource,
    JsRunnerErrorTimeout,
    JsRunnerErrorMax,
} JsRunnerError;

typedef enum JsRunnerConsoleSeverity {
    JsRunnerConsoleSeverityLog,
    JsRunnerConsoleSeverityInfo,
    JsRunnerConsoleSeverityError,
} JsRunnerConsoleSeverity;

typedef enum JsRunnerConsoleSeparator {
    JsRunnerConsoleSeparatorNone,
    JsRunnerConsoleSeparatorSpace,
    JsRunnerConsoleSeparatorNewline,
} JsRunnerConsoleSeparator;

typedef void (*JsRunnerConsoleOutCallback)(
    JsRunnerConsoleSeverity severity,
    const char* buf,
    size_t size,
    JsRunnerConsoleSeparator separator,
    void* context);

typedef struct JsRunnerContextHandle JsRunnerContextHandle;

typedef struct JsRunnerContextInitResult {
    JsRunnerError error;
    JsRunnerContextHandle* handle;
} JsRunnerContextInitResult;

typedef struct JsRunnerExecutionHandle JsRunnerExecutionHandle;

typedef struct JsRunnerRunResult {
    JsRunnerError error;
    JsRunnerExecutionHandle* handle;
} JsRunnerRunResult;

typedef void (*JsRunnerTerminationCallback)(void* context);

/** @brief Allocate a Javascript execution context.
 *
 * @param instance JsRunner instance. Can be obtained with furi_record_open().
 * @param app_id JS application ID (see js_app_launcher).
 * @param heap_size JS heap size for the app in bytes.
 * @param console_write_cb callback function for JS console methods (console.log, console.error, console.info). Supply NULL to disable console.
 * @param console_write_context user pointer passed to console_write_cb.
 *
 * @return operation result. If error is JsRunnerErrorNone, handle is valid.
 */
JsRunnerContextInitResult js_runner_context_alloc(
    JsRunner* instance,
    const char* app_id,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_write_cb,
    void* console_write_context);

/** @brief Free a Javascript execution context.
 *
 * Any execution handles obtained with this context handle must be joined before calling this function.
 *
 * @param handle context handle to be freed.
 */
void js_runner_context_free(JsRunnerContextHandle* handle);

/** @brief Run a JS application.
 *
 * @param handle context handle previously created by js_runner_context_alloc.
 * @param path entry point script path.
 * @param on_terminate function to be called when script terminates.
 * @param context user pointer passed to the on_terminate function.
 *
 * @return operation result. If error is JsRunnerErrorNone, handle is valid.
 */
JsRunnerRunResult js_runner_run(
    JsRunnerContextHandle* handle,
    const char* path,
    JsRunnerTerminationCallback on_terminate,
    void* context);

/** @brief Run a JS code snippet.
 *
 * @param handle context handle previously created by js_runner_context_alloc.
 * @param code JS code (encoding: UTF-8).
 * @param print_result if true, evaluation result of the code snippet is printed using the console callback (severity: log).
 * @param on_terminate function to be called when snippet terminates.
 * @param context user pointer passed to the on_terminate function.
 *
 * @return operation result. If error is JsRunnerErrorNone, handle is valid.
 */
JsRunnerRunResult js_runner_run_snippet(
    JsRunnerContextHandle* handle,
    const char* code,
    bool print_result,
    JsRunnerTerminationCallback on_terminate,
    void* context);

/** @brief Wait until JS run job completes.
 *
 * This function blocks until the script terminates. To forcefully terminate the script use js_runner_abort().
 *
 * @param handle execution handle previously created by js_runner_run or js_runner_run_snippet.
 * @param timeout operation timeout in ticks.
 *
 * @return operation result (JsRunnerErrorTimeout or JsRunnerErrorNone).
 */
JsRunnerError js_runner_join(JsRunnerExecutionHandle* handle, uint32_t timeout);

/** @brief Forcefully terminate a running JS application. The handle must be joined afterwards.
 *
 * @param handle execution handle of the running script.
 */
void js_runner_abort(JsRunnerExecutionHandle* handle);

/** @brief Forcefully terminate all running JS applications.
 *
 * @param instance JsRunner instance. Can be obtained with furi_record_open().
 */
void js_runner_abort_all(JsRunner* instance);

/** @brief Get human-readable error message corresponding to an error code.
 *
 * @param error error code
 * @return error message
 */
const char* js_runner_get_error_message(JsRunnerError error);
