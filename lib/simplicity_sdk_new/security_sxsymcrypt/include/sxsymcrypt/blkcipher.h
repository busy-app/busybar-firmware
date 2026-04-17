/** Common simple block cipher modes.
 *
 * All block cipher modes here perform simple encryption and decryption
 * without any authentication.
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
   1. One-shot operation
      a. Encryption
          sx_blkcipher_create_aescbc_enc(ctx, ...)
          sx_blkcipher_crypt(ctx, ...)
          sx_blkcipher_run(ctx)
          sx_blkcipher_wait(ctx)
      b. Decryption
          sx_blkcipher_create_aescbc_dec(ctx, ...)
          sx_blkcipher_crypt(ctx, ...)
          sx_blkcipher_run(ctx)
          sx_blkcipher_wait(ctx)
   2. Context-saving operation
      a. Encryption
          First round:
              sx_blkcipher_create_aescbc_enc(ctx)
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
      b. Decryption
          First round:
              sx_blkcipher_create_aescbc_dec(ctx)
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

#ifndef BLKCIPHER_HEADER_FILE
#define BLKCIPHER_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
struct sxblkcipher;
struct sxdataref;


/** Adds data to be encrypted/decrypted.
 *
 * The function will return immediately.
 *
 * In order to start the operation sx_blkcipher_run() must be called.
 *
 * \p sz must ensure the following restrictions based on the mode used:
 *  Algorithm | \p sz        | Remarks
 *  --------: | :----------: | :-----------
 *        ECB | N * 16 bytes | N > 0
 *        CBC | N * 16 bytes | N > 0
 *        CFB | N * 16 bytes | N > 0
 *        OFB | N * 16 bytes | N > 0
 *        XTS | >= 16 bytes  | none
 *        CTR | > 0 bytes    | none
 *
 * The restrictions above are applicable ONLY for the last chunk of the data
 * to be processed when doing a partial operation. \p sz must be a multiple of
 * block size(16 bytes) when doing a partial operation(besides last chunk).
 *
 * @param[in,out] c block cipher operation context
 * @param[in] datain data to be encrypted or decrypted, with size \p sz
 * @param[in] sz size, in bytes, of data to be decrypted
 * @param[out] dataout encrypted or decrypted data, must have \p sz bytes
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_TOO_BIG
 *
 * @pre - one of the sx_blkcipher_create_*() functions must be called first
 *
 * @remark - this function can not be called after having called sx_blkcipher_crypt().
 * @remark - this function can not be called after having called sx_blkcipher_multifeed_crypt().
 */
int sx_blkcipher_crypt(struct sxblkcipher *c, const char *datain, size_t sz,
    char *dataout);


/** Adds multiple data to be encrypted/decrypted.
 *
 * This function is used when multiple data inputs and outputs need to be
 * used. The number of inputs and outputs can be different, however the total
 * input size must be equal to the total output size.
 *
 * The function will return immediately.
 *
 * In order to start the operation sx_blkcipher_run() must be called.
 *
 * @param[in,out] c block cipher operation context
 * @param[in] datain head of the array containing input buffers be encrypted
 *                   or decrypted, with count \p countdatain
 * @param[in] countdatain number of input buffers
 * @param[in] dataout head of the array containing output buffers where the encrypted or
 *                    decrypted data will be stored, with count \p countdataout
 * @param[in] countdataout number of output buffers
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_TOO_BIG
 * @return ::SX_ERR_TOO_SMALL
 * @return ::SX_ERR_FEED_COUNT_EXCEEDED
 * @return ::SX_ERR_FEED_UNBALANCED
 *
 * @pre - one of the sx_blkcipher_create_*() functions must be called first
 *
 * @remark - this function can be called even if data size, \p countdatain or
 *           \p countdataout, is 0.
 * @remark - this function can be called only once before calling
 *           the hardware.
 * @remark - this function can not be called after having called sx_blkcipher_crypt().
 * @remark - this function can not be called after having called sx_blkcipher_multifeed_crypt().
 * @remark - the total maximum number of IN buffer is SX_BLKCIPHER_EXTRA_DESC_IN + 1 and
 *           the total maximum number of OUT buffers is SX_BLKCIPHER_EXTRA_DESC_OUT + 1
 */
