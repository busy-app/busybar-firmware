#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define CEILING_MULTIPLE_OF(x, n)  \
    ({                             \
        __typeof__(x) _x = (x);    \
        __typeof__(n) _n = (n);    \
        ((_x + _n - 1) / _n) * _n; \
    })

#include <stdbool.h>

/** Compare two floating point numbers
 * @param a         First number to compare
 * @param b         Second number to compare
 *
 * @return          bool true if a equals b, false otherwise
 */
bool float_is_equal(float a, float b);

/**
 * @brief Linear interpolation between `a` and `b`, selected by `slider`
 * @param a First value
 * @param b Second value
 * @param slider Selector: `0` means first value, `1` means second value,
 *               something in between means a mix of the two.
 */
float float_lerp(float a, float b, float slider);

#ifdef __cplusplus
}
#endif
