/** Status/error codes of sxsymcrypt functions
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 */

#ifndef STATUSCODES_HEADER_FILE
#define STATUSCODES_HEADER_FILE

#ifndef SX_OK
/** The function or operation succeeded */
#define SX_OK 0
#endif

/** Waiting on the hardware to process this operation */
#define SX_ERR_HW_PROCESSING -1

/** No hardware available for a new operation. Retry later. */
#define SX_ERR_RETRY -2

/** No compatible hardware for this operation.
 *
 * This error occurs if the dedicated hardware to execute the operation is not
 * present, or hardware is present and operation not supported by it.
 */
#define SX_ERR_INCOMPATIBLE_HW -3

/** Invalid authentication tag in authenticated decryption */
#define SX_ERR_INVALID_TAG -16

/** Hardware DMA error
 *
 * Fatal error that should never happen. Can be caused by invalid or
 * wrong addresses, RAM corruption, a hardware or software bug or system
 * corruption.
 */
#define SX_ERR_DMA_FAILED -32

/** Fatal error, trying to call a function with an uninitialized object
 *
 * For example calling sx_aead_decrypt() with an sxaead object which
 * has not been created yet with sx_aead_create_*() function.
 */
#define SX_ERR_UNITIALIZED_OBJ -33

/** Fatal error, trying to call an AEAD or block cipher create function with an
 * uninitialized or invalid key reference.
 *
 * Examples: calling sx_blkcipher_create_aesecb() with a key reference which
 * has not been initialized yet with sx_keyref_load_material() or sx_keyref_load_by_id()
 * function, sx_keyref_load_material() was called with key NULL or size 0, or
 * sx_keyref_load_by_id() was called with an invalid index ID.
 */
#define SX_ERR_INVALID_KEYREF -34

/** Fatal error, trying to create instance with not enough memory */
#define SX_ERR_ALLOCATION_TOO_SMALL -35

/** Input or output buffer size too large */
#define SX_ERR_TOO_BIG -64

/** Input or output buffer size too small */
#define SX_ERR_TOO_SMALL -65

/** The given key size is not supported by the algorithm or the hardware */
#define SX_ERR_INVALID_KEY_SZ -66

/** Input tag size is invalid */
#define SX_ERR_INVALID_TAG_SIZE -67

/** Input nonce size is invalid */
#define SX_ERR_INVALID_NONCE_SIZE -68

/** Too many feeds were inputed */
#define SX_ERR_FEED_COUNT_EXCEEDED -69

/** Input data size granularity is incorrect */
#define SX_ERR_WRONG_SIZE_GRANULARITY -70

/** Attempt to use HW keys with a mode that does not support HW keys */
#define SX_ERR_HW_KEY_NOT_SUPPORTED -71

/** Attempt to use a mode or engine that does not support context saving */
#define SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED -72

/** Attempt to feed AAD after input data was fed */
#define SX_ERR_FEED_AFTER_DATA -73

/** Attempt to feed multiple IN and OUT buffers with total IN size
 * different than total OUT size.
 */
#define SX_ERR_FEED_UNBALANCED -74

/** Attempt to feed data even though data has already been fed without intermediate
 * hardware call.
 */
#define SX_ERR_FEED_AFTER_FEED -75

/** Hardware cannot work anymore.
 *
 * To recover, reset the hardware or call *_init() function.
 * For example, if this error is received when sx_trng_get()
 * is called, call again sx_trng_reset() to reset the hardware.
 */
#define SX_ERR_RESET_NEEDED -82

/** A self-test has failed
 *
 * For example, if this error is received when sx_trng_init_selftests() is
 * called, one of the TRNG self-tests failed and the HW cannot be used. */
#define SX_ERR_SELF_TEST_FAILURE -83

#endif
