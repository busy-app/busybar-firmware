// #include "led_display_test.h"

// #include <furi/furi.h>
// #include <led_display/led_display.h>

// typedef struct {
//     uint8_t red;
//     uint8_t green;
//     uint8_t blue;
// } LedDisplayTestColorCode;

// typedef struct {
//     LedDisplayTestColorCode code;
//     const char* name;
// } LedDisplayTestColorData;

// static const LedDisplayTestColorData led_display_test_color[LedDisplayTestColorNum] = {
//     [LedDisplayTestColorRed] =
//         {
//             .name = "Red",
//             .code =
//                 {
//                     .red = 0xff,
//                     .green = 0x00,
//                     .blue = 0x00,
//                 },
//         },
//     [LedDisplayTestColorGreen] =
//         {
//             .name = "Green",
//             .code =
//                 {
//                     .red = 0x00,
//                     .green = 0xff,
//                     .blue = 0x00,
//                 },
//         },
//     [LedDisplayTestColorBlue] =
//         {
//             .name = "Blue",
//             .code =
//                 {
//                     .red = 0x00,
//                     .green = 0x00,
//                     .blue = 0xff,
//                 },
//         },
//     [LedDisplayTestColorYellow] =
//         {
//             .name = "Yellow",
//             .code =
//                 {
//                     .red = 0xff,
//                     .green = 0xff,
//                     .blue = 0x00,
//                 },
//         },
//     [LedDisplayTestColorCian] =
//         {
//             .name = "Cian",
//             .code =
//                 {
//                     .red = 0x00,
//                     .green = 0xff,
//                     .blue = 0xff,
//                 },
//         },
//     [LedDisplayTestColorPurple] =
//         {
//             .name = "Purple",
//             .code =
//                 {
//                     .red = 0xff,
//                     .green = 0x00,
//                     .blue = 0xff,
//                 },
//         },
//     [LedDisplayTestColorWhite] =
//         {
//             .name = "White",
//             .code =
//                 {
//                     .red = 0xff,
//                     .green = 0xff,
//                     .blue = 0xff,
//                 },
//         },
// };

// typedef void (*LedDisplayTestPatternSet)(LedDisplayTestColorCode color_code);

// typedef struct {
//     LedDisplayTestPatternSet set;
//     const char* name;
// } LedDisplayTestPatternData;

// static void led_display_test_set_pattern_chess(LedDisplayTestColorCode color_code) {
//     const uint8_t cell_size = 4;

//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if(((x / cell_size) + (y / cell_size)) % 2 == 0) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_lines_horizontal(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if(y % 2 == 0) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_lines_vertical(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if(x % 2 == 0) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_full_fill(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//         }
//     }
// }

// static void led_display_test_set_pattern_rectangulars(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if((x % 24) < 12) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static size_t animation_frame = 0;

// static void
//     led_display_test_set_pattern_animated_rectangulars(LedDisplayTestColorCode color_code) {
//     size_t frame = animation_frame % 24;

//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if((x % 24) < frame) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void
//     led_display_test_set_pattern_animated_rectangulars_half(LedDisplayTestColorCode color_code) {
//     size_t frame = animation_frame % 12;

//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if((x % 24) < frame) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void
//     led_display_test_set_pattern_animated_fill_10_noise(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             bool pixel_set = rand() % 10 == 0;

//             if(pixel_set) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void
//     led_display_test_set_pattern_animated_fill_25_noise(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             bool pixel_set = rand() % 4 == 0;

//             if(pixel_set) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }
// static void
//     led_display_test_set_pattern_animated_fill_50_noise(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             bool pixel_set = rand() % 2 == 0;

//             if(pixel_set) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_cross(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if((2 * x) - (9 * y) < 6) {
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//                 led_display_set_pixel(
//                     x, DISPLAY_H - 1 - y, color_code.red, color_code.green, color_code.blue);
//             }
//         }
//     }
// }

// static void led_display_test_set_pattern_frame(LedDisplayTestColorCode color_code) {
//     for(size_t x = 0; x < DISPLAY_W; x++) {
//         for(size_t y = 0; y < DISPLAY_H; y++) {
//             if((x == 0) || (y == 0) || (x == DISPLAY_W - 1) || (y == DISPLAY_H - 1))
//                 led_display_set_pixel(x, y, color_code.red, color_code.green, color_code.blue);
//         }
//     }
// }

// static const LedDisplayTestPatternData led_display_test_pattern[LedDisplayTestPatternNum] = {
//     [LedDisplayTestPatternChess] =
//         {
//             .set = led_display_test_set_pattern_chess,
//             .name = "Chess",
//         },
//     [LedDisplayTestPatternLinesHorizontal] =
//         {
//             .set = led_display_test_set_pattern_lines_horizontal,
//             .name = "Horizontal Lines",
//         },
//     [LedDisplayTestPatternLinesVertical] =
//         {
//             .set = led_display_test_set_pattern_lines_vertical,
//             .name = "Vertical Lines",
//         },
//     [LedDisplayTestPatternFullFill] =
//         {
//             .set = led_display_test_set_pattern_full_fill,
//             .name = "Full Fill",
//         },
//     [LedDisplayTestPatternRectangulars] =
//         {
//             .set = led_display_test_set_pattern_rectangulars,
//             .name = "Rectangulars",
//         },
//     [LedDisplayTestPatternCross] =
//         {
//             .set = led_display_test_set_pattern_cross,
//             .name = "Cross",
//         },
//     [LedDisplayTestPatternFrame] =
//         {
//             .set = led_display_test_set_pattern_frame,
//             .name = "Frame",
//         },
//     [LedDisplayTestPatternAnimFill] =
//         {
//             .set = led_display_test_set_pattern_animated_rectangulars,
//             .name = "Animated Rectangulars Fill",
//         },
//     [LedDisplayTestPatternAnimHalfFill] =
//         {
//             .set = led_display_test_set_pattern_animated_rectangulars_half,
//             .name = "Animated Rectangulars Half Fill",
//         },
//     [LedDisplayTestPatternAnimFill10Noise] =
//         {
//             .set = led_display_test_set_pattern_animated_fill_10_noise,
//             .name = "Animated Fill 10% Noise",
//         },
//     [LedDisplayTestPatternAnimFill25Noise] =
//         {
//             .set = led_display_test_set_pattern_animated_fill_25_noise,
//             .name = "Animated Fill 25% Noise",
//         },
//     [LedDisplayTestPatternAnimFill50Noise] =
//         {
//             .set = led_display_test_set_pattern_animated_fill_50_noise,
//             .name = "Animated Fill 50% Noise",
//         },
// };

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

// void led_display_test_set(LedDisplayTestPattern pattern, LedDisplayTestColor color) {
//     furi_check(pattern < LedDisplayTestPatternNum);
//     furi_check(color < LedDisplayTestColorNum);

//     led_display_reset();
//     led_display_test_pattern[pattern].set(led_display_test_color[color].code);
// }

// const char* led_display_get_pattern_str(LedDisplayTestPattern pattern) {
//     furi_check(pattern < LedDisplayTestPatternNum);

//     return led_display_test_pattern[pattern].name;
// }

// const char* led_display_get_color_str(LedDisplayTestColor color) {
//     furi_check(color < LedDisplayTestColorNum);

//     return led_display_test_color[color].name;
// }
