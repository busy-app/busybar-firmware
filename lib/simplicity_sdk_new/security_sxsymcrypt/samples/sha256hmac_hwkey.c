/** Compute HMAC(sha256) on data read from stdin
 *
 * Implements keyed-hash message authentication code as explained in
 * https://en.wikipedia.org/wiki/Hmac.
 *
 * This implementation can use a predefined hardware key by specifying
 * the option -hw on the command line.
 *
 * Offloads HMAC computations to the hardware.
  *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/hash.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/mac.h>
#include <sxsymcrypt/hmac.h>
#include <string.h>
#include "hexdump.h"
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);

#define MAX_TOTAL_SZ 4096
static char *dmamem;


int sha256hmac(struct sxkeyref keyref)
{
    struct sxmac c;
    int r;
    size_t fullsz;
    char *msg = dmamem;
    char *mac = dmamem;

    fullsz = readdata(msg, MAX_TOTAL_SZ);

    r = sx_mac_create_hmac_sha2_256(&c, &keyref);
    if (r != SX_OK)
        return r;

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

    hexdump("hmac", mac, 32);

    return r;
}


void usage(void)
{
    displaymsg("Usage: sha256hmac_hwkey -hw\n");
    displaymsg("\n"
        "flags:\n"
        "   -hw - uses the ip block attached hmac key\n\n"
        "example:\n"
        " $ echo -n \"The quick brown fox jumps over the lazy dog\" | ./sha256hmac_hwkey -hw\n"
        "\n hmac:\n"
        " 38dd1332f6a5f060 3054ef03e2d49f27\n"
        " 6fc285dff5f7b8c8 e5a393b652d4dfd9\n"
    );
}


int main(int argc, char **argv)
{
    int r = 1;
    int usehwkey = 0;

    const char default_hw_key[32] =
        "\x3f\xd0\x71\x64\xa6\xd8\x1b\x62\x09\x9d\xed\x13\x8b\x89\x25\xec"
        "\x77\x8c\x7e\x34\x35\xc5\xa6\x1c\x78\xc9\xf5\x5d\x45\xca\xfc\x72";

    for(int i = 1; i < argc; i ++) {
        if (!strcmp(argv[i], "-hw")) {
            usehwkey = 1;
        } else if (!strcmp(argv[i], "-?")) {
            usage();
            return 0;
        }
    }

    struct sxkeyref keyref;

    if (usehwkey)
        keyref = sx_keyref_load_by_id(0);
    else
        keyref = sx_keyref_load_material(sizeof(default_hw_key), default_hw_key);

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return -1;
    }

    r = sha256hmac(keyref);

    if (r != SX_OK)
        return r;

    return 0;
}
