#pragma once

#include "back_display_factory.h"

#ifdef __cplusplus
extern "C" {
#endif

void back_display_pattern_update(Canvas* canvas, BackDisplayPattern pattern);

void back_display_pattern_to_string(BackDisplayPattern pattern, FuriString* str);

#ifdef __cplusplus
}
#endif
