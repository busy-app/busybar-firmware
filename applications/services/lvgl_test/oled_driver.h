#pragma once

#include <stdint.h>
#include <stddef.h>

#define OLED_W        (160)
#define OLED_H        (80)
#define OLED_BPP      (4)
#define OLED_BUF_SIZE (OLED_W * OLED_H * OLED_BPP / 8)

void oled_driver_init(void);

void oled_driver_draw(const uint8_t* buf);
