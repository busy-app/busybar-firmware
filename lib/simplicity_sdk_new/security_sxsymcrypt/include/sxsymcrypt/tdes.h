/** Triple DES (TDES) block cipher, supported modes are ECB and CBC.
 *
 * The "create operation" functions are specific to TDES. The TDES encrypt and
 * decrypt are done using the following block cipher API functions:
 * sx_blkcipher_encrypt(), sx_blkcipher_decrypt, sx_blkcipher_status() and
 * sx_blkcipher_wait().
 *
 * @remark - supported key size is 24 bytes
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 *
 * Examples:
 * The following examples show typical sequences of function calls for
 * encryption and decryption a message.
   @code
   1. Encryption
       sx_blkcipher_create_tdesecb_enc(ctx, ...)
       sx_blkcipher_crypt(ctx, ...)
       sx_blkcipher_run(ctx)
       sx_blkcipher_wait(ctx)
   2. Decryption
       sx_blkcipher_create_tdesecb_dec(ctx, ...)
       sx_blkcipher_crypt(ctx, ...)
       sx_blkcipher_run(ctx)
       sx_blkcipher_wait(ctx)
   @endcode
 */

#ifndef TDES_HEADER_FILE
#define TDES_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "internal.h"


/** Prepares a TDES ECB block cipher encryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the TDES ECB operation and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, size must be 24 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @remark - TDES ECB works with data multiple of block size(8 bytes).
 */
int sx_blkcipher_create_tdesecb_enc(struct sxblkcipher *c, const char *key);


/** Prepares a TDES ECB block cipher decryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the TDES ECB operation and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, size must be 24 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @remark - TDES ECB works with data multiple of block size(8 bytes).
 */
int sx_blkcipher_create_tdesecb_dec(struct sxblkcipher *c, const char *key);


/** Prepares a TDES CBC block cipher encryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the TDES CBC operation and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, size must be 24 bytes
 * @param[in] iv initialization vector, size must 8 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @remark - TDES CBC works with data multiple of block size(8 bytes).
 */
int sx_blkcipher_create_tdescbc_enc(struct sxblkcipher *c, const char *key,
    const char *iv);


/** Prepares a TDES CBC block cipher decryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the TDES CBC operation and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, size must be 24 bytes
 * @param[in] iv initialization vector, size must 8 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @remark - TDES CBC works with data multiple of block size(8 bytes).
 */
int sx_blkcipher_create_tdescbc_dec(struct sxblkcipher *c, const char *key,
    const char *iv);

#ifdef __cplusplus
}
#endif

#endif
