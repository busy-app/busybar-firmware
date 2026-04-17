/** Sample for usage of transfer API.
 *
 * The sample copies a buffer of 1024 bytes from one location to another.
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include <sxsymcrypt/channel.h>
#include <sxsymcrypt/transfer.h>
#include "env/io.h"

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 2);

#define CHUNK_SZ (1024)
#define MAX_TOTAL_SZ (2 * CHUNK_SZ)

static char *dmamem;

int run_memory_copy()
{
    struct sxchannel c;
    char *src, *dst;
    int r = 0;

    src = dmamem;
    dst = src + CHUNK_SZ;

    /* Set src to a repeating sequence of incremented bytes. */
    for (size_t k = 0; k < CHUNK_SZ; k++)
        src[k] = (char)k;

    r = sx_transfer_create_copier(&c);
    if (r != SX_OK)
        return r;
    r = sx_channel_transform(&c, src, CHUNK_SZ, dst);
    if (r != SX_OK)
        return r;
    r = sx_channel_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_channel_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(src, dst, CHUNK_SZ);
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int r;

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return 1;
    }

    r = run_memory_copy();

    return r;
}
