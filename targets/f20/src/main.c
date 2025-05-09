#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>

#include <stm32u5xx_ll_cortex.h>
#include <stm32u5xx_ll_system.h>
#include <stm32u5xx_ll_pwr.h>
#include <stm32u5xx_ll_utils.h>
#include <furi_hal_clock.h>

#define TAG "Main"

int32_t init_task(void* context) {
    UNUSED(context);

    // Flipper FURI HAL
    furi_hal_init();

    // Set the UART for logging output
    furi_hal_serial_control_set_logging_config(FuriHalSerialIdUsart6, 230400);

    // Init flipper
    flipper_init();

    furi_background();

    return 0;
}

int main(void) {
    // Initialize FURI layer

    furi_init();
    furi_log_set_level(FuriLogLevelDebug);

    // Flipper critical FURI HAL
    furi_hal_init_early();

    FuriThread* main_thread = furi_thread_alloc_ex("Init", 4096, init_task, NULL);

    furi_thread_start(main_thread);

    // Run Kernel
    furi_run();

    furi_crash("Kernel is Dead");
}

void abort(void) {
    furi_crash("AbortHandler");
}
