#include "cli_intercom.h"
#include "cli_intercom_common_i.h"
#include <intercom/intercom.h>
#include <toolbox/api_lock.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_main_shell.h>
#include <cli/cli_commands.h>

#define TAG "CliIntercom"
// #define CLI_INTERCOM_TRACE_ENABLE

#define CLI_INTERCOM_TIMEOUT    (100UL)
#define CLI_INTERCOM_TX_TIMEOUT (2000UL)
#define CLI_INTERCOM_TX_STACK   (1024UL)

#define PIPE_SZ_PER_DIRECTION 1024
#define MSG_Q_SIZE            4
#define RX_STREAM_SIZE        (4 * 1024)
#define DATA_BATCH_SIZE       (2 * CLI_INTERCOM_MAX_PAYLOAD_LEN)

#ifdef CLI_INTERCOM_TRACE_ENABLE
#define CLI_INTERCOM_TRACE(...) FURI_LOG_T(__VA_ARGS__)
#else
#define CLI_INTERCOM_TRACE(...)
#endif

struct CliIntercom {
    IntercomChannel* intercom_ch;
    CliRegistry* registry;
    FuriMessageQueue* msg_queue;
    FuriStreamBuffer* intercom_rx_stream;
    FuriEventLoop* event_loop;

    CliShell* cli_shell;
    PipeSide* own_pipe;
    bool close_pipe_on_detach;

    FuriThread* tx_thread;
    FuriSemaphore* tx_data_available;
    volatile bool tx_shutdown;

    FuriApiLock join_lock;
};

typedef enum {
    // Protocol events:
    CliIntercomInternalEventTypeProtocolSpawn,
    CliIntercomInternalEventTypeProtocolDisconnect,
    // Public API events:
    CliIntercomInternalEventTypeApiSpawn,
    CliIntercomInternalEventTypeApiJoin,
    // Internal events:
    CliIntercomInternalEventTypeIntercomDesync,
} CliIntercomInternalEventType;

typedef struct {
    CliIntercomInternalEventType type;
    FuriApiLock api_lock;
    union {
        struct {
            PipeSide* pipe;
            CliIntercomSpawnStatus* spawn_status;
            bool close_pipe_on_detach;
        };
    };
} CliIntercomInternalEvent;

static void cli_intercom_pipe_broken(PipeSide* pipe, void* context);
static void cli_intercom_data_from_pipe(PipeSide* pipe, void* context);
static void cli_intercom_drain_rx_to_pipe(CliIntercom* cli_intercom);
static int32_t cli_intercom_tx_worker(void* context);

// =================
// Protocol handling
// =================

// Return true on success
static bool cli_intercom_send_protocol(
    CliIntercom* cli_intercom,
    const uint8_t* data,
    size_t size,
    uint32_t timeout) {
    size_t tx_bytes =
        intercom_tx(cli_intercom->intercom_ch, data, size, timeout ? timeout : FuriWaitForever);
    return tx_bytes == size;
}

static bool cli_intercom_send_protocol_status(
    CliIntercom* cli_intercom,
    CliIntercomMessageType type,
    uint32_t timeout) {
    CLI_INTERCOM_TRACE(TAG, "OutStatus type=%d", type);
    furi_assert(type < CliIntercomMessageTypeStatusMAX);
    uint8_t message[] = {
        cli_intercom_construct_status(type, 0),
    };
    return cli_intercom_send_protocol(cli_intercom, message, sizeof(message), timeout);
}

static bool cli_intercom_send_protocol_payload(
    CliIntercom* cli_intercom,
    CliIntercomMessageType type,
    const uint8_t* payload,
    size_t payload_size,
    uint32_t timeout) {
    CLI_INTERCOM_TRACE(TAG, "OutPayload type=%d len=%zu", type, payload_size);
    furi_assert(type < CliIntercomMessageTypeMAX);
    furi_assert(type >= CliIntercomMessageTypeStatusMAX);
    uint8_t message[payload_size + 1];
    message[0] = cli_intercom_construct_status(type, payload_size);
    memcpy(message + 1, payload, payload_size);
    return cli_intercom_send_protocol(cli_intercom, message, sizeof(message), timeout);
}

