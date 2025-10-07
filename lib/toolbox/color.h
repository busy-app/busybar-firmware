/**
 * @file color.h
 * @brief Color declaration and conversion API
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Converts an `R, G, B` representation into a color
 */
#define COLOR_MAKE_RGB(rv, gv, bv) {.a = 255, .b = (bv), .g = (gv), .r = (rv)}

/**
 * @brief Converts an `R, G, B, A` representation into a color
 */
#define COLOR_MAKE_RGBA(rv, gv, bv, av) {.a = (av), .b = (bv), .g = (gv), .r = (rv)}

/**
 * @brief Converts an `0xRRGGBB` representation into a color
 */
#define COLOR_MAKE_HEX(hex) \
    {.a = 255, .b = (hex) & 0xFF, .g = ((hex) >> 8) & 0xFF, .r = ((hex) >> 16) & 0xFF}

/**
 * @brief Converts an `0xRRGGBBAA` representation into a color
 */
#define COLOR_MAKE_HEXA(hexa) \
    {.a = (hexa) & 0xFF, .b = ((hexa) >> 8) & 0xFF, .g = ((hexa) >> 16) & 0xFF, r = ((hexa) >> 24) & 0xFF}

/** RGB color structure */
typedef struct {
    uint8_t b; /**< Blue component */
    uint8_t g; /**< Green component */
    uint8_t r; /**< Red component */
    uint8_t a; /**< Alpha component */
} Color;

/** HSV color structure */
typedef struct {
    uint8_t h; /**< Hue component */
    uint8_t s; /**< Saturation component */
    uint8_t v; /**< Value component */
    uint8_t a; /**< Alpha component */
} ColorHsv;

/**
 * @brief Convert HSV color to RGB color
 * 
 * @param hsv ColorHsv structure to convert
 *
 * @return Color structure
 */
Color color_hsv_to_rgb(ColorHsv hsv);

/**
 * @brief Convert a HEX `0xRRGGBB` representation to an RGB color
 *
 * @param hex Hex value to convert (`0xRRGGBB`)
 *
 * @return Color structure
 */
Color color_hex_to_rgb(uint32_t hex);

/**
 * @brief Convert a HEX `0xRRGGBBAA` representation to an RGB color
 *
 * @param hex Hex value to convert (`0xRRGGBBAA`)
 *
 * @return Color structure
 */
Color color_hexa_to_rgb(uint32_t hexa);

#ifdef __cplusplus
}
#endif
