#include "cli_socket_client.h"
#include <containers/pipe.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_registry.h>
#include <cli/cli_main_shell.h>
#include <cli/cli_commands.h>
#include <cli/cli_ansi.h>
#include <netstat/netstat.h>

#include <lwip/tcpip.h>

#define THREAD_STACK_SIZE     (2 * 1024)
#define PIPE_SZ_PER_DIRECTION (8 * 1024U)
#define COPY_BUF_SZ           256U
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
    FuriSemaphore* tcpip_semaphore;
    FuriSemaphore* tx_semaphore;
    FuriEventLoop* event_loop;
    PipeSide* own_pipe;
    PipeSide* shell_pipe;
    CliRegistry* main_registry;
    CliShell* shell;
    FuriEventFlag* event_flag;

    // Pending data from TCP that didn't fit in the pipe
    struct pbuf* pending_data;
    size_t pending_offset; // offset into first pbuf of pending_data

    // Reusable buffer for shell->client copies
    uint8_t copy_buf[COPY_BUF_SZ];
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
// Thread synchronisation
// ============

static void cli_socket_client_tcpip_unlock(CliSocketClient* client) {
    furi_check(furi_semaphore_release(client->tcpip_semaphore) == FuriStatusOk);
}

static void cli_socket_client_wait_tcpip_unlock(CliSocketClient* client) {
    furi_check(furi_semaphore_acquire(client->tcpip_semaphore, FuriWaitForever) == FuriStatusOk);
}

// ============
// Data copying
// ============

/**
 * @brief Drain pipe into TCP socket in COPY_BUF_SZ chunks.
 *
 * @param client      Client context
 * @param to_send     Number of bytes to transfer (must be <= pipe available AND tcp_sndbuf)
 * @param pipe_avail  Total bytes available in pipe (used for TCP_WRITE_FLAG_MORE hint)
 */
static void
    cli_socket_client_pipe_to_tcp(CliSocketClient* client, size_t to_send, size_t pipe_avail) {
    size_t sent = 0;
    while(sent < to_send) {
        size_t chunk = MIN(to_send - sent, sizeof(client->copy_buf));
        furi_check(pipe_receive(client->own_pipe, client->copy_buf, chunk) == chunk);
        sent += chunk;

        bool more_coming = (sent < to_send) || (to_send < pipe_avail);
        uint8_t flags = TCP_WRITE_FLAG_COPY | (more_coming ? TCP_WRITE_FLAG_MORE : 0);

        if(tcp_write(client->socket, client->copy_buf, chunk, flags) != ERR_OK) {
            FURI_LOG_E(TAG, "tcp_write error");
            break;
        }
    }

    err_t err = tcp_output(client->socket);
    if(err != ERR_OK) {
        FURI_LOG_W(TAG, "tcp_output error: %d (data still queued)", err);
    }
}

static void cli_socket_client_try_copy_sh2cl(void* context) {
    furi_assert(context);
    CliSocketClient* client = context;

    do {
        if(!client->socket) {
            FURI_LOG_E(TAG, "Socket is null, not copying sh->cl");
            break;
        }
        if(client->socket->state != ESTABLISHED) {
            FURI_LOG_E(
                TAG,
                "%s:%d not established: %d",
                ipaddr_ntoa(&client->socket->remote_ip),
                client->socket->remote_port,
                client->socket->state);
            break;
        }

        size_t tcp_space = tcp_sndbuf(client->socket);
        size_t pipe_avail = pipe_bytes_available(client->own_pipe);
        size_t to_send = MIN(pipe_avail, tcp_space);
        CLI_SOCKET_TRACE(
            TAG, DIR_SH_CL ": to_send=%zu (tcp=%zu pipe=%zu)", to_send, tcp_space, pipe_avail);

        if(!to_send) break;

        if(furi_semaphore_release(client->tx_semaphore) != FuriStatusOk) {
            CLI_SOCKET_TRACE(TAG, DIR_SH_CL ": tx locked");
            break;
        }

        cli_socket_client_pipe_to_tcp(client, to_send, pipe_avail);
    } while(false);

    cli_socket_client_tcpip_unlock(client);
}

// Forward declaration
static void cli_socket_client_drain_pending(void* context);

/**
 * @brief Try to copy data from a pbuf chain into the pipe.
 *
 * Consumes as much data as fits into the pipe without blocking.
 * Any unconsumed remainder is stored in client->pending_data/pending_offset
 * for later retry when the pipe has space.
 *
 * @param client     Client context
 * @param chain      pbuf chain to consume from
 * @param offset     byte offset into the first pbuf to start from
 */
