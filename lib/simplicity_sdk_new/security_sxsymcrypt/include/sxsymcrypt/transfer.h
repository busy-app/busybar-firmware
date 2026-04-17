/** Copy data using CryptoMaster DMA.
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 *
 * Examples:
 * The following example shows typical sequence of function calls for
 * using the CM DMA to copy data.
   @code
       sx_transfer_create_copier(ctx)
       sx_channel_transform(ctx, src1, sz1, dst1)
       sx_channel_run(ctx)
       sx_channel_wait(ctx)
   @endcode
 */

#ifndef TRANSFER_HEADER_FILE
#define TRANSFER_HEADER_FILE

#include <stddef.h>
#include "internal.h"


/** Prepares a transfer copier.
 *
 * Configure a channel context to copy memory from source to destination.
 * To give the source and destination, call sx_channel_transform().
 *
 * @param[out] c channel context
 * @return ::SX_OK
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @remark when using the copier, one must call sx_channel_transform() only once.
 */
int sx_transfer_create_copier(struct sxchannel *c);

#endif
