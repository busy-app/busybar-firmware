#include <furi_hal.h>

typedef struct {
    volatile uint8_t insomnia;
} FuriHalPower;

static volatile FuriHalPower furi_hal_power = {
    .insomnia = 0,
};

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

uint16_t furi_hal_power_insomnia_level(void) {
    return furi_hal_power.insomnia;
}

void furi_hal_power_insomnia_enter(void) {
    FURI_CRITICAL_ENTER();
    furi_check(furi_hal_power.insomnia < UINT8_MAX);
    furi_hal_power.insomnia++;
    FURI_CRITICAL_EXIT();
}

void furi_hal_power_insomnia_exit(void) {
    FURI_CRITICAL_ENTER();
    furi_check(furi_hal_power.insomnia > 0);
    furi_hal_power.insomnia--;
    FURI_CRITICAL_EXIT();
}

bool furi_hal_power_sleep_available(void) {
    return furi_hal_power.insomnia == 0;
}