static void
    cli_socket_client_try_copy_cl2sh(CliSocketClient* client, struct pbuf* chain, size_t offset) {
    CLI_SOCKET_TRACE(TAG, DIR_CL_SH ": pbuf chain (offset=%zu):", offset);
    size_t read_total = 0;
    bool pipe_full = false;

    struct pbuf* chunk = chain;
    size_t chunk_offset = offset;

    while(chunk && !pipe_full) {
        uint8_t* payload = (uint8_t*)chunk->payload + chunk_offset;
        size_t available_in_chunk = chunk->len - chunk_offset;
        CLI_SOCKET_TRACE(TAG, DIR_CL_SH ":   tcp_chunk=%zu", available_in_chunk);

        while(available_in_chunk) {
            size_t pipe_space = pipe_spaces_available(client->own_pipe);
            if(pipe_space == 0) {
                CLI_SOCKET_TRACE(TAG, DIR_CL_SH ":     pipe full, stopping");
                pipe_full = true;
                break;
            }

            size_t batch_sz = MIN(available_in_chunk, MIN(pipe_space, PIPE_SZ_PER_DIRECTION));
            CLI_SOCKET_TRACE(
                TAG, DIR_CL_SH ":     batch=%zu (left=%zu)", batch_sz, available_in_chunk);

            size_t sent = pipe_send(client->own_pipe, payload, batch_sz);
            read_total += sent;
            chunk_offset += sent;
            if(sent != batch_sz) {
                pipe_full = true;
                break;
            }

            available_in_chunk -= batch_sz;
            payload += batch_sz;
        }

        if(!pipe_full) {
            chunk = chunk->next;
            chunk_offset = 0;
        }
    }

    CLI_SOCKET_TRACE(TAG, DIR_CL_SH ": total=%zu", read_total);
    if(read_total > 0) {
        tcp_recved(client->socket, read_total);
    }

    if(pipe_full && chunk) {
        // Free fully consumed pbufs from the head of the chain.
        // Each pbuf is detached before freeing to prevent pbuf_free from
        // cascading into the remaining (unconsumed) tail of the chain.
        while(chain != chunk) {
            struct pbuf* consumed = chain;
            chain = chain->next;
            consumed->next = NULL;
            pbuf_free(consumed);
        }

        client->pending_data = chain;
        client->pending_offset = chunk_offset;
        CLI_SOCKET_TRACE(
            TAG,
            DIR_CL_SH ": pending %zu bytes (offset=%zu)",
            chain->tot_len - chunk_offset,
            chunk_offset);
    } else {
        // All data consumed, free the entire chain
        client->pending_data = NULL;
        client->pending_offset = 0;
        pbuf_free(chain);
    }
}

/**
 * @brief Drain pending data into the pipe (requires lwip thread context).
 */
static void cli_socket_client_drain_pending(void* context) {
    CliSocketClient* client = context;

    if(!client->pending_data || !client->socket) {
        cli_socket_client_tcpip_unlock(client);
        return;
    }

    struct pbuf* chain = client->pending_data;
    size_t offset = client->pending_offset;
    client->pending_data = NULL;
    client->pending_offset = 0;

    cli_socket_client_try_copy_cl2sh(client, chain, offset);
    cli_socket_client_tcpip_unlock(client);
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
        if(client->pending_data) {
            // Still have pending data that hasn't been drained yet,
            // concatenate new data to pending chain
            CLI_SOCKET_TRACE(TAG, "evt: " DIR_CL_SH " (appending to pending)");
            pbuf_cat(client->pending_data, data);
        } else {
            CLI_SOCKET_TRACE(TAG, "evt: " DIR_CL_SH);
            cli_socket_client_try_copy_cl2sh(client, data, 0);
        }
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
    if(flags & FuriFlagError) {
        // flags were already consumed by a previous edge callback
        return;
    }

    if(flags & CliSocketClientEventTcpTxDone) {
        furi_semaphore_acquire(client->tx_semaphore, 0);
        if(pipe_bytes_available(client->own_pipe) > 0) {
            tcpip_callback(cli_socket_client_try_copy_sh2cl, client);
            cli_socket_client_wait_tcpip_unlock(client);
        }
    }

    if((flags & CliSocketClientEventDisconnected) || (flags & CliSocketClientEventShellExit)) {
        pipe_set_data_arrived_callback(client->own_pipe, NULL, 0);
        pipe_set_space_freed_callback(client->own_pipe, NULL, 0);
        furi_event_loop_stop(client->event_loop);
    }
}

static void cli_socket_client_data_from_shell(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliSocketClient* client = context;
    CLI_SOCKET_TRACE(TAG, "evt: " DIR_SH_CL);
    tcpip_callback(cli_socket_client_try_copy_sh2cl, client);
    cli_socket_client_wait_tcpip_unlock(client);
}

static void cli_socket_client_space_freed(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliSocketClient* client = context;
    if(client->pending_data) {
        CLI_SOCKET_TRACE(TAG, "evt: pipe space freed, draining pending");
        tcpip_callback(cli_socket_client_drain_pending, client);
        cli_socket_client_wait_tcpip_unlock(client);
    }
}

static void cli_socket_client_init_callback(void* context) {
    furi_assert(context);
    CliSocketClient* client = context;

    tcp_arg(client->socket, client);
    tcp_err(client->socket, cli_socket_client_tcp_err);
    tcp_sent(client->socket, cli_socket_client_tcp_tx_done);
    tcp_recv(client->socket, cli_socket_data_from_client);

    cli_socket_client_tcpip_unlock(client);
}

