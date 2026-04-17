/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/keyref.h"


#ifndef SX_KEYREF_MAX_ID
#define SX_KEYREF_MAX_ID 1
#else
#if SX_KEYREF_MAX_ID > 62
#error "SX_KEYREF_MAX_ID exceeds maximum number of HW keys. Maximum 62 keys."
#endif
#endif

struct sxkeyref sx_keyref_load_material(size_t keysz, const char *keymaterial)
{
    struct sxkeyref k;
    k.key = keymaterial;
    k.sz = keysz;
    k.cfg = 0;

    return k;
}


struct sxkeyref sx_keyref_load_by_id(size_t keyindex)
{
    struct sxkeyref k;

    k.key = NULL;
    k.sz = 0;
    if (keyindex > SX_KEYREF_MAX_ID) {
        k.cfg = 0;
        return k;
    }
    k.cfg = keyindex + 1;

    return k;
}