static void
    cli_intercom_send_simple_event(CliIntercom* cli_intercom, CliIntercomInternalEventType type) {
    CliIntercomInternalEvent event = {
        .type = type,
    };
    furi_check(furi_message_queue_put(cli_intercom->msg_queue, &event, 0) == FuriStatusOk);
}

static void cli_intercom_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    CliIntercom* cli_intercom = context;
    const uint8_t* buffer = data;

    uint8_t status = buffer[0];
    CliIntercomMessageType msg_type;
    uint8_t payload_length;
    cli_intercom_parse_status(status, &msg_type, &payload_length);
    furi_assert(data_size == payload_length + 1U);

    CLI_INTERCOM_TRACE(TAG, "In type=%d len=%zu", msg_type, payload_length);

    switch(msg_type) {
    case CliIntercomMessageTypeSpawn:
        cli_intercom_send_simple_event(cli_intercom, CliIntercomInternalEventTypeProtocolSpawn);
        break;

    case CliIntercomMessageTypeDisconnect:
        cli_intercom_send_simple_event(
            cli_intercom, CliIntercomInternalEventTypeProtocolDisconnect);
        break;

    case CliIntercomMessageTypeData: {
        const uint8_t* payload = buffer + 1;
        while(payload_length) {
            size_t sent = furi_stream_buffer_send(
                cli_intercom->intercom_rx_stream, payload, payload_length, FuriWaitForever);
            payload += sent;
            payload_length -= sent;
        }
        break;

    default:
        furi_crash();
    }
    }
}

// ===============
// Service helpers
// ===============

static void cli_intercom_pipe_space_freed(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliIntercom* cli_intercom = context;
    cli_intercom_drain_rx_to_pipe(cli_intercom);
}

static void cli_intercom_attach_own_pipe(CliIntercom* cli_intercom, PipeSide* pipe) {
    furi_check(!cli_intercom->own_pipe);
    cli_intercom->own_pipe = pipe;
    pipe_attach_to_event_loop(pipe, cli_intercom->event_loop);
    pipe_set_callback_context(pipe, cli_intercom);
    pipe_set_data_arrived_callback(pipe, cli_intercom_data_from_pipe, FuriEventLoopEventFlagEdge);
    pipe_set_space_freed_callback(pipe, cli_intercom_pipe_space_freed, FuriEventLoopEventFlagEdge);
    pipe_set_broken_callback(pipe, cli_intercom_pipe_broken, FuriEventLoopEventFlagEdge);

    // Start the TX worker thread (sole pipe reader, handles blocking intercom_tx)
    cli_intercom->tx_shutdown = false;
    cli_intercom->tx_thread = furi_thread_alloc_ex(
        "CliIntercomTx", CLI_INTERCOM_TX_STACK, cli_intercom_tx_worker, cli_intercom);
    furi_thread_start(cli_intercom->tx_thread);

    // Drain any data that arrived in the stream buffer before the pipe was attached
    cli_intercom_drain_rx_to_pipe(cli_intercom);

    // Kick the TX thread in case the pipe already has data
    furi_semaphore_release(cli_intercom->tx_data_available);
}

static void cli_intercom_detach_own_pipe(CliIntercom* cli_intercom) {
    furi_check(cli_intercom->own_pipe);

    pipe_detach_from_event_loop(cli_intercom->own_pipe);
#ifdef CLI_INTERCOM_SLAVE
    // on SLAVE (917), own_pipe is created by us in do_protocol_spawn
    pipe_free(cli_intercom->own_pipe);
#else
    // on MASTER (U5), own_pipe is provided externally via cli_intercom_spawn.
    // Only close/break the pipe if the caller requested it (e.g. programmatic
    // callers blocked in pipe_copy_until that need to be unblocked on disconnect).
    // Interactive callers (sl_cli) pass their outer shell pipe and need it intact.
    if(cli_intercom->close_pipe_on_detach) {
        pipe_close(cli_intercom->own_pipe);
    }
#endif
    cli_intercom->own_pipe = NULL;
}

