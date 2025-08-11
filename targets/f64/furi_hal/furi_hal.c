#include <furi_hal.h>
#include <sl_mbedtls.h>

void furi_hal_init_early(void) {
    furi_hal_cortex_init_early();
    furi_hal_clock_init_early();
    furi_hal_bus_init_early();
    furi_hal_dma_init_early();
    furi_hal_resources_init_early();
    furi_hal_os_init();
    // furi_hal_spi_config_init_early();
    // furi_hal_i2c_init_early();
    // furi_hal_light_init();
    // furi_hal_rtc_init_early();
    // furi_hal_version_init();
}

void furi_hal_deinit_early(void) {
    // furi_hal_rtc_deinit_early();
    // furi_hal_i2c_deinit_early();
    // furi_hal_spi_config_deinit_early();
    furi_hal_pwm_deinit();
    furi_hal_resources_deinit_early();
    furi_hal_dma_deinit_early();
    furi_hal_bus_deinit_early();
    furi_hal_clock_deinit_early();
}

void furi_hal_init(void) {
    furi_hal_memory_init();
    // furi_hal_mpu_init();
    // furi_hal_adc_init();
    furi_hal_clock_init();
    furi_hal_random_init();
    furi_hal_serial_control_init();
    // furi_hal_rtc_init();
    furi_hal_interrupt_init();
    // furi_hal_flash_init();
    furi_hal_resources_init();
    // furi_hal_region_init();
    // furi_hal_spi_config_init();
    // furi_hal_spi_dma_init();
    // furi_hal_crypto_init();
    // furi_hal_i2c_init();
    // furi_hal_power_init();
    // furi_hal_light_init();
    furi_hal_pwm_init();
    sl_mbedtls_init();

#ifndef FURI_RAM_EXEC
#endif
}

void furi_hal_switch(void* address) {
    __set_BASEPRI(0);
    // This code emulates system reset: sets MSP and calls Reset ISR
    asm volatile("ldr    r3, [%0]    \n" // Load SP from new vector to r3
                 "msr    msp, r3     \n" // Set MSP from r3
                 "ldr    r3, [%1]    \n" // Load Reset Handler address to r3
                 "mov    pc, r3      \n" // Set PC from r3 (jump to Reset ISR)
                 :
                 : "r"(address), "r"(address + 0x4)
                 : "r3");
}
