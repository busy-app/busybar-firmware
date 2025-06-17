#include <furi.h>

#include <wifi/wifi.h>
#include <sockets/sockets.h>

#define TAG "SocketsTestApp"

#define MAX_CHUNK_SIZE (1014) // Limited by intercom et al

#define WIFI_SSID "Your SSID"
#define WIFI_PASS "Your passphrase"
#define WIFI_MODE WifiSecurityModeWpa2

#define CONNECT_PORT    8080
#define LISTEN_PORT     8081
#define UDP_PORT        8082
#define CONNECT_ADDRESS 10, 46, 30, 175 // MUST be commas, not dots

#define MAX_TCP_CLIENTS 1

typedef enum {
    SocketIndexTcpClient,
    SocketIndexTcpServer,
    SocketIndexTcpRemoteClient,
    SocketIndexUdpClientServer,
    SocketIndexMax,
} SocketIndex;

typedef struct {
    Wifi* wifi;
    SocketSrv* sockets_srv;
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    Socket* sockets[SocketIndexMax];
    WifiInfo wifi_info;
    char tmp_buf[MAX_CHUNK_SIZE];
} SocketsTestApp;

static void socket_event_callback(const SocketEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SocketsTestApp* instance = context;

    furi_check(
        furi_message_queue_put(instance->event_queue, event, FuriWaitForever) == FuriStatusOk);
}

static void
    sockets_test_app_print_connection_info(const char* message, const SocketConnectionInfo* info) {
    FURI_LOG_I(
        TAG,
        "%s %hhu.%hhu.%hhu.%hhu:%hu",
        message,
        info->address.v4[0],
        info->address.v4[1],
        info->address.v4[2],
        info->address.v4[3],
        info->port);
}

static void sockets_test_app_wifi_info_to_connection_info(
    const WifiInfo* wifi_info,
    SocketConnectionInfo* connection_info) {
    connection_info->ip_type = wifi_info->ip_config.type;
    memcpy(
        &connection_info->address,
        &wifi_info->ip_config.address,
        sizeof(connection_info->address));
}

static bool sockets_test_app_init_wifi(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "Wifi initialisation start ...");

        if(wifi_init(instance->wifi) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to init Wifi");
            break;
        }

        FURI_LOG_I(TAG, "Wifi initialised!");

        const WifiCredentials credentials = {
            .ssid = WIFI_SSID,
            .passphrase = WIFI_PASS,
            .security_mode = WIFI_MODE,
        };

        const WifiIpConfig ip_config = {
            .mgmt = WifiIpManagementDynamic,
            .type = WifiIpTypeV4,
        };

        if(wifi_connect(instance->wifi, &credentials, &ip_config) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to connect to Wifi network");
            break;
        }

        FURI_LOG_I(TAG, "Wifi connected to %s!", WIFI_SSID);

        if(wifi_get_info(instance->wifi, &instance->wifi_info) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get Wifi info");
            break;
        }

        success = true;

    } while(false);

    return success;
}

static bool sockets_test_app_init_tcp_client(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "TCP client initialisation start ...");

        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolTcp,
        };

        Socket* socket;
        Socket** socket_slot = &instance->sockets[SocketIndexTcpClient];

        *socket_slot =
            socket_alloc(instance->sockets_srv, &socket_info, socket_event_callback, instance);
        socket = *socket_slot;

        if(socket == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate client socket");
            break;
        }

        FURI_LOG_I(TAG, "Client socket allocated successfully!");

        const SocketConnectionInfo connection_info = {
            .port = CONNECT_PORT,
            .ip_type = SocketIpTypeV4,
            .address.v4 = {CONNECT_ADDRESS},
        };

        if(socket_connect(socket, &connection_info) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Connection failed");
            break;
        }

        sockets_test_app_print_connection_info("Connected to", &connection_info);

        success = true;

    } while(false);

    return success;
}

static bool sockets_test_app_init_tcp_server(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "TCP server initialisation start ...");

        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolTcp,
        };

        Socket* socket;
        Socket** socket_slot = &instance->sockets[SocketIndexTcpServer];

        *socket_slot =
            socket_alloc(instance->sockets_srv, &socket_info, socket_event_callback, instance);
        socket = *socket_slot;

        if(socket == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate Server socket");
            break;
        }

        FURI_LOG_I(TAG, "Server socket allocated successfully!");

        SocketConnectionInfo bind_info;
        sockets_test_app_wifi_info_to_connection_info(&instance->wifi_info, &bind_info);
        bind_info.port = LISTEN_PORT;

        if(socket_bind(socket, &bind_info) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to bind socket");
            break;
        }

        FURI_LOG_I(TAG, "Server socket bound successfully!");

        if(socket_listen(socket, MAX_TCP_CLIENTS) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to listen on socket");
            break;
        }

        sockets_test_app_print_connection_info("Listening on", &bind_info);

        success = true;

    } while(false);

    return success;
}

