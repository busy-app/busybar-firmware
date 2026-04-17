/** Common simple 3gpp modes.
 *
 * All 3gpp modes here perform simple encryption and decryption
 * without any authentication. MAC mode perform the authentication.
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 *
 * Examples:
 * The following examples show typical sequences of function calls for
 * encryption and decryption of a message.
   @code
      a. Encryption
          sx_3gpp_create_xxx(ctx, ...)
          sx_3gpp_crypt(ctx, ...)
          sx_3gpp_run(ctx)
          sx_3gpp_wait(ctx)
      b. Decryption
          sx_3gpp_create_xxx(ctx, ...)
          sx_3gpp_crypt(ctx, ...)
          sx_3gpp_run(ctx)
          sx_3gpp_wait(ctx)
      c. One-shot operation MAC generation
          sx_3gpp_mac_create_snow3g(ctx, ...)
          sx_3gpp_mac_feed(ctx, ...)
          sx_3gpp_generate(ctx, ...)
          sx_3gpp_wait(ctx)
      d. Cipher or key stream generation
          sx_3gpp_create_kasumi_keystream(ctx, ...)
          sx_3gpp_generate(ctx, ...)
          sx_3gpp_wait(ctx)
   @endcode
 */

#ifndef SXSYMCRYPT_3GPP_HEADER_FILE
#define SXSYMCRYPT_3GPP_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "internal.h"

struct sx3gpp;
struct sx3gppalg;

/** Mode, in integer, for selecting KASUMI as GSM_A53 key stream generation mode */
extern const struct sx3gppalg sx3gppalg_kasumi_gsm_a53;

/** Mode, in integer, for selecting KASUMI as ECSD_A53 key stream generation mode */
extern const struct sx3gppalg sx3gppalg_kasumi_ecsd_a53;

/** Mode, in integer, for selecting KASUMI as GEA3 key stream generation mode */
extern const struct sx3gppalg sx3gppalg_kasumi_gea3;

/** Mode, in integer, for selecting KASUMI as GSM_A54 key stream generation mode */
extern const struct sx3gppalg sx3gppalg_kasumi_gsm_a54;

/** Mode, in integer, for selecting KASUMI as ECSD_A54 key stream generation mode */
extern const struct sx3gppalg sx3gppalg_kasumi_ecsd_a54;

/** Mode, in integer, for selecting KASUMI as GEA4 key stream generation mode */
extern const struct sx3gppalg sx3gppalg_kasumi_gea4;


/** Prepare an SNOW 3G encryption and decryption.
 *
 * This function initializes the user allocated object \p c with a new 3gpp
 * operation context needed to run the SNOW 3G encryption and decryption
 * and reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the 3gpp functions.
 *
 * @param[out] c 3gpp operation context
 * @param[in] key used for the 3gpp operation, expected size 16 bytes
 * @param[in] dir direction of the transmision, downlink/uplink, expected value
 *                0 or non zero
 * @param[in] bearer value less then 32.
 * @param[in] count parameter used for the 3gpp operation
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 * @return ::SX_ERR_HW_KEY_NOT_SUPPORTED
 * @return ::SX_ERR_UNITIALIZED_OBJ
  *
 * @pre - key references provided by \p key must be initialized
 *        using sx_keyref_load_material().
 * @note Encryption/decryption operations are identical operations, therefore,
 *       this create function must be called for both cases.
 */
int sx_3gpp_create_snow3g(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir,
    uint32_t bearer, uint32_t count);


/** Prepare a ZUC encryption and decryption.
 *
 * This function initializes the user allocated object \p c with a new 3gpp
 * operation context needed to run the ZUC encryption and decryption and
 * reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the 3gpp functions.
 *
 * @param[out] c 3gpp operation context
 * @param[in] key used for the 3gpp operation, expected size 16 bytes
 * @param[in] dir direction of the transmision, downlink/uplink, expected value
 *                0 or non zero
 * @param[in] bearer value less than 32.
 * @param[in] count parameter used for the 3gpp operation
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 * @return ::SX_ERR_HW_KEY_NOT_SUPPORTED
 * @return ::SX_ERR_UNITIALIZED_OBJ
  *
 * @pre - key references provided by \p key must be initialized
 *        using sx_keyref_load_material().
 * @note Encryption/decryption operations are identical operations, therefore,
 *       this create function must be called for both cases.
 */
int sx_3gpp_create_zuc(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir,
    uint32_t bearer, uint32_t count);


/** Prepare a KASUMI encryption and decryption.
 *
 * This function initializes the user allocated object \p c with a new 3gpp
 * operation context needed to run the KASUMI encryption and decryption
 * and reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the 3gpp functions.
 *
 * @param[out] c 3gpp operation context
 * @param[in] key used for the 3gpp operation, expected size 16 bytes
 * @param[in] dir direction of the transmision, downlink/uplink, expected value
 *                0 or non zero
 * @param[in] bearer parameter used for the 3gpp operation, Bearer[4:0]
 *                   (27 MSB must be null, 5 LSB must be set with Bearer value).
 * @param[in] count parameter used for the 3gpp operation, size must be 4
 *                  bytes
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 * @return ::SX_ERR_HW_KEY_NOT_SUPPORTED
 * @return ::SX_ERR_UNITIALIZED_OBJ
  *
 * @pre - key references provided by \p key must be initialized
 *        using sx_keyref_load_material().
 * @note Encryption/decryption operations are identical operations, therefore,
 *       this create function must be called for both cases.
 */
int sx_3gpp_create_kasumi(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir, uint32_t bearer,
    uint32_t count);


/** Prepare a KASUMI key stream generation.
 *
 * This function initializes the user allocated object \p c with a new 3gpp
 * operation context needed to run the KASUMI key stream generation and
 * reserves the HW resource.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the 3gpp functions.
 *
 * @param[out] c 3gpp operation context
 * @param[in] key used for the 3gpp operation, expected size 16 bytes
 * @param[in] alg used for the 3gpp operation
 * @param[in] framein Frame dependent input (also called count for A5)
 * @param[in] outsz output data size in bytes (for GEA modes only).
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 * @return ::SX_ERR_HW_KEY_NOT_SUPPORTED
 * @return ::SX_ERR_UNITIALIZED_OBJ
  *
 * @pre - key references provided by \p key must be initialized
 *        using sx_keyref_load_material().
 */
int sx_3gpp_create_kasumi_keystream(struct sx3gpp *c,
    const struct sxkeyref *key, const struct sx3gppalg *alg,
    uint32_t framein, uint32_t outsz);


/** Add data to be encrypted/decrypted.
 *
 * The function will return immediately.
 *
 * In order to start the operation sx_3gpp_run() must be called.
 *
 * @param[in,out] c 3gpp operation context
 * @param[in] datain data to be encrypted or decrypted, with size \p inbitsz
 * @param[in] inbitsz size, in bits, of data to be decrypted
 * @param[out] dataout encrypted or decrypted data
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_TOO_BIG
 * @return ::SX_ERR_TOO_SMALL
 *
 * @pre - one of the sx_3gpp_create_*() functions must be called first
 */
int sx_3gpp_crypt(struct sx3gpp *c, const char *datain, size_t inbitsz,
    char *dataout);


/** Start a 3gpp operation.
 *
 * This function is used to start an encryption or a decryption based on what
 * create function was used, sx_3gpp_create_*_enc() or
 * sx_3gpp_create_*_dec(). The function will return immediately.
 *
 * The result will be transfered only after the operation is successfully
 * completed. The user shall check operation status with sx_3gpp_status()
 * or sx_3gpp_wait().
 *
 * @param[in,out] c 3gpp operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_TOO_SMALL
 *
 * @pre - sx_3gpp_crypt() function must be called first
 */
int sx_3gpp_run(struct sx3gpp *c);


/** Wait until the given 3gpp operation has finished
 *
 * This function returns when the 3gpp operation was successfully
 * completed, or when an error has occurred that caused the operation to
 * terminate. The return value of this function is the operation status.
 *
 * After this call, all resources have been released and \p c cannot be used
 * again unless sx_3gpp_create_*() is used.
 *
 * @param[in,out] c 3gpp operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_DMA_FAILED
 *
 * @see sx_3gpp_status().
 *
 * @remark - this function is blocking until operation finishes.
 */
int sx_3gpp_wait(struct sx3gpp *c);


