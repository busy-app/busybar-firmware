#include "js_runner_i.h"
#include "js_fetch.h"
#include "js_interval.h"
#include "js_console.h"
#include "js_local_storage.h"

#define TAG "JsRunner"

JsRunnerStaticContext js_runner_static_context = {
    .app = NULL,
    .is_running = ATOMIC_FLAG_INIT,
};

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
        furi_assert(app->jrs_context);
        result = app->jrs_context;
    });
    return result;
}

bool js_runner_get_root_path(FuriString* path) {
    bool result = false;
    WITH_JS_RUNNER_APP(app, {
        if(app->root_path) {
            furi_string_set(path, app->root_path);
            result = true;
        }
    });
    return result;
}

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
        JsRunnerExecutionHandle* handle = app->execution_handle;
        JsRunnerTerminationCallback termination_callback = handle->termination_callback;
        void* callback_context = handle->termination_callback_context;
        furi_event_flag_set(app->is_idle, JS_RUNNER_APP_FLAG_IDLE);
        if(termination_callback) {
            termination_callback(callback_context);
        }
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

typedef void (*CommandQueueHandler)(JsRunnerApp* app, JsRunnerAppCommand* cmd);

static void send_comand_message(JsRunnerContextHandle* handle, const JsRunnerAppCommand* message);

static void abort_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd);
static void run_file_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd);
static void run_snippet_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd);
static void quit_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd);

static const CommandQueueHandler command_handlers[] = {
    [JsRunnerAppCommandTypeAbort] = abort_cmd_handler,
    [JsRunnerAppCommandTypeRunFile] = run_file_cmd_handler,
    [JsRunnerAppCommandTypeQuit] = quit_cmd_handler,
    [JsRunnerAppCommandTypeRunSnippet] = run_snippet_cmd_handler,
};
static_assert(COUNT_OF(command_handlers) == JsRunnerAppCommandTypeMax);

static void app_terminate_from_app_thread(JsRunnerApp* app);

static void command_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);
    JsRunnerApp* app = context;
    JsRunnerAppCommand cmd;
    furi_check(furi_message_queue_get(app->command_queue, &cmd, 0) == FuriStatusOk);
    CommandQueueHandler handler = command_handlers[cmd.type];
    furi_check(handler);
    handler(app, &cmd);
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

typedef struct AppThreadParams {
    JsRunner* instance;
    FuriString* app_id;
    size_t heap_size;
    JsRunnerConsoleOutCallback console_write_cb;
    void* console_write_context;
    FuriMessageQueue* command_queue;

    JsRunnerContextHandle** out_handle;
    FuriApiLock out_handle_lock;
} AppThreadParams;

static AppThreadParams* app_thread_params_alloc(
    JsRunner* instance,
    const char* app_id,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_write_cb,
    void* console_write_context,
    FuriMessageQueue* command_queue,
    JsRunnerContextHandle** out_handle) {
    AppThreadParams* params = malloc(sizeof(AppThreadParams));
    params->instance = instance;
    params->app_id = furi_string_alloc_set(app_id);
    params->heap_size = heap_size;
    params->console_write_cb = console_write_cb;
    params->console_write_context = console_write_context;
    params->command_queue = command_queue;

    params->out_handle = out_handle;
    params->out_handle_lock = api_lock_alloc_locked();

    return params;
}
static void app_thread_params_free(AppThreadParams* params) {
    furi_string_free(params->app_id);
    free(params);
}

static void js_runner_app_init(JsRunnerApp* app, const AppThreadParams* params) {
    JS_TRACE("app init");

    app->app_id = params->app_id;
    app->thread = furi_thread_get_current();
    app->command_queue = params->command_queue;
    app->should_terminate = false;
    atomic_flag_clear(&app->is_execution_handle_taken);
    app->execution_handle = NULL;
    app->heap_size = params->heap_size;
    app->jrs_context = NULL;
    app->event_loop = furi_event_loop_alloc();

    app->root_path = NULL;
    app->is_idle = furi_event_flag_alloc();
    furi_event_flag_set(app->is_idle, JS_RUNNER_APP_FLAG_IDLE);

    js_runner_app_console_init(
        &app->console, params->console_write_cb, params->console_write_context);
    js_runner_app_interval_init(&app->interval);
    js_runner_app_fetch_init(&app->fetch);
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->fetch.event_queue,
        FuriEventLoopEventIn,
        fetch_event_queue_callback,
        app);

    furi_event_loop_subscribe_message_queue(
        app->event_loop, app->command_queue, FuriEventLoopEventIn, command_queue_callback, app);
}

