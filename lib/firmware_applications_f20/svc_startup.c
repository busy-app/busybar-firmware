#include "applications.h"

#include <flipper.h>
#include <furi.h>

#include <furi_hal_sdmmc.h>

#define TAG "SvcStartup"

void flipper_init_services(void) {
    static uint8_t buffer[512];

    FURI_LOG_I(TAG, "Starting %d services", FLIPPER_SERVICES_COUNT);

    bool read_ok = false;
    for(size_t i = 0; i < FLIPPER_SERVICES_COUNT; i++) {
        FURI_LOG_D(TAG, "Starting service %s", FLIPPER_SERVICES[i].name);

        FuriThread* thread = furi_thread_alloc_service(
            FLIPPER_SERVICES[i].name,
            FLIPPER_SERVICES[i].stack_size,
            FLIPPER_SERVICES[i].app,
            NULL);
        furi_thread_set_appid(thread, FLIPPER_SERVICES[i].appid);

        furi_thread_start(thread);

        if(!read_ok) {
            furi_delay_ms(300);
        }
        read_ok =
            read_ok ||
            furi_hal_sdmmc_read_blocks(buffer, 0, 1, 1000); // Dummy read to initialize the SD card

        if(!read_ok) {
            FURI_LOG_E(TAG, "Failed to read from SD card");
        } else {
            FURI_LOG_I(TAG, "SD card alive");
        }
    }
}
