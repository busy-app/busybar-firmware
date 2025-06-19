#include "cli_socket_client.h"
#include <containers/pipe.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_registry.h>
#include <cli/cli_main_shell.h>
#include <cli/cli_commands.h>
#include <cli/cli_ansi.h>

#include <lwip/tcpip.h>

#define THREAD_STACK_SIZE     (2 * 1024)
#define PIPE_SZ_PER_DIRECTION 1024U
// #define CLI_SOCKET_TRACE_ENABLE

#define TAG       "CliSocketClient"
#define DIR_CL_SH ANSI_FG_GREEN "cl->sh" ANSI_RESET
#define DIR_SH_CL ANSI_FG_YELLOW "sh->cl" ANSI_RESET

#ifdef CLI_SOCKET_TRACE_ENABLE
#define CLI_SOCKET_TRACE(...) FURI_LOG_T(__VA_ARGS__)
#else
#define CLI_SOCKET_TRACE(...)
#endif

typedef struct {
    // Allocated externally
    struct tcp_pcb* socket;

    // Allocated in thread
    FuriSemaphore* tx_semaphore;
    FuriEventLoop* event_loop;
    PipeSide* own_pipe;
    PipeSide* shell_pipe;
    CliRegistry* main_registry;
    CliShell* shell;
    FuriEventFlag* event_flag;
} CliSocketClient;

typedef enum {
    CliSocketClientEventTcpTxDone = (1 << 0),
    CliSocketClientEventDisconnected = (1 << 1),
    CliSocketClientEventShellExit = (1 << 2),
} CliSocketClientEvent;

#define CliSocketClientEventAll                                         \
    (CliSocketClientEventTcpTxDone | CliSocketClientEventDisconnected | \
     CliSocketClientEventShellExit)

// ============
// Data copying
// ============

static void cli_socket_client_try_copy_sh2cl(CliSocketClient* client) {
    LOCK_TCPIP_CORE();

    if(!client->socket || client->socket->state != ESTABLISHED) {
        if(client->socket) {
            FURI_LOG_E(
                TAG,
                "%s:%d not established: %d",
                ipaddr_ntoa(&client->socket->remote_ip),
                client->socket->remote_port,
                client->socket->state);
        } else {
            FURI_LOG_E(TAG, "Socket is null, not copying sh->cl");
        }
        UNLOCK_TCPIP_CORE();
        return;
    }

    size_t available_in_tcp_buf = tcp_sndbuf(client->socket);
    size_t available_in_pipe = pipe_bytes_available(client->own_pipe);
    size_t batch_sz = MIN(available_in_pipe, available_in_tcp_buf);
    CLI_SOCKET_TRACE(
        TAG,
        DIR_SH_CL ": batch=%zu (tcp=%zu pipe=%zu)",
        batch_sz,
        available_in_tcp_buf,
        available_in_pipe);

    do {
        if(!batch_sz) break;

        if(furi_semaphore_release(client->tx_semaphore) != FuriStatusOk) {
            CLI_SOCKET_TRACE(TAG, DIR_SH_CL ": tx locked");
            break;
        }

        uint8_t buf[batch_sz];
        furi_check(pipe_receive(client->own_pipe, buf, sizeof(buf)) == sizeof(buf));

        uint8_t lwip_flags = TCP_WRITE_FLAG_COPY;
        if(batch_sz < available_in_pipe) lwip_flags |= TCP_WRITE_FLAG_MORE;

        err_t err;
        err = tcp_write(client->socket, buf, sizeof(buf), lwip_flags);
        if(err != ERR_OK) {
            FURI_LOG_E(TAG, "tcp_write error: %d", err);
            furi_crash();
        }

        err = tcp_output(client->socket);
        if(err != ERR_OK) {
            FURI_LOG_E(TAG, "tcp_output error: %d", err);
            furi_crash();
        }
    } while(false);

    UNLOCK_TCPIP_CORE();
}

