#include "js_runner_i.h"
#include "js_fetch.h"
#include "js_interval.h"
#include "js_console.h"
#include "js_local_storage.h"

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

static bool has_active_fetch(JsRunnerAppFetch* instance) {
    for(size_t i = 0; i != FetchArray_size(instance->fetches); ++i) {
        if(*FetchArray_cget(instance->fetches, i)) {
            return true;
        }
    }
    return false;
}

static bool has_active_interval(JsRunnerAppInterval* instance) {
    return !IntervalDict_empty_p(instance->intervals);
}

static bool app_has_background_tasks(JsRunnerApp* app) {
    return has_active_interval(&app->interval) || has_active_fetch(&app->fetch);
}

void js_runner_app_stop_if_done(JsRunnerApp* app) {
    if(!app_has_background_tasks(app)) {
        JS_TRACE("No more tasks");
        furi_event_loop_stop(app->event_loop);
    }
}

void js_run_jobs(void) {
    bool run = true;
    while(run) {
        jerry_value_t jobs_result = jerry_run_jobs();
        if(jerry_value_is_exception(jobs_result)) {
            FURI_LOG_E(TAG, "Exception when running jobs");
            if(jerry_value_is_abort(jobs_result)) {
                FURI_LOG_E(TAG, "Must terminate");
            }
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

typedef void (*CommandQueueHandler)(JsRunnerApp* app, JsRunnerAppCommandType cmd);

static void abort_cmd_handler(JsRunnerApp* app, JsRunnerAppCommandType cmd);

static const CommandQueueHandler command_handlers[] = {
    [JsRunnerAppCommandTypeAbort] = abort_cmd_handler,
};
static_assert(COUNT_OF(command_handlers) == JsRunnerAppCommandTypeMax);

static void app_terminate_from_app_thread(JsRunnerApp* app);

static void command_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);
    JsRunnerApp* app = context;
    JsRunnerAppCommandType cmd;
    furi_check(furi_message_queue_get(app->command_queue, &cmd, 0) == FuriStatusOk);
    CommandQueueHandler handler = command_handlers[cmd];
    furi_check(handler);
    handler(app, cmd);
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
    FetchArray_init(fetch->fetches);
    fetch->event_queue = furi_message_queue_alloc(MAX_FETCH_MESSAGES, sizeof(JsFetchEvent));
}

static void js_runner_app_fetch_deinit(JsRunnerAppFetch* fetch) {
    furi_check(!has_active_fetch(fetch));

    FetchArray_clear(fetch->fetches);
    furi_message_queue_free(fetch->event_queue);
}

static void js_runner_app_init(
    JsRunnerApp* app,
    const char* app_id,
    const char* script_path,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_out_cb,
    void* console_cb_context) {
    app->app_id = furi_string_alloc_set(app_id);
    app->should_terminate = false;
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

    app->command_queue =
        furi_message_queue_alloc(MAX_COMMAND_MESSAGES, sizeof(JsRunnerAppCommandType));
    furi_event_loop_subscribe_message_queue(
        app->event_loop, app->command_queue, FuriEventLoopEventIn, command_queue_callback, app);
}

static void js_runner_app_deinit(JsRunnerApp* app) {
    JS_TRACE("app deinit");
    furi_string_free(app->app_id);
    furi_event_loop_unsubscribe(app->event_loop, app->command_queue);
    furi_message_queue_free(app->command_queue);
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

static jerry_value_t engine_halt_callback(void* user_p) {
    JsRunnerApp* app = user_p;
    if(app->should_terminate) {
        JS_TRACE("terminate!");
        return jerry_string_sz("aborted");
    } else {
        return jerry_undefined();
    }
}

void js_runner_add_fetch_thread(JsRunnerApp* app, JsFetch* fetch) {
    JS_TRACE("Add fetch thread");
    for(size_t i = 0; i != FetchArray_size(app->fetch.fetches); ++i) {
        JsFetch** cell = FetchArray_get(app->fetch.fetches, i);
        if(*cell == NULL) {
            *cell = fetch;
            return;
        }
    }
    FetchArray_push_back(app->fetch.fetches, fetch);
}

void js_runner_del_fetch_thread(JsRunnerApp* app, JsFetch* fetch) {
    bool found = false;
    JS_TRACE("Delete fetch thread");
    for(size_t i = 0; i != FetchArray_size(app->fetch.fetches); ++i) {
        JsFetch** cell = FetchArray_get(app->fetch.fetches, i);
        if(*cell == fetch) {
            *cell = NULL;
            found = true;
            break;
        }
    }
    furi_check(found);
    js_runner_app_stop_if_done(app);
}

const char* js_runner_app_get_id(const JsRunnerApp* app) {
    return furi_string_get_cstr(app->app_id);
}

bool validate_app_id(const char* app_id) {
    while(*app_id) {
        int c = *app_id;
        if(!isalnum(c) && c != '.' && c != '_') {
            return false;
        }
        ++app_id;
    }
    return true;
}

JsRunnerError js_runner_run(
    JsRunner* instance,
    const char* app_id,
    const char* path,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_out_cb,
    void* console_write_context) {
    if(!validate_app_id(app_id)) {
        return JsRunnerErrorInvalidAppId;
    }
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
        js_runner_app_init(&app, app_id, path, heap_size, console_out_cb, console_write_context);

        {
            furi_check(furi_mutex_acquire(instance->apps_mutex, FuriWaitForever) == FuriStatusOk);
            AppDict_set_at(instance->apps, furi_thread_get_current(), &app);
            furi_check(furi_mutex_release(instance->apps_mutex) == FuriStatusOk);
        }

        jerry_init(JERRY_INIT_EMPTY);
        jerry_halt_handler(1, engine_halt_callback, &app);
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
        js_setup_local_storage();

        FuriString* path_furi = furi_string_alloc_set_str(path);
        FuriString* filename_furi = furi_string_alloc();
        path_extract_filename(path_furi, filename_furi, false);
        jerry_value_t source_name = js_utf8_string(filename_furi);
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
                ret = JsRunnerErrorParseException;
                break;
            } else {
                jerry_value_t link_result = jerry_module_link(parsed_script, NULL, NULL);
                if(jerry_value_is_exception(link_result)) {
                    js_log_exception(TAG, "Error linking modules", link_result);
                } else {
                    jerry_value_t result = jerry_module_evaluate(parsed_script);
                    if(jerry_value_is_exception(result)) {
                        js_log_exception(TAG, "Error running script", result);
                        app_terminate_from_app_thread(&app);
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
            furi_check(furi_mutex_acquire(instance->apps_mutex, FuriWaitForever) == FuriStatusOk);
            AppDict_erase(instance->apps, furi_thread_get_current());
            furi_check(furi_mutex_release(instance->apps_mutex) == FuriStatusOk);
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

static void abort_fetches(JsRunnerAppFetch* instance) {
    for(size_t i = 0; i != FetchArray_size(instance->fetches); ++i) {
        JsFetch* fetch = *FetchArray_cget(instance->fetches, i);
        if(fetch) {
            js_fetch_abort(fetch);
        }
    }
}

static void abort_intervals(JsRunnerApp* app) {
    JS_TRACE("Delete fetch thread");
    while(!IntervalDict_empty_p(app->interval.intervals)) {
        IntervalDict_it_t iter;
        IntervalDict_it(iter, app->interval.intervals);
        uint32_t id = IntervalDict_ref(iter)->key;
        js_interval_abort(app, id);
    }
}

static void abort_cmd_handler(JsRunnerApp* app, JsRunnerAppCommandType cmd) {
    furi_check(cmd == JsRunnerAppCommandTypeAbort);
    app_terminate_from_app_thread(app);
    js_runner_app_stop_if_done(app);
}

static void app_terminate_from_app_thread(JsRunnerApp* app) {
    app->should_terminate = true;
    abort_fetches(&app->fetch);
    abort_intervals(app);
}

static void app_terminate_from_another_thread(JsRunnerApp* app) {
    if(!app->should_terminate) {
        app->should_terminate = true;
        JsRunnerAppCommandType cmd = JsRunnerAppCommandTypeAbort;
        furi_message_queue_put(app->command_queue, &cmd, FuriWaitForever);
    }
}

bool js_runner_abort(JsRunner* instance, FuriThread* thread) {
    bool result = false;
    furi_check(furi_mutex_acquire(instance->apps_mutex, FuriWaitForever) == FuriStatusOk);
    JsRunnerApp** app_ptr = AppDict_get(instance->apps, thread);
    if(app_ptr) {
        app_terminate_from_another_thread(*app_ptr);
        result = true;
    }
    furi_check(furi_mutex_release(instance->apps_mutex) == FuriStatusOk);
    return result;
}

void js_runner_abort_all(JsRunner* instance) {
    furi_check(furi_mutex_acquire(instance->apps_mutex, FuriWaitForever) == FuriStatusOk);
    AppDict_it_t iter;
    for(AppDict_it(iter, instance->apps); !AppDict_end_p(iter); AppDict_next(iter)) {
        JsRunnerApp* app = AppDict_cref(iter)->value;
        app_terminate_from_another_thread(app);
    }
    furi_check(furi_mutex_release(instance->apps_mutex) == FuriStatusOk);
}

int32_t js_runner_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Service starting...");

    JsRunner* instance = js_runner_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const char* const error_messages[] = {
    [JsRunnerErrorNone] = "OK",
    [JsRunnerErrorCannotOpenFile] = "Cannot open file",
    [JsRunnerErrorInvalidFileSize] = "Invalid file size",
    [JsRunnerErrorCannotReadFile] = "Cannot read file",
    [JsRunnerErrorParseException] = "Parse exception",
    [JsRunnerErrorInvalidAppId] = "Invalid App ID",
};

static_assert(COUNT_OF(error_messages) == JsRunnerErrorMax);

const char* js_runner_get_error_message(JsRunnerError error) {
    if(error >= JsRunnerErrorMax) {
        return "Unknown";
    }
    return error_messages[error];
}
