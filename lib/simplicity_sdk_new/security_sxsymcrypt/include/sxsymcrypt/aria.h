/** ARIA common modes.
 *
 * Supported modes are ECB, CBC, OFB, CFB, CTR, GCM, CCM and CMAC.
 *
 * The "create operation" functions are specific to ARIA.
 * The ARIA encrypt and decrypt are done using the following block cipher API
 * functions: sx_blkcipher_crypt(), sx_blkcipher_encrypt(), sx_blkcipher_decrypt(),
 * sx_blkcipher_resume_state(), sx_blkcipher_save_state(), , sx_blkcipher_status()
 * and sx_blkcipher_wait(). For further reference check blkcipher.h.
 *
 * The ARIA AEAD tag produce and tag verify are done using the following
 * AEAD API functions: sx_aead_feed_aad(), sx_aead_crypt(), sx_aead_produce_tag(),
 * sx_aead_verify_tag(), sx_aead_resume_state(), sx_aead_save_state(),
 * sx_aead_status() and sx_aead_wait(). For further reference check aead.h.
 *
 * The ARIA CMAC message authentication code generation is using the following
 * MAC API functions: sx_mac_feed(), sx_mac_generate(), sx_mac_resume_state(),
 * sx_mac_save_state(), sx_mac_status() and sx_mac_wait(). For further reference
 * check mac.h.
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 *
 * Examples:
 * The following examples show typical sequences of function calls for
 * encrypting a message. The decryption mechanism is identical to the
 * encryption besides the create function that needs to be one of the
 * sx_blkcipher_aria*_dec() functions.
   @code
   1. One-shot encryption operation
          sx_blkcipher_create_ariacbc_enc(ctx, ...)
          sx_blkcipher_crypt(ctx, ...)
          sx_blkcipher_run(ctx)
          sx_blkcipher_wait(ctx)
   2. Context-saving encryption operation
          First round:
              sx_blkcipher_create_ariacbc_enc(ctx)
              sx_blkcipher_crypt(ctx, 'first chunk')
              sx_blkcipher_save_state(ctx)
              sx_blkcipher_wait(ctx)
          Intermediary rounds:
              sx_blkcipher_resume_state(ctx)
              sx_blkcipher_crypt(ctx, 'n-th chunk')
              sx_blkcipher_save_state(ctx)
              sx_blkcipher_wait(ctx)
          Last round:
              sx_blkcipher_resume_state(ctx)
              sx_blkcipher_crypt(ctx, 'last chunk')
              sx_blkcipher_run(ctx)
              sx_blkcipher_wait(ctx)
   @endcode
 */

#ifndef ARIA_HEADER_FILE
#define ARIA_HEADER_FILE

#include <stddef.h>
#include "internal.h"


/** Prepares an ARIA CTR block cipher encryption.
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA CTR encryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariactr_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA CTR block cipher decryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA CTR decryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariactr_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA ECB block cipher encryption.
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA ECB encryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariaecb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key);


/** Prepares an ARIA ECB block cipher decryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA ECB decryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariaecb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key);


/** Prepares an ARIA CBC block cipher encryption.
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA CBC encryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariacbc_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA CBC block cipher decryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA CBC decryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariacbc_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA CFB block cipher encryption.
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA CFB encryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariacfb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA CFB block cipher decryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA CFB decryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariacfb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA OFB block cipher encryption.
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA OFB encryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariaofb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA OFB block cipher decryption
 *
 * This function initializes the user allocated object \p c with a new block
 * cipher operation context needed to run the ARIA OFB decryption and reserves
 * the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions.
 *
 * @param[out] c block cipher operation context
 * @param[in] key key used for the block cipher operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_blkcipher_create_ariaofb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv);


/** Prepares an ARIA GCM AEAD encryption operation.
 *
 * This function initializes the user allocated object \p c with a new AEAD
 * encryption operation context needed to run the ARIA GCM operation and
 * reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the AEAD functions.
 *
 * @param[out] c AEAD operation context
 * @param[in] key key used for the AEAD operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 12 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 *
 * @remark - \p key and \p iv buffers should not be changed until the operation
 *           is completed.
 * @remark - GMAC is supported by using GCM with plaintext with size 0.
 * @remark - GCM and GMAC support AAD split in multiple chunks, using context
 *           saving.
 */