/** Return the 3gpp operation status.
 *
 * If the operation is still ongoing, return ::SX_ERR_HW_PROCESSING.
 * In that case, the user can retry later.
 *
 * When this function returns with a code different than ::SX_ERR_HW_PROCESSING,
 * the 3gpp operation has ended and all resources used by 3gpp
 * operation context \p c have been released. In this case, \p c cannot be used
 * for a new operation until one of the sx_3gpp_create_*() functions is
 * called again.
 *
 * @param[in,out] c 3gpp operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_HW_PROCESSING
 * @return ::SX_ERR_DMA_FAILED
 *
 * @pre - sx_3gpp_crypt and sx_3gpp_run() functions must be called
 *        first
 */
int sx_3gpp_status(struct sx3gpp *c);


/** Prepare an SNOW 3G MAC generation.
 *
 * This function initializes the user allocated object \p c with a new SNOW 3G
 * MAC operation context needed to run the MAC generation.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the MAC functions.
 *
 * @param[out] c 3gpp operation context
 * @param[in] key key used for the MAC generation operation, expected size 16
 *                bytes
 * @param[in] dir direction of the transmision, downlink/uplink, expected value
 *                0 or non zero
 * @param[in] fresh parameter used for the 3gpp operation
 * @param[in] count parameter used for the 3gpp operation
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_3gpp_mac_create_snow3g(struct sx3gpp *c, const struct sxkeyref *key,
    const uint32_t dir, uint32_t fresh, uint32_t count);


/** Prepare an ZUC MAC generation.
 *
 * This function initializes the user allocated object \p c with a new ZUC MAC
 * operation context needed to run the MAC generation.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the MAC functions.
 *
 * @param[out] c 3gpp operation context
 * @param[in] key key used for the MAC generation operation, expected size 16
 *                bytes
 * @param[in] dir direction of the transmision, downlink/uplink, expected value
 *                0 or non zero
 * @param[in] bearer value less than 32.
 * @param[in] count parameter used for the 3gpp operation
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_3gpp_mac_create_zuc(struct sx3gpp *c, const struct sxkeyref *key,
    const uint32_t dir, uint32_t bearer, uint32_t count);


/** Prepare a Kasumi MAC generation.
 *
 * This function initializes the user allocated object \p c with a new Kasumi
 * MAC operation context needed to run the MAC generation.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the MAC functions.
 *
 * @param[out] c 3gpp operation context
 * @param[in] key key used for the MAC generation operation, expected size 16
 *                bytes
 * @param[in] dir direction of the transmision, downlink/uplink, expected value
 *                0 or non zero
 * @param[in] fresh parameter used for the 3gpp operation
 * @param[in] count parameter used for the 3gpp operation
 * @return ::SX_OK
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
  *
 * @pre - key reference provided by \p key must be initialized using
 *        sx_keyref_load_material() or sx_keyref_load_by_id()
 */
int sx_3gpp_mac_create_kasumi(struct sx3gpp *c, const struct sxkeyref *key,
    const uint32_t dir, uint32_t fresh, uint32_t count);


/** Feed data to authenticate with a MAC
 *
 * The function will return immediately.
 * After this, sx_3gpp_generate() should be called.
 *
 * @param[in,out] c 3gpp operation context
 * @param[in] datain data to be processed, with bit size \p bitsz
 * @param[in] bitsz size, in bits, of data to be processed
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_TOO_BIG
 * @return ::SX_ERR_FEED_COUNT_EXCEEDED
 *
 * @pre - sx_3gpp_mac_create_*() function must be called first
 *
 * @remark - this function can be called even if data size, \p sz, is 0.
 */
int sx_3gpp_mac_feed(struct sx3gpp *c, const char *datain, size_t bitsz);


/** Generate output based on previous input.
 *
 * The function will return immediately.
 *
 * @param[in,out] c 3gpp operation context
 * @param[out] output where the output will be written.
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 *
 * @pre - sx_3gpp_mac_create_*() function must be called first
 *
 * @remark - this function can be called even if data size, \p sz, is 0.
 */
int sx_3gpp_generate(struct sx3gpp *c, char *output);

#ifdef __cplusplus
}
#endif

#endif
