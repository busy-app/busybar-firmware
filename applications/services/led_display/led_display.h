#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_DOT_MATRIX "dot_matrix"

#define DOT_MATRIX_W        (72)
#define DOT_MATRIX_H        (16)
#define DOT_MATRIX_BPP      (24)
#define DOT_MATRIX_BUF_SIZE (DOT_MATRIX_W * DOT_MATRIX_H * DOT_MATRIX_BPP / 8)

#define DOT_MATRIX_BRIGHTNESS_MIN  (0)
#define DOT_MATRIX_BRIGHTNESS_MAX  (100)
#define DOT_MATRIX_BRIGHTNESS_AUTO (255)

typedef struct DotMatrixSrv DotMatrixSrv;

void dot_matrix_reset(DotMatrixSrv* instance);

void dot_matrix_draw(DotMatrixSrv* instance, const uint8_t* buf);

/**
 * @brief Set the brightness of the dot matrix display
 * 
 * @param instance Pointer to the DotMatrixSrv instance
 * @param brightness Brightness value (DOT_MATRIX_BRIGHTNESS_MIN to DOT_MATRIX_BRIGHTNESS_MAX),
 *                   or DOT_MATRIX_BRIGHTNESS_AUTO for automatic brightness adjustment
 */
void dot_matrix_set_brightness(DotMatrixSrv* instance, uint8_t brightness);
