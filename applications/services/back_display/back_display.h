
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_BACK_DISPLAY "back_display"

#define BACK_DISPLAY_W        (160)
#define BACK_DISPLAY_H        (80)
#define BACK_DISPLAY_BPP      (4)
#define BACK_DISPLAY_BUF_SIZE (BACK_DISPLAY_W * BACK_DISPLAY_H * BACK_DISPLAY_BPP / 8)

typedef struct BackDisplaySrv BackDisplaySrv;

void back_display_draw(BackDisplaySrv* instance, const uint8_t* buf);
