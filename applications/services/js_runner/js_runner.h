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

/** @brief Run a JS application.
 *
 * This function blocks until the script terminates. To forcefully terminate the script use js_runner_kill().
 *
 * @param instance JsRunner instance. Can be obtained with furi_record_open().
 * @param app_id JS application ID (see js_app_launcher).
 * @param path entry point script path.
 * @param heap_size JS heap size for the app in bytes.
 * @param console_write_cb callback function for JS console methods (console.log, console.error, console.info). Supply NULL to disable console.
 * @param console_write_context user pointer passed to console_write_cb.
 *
 * @return error code
 */
JsRunnerError js_runner_run(
    JsRunner* instance,
    const char* app_id,
    const char* path,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_write_cb,
    void* console_write_context);

/** @brief Forcefully terminate a running JS application.
 *
 * @param instance JsRunner instance. Can be obtained with furi_record_open().
 * @param thread thread the thread in which js_runner_run is running.
 * @return true on success, false on failure (given thread does not have a running JS app).
 */
bool js_runner_abort(JsRunner* instance, FuriThread* thread);

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
