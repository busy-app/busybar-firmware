#include "float_tools.h"

#include <math.h>
#include <float.h>

bool float_is_equal(float a, float b) {
    return fabsf(a - b) <= FLT_EPSILON * fmaxf(fabsf(a), fabsf(b));
}

float float_lerp(float a, float b, float slider) {
    float diff = b - a;
    return a + (diff * slider);
}
