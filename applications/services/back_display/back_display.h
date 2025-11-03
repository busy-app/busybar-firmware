
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_BACK_DISPLAY "back_display"

#define BACK_DISPLAY_W (160)
#define BACK_DISPLAY_H (80)

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
 * @brief Controls the display's power setting
 * 
 * @note This configuration will not be persisted across reboots. The display is
 * always turned on on power up.
 * 
 * @warning This function counts how many apps are currently keeping sleep
 * active, and only puts the display out of it once the count reaches zero. A
 * call to this function with the value of `false` is thus not guaranteed to
 * pull the display out of sleep.
 * 
 * @param instance back display service instance
 * @param sleep `true` if the display is to be put into sleep (turned off),
 *              `false` if it is to be put out of sleep (turned on)
 */
void back_display_sleep_mode(BackDisplaySrv* instance, bool sleep);

/**
 * @brief Set the back display brightness 
 * 
 * @param instance Pointer to the FrontDisplaySrv instance
 * @param brightness Brightness value (BACK_DISPLAY_BRIGHTNESS_MIN to BACK_DISPLAY_BRIGHTNESS_MAX),
 *                   or BACK_DISPLAY_BRIGHTNESS_AUTO for automatic brightness adjustment
 */
void back_display_set_brightness(BackDisplaySrv* instance, uint8_t brightness);

/**
 * @brief Get the back display brightness 
 * 
 * @param instance Pointer to the FrontDisplaySrv instance
 * @return Brightness value (BACK_DISPLAY_BRIGHTNESS_MIN to BACK_DISPLAY_BRIGHTNESS_MAX),
 *                   or BACK_DISPLAY_BRIGHTNESS_AUTO if automatic brightness is enabled
 */
uint8_t back_display_get_brightness(BackDisplaySrv* instance);

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
