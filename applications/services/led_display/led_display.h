#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DISPLAY_W 72
#define DISPLAY_H 16

void led_display_reset(void);

void led_display_set_default_img(void);

void led_display_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
