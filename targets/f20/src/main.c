#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_nvm.h>

#include <flipper.h>

#include <platform_startup.h>

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

    // Critical FURI HAL
    furi_hal_init_early();

    FuriThread* main_thread = furi_thread_alloc_ex("Init", 4096, init_task, NULL);
    furi_thread_set_priority(main_thread, FuriThreadPriorityInit);
#ifdef FURI_RAM_EXEC
    furi_thread_start(main_thread);
#else
    FuriHalNvmBootMode boot_mode = furi_hal_nvm_get_boot_mode();
    if(boot_mode == FuriHalNvmBootModeUpdate || true) {
        platform_boot_exec_update();
        // If we are here, the switch to the update was not successful
        // FURI_LOG_W(TAG, "Failed to switch to update mode");
        furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeNormal);
        furi_hal_power_reset();
    } else {
        furi_thread_start(main_thread);
    }

#endif
    // Run Kernel
    furi_run();

    furi_crash("Kernel is Dead");
}

void abort(void) {
    furi_crash("AbortHandler");
}
