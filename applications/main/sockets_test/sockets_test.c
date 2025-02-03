#include <furi.h>

#include <wifi/wifi.h>
#include <sockets/sockets.h>

#define TAG "SocketsTestApp"

#define RX_BUFFER_SIZE (2048UL)
#define MAX_CHUNK_SIZE (1014) // Limited by intercom et al

#define WIFI_SSID "Your SSID"
#define WIFI_PASS "Your passphrase"
#define WIFI_MODE WifiSecurityModeWpa2

#define CONNECT_PORT    8080
#define LISTEN_PORT     8081
#define CONNECT_ADDRESS 10, 46, 30, 131 // MUST be commas, not dots

typedef enum {
    SocketIndexClient,
    SocketIndexServer,
    SocketIndexRemoteClient,
    SocketIndexMax,
} SocketIndex;

typedef struct {
    Wifi* wifi;
    SocketSrv* sockets_srv;
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriStreamBuffer* buffers[SocketIndexMax];
    Socket* sockets[SocketIndexMax];
    SocketConnectionInfo bind_info;
} SocketsTestApp;

static void sockets_test_app_stream_buffer_callback(FuriEventLoopObject* object, void* context);

static inline FuriStreamBuffer*
    sockets_test_app_get_buffer_by_socket(const SocketsTestApp* instance, const Socket* socket) {
    FuriStreamBuffer* ret = NULL;

    for(uint32_t i = 0; i < SocketIndexMax; ++i) {
        if(socket == instance->sockets[i]) {
            ret = instance->buffers[i];
            break;
        }
    }

    return ret;
}

static inline Socket* sockets_test_app_get_socket_by_buffer(
    const SocketsTestApp* instance,
    const FuriStreamBuffer* buffer) {
    Socket* ret = NULL;

    for(uint32_t i = 0; i < SocketIndexMax; ++i) {
        if(instance->buffers[i] == buffer) {
            ret = instance->sockets[i];
            break;
        }
    }

    return ret;
}

static void socket_event_callback(Socket* socket, const SocketEvent* event, void* context) {
    furi_assert(socket);
    furi_assert(event);
    furi_assert(context);

    SocketsTestApp* instance = context;

    // Special case: Do not queue event, put the data in the stream buffer
    if(event->type == SocketEventTypeReceive) {
        FuriStreamBuffer* buf = sockets_test_app_get_buffer_by_socket(instance, socket);
        furi_check(buf, "Invalid stream buffer");

        const size_t data_size = event->receive.data_size;
        furi_check(
            furi_stream_buffer_send(buf, event->receive.data, data_size, FuriWaitForever) ==
            data_size);

    } else {
        furi_check(
            furi_message_queue_put(instance->event_queue, event, FuriWaitForever) == FuriStatusOk);
    }
}

static bool sockets_test_app_init_wifi(SocketsTestApp* instance) {
    UNUSED(instance);

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

        WifiInfo wifi_info;

        if(wifi_get_info(instance->wifi, &wifi_info) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get Wifi info");
            break;
        }

        instance->bind_info.port = LISTEN_PORT;
        instance->bind_info.ip_type = wifi_info.ip_config.type;
        memcpy(
            &instance->bind_info.address,
            &wifi_info.ip_config.address,
            sizeof(instance->bind_info.address));

        success = true;

    } while(false);

    return success;
}

static void sockets_test_app_init_rx_buffer(SocketsTestApp* instance, SocketIndex socket_index) {
    instance->buffers[socket_index] = furi_stream_buffer_alloc(RX_BUFFER_SIZE, 1);

    furi_event_loop_subscribe_stream_buffer(
        instance->event_loop,
        instance->buffers[socket_index],
        FuriEventLoopEventIn,
        sockets_test_app_stream_buffer_callback,
        instance);
}

static bool sockets_test_app_init_client(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "TCP client initialisation start ...");

        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolTcp,
        };

        Socket* socket;
        Socket** socket_slot = &instance->sockets[SocketIndexClient];

        *socket_slot = socket_alloc(instance->sockets_srv, &socket_info);
        socket = *socket_slot;

        if(socket == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate client socket");
            break;
        }

        FURI_LOG_I(TAG, "Client socket allocated successfully!");

        sockets_test_app_init_rx_buffer(instance, SocketIndexClient);
        socket_set_event_callback(socket, socket_event_callback, instance);

        const SocketConnectionInfo connection_info = {
            .port = CONNECT_PORT,
            .ip_type = SocketIpTypeV4,
            .address.v4 = {CONNECT_ADDRESS},
        };

        if(socket_connect(socket, &connection_info) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Connection failed");
            break;
        }

        FURI_LOG_I(
            TAG,
            "Connected to %hhu.%hhu.%hhu.%hhu:%hu!",
            connection_info.address.v4[0],
            connection_info.address.v4[1],
            connection_info.address.v4[2],
            connection_info.address.v4[3],
            connection_info.port);

        success = true;

    } while(false);

    return success;
}

static bool sockets_test_app_init_server(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "TCP server initialisation start ...");

        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolTcp,
        };

        Socket* socket;
        Socket** socket_slot = &instance->sockets[SocketIndexServer];

        *socket_slot = socket_alloc(instance->sockets_srv, &socket_info);
        socket = *socket_slot;

        if(socket == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate Server socket");
            break;
        }

        FURI_LOG_I(TAG, "Server socket allocated successfully!");

        socket_set_event_callback(socket, socket_event_callback, instance);

        if(socket_accept(socket, &instance->bind_info) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Server start failed");
            break;
        }

        FURI_LOG_I(
            TAG,
            "Listening on %hhu.%hhu.%hhu.%hhu:%hu ...",
            instance->bind_info.address.v4[0],
            instance->bind_info.address.v4[1],
            instance->bind_info.address.v4[2],
            instance->bind_info.address.v4[3],
            instance->bind_info.port);

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

        FuriStreamBuffer* buffer = instance->buffers[i];
        if(buffer) {
            furi_event_loop_unsubscribe(instance->event_loop, buffer);
            furi_stream_buffer_free(buffer);
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

    if(event.type == SocketEventTypeSend) {
        // Won't run
        FURI_LOG_I(TAG, "%hu bytes confirmed!", event.send.data_size);

    } else if(event.type == SocketEventTypeAccept) {
        const SocketConnectionInfo* connection_info = &event.accept.connection_info;

        FURI_LOG_I(
            TAG,
            "Accepting new connection from %hhu.%hhu.%hhu.%hhu:%hu",
            connection_info->address.v4[0],
            connection_info->address.v4[1],
            connection_info->address.v4[2],
            connection_info->address.v4[3],
            connection_info->port);

        Socket* client_socket = event.accept.client_socket;
        instance->sockets[SocketIndexRemoteClient] = client_socket;

        sockets_test_app_init_rx_buffer(instance, SocketIndexRemoteClient);
        socket_set_event_callback(client_socket, socket_event_callback, instance);

    } else if(event.type == SocketEventTypeClose) {
        FURI_LOG_I(TAG, "Socket closed!");
        furi_event_loop_stop(instance->event_loop);
    }
}

static void sockets_test_app_stream_buffer_callback(FuriEventLoopObject* object, void* context) {
    FURI_LOG_I(TAG, "Data received!");

    SocketsTestApp* instance = context;
    FuriStreamBuffer* buffer = object;
    Socket* socket = sockets_test_app_get_socket_by_buffer(instance, buffer);

    furi_check(socket, "Invalid socket");

    static char tmp[MAX_CHUNK_SIZE + 1]; // + space for null terminator
    const size_t rx_size = furi_stream_buffer_receive(buffer, tmp, sizeof(tmp) - 1, 0);
    furi_check(rx_size);

    // Terminate and print string
    tmp[rx_size] = 0;
    FURI_LOG_I(TAG, "Data: %s", tmp);

    size_t tx_size;
    if((socket_send(socket, tmp, rx_size, &tx_size) != SocketStatusOk) || (tx_size != rx_size)) {
        FURI_LOG_E(TAG, "Failed to send %zu bytes", tx_size);
        furi_event_loop_stop(instance->event_loop);
    }

    FURI_LOG_D(TAG, "Echo'd %zu bytes!", tx_size);
}

static SocketsTestApp* sockets_test_app_alloc(void) {
    SocketsTestApp* instance = malloc(sizeof(SocketsTestApp));

    instance->wifi = furi_record_open(RECORD_WIFI);
    instance->sockets_srv = furi_record_open(RECORD_SOCKETS);
    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(SocketEvent));

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
        if(!sockets_test_app_init_client(instance)) break;
        if(!sockets_test_app_init_server(instance)) break;

        furi_event_loop_run(instance->event_loop);

    } while(false);

    sockets_test_app_deinit_sockets(instance);
    sockets_test_app_deinit_wifi(instance);
    sockets_test_app_free(instance);

    return 0;
}
