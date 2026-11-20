#pragma once
#include <stdint.h>
#define RECORD_BACK_DISPLAY "back_display"
typedef struct BackDisplaySrv BackDisplaySrv;
typedef struct {
    uint8_t val;
} BackDisplayContrast;
void back_display_set_contrast(BackDisplaySrv*, BackDisplayContrast);
