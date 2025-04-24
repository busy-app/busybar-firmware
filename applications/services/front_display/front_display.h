#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_FRONT_DISPLAY "front_display"

#define FRONT_DISPLAY_W        (72)
#define FRONT_DISPLAY_H        (16)
#define FRONT_DISPLAY_BPP      (24)
#define FRONT_DISPLAY_BUF_SIZE (FRONT_DISPLAY_W * FRONT_DISPLAY_H * FRONT_DISPLAY_BPP / 8)

#define FRONT_DISPLAY_BRIGHTNESS_MIN  (0)
#define FRONT_DISPLAY_BRIGHTNESS_MAX  (100)
#define FRONT_DISPLAY_BRIGHTNESS_AUTO (255)

typedef struct FrontDisplaySrv FrontDisplaySrv;

void front_display_reset(FrontDisplaySrv* instance);

void front_display_draw(FrontDisplaySrv* instance, const uint8_t* buf);

/**
 * @brief Set the brightness of front display
 * 
 * @param instance Pointer to the FrontDisplaySrv instance
 * @param brightness Brightness value (FRONT_DISPLAY_BRIGHTNESS_MIN to FRONT_DISPLAY_BRIGHTNESS_MAX),
 *                   or FRONT_DISPLAY_BRIGHTNESS_AUTO for automatic brightness adjustment
 */
void front_display_set_brightness(FrontDisplaySrv* instance, uint8_t brightness);
