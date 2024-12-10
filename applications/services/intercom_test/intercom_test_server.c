#include <furi.h>
#include <intercom/intercom.h>

#include "intercom_test.h"

typedef struct {
    Intercom* intercom;
    FuriSemaphore* semaphore;
    FuriEventLoop* event_loop;
    uint8_t buffer[BUFFER_SIZE];
} IntercomTest;

static void intercom_test_semaphore_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    IntercomTest* instance = context;
    furi_assert(instance->semaphore == object);

    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelDebug, instance->buffer, BUFFER_SIZE, FuriWaitForever);

    furi_check(tx_size == BUFFER_SIZE);
}

static void intercom_test_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size == BUFFER_SIZE);

    IntercomTest* instance = context;

    furi_check(furi_semaphore_acquire(instance->semaphore, 0) == FuriStatusOk, "Not fast enough");

    memcpy(instance->buffer, data, BUFFER_SIZE);

    furi_semaphore_release(instance->semaphore);
}

static IntercomTest* intercom_test_alloc(void) {
    IntercomTest* instance = malloc(sizeof(IntercomTest));

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->semaphore = furi_semaphore_alloc(1, 1);
    instance->event_loop = furi_event_loop_alloc();

    furi_event_loop_subscribe_semaphore(
        instance->event_loop,
        instance->semaphore,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        intercom_test_semaphore_callback,
        instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelDebug, intercom_test_rx_callback, instance);

    return instance;
}

int32_t intercom_test_srv(void* arg) {
    UNUSED(arg);

    IntercomTest* instance = intercom_test_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
