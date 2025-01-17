#include <furi.h>

#include <wifi/wifi.h>
#include <sockets/sockets.h>

#define TAG "SocketsTestApp"

#define RX_BUFFER_SIZE (2048UL)

#define WIFI_SSID "Your SSID"
#define WIFI_PASS "Your passphrase"
#define WIFI_MODE WifiSecurityModeWpa2

#define CONNECT_PORT    8080
#define LISTEN_PORT     8081
#define CONNECT_ADDRESS 10, 46, 30, 131 // MUST be commas, not dots

typedef enum {
    SocketsTestAppIndexClient,
    SocketsTestAppIndexServer,
    SocketsTestAppIndexRemoteClient,
    SocketsTestAppIndexMax,
} SocketsTestAppIndex;

typedef enum {
    SocketsTestCustomEventReceive = 1UL << 0,
} SocketsTestCustomEvent;

typedef struct {
    Wifi* wifi;
    Sockets* sockets_srv;
    FuriEventLoop* event_loop;
    Socket* sockets[SocketsTestAppIndexMax];
    SocketConnectionInfo bind_info;
    uint8_t buf[RX_BUFFER_SIZE];
} SocketsTestApp;

static void socket_event_callback(Socket* socket, const SocketEvent* event, void* context) {
    UNUSED(socket);

    SocketsTestApp* instance = context;

    if(event->type == SocketEventTypeSend) {
        FURI_LOG_I(TAG, "Sent %hu byte(s)", event->data_size);

    } else if(event->type == SocketEventTypeReceive) {
        furi_event_loop_set_custom_event(instance->event_loop, SocketsTestCustomEventReceive);

    } else if(event->type == SocketEventTypeAccept) {
        const SocketConnectionInfo* connection_info = &event->accept.connection_info;
        FURI_LOG_I(
            TAG,
            "Accepting new connection from %hhu.%hhu.%hhu.%hhu:%hu",
            connection_info->address.v4[0],
            connection_info->address.v4[1],
            connection_info->address.v4[2],
            connection_info->address.v4[3],
            connection_info->port);

        Socket* client_socket = event->accept.client_socket;
        socket_set_event_callback(client_socket, socket_event_callback, instance);

        instance->sockets[SocketsTestAppIndexRemoteClient] = client_socket;

    } else if(event->type == SocketEventTypeClose) {
        FURI_LOG_I(TAG, "Socket closed!");
        furi_event_loop_stop(instance->event_loop);
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

static bool sockets_test_app_init_client(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "TCP client initialisation start ...");

        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolTcp,
        };

        Socket* socket;
        Socket** socket_slot = &instance->sockets[SocketsTestAppIndexClient];

        *socket_slot = socket_alloc(instance->sockets_srv, &socket_info);
        socket = *socket_slot;

        if(socket == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate client socket");
            break;
        }

        FURI_LOG_I(TAG, "Client socket allocated successfully!");

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
        Socket** socket_slot = &instance->sockets[SocketsTestAppIndexServer];

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
    for(uint32_t i = 0; i < SocketsTestAppIndexMax; ++i) {
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

static bool sockets_test_app_send_buffer(Socket* socket, const uint8_t* data, size_t data_size) {
    size_t total_size;

    for(total_size = 0; total_size < data_size;) {
        const void* tx_ptr = data + total_size;
        const size_t tx_size = data_size - total_size;

        size_t tx_size_actual;

        if(socket_send(socket, tx_ptr, tx_size, &tx_size_actual) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to send %zu bytes", tx_size_actual);
            break;
        }

        total_size += tx_size_actual;
    }

    return total_size == data_size;
}

static bool sockets_test_app_echo(uint8_t* buf, size_t buffer_size, Socket* socket) {
    bool success = false;

    for(;;) {
        size_t rx_size;

        if(socket_receive(socket, buf, buffer_size, &rx_size) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to get received data");
            break;
        }

        if(rx_size == 0) {
            success = true;
            break;
        }

        if(!sockets_test_app_send_buffer(socket, buf, rx_size)) {
            FURI_LOG_E(TAG, "Failed to echo received data");
            break;
        }
    }

    return success;
}

static void sockets_test_app_custom_event_callback(uint32_t events, void* context) {
    SocketsTestApp* instance = context;

    if(events) {
        FURI_LOG_I(TAG, "Data received!");

        for(uint32_t i = 0; i < SocketsTestAppIndexMax; ++i) {
            Socket* socket = instance->sockets[i];
            if(!socket) continue;
            if(!sockets_test_app_echo(instance->buf, RX_BUFFER_SIZE, socket)) {
                furi_event_loop_stop(instance->event_loop);
                return;
            }
        }
    }
}

static SocketsTestApp* sockets_test_app_alloc(void) {
    SocketsTestApp* instance = malloc(sizeof(SocketsTestApp));

    instance->wifi = furi_record_open(RECORD_WIFI);
    instance->sockets_srv = furi_record_open(RECORD_SOCKETS);
    instance->event_loop = furi_event_loop_alloc();
    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sockets_test_app_custom_event_callback, instance);

    return instance;
}

static void sockets_test_app_free(SocketsTestApp* instance) {
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