static void cli_intercom_free_shell(CliIntercom* cli_intercom) {
#ifdef CLI_INTERCOM_SLAVE
    furi_check(cli_intercom->cli_shell);
    cli_shell_join(cli_intercom->cli_shell);
    cli_shell_free(cli_intercom->cli_shell);
    cli_intercom->cli_shell = NULL;
#else
    UNUSED(cli_intercom);
#endif
}

static void cli_intercom_do_protocol_spawn(CliIntercom* cli_intercom) {
    FURI_LOG_D(TAG, "ProtocolSpawn");

#ifdef CLI_INTERCOM_MASTER
    furi_crash(); // can't spawn a shell on f20
#endif
    furi_check(!cli_intercom->own_pipe);
    furi_check(!cli_intercom->cli_shell);

    PipeSideBundle bundle = pipe_alloc(PIPE_SZ_PER_DIRECTION, 1);
    PipeSide* shell_pipe = bundle.bobs_side;

    cli_intercom_attach_own_pipe(cli_intercom, bundle.alices_side);

    cli_intercom->cli_shell =
        cli_shell_alloc(cli_main_motd, NULL, shell_pipe, cli_intercom->registry, NULL);
    cli_shell_free_pipe_on_exit(cli_intercom->cli_shell);
    cli_shell_set_prompt(cli_intercom->cli_shell, "917");
    cli_shell_start(cli_intercom->cli_shell);
}

static void cli_intercom_handle_disconnect(CliIntercom* cli_intercom) {
    // Stop the TX worker thread first (it reads from the pipe)
    if(cli_intercom->tx_thread) {
        cli_intercom->tx_shutdown = true;
        furi_semaphore_release(cli_intercom->tx_data_available);
        furi_thread_join(cli_intercom->tx_thread);
        furi_thread_free(cli_intercom->tx_thread);
        cli_intercom->tx_thread = NULL;
    }

    if(cli_intercom->own_pipe) {
        cli_intercom_detach_own_pipe(cli_intercom);
    }

    if(cli_intercom->intercom_rx_stream) {
        furi_stream_buffer_reset(cli_intercom->intercom_rx_stream);
    }

    if(cli_intercom->join_lock) {
        api_lock_unlock(cli_intercom->join_lock);
        cli_intercom->join_lock = NULL;
    }

    if(cli_intercom->cli_shell) {
        cli_intercom_free_shell(cli_intercom);
    }
}

static void cli_intercom_do_protocol_disconnect(CliIntercom* cli_intercom) {
    FURI_LOG_D(TAG, "ProtocolDisconnect");

    cli_intercom_handle_disconnect(cli_intercom);
}

static void cli_intercom_do_api_spawn(CliIntercom* cli_intercom, CliIntercomInternalEvent* event) {
    FURI_LOG_D(TAG, "ApiSpawn");

#ifdef CLI_INTERCOM_SLAVE
    furi_crash(); // can't spawn a shell on f20
#endif
    do {
        if(cli_intercom->own_pipe) {
            *event->spawn_status = CliIntercomSpawnStatusChannelTaken;
            break;
        }

        cli_intercom->close_pipe_on_detach = event->close_pipe_on_detach;
        cli_intercom_attach_own_pipe(cli_intercom, event->pipe);

        if(!cli_intercom_send_protocol_status(
               cli_intercom, CliIntercomMessageTypeSpawn, CLI_INTERCOM_TIMEOUT)) {
            *event->spawn_status = CliIntercomSpawnStatusTimeout;
            cli_intercom_handle_disconnect(cli_intercom);
        } else {
            *event->spawn_status = CliIntercomSpawnStatusOk;
        }
    } while(0);

    api_lock_unlock(event->api_lock);
}

