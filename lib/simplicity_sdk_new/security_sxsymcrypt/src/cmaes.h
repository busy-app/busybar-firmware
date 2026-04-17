/** Common definitions for Cryptomaster AES modules
 *
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#ifndef CMAES_HEADER_FILE
#define CMAES_HEADER_FILE

#define CM_CFG_DECRYPT 1
#define CM_CFG_ENCRYPT 0
#define CM_TEMPORAL_REDUNDANCY_CFG_EN  (1u << 1)
#define CM_TEMPORAL_REDUNDANCY_CFG2_EN (1u << 28)
#define CM_AES_TEMPO_REDUNDANCY_IS_SET(config) (((config) & (1u)) == 1u)

static inline uint32_t sx_aes_keysz(size_t keysz)
{
    if (keysz == 16)
        return (0u << 2);
    else if (keysz == 24)
        return (1u << 3);
    else if (keysz == 32)
        return (1u << 2);
    else
        return ~0;
}

#endif
