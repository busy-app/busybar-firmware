#include "led_display_test.h"

#include <furi/furi.h>
#include <led_display/led_display.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LedDisplayTestColorCode;

typedef struct {
    LedDisplayTestColorCode code;
    const char* name;
} LedDisplayTestColorData;

static const LedDisplayTestColorData led_display_test_color[LedDisplayTestColorNum] = {
    [LedDisplayTestColorRed] =
        {
            .name = "Red",
            .code =
                {
                    .red = 0xff,
                    .green = 0x00,
                    .blue = 0x00,
                },
        },
    [LedDisplayTestColorGreen] =
        {
            .name = "Green",
            .code =
                {
                    .red = 0x00,
                    .green = 0xff,
                    .blue = 0x00,
                },
        },
    [LedDisplayTestColorBlue] =
        {
            .name = "Blue",
            .code =
                {
                    .red = 0x00,
                    .green = 0x00,
                    .blue = 0xff,
                },
        },
    [LedDisplayTestColorYellow] =
        {
            .name = "Yellow",
            .code =
                {
                    .red = 0xff,
                    .green = 0xff,
                    .blue = 0x00,
                },
        },
    [LedDisplayTestColorCian] =
        {
            .name = "Cian",
            .code =
                {
                    .red = 0x00,
                    .green = 0xff,
                    .blue = 0xff,
                },
        },
    [LedDisplayTestColorPurple] =
        {
            .name = "Purple",
            .code =
                {
                    .red = 0xff,
                    .green = 0x00,
                    .blue = 0xff,
                },
        },
    [LedDisplayTestColorWhite] =
        {
            .name = "White",
            .code =
                {
                    .red = 0xff,
                    .green = 0xff,
                    .blue = 0xff,
                },
        },
};

typedef void (*LedDisplayTestPatternSet)(uint8_t* buff, LedDisplayTestColorCode color_code);

typedef struct {
    LedDisplayTestPatternSet set;
    const char* name;
} LedDisplayTestPatternData;

static void led_display_set_pixel(
    uint8_t* buff,
    uint8_t x,
    uint8_t y,
    uint8_t red,
    uint8_t green,
    uint8_t blue) {
    uint32_t pixel_offset = y * 72 + x;
    buff[pixel_offset * 3 + 0] = red;
    buff[pixel_offset * 3 + 1] = green;
    buff[pixel_offset * 3 + 2] = blue;
}

static void led_display_test_set_pattern_chess(uint8_t* buff, LedDisplayTestColorCode color_code) {
    const uint8_t cell_size = 4;

    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            if(((x / cell_size) + (y / cell_size)) % 2 == 0) {
                led_display_set_pixel(
                    buff, x, y, color_code.red, color_code.green, color_code.blue);
            }
        }
    }
}

static void led_display_test_set_pattern_lines_horizontal(
    uint8_t* buff,
    LedDisplayTestColorCode color_code) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            if(y % 2 == 0) {
                led_display_set_pixel(
                    buff, x, y, color_code.red, color_code.green, color_code.blue);
            }
        }
    }
}

static void
    led_display_test_set_pattern_lines_vertical(uint8_t* buff, LedDisplayTestColorCode color_code) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            if(x % 2 == 0) {
                led_display_set_pixel(
                    buff, x, y, color_code.red, color_code.green, color_code.blue);
            }
        }
    }
}

static void
    led_display_test_set_pattern_full_fill(uint8_t* buff, LedDisplayTestColorCode color_code) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            led_display_set_pixel(buff, x, y, color_code.red, color_code.green, color_code.blue);
        }
    }
}

static void
    led_display_test_set_pattern_rectangulars(uint8_t* buff, LedDisplayTestColorCode color_code) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            if((x % 24) < 12) {
                led_display_set_pixel(
                    buff, x, y, color_code.red, color_code.green, color_code.blue);
            }
        }
    }
}

// static size_t animation_frame = 0;

// static void led_display_test_set_pattern_animated_rectangulars(
//     uint8_t* buff,
//     LedDisplayTestColorCode color_code) {
//     size_t frame = animation_frame % 24;

//     for(size_t x = 0; x < DOT_MATRIX_W; x++) {
//         for(size_t y = 0; y < DOT_MATRIX_H; y++) {
//             if((x % 24) < frame) {
//                 led_display_set_pixel(
//                     buff, x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_animated_rectangulars_half(
//     uint8_t* buff,
//     LedDisplayTestColorCode color_code) {
//     size_t frame = animation_frame % 12;

//     for(size_t x = 0; x < DOT_MATRIX_W; x++) {
//         for(size_t y = 0; y < DOT_MATRIX_H; y++) {
//             if((x % 24) < frame) {
//                 led_display_set_pixel(
//                     buff, x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_animated_fill_10_noise(
//     uint8_t* buff,
//     LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DOT_MATRIX_W; x++) {
//         for(size_t y = 0; y < DOT_MATRIX_H; y++) {
//             bool pixel_set = rand() % 10 == 0;

//             if(pixel_set) {
//                 led_display_set_pixel(
//                     buff, x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_animated_fill_25_noise(
//     uint8_t* buff,
//     LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DOT_MATRIX_W; x++) {
//         for(size_t y = 0; y < DOT_MATRIX_H; y++) {
//             bool pixel_set = rand() % 4 == 0;

