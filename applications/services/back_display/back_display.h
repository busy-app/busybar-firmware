
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_BACK_DISPLAY "back_display"

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
 * @brief Set back display brightness. To some value, or make it auto
 *
 * @param instance back display service instance
 * @param auto_brightness if true next param doesn't play any role, brightness will be taken from light sensor. 
 * @param brightness desired brightness value from 0 to LIGHT_SENSOR_LIGHT_LEVEL_MAX
 */
void back_display_set_brightness(
    BackDisplaySrv* instance,
    bool auto_brightness,
    uint8_t brightness);

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
