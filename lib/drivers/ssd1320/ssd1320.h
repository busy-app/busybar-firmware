#pragma once

#include <stdint.h>
#include <stddef.h>

#define SSD1320_W        (160)
#define SSD1320_H        (80)
#define SSD1320_BPP      (4)
#define SSD1320_BUF_SIZE (SSD1320_W * SSD1320_H * SSD1320_BPP / 8)

#define SSD1320_CONTRAST_MIN (1)
#define SSD1320_CONTRAST_MAX (255)

void ssd1320_init(void);

void ssd1320_draw(const uint8_t* buf);

void ssd1320_set_contrast(uint8_t contrast);