static void js_runner_app_deinit(JsRunnerApp* app) {
    JS_TRACE("app deinit");
    furi_event_loop_unsubscribe(app->event_loop, app->command_queue);
    furi_event_loop_unsubscribe(app->event_loop, app->fetch.event_queue);
    furi_event_loop_free(app->event_loop);
    if(app->root_path) {
        furi_string_free(app->root_path);
    }
    js_runner_app_interval_deinit(&app->interval);
    js_runner_app_fetch_deinit(&app->fetch);
    furi_event_flag_free(app->is_idle);
}

static void js_runner_app_set_root_path(JsRunnerApp* app, const char* script_path) {
    if(app->root_path) {
        furi_string_free(app->root_path);
        app->root_path = NULL;
    }
    if(script_path) {
        app->root_path = furi_string_alloc();
        path_extract_dirname(script_path, app->root_path);
    }
}

void js_runner_byte_array_destructor(void* object, void* user_p) {
    UNUSED(object);
    JsRunnerByteArrayDestructor* destructor = user_p;
    ByteArray_clear(*destructor->byte_array);
    free(destructor->byte_array);
    free(destructor);
}

void js_runner_heap_destructor(void* object, void* user_p) {
    UNUSED(user_p);
    free(object);
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
        JsRunnerExternalDataDestructor destructor = user_p;
        destructor(buffer_p, arraybuffer_user_p);
    }
}

