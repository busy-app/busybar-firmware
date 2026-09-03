#include "../unit_tests.h"
#include <js_runner/js_runner.h>
#include <storage/storage.h>
#include <string.h>

#define APP_ID "app.busy.js_test"

#define SCRIPT_FILE UNIT_TESTS_PATH("test.js")

#define MODULE1_FILE UNIT_TESTS_PATH("mod1.js")
#define MODULE2_FILE UNIT_TESTS_PATH("mod2.js")

#define HEAP_SIZE 4096

static bool create_file(Storage* storage, const char* path, const char* data) {
    File* file = storage_file_alloc(storage);
    bool result = false;
    do {
        if(!storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }

        if(storage_file_write(file, data, strlen(data)) != strlen(data)) {
            break;
        }

        if(!storage_file_close(file)) {
            break;
        }

        result = true;
    } while(0);

    storage_file_free(file);
    return result;
}

static void js_console_cb(
    JsRunnerConsoleSeverity severity,
    const char* buf,
    size_t size,
    JsRunnerConsoleSeparator separator,
    void* context) {
    UNUSED(severity);

    char* out_buf = context;
    size_t out_buf_len = strlen(out_buf);
    memcpy(out_buf + out_buf_len, buf, size);
    switch(separator) {
    case JsRunnerConsoleSeparatorNone:
        break;
    case JsRunnerConsoleSeparatorSpace:
        out_buf[out_buf_len + size] = ' ';
        break;
    case JsRunnerConsoleSeparatorNewline:
        out_buf[out_buf_len + size] = '\n';
        break;
    }
}

static JsRunnerError run_and_join(
    const char* path,
    JsRunnerConsoleOutCallback console_write_cb,
    void* console_write_context) {
    JsRunnerError result = JsRunnerErrorNone;

    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    do {
        JsRunnerContextInitResult context_init_result = js_runner_context_alloc(
            js_runner, APP_ID, HEAP_SIZE, console_write_cb, console_write_context);
        if(context_init_result.error != JsRunnerErrorNone) {
            result = context_init_result.error;
            break;
        }
        do {
            JsRunnerRunResult run_result =
                js_runner_run(context_init_result.handle, path, NULL, NULL);
            if(run_result.error != JsRunnerErrorNone) {
                result = run_result.error;
                break;
            }
            result = js_runner_join(run_result.handle, FuriWaitForever);
        } while(false);
        js_runner_context_free(context_init_result.handle);
    } while(false);
    furi_record_close(RECORD_JS_RUNNER);
    return result;
}

MU_TEST(js_tests_console) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(storage, SCRIPT_FILE, "console.log(\"flipppper\");"));
    furi_record_close(RECORD_STORAGE);

    char buf[64] = {0};

    mu_assert_int_eq(JsRunnerErrorNone, run_and_join(SCRIPT_FILE, js_console_cb, buf));

    mu_assert_string_eq("flipppper\n", buf);
}

MU_TEST(js_tests_local_storage) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(
        storage,
        SCRIPT_FILE,
        "localStorage.clear();"
        "localStorage.setItem('hello', 'world');"
        "localStorage.setItem('storage', 'test');"
        "let ok = true;"
        "ok = ok && localStorage.length === 2;"
        "ok = ok && localStorage.getItem('hello') === 'world';"
        "ok = ok && localStorage.getItem('storage') === 'test';"
        "ok = ok && localStorage.key(0) !== null;"
        "ok = ok && localStorage.key(1) !== null;"
        "ok = ok && localStorage.key(2) === null;"
        "if(ok) { console.log('OK'); }"));

    char buf[64] = {0};

    mu_assert_int_eq(JsRunnerErrorNone, run_and_join(SCRIPT_FILE, js_console_cb, buf));

    mu_assert_string_eq("OK\n", buf);

    buf[0] = 0;

    mu_check(create_file(
        storage,
        SCRIPT_FILE,
        "let ok = true;"
        "ok = ok && localStorage.length === 2;"
        "ok = ok && localStorage.getItem('hello') === 'world';"
        "ok = ok && localStorage.getItem('storage') === 'test';"
        "localStorage.removeItem('storage');"
        "ok = ok && localStorage.length === 1;"
        "ok = ok && localStorage.getItem('hello') === 'world';"
        "ok = ok && localStorage.getItem('storage') === null;"
        "if(ok) { console.log('OK'); }"));

    mu_assert_int_eq(JsRunnerErrorNone, run_and_join(SCRIPT_FILE, js_console_cb, buf));

    mu_assert_string_eq("OK\n", buf);

    furi_record_close(RECORD_STORAGE);
}

MU_TEST(js_tests_modules) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(
        storage,
        MODULE1_FILE,
        "import { hello } from './mod2.js'; console.log(\"module\"); hello(\"flipper\");"));
    mu_check(create_file(storage, MODULE2_FILE, "export function hello(arg) {console.log(arg);}"));
    furi_record_close(RECORD_STORAGE);

    char buf[64] = {0};

    mu_assert_int_eq(JsRunnerErrorNone, run_and_join(MODULE1_FILE, js_console_cb, buf));

    mu_assert_string_eq("module\nflipper\n", buf);
}

static void interval_test_js_console_cb(
    JsRunnerConsoleSeverity severity,
    const char* buf,
    size_t size,
    JsRunnerConsoleSeparator separator,
    void* context) {
    UNUSED(severity);
    UNUSED(separator);

    FuriMessageQueue* queue = context;

    FuriString* string = furi_string_alloc_printf("%.*s", size, buf);
    furi_message_queue_put(queue, &string, FuriWaitForever);
}

