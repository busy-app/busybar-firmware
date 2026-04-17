/** Sample program for SHAKE256 computation (output size fixed to 114 bytes).
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */


#include <string.h>
#include <sxsymcrypt/hash.h>
#include <sxsymcrypt/sha3.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/memdiff.h>
#include <sxsymcrypt/statuscodes.h>
#include "hexdump.h"
#include "env/io.h"


#define DMA_TOTAL_SZ 1024

char *dmamem;


int run_shake_256()
{
    int r;
    struct sxhash c = {0};
    /* The test vector is from NIST file SHAKE256VariableOut.rsp. */
    const char msg[32] =
        "\xe3\xef\x12\x7e\xad\xfa\xfa\xf4\x04\x08\xce\xbb\x28\x70\x5d\xf3"
        "\x0b\x68\xd9\x9d\xfa\x18\x93\x50\x7e\xf3\x06\x2d\x85\x46\x17\x15";
    const char expected_out[114] =
        "\x73\x14\x00\x29\x48\xc0\x57\x00\x6d\x4f\xc2\x1e\x3e\x19\xc2\x58"
        "\xfb\x5b\xdd\x57\x72\x8f\xe9\x3c\x9c\x6e\xf2\x65\xb6\xd9\xf5\x59"
        "\xca\x73\xda\x32\xc4\x27\xe1\x35\xba\x0d\xb9\x00\xd9\x00\x3b\x19"
        "\xc9\xcf\x11\x6f\x54\x2a\x76\x04\x18\xb1\xa4\x35\xac\x75\xed\x5a"
        "\xb4\xef\x15\x18\x08\xc3\x84\x9c\x3b\xce\x11\xc3\xcd\x28\x5d\xd7"
        "\x5e\x5c\x9f\xd0\xa0\xb3\x2a\x89\x64\x0a\x68\xe6\xe5\xb2\x70\xf9"
        "\x66\xf3\x39\x11\xcf\xdf\xfd\x03\x48\x8b\x52\xb4\xc7\xfd\x1b\x22"
        "\x19\xde";
    char *dmamem_msg = dmamem;
    char *dmamem_out = dmamem_msg + sizeof(msg);

    memcpy(dmamem_msg, msg, sizeof(msg));

    r = sx_hash_create(&c, &sxhashalg_shake256_114, sizeof(c));
    assert(r == SX_OK);

    r = sx_hash_feed(&c, dmamem_msg, sizeof(msg));
    assert(r == SX_OK);

    r = sx_hash_digest(&c, dmamem_out);
    assert(r == SX_OK);

    r = sx_hash_wait(&c);
    assert(r == SX_OK);

    r = sx_memdiff(dmamem_out, expected_out, sizeof(expected_out));

    return r;
}


int main()
{
    int r;

    dmamem = sx_alloc_global_dmamem(DMA_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory.\n");
        return -1;
    }

    r = run_shake_256();

    return r;
}
