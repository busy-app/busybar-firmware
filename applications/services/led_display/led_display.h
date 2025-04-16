#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_DOT_MATRIX "dot_matrix"

#define DOT_MATRIX_W        (72)
#define DOT_MATRIX_H        (16)
#define DOT_MATRIX_BPP      (24)
#define DOT_MATRIX_BUF_SIZE (DOT_MATRIX_W * DOT_MATRIX_H * DOT_MATRIX_BPP / 8)

typedef struct DotMatrixSrv DotMatrixSrv;

void dot_matrix_reset(DotMatrixSrv* instance);

void dot_matrix_draw(DotMatrixSrv* instance, const uint8_t* buf);

void dot_matrix_set_brightness(DotMatrixSrv* instance, bool auto_brightness, uint8_t brightness);