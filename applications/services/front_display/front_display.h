#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_FRONT_DISPLAY "front_display"

#define FRONT_DISPLAY_W        (72)
#define FRONT_DISPLAY_H        (16)
#define FRONT_DISPLAY_BPP      (24)
#define FRONT_DISPLAY_BUF_SIZE (FRONT_DISPLAY_W * FRONT_DISPLAY_H * FRONT_DISPLAY_BPP / 8)

typedef struct FrontDisplaySrv FrontDisplaySrv;

typedef struct FrontDisplayBrightness {
    uint8_t val;
} FrontDisplayBrightness;

void front_display_draw(FrontDisplaySrv* instance, const uint8_t* buf);

/**
 * @brief Set the brightness of front display
 * 
 * @param instance Pointer to the FrontDisplaySrv instance
 * @param brightness Brightness value (FRONT_DISPLAY_BRIGHTNESS_MIN to FRONT_DISPLAY_BRIGHTNESS_MAX).
 */
void front_display_set_brightness(FrontDisplaySrv* instance, FrontDisplayBrightness brightness);

/**
 * @brief Enable or disable display blanking.
 *
 * When the display is blanked, it will not show anything until it is unblanked.
 *
 * @param instance Pointer to the FrontDisplaySrv instance
 * @param is_blanked blank the display if @c true, unblank otherwise
 */
void front_display_set_blanked(FrontDisplaySrv* instance, bool is_blanked);

/**
 * @brief Enter or exit low-power sleep mode.
 *
 * Display power is completely shut down in sleep mode. 
 * When exiting sleep mode, the display will be re-initialized, which may cause a delay before it starts showing content again.
 *
 * @param instance Pointer to the FrontDisplaySrv instance
 * @param sleep enter sleep mode if @c true, exit sleep mode otherwise
 */
void front_display_sleep_mode(FrontDisplaySrv* instance, bool sleep);