//             if(pixel_set) {
//                 led_display_set_pixel(
//                     buff, x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }
// static void led_display_test_set_pattern_animated_fill_50_noise(
//     uint8_t* buff,
//     LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DOT_MATRIX_W; x++) {
//         for(size_t y = 0; y < DOT_MATRIX_H; y++) {
//             bool pixel_set = rand() % 2 == 0;

//             if(pixel_set) {
//                 led_display_set_pixel(
//                     buff, x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

static void led_display_test_set_pattern_cross(uint8_t* buff, LedDisplayTestColorCode color_code) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            if((2 * x) - (9 * y) < 6) {
                led_display_set_pixel(
                    buff, x, y, color_code.red, color_code.green, color_code.blue);
                led_display_set_pixel(
                    buff,
                    x,
                    DOT_MATRIX_H - 1 - y,
                    color_code.red,
                    color_code.green,
                    color_code.blue);
            }
        }
    }
}

static void led_display_test_set_pattern_frame(uint8_t* buff, LedDisplayTestColorCode color_code) {
    for(size_t x = 0; x < DOT_MATRIX_W; x++) {
        for(size_t y = 0; y < DOT_MATRIX_H; y++) {
            if((x == 0) || (y == 0) || (x == DOT_MATRIX_W - 1) || (y == DOT_MATRIX_H - 1))
                led_display_set_pixel(
                    buff, x, y, color_code.red, color_code.green, color_code.blue);
        }
    }
}

static const LedDisplayTestPatternData led_display_test_pattern[LedDisplayTestPatternNum] = {
    [LedDisplayTestPatternChess] =
        {
            .set = led_display_test_set_pattern_chess,
            .name = "Chess",
        },
    [LedDisplayTestPatternLinesHorizontal] =
        {
            .set = led_display_test_set_pattern_lines_horizontal,
            .name = "Horizontal Lines",
        },
    [LedDisplayTestPatternLinesVertical] =
        {
            .set = led_display_test_set_pattern_lines_vertical,
            .name = "Vertical Lines",
        },
    [LedDisplayTestPatternFullFill] =
        {
            .set = led_display_test_set_pattern_full_fill,
            .name = "Full Fill",
        },
    [LedDisplayTestPatternRectangulars] =
        {
            .set = led_display_test_set_pattern_rectangulars,
            .name = "Rectangulars",
        },
    [LedDisplayTestPatternCross] =
        {
            .set = led_display_test_set_pattern_cross,
            .name = "Cross",
        },
    [LedDisplayTestPatternFrame] =
        {
            .set = led_display_test_set_pattern_frame,
            .name = "Frame",
        },
    // [LedDisplayTestPatternAnimFill] =
    //     {
    //         .set = led_display_test_set_pattern_animated_rectangulars,
    //         .name = "Animated Rectangulars Fill",
    //     },
    // [LedDisplayTestPatternAnimHalfFill] =
    //     {
    //         .set = led_display_test_set_pattern_animated_rectangulars_half,
    //         .name = "Animated Rectangulars Half Fill",
    //     },
    // [LedDisplayTestPatternAnimFill10Noise] =
    //     {
    //         .set = led_display_test_set_pattern_animated_fill_10_noise,
    //         .name = "Animated Fill 10% Noise",
    //     },
    // [LedDisplayTestPatternAnimFill25Noise] =
    //     {
    //         .set = led_display_test_set_pattern_animated_fill_25_noise,
    //         .name = "Animated Fill 25% Noise",
    //     },
    // [LedDisplayTestPatternAnimFill50Noise] =
    //     {
    //         .set = led_display_test_set_pattern_animated_fill_50_noise,
    //         .name = "Animated Fill 50% Noise",
    //     },
};

// size_t led_display_get_pattern_frame_time(LedDisplayTestPattern pattern) {
//     switch(pattern) {
//     case LedDisplayTestPatternChess ... LedDisplayTestPatternFrame:
//         return FuriWaitForever;
//     case LedDisplayTestPatternAnimFill ... LedDisplayTestPatternAnimFill50Noise:
//         return 1000 / 60;
//     case LedDisplayTestPatternNum:
//         break;
//     }

//     furi_crash();
// }

// void led_display_test_advance_frame(LedDisplayTestPattern pattern) {
//     switch(pattern) {
//     case LedDisplayTestPatternChess ... LedDisplayTestPatternFrame:
//         return;
//     case LedDisplayTestPatternAnimFill ... LedDisplayTestPatternAnimFill50Noise:
//         animation_frame++;
//         return;
//     case LedDisplayTestPatternNum:
//         furi_crash();
//     }
// }

void led_display_test_set(uint8_t* buff, LedDisplayTestPattern pattern, LedDisplayTestColor color) {
    furi_check(buff);
    furi_check(pattern < LedDisplayTestPatternNum);
    furi_check(color < LedDisplayTestColorNum);

    led_display_test_pattern[pattern].set(buff, led_display_test_color[color].code);
}

const char* led_display_get_pattern_str(LedDisplayTestPattern pattern) {
    furi_check(pattern < LedDisplayTestPatternNum);

    return led_display_test_pattern[pattern].name;
}

const char* led_display_get_color_str(LedDisplayTestColor color) {
    furi_check(color < LedDisplayTestColorNum);

    return led_display_test_color[color].name;
}
