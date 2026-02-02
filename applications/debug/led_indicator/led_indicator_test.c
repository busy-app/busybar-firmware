#include <furi_hal.h>

static void furi_hal_led_indicator_set_color(uint8_t r, uint8_t g, uint8_t b) {
    FURI_LOG_W("", "TODO: %s", __func__);
    UNUSED(r);
    UNUSED(g);
    UNUSED(b);
}

int32_t led_indicator_test(void* p) {
    UNUSED(p);

    furi_hal_led_indicator_set_color(10, 0, 0);
    furi_delay_ms(500);
    furi_hal_led_indicator_set_color(0, 10, 0);
    furi_delay_ms(500);
    furi_hal_led_indicator_set_color(0, 0, 10);
    furi_delay_ms(500);
    furi_hal_led_indicator_set_color(10, 10, 10);
    furi_delay_ms(500);

    furi_hal_led_indicator_set_color(255, 0, 0);
    furi_delay_ms(500);
    furi_hal_led_indicator_set_color(0, 255, 0);
    furi_delay_ms(500);
    furi_hal_led_indicator_set_color(0, 0, 255);
    furi_delay_ms(500);
    furi_hal_led_indicator_set_color(255, 255, 255);
    furi_delay_ms(500);

    furi_hal_led_indicator_set_color(0, 0, 0);

    return 0;
}
