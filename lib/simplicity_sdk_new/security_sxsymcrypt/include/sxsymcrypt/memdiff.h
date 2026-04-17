/** Buffer comparison
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 */

#ifndef MEMDIFF_HEADER_FILE
#define MEMDIFF_HEADER_FILE

#include "sl_code_classification.h"

#ifdef __cplusplus
extern "C" {
#endif

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static inline int sx_memdiff(const char *a, const char *b, size_t sz)
{
    int r = 0;
    size_t i;

    for (i = 0; i < sz; i++, a++, b++)
        r |= *a ^ *b;

    return r;
}


#ifdef __cplusplus
}
#endif

#endif
