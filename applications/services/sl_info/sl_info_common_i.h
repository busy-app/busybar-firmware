#pragma once

#include <stdbool.h>

#define SL_INFO_KEY_LEN   (128)
#define SL_INFO_VALUE_LEN (128)

typedef struct {
    char key[SL_INFO_KEY_LEN + 1];
    char value[SL_INFO_VALUE_LEN + 1];
    bool is_last;
} SlInfoIntercomFrame;