static void
    external_string_free_callback(jerry_char_t* string_p, jerry_size_t string_size, void* user_p) {
    UNUSED(string_size);
    JS_TRACE("free external string");
    if(user_p) {
        JsRunnerExternalDataDestructor destructor = user_p;
        destructor(string_p, user_p);
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

static bool validate_app_id(const char* app_id) {
    while(*app_id) {
        int c = *app_id;
        if(!isalnum(c) && c != '.' && c != '_') {
            return false;
        }
        ++app_id;
    }
    return true;
}

JsRunnerContextHandle*
    context_handle_alloc(JsRunner* instance, JsRunnerApp* app, FuriMessageQueue* command_queue) {
    JsRunnerContextHandle* handle = malloc(sizeof(JsRunnerContextHandle));

    handle->instance = instance;
    handle->app = app;
    handle->command_queue = command_queue;
    handle->thread = furi_thread_get_current();

    return handle;
}

static int32_t app_thread_callback(void* context) {
    AppThreadParams* params = context;

    JsRunnerApp app;
    *params->out_handle = context_handle_alloc(params->instance, &app, params->command_queue);
    js_runner_app_init(&app, params);
    js_runner_static_context.app = &app;

    jerry_init(JERRY_INIT_EMPTY);
    jerry_halt_handler(1, engine_halt_callback, &app);
    jerry_arraybuffer_allocator(NULL, arraybuffer_free_callback, NULL);
    jerry_string_external_on_free(external_string_free_callback);

    js_setup_console(&app.console);
    js_setup_interval_methods();
    js_setup_fetch();
    js_setup_local_storage();

    api_lock_unlock(params->out_handle_lock);

    furi_event_loop_run(app.event_loop);

    jerry_cleanup();
    js_runner_app_deinit(&app);
    js_runner_static_context.app = NULL;
    atomic_flag_clear(&js_runner_static_context.is_running);

    app_thread_params_free(params);
    return 0;
}

JsRunnerContextInitResult js_runner_context_alloc(
    JsRunner* instance,
    const char* app_id,
    size_t heap_size,
    JsRunnerConsoleOutCallback console_write_cb,
    void* console_write_context) {
    if(!validate_app_id(app_id)) {
        return (JsRunnerContextInitResult){
            .error = JsRunnerErrorInvalidAppId,
            .handle = NULL,
        };
    }
    if(atomic_flag_test_and_set(&js_runner_static_context.is_running)) {
        return (JsRunnerContextInitResult){
            .error = JsRunnerErrorResource,
            .handle = NULL,
        };
    }

    FuriMessageQueue* command_queue =
        furi_message_queue_alloc(MAX_COMMAND_MESSAGES, sizeof(JsRunnerAppCommand));
    JsRunnerContextHandle* handle = NULL;
    AppThreadParams* params = app_thread_params_alloc(
        instance,
        app_id,
        heap_size,
        console_write_cb,
        console_write_context,
        command_queue,
        &handle);

    FuriThread* thread = furi_thread_alloc_ex(
        app_id ? app_id : "JsApp", APP_THREAD_STACK_SIZE, app_thread_callback, params);
    furi_thread_start(thread);

    api_lock_wait_unlock_and_free(params->out_handle_lock);

    return (JsRunnerContextInitResult){.error = JsRunnerErrorNone, .handle = handle};
}

void js_runner_context_free(JsRunnerContextHandle* handle) {
    JS_TRACE("Stopping the thread");
    if(atomic_flag_test_and_set(&handle->app->is_execution_handle_taken)) {
        furi_check(!"Execution handles must be joined before calling js_runner_context_free");
    }
    JsRunnerAppCommand cmd = {.type = JsRunnerAppCommandTypeQuit};
    furi_message_queue_put(handle->command_queue, &cmd, FuriWaitForever);
    furi_thread_join(handle->thread);
    furi_thread_free(handle->thread);
    furi_message_queue_free(handle->command_queue);
    free(handle);
}

static JsRunnerExecutionHandle* execution_handle_alloc(
    JsRunnerContextHandle* parent,
    JsRunnerTerminationCallback on_terminate,
    void* context) {
    if(atomic_flag_test_and_set(&parent->app->is_execution_handle_taken)) {
        return NULL;
    }
    JsRunnerExecutionHandle* handle = malloc(sizeof(JsRunnerExecutionHandle));
    handle->app = parent->app;
    handle->context_handle = parent;
    handle->termination_callback = on_terminate;
    handle->termination_callback_context = context;
    parent->app->execution_handle = handle;
    return handle;
}

static void execution_handle_free(JsRunnerExecutionHandle* handle) {
    handle->app->execution_handle = NULL;
    atomic_flag_clear(&handle->app->is_execution_handle_taken);
    free(handle);
}

JsRunnerRunResult js_runner_run(
    JsRunnerContextHandle* handle,
    const char* path,
    JsRunnerTerminationCallback on_terminate,
    void* context) {
    FURI_LOG_I(TAG, "Running script: %s", path);

    JsRunnerError result = JsRunnerErrorNone;
    JsRunnerExecutionHandle* exec_handle = NULL;
    do {
        exec_handle = execution_handle_alloc(handle, on_terminate, context);
        if(!exec_handle) {
            result = JsRunnerErrorResource;
            break;
        }

        JsRunnerAppCommand cmd = {
            .type = JsRunnerAppCommandTypeRunFile,
            .lock = api_lock_alloc_locked(),
            .result = &result,
            .run_file =
                {
                    .path = path,
                    .context_handle = handle,
                },
        };

        send_comand_message(handle, &cmd);
        if(result != JsRunnerErrorNone) {
            execution_handle_free(exec_handle);
            exec_handle = NULL;
        }
    } while(false);

    return (JsRunnerRunResult){
        .error = result,
        .handle = exec_handle,
    };
}

JsRunnerRunResult js_runner_run_snippet(
    JsRunnerContextHandle* handle,
    const char* code,
    bool print_result,
    JsRunnerTerminationCallback on_terminate,
    void* context) {
    JsRunnerError result = JsRunnerErrorNone;
    JsRunnerExecutionHandle* exec_handle = NULL;
    do {
        exec_handle = execution_handle_alloc(handle, on_terminate, context);
        if(!exec_handle) {
            result = JsRunnerErrorResource;
            break;
        }

        JsRunnerAppCommand cmd = {
            .type = JsRunnerAppCommandTypeRunSnippet,
            .lock = api_lock_alloc_locked(),
            .result = &result,
            .run_snippet =
                {
                    .code = code,
                    .print_result = print_result,
                    .context_handle = handle,
                },
        };

        send_comand_message(handle, &cmd);
        if(result != JsRunnerErrorNone) {
            execution_handle_free(exec_handle);
            exec_handle = NULL;
        }
    } while(false);

    return (JsRunnerRunResult){
        .error = result,
        .handle = exec_handle,
    };
}

JsRunnerError js_runner_join(JsRunnerExecutionHandle* handle, uint32_t timeout) {
    uint32_t wait_result = furi_event_flag_wait(
        handle->app->is_idle, JS_RUNNER_APP_FLAG_IDLE, FuriFlagNoClear, timeout);

    JsRunnerError result = JsRunnerErrorNone;
    if(wait_result == JS_RUNNER_APP_FLAG_IDLE) {
        handle->app->should_terminate = false;
        execution_handle_free(handle);
    } else if((FuriStatus)wait_result == FuriStatusErrorTimeout) {
        result = JsRunnerErrorTimeout;
    } else {
        furi_check(false);
    }
    return result;
}

static JsRunner* js_runner_alloc(void) {
    JsRunner* instance = malloc(sizeof(JsRunner));
    instance->event_loop = furi_event_loop_alloc();
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

static void unlock_with_result(JsRunnerAppCommand* cmd, JsRunnerError result) {
    if(cmd->result) {
        *cmd->result = result;
    }
    if(cmd->lock) {
        api_lock_unlock(cmd->lock);
    }
}

static void abort_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd) {
    furi_check(cmd->type == JsRunnerAppCommandTypeAbort);
    app_terminate_from_app_thread(app);
    js_runner_app_stop_if_done(app);
}

static void run_file_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd) {
    furi_check(cmd->type == JsRunnerAppCommandTypeRunFile);

    if((furi_event_flag_get(app->is_idle) & JS_RUNNER_APP_FLAG_IDLE) == 0) {
        unlock_with_result(cmd, JsRunnerErrorResource);
        return;
    }

    bool unlocked = false;
    JsRunnerError ret = JsRunnerErrorNone;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(storage);
    js_runner_app_set_root_path(app, cmd->run_file.path);
    do {
        if(!storage_file_open(f, cmd->run_file.path, FSAM_READ, FSOM_OPEN_EXISTING)) {
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

        FuriString* path_furi = furi_string_alloc_set_str(cmd->run_file.path);
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
                    jerry_value_free(link_result);
                    ret = JsRunnerErrorParseException;
                    break;
                } else {
                    // Evaluating the script can take an arbitrary amount of time.
                    // Unlocking to let the API caller proceed.
                    furi_event_flag_clear(app->is_idle, JS_RUNNER_APP_FLAG_IDLE);
                    unlock_with_result(cmd, JsRunnerErrorNone);
                    unlocked = true;

                    jerry_value_t result = jerry_module_evaluate(parsed_script);
                    if(jerry_value_is_exception(result)) {
                        js_log_exception(TAG, "Error running script", result);
                        app_terminate_from_app_thread(app);
                    }
                    js_run_jobs();
                    js_runner_app_stop_if_done(app);
                    jerry_value_free(result);
                }
                jerry_value_free(link_result);
            }
        } while(false);
        jerry_value_free(parsed_script);
        jerry_value_free(source_name);
    } while(false);
    js_runner_app_set_root_path(app, NULL);
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
    if(!unlocked) {
        unlock_with_result(cmd, ret);
    }
}