int sx_aead_create_ariagcm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *iv);


/** Prepares an ARIA GCM AEAD decryption operation.
 *
 * This function initializes the user allocated object \p c with a new AEAD
 * decryption operation context needed to run the ARIA GCM operation and
 * reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the AEAD functions.
 *
 * @param[out] c AEAD operation context
 * @param[in] key key used for the AEAD operation, expected size 16, 24 or 32 bytes
 * @param[in] iv initialization vector, size must be 12 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 *
 * @remark - \p key and \p iv buffers should not be changed until the operation
 *           is completed.
 * @remark - GMAC is supported by using GCM with ciphertext with size 0.
 * @remark - GCM and GMAC support AAD split in multiple chunks, using context
 *           saving.
 */
int sx_aead_create_ariagcm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *iv);


/** Prepares an ARIA CCM AEAD encryption operation.
 *
 * This function initializes the user allocated object \p c with a new AEAD
 * encryption operation context needed to run the ARIA GCM operation and
 * reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the AEAD functions.
 *
 * @param[out] c AEAD operation context
 * @param[in] key key used for the AEAD operation, expected size 16, 24 or 32 bytes
 * @param[in] noncesz size, in bytes, of the nonce, between 7 and 13 bytes
 * @param[in] nonce nonce used for the AEAD operation, with size \p noncesz
 * @param[in] tagsz size, in bytes, of the tag used for the AEAD operation,
 *            must be a value in {4, 6, 8, 10, 12, 14, 16}
 * @param[in] aadsz size, in bytes, of the additional authenticated data(AAD)
 * @param[in] datasz size, in bytes, of the data to be processed
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 *
 * @remark - the same aadsz and datasz must be provided to sx_aead_encrypt()
 *           or sx_aead_decrypt() functions.
 * @remark - \p key and \p nonce buffers should not be changed until the
 *           operation is completed.
 * @remark - CCM DOES NOT support AAD split in multiple chunks
 */
int sx_aead_create_ariaccm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz,
    size_t tagsz, size_t aadsz, size_t datasz);


/** Prepares an ARIA CCM AEAD decryption operation.
 *
 * This function initializes the user allocated object \p c with a new AEAD
 * decryption operation context needed to run the ARIA GCM operation and
 * reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the AEAD functions.
 *
 * @param[out] c AEAD operation context
 * @param[in] key key used for the AEAD operation, expected size 16, 24 or 32 bytes
 * @param[in] noncesz size, in bytes, of the nonce, between 7 and 13 bytes
 * @param[in] nonce nonce used for the AEAD operation, with size \p noncesz
 * @param[in] tagsz size, in bytes, of the tag used for the AEAD operation,
 *            must be a value in {4, 6, 8, 10, 12, 14, 16}
 * @param[in] aadsz size, in bytes, of the additional authenticated data(AAD)
 * @param[in] datasz size, in bytes, of the data to be processed
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 *
 * @remark - the same aadsz and datasz must be provided to sx_aead_encrypt()
 *           or sx_aead_decrypt() functions.
 * @remark - \p key and \p nonce buffers should not be changed until the
 *           operation is completed.
 * @remark - CCM DOES NOT support AAD split in multiple chunks
 */
int sx_aead_create_ariaccm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz,
    size_t tagsz, size_t aadsz, size_t datasz);


/** Prepares an ARIA CMAC generation.
 *
 * This function initializes the user allocated object \p c with a new ARIA CMAC
 * operation context needed to run the MAC generation and reserves the HW
 * resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the MAC functions.
 *
 * @param[out] c MAC operation context
 * @param[in] key key used for the MAC generation operation, size must be
 *                16 bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_mac_create_ariacmac(struct sxmac *c, const struct sxkeyref *key);
#endif
