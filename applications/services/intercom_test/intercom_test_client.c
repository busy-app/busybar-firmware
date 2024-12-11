#include <furi.h>

#include <intercom/intercom.h>

#include <stm32u5xx.h>

#include "intercom_test.h"

#define TAG "IntercomTestSrv"

typedef enum {
    IntercomTestFlagResponse = 1UL << 0,
} IntercomTestFlag;

#define INTERCOM_TEST_FLAG_ALL (IntercomTestFlagResponse)

typedef struct {
    FuriThreadId thread_id;
    Intercom* intercom;
    uint8_t buffer[BUFFER_SIZE];
} IntercomTest;

static void intercom_test_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size == BUFFER_SIZE);

    IntercomTest* instance = context;

    furi_check(memcmp(instance->buffer, data, BUFFER_SIZE) == 0);

    furi_thread_flags_set(instance->thread_id, IntercomTestFlagResponse);
}

static IntercomTest* intercom_test_alloc(void) {
    IntercomTest* instance = malloc(sizeof(IntercomTest));

    instance->thread_id = furi_thread_get_current_id();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    for(uint32_t i = 0; i < BUFFER_SIZE; ++i) {
        instance->buffer[i] = (uint8_t)i;
    }

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelDebug, intercom_test_rx_callback, instance);

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

        const size_t tx_size = intercom_tx(
            instance->intercom,
            IntercomChannelDebug,
            instance->buffer,
            BUFFER_SIZE,
            FuriWaitForever);

        furi_check(tx_size == BUFFER_SIZE);

        const uint32_t flags =
            furi_thread_flags_wait(INTERCOM_TEST_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0);

        const uint32_t now = DWT->CYCCNT;
        const uint32_t time_elapsed_us = (now - start) / (SystemCoreClock / 1000000UL);

        iteration_count++;
        total_time_us += time_elapsed_us;

        if(total_time_us >= 500000UL) {
            const uint32_t roundrip_time_us = total_time_us / iteration_count;
            const uint32_t bit_s = ((uint64_t)BUFFER_SIZE * 160000000UL) / roundrip_time_us;
            FURI_LOG_I(TAG, "Avg. roundtrip time: %lu us (%lu bit/s)", roundrip_time_us, bit_s);

            total_time_us = 0;
            iteration_count = 0;
        }

        start = now;
    }

    return 0;
}
