#include <furi.h>

#include <lwip/tcp.h>
#include <lwip/tcpip.h>

#include <wifi/wifi.h>
#include <sockets/sockets.h>
#include <usb_network/usb_network.h>

#define TAG "WifiBridgeSrv"

#define MESSAGE_QUEUE_SIZE (8)
#define HTTP_PORT          (80)

typedef enum {
    WifiBridgeEventTypeWifi,
    WifiBridgeEventTypeSocket,
    WifiBridgeEventTypeMax,
} WifiBridgeEventType;

typedef struct {
    WifiBridgeEventType type;
    union {
        WifiState wifi_state;
        SocketEvent socket_event;
    };
} WifiBridgeEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    struct tcp_pcb* lo_socket;
    SocketSrv* socket_srv;
    Socket* server_socket;
    Socket* client_socket;
    uint8_t buf[TCP_MSS];
} WifiBridge;

static void wifi_bridge_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    WifiBridge* instance = context;

    const WifiBridgeEvent event = {
        .type = WifiBridgeEventTypeWifi,
        .wifi_state = *(WifiState*)message,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &event, FuriWaitForever) == FuriStatusOk);
}

static void wifi_bridge_socket_callback(const SocketEvent* socket_event, void* context) {
    furi_assert(socket_event);
    furi_assert(context);

    WifiBridge* instance = context;

    const WifiBridgeEvent event = {
        .type = WifiBridgeEventTypeSocket,
        .socket_event = *(SocketEvent*)socket_event,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &event, FuriWaitForever) == FuriStatusOk);
}

static err_t wifi_bridge_loopback_connected_callback(void* arg, struct tcp_pcb* tpcb, err_t err) {
    UNUSED(arg);
    UNUSED(tpcb);

    if(err != ERR_OK) {
        FURI_LOG_E(TAG, "Error connecting: %d", err);
        return err;
    }

    return ERR_OK;
}

static void wifi_bridge_loopback_error_callback(void* arg, err_t err) {
    UNUSED(arg);
    FURI_LOG_E(TAG, "lwIP error: %d", err);
}

static void wifi_bridge_rearm_server(WifiBridge* instance) {
    furi_check(instance->lo_socket);

    if(tcp_close(instance->lo_socket) != ERR_OK) {
        FURI_LOG_E(TAG, "Failed to close loopback_socket");
    }

    furi_check(instance->client_socket);
    socket_free(instance->client_socket);

    instance->client_socket = NULL;
    instance->lo_socket = NULL;

    if(socket_accept(instance->server_socket) != SocketStatusOk) {
        FURI_LOG_E(TAG, "Failed to accept on wifi socket");
    }
}

static size_t wifi_bridge_send_client(WifiBridge* instance, struct pbuf* pbuf) {
    size_t bytes_received = 0;

    for(struct pbuf* pbuf_cur = pbuf; pbuf_cur; pbuf_cur = pbuf_cur->next) {
        const uint16_t bytes_available = pbuf_cur->len;
        size_t total_sent_size = 0;

        while(total_sent_size < bytes_available) {
            size_t sent_size;
            const SocketStatus status = socket_send(
                instance->client_socket,
                pbuf_cur->payload + total_sent_size,
                bytes_available - total_sent_size,
                &sent_size);

            if(status != SocketStatusOk) {
                FURI_LOG_E(TAG, "Failed to send to wifi socket");
                break;
            }

            total_sent_size += sent_size;
        }

        bytes_received += total_sent_size;
    }

    tcp_recved(instance->lo_socket, bytes_received);
    pbuf_free(pbuf);

    return bytes_received;
}

static err_t wifi_bridge_loopback_received_callback(
    void* arg,
    struct tcp_pcb* tpcb,
    struct pbuf* pbuf,
    err_t err) {
    furi_assert(arg);

    if(err != ERR_OK) {
        FURI_LOG_E(TAG, "Error receiving: %d", err);
        return err;
    }

    WifiBridge* instance = arg;

    if(pbuf) {
        furi_check(tpcb == instance->lo_socket);
        wifi_bridge_send_client(instance, pbuf);

    } else if(instance->lo_socket) {
        FURI_LOG_D(TAG, "Connection closed");
        wifi_bridge_rearm_server(instance);
    }

    return ERR_OK;
}

static bool wifi_bridge_enable_loopback(WifiBridge* instance) {
    LOCK_TCPIP_CORE();
    furi_check(instance->lo_socket == NULL);

    instance->lo_socket = tcp_new();
    tcp_arg(instance->lo_socket, instance);
    tcp_err(instance->lo_socket, wifi_bridge_loopback_error_callback);
    tcp_recv(instance->lo_socket, wifi_bridge_loopback_received_callback);

    const ip_addr_t addr = IPADDR4_INIT(PP_HTONL(IPADDR_LOOPBACK));
    const err_t status = tcp_connect(
        instance->lo_socket, &addr, HTTP_PORT, wifi_bridge_loopback_connected_callback);

    UNLOCK_TCPIP_CORE();

    return status == ERR_OK;
}