static void console_print(
    JsRunnerApp* app,
    JsRunnerConsoleSeverity severity,
    const char* message,
    JsRunnerConsoleSeparator separator) {
    if(app->console.callback) {
        app->console.callback(
            severity, message, strlen(message), separator, app->console.callback_context);
    }
}

static void run_snippet_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd) {
    furi_check(cmd->type == JsRunnerAppCommandTypeRunSnippet);

    if((furi_event_flag_get(app->is_idle) & JS_RUNNER_APP_FLAG_IDLE) == 0) {
        unlock_with_result(cmd, JsRunnerErrorResource);
        return;
    }

    bool unlocked = false;
    JsRunnerError ret = JsRunnerErrorNone;
    jerry_parse_options_t parse_options = {
        .options = 0,
    };
    jerry_value_t parsed_script = jerry_parse(
        (const jerry_char_t*)cmd->run_snippet.code, strlen(cmd->run_snippet.code), &parse_options);
    do {
        if(jerry_value_is_exception(parsed_script)) {
            FuriString* error = js_get_exception_string(parsed_script);
            console_print(
                app,
                JsRunnerConsoleSeverityError,
                furi_string_get_cstr(error),
                JsRunnerConsoleSeparatorNewline);
            furi_string_free(error);
            ret = JsRunnerErrorParseException;
            break;
        } else {
            furi_event_flag_clear(app->is_idle, JS_RUNNER_APP_FLAG_IDLE);
            unlock_with_result(cmd, JsRunnerErrorNone);
            unlocked = true;

            jerry_value_t result = jerry_run(parsed_script);
            if(cmd->run_snippet.print_result && !jerry_value_is_exception(result)) {
                jerry_value_t string = jerry_value_to_string(result);
                jerry_value_free(result);
                result = string;
            }
            if(jerry_value_is_exception(result)) {
                FuriString* error = js_get_exception_string(result);
                console_print(
                    app,
                    JsRunnerConsoleSeverityError,
                    furi_string_get_cstr(error),
                    JsRunnerConsoleSeparatorNewline);
                furi_string_free(error);
                app_terminate_from_app_thread(app);
            } else if(cmd->run_snippet.print_result) {
                char* str = js_string_to_c_string(result);

                console_print(
                    app, JsRunnerConsoleSeverityLog, str, JsRunnerConsoleSeparatorNewline);
                free(str);
            }
            jerry_value_free(result);
            js_run_jobs();
            js_runner_app_stop_if_done(app);
        }
    } while(false);
    jerry_value_free(parsed_script);
    if(!unlocked) {
        unlock_with_result(cmd, ret);
    }
}

