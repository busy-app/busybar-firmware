/** Read a stream of input data from stdin and write the encrypted or
 * decrypted result on stdout.
  *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include <sxsymcrypt/hash.h>
#include <sxsymcrypt/sha1.h>
#include <sxsymcrypt/sha2.h>
#include <sxsymcrypt/sha3.h>
#include <sxsymcrypt/sm3.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/version.h>
#include <stddef.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


/*
 * MAX_HASH_MSG_SZ, only for demo purpose.
 * Note: this value must take in consideration block size for the hash mode used.
*/
#define MAX_HASH_MSG_SZ 1024

#define MAX_TOTAL_SZ 4096
static char *dmamem;

#define ARRAY_COUNT(x) (sizeof(x)/sizeof((x)[0]))


int streamedhash(const struct sxhashalg *alg)
{
    size_t fullsz;
    size_t digestsz;
    int r;
    char *msg = dmamem;
    char *digest = dmamem;
    struct sxhash c = {0};

    r = sx_hash_create(&c, alg, sizeof(c));
    if (r != SX_OK)
        return r;
    digestsz = sx_hash_get_digestsz(&c);
    fullsz = readdata(msg, MAX_TOTAL_SZ);
    while (fullsz) {
        size_t rsz = (fullsz < 64) ? fullsz : 64;
        r = sx_hash_feed(&c, msg, rsz);
        assert(r == SX_OK);
        msg += rsz;
        fullsz -= rsz;
    }
    r = sx_hash_digest(&c, digest);
    assert(r == SX_OK);
    r = sx_hash_wait(&c);
    assert(r == SX_OK);

    writedata(digest, digestsz);

    return r;
}


int context_saving_sha(const struct sxhashalg *alg)
{
    char *msg = dmamem;
    char *digest = dmamem;
    int r;
    size_t sz;
    struct sxhash c = {0};

    r = sx_hash_create(&c, alg, sizeof(c));
    assert(r == SX_OK);

    sz = readdata(msg, MAX_TOTAL_SZ);

    uint32_t i = 0;

    //Processes block by block
    for (i = 0; i < (sz / MAX_HASH_MSG_SZ); i++) {
        r = sx_hash_feed(&c, msg + i * MAX_HASH_MSG_SZ, MAX_HASH_MSG_SZ);
        assert(r == SX_OK);
        r = sx_hash_save_state(&c);
        assert(r == SX_OK);
        r = sx_hash_wait(&c);
        assert(r == SX_OK);
        r = sx_hash_resume_state(&c);
        assert(r == SX_OK);
    }

    r = sx_hash_feed(&c, msg + i * MAX_HASH_MSG_SZ, sz % MAX_HASH_MSG_SZ);
    assert(r == SX_OK);
    sx_hash_digest(&c, digest);
    r = sx_hash_wait(&c);
    assert(r == SX_OK);

    writedata(digest, sx_hash_get_digestsz(&c));

    return r;
}

struct streamhashop {
    const char *name;
    const struct sxhashalg *alg;
};

struct streamhashop ops[] = {
    {"sha256", &sxhashalg_sha2_256},
    {"sha384", &sxhashalg_sha2_384},
    {"sha512", &sxhashalg_sha2_512},
    {"sha224", &sxhashalg_sha2_224},
    {"sha1", &sxhashalg_sha1},
    {"sm3", &sxhashalg_sm3},
};

void usage(void)
{
    int i;
    const int OPSCOUNT = ARRAY_COUNT(ops);

    displaymsg("Usage: streamhash <op>\n");
    for (i = 0; i < OPSCOUNT; i++) {
        displaymsg("  ");
        displaymsg(ops[i].name);
        display("  op =", i);
    }
    for (i = 0; i < OPSCOUNT; i++) {
        displaymsg("  ");
        displaymsg(ops[i].name);
        display("with context saving  op =", i + OPSCOUNT);
    }
}

int main(int argc, char **argv)
{
    int op;
    int r = 1;
    const int OPSCOUNT = ARRAY_COUNT(ops);

    if (argc < 2) {
        usage();
        return -1;
    }

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return -1;
    }
    op = sx_atoi(argv[1]);
    if (op < 0 || op >= (2 * OPSCOUNT)) {
        usage();
        return -1;
    }
    if (op < OPSCOUNT)
        r = streamedhash(ops[op].alg);
    else
        r = context_saving_sha(ops[op - OPSCOUNT].alg);

    if (r != SX_OK)
        return r;

    return 0;
}
