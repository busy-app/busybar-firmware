/** Hardware interrupts
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 */

#ifndef INTERRUPTS_HEADER_FILE
#define INTERRUPTS_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "internal.h"

/** Prepares the hardware to use hardware interrupts.
 *
 * This function may be called only once, before any function that starts an
 * aead, blkcipher, or hash operation.
 *
 * @return ::SX_OK
 *
 * @remark - hardware interrupts are not available for cmmask.
 */
int sx_interrupts_enable(void);

/** Disables all hardware interrupts.
 *
 * This function may be called only when there is no ongoing hardware
 * processing.
 *
 * @return ::SX_OK
 */
int sx_interrupts_disable(void);

#ifdef __cplusplus
}
#endif

#endif