static void cli_intercom_do_api_join(CliIntercom* cli_intercom, CliIntercomInternalEvent* event) {
    FURI_LOG_D(TAG, "ApiJoin");

#ifdef CLI_INTERCOM_SLAVE
    furi_crash(); // can't spawn a shell on f20
#endif
    if(!cli_intercom->own_pipe) {
        api_lock_unlock(event->api_lock);
        return;
    }

    cli_intercom->join_lock = event->api_lock;
}

// ===================
// EventLoop callbacks
// ===================

static void cli_intercom_msg_handler(FuriEventLoopObject* object, void* context) {
    FuriMessageQueue* msg_queue = object;
    CliIntercom* cli_intercom = context;

    CliIntercomInternalEvent event;
    furi_check(furi_message_queue_get(msg_queue, &event, 0) == FuriStatusOk);

    switch(event.type) {
    case CliIntercomInternalEventTypeProtocolSpawn:
        cli_intercom_do_protocol_spawn(cli_intercom);
        break;
    case CliIntercomInternalEventTypeProtocolDisconnect:
        cli_intercom_do_protocol_disconnect(cli_intercom);
        break;
    case CliIntercomInternalEventTypeApiSpawn:
        cli_intercom_do_api_spawn(cli_intercom, &event);
        break;
    case CliIntercomInternalEventTypeApiJoin:
        cli_intercom_do_api_join(cli_intercom, &event);
        break;
    case CliIntercomInternalEventTypeIntercomDesync:
        cli_intercom_handle_disconnect(cli_intercom);
        break;
    }
}

static void cli_intercom_data_from_pipe(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliIntercom* cli_intercom = context;
    // Signal the TX worker thread that pipe data is available.
    // The TX thread is the sole reader of the pipe, keeping intercom_tx
    // off the event loop to avoid blocking drain_rx_to_pipe.
    furi_semaphore_release(cli_intercom->tx_data_available);
}

static void cli_intercom_pipe_broken(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliIntercom* cli_intercom = context;
    // TX worker thread will send disconnect on exit
    cli_intercom_handle_disconnect(cli_intercom);
}

static int32_t cli_intercom_tx_worker(void* context) {
    CliIntercom* cli_intercom = context;

    while(!cli_intercom->tx_shutdown) {
        furi_semaphore_acquire(cli_intercom->tx_data_available, 100);

        if(cli_intercom->tx_shutdown) break;

        // Drain all available data from the pipe in protocol-sized chunks
        while(!cli_intercom->tx_shutdown) {
            size_t bytes = pipe_bytes_available(cli_intercom->own_pipe);
            if(!bytes) break;

            size_t to_read = MIN(bytes, CLI_INTERCOM_MAX_PAYLOAD_LEN);
            uint8_t buffer[to_read];
            furi_check(
                pipe_receive(cli_intercom->own_pipe, buffer, sizeof(buffer)) == sizeof(buffer));

            if(!cli_intercom_send_protocol_payload(
                   cli_intercom,
                   CliIntercomMessageTypeData,
                   buffer,
                   sizeof(buffer),
                   CLI_INTERCOM_TX_TIMEOUT)) {
                if(cli_intercom->tx_shutdown) return 0;
                FURI_LOG_E(TAG, "TX failed, disconnecting");
                cli_intercom_send_simple_event(
                    cli_intercom, CliIntercomInternalEventTypeProtocolDisconnect);
                return 0;
            }
        }
    }

    // Send disconnect message to the peer before exiting
    cli_intercom_send_protocol_status(
        cli_intercom, CliIntercomMessageTypeDisconnect, CLI_INTERCOM_TIMEOUT);

    return 0;
}

static void cli_intercom_intercom_event_callback(const void* message, void* context) {
    CliIntercom* cli_intercom = context;
    const IntercomEvent* event = message;

    if(event->type == IntercomEventTypeSyncStateChanged && !event->is_in_sync) {
        FURI_LOG_W(TAG, "Intercom lost sync, signaling death");
        cli_intercom_send_simple_event(cli_intercom, CliIntercomInternalEventTypeIntercomDesync);
    }
}

