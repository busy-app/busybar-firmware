#include "cli_worker.h"
#include <furi.h>

#define TAG "Cli Worker"

#define BUF_TX                              1024
#define THREAD_INPUT_STACK_SIZE             1024
#define THREAD_PROCESS_APP_STACK_SIZE       3 * 1024
#define CLI_WORKER_TIMEOUT_BETWEEN_MESSAGES 500
#define CLI_WORKER_RX_BUFFER_SIZE           2048
#define CLI_WORKER_TX_BUFFER_SIZE           BUF_TX

#define MESSAGE_MAX_LEN 128

typedef enum {
    CliEventNoEvent,
    CliEventStartApp,
    CliEventExitApp,
    CliEventInputData,
    CliEventRXData,
    //CliEventNewMessage,
} CliEventType;

typedef struct {
    CliEventType event;
    char c;
} CliEvent;

struct CliWorker {
    FuriThread* thread_input;
    FuriThread* thread_process_app;
    FuriString* workspace_name;
    volatile bool worker_running;
    volatile bool worker_stoping;
    FuriSemaphore* semaphore_process_app;
    FuriMessageQueue* event_queue;
    //uint32_t last_time_rx_data;
    FuriStreamBuffer* rx_stream;
    FuriStreamBuffer* tx_stream;
    uint8_t data_tx[BUF_TX];
    Cli* cli;

    CliWorkerAppStartCallback app_start_callback;
    CliWorkerAppProcessCallback app_process_callback;
    CliWorkerAppExitCallback app_exit_callback;
    void* app_handle;
};

static size_t cli_worker_available(CliWorker* instance) {
    furi_assert(instance);
    return furi_stream_buffer_bytes_available(instance->rx_stream);
}

static size_t cli_worker_read(CliWorker* instance, uint8_t* data, size_t size) {
    furi_assert(instance);
    return furi_stream_buffer_receive(instance->rx_stream, data, size, 0);
}

static bool cli_worker_write(CliWorker* instance, uint8_t* data, size_t size) {
    furi_assert(instance);
    UNUSED(data);
    UNUSED(size);

    furi_stream_buffer_send(instance->tx_stream, data, size, FuriWaitForever);
    furi_semaphore_release(instance->semaphore_process_app);
    return true;
}

static void cli_worker_put_event(CliWorker* instance, CliEvent* event) {
    furi_assert(instance);
    furi_message_queue_put(instance->event_queue, event, FuriWaitForever);
}

