/** AES counter-measures mask load
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 *
 * Examples:
 * The following example shows typical sequence of function calls for
 * loading the counter-measure mask.
   @code
       sx_cm_load_mask(ctx, value)
       sx_cm_load_mask_wait(ctx)
   @endcode
 */

#ifndef CMMASK_HEADER_FILE
#define CMMASK_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "internal.h"
#include "channel.h"


/** Provide random mask value for counter-measures of cipher hardware.
 *
 * Counter-measures are available for the AES, if enabled in HW.
 * This function initializes the user allocated object \p c with a new
 * counter-measures mask load operation context and reserves the HW resource.
 *
 * After the initialization of \p c, this function starts the load of the
 * counter-measure cryptographically secure random \p value.
 * The function will return immediately. No data will be received for the
 * load operation.
 *
 * The operation is considered successful if the status returned by
 * sx_cm_load_mask_wait() or sx_cm_load_mask_status() is ::SX_OK.
 *
 * When called multiple times, the random value is added to the entropy
 * inside the hardware that generates the mask values.
 *
 * @param[out] c counter-measures mask load operation context
 * @param[in] value counter-measures random value to be loaded
 * @return ::SX_OK
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @remark - it is under the user responsibility to call it after system boot
 *           (not automatically called).
 * @remark - for more details see the technical report: "Secure and Efficient
 *           Masking of AES - A Mission Impossible?", June 2004)
 */
int sx_cm_load_mask(struct sxcmmask *c, uint32_t value);


/** Waits until the given AES counter-measures load operation has finished.
 *
 * This function returns when the counter-measures load operation was
 * successfully completed, or when an error has occurred that caused the
 * operation to terminate.
 *
 * The return value of this function is the operation status.
 *
 * After this call, all resources have been released and \p c cannot be used
 * again unless sx_cm_load_mask() is used.
 *
 * @param[in,out] c counter-measures mask load operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_DMA_FAILED
 *
 * @pre - sx_cm_load_mask() function must be called first
 *
 * @see sx_cm_load_mask_status().
 *
 * @remark - this function is blocking until operation finishes.
 */
static inline int sx_cm_load_mask_wait(struct sxcmmask *c)
{
    return sx_channel_wait(&c->channel);
}


/** Returns the status of the given AES counter-measures load operation context.
 *
 * If the operation is still ongoing, return ::SX_ERR_HW_PROCESSING.
 * In that case, the user can retry later.
 *
 * When this function returns with a code different than ::SX_ERR_HW_PROCESSING,
 * the counter-measures mask load operation has ended and all resources used by
 * counter-measures mask load operation context \p c have been released. In this
 * case, \p c cannot be used for a new operation until sx_cm_load_mask() is
 * called again.
 *
 * @param[in,out] c counter-measures mask load operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_HW_PROCESSING
 * @return ::SX_ERR_DMA_FAILED
 *
 * @pre - sx_cm_load_mask() function must be called first
 */
static inline int sx_cm_load_mask_status(struct sxcmmask *c)
{
    return sx_channel_status(&c->channel);
}

#ifdef __cplusplus
}
#endif

#endif