static void quit_cmd_handler(JsRunnerApp* app, JsRunnerAppCommand* cmd) {
    furi_check(cmd->type == JsRunnerAppCommandTypeQuit);
    furi_check(furi_event_flag_get(app->is_idle) & JS_RUNNER_APP_FLAG_IDLE);
    furi_event_loop_stop(app->event_loop);
}

static void app_terminate_from_app_thread(JsRunnerApp* app) {
    app->should_terminate = true;
    abort_fetches(&app->fetch);
    abort_intervals(app);
}

static void app_terminate_from_another_thread(JsRunnerApp* app) {
    if(!app->should_terminate) {
        app->should_terminate = true;
        JsRunnerAppCommand cmd = {
            .type = JsRunnerAppCommandTypeAbort,
        };
        furi_message_queue_put(app->command_queue, &cmd, FuriWaitForever);
    }
}

static void send_comand_message(JsRunnerContextHandle* handle, const JsRunnerAppCommand* message) {
    furi_check(
        furi_message_queue_put(handle->command_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) {
        api_lock_wait_unlock_and_free(message->lock);
    }
}

void js_runner_abort(JsRunnerExecutionHandle* handle) {
    // Only single execution handle is available at a time: just checking if user owns it
    furi_check(handle);
    furi_check(handle->context_handle);

    JsRunnerApp* app = js_runner_static_context.app;
    app_terminate_from_another_thread(app);
}

void js_runner_abort_all(JsRunner* instance) {
    UNUSED(instance);
    if(!js_runner_static_context.app) {
        return;
    }
    app_terminate_from_another_thread(js_runner_static_context.app);
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
    [JsRunnerErrorResource] = "Out of resources",
    [JsRunnerErrorTimeout] = "Timeout",
};

static_assert(COUNT_OF(error_messages) == JsRunnerErrorMax);

const char* js_runner_get_error_message(JsRunnerError error) {
    if(error >= JsRunnerErrorMax) {
        return "Unknown";
    }
    return error_messages[error];
}