static CliEvent cli_worker_get_event(CliWorker* instance) {
    furi_assert(instance);
    CliEvent event;
    if(furi_message_queue_get(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        return event;
    } else {
        event.event = CliEventNoEvent;
        return event;
    }
}

static void cli_worker_process(CliWorker* worker) {
    printf("\033[0;34m%s worker started\r\n", furi_string_get_cstr(worker->workspace_name));
    printf("Press CTRL+C to stop\033[0m\r\n");

    CliEvent worker_event;
    bool exit = false;
    uint8_t message[MESSAGE_MAX_LEN] = {0};
    FuriString* input = furi_string_alloc();
    FuriString* sysmsg = furi_string_alloc();
    FuriString* output = furi_string_alloc();

    if(furi_string_utf8_length(worker->workspace_name)) {
        furi_string_printf(output, "%s", furi_string_get_cstr(worker->workspace_name));
        furi_string_printf(
            worker->workspace_name, "\033[0;33m%s\033[0m>: ", furi_string_get_cstr(output));
    }
    furi_string_printf(output, "%s", furi_string_get_cstr(worker->workspace_name));
    furi_string_set(input, worker->workspace_name);
    printf("%s", furi_string_get_cstr(input));
    fflush(stdout);

    while(!exit) {
        worker_event = cli_worker_get_event(worker);
        switch(worker_event.event) {
        case CliEventInputData:
        if(worker_event.c == 0x00){
            //do nothing
        } else if(worker_event.c == CliSymbolAsciiETX) {
                printf("\r\n");
                worker_event.event = CliEventExitApp;
                cli_worker_put_event(worker, &worker_event);
                break;
            } else if(
                (worker_event.c == CliSymbolAsciiBackspace) ||
                (worker_event.c == CliSymbolAsciiDel)) {
                size_t len = furi_string_utf8_length(input);
                if(len > furi_string_utf8_length(worker->workspace_name)) {
                    printf("%s", "\e[D\e[1P");
                    fflush(stdout);
                    //delete 1 char UTF
                    const char* str = furi_string_get_cstr(input);
                    size_t size = 0;
                    FuriStringUTF8State s = FuriStringUTF8StateStarting;
                    FuriStringUnicodeValue u = 0;
                    furi_string_reset(sysmsg);
                    while(*str) {
                        furi_string_utf8_decode(*str, &s, &u);
                        if((s == FuriStringUTF8StateError) || s == FuriStringUTF8StateStarting) {
                            furi_string_utf8_push(sysmsg, u);
                            if(++size >= len - 1) break;
                            s = FuriStringUTF8StateStarting;
                        }
                        str++;
                    }
                    furi_string_set(input, sysmsg);
                }
            } else if(worker_event.c == CliSymbolAsciiCR) {
                printf("\r\n");
                furi_string_right(input, furi_string_utf8_length(worker->workspace_name));
                while(!cli_worker_write(
                    worker,
                    (uint8_t*)furi_string_get_cstr(input),
                    strlen(furi_string_get_cstr(input)))) {
                    furi_delay_ms(10);
                }

                furi_string_printf(input, "%s", furi_string_get_cstr(worker->workspace_name));
                printf("%s", furi_string_get_cstr(input));
                fflush(stdout);
            } else if(worker_event.c == CliSymbolAsciiLF) {
                //cut out the symbol \n
            } else {
                putc(worker_event.c, stdout);
                fflush(stdout);
                furi_string_push_back(input, worker_event.c);
            }
            break;
        case CliEventRXData:
            do {
                memset(message, 0x00, MESSAGE_MAX_LEN);
                size_t len = cli_worker_read(worker, message, MESSAGE_MAX_LEN);
                for(size_t i = 0; i < len; i++) {
                    furi_string_push_back(output, message[i]);
                    if(message[i] == '\n') {
                        printf("\r");
                        for(uint8_t i = 0; i < 80; i++) {
                            printf(" ");
                        }
                        printf("\r%s", furi_string_get_cstr(output));
                        printf("%s", furi_string_get_cstr(input));
                        fflush(stdout);
                        furi_string_printf(
                            output, "%s", furi_string_get_cstr(worker->workspace_name));
                    }
                }
            } while(cli_worker_available(worker));
            break;
        // case CliEventNewMessage:
        //     //notification_message(notification, &sequence_single_vibro);
        //     break;
        case CliEventStartApp:
            //init start message
            cli_worker_write(
                worker,
                (uint8_t*)furi_string_get_cstr(sysmsg),
                strlen(furi_string_get_cstr(sysmsg)));
            break;
        case CliEventExitApp:
            exit = true;
            break;
        default:
            FURI_LOG_W(TAG, "Error event");
            break;
        }

        if(!cli_is_connected(worker->cli)) {
            printf("\r\n");
            worker_event.event = CliEventExitApp;
            cli_worker_put_event(worker, &worker_event);
        }
    }

    furi_string_free(output);
    furi_string_free(sysmsg);
    furi_string_free(input);
}

static int32_t cli_worker_thread_process_app(void* context) {
    CliWorker* instance = context;
    while(instance->worker_running) {
        furi_semaphore_acquire(instance->semaphore_process_app, FuriWaitForever);

        size_t len = furi_stream_buffer_receive(instance->tx_stream, instance->data_tx, BUF_TX, 0);
        if(len > 0) {
            instance->app_process_callback(instance->app_handle, instance->data_tx, len);
        }
    }
    return 0;
}

static int32_t cli_worker_thread(void* context) {
    CliWorker* instance = context;
    FURI_LOG_I(TAG, "Start");
    char c;

    furi_thread_start(instance->thread_process_app);

    CliEvent event;
    event.event = CliEventStartApp;
    furi_message_queue_put(instance->event_queue, &event, 0);
    while(instance->worker_running) {
        if(cli_read_timeout(instance->cli, (uint8_t*)&c, 1, 100) == 1) {
            event.event = CliEventInputData;
            event.c = c;
            furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
        }
    }
    furi_semaphore_release(instance->semaphore_process_app);
    furi_thread_join(instance->thread_process_app);
    FURI_LOG_I(TAG, "Stop");
    return 0;
}

static void cli_worker_update_rx_event(void* context) {
    furi_assert(context);
    CliWorker* instance = context;
    CliEvent event;
    // if((furi_get_tick() - instance->last_time_rx_data) >
    //    CLI_WORKER_TIMEOUT_BETWEEN_MESSAGES) {
    //     event.event = CliEventNewMessage;
    //     furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
    // }
    // instance->last_time_rx_data = furi_get_tick();
    event.event = CliEventRXData;
    furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
}

CliWorker* cli_worker_alloc(const char* workspace_name, Cli* cli) {
    CliWorker* instance = malloc(sizeof(CliWorker));

    instance->cli = cli;

    instance->thread_input =
        furi_thread_alloc_ex("Cli_Worker", THREAD_INPUT_STACK_SIZE, cli_worker_thread, instance);
    instance->thread_process_app = furi_thread_alloc_ex(
        "Cli_Worker_App", THREAD_PROCESS_APP_STACK_SIZE, cli_worker_thread_process_app, instance);
    instance->semaphore_process_app = furi_semaphore_alloc(1, 0);
    instance->event_queue = furi_message_queue_alloc(80, sizeof(CliEvent));
    instance->rx_stream = furi_stream_buffer_alloc(CLI_WORKER_RX_BUFFER_SIZE, 1);
    instance->tx_stream = furi_stream_buffer_alloc(CLI_WORKER_TX_BUFFER_SIZE, 1);
    instance->workspace_name = furi_string_alloc_printf("%s", workspace_name);
    return instance;
}

void cli_worker_free(CliWorker* instance) {
    furi_check(instance);
    furi_check(!instance->worker_running);
    furi_stream_buffer_free(instance->rx_stream);
    furi_stream_buffer_free(instance->tx_stream);
    furi_message_queue_free(instance->event_queue);
    furi_semaphore_free(instance->semaphore_process_app);
    furi_thread_free(instance->thread_process_app);
    furi_thread_free(instance->thread_input);
    furi_string_free(instance->workspace_name);

    free(instance);
}

bool cli_worker_start(CliWorker* instance) {
    furi_check(instance);
    furi_check(!instance->worker_running);
    furi_message_queue_reset(instance->event_queue);
    instance->worker_running = true;
    //instance->last_time_rx_data = 0;
    furi_thread_start(instance->thread_input);
    furi_check(
        instance->app_start_callback && instance->app_process_callback &&
        instance->app_exit_callback);
    instance->app_handle = instance->app_start_callback(instance);
    return instance->app_handle != NULL;
}

void cli_worker_run(CliWorker* instance) {
    furi_check(instance);
    furi_check(instance->worker_running);
    cli_worker_process(instance);
}

void cli_worker_stop(CliWorker* instance) {
    furi_check(instance);
    furi_check(instance->worker_running);

    instance->app_exit_callback(instance->app_handle);

    instance->worker_running = false;
    furi_thread_join(instance->thread_input);
}

bool cli_worker_is_running(CliWorker* instance) {
    furi_check(instance);
    return instance->worker_running;
}

size_t cli_worker_add_rx_data(CliWorker* instance, uint8_t* data, size_t size) {
    furi_assert(instance);
    size_t len = furi_stream_buffer_send(instance->rx_stream, data, size, 100);
    cli_worker_update_rx_event(instance);
    return len;
}

void cli_worker_set_callback(
    CliWorker* instance,
    CliWorkerAppStartCallback app_start_callback,
    CliWorkerAppProcessCallback app_process_callback,
    CliWorkerAppExitCallback app_exit_callback) {
    furi_check(instance);
    furi_check(app_start_callback);
    furi_check(app_process_callback);
    furi_check(app_exit_callback);
    instance->app_start_callback = app_start_callback;
    instance->app_process_callback = app_process_callback;
    instance->app_exit_callback = app_exit_callback;
}
