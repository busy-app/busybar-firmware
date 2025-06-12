#include "applications.h"

#include <flipper.h>
#include <furi.h>

#define TAG "SvcStartup"

void flipper_init_services(void) {
    FURI_LOG_I(TAG, "Starting %d services", FLIPPER_SERVICES_COUNT);

    for(size_t i = 0; i < FLIPPER_SERVICES_COUNT; i++) {
        FURI_LOG_D(TAG, "Starting service %s", FLIPPER_SERVICES[i].name);

        FuriThread* thread = furi_thread_alloc_service(
            FLIPPER_SERVICES[i].name,
            FLIPPER_SERVICES[i].stack_size,
            FLIPPER_SERVICES[i].app,
            NULL);
        furi_thread_set_appid(thread, FLIPPER_SERVICES[i].appid);

        furi_thread_start(thread);
    }
}
