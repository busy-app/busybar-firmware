#include "furi_hal_display.h"
#include "furi_hal_gpio.h"
#include "furi_hal_resources.h"

void furi_hal_display_power_pin_init(void) {
    furi_hal_gpio_init(
        &gpio_front_display_power_en, GpioModeInterruptRiseFall, GpioPullUp, GpioSpeedLow);
    LL_GPIO_SetPinOutputType(
        gpio_front_display_power_en.port,
        gpio_front_display_power_en.pin,
        LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetPinMode(
        gpio_front_display_power_en.port, gpio_front_display_power_en.pin, LL_GPIO_MODE_OUTPUT);
}

bool furi_hal_display_power_pin_read(void) {
    return furi_hal_gpio_read(&gpio_front_display_power_en);
}

void furi_hal_display_power_pin_attach_callback(GpioExtiCallback cb, void* ctx) {
    furi_hal_gpio_add_int_callback(&gpio_front_display_power_en, cb, ctx);
}

void furi_hal_display_power_pin_interrupt_enable(void) {
    furi_hal_gpio_enable_int_callback(&gpio_front_display_power_en);
}

void furi_hal_display_power_pin_interrupt_disable(void) {
    furi_hal_gpio_disable_int_callback(&gpio_front_display_power_en);
}

void furi_hal_display_power_enable(void) {
    furi_hal_gpio_write(&gpio_front_display_power_en, true);
}

void furi_hal_display_power_disable(void) {
    furi_hal_gpio_write(&gpio_front_display_power_en, false);
}
