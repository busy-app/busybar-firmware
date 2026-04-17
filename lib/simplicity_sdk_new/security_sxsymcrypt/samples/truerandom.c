/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 *
 * Sample showing how to use Secure-IC TRNG. After initializing the TRNG, it
 * generates random bytes and writes them on standard output.
 *
 *  Usage: truerandom [length]
 *    The length argument specifies the output size in bytes. Default is 16.
 */

#include <sxsymcrypt/trng.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/version.h>
#include "env/io.h"

#ifndef TRNG_MAX_CHUNK_SZ
#define TRNG_MAX_CHUNK_SZ 32
#endif

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);

int main(int argc, char **argv)
{
    int length = 16;
    int result;
    int chunksz;
    char rnd_bytes[TRNG_MAX_CHUNK_SZ];
    struct sx_trng ctx;

    if (argc > 1)
        length = sx_atoi(argv[1]);

    result = sx_trng_init(&ctx, NULL);
    if (result != SX_OK)
        return result;

    while (length > 0) {
        chunksz = (length > TRNG_MAX_CHUNK_SZ) ? TRNG_MAX_CHUNK_SZ : length;
        result = sx_trng_get(&ctx, rnd_bytes, chunksz);
        if (result == SX_ERR_HW_PROCESSING)
            continue;
        if (result != SX_OK)
            return result;
        writedata(rnd_bytes, chunksz);
        length -= chunksz;
    }

    return 0;
}
