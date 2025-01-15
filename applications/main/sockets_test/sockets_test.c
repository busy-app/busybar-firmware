#include <furi.h>

#include <wifi/wifi.h>
#include <sockets/sockets.h>

#define TAG "SocketsTestApp"

#define WIFI_SSID "Your SSID"
#define WIFI_PASS "Your passphrase"
#define WIFI_MODE WifiSecurityModeWpa2

#define SERVER_PORT    8080
#define SERVER_ADDRESS 10, 46, 30, 131 // MUST be commas, not dots

typedef enum {
    SocketsTestAppEventSendComplete = 1UL << 0,
    SocketsTestAppEventReceiveReady = 1UL << 1,
    SocketsTestAppEventClosed = 1UL << 2,
} SocketsTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    Socket* socket;
    uint8_t buf[2048];
} SocketsTestApp;

static void socket_event_callback(Socket* socket, const SocketEvent* event, void* context) {
    UNUSED(socket);

    SocketsTestApp* instance = context;
    const SocketEventType event_type = event->type;

    uint32_t custom_event;

    if(event_type == SocketEventTypeSendComplete) {
        custom_event = SocketsTestAppEventSendComplete;
    } else if(event_type == SocketEventTypeReceiveReady) {
        custom_event = SocketsTestAppEventReceiveReady;
    } else if(event_type == SocketEventTypeClosed) {
        custom_event = SocketsTestAppEventClosed;
    } else {
        furi_crash("Invalid SocketEventType value");
    }

    furi_event_loop_set_custom_event(instance->event_loop, custom_event);
}

static bool sockets_test_app_init_wifi(SocketsTestApp* instance) {
    UNUSED(instance);

    bool success = false;

    do {
        FURI_LOG_I(TAG, "Wifi initialisation start ...");

        Wifi* wifi = furi_record_open(RECORD_WIFI);

        if(wifi_init(wifi) != WifiStatusOk) {
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

        if(wifi_connect(wifi, &credentials, &ip_config) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to connect to Wifi network");
            break;
        }

        FURI_LOG_I(TAG, "Wifi connected to %s!", WIFI_SSID);
        success = true;

    } while(false);

    return success;
}

static bool sockets_test_app_init_client(SocketsTestApp* instance) {
    bool success = false;

    do {
        FURI_LOG_I(TAG, "TCP client initialisation start ...");

        Sockets* sockets = furi_record_open(RECORD_SOCKETS);

        const SocketInfo socket_info = {
            .ip_type = SocketIpTypeV4,
            .protocol = SocketProtocolTcp,
        };

        instance->socket = socket_alloc(sockets, &socket_info);

        if(instance->socket == NULL) {
            FURI_LOG_E(TAG, "Failed to allocate socket");
            break;
        }

        FURI_LOG_I(TAG, "Socket allocated successfully!");

        socket_set_event_callback(instance->socket, socket_event_callback, instance);

        const SocketConnectionInfo connection_info = {
            .port = SERVER_PORT,
            .ip_type = SocketIpTypeV4,
            .address.v4 = {SERVER_ADDRESS},
        };

        if(socket_connect(instance->socket, &connection_info) != SocketStatusOk) {
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

static void sockets_test_app_deinit_client(SocketsTestApp* instance) {
    if(instance->socket) {
        if(socket_free(instance->socket) != SocketStatusOk) {
            FURI_LOG_E(TAG, "Failed to free socket");
        }
    }

    furi_record_close(RECORD_SOCKETS);
}

static void sockets_test_app_custom_event_callback(uint32_t events, void* context) {
    SocketsTestApp* instance = context;

    if(events & SocketsTestAppEventClosed) {
        FURI_LOG_I(TAG, "Socket closed!");

        furi_event_loop_stop(instance->event_loop);
        return;
    }

    if(events & SocketsTestAppEventReceiveReady) {
        FURI_LOG_I(TAG, "Data received!");

        size_t rx_size;

        do {
            socket_receive(instance->socket, instance->buf, sizeof(instance->buf), &rx_size);
            if(socket_send(instance->socket, instance->buf, rx_size, NULL) != SocketStatusOk) {
                FURI_LOG_E(TAG, "Failed to send %zu bytes", rx_size);

                furi_event_loop_stop(instance->event_loop);
                return;
            }

        } while(rx_size > 0);
    }

    if(events & SocketsTestAppEventSendComplete) {
        FURI_LOG_I(TAG, "Send complete!");
    }
}

static SocketsTestApp* sockets_test_app_alloc(void) {
    SocketsTestApp* instance = malloc(sizeof(SocketsTestApp));

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

        furi_event_loop_run(instance->event_loop);

    } while(false);

    sockets_test_app_deinit_client(instance);
    sockets_test_app_free(instance);

    return 0;
}
