#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SSD1320_W        (160)
#define SSD1320_H        (80)
#define SSD1320_BPP      (4)
#define SSD1320_BUF_SIZE (SSD1320_W * SSD1320_H * SSD1320_BPP / 8)

#define SSD1320_GRAYSCALE_TABLE_SIZE (15)

#define SSD1320_CONTRAST_MIN (1)
#define SSD1320_CONTRAST_MAX (255)

typedef struct {
    uint8_t data[SSD1320_GRAYSCALE_TABLE_SIZE];
} SSD1320GrayscaleTable;

void ssd1320_init(void);

void ssd1320_draw(const uint8_t* buf);

void ssd1320_sleep_mode(bool sleep);

void ssd1320_set_contrast(uint8_t contrast);

void ssd1320_set_grayscale_table(const SSD1320GrayscaleTable* grayscale_table);