static bool sockets_test_app_init_udp_client_server(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "UDP client/server initialisation start ...");

        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolUdp,
        };

        Socket* socket;
        Socket** socket_slot = &instance->sockets[SocketIndexUdpClientServer];

        *socket_slot =
            socket_alloc(instance->sockets_srv, &socket_info, socket_event_callback, instance);
        socket = *socket_slot;

        if(socket == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate UDP client/server socket");
            break;
        }

        FURI_LOG_I(TAG, "UDP client/server socket allocated successfully!");

        SocketConnectionInfo bind_info;
        sockets_test_app_wifi_info_to_connection_info(&instance->wifi_info, &bind_info);
        bind_info.port = UDP_PORT;

        if(socket_bind(socket, &bind_info) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to bind socket");
            break;
        }

        FURI_LOG_I(TAG, "UDP client/server socket bound successfully!");

        const SocketConnectionInfo connection_info = {
            .port = UDP_PORT,
            .ip_type = SocketIpTypeV4,
            .address.v4 = {CONNECT_ADDRESS},
        };

        // For UDP sockets, connect() sets the address used for send()
        if(socket_connect(socket, &connection_info) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Connection failed");
            break;
        }

        sockets_test_app_print_connection_info("UDP destination set to", &connection_info);

        success = true;

    } while(false);

    return success;
}

static void sockets_test_app_deinit_sockets(SocketsTestApp* instance) {
    for(uint32_t i = 0; i < SocketIndexMax; ++i) {
        Socket* socket = instance->sockets[i];
        if(socket) {
            if(socket_free(socket) != SocketStatusOk) {
                FURI_LOG_E(TAG, "Failed to free socket");
            }
        }
    }

    furi_record_close(RECORD_SOCKETS);
}

static void sockets_test_app_deinit_wifi(SocketsTestApp* instance) {
    if(wifi_disconnect(instance->wifi) != WifiStatusOk) {
        FURI_LOG_E(TAG, "Failed to disconnect Wifi");
    }

    if(wifi_deinit(instance->wifi) != WifiStatusOk) {
        FURI_LOG_E(TAG, "Failed to deinit Wifi");
    }

    furi_record_close(RECORD_WIFI);
}

static void sockets_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    SocketsTestApp* instance = context;
    furi_assert(object == instance->event_queue);

    SocketEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    const SocketEventType event_type = event.type;

    if(event_type == SocketEventTypeReceive) {
        SocketStatus status;

        do {
            size_t data_size;
            status = socket_receive(event.socket, instance->tmp_buf, MAX_CHUNK_SIZE, &data_size);

            if(status != SocketStatusOk) {
                FURI_LOG_E(TAG, "Failed to receive the data");
                break;
            }

            status = socket_send(event.socket, instance->tmp_buf, data_size, &data_size);

            if(status != SocketStatusOk) {
                FURI_LOG_E(TAG, "Failed to send %zu bytes", data_size);
                break;
            }

        } while(false);

        if(status != SocketStatusOk) {
            furi_event_loop_stop(instance->event_loop);
        }

    } else if(event_type == SocketEventTypeAccept) {
        const SocketAcceptEvent* accept_event = &event.accept;
        const SocketConnectionInfo* connection_info = &accept_event->connection_info;

        sockets_test_app_print_connection_info("Accepting new connection from", connection_info);

        Socket* client_socket = accept_event->client_socket;
        instance->sockets[SocketIndexTcpRemoteClient] = client_socket;

    } else if(event_type == SocketEventTypeClose) {
        FURI_LOG_I(TAG, "Socket closed!");
        furi_event_loop_stop(instance->event_loop);
    }
}

static SocketsTestApp* sockets_test_app_alloc(void) {
    SocketsTestApp* instance = malloc(sizeof(SocketsTestApp));

    instance->wifi = furi_record_open(RECORD_WIFI);
    instance->sockets_srv = furi_record_open(RECORD_SOCKETS);
    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(8, sizeof(SocketEvent));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        sockets_test_app_event_queue_callback,
        instance);

    return instance;
}

static void sockets_test_app_free(SocketsTestApp* instance) {
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t sockets_test_app(void* arg) {
    UNUSED(arg);

    SocketsTestApp* instance = sockets_test_app_alloc();

    do {
        if(!sockets_test_app_init_wifi(instance)) break;
        if(!sockets_test_app_init_tcp_client(instance)) break;
        if(!sockets_test_app_init_tcp_server(instance)) break;
        if(!sockets_test_app_init_udp_client_server(instance)) break;

        furi_event_loop_run(instance->event_loop);

    } while(false);

    sockets_test_app_deinit_sockets(instance);
    sockets_test_app_deinit_wifi(instance);
    sockets_test_app_free(instance);

    return 0;
}
