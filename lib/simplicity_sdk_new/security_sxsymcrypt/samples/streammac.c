/** Read a stream of input data from stdin and compute HMAC/CMAC on stdout.
  *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/hmac.h>
#include <sxsymcrypt/mac.h>
#include <sxsymcrypt/cmac.h>
#include <sxsymcrypt/sm4.h>
#include <sxsymcrypt/sha1.h>
#include <sxsymcrypt/sha2.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/version.h>
#include <stddef.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define MAX_TOTAL_SZ 4096
static char *dmamem;


int cmac_generate(int keysz)
{
    struct sxmac c;
    int r;
    size_t fullsz, datasz;
    const char *key, *data;
    char *mac;
    struct sxkeyref k;

    fullsz = readdata(dmamem, MAX_TOTAL_SZ);
    key = dmamem;
    data = key + keysz;
    datasz = fullsz - keysz;
    mac = dmamem;
    k = sx_keyref_load_material(keysz, key);
    r = sx_mac_create_aescmac(&c, &k);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, data, datasz);
    if (r != SX_OK)
        return r;
    r = sx_mac_generate(&c, mac);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(mac, 16);

    return r;
}


int hmac(int keysz, int (*create_func)(struct sxmac *c, struct sxkeyref *keyref))
{
    struct sxmac c;
    int r;
    size_t fullsz;
    char *msg = dmamem;
    char *mac = dmamem;

    struct sxkeyref keyref = sx_keyref_load_material(keysz, msg);
    fullsz = readdata(msg, MAX_TOTAL_SZ);

    r = create_func(&c, &keyref);
    assert(r == SX_OK);
    msg += keysz;
    fullsz -= keysz;
    while (fullsz) {
        size_t rsz = (fullsz < 64) ? fullsz : 64;
        r = sx_mac_feed(&c, msg, rsz);
        assert(r == SX_OK);
        msg += rsz;
        fullsz -= rsz;
    }
    r = sx_mac_generate(&c, mac);
    assert(r == SX_OK);
    r = sx_mac_wait(&c);
    assert(r == SX_OK);

    writedata(mac, c.macsz);

    return r;
}


#define SM4_KEY_SZ (16)
#define SM4_CMAC_MIN_INDATA_SZ (16)
#define SM4_CMAC_TAG_SZ (16)

int sm4_cmac_generate()
{
    struct sxmac c;
    int r;
    size_t fullsz, datasz;
    const char *key, *data;
    char *mac;
    struct sxkeyref k;

    fullsz = readdata(dmamem, MAX_TOTAL_SZ);
    key = dmamem;
    data = key + SM4_KEY_SZ;
    datasz = fullsz - SM4_KEY_SZ;
    mac = dmamem;
    k = sx_keyref_load_material(SM4_KEY_SZ, key);
    r = sx_mac_create_sm4cmac(&c, &k);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, data, datasz);
    if (r != SX_OK)
        return r;
    r = sx_mac_generate(&c, mac);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(mac, SM4_CMAC_TAG_SZ);

    return r;
}

#define SM4_CMAC_CTX_FEED_SZ (16)
int sm4_cmac_generate_ctx_saving()
{
    struct sxmac c;
    int r;
    size_t fullsz, datasz;
    const char *key, *data;
    char *mac;
    struct sxkeyref k;
    size_t i;

    fullsz = readdata(dmamem, MAX_TOTAL_SZ);
    key = dmamem;
    data = key + SM4_KEY_SZ;
    datasz = fullsz - SM4_KEY_SZ;
    assert(datasz > SM4_CMAC_MIN_INDATA_SZ);
    mac = dmamem;

    k = sx_keyref_load_material(SM4_KEY_SZ, key);
    r = sx_mac_create_sm4cmac(&c, &k);
    if (r != SX_OK)
        return r;

    for (i = 0; i < (datasz - SM4_CMAC_CTX_FEED_SZ); i += SM4_CMAC_CTX_FEED_SZ) {
        r = sx_mac_feed(&c, data + i, SM4_CMAC_CTX_FEED_SZ);
        if (r != SX_OK)
            return r;
        r = sx_mac_save_state(&c);
        if (r != SX_OK)
            return r;
        r = sx_mac_wait(&c);
        if (r != SX_OK)
            return r;
        r = sx_mac_resume_state(&c);
        if (r != SX_OK)
            return r;
    }

    r = sx_mac_feed(&c, data + i, datasz - i);
    if (r != SX_OK)
        return r;
    r = sx_mac_generate(&c, mac);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(mac, SM4_CMAC_TAG_SZ);

    return r;
}


void usage(void)
{
    displaymsg("Usage: sample <op> <keysize>\n");
    displaymsg("  op = 1 for HMAC-sha256 ; keysz (can be 0)\n");
    displaymsg("  op = 2 for HMAC-sha384 ; keysz (can be 0)\n");
    displaymsg("  op = 3 for HMAC-sha512 ; keysz (can be 0)\n");
    displaymsg("  op = 4 for HMAC-sha224 ; keysz (can be 0)\n");
    displaymsg("  op = 5 for HMAC-sha1 ; keysz (can be 0)\n");
    displaymsg("  op = 6 for AES CMAC generation\n");
    displaymsg("  op = 7 for SM4 CMAC generation\n");
    displaymsg("  op = 8 for SM4 CMAC generation with context saving\n");
    displaymsg("  op = 9 for HMAC-sha3-224 ; keysz (can be 0)\n");
    displaymsg("  op = 10 for HMAC-sha3-256 ; keysz (can be 0)\n");
    displaymsg("  op = 11 for HMAC-sha3-384 ; keysz (can be 0)\n");
    displaymsg("  op = 12 for HMAC-sha3-512 ; keysz (can be 0)\n");
}


int main(int argc, char **argv)
{
    int op;
    int r = 1;

    if (argc < 2) {
        usage();
        return -1;
    }
    op = sx_atoi(argv[1]);
    if ((op < 6 || op > 8)  && argc < 3) {
        usage();
        return -1;
    }

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return -1;
    }

    switch (op) {
    case 1:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha2_256);
        break;
    case 2:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha2_384);
        break;
    case 3:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha2_512);
        break;
    case 4:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha2_224);
        break;
    case 5:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha1);
        break;
    case 6:
        r = cmac_generate(16);
        break;
    case 7:
        r = sm4_cmac_generate();
        break;
    case 8:
        r = sm4_cmac_generate_ctx_saving();
        break;
    case 9:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha3_224);
        break;
     case 10:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha3_256);
        break;
     case 11:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha3_384);
        break;
     case 12:
        r = hmac(sx_atoi(argv[2]), sx_mac_create_hmac_sha3_512);
        break;
    }
    if (r != SX_OK)
        return r;

    return 0;
}

