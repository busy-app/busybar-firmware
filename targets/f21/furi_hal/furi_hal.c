#include <furi_hal.h>
#include <furi_hal_nvm.h>

#define TAG "FuriHal"

void furi_hal_init_early(void) {
    furi_hal_cortex_init_early();
    furi_hal_clock_init_early();
    furi_hal_bus_init_early();
    furi_hal_dma_init_early();
    furi_hal_resources_init_early();
    furi_hal_os_init();
    furi_hal_spi_config_init_early();
    furi_hal_i2c_init_early();
    furi_hal_rtc_init_early();
    furi_hal_nvm_init_early();
}

void furi_hal_deinit_early(void) {
    furi_hal_rtc_deinit_early();
    furi_hal_i2c_deinit_early();
    furi_hal_resources_deinit_early();
    furi_hal_dma_deinit_early();
    furi_hal_bus_deinit_early();
    furi_hal_clock_deinit_early();
}

void furi_hal_init(void) {
    furi_hal_mpu_init();
    furi_hal_clock_init();
    furi_hal_nvm_init();
    furi_hal_sdmmc_init(false);
    furi_hal_random_init();
    furi_hal_serial_control_init();
    furi_hal_rtc_init();
    furi_hal_interrupt_init();
    furi_hal_flash_init();
    furi_hal_spi_config_init();
    furi_hal_i2c_init();
    furi_hal_usb_init();
    furi_hal_sai_init();
}
