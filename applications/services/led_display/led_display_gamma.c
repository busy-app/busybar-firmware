#include <stdint.h>
#include <string.h>
#include <math.h>
#include "led_display_i.h"

// Gamma correction

#define BRIGHTNESS_VAL_MAX 100

inline uint16_t led_display_gamma_apply(const uint16_t* gamma_lut, uint8_t in_val) {
    return (gamma_lut[in_val]);
}

void led_display_gamma_lut_generate(uint16_t* gamma_lut, float gamma_val, uint8_t brightness) {
    if(brightness > BRIGHTNESS_VAL_MAX) {
        brightness = BRIGHTNESS_VAL_MAX;
    }

    uint32_t out_max = (brightness * 65535) / BRIGHTNESS_VAL_MAX;

    float inv_gamma = 1.f / (float)gamma_val;

    for(uint16_t i = 0; i < 256; i++) {
        float val_in = ((float)i) / 255.f;
        float val_out = powf(val_in, inv_gamma);
        gamma_lut[i] = (uint16_t)(val_out * out_max);
    }
}
