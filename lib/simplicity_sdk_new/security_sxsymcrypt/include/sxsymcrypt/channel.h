/** Channel functionality.
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 */

#ifndef CHANNEL_HEADER_FILE
#define CHANNEL_HEADER_FILE

#include <stddef.h>
#include "internal.h"


/** Adds data to be transformed.
 *
 * @param[in,out] c channel context
 * @param[in] src address of the data source to be transformed
 * @param[in] sz size, in bytes, of data to be transformed
 * @param[out] dst address of the destination where data will be transformed
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 *
 * @pre - sx_transfer_create_copier() functions must be called first
 */
int sx_channel_transform(struct sxchannel *c, const char *src, const size_t sz,
        char *dst);


/** Starts a channel operation.
 *
 * @param[in,out] c channel context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_TOO_SMALL
 *
 * @pre - sx_channel_transform() must be called first
 */
int sx_channel_run(struct sxchannel *c);


/** Returns the status of the given channel context.
 *
 * If the operation is still ongoing, return ::SX_ERR_HW_PROCESSING.
 * In that case, the user can retry later.
 *
 * When this function returns with a code different than ::SX_ERR_HW_PROCESSING,
 * the channel operation has ended and all resources used by channel operation
 * context \p c have been released. In this case, \p c cannot be used.
 *
 * @param[in,out] c channel context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_HW_PROCESSING
 * @return ::SX_ERR_DMA_FAILED
 *
 * @pre - sx_channel_run() must be called first
 */
int sx_channel_status(struct sxchannel *c);


/** Waits until the given channel operation has finished.
 *
 * This function returns when the channel operation was successfully completed,
 * or when an error has occurred that caused the operation to terminate.
 *
 * The return value of this function is the operation status.
 *
 * After this call, all resources have been released and \p c cannot be used
 * again.
 *
 * @param[in,out] c channel context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_DMA_FAILED
 *
 * @pre - or sx_channel_transform() must be called first
 *
 * @see sx_channel_status().
 */
int sx_channel_wait(struct sxchannel *c);

#endif