static void cli_intercom_drain_rx_to_pipe(CliIntercom* cli_intercom) {
    if(!cli_intercom->own_pipe) return;

    while(true) {
        size_t bytes_in_buffer =
            furi_stream_buffer_bytes_available(cli_intercom->intercom_rx_stream);
        if(!bytes_in_buffer) break;

        size_t spaces_in_pipe = pipe_spaces_available(cli_intercom->own_pipe);
        if(!spaces_in_pipe) break;

        size_t to_transfer = MIN(MIN(bytes_in_buffer, spaces_in_pipe), DATA_BATCH_SIZE);
        uint8_t buffer[to_transfer];
        furi_check(
            furi_stream_buffer_receive(
                cli_intercom->intercom_rx_stream, buffer, sizeof(buffer), 0) == sizeof(buffer));
        pipe_send(cli_intercom->own_pipe, buffer, sizeof(buffer));
    }
}

static void cli_intercom_intercom_rx_handler(FuriEventLoopObject* object, void* context) {
    UNUSED(object);
    CliIntercom* cli_intercom = context;
    cli_intercom_drain_rx_to_pipe(cli_intercom);
}

// ============
// Thread setup
// ============

static CliIntercom* cli_intercom_alloc(void) {
    CliIntercom* cli_intercom = malloc(sizeof(CliIntercom));

    cli_intercom->registry = furi_record_open(RECORD_CLI);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    cli_intercom->intercom_ch = intercom_channel_open(
        intercom, IntercomChannelIdCli, cli_intercom_intercom_rx_callback, cli_intercom);
    furi_pubsub_subscribe(
        intercom_get_pubsub(intercom), cli_intercom_intercom_event_callback, cli_intercom);

    cli_intercom->event_loop = furi_event_loop_alloc();

    cli_intercom->msg_queue =
        furi_message_queue_alloc(MSG_Q_SIZE, sizeof(CliIntercomInternalEvent));
    furi_event_loop_subscribe_message_queue(
        cli_intercom->event_loop,
        cli_intercom->msg_queue,
        FuriEventLoopEventIn,
        cli_intercom_msg_handler,
        cli_intercom);

    cli_intercom->intercom_rx_stream = furi_stream_buffer_alloc(RX_STREAM_SIZE, 1);
    cli_intercom->tx_data_available = furi_semaphore_alloc(1, 0);

    furi_event_loop_subscribe_stream_buffer(
        cli_intercom->event_loop,
        cli_intercom->intercom_rx_stream,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        cli_intercom_intercom_rx_handler,
        cli_intercom);

    furi_record_create(RECORD_CLI_INTERCOM, cli_intercom);
    return cli_intercom;
}

int32_t cli_intercom_srv(void* context) {
    UNUSED(context);

    CliIntercom* cli_intercom = cli_intercom_alloc();
    furi_event_loop_run(cli_intercom->event_loop);

    return 0;
}

// ==========
// Public API
// ==========

static void cli_intercom_api_call(CliIntercom* cli_intercom, CliIntercomInternalEvent* event) {
    furi_check(cli_intercom);
    furi_assert(event);

    event->api_lock = api_lock_alloc_locked();
    furi_check(furi_message_queue_put(cli_intercom->msg_queue, event, 0) == FuriStatusOk);
    api_lock_wait_unlock_and_free(event->api_lock);
}

CliIntercomSpawnStatus
    cli_intercom_spawn(CliIntercom* cli_intercom, PipeSide* pipe, bool close_pipe_on_detach) {
    furi_check(pipe);
    CliIntercomSpawnStatus spawn_status;
    CliIntercomInternalEvent event = {
        .type = CliIntercomInternalEventTypeApiSpawn,
        .pipe = pipe,
        .spawn_status = &spawn_status,
        .close_pipe_on_detach = close_pipe_on_detach,
    };
    cli_intercom_api_call(cli_intercom, &event);
    return spawn_status;
}

void cli_intercom_join(CliIntercom* cli_intercom) {
    CliIntercomInternalEvent event = {
        .type = CliIntercomInternalEventTypeApiJoin,
    };
    cli_intercom_api_call(cli_intercom, &event);
}