static void cli_socket_client_try_copy_cl2sh(CliSocketClient* client, struct pbuf* chunk_chain) {
    CLI_SOCKET_TRACE(TAG, DIR_CL_SH ": pbuf chain:");
    size_t read_total = 0;

    struct pbuf* chunk = chunk_chain;
    while(chunk) {
        uint8_t* payload = chunk->payload;
        size_t available_in_chunk = chunk->len;
        CLI_SOCKET_TRACE(TAG, DIR_CL_SH ":   tcp_chunk=%zu", available_in_chunk);

        while(available_in_chunk) {
            size_t batch_sz = MIN(available_in_chunk, PIPE_SZ_PER_DIRECTION);
            CLI_SOCKET_TRACE(
                TAG, DIR_CL_SH ":     batch=%zu (left=%zu)", batch_sz, available_in_chunk);

            uint8_t buf[batch_sz];
            memcpy(buf, payload, sizeof(buf));
            if(pipe_send(client->own_pipe, buf, sizeof(buf)) != sizeof(buf)) break;

            available_in_chunk -= batch_sz;
            payload += batch_sz;
            read_total += batch_sz;
        }

        chunk = chunk->next;
    }

    CLI_SOCKET_TRACE(TAG, DIR_CL_SH ": total=%zu", read_total);
    tcp_recved(client->socket, read_total);
    pbuf_free(chunk_chain);
}

// ==============
// lwIP callbacks
// ==============

static void cli_socket_client_set_flag(CliSocketClient* client, CliSocketClientEvent event) {
    uint32_t ret = furi_event_flag_set(client->event_flag, event);
    furi_check(!(ret & FuriFlagError));
}

static void cli_socket_client_tcp_err(void* context, err_t err) {
    CliSocketClient* client = context;
    UNUSED(err);
    CLI_SOCKET_TRACE(TAG, "evt: tcp err %d", err);
    // pcb is already freed by lwip, so we must not use it
    client->socket = NULL;
    cli_socket_client_set_flag(client, CliSocketClientEventDisconnected);
}

static void cli_socket_client_shell_exit_callback(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliSocketClient* client = context;
    CLI_SOCKET_TRACE(TAG, "evt: shell exit");
    cli_socket_client_set_flag(client, CliSocketClientEventShellExit);
}

static err_t cli_socket_client_tcp_tx_done(void* context, struct tcp_pcb* socket, uint16_t len) {
    UNUSED(socket);
    UNUSED(len);
    CliSocketClient* client = context;
    cli_socket_client_set_flag(client, CliSocketClientEventTcpTxDone);
    CLI_SOCKET_TRACE(TAG, "evt: tcp_tx_done");
    return ERR_OK;
}

static err_t cli_socket_data_from_client(
    void* context,
    struct tcp_pcb* socket,
    struct pbuf* data,
    err_t err) {
    UNUSED(socket);
    CliSocketClient* client = context;

    if(err != ERR_OK) return ERR_OK;

    if(data) {
        CLI_SOCKET_TRACE(TAG, "evt: " DIR_CL_SH);
        cli_socket_client_try_copy_cl2sh(client, data);
    } else {
        // Connection closed by client
        CLI_SOCKET_TRACE(TAG, "evt: client disconnected");
        cli_socket_client_set_flag(client, CliSocketClientEventDisconnected);
    }

    return ERR_OK;
}

// ===========
// Thread code
// ===========

static void cli_socket_client_event(FuriEventLoopObject* object, void* context) {
    FuriEventFlag* flag = object;
    CliSocketClient* client = context;

    uint32_t flags = furi_event_flag_wait(flag, CliSocketClientEventAll, FuriFlagWaitAny, 0);
    furi_check(!(flags & FuriFlagError));

    if(flags & CliSocketClientEventTcpTxDone) {
        furi_semaphore_acquire(client->tx_semaphore, 0);
    }

    if((flags & CliSocketClientEventDisconnected) || (flags & CliSocketClientEventShellExit)) {
        pipe_set_data_arrived_callback(client->own_pipe, NULL, 0);
        furi_event_loop_stop(client->event_loop);
    }
}

static void cli_socket_client_data_from_shell(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliSocketClient* client = context;
    CLI_SOCKET_TRACE(TAG, "evt: " DIR_SH_CL);
    cli_socket_client_try_copy_sh2cl(client);
}

