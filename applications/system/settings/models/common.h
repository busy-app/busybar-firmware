#pragma once

#define CLOSEST_MULTIPLE_OF(x, n)  \
    ({                             \
        __typeof__(x) _x = (x);    \
        __typeof__(n) _n = (n);    \
        ((_x + _n / 2) / _n) * _n; \
    })
