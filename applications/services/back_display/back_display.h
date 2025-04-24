
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_BACK_DISPLAY "back_display"

#define BACK_DISPLAY_BRIGHTNESS_MIN  (0)
#define BACK_DISPLAY_BRIGHTNESS_MAX  (100)
#define BACK_DISPLAY_BRIGHTNESS_AUTO (255)

typedef struct BackDisplaySrv BackDisplaySrv;

/**
 * @brief Draw the back display data.
 *
 * @param instance back display service instance
 * @param buf back display data to draw
 * @warning this will enqueue the data to be drawn on the next frame, not immediately
 */
void back_display_draw(BackDisplaySrv* instance, const uint8_t* data);

/**
 * @brief Set the back display brightness 
 * 
 * @param instance Pointer to the FrontDisplaySrv instance
 * @param brightness Brightness value (BACK_DISPLAY_BRIGHTNESS_MIN to BACK_DISPLAY_BRIGHTNESS_MAX),
 *                   or BACK_DISPLAY_BRIGHTNESS_AUTO for automatic brightness adjustment
 */
void back_display_set_brightness(BackDisplaySrv* instance, uint8_t brightness);

/**
 * @brief Get the width of the back display.
 *
 * @return the width of the back display
 */
size_t back_display_get_width(void);

/**
 * @brief Get the height of the back display.
 *
 * @return the height of the back display
 */
size_t back_display_get_height(void);

#ifdef __cplusplus
}
#endif
