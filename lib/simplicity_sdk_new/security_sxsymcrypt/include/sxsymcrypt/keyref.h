/** Common function definitions for keys.
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 *
 * SPDX-License-Identifier: Zlib
 */

#ifndef KEYREF_HEADER_FILE
#define KEYREF_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include "internal.h"
#include "sl_code_classification.h"

/** Returns a reference to a key whose key material is in user memory.
 *
 * This function loads the user provided key data and returns an initialized
 * sxkeyref object.
 *
 * The returned object can be passed to any of the sx_aead_create_*() or
 * sx_blkcipher_create_*() functions.
 *
 * @param[in] keysz size of the key to be loaded
 * @param[in] keymaterial key to be loaded with size \p keysz
 * @return sxkeyref initialized object with provided inputs
 *
 * @remark - \p keymaterial buffer should not be changed until the operation
 *           is completed.
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
struct sxkeyref sx_keyref_load_material(size_t keysz, const char *keymaterial);


/** Returns a reference to a key selected by an index.
 *
 * This function initializes a sxkeyref object to use predefined hardware keys.
 * Currently, predefined hardware keys can be used with AES(BA411) and
 * SM4(BA419).
 *
 * The returned object can be passed to any of the sx_aead_create_*() or
 * sx_blkcipher_create_*() functions.
 *
 * @param[in] keyindex index of the hardware key, must be 0 or 1.
 * @return sxkeyref initialized object with configuration of the hardware key
 *         index provided
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
struct sxkeyref sx_keyref_load_by_id(size_t keyindex);

#ifdef __cplusplus
}
#endif

#endif
