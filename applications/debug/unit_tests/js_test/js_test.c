#include "../unit_tests.h"
#include <js_runner/js_runner.h>
#include <js_app/js_app.h>
#include <js_app/js_app_installer.h>
#include <js_app/js_app_registry.h>
#include <input/input.h>
#include <storage/storage.h>
#include <toolbox/tar/tar_archive.h>
#include <string.h>

#define APP_ID "app.busy.js_test"

#define SCRIPT_FILE UNIT_TESTS_PATH("test.js")

#define MODULE1_FILE UNIT_TESTS_PATH("mod1.js")
#define MODULE2_FILE UNIT_TESTS_PATH("mod2.js")

#define SETTINGS_APP_ROOT      EXT_PATH("user_assets/" APP_ID)
#define SETTINGS_APPMETA_DIR   SETTINGS_APP_ROOT "/appmeta"
#define SETTINGS_SCRIPTS_DIR   SETTINGS_APP_ROOT "/scripts"
#define SETTINGS_MANIFEST_FILE SETTINGS_APPMETA_DIR "/manifest.json"
#define SETTINGS_SCHEMA_FILE   SETTINGS_APPMETA_DIR "/settings.json"
#define SETTINGS_MAIN_FILE     SETTINGS_SCRIPTS_DIR "/main.js"
#define SETTINGS_VALUES_FILE   EXT_PATH("apps_data/jsrunner/" APP_ID ".json")

#define INSTALL_APP_ID       "app.busy.js_install_test"
#define INSTALL_APP_ROOT     EXT_PATH("user_assets/" INSTALL_APP_ID)
#define INSTALL_ARCHIVE_FILE UNIT_TESTS_PATH("js_install_test.tgz")
#define INSTALL_ESCAPE_FILE  EXT_PATH("tmp/escaped.js")

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

static bool
    create_install_archive(Storage* storage, const char* version, bool include_path_traversal) {
    FuriString* manifest = furi_string_alloc_printf(
        "{\"format_version\":1,\"id\":\"%s\",\"name\":\"Install test\","
        "\"version\":\"%s\"}",
        INSTALL_APP_ID,
        version);
    TarArchive* archive = tar_archive_alloc(storage);
    bool success = false;

    do {
        if(!tar_archive_open(archive, INSTALL_ARCHIVE_FILE, TarOpenModeWrite)) break;
        if(!tar_archive_dir_add_element(archive, "appmeta")) break;
        if(!tar_archive_dir_add_element(archive, "scripts")) break;
        if(!tar_archive_store_data(
               archive,
               "appmeta/manifest.json",
               (const uint8_t*)furi_string_get_cstr(manifest),
               furi_string_size(manifest))) {
            break;
        }
        if(!tar_archive_store_data(archive, "scripts/main.js", (const uint8_t*)";", 1)) {
            break;
        }
        if(include_path_traversal &&
           !tar_archive_store_data(archive, "../escaped.js", (const uint8_t*)";", 1)) {
            break;
        }
        success = tar_archive_finalize(archive);
    } while(false);

    tar_archive_free(archive);
    furi_string_free(manifest);
    return success;
}

