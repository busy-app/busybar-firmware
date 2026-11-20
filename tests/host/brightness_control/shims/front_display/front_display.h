#pragma once
#include <stdint.h>
#define RECORD_FRONT_DISPLAY "front_display"
typedef struct FrontDisplaySrv FrontDisplaySrv;
typedef struct {
    uint8_t val;
} FrontDisplayBrightness;
void front_display_set_brightness(FrontDisplaySrv*, FrontDisplayBrightness);
