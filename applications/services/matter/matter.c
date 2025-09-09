#include <intercom/intercom.h>
#include <toolbox/api_lock.h>

#include "matter.h"
#include "matter_common_i.h"

#define TAG            "MatterSrv"
#define FRAME_Q_SIZE   4
#define REQUEST_Q_SIZE 1

struct MatterSrv {
    FuriEventLoop* event_loop;
    FuriMessageQueue* frame_queue;
    FuriMessageQueue* request_queue;
    FuriPubSub* pubsub;

    MatterVirtualDeviceState device_state[MatterVirtualDeviceMAX];

    Intercom* intercom;
};

// =========
// Utilities
// =========

static void matter_send_state_update(MatterSrv* matter, MatterVirtualDeviceState state) {
    MatterEvent event = {
        .type = MatterEventTypeStateUpdate,
        .update = {
            .new_state = state,
        },
    };
    furi_pubsub_publish(matter->pubsub, &event);
}

// ======================
// Communication with f64
// ======================

static void matter_forward_frame_to_thread(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(data_size == sizeof(MatterIntercomFrame));
    furi_check(context);
    MatterSrv* matter = context;
    const MatterIntercomFrame* frame = data;

    furi_check(furi_message_queue_put(matter->frame_queue, frame, 0) == FuriStatusOk);
}

static void matter_handle_frame(FuriEventLoopObject* object, void* context) {
    furi_assert(object);
    furi_assert(context);
    MatterSrv* matter = context;
    FuriMessageQueue* queue = object;
    furi_assert(queue == matter->frame_queue);

    MatterIntercomFrame frame;
    furi_check(furi_message_queue_get(queue, &frame, 0) == FuriStatusOk);

    if(frame.type == MatterIntercomFrameTypeUpdate) {
        MatterIntercomUpdateFrame* update = &frame.update;
        MatterVirtualDeviceState state = update->new_state;
        matter->device_state[state.device] = state;
        matter_send_state_update(matter, state);

    } else {
        furi_crash();
    }
}

static void matter_send_frame(MatterSrv* matter, const MatterIntercomFrame* frame) {
    furi_check(intercom_tx(matter->intercom, IntercomChannelMatter, frame, sizeof(*frame), FuriWaitForever) == sizeof(*frame));
}

// ==========
// Public API
// ==========

typedef enum {
    MatterApiRequestTypeGetState,
    MatterApiRequestTypeSetState,
    MatterApiRequestTypeReset,
} MatterApiRequestType;

typedef struct {
    FuriApiLock lock;
    MatterApiRequestType type;
    MatterVirtualDevice device;
    MatterVirtualDeviceState state;
} MatterApiRequest;

static void matter_handle_api_request(FuriEventLoopObject* object, void* context) {
    furi_assert(object);
    furi_assert(context);
    MatterSrv* matter = context;
    FuriMessageQueue* queue = object;
    furi_assert(queue == matter->request_queue);

    MatterApiRequest* request;
    furi_check(furi_message_queue_get(queue, &request, 0) == FuriStatusOk);
    furi_check(request);

    switch(request->type) {
    case MatterApiRequestTypeGetState: {
        request->state = matter->device_state[request->device];
        break;
    }

    case MatterApiRequestTypeSetState: {
        matter->device_state[request->device] = request->state;
        MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeRequest,
            .request = {
                .req_state = request->state,
            },
        };
        matter_send_frame(matter, &frame);
        break;
    }

    case MatterApiRequestTypeReset: {
        MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeReset,
        };
        matter_send_frame(matter, &frame);
        break;
    }
    }

    api_lock_unlock(request->lock);
}

static void matter_synchronous_request(MatterSrv* matter, MatterApiRequest* request) {
    request->lock = api_lock_alloc_locked();
    furi_check(furi_message_queue_put(matter->request_queue, &request, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(request->lock);
}

FuriPubSub* matter_get_pubsub(MatterSrv* matter) {
    furi_check(matter);
    return matter->pubsub;
}

MatterVirtualDeviceState matter_get_state(MatterSrv* matter, MatterVirtualDevice device) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeGetState,
        .device = device,
    };
    matter_synchronous_request(matter, &request);
    return request.state;
}

void matter_set_state(MatterSrv* matter, MatterVirtualDeviceState state) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeSetState,
        .state = state,
        .device = state.device,
    };
    matter_synchronous_request(matter, &request);
}

void matter_factory_reset(MatterSrv* matter) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeReset,
    };
    matter_synchronous_request(matter, &request);
}

// =============
// Service setup
// =============

MatterSrv* matter_srv_alloc(void) {
    MatterSrv* matter = malloc(sizeof(MatterSrv));

    matter->event_loop = furi_event_loop_alloc();

    matter->frame_queue = furi_message_queue_alloc(FRAME_Q_SIZE, sizeof(MatterIntercomFrame));
    furi_event_loop_subscribe_message_queue(matter->event_loop, matter->frame_queue, FuriEventLoopEventIn, matter_handle_frame, matter);

    matter->request_queue = furi_message_queue_alloc(REQUEST_Q_SIZE, sizeof(MatterApiRequest*));
    furi_event_loop_subscribe_message_queue(matter->event_loop, matter->request_queue, FuriEventLoopEventIn, matter_handle_api_request, matter);

    matter->pubsub = furi_pubsub_alloc();

    matter->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(matter->intercom, IntercomChannelMatter, matter_forward_frame_to_thread, matter);

    furi_record_create(RECORD_MATTER, matter);
    return matter;
}

int matter_srv(void* arg) {
    UNUSED(arg);

    MatterSrv* matter = matter_srv_alloc();
    furi_event_loop_run(matter->event_loop);

    return 0;
}