MU_TEST(js_tests_app_install) {
    mu_check(js_app_id_is_valid(INSTALL_APP_ID));
    mu_check(js_app_id_is_valid("app.example_1-test"));
    mu_check(!js_app_id_is_valid(""));
    mu_check(!js_app_id_is_valid(".."));
    mu_check(!js_app_id_is_valid("app/example"));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove_recursive(storage, INSTALL_APP_ROOT);
    storage_simply_remove(storage, INSTALL_ARCHIVE_FILE);
    storage_simply_remove(storage, INSTALL_ESCAPE_FILE);
    furi_record_close(RECORD_STORAGE);

    FuriString* installed_id = furi_string_alloc();

    storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_install_archive(storage, "1.0.0", false));
    furi_record_close(RECORD_STORAGE);
    mu_assert_int_eq(
        JsAppInstallResultOk, js_app_installer_install(INSTALL_ARCHIVE_FILE, installed_id));
    mu_assert_string_eq(INSTALL_APP_ID, furi_string_get_cstr(installed_id));

    mu_assert_int_eq(
        JsAppInstallResultVersionConflict,
        js_app_installer_install(INSTALL_ARCHIVE_FILE, installed_id));

    storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_install_archive(storage, "1.0.1-rc.1", false));
    furi_record_close(RECORD_STORAGE);
    mu_assert_int_eq(
        JsAppInstallResultOk, js_app_installer_install(INSTALL_ARCHIVE_FILE, installed_id));

    storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_install_archive(storage, "1.0.1", false));
    furi_record_close(RECORD_STORAGE);
    mu_assert_int_eq(
        JsAppInstallResultOk, js_app_installer_install(INSTALL_ARCHIVE_FILE, installed_id));

    JsApp* installed_app = js_app_registry_get_app(INSTALL_APP_ID);
    mu_check(installed_app != NULL);
    js_app_free(installed_app);

    storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_install_archive(storage, "2.0.0", true));
    furi_record_close(RECORD_STORAGE);
    mu_assert_int_eq(
        JsAppInstallResultInvalidArchive,
        js_app_installer_install(INSTALL_ARCHIVE_FILE, installed_id));

    storage = furi_record_open(RECORD_STORAGE);
    mu_check(!storage_common_exists(storage, INSTALL_ESCAPE_FILE));
    furi_record_close(RECORD_STORAGE);

    mu_check(js_app_registry_remove_app(INSTALL_APP_ID));
    furi_string_free(installed_id);

    storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove(storage, INSTALL_ARCHIVE_FILE);
    furi_record_close(RECORD_STORAGE);
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

MU_TEST(js_tests_settings) {
    static const char* manifest = "{\"format_version\":1,\"id\":\"" APP_ID
                                  "\",\"name\":\"Settings test\",\"version\":\"1.0.0\"}";
    static const char* schema =
        "{\"format_version\":1,\"version\":1,\"settings\":[{\"name\":\"timer\","
        "\"label\":\"Timer\",\"type\":\"group\",\"sub_label_setting\":\"mode\","
        "\"settings\":[{\"name\":\"mode\",\"label\":\"Mode\","
        "\"type\":\"selector\",\"default\":\"simple\",\"options\":["
        "{\"value\":\"simple\",\"label\":\"Simple\"},"
        "{\"value\":\"interval\",\"label\":\"Interval\"}]},"
        "{\"name\":\"cycles\",\"label\":\"Cycles\",\"type\":\"spinbox\","
        "\"default\":3,\"min\":2,\"max\":35,\"step\":1,"
        "\"visible_if\":{\"setting\":\"mode\",\"equals\":\"interval\"}}]}]}";

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove(storage, SETTINGS_VALUES_FILE);
    mu_check(storage_simply_mkpath(storage, SETTINGS_APPMETA_DIR));
    mu_check(storage_simply_mkpath(storage, SETTINGS_SCRIPTS_DIR));
    mu_check(create_file(storage, SETTINGS_MANIFEST_FILE, manifest));
    mu_check(create_file(storage, SETTINGS_SCHEMA_FILE, schema));
    mu_check(create_file(storage, SETTINGS_MAIN_FILE, ";"));
    mu_check(create_file(
        storage,
        SCRIPT_FILE,
        "Settings.load().then(function(config) {"
        "let values = config.values;"
        "let ok = config.format_version === 1 && config.version === 1;"
        "ok = ok && values.timer.mode === 'simple' && values.timer.cycles === 3;"
        "values.timer.mode = 'interval'; values.timer.cycles = 4;"
        "Settings.save(values).then(function() { if(ok) console.log('OK'); });"
        "});"));
    furi_record_close(RECORD_STORAGE);

    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    char buf[64] = {0};
    mu_assert_int_eq(
        JsRunnerErrorNone,
        js_runner_run(js_runner, APP_ID, SCRIPT_FILE, 8192, js_console_cb, buf));
    mu_assert_string_eq("OK\n", buf);

    buf[0] = 0;
    storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(
        storage,
        SCRIPT_FILE,
        "Settings.load().then(function(config) {"
        "let values = config.values;"
        "let ok = values.timer.mode === 'interval' && values.timer.cycles === 4;"
        "values.timer.cycles = 100;"
        "Settings.save(values).then("
        "function() { console.log('INVALID'); },"
        "function() { if(ok) console.log('OK'); });"
        "});"));
    furi_record_close(RECORD_STORAGE);

    mu_assert_int_eq(
        JsRunnerErrorNone,
        js_runner_run(js_runner, APP_ID, SCRIPT_FILE, 8192, js_console_cb, buf));
    mu_assert_string_eq("OK\n", buf);
    furi_record_close(RECORD_JS_RUNNER);

    storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove_recursive(storage, SETTINGS_APP_ROOT);
    storage_simply_remove(storage, SETTINGS_VALUES_FILE);
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

