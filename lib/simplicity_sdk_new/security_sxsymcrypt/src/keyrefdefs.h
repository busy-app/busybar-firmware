/** Key reference common defines used by AES, SM4 and AEAD.
 *
 * @file
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#ifndef KEYREFDEFS_HEADER_FILE
#define KEYREFDEFS_HEADER_FILE

#include <stddef.h>


/* Used for validating key reference */
#define KEYREF_IS_INVALID(k) (((k) == NULL) || (((k)->key == NULL) && ((k)->cfg == 0)))


/* Used for checking if \p k is user provided key */
#define KEYREF_IS_USR(k) ((k)->cfg == 0)

/** BA411E-AES and BA415-AES GCM HP Config register -> KeySel[1:0] = [7:6], KeySel[5:2] = [31:28] */
#define KEYREF_AES_HWKEY_CONF(index) ((((index) & 0x3) << 6) | ((((index) >> 2) & 0xF) << 28))

/** BA419-SM4 Config register -> KeySel[1:0] = [7:6] */
#define KEYREF_SM4_HWKEY_CONF(index) (((index) & 0x3) << 6)

/** BA417-ChaCha20Poly1305 Config register -> KeySel[5:0] = [13:8] */
#define KEYREF_CHACHAPOLY_HWKEY_CONF(index) (((index) & 0x3F) << 8)
#endif