static void cli_socket_client_thread_init(CliSocketClient* client) {
    client->event_loop = furi_event_loop_alloc();

    client->event_flag = furi_event_flag_alloc();
    furi_event_loop_subscribe_event_flag(
        client->event_loop,
        client->event_flag,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        cli_socket_client_event,
        client);

    client->tx_semaphore = furi_semaphore_alloc(1, 0);
    PipeSideBundle pipes = pipe_alloc(PIPE_SZ_PER_DIRECTION, 1);
    client->own_pipe = pipes.alices_side;
    client->shell_pipe = pipes.bobs_side;
    pipe_attach_to_event_loop(client->own_pipe, client->event_loop);
    pipe_set_callback_context(client->own_pipe, client);
    pipe_set_data_arrived_callback(client->own_pipe, cli_socket_client_data_from_shell, 0);
    pipe_set_broken_callback(client->own_pipe, cli_socket_client_shell_exit_callback, 0);

    LOCK_TCPIP_CORE();
    tcp_arg(client->socket, client);
    tcp_err(client->socket, cli_socket_client_tcp_err);
    tcp_sent(client->socket, cli_socket_client_tcp_tx_done);
    tcp_recv(client->socket, cli_socket_data_from_client);
    UNLOCK_TCPIP_CORE();

    client->main_registry = furi_record_open(RECORD_CLI);

    client->shell =
        cli_shell_alloc(cli_main_motd, NULL, client->shell_pipe, client->main_registry, NULL);
    cli_shell_free_pipe_on_exit(client->shell);
    cli_shell_start(client->shell);
}

static void cli_socket_client_thread_deinit(CliSocketClient* client) {
    LOCK_TCPIP_CORE();
    if(client->socket) {
        tcp_arg(client->socket, NULL);
        tcp_sent(client->socket, NULL);
        tcp_recv(client->socket, NULL);
        tcp_err(client->socket, NULL);
        err_t err = tcp_close(client->socket);
        if(err != ERR_OK) {
            FURI_LOG_W(TAG, "tcp_close failed with err %d, aborting", err);
            tcp_abort(client->socket);
        }
        client->socket = NULL;
    }
    UNLOCK_TCPIP_CORE();

    pipe_set_data_arrived_callback(client->own_pipe, NULL, 0);
    pipe_set_broken_callback(client->own_pipe, NULL, 0);
    pipe_detach_from_event_loop(client->own_pipe);
    pipe_free(client->own_pipe);

    cli_shell_join(client->shell);
    cli_shell_free(client->shell);

    furi_semaphore_free(client->tx_semaphore);

    furi_record_close(RECORD_CLI);

    furi_event_loop_unsubscribe(client->event_loop, client->event_flag);
    furi_event_flag_free(client->event_flag);

    furi_event_loop_free(client->event_loop);
}

static int32_t cli_socket_client_thread(void* context) {
    CliSocketClient* client = context;
    furi_check(client && client->socket);

    cli_socket_client_thread_init(client);
    FURI_LOG_D(
        TAG,
        "Started for %s:%d",
        ipaddr_ntoa(&client->socket->remote_ip),
        client->socket->remote_port);

    CLI_SOCKET_TRACE(TAG, "Socket state: %d", client->socket->state)

    furi_event_loop_run(client->event_loop);

    FURI_LOG_D(
        TAG,
        "Stopped for %s:%d",
        client->socket ? ipaddr_ntoa(&client->socket->remote_ip) : "NULL",
        client->socket ? client->socket->remote_port : 0);
    cli_socket_client_thread_deinit(client);

    return 0;
}

static void cli_socket_client_thread_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    CliSocketClient* client = context;
    UNUSED(client);
    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
}

// ==========
// Public API
// ==========

void cli_socket_client_start(struct tcp_pcb* client_socket) {
    furi_check(client_socket);

    CliSocketClient* client = malloc(sizeof(CliSocketClient));
    client->socket = client_socket;

    FuriThread* thread =
        furi_thread_alloc_ex(TAG, THREAD_STACK_SIZE, cli_socket_client_thread, client);
    furi_thread_set_state_callback(thread, cli_socket_client_thread_state_callback);
    furi_thread_start(thread);
}
