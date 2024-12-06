#include <furi.h>

#include <rpc/rpc_i.h>
#include <intercom/intercom_rpc.h>

#include <main.pb.h>
#include <stm32u5xx.h>

#include "intercom_test.h"

#define TAG "IntercomTestSrv"

typedef enum {
    IntercomTestFlagResponse = 1UL << 0,
} IntercomTestFlag;

#define INTERCOM_TEST_FLAG_ALL (IntercomTestFlagResponse)

typedef struct {
    FuriThreadId thread_id;
    RpcSession* session;
    PB_Main tx_message;
} IntercomTest;

static void intercom_test_transfer_handler(const PB_Main* message, void* context) {
    furi_assert(context);
    IntercomTest* instance = context;

    const pb_bytes_array_t* rx_buffer = message->content.transfer_request.buffer;
    const pb_bytes_array_t* tx_buffer = instance->tx_message.content.transfer_request.buffer;

    furi_check(memcmp(tx_buffer, rx_buffer, PB_BYTES_ARRAY_T_ALLOCSIZE(BUFFER_SIZE)) == 0);

    furi_thread_flags_set(instance->thread_id, IntercomTestFlagResponse);
}

static void intercom_test_empty_handler(const PB_Main* message, void* context) {
    UNUSED(message);
    UNUSED(context);
    furi_crash("Empty received");
}

static void intercom_fill_tx_message(PB_Main* message) {
    message->which_content = PB_Main_transfer_request_tag;
    PB_Debug_TransferRequest* content = &message->content.transfer_request;

    content->buffer = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(BUFFER_SIZE));
    content->buffer->size = BUFFER_SIZE;

    for(uint32_t i = 0; i < BUFFER_SIZE; ++i) {
        content->buffer->bytes[i] = i % 0xff;
    }
}

static IntercomTest* intercom_test_alloc(void) {
    IntercomTest* instance = malloc(sizeof(IntercomTest));

    instance->thread_id = furi_thread_get_current_id();
    instance->session = furi_record_open(RECORD_INTERCOM_RPC);

    RpcHandler handler = {
        .context = instance,
    };

    handler.message_handler = intercom_test_transfer_handler,
    rpc_add_handler(instance->session, PB_Main_transfer_request_tag, &handler);

    handler.message_handler = intercom_test_empty_handler;
    rpc_add_handler(instance->session, PB_Main_empty_tag, &handler);

    intercom_fill_tx_message(&instance->tx_message);

    return instance;
}

int32_t intercom_test_srv(void* arg) {
    UNUSED(arg);

    IntercomTest* instance = intercom_test_alloc();

    uint32_t total_time_us = 0;
    uint32_t iteration_count = 0;

    uint32_t start;

    for(;;) {
        start = DWT->CYCCNT;

        rpc_send(instance->session, &instance->tx_message);

        const uint32_t flags =
            furi_thread_flags_wait(INTERCOM_TEST_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0);

        const uint32_t now = DWT->CYCCNT;
        const uint32_t time_elapsed_us = (now - start) / (SystemCoreClock / 1000000UL);

        iteration_count++;
        total_time_us += time_elapsed_us;

        if(total_time_us >= 500000UL) {
            FURI_LOG_I(TAG, "Avg. roundtrip time: %lu us", total_time_us / iteration_count);

            total_time_us = 0;
            iteration_count = 0;
        }

        start = now;

        furi_delay_ms(10);
    }

    return 0;
}
