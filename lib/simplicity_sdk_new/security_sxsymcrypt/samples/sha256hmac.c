/** Compute HMAC(sha256) on data read from stdin
 *
 * Implements keyed-hash message authentication code as explained in
 * https://en.wikipedia.org/wiki/Hmac.
 *
 * Offloads all sha-256 computations to the hardware. The sample handles
 * context-switching. That allows HMAC messages bigger than the internal
 * input buffer of 1KB.
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/hash.h>
#include <sxsymcrypt/sha2.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/version.h>
#include <string.h>
#include "hexdump.h"
#include "env/io.h"

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define HMAC_BLOCK_SZ 64

/*
 * MAX_HASH_MSG_SZ, only for demo purpose.
 * Note: this value must take in consideration block size for the hash mode used.
*/
#define MAX_HASH_MSG_SZ 1024

#define MAX_TOTAL_SZ 4096
static char *dmamem;

struct hmacctx {
    struct sxhash c;
};


static inline int sx_strlen(const char* str)
{
   size_t r = 0;

    while (*(str + r))
        r++;

    return r;
}


/** Load a key smaller than HMAC block size (64B) */
static void hmacloadsmallkey(char *key, size_t ksz, char *dst)
{
    size_t i;

    assert(ksz <= HMAC_BLOCK_SZ);
    memset(dst, 0, HMAC_BLOCK_SZ);
    for (i = 0; i < ksz; i++) {
        *dst = *key;
        key++;
        dst++;
    }
}


/** Load a key bigger than HMAC block size (64B) */
static void hmacloadbigkey(char *key, size_t ksz, char *dst)
{
    struct sxhash c = {0};
    int r;

    assert(ksz > HMAC_BLOCK_SZ);
    memset(dst, 0, HMAC_BLOCK_SZ);
    r = sx_hash_create(&c, &sxhashalg_sha2_256, sizeof(c));
    assert(r == SX_OK);
    r = sx_hash_feed(&c, key, ksz);
    assert(r == SX_OK);
    r = sx_hash_digest(&c, dst);
    assert(r == SX_OK);
    r = sx_hash_wait(&c);
    assert(r == SX_OK);
}


/** Load a key for HMAC in dst
 *
 * dst must have room for HMAC_BLOCK_SIZE bytes.
 */
void hmacloadkey(char *key, size_t ksz, char *dst)
{
    if (ksz <= HMAC_BLOCK_SZ) {
        hmacloadsmallkey(key, ksz, dst);
    } else {
        hmacloadbigkey(key, ksz, dst);
    }
}


/** Xor bytes in 'buf' with a constant 'v' in place */
static void xorbuf(char *buf, char v, size_t sz)
{
    size_t i;

    for (i = 0; i < sz; i++) {
        *buf = *buf^ v;
        buf++;
    }
}


/** Compute the HMAC with SHA-256 over text read from stdin */
int sha256hmac(char *key, size_t ksz)
{
    char *msg = dmamem + HMAC_BLOCK_SZ;
    char *keypad = dmamem;
    char *digest = dmamem + HMAC_BLOCK_SZ;
    int r;
    size_t sz;
    struct hmacctx hmac = {{0}};

    /* Compute key_ipad = key' ^ 64 bytes of 0x36 */
    hmacloadkey(key, ksz, keypad);
    xorbuf(keypad, 0x36, HMAC_BLOCK_SZ);

    r = sx_hash_create(&hmac.c, &sxhashalg_sha2_256, sizeof(hmac.c));
    assert(r == SX_OK);
    r = sx_hash_feed(&hmac.c, keypad, HMAC_BLOCK_SZ);
    assert(r == SX_OK);

    while (1) {
        sz = readdata(msg, MAX_HASH_MSG_SZ);
        r = sx_hash_feed(&hmac.c, msg, sz);
        assert(r == SX_OK);
        if (sz != MAX_HASH_MSG_SZ)
            break;
        r = sx_hash_save_state(&hmac.c);
        assert(r == SX_OK);
        r = sx_hash_wait(&hmac.c);
        assert(r == SX_OK);
        r = sx_hash_resume_state(&hmac.c);
        assert(r == SX_OK);
    }

    r = sx_hash_digest(&hmac.c, digest);
    assert(r == SX_OK);
    r = sx_hash_wait(&hmac.c);
    assert(r == SX_OK);

    /* 2nd step: hash of (key_opad + digest).
     * With sx_hash_feed() we can hash the concatenation of 2
     * non-contiguous buffers in one go without copying. */
    r = sx_hash_create(&hmac.c, &sxhashalg_sha2_256, sizeof(hmac.c));
    assert(r == SX_OK);
    /* Convert key_ipad to key_opad */
    xorbuf(keypad, 0x36 ^ 0x5c, HMAC_BLOCK_SZ);
    r = sx_hash_feed(&hmac.c, keypad, HMAC_BLOCK_SZ);
    assert(r == SX_OK);
    r = sx_hash_feed(&hmac.c, digest, 32);
    assert(r == SX_OK);
    r = sx_hash_digest(&hmac.c, digest);
    assert(r == SX_OK);
    r = sx_hash_wait(&hmac.c);
    assert(r == SX_OK);

    hexdump("hmac", digest, 32);

    return r;
}


void usage(void)
{
    displaymsg("Usage: sha256hmac [key]\n");
    displaymsg("\n"
        "example:\n"
        " $ echo -n \"The quick brown fox jumps over the lazy dog\" | ./sha256hmac\n"
        "\n hmac:\n"
        " f7bc83f430538424 b13298e6aa6fb143\n"
        " ef4d59a149461759 97479dbc2d1a3cd8\n");
}


int main(int argc, char **argv)
{
    int r = 1;
    char *key;
    const char *srckey = "key";
    size_t ksz = 0;

    if (argc == 2) {
        srckey = argv[1];
    } else if (argc > 2) {
        usage();
        return -1;
    }

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return -1;
    }

    ksz = sx_strlen(srckey);
    key = dmamem + HMAC_BLOCK_SZ;
    memcpy(key, srckey, ksz);
    r = sha256hmac(key, ksz);

    return r;
}
