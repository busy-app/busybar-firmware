/** AES counter-measure mask load, reads values from the command line and sends
 *  it to AES engine.
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <stddef.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/cmmask.h>
#include <sxsymcrypt/version.h>
#include "env/io.h"

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);

int aes_cm_load_mask(uint32_t value)
{
    struct sxcmmask c;
    int r;

    r = sx_cm_load_mask(&c, value);
    if (r != SX_OK)
        return r;

    r = sx_cm_load_mask_wait(&c);
    if (r != SX_OK)
        return r;

    return 0;
}


int main(int argc, char **argv)
{
    uint32_t value = 0;
    int i;
    int r = 0;

    if (argc < 2) {
        displaymsg("Usage: ./cmmask_sample [value1] ([value2] ...)\n\n");
        return -1;
    }

    sx_alloc_global_dmamem(0);

    for (i = 1; i < argc; i++) {
        value = sx_atoi(argv[i]);
        r |= aes_cm_load_mask(value);
    }

    return r;
}
