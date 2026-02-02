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

#ifdef __cplusplus
}
#endif
