#include "ns4168.h"
#include <furi.h>
#include <furi_hal_resources.h>

#define NS4168_T_OFF  (120U) // >100us
#define NS4168_T_BOOT (10000U)
#define NS4168_T_CTRL (5U) // 1..12us

struct NS4168 {
    const GpioPin* gpio_pin;
};

NS4168* ns4168_alloc(const GpioPin* gpio_pin) {
    NS4168* handle = malloc(sizeof(NS4168));
    handle->gpio_pin = gpio_pin;
    return handle;
}

void ns4168_free(NS4168* handle) {
    if(handle) {
        free(handle);
    }
}

void ns4168_init(NS4168* handle) {
    furi_hal_gpio_write(handle->gpio_pin, false);
    furi_hal_gpio_init_simple(handle->gpio_pin, GpioModeOutputPushPull);
}

void ns4168_deinit(NS4168* handle) {
    furi_hal_gpio_write(handle->gpio_pin, false);
    furi_hal_gpio_init_simple(handle->gpio_pin, GpioModeAnalog);
}

void ns4168_power_on(NS4168* handle, Ns4168Hpf hpf) {
    furi_hal_gpio_write(handle->gpio_pin, false);
    furi_delay_us(NS4168_T_OFF);
    furi_hal_gpio_write(handle->gpio_pin, true);
    for(uint32_t i = 1; i < hpf; i++) {
        furi_delay_us(NS4168_T_CTRL);
        furi_hal_gpio_write(handle->gpio_pin, false);
        furi_delay_us(NS4168_T_CTRL);
        furi_hal_gpio_write(handle->gpio_pin, true);
    }
    furi_delay_us(NS4168_T_BOOT); //Todo: check if this delay is needed for start
}

void ns4168_power_off(NS4168* handle) {
    furi_hal_gpio_write(handle->gpio_pin, false);
}
