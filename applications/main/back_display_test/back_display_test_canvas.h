#pragma once

#include "back_display_test.h"

#ifdef __cplusplus
extern "C" {
#endif

void back_display_test_canvas_update(
    Canvas* canvas,
    BackDisplayTestPattern pattern,
    BackDisplayTestColor color);

const char* back_display_test_pattern_to_string(BackDisplayTestPattern pattern);

const char* back_display_test_color_to_string(BackDisplayTestColor color);

#ifdef __cplusplus
}
#endif