static CliSocketClient* cli_socket_client_thread_init(struct tcp_pcb* client_socket) {
    CliSocketClient* client = malloc(sizeof(CliSocketClient));
    client->socket = client_socket;

    client->event_loop = furi_event_loop_alloc();

    client->event_flag = furi_event_flag_alloc();
    furi_event_loop_subscribe_event_flag(
        client->event_loop,
        client->event_flag,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        cli_socket_client_event,
        client);

    client->tcpip_semaphore = furi_semaphore_alloc(1, 0);
    client->tx_semaphore = furi_semaphore_alloc(1, 0);
    PipeSideBundle pipes = pipe_alloc(PIPE_SZ_PER_DIRECTION, 1);
    client->own_pipe = pipes.alices_side;
    client->shell_pipe = pipes.bobs_side;
    pipe_attach_to_event_loop(client->own_pipe, client->event_loop);
    pipe_set_callback_context(client->own_pipe, client);
    pipe_set_data_arrived_callback(
        client->own_pipe, cli_socket_client_data_from_shell, FuriEventLoopEventFlagEdge);
    pipe_set_space_freed_callback(
        client->own_pipe, cli_socket_client_space_freed, FuriEventLoopEventFlagEdge);
    pipe_set_broken_callback(client->own_pipe, cli_socket_client_shell_exit_callback, 0);

    tcpip_callback(cli_socket_client_init_callback, client);
    cli_socket_client_wait_tcpip_unlock(client);

    client->main_registry = furi_record_open(RECORD_CLI);

    client->shell =
        cli_shell_alloc(cli_main_motd, NULL, client->shell_pipe, client->main_registry, NULL);
    cli_shell_free_pipe_on_exit(client->shell);
    cli_shell_start(client->shell);

    return client;
}

static void cli_socket_client_deinit_callback(void* context) {
    furi_assert(context);
    CliSocketClient* client = context;

    if(client->pending_data) {
        pbuf_free(client->pending_data);
        client->pending_data = NULL;
        client->pending_offset = 0;
    }

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

    cli_socket_client_tcpip_unlock(client);
}

static void cli_socket_client_thread_deinit(CliSocketClient* client) {
    tcpip_callback(cli_socket_client_deinit_callback, client);
    cli_socket_client_wait_tcpip_unlock(client);

    pipe_set_data_arrived_callback(client->own_pipe, NULL, 0);
    pipe_set_space_freed_callback(client->own_pipe, NULL, 0);
    pipe_set_broken_callback(client->own_pipe, NULL, 0);
    pipe_detach_from_event_loop(client->own_pipe);
    pipe_free(client->own_pipe);

    if(client->pending_data) {
        pbuf_free(client->pending_data);
        client->pending_data = NULL;
    }

    cli_shell_join(client->shell);
    cli_shell_free(client->shell);

    furi_semaphore_free(client->tx_semaphore);
    furi_semaphore_free(client->tcpip_semaphore);

    furi_record_close(RECORD_CLI);

    furi_event_loop_unsubscribe(client->event_loop, client->event_flag);
    furi_event_flag_free(client->event_flag);

    furi_event_loop_free(client->event_loop);

    free(client);
}

static int32_t cli_socket_client_thread(void* context) {
    struct tcp_pcb* client_socket = context;
    furi_assert(client_socket);

    if(netstat_is_overloaded(NetstatLogOnOverload)) {
        const char* art = ANSI_FLIPPER_BRAND_ORANGE
            "\r\n"
            "    ____    _____________    ____ \r\n"
            "  _L____J____I_________I____L____J_\r\n"
            " / " ANSI_FG_BR_RED " ___ _   _ _____   __           " ANSI_FLIPPER_BRAND_ORANGE
            "\\\r\n"
            "|  " ANSI_FG_BR_RED "| _ ) | | / __\\ \\ / /   X    X   " ANSI_FLIPPER_BRAND_ORANGE
            "|\r\n"
            "|  " ANSI_FG_BR_RED "| _ \\ |_| \\__ \\\\ V /   --------  " ANSI_FLIPPER_BRAND_ORANGE
            "|\r\n"
            "|  " ANSI_FG_BR_RED "|___/\\___/|___/ |_|   /        \\ " ANSI_FLIPPER_BRAND_ORANGE
            "|\r\n"
            " \\_________________________________/\r\n"
            "\r\n" ANSI_FG_BR_RED NETSTAT_RECOMMENDED_ERROR ANSI_RESET "\r\n";

        LOCK_TCPIP_CORE();
        tcp_write(client_socket, art, strlen(art), TCP_WRITE_FLAG_COPY);
        if(tcp_close(client_socket) != ERR_OK) tcp_abort(client_socket);
        UNLOCK_TCPIP_CORE();

        return -1;
    }

    CliSocketClient* client = cli_socket_client_thread_init(client_socket);
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

    FuriThread* thread =
        furi_thread_alloc_ex(TAG, THREAD_STACK_SIZE, cli_socket_client_thread, client_socket);
    furi_thread_set_state_callback(thread, cli_socket_client_thread_state_callback);
    furi_thread_start(thread);
}
