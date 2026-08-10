/**
 * @file color.h
 * @brief Color declaration and conversion API
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Converts an `R, G, B` representation into a color
 */
#define COLOR_MAKE_RGB(rv, gv, bv) {.b = (bv), .g = (gv), .r = (rv), .a = 255}

/**
 * @brief Converts an `R, G, B, A` representation into a color
 */
#define COLOR_MAKE_RGBA(rv, gv, bv, av) {.b = (bv), .g = (gv), .r = (rv), .a = (av)}

/**
 * @brief Converts an `0xRRGGBB` representation into a color
 */
#define COLOR_MAKE_HEX(hex) \
    {.b = (hex) & 0xFF, .g = ((hex) >> 8) & 0xFF, .r = ((hex) >> 16) & 0xFF, .a = 255}

/**
 * @brief Converts an `0xRRGGBBAA` representation into a color
 */
#define COLOR_MAKE_HEXA(hexa)    \
    {.a = (hexa) & 0xFF,         \
     .b = ((hexa) >> 8) & 0xFF,  \
     .g = ((hexa) >> 16) & 0xFF, \
     .r = ((hexa) >> 24) & 0xFF}

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
 * @param hexa Hex value to convert (`0xRRGGBBAA`)
 *
 * @return Color structure
 */
Color color_hexa_to_rgb(uint32_t hexa);

/**
 * @brief Convert a HEX `"#RRGGBBAA"` representation to an RGB color
 * 
 * @param[in] hexa Hex value to convert (`"#RRGGBBAA"`)
 * @param[out] color_out Color structure to fill
 * 
 * @return Parsing status (`true` = success)
 */
bool color_parse_hexa_string(const char* hexa, Color* color_out);

/**
 * @brief Convert an RGB color to an L8 luminance value
 *
 * @param[in] color Color to convert
 *
 * @return Luminance value in the range 0–255
 */
uint8_t color_rgb_to_l8(Color color);

/**
 * @brief Convert an L8-encoded color data into L4
 *
 * @param dst destination buffer
 * @param src source buffer
 * @param size size of source buffer. Destination buffer must hold at least size/2 bytes.
 */
void color_buf_l8_to_l4(void* dst, const void* src, size_t size);

#ifdef __cplusplus
}
#endif
