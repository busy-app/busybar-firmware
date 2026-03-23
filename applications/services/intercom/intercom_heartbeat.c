#include "intercom_i.h"

#define TAG "IntercomHeartbeat"

#define INTERCOM_HEARTBEAT_THREAD_NAME       TAG "Srv"
#define INTERCOM_HEARTBEAT_THREAD_STACK_SIZE (768)

#define INTERCOM_HEARTBEAT_PERIOD_MS (5000)

static int32_t intercom_heartbeat_thread(void* arg) {
    furi_assert(arg);
    Intercom* instance = arg;

    for(;;) {
        intercom_meta_send_heartbeat(instance);
        furi_delay_ms(INTERCOM_HEARTBEAT_PERIOD_MS);
    }

    return 0;
}

void intercom_start_heartbeat_thread(Intercom* instance) {
    FuriThread* heartbeat_thread = furi_thread_alloc_service(
        INTERCOM_HEARTBEAT_THREAD_NAME,
        INTERCOM_HEARTBEAT_THREAD_STACK_SIZE,
        intercom_heartbeat_thread,
        instance);
    furi_thread_start(heartbeat_thread);
}
