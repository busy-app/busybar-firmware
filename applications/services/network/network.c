#include "network.h"

#include <furi.h>

#include <lwip/api.h>
#include <lwip/tcpip.h>

#define TAG "Network"

static void network_tcpip_init_done_callback(void* arg) {
    furi_assert(arg);
    FuriSemaphore* lwip_start_sem = arg;
    furi_semaphore_release(lwip_start_sem);
}

void network_init_current_thread(Network* instance) {
    UNUSED(instance);
    netconn_thread_init();
}

void network_deinit_current_thread(Network* instance) {
    UNUSED(instance);
    netconn_thread_cleanup();
}

void network_on_system_start(void) {
    FuriSemaphore* lwip_start_sem = furi_semaphore_alloc(1, 0);

    tcpip_init(network_tcpip_init_done_callback, lwip_start_sem);
    furi_check(furi_semaphore_acquire(lwip_start_sem, FuriWaitForever) == FuriStatusOk);

    furi_semaphore_free(lwip_start_sem);

    furi_record_create(RECORD_NETWORK, NULL);
}
