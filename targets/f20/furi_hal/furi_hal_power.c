#include <furi_hal.h>

void furi_hal_power_reset(void) {
    furi_hal_cortex_system_reset();
}

void furi_hal_power_reset_917(bool to_dfu) {
    furi_hal_gpio_write(&gpio_917_swo, true);
    furi_hal_gpio_write(&gpio_917_rst, true);

    furi_hal_gpio_init_simple(&gpio_917_swo, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(&gpio_917_rst, GpioModeOutputPushPull);

    if(to_dfu) {
        furi_hal_gpio_write(&gpio_917_swo, false);
    }
    furi_hal_gpio_write(&gpio_917_rst, false);
    furi_delay_ms(20);
    furi_hal_gpio_write(&gpio_917_rst, true);

    if(to_dfu) {
        furi_delay_ms(150);
    }

    furi_hal_gpio_init_simple(&gpio_917_swo, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_917_rst, GpioModeAnalog);
}
