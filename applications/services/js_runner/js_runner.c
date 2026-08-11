#include "js_runner_i.h"
#include "js_fetch.h"
#include "js_interval.h"
#include "js_console.h"

#define TAG "JsRunner"

size_t js_runner_thread_context_alloc(size_t context_size) {
    size_t alloc_size = 0;
    WITH_JS_RUNNER_APP(app, {
        furi_check(!app->jrs_context);
        alloc_size = context_size + app->heap_size;
        app->jrs_context = malloc(alloc_size);
    });
    return alloc_size;
}

void js_runner_thread_context_free(void) {
    WITH_JS_RUNNER_APP(app, {
        furi_check(app->jrs_context);
        free(app->jrs_context);
        app->jrs_context = NULL;
    });
}

void* js_runner_thread_context_get(void) {
    void* result = NULL;
    WITH_JS_RUNNER_APP(app, {
        furi_check(app->jrs_context);
        result = app->jrs_context;
    });
    return result;
}

void js_runner_get_root_path(FuriString* path) {
    WITH_JS_RUNNER_APP(app, { furi_string_set(path, app->root_path); });
}

static const jerry_object_native_info_t global_native_info = {
    .free_cb = NULL,
};

static bool app_has_background_tasks(JsRunnerApp* app) {
    return !IntervalDict_empty_p(app->interval.intervals) || app->fetch.num_threads > 0;
}

void js_runner_check_event_loop(JsRunnerApp* app) {
    if(!app_has_background_tasks(app)) {
        furi_event_loop_stop(app->event_loop);
    }
}

void js_run_jobs(void) {
    bool run = true;
    while(run) {
        jerry_value_t jobs_result = jerry_run_jobs();
        if(jerry_value_is_exception(jobs_result)) {
            FURI_LOG_E(TAG, "Exception when running jobs");
            // TODO abort event loop
            run = false;
        } else {
            run = false;
        }
        jerry_value_free(jobs_result);
    }
}

static void fetch_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);
    JsRunnerApp* app = context;
    JsFetchEvent event;
    furi_check(furi_message_queue_get(app->fetch.event_queue, &event, 0) == FuriStatusOk);
    js_fetch_process_event(&event);
    js_run_jobs();
}

static void js_runner_app_console_init(
    JsRunnerAppConsole* console,
    JsRunnerConsoleOutCallback console_out_cb,
    void* console_cb_context) {
    console->callback = console_out_cb;
    console->callback_context = console_cb_context;
}

static void js_runner_app_interval_init(JsRunnerAppInterval* interval) {
    interval->last_id = 0;
    IntervalDict_init(interval->intervals);
}

static void js_runner_app_interval_deinit(JsRunnerAppInterval* interval) {
    IntervalDict_clear(interval->intervals);
}

static void js_runner_app_fetch_init(JsRunnerAppFetch* fetch) {
    fetch->num_threads = 0;
    fetch->event_queue = furi_message_queue_alloc(MAX_FETCH_MESSAGES, sizeof(JsFetchEvent));
}

static void js_runner_app_fetch_deinit(JsRunnerAppFetch* fetch) {
    furi_check(fetch->num_threads == 0);
    furi_message_queue_free(fetch->event_queue);
}

static void js_runner_app_init(
    JsRunnerApp* app,
    const char* script_path,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_out_cb,
    void* console_cb_context) {
    app->heap_size = heap_size;
    app->jrs_context = NULL;
    app->event_loop = furi_event_loop_alloc();

    app->root_path = furi_string_alloc();
    path_extract_dirname(script_path, app->root_path);
    js_runner_app_console_init(&app->console, console_out_cb, console_cb_context);
    js_runner_app_interval_init(&app->interval);
    js_runner_app_fetch_init(&app->fetch);
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->fetch.event_queue,
        FuriEventLoopEventIn,
        fetch_event_queue_callback,
        app);
}

static void js_runner_app_deinit(JsRunnerApp* app) {
    JS_TRACE("app deinit");
    furi_event_loop_unsubscribe(app->event_loop, app->fetch.event_queue);
    furi_event_loop_free(app->event_loop);
    furi_string_free(app->root_path);
    js_runner_app_interval_deinit(&app->interval);
    js_runner_app_fetch_deinit(&app->fetch);
}

static void arraybuffer_free_callback(
    jerry_arraybuffer_type_t buffer_type,
    uint8_t* buffer_p,
    uint32_t buffer_size,
    void* arraybuffer_user_p,
    void* user_p) {
    UNUSED(buffer_type);
    UNUSED(buffer_size);
    UNUSED(user_p);
    JS_TRACE("free arraybuffer");
    if(arraybuffer_user_p) {
        ByteArray_t* array = arraybuffer_user_p;
        ByteArray_clear(*array);
        free(array);
    } else {
        free(buffer_p);
    }
}

static void
    external_string_free_callback(jerry_char_t* string_p, jerry_size_t string_size, void* user_p) {
    UNUSED(string_size);
    JS_TRACE("free external string");
    if(user_p) {
        ByteArray_t* array = user_p;
        ByteArray_clear(*array);
        free(array);
    } else {
        free(string_p);
    }
}

void js_runner_add_fetch_thread(JsRunnerApp* app) {
    app->fetch.num_threads += 1;
}

void js_runner_del_fetch_thread(JsRunnerApp* app) {
    app->fetch.num_threads -= 1;
    js_runner_check_event_loop(app);
}

JsRunnerError js_runner_run(
    JsRunner* instance,
    const char* path,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_out_cb,
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
        if((file_size == 0) || (file_size > JS_RUNNER_MAX_SCRIPT_SIZE)) {
            ret = JsRunnerErrorInvalidFileSize;
            break;
        }
        char* buf = malloc(file_size);
        if(storage_file_read(f, buf, file_size) != file_size) {
            ret = JsRunnerErrorCannotReadFile;
            free(buf);
            break;
        }

        JsRunnerApp app;
        js_runner_app_init(&app, path, heap_size, console_out_cb, console_write_context);

        {
            furi_mutex_acquire(instance->apps_mutex, FuriWaitForever);
            AppDict_set_at(instance->apps, furi_thread_get_current(), &app);
            furi_mutex_release(instance->apps_mutex);
        }

        jerry_init(JERRY_INIT_EMPTY);
        jerry_arraybuffer_allocator(NULL, arraybuffer_free_callback, NULL);
        jerry_string_external_on_free(external_string_free_callback);

        {
            jerry_value_t global_obj = jerry_current_realm();
            jerry_object_set_native_ptr(global_obj, &global_native_info, instance);
            jerry_value_free(global_obj);
        }
        js_setup_console(&app.console);
        js_setup_interval_methods();
        js_setup_fetch();

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
                js_log_exception(TAG, "Error parsing script", parsed_script);
                ret = JsRunnerParseException;
                break;
            } else {
                jerry_value_t link_result = jerry_module_link(parsed_script, NULL, NULL);
                if(jerry_value_is_exception(link_result)) {
                    js_log_exception(TAG, "Error linking modules", link_result);
                } else {
                    jerry_value_t result = jerry_module_evaluate(parsed_script);
                    if(jerry_value_is_exception(result)) {
                        js_log_exception(TAG, "Error running script", result);
                        // TODO terminate background tasks
                        furi_check(false);
                    }
                    js_run_jobs();
                    if(app_has_background_tasks(&app)) {
                        furi_event_loop_run(app.event_loop);
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

        js_runner_app_deinit(&app);
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
