#include "js_runner_i.h"

#define TAG "JsRunner"

size_t js_runner_context_alloc(JsRunner* instance, size_t context_size) {
    size_t alloc_size = 0;
    WITH_JS_RUNNER_APP(app, {
        furi_check(!app->jrs_context);
        alloc_size = context_size + app->heap_size;
        app->jrs_context = malloc(alloc_size);
    });
    return alloc_size;
}

void js_runner_context_free(JsRunner* instance) {
    WITH_JS_RUNNER_APP(app, {
        furi_check(app->jrs_context);
        free(app->jrs_context);
        app->jrs_context = NULL;
    });
}

void* js_runner_context_get(JsRunner* instance) {
    void* result = NULL;
    WITH_JS_RUNNER_APP(app, {
        furi_check(app->jrs_context);
        result = app->jrs_context;
    });
    return result;
}

void js_runner_get_root_path(JsRunner* instance, FuriString* path) {
    WITH_JS_RUNNER_APP(app, { furi_string_set(path, app->root_path); });
}

static const jerry_object_native_info_t global_native_info = {
    .free_cb = NULL,
};

JsRunner* js_runner_get_instance(void) {
    jerry_value_t global_obj = jerry_current_realm();
    JsRunner* instance = jerry_object_get_native_ptr(global_obj, &global_native_info);
    furi_check(instance);
    jerry_value_free(global_obj);
    return instance;
}

static bool app_has_background_tasks(JsRunnerApp* app) {
    return !IntervalDict_empty_p(app->intervals);
}

void js_runner_check_event_loop(JsRunnerApp* app) {
    if(!app_has_background_tasks(app)) {
        furi_event_loop_stop(app->event_loop);
    }
}

void js_runner_check_and_free(jerry_value_t val) {
    furi_check(!jerry_value_is_exception(val));
    jerry_value_free(val);
}

FuriString* js_runner_get_exception_string(jerry_value_t exception) {
    jerry_value_t val = jerry_exception_value(exception, false);
    jerry_value_t str = jerry_value_to_string(val);
    jerry_size_t string_size = jerry_string_size(str, JERRY_ENCODING_UTF8);
    char* buf = malloc(string_size + 1);
    jerry_string_to_buffer(str, JERRY_ENCODING_UTF8, (jerry_char_t*)buf, string_size);
    FuriString* result = furi_string_alloc();
    furi_string_set_strn(result, buf, string_size);
    free(buf);
    jerry_value_free(str);
    jerry_value_free(val);
    return result;
}

static void log_exception(const char* msg, jerry_value_t exception) {
    FuriString* exception_string = js_runner_get_exception_string(exception);
    FURI_LOG_E(TAG, "%s: %s", msg, furi_string_get_cstr(exception_string));
    furi_string_free(exception_string);
}

JsRunnerError js_runner_run(
    JsRunner* instance,
    const char* path,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_write_cb,
    void* console_write_context) {
    FURI_LOG_I(TAG, "Running script: %s", path);

    JsRunnerError ret = JsRunnerErrorNone;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(storage);
    do {
        if(!storage_file_open(f, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            ret = JsRunnerErrorCannotOpenFile;
            break;
        }
        uint64_t file_size = storage_file_size(f);
        char* buf = malloc(file_size);
        if(storage_file_read(f, buf, file_size) != file_size) {
            ret = JsRunnerErrorCannotOpenFile;
            free(buf);
            break;
        }

        JsRunnerApp app = {
            .heap_size = heap_size,
            .jrs_context = NULL,
            .event_loop = furi_event_loop_alloc(),
            .console_callback = console_write_cb,
            .console_callback_context = console_write_context,
            .root_path = furi_string_alloc(),
            .last_interval_id = 0,
        };
        IntervalDict_init(app.intervals);
        path_extract_dirname(path, app.root_path);

        {
            furi_mutex_acquire(instance->apps_mutex, FuriWaitForever);
            AppDict_set_at(instance->apps, furi_thread_get_current(), &app);
            furi_mutex_release(instance->apps_mutex);
        }

        jerry_init(JERRY_INIT_EMPTY);

        {
            jerry_value_t global_obj = jerry_current_realm();
            jerry_object_set_native_ptr(global_obj, &global_native_info, instance);
            jerry_value_free(global_obj);
        }
        js_runner_setup_console(&app);
        js_runner_setup_interval_methods();

        FuriString* path_furi = furi_string_alloc_set_str(path);
        FuriString* filename_furi = furi_string_alloc();
        path_extract_filename(path_furi, filename_furi, false);
        jerry_value_t source_name = jerry_string_sz(furi_string_get_cstr(filename_furi));
        furi_string_free(filename_furi);
        furi_string_free(path_furi);

        jerry_parse_options_t parse_options = {
            .options = JERRY_PARSE_HAS_SOURCE_NAME | JERRY_PARSE_MODULE,
            .source_name = source_name,
        };

        jerry_value_t parsed_script =
            jerry_parse((const jerry_char_t*)buf, file_size, &parse_options);
        free(buf);
        do {
            if(jerry_value_is_exception(parsed_script)) {
                log_exception("Error parsing script", parsed_script);
                ret = JsRunnerParseException;
                break;
            } else {
                jerry_value_t link_result = jerry_module_link(parsed_script, NULL, NULL);
                if(jerry_value_is_exception(link_result)) {
                    log_exception("Error linking modules", link_result);
                } else {
                    jerry_value_t result = jerry_module_evaluate(parsed_script);
                    if(jerry_value_is_exception(result)) {
                        log_exception("Error running script", result);
                    } else {
                        if(app_has_background_tasks(&app)) {
                            furi_event_loop_run(app.event_loop);
                        }
                    }
                    jerry_value_free(result);
                }
                jerry_value_free(link_result);
            }
        } while(false);
        jerry_value_free(parsed_script);
        jerry_value_free(source_name);
        jerry_cleanup();

        {
            furi_mutex_acquire(instance->apps_mutex, FuriWaitForever);
            AppDict_erase(instance->apps, furi_thread_get_current());
            furi_mutex_release(instance->apps_mutex);
        }

        furi_event_loop_free(app.event_loop);
        furi_string_free(app.root_path);
        IntervalDict_clear(app.intervals);
    } while(false);
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
    return ret;
}

static JsRunner* js_runner_alloc(void) {
    JsRunner* instance = malloc(sizeof(JsRunner));
    instance->event_loop = furi_event_loop_alloc();
    instance->apps_mutex = furi_mutex_alloc(FuriMutexTypeRecursive);
    AppDict_init(instance->apps);
    furi_record_create(RECORD_JS_RUNNER, instance);
    return instance;
}

int32_t js_runner_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Service starting...");

    JsRunner* instance = js_runner_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