static err_t
    wifi_bridge_send_loopback(struct tcp_pcb* socket, const void* data, size_t data_size) {
    size_t total_bytes_sent = 0;
    err_t status = ERR_OK;

    LOCK_TCPIP_CORE();

    while(total_bytes_sent < data_size) {
        const size_t space_available = tcp_sndbuf(socket);

        if(space_available == 0) {
            FURI_LOG_W(TAG, "No space available");
            status = tcp_output(socket);

            if(status != ERR_OK) {
                break;
            }

            UNLOCK_TCPIP_CORE();
            furi_thread_yield();
            LOCK_TCPIP_CORE();

            continue;
        }

        const size_t chunk_size = data_size - total_bytes_sent;
        const size_t bytes_to_send = MIN(chunk_size, space_available);

        const uint8_t tcp_flags = bytes_to_send < chunk_size ?
                                      (TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE) :
                                      TCP_WRITE_FLAG_COPY;

        status = tcp_write(socket, data + total_bytes_sent, bytes_to_send, tcp_flags);

        if(status != ERR_OK) {
            break;
        }

        status = tcp_output(socket);

        if(status != ERR_OK) {
            break;
        }

        total_bytes_sent += bytes_to_send;
    }

    UNLOCK_TCPIP_CORE();

    return status;
}

static bool wifi_bridge_enable_server(WifiBridge* instance) {
    bool success = false;

    do {
        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolTcp,
        };

        instance->server_socket = socket_alloc(
            instance->socket_srv, &socket_info, wifi_bridge_socket_callback, instance);

        if(instance->server_socket == NULL) {
            FURI_LOG_E(TAG, "Failed to create server socket");
            break;
        }

        const SocketConnectionInfo bind_info = {
            .ip_type = SocketIpTypeV4,
            .port = HTTP_PORT,
        };

        if(socket_bind(instance->server_socket, &bind_info) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to bind server socket");
            break;
        }

        if(socket_listen(instance->server_socket, 1) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to listen on server socket");
            break;
        }

        if(socket_accept(instance->server_socket) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to accept on server socket");
            break;
        }

        success = true;
    } while(false);

    return success;
}

static void wifi_bridge_disable_server(WifiBridge* instance) {
    if(instance->client_socket) {
        if(socket_free(instance->client_socket) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to free client socket");
        }

        instance->client_socket = NULL;
    }

    if(instance->server_socket) {
        if(socket_free(instance->server_socket) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to free server socket");
        }

        instance->server_socket = NULL;
    }
}

static void wifi_bridge_handle_wifi_state(WifiBridge* instance, WifiState wifi_state) {
    if(wifi_state == WifiStateUp) {
        wifi_bridge_enable_server(instance);
    } else {
        wifi_bridge_disable_server(instance);
    }
}

static void
    wifi_bridge_handle_socket_event(WifiBridge* instance, const SocketEvent* socket_event) {
    const SocketEventType event_type = socket_event->type;

    if(event_type == SocketEventTypeAccept) {
        FURI_LOG_D(TAG, "Accept");

        furi_check(instance->client_socket == NULL);
        instance->client_socket = socket_event->accept.client_socket;

        if(!wifi_bridge_enable_loopback(instance)) {
            FURI_LOG_E(TAG, "Failed to open loopback_socket");
        }

    } else if(event_type == SocketEventTypeClose) {
        FURI_LOG_D(TAG, "Close");

        LOCK_TCPIP_CORE();
        wifi_bridge_rearm_server(instance);
        UNLOCK_TCPIP_CORE();

    } else if(event_type == SocketEventTypeReceive) {
#ifdef WIFI_BRIDGE_SLOW_LOGS
        FURI_LOG_D(TAG, "Receive");
#endif
        furi_check(instance->client_socket);

        do {
            int status;

            size_t bytes_received;
            status = socket_receive(
                instance->client_socket, instance->buf, sizeof(instance->buf), &bytes_received);

            if(status != SocketStatusOk) {
                FURI_LOG_E(TAG, "Failed to receive from wifi socket");
                break;
            }

            status = wifi_bridge_send_loopback(instance->lo_socket, instance->buf, bytes_received);

            if(status != ERR_OK) {
                FURI_LOG_E(TAG, "Failed to send to loopback socket");
                break;
            }

        } while(false);
    }
}

static void wifi_bridge_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    WifiBridge* instance = context;
    furi_assert(instance->message_queue == object);

    WifiBridgeEvent event;
    while(furi_message_queue_get(instance->message_queue, &event, 0) == FuriStatusOk) {
        if(event.type == WifiBridgeEventTypeWifi) {
            wifi_bridge_handle_wifi_state(instance, event.wifi_state);
        } else if(event.type == WifiBridgeEventTypeSocket) {
            wifi_bridge_handle_socket_event(instance, &event.socket_event);
        }
    }
}

static WifiBridge* wifi_bridge_alloc(void) {
    WifiBridge* instance = malloc(sizeof(WifiBridge));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue =
        furi_message_queue_alloc(MESSAGE_QUEUE_SIZE, sizeof(WifiBridgeEvent));
    instance->socket_srv = furi_record_open(RECORD_SOCKETS);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        wifi_bridge_message_queue_callback,
        instance);

    Wifi* wifi = furi_record_open(RECORD_WIFI);
    furi_pubsub_subscribe(wifi_get_pubsub(wifi), wifi_bridge_pubsub_callback, instance);

    return instance;
}

int wifi_bridge_srv(void* arg) {
    UNUSED(arg);

    WifiBridge* instance = wifi_bridge_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
