#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize PWM
 */
void furi_hal_pwm_init(void);

/**
 * Deinitialize PWM
 */
void furi_hal_pwm_deinit(void);

/**
 * Start PWM
 */
void furi_hal_pwm_start(void);

/**
 * Stop PWM
 */
void furi_hal_pwm_stop(void);

/**
 * Set RGB color
 * @param red   Red color
 * @param green Green color
 * @param blue  Blue color
 */
void furi_hal_pwm_set_rgb(uint16_t red, uint16_t green, uint16_t blue);

#ifdef __cplusplus
}
#endif
