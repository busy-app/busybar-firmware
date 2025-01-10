#include "sockets.h"
#include "sockets_common_i.h"

#include <furi.h>
#include <intercom/intercom.h>

#define TAG "Sockets"

typedef struct {
    SocketRequestType request_type;
} SocketsMessage;

struct Sockets {
    FuriEventLoop* event_loop;
    Intercom* intercom;
    SocketsMessage* message;
    SocketRequest request;
    SocketResponse response;
};

static void sockets_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    UNUSED(data);
    UNUSED(data_size);
    UNUSED(context);
}

Sockets* sockets_alloc(void) {
    Sockets* instance = malloc(sizeof(Sockets));

    instance->event_loop = furi_event_loop_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelSockets, sockets_intercom_rx_callback, instance);

    furi_record_create(RECORD_SOCKETS, instance);

    return instance;
}

int32_t sockets_srv(void* arg) {
    UNUSED(arg);

    Sockets* instance = sockets_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