static int32_t input_test_thread_cb(void* context) {
    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    JsRunnerError error =
        js_runner_run(js_runner, APP_ID, SCRIPT_FILE, 4096, interval_test_js_console_cb, context);
    furi_record_close(RECORD_JS_RUNNER);
    return error;
}

static void input_test_expect_output(FuriMessageQueue* queue, const char* expected) {
    FuriString* console_string = NULL;
    FuriStatus status = furi_message_queue_get(queue, &console_string, furi_ms_to_ticks(1000));
    mu_assert_int_eq(FuriStatusOk, status);
    mu_assert_string_eq(expected, furi_string_get_cstr(console_string));
    furi_string_free(console_string);
}

MU_TEST(js_tests_input_button_actions) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    mu_check(create_file(
        storage,
        SCRIPT_FILE,
        "let backRejected = false, modeRejected = false;"
        "try { Input.on('back', function(event) {}); } catch(error) { backRejected = true; }"
        "try { Input.on('mode', function(event) {}); } catch(error) { modeRejected = true; }"
        "console.log(backRejected && modeRejected ? 'reserved' : 'exposed');"
        "Input.on('startPause', function(event) { console.log(event.action); });"
        "console.log('ready');"));
    furi_record_close(RECORD_STORAGE);

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(FuriString*));
    FuriThread* thread = furi_thread_alloc_ex("js_input_test", 8192, input_test_thread_cb, queue);
    furi_thread_start(thread);

    input_test_expect_output(queue, "reserved");
    input_test_expect_output(queue, "ready");

    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    InputEvent events[] = {
        {.key = InputKeyStart, .type = InputTypePress},
        {.key = InputKeyStart, .type = InputTypeLong},
        {.key = InputKeyStart, .type = InputTypeRepeat},
        {.key = InputKeyStart, .type = InputTypeRelease},
        {.key = InputKeyBack, .type = InputTypePress},
        {.key = InputKeyBack, .type = InputTypeRelease},
        {.key = InputKeyBack, .type = InputTypeShort},
        {.key = InputKeyApps, .type = InputTypePress},
    };

    for(size_t i = 0; i < COUNT_OF(events); i++) {
        furi_pubsub_publish(input_events, &events[i]);
    }
    furi_record_close(RECORD_INPUT_EVENTS);

    input_test_expect_output(queue, "press");
    input_test_expect_output(queue, "release");

    FuriString* unexpected_output = NULL;
    mu_assert_int_eq(
        FuriStatusErrorTimeout,
        furi_message_queue_get(queue, &unexpected_output, furi_ms_to_ticks(50)));

    JsRunner* js_runner = furi_record_open(RECORD_JS_RUNNER);
    mu_check(js_runner_abort(js_runner, thread));
    furi_record_close(RECORD_JS_RUNNER);

    furi_thread_join(thread);
    mu_assert_int_eq(JsRunnerErrorNone, furi_thread_get_return_code(thread));
    furi_thread_free(thread);
    furi_message_queue_free(queue);
}

MU_TEST_SUITE(js_test_suite) {
    MU_RUN_TEST(js_tests_app_install);
    MU_RUN_TEST(js_tests_console);
    MU_RUN_TEST(js_tests_modules);
    MU_RUN_TEST(js_tests_set_interval);
    MU_RUN_TEST(js_tests_local_storage);
    MU_RUN_TEST(js_tests_settings);
    MU_RUN_TEST(js_tests_input_button_actions);
}

int run_minunit_js_test(void) {
    MU_RUN_SUITE(js_test_suite);
    return MU_EXIT_CODE;
}