int sx_blkcipher_multifeed_crypt(struct sxblkcipher *c,
    const struct sxdataref *datain, size_t countdatain,
    const struct sxdataref *dataout, size_t countdataout);


/** Starts a block cipher operation.
 *
 * This function is used to start an encryption or a decryption based on what
 * create function was used, sx_blkcipher_create_*_enc() or
 * sx_blkcipher_create_*_dec(). The function will return immediately.
 *
 * The result will be transfered only after the operation is successfully
 * completed. The user shall check operation status with sx_blkcipher_status()
 * or sx_blkcipher_wait().
 *
 * @param[in,out] c block cipher operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_TOO_SMALL
 * @return ::SX_ERR_WRONG_SIZE_GRANULARITY
 *
 * @pre - sx_blkcipher_crypt() function must be called first
 */
int sx_blkcipher_run(struct sxblkcipher *c);


/** Resumes AES operation in context-saving.
 *
 * This function shall be called when using context-saving to load the state
 * that was previously saved by sx_blkcipher_save_state() in the sxblkcipher
 * operation context \p c. It must be called with the same sxblkcipher operation
 * context \p c that was used with sx_sxblkcipher_save_state(). It will reserve
 * all hardware resources required to run the partial AES operation.
 * Previously used mode and direction are already stored in sxblkcipher \p c.
 *
 * After successful execution of this function, the context \p c can be passed
 * to any of the block cipher functions, except the sx_blkcipher_create_*()
 * functions.
 *
 * @param[in,out] c block cipher operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED
 * @return ::SX_ERR_INVALID_KEYREF
 * @return ::SX_ERR_INVALID_KEY_SZ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_RETRY
 *
 * @pre - sx_blkcipher_create_aes*() and sx_blkcipher_save_state() functions
 *        must be called before, for first part of the message.
 * @pre - must be called for each part of the message(besides first), before
 *        sx_blkchiper_crypt() or sx_blkcipher_save_state().
 *
 * @remark - the user must not change the key until all parts of the message to
 *           be encrypted/decrypted are processed.
 * @remark - AES ECB does not support context saving.
 */
int sx_blkcipher_resume_state(struct sxblkcipher *c);


/** Starts a partial block cipher operation.
 *
 * This function is used to start a partial encryption or decryption of
 * \p datain. The function will return immediately.
 *
 * The partial result will be transfered to \p dataout only after the operation
 * is successfully completed. The user shall check operation status with
 * sx_blkcipher_status() or sx_blkcipher_wait().
 *
 * @param[in,out] c block cipher operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED
 *
 * @pre - sx_blkcipher_crypt() should be called first.
 *
 * @remark - the user must not change the key until all parts of the message to
 *           be encrypted/decrypted are processed.
 */
int sx_blkcipher_save_state(struct sxblkcipher *c);


/** Waits until the given block cipher operation has finished
 *
 * This function returns when the block cipher operation was successfully
 * completed, or when an error has occurred that caused the operation to
 * terminate. The return value of this function is the operation status.
 *
 * After this call, all resources have been released and \p c cannot be used
 * again unless sx_blkcipher_create_*() is used.
 *
 * @param[in,out] c block cipher operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_DMA_FAILED
 *
 * @see sx_blkcipher_status().
 *
 * @remark - this function is blocking until operation finishes.
 */
int sx_blkcipher_wait(struct sxblkcipher *c);


/** Returns the block cipher operation status.
 *
 * If the operation is still ongoing, return ::SX_ERR_HW_PROCESSING.
 * In that case, the user can retry later.
 *
 * When this function returns with a code different than ::SX_ERR_HW_PROCESSING,
 * the block cipher operation has ended and all resources used by block cipher
 * operation context \p c have been released. In this case, \p c cannot be used
 * for a new operation until one of the sx_blkcipher_create_*() functions is
 * called again.
 *
 * @param[in,out] c block cipher operation context
 * @return ::SX_OK
 * @return ::SX_ERR_UNITIALIZED_OBJ
 * @return ::SX_ERR_INCOMPATIBLE_HW
 * @return ::SX_ERR_HW_PROCESSING
 * @return ::SX_ERR_DMA_FAILED
 *
 * @pre - sx_blkcipher_crypt and sx_blkcipher_run() functions must be called
 *        first
 */
int sx_blkcipher_status(struct sxblkcipher *c);

#ifdef __cplusplus
}
#endif

#endif
