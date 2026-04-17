/*
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 */
#ifndef TRNG_INTERNAL_H
#define TRNG_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct sx_regs;

/** Internal state of the TRNG hardware
 *
 * All members should be considered INTERNAL and may not be accessed
 * directly.
 */
struct sx_trng {
    struct sx_regs *regs;
    int conditioning_key_set;
};

#ifdef __cplusplus
}
#endif

#endif
