/** Sample for usage of interrupts with a large hash.
 *
 * This sample hashes a 60KB message, split into 5 feeds, each feed contains
 * same buffer, 12288 times 0xAB. For each hash only one interrupt is received,
 * when digest was generated.
 * The sample executes the hash operation 1024 times. A total of 60MB of data
 * are processed.
 *
 * This sample can be run in Linux using "time" command to see that the time
 * spent in "user" is very small. This emphasize that interrupts offload the
 * CPU.
 * Example: time ./largesha256_irq
  *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/hash.h>
#include <sxsymcrypt/sha2.h>
#include <sxsymcrypt/interrupts.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define MAX_TOTAL_SZ 16*1024
#define MSG_SZ 12*1024
#define TEST_MAX_NO 1024
#define HASH_MAX_FEED 5
static char *dmamem;

static const char reference_digest[32] =
        "\x4f\xcc\x0b\x1e\xf3\xaf\x96\xef"
        "\x07\xf3\xf1\xfa\x02\x37\x2b\xfb"
        "\xc8\x2b\xf6\xb8\xa2\xdf\x3d\xd6"
        "\x1c\xea\x9a\x0d\x12\xdc\xca\x6e";


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct sxhash c = {0};
    char *msg, *digest;
    int r = 0;

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return 1;
    }
    msg = dmamem;
    digest = msg + MSG_SZ;
    memset(msg, 0xAB, MSG_SZ);

    //Enable interrupts
    sx_interrupts_enable();

    for (int i = 0; i < TEST_MAX_NO; i++) {
        r = sx_hash_create(&c, &sxhashalg_sha2_256, sizeof(c));
        if (r != SX_OK)
            break;
        for (int k = 0; k < HASH_MAX_FEED; k++) {
            r = sx_hash_feed(&c, msg, MSG_SZ);
            if (r != SX_OK)
                break;
        }
        r = sx_hash_digest(&c, digest);
        if (r != SX_OK)
            break;
        r = sx_hash_wait(&c);
        if (r != SX_OK)
            break;

        if (sx_memdiff(digest, reference_digest, sizeof(reference_digest))) {
            r = ~SX_OK;
            break;
        }
    }

    //Disable interrupts
    sx_interrupts_disable();

    if (r != SX_OK)
        return r;

    return 0;
}
