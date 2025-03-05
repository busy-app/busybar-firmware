/**
 * @file color.h
 * @brief Color declaration and conversion API
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** RGB color structure */
typedef struct {
    uint8_t r; /**< Red component */
    uint8_t g; /**< Green component */
    uint8_t b; /**< Blue component */
} Color;

/** HSV color structure */
typedef struct {
    uint8_t h; /**< Hue component */
    uint8_t s; /**< Saturation component */
    uint8_t v; /**< Value component */
} ColorHsv;

/**
 * @brief Convert HSV color to RGB color
 * 
 * @param hsv ColorHsv structure to convert
 *
 * @return Color structure
 */
Color color_hsv_to_rgb(ColorHsv hsv);

#ifdef __cplusplus
}
#endif
