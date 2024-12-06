#include <furi.h>

#include <rpc/rpc_i.h>
#include <intercom/intercom_rpc.h>

#include <main.pb.h>

#include "intercom_test.h"

typedef struct {
    RpcSession* session;
    FuriSemaphore* semaphore;
    FuriEventLoop* event_loop;
    PB_Main tx_message;
} IntercomTest;

static void intercom_test_transfer_handler(const PB_Main* message, void* context) {
    furi_assert(context);

    IntercomTest* instance = context;
    furi_check(furi_semaphore_acquire(instance->semaphore, 0) == FuriStatusOk, "Not fast enough");

    const pb_bytes_array_t* rx_buffer = message->content.transfer_request.buffer;
    furi_check(rx_buffer->size == BUFFER_SIZE, "Wrong buffer size");

    pb_bytes_array_t* tx_buffer = instance->tx_message.content.transfer_request.buffer;
    memcpy(tx_buffer->bytes, rx_buffer->bytes, BUFFER_SIZE);

    furi_semaphore_release(instance->semaphore);
}

static void intercom_test_empty_handler(const PB_Main* message, void* context) {
    UNUSED(message);
    UNUSED(context);
    furi_crash("Empty received");
}

static void intercom_test_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    IntercomTest* instance = context;
    furi_assert(instance->semaphore == object);

    rpc_send(instance->session, &instance->tx_message);
}

static IntercomTest* intercom_test_alloc(void) {
    IntercomTest* instance = malloc(sizeof(IntercomTest));

    instance->session = furi_record_open(RECORD_INTERCOM_RPC);
    instance->semaphore = furi_semaphore_alloc(1, 1);
    instance->event_loop = furi_event_loop_alloc();

    instance->tx_message.which_content = PB_Main_transfer_request_tag;
    instance->tx_message.content.transfer_request.buffer =
        malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(BUFFER_SIZE));
    instance->tx_message.content.transfer_request.buffer->size = BUFFER_SIZE;

    furi_event_loop_subscribe_semaphore(
        instance->event_loop,
        instance->semaphore,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        intercom_test_semaphore_callback,
        instance);

    RpcHandler handler = {
        .context = instance,
    };

    handler.message_handler = intercom_test_transfer_handler,
    rpc_add_handler(instance->session, PB_Main_transfer_request_tag, &handler);

    handler.message_handler = intercom_test_empty_handler;
    rpc_add_handler(instance->session, PB_Main_empty_tag, &handler);

    return instance;
}

int32_t intercom_test_srv(void* arg) {
    UNUSED(arg);

    IntercomTest* instance = intercom_test_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
