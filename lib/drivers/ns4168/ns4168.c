#include "ns4168.h"
#include <furi.h>
#include <furi_hal_resources.h>

#define NS4168_T_OFF  (120U) // >100us
#define NS4168_T_BOOT (10000U)
#define NS4168_T_CTRL (5U) // 1..12us

void ns4168_init(void) {
    furi_hal_gpio_write(&gpio_audio_en_and_boot0, false);
    furi_hal_gpio_init_simple(&gpio_audio_en_and_boot0, GpioModeOutputPushPull);
}

void ns4168_deinit(void) {
    furi_hal_gpio_write(&gpio_audio_en_and_boot0, false);
    furi_hal_gpio_init_simple(&gpio_audio_en_and_boot0, GpioModeAnalog);
}

void ns4168_power_on(Ns4168Hpf hpf) {
    furi_hal_gpio_write(&gpio_audio_en_and_boot0, false);
    furi_delay_us(NS4168_T_OFF);
    furi_hal_gpio_write(&gpio_audio_en_and_boot0, true);
    for(uint32_t i = 1; i < hpf; i++) {
        furi_delay_us(NS4168_T_CTRL);
        furi_hal_gpio_write(&gpio_audio_en_and_boot0, false);
        furi_delay_us(NS4168_T_CTRL);
        furi_hal_gpio_write(&gpio_audio_en_and_boot0, true);
    }
    furi_delay_us(NS4168_T_BOOT); //Todo: check if this delay is needed for start
}

void ns4168_power_off(void) {
    furi_hal_gpio_write(&gpio_audio_en_and_boot0, false);
}