static int32_t set_interval_thread_cb(void* context) {
    JsRunnerError error = run_and_join(SCRIPT_FILE, interval_test_js_console_cb, context);
    FURI_LOG_D("JsTest", "run returned %d", error);
    return error;
}

static void set_interval_check_output(FuriMessageQueue* queue, bool* success) {
    FuriString* console_string = NULL;
    FuriStatus status;

    status = furi_message_queue_get(queue, &console_string, furi_ms_to_ticks(1000));
    mu_assert_int_eq(FuriStatusOk, status);
    mu_assert_string_eq("Hello 0", furi_string_get_cstr(console_string));
    furi_string_free(console_string);

    status = furi_message_queue_get(queue, &console_string, furi_ms_to_ticks(50));
    mu_assert_int_eq(FuriStatusErrorTimeout, status);

    status = furi_message_queue_get(queue, &console_string, furi_ms_to_ticks(200));
    mu_assert_int_eq(FuriStatusOk, status);
    mu_assert_string_eq("Hello 1", furi_string_get_cstr(console_string));
    furi_string_free(console_string);

    status = furi_message_queue_get(queue, &console_string, furi_ms_to_ticks(300));
    mu_assert_int_eq(FuriStatusErrorTimeout, status);

    FURI_LOG_D("JsTest", "all strings ok");

    *success = true;
}

MU_TEST(js_tests_set_interval) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(
        storage,
        SCRIPT_FILE,
        "let count = 0; let id = undefined;"
        "function callback() {"
        "    if(count == 2) {"
        "        clearInterval(id);"
        "    } else {"
        "        console.log('Hello ' + count);"
        "        count++;"
        "    }"
        "}"
        "id = setInterval(callback, 200);"));
    furi_record_close(RECORD_STORAGE);

    FuriMessageQueue* queue = furi_message_queue_alloc(4, sizeof(FuriString*));

    FuriThread* thread =
        furi_thread_alloc_ex("js_set_interval_test", 8192, set_interval_thread_cb, queue);

    furi_thread_start(thread);

    bool success = false;
    set_interval_check_output(queue, &success);

    furi_thread_join(thread);
    mu_assert_int_eq(JsRunnerErrorNone, furi_thread_get_return_code(thread));
    furi_thread_free(thread);

    furi_message_queue_free(queue);

    mu_check(success);
}

MU_TEST(js_tests_handle) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(storage, SCRIPT_FILE, "undefined;"));
    furi_record_close(RECORD_STORAGE);

    JsRunner* instance = furi_record_open(RECORD_JS_RUNNER);
    JsRunnerContextInitResult result1 =
        js_runner_context_alloc(instance, APP_ID, HEAP_SIZE, NULL, NULL);
    mu_assert_int_eq(JsRunnerErrorNone, result1.error);
    mu_assert_not_null(result1.handle);
    JsRunnerContextInitResult result2 =
        js_runner_context_alloc(instance, APP_ID, HEAP_SIZE, NULL, NULL);
    mu_assert_int_eq(JsRunnerErrorResource, result2.error);
    mu_assert_null(result2.handle);

    JsRunnerRunResult run_result1 = js_runner_run(result1.handle, SCRIPT_FILE, NULL, NULL);
    mu_assert_int_eq(JsRunnerErrorNone, run_result1.error);
    mu_assert_not_null(run_result1.handle);

    JsRunnerRunResult run_result2 = js_runner_run(result1.handle, SCRIPT_FILE, NULL, NULL);
    mu_assert_int_eq(JsRunnerErrorResource, run_result2.error);
    mu_assert_null(run_result2.handle);

    mu_assert_int_eq(JsRunnerErrorNone, js_runner_join(run_result1.handle, FuriWaitForever));

    JsRunnerRunResult run_result3 = js_runner_run(result1.handle, SCRIPT_FILE, NULL, NULL);
    mu_assert_int_eq(JsRunnerErrorNone, run_result3.error);
    mu_assert_not_null(run_result3.handle);

    mu_assert_int_eq(JsRunnerErrorNone, js_runner_join(run_result3.handle, FuriWaitForever));

    js_runner_context_free(result1.handle);

    furi_record_close(RECORD_JS_RUNNER);
}

static void terminate_callback(JsRunnerExecutionHandle* handle, void* context) {
    UNUSED(handle);
    bool* done = context;
    *done = true;
}

MU_TEST(js_tests_callback) {
    JsRunner* instance = furi_record_open(RECORD_JS_RUNNER);

    JsRunnerContextInitResult result =
        js_runner_context_alloc(instance, APP_ID, HEAP_SIZE, NULL, NULL);
    mu_assert_int_eq(JsRunnerErrorNone, result.error);
    mu_assert_not_null(result.handle);

    bool done = false;
    JsRunnerRunResult run_result =
        js_runner_run_snippet(result.handle, "2+2", false, terminate_callback, &done);
    mu_assert_int_eq(JsRunnerErrorNone, run_result.error);
    mu_assert_not_null(run_result.handle);

    mu_assert_int_eq(JsRunnerErrorNone, js_runner_join(run_result.handle, FuriWaitForever));

    mu_check(done);

    js_runner_context_free(result.handle);

    furi_record_close(RECORD_JS_RUNNER);
}

MU_TEST_SUITE(js_test_suite) {
    MU_RUN_TEST(js_tests_console);
    MU_RUN_TEST(js_tests_modules);
    MU_RUN_TEST(js_tests_set_interval);
    MU_RUN_TEST(js_tests_local_storage);
    MU_RUN_TEST(js_tests_handle);
    MU_RUN_TEST(js_tests_callback);
}

int run_minunit_js_test(void) {
    MU_RUN_SUITE(js_test_suite);
    return MU_EXIT_CODE;
}
