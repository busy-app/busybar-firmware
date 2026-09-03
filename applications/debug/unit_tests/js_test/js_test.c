#include "../unit_tests.h"
#include <js_runner/js_runner.h>
#include <storage/storage.h>
#include <string.h>

#define APP_ID "app.busy.js_test"

#define SCRIPT_FILE UNIT_TESTS_PATH("test.js")

#define MODULE1_FILE UNIT_TESTS_PATH("mod1.js")
#define MODULE2_FILE UNIT_TESTS_PATH("mod2.js")

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

MU_TEST(js_tests_console) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(storage, SCRIPT_FILE, "console.log(\"flipppper\");"));
    furi_record_close(RECORD_STORAGE);

    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    char buf[64] = {0};

    mu_assert_int_eq(
        JsRunnerErrorNone,
        js_runner_run(js_runner, APP_ID, SCRIPT_FILE, 4096, js_console_cb, buf));

    mu_assert_string_eq("flipppper\n", buf);

    furi_record_close(RECORD_JS_RUNNER);
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

    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    char buf[64] = {0};

    mu_assert_int_eq(
        JsRunnerErrorNone,
        js_runner_run(js_runner, APP_ID, SCRIPT_FILE, 4096, js_console_cb, buf));

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

    mu_assert_int_eq(
        JsRunnerErrorNone,
        js_runner_run(js_runner, APP_ID, SCRIPT_FILE, 4096, js_console_cb, buf));

    mu_assert_string_eq("OK\n", buf);

    furi_record_close(RECORD_JS_RUNNER);
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

    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    char buf[64] = {0};

    mu_assert_int_eq(
        JsRunnerErrorNone,
        js_runner_run(js_runner, APP_ID, MODULE1_FILE, 4096, js_console_cb, buf));

    mu_assert_string_eq("module\nflipper\n", buf);

    furi_record_close(RECORD_JS_RUNNER);
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
    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    JsRunnerError error =
        js_runner_run(js_runner, APP_ID, SCRIPT_FILE, 4096, interval_test_js_console_cb, context);
    furi_record_close(RECORD_JS_RUNNER);
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

MU_TEST_SUITE(js_test_suite) {
    MU_RUN_TEST(js_tests_console);
    MU_RUN_TEST(js_tests_modules);
    MU_RUN_TEST(js_tests_set_interval);
    MU_RUN_TEST(js_tests_local_storage);
}

int run_minunit_js_test(void) {
    MU_RUN_SUITE(js_test_suite);
    return MU_EXIT_CODE;
}
