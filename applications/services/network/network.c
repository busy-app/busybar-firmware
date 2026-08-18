#include "network.h"

#include <furi.h>

#include <lwip/api.h>
#include <lwip/tcpip.h>

#if defined(BSB_MCU_U5)
#include <mongoose_glue.h>
#include <mongoose_dns_i.h>
#endif // BSB_MCU_U5

#define TAG "Network"

static const char* const netif_names[] = {
    [NetworkNetifWifi] = "WL",
    [NetworkNetifUsb] = "EX",
};

void network_netif_assign_name(struct netif* netif, NetworkNetif id) {
    furi_assert(netif);
    furi_assert(id < NetworkNetifCount);
    const char* name = netif_names[id];
    netif->name[0] = name[0];
    netif->name[1] = name[1];
}

struct netif* network_find_netif(NetworkNetif id) {
    furi_assert(id < NetworkNetifCount);
    const char* name = netif_names[id];
    struct netif* n;
    NETIF_FOREACH(n) {
        if(n->name[0] == name[0] && n->name[1] == name[1]) {
            return n;
        }
    }
    return NULL;
}

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

#if defined(BSB_MCU_U5)
    mg_init_early();
#endif // BSB_MCU_U5

    furi_record_create(RECORD_NETWORK, NULL);

#if defined(BSB_MCU_U5)
    mongoose_dns_startup();
#endif // BSB_MCU_U5
}
