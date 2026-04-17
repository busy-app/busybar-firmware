/** Sample for usage of ZUC encryption, decryption, authentication.
 *
 * The sample computes a ZUC encryption, decryption, and authentication and
 * validates it against the reference.
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/3gpp.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 1);


#define MAX_TOTAL_SZ 4*128
static char *dmamem;


#define TV_ARRAY_SZ 2
#define MAC_TV_NUM 2


struct tv {
    uint32_t count;
    const uint32_t bearer;
    int direction;
    int bitlen;
    const char *key; /* key size is 16 bytes in ZUC */
    const char *ciphertext;
    const char *plaintext;
};


struct mac_tv {
    uint32_t count;
    uint32_t bearer;
    const char *key;
    int direction;
    int inbitsz;
    const char *input;
    const char *expect;
};

struct tv array[TV_ARRAY_SZ] = {
    {
        0x00056823,
        0x18,
        1,
        800,
        "\xe5\xbd\x3e\xa0\xeb\x55\xad\xe8\x66\xc6\xac\x58\xbd\x54\x30\x2a",
        /* cipher text */
        "\x14\xa8\xef\x69\x3d\x67\x85\x07\xbb\xe7\x27\x0a\x7f\x67\xff\x50"
        "\x06\xc3\x52\x5b\x98\x07\xe4\x67\xc4\xe5\x60\x00\xba\x33\x8f\x5d"
        "\x42\x95\x59\x03\x67\x51\x82\x22\x46\xc8\x0d\x3b\x38\xf0\x7f\x4b"
        "\xe2\xd8\xff\x58\x05\xf5\x13\x22\x29\xbd\xe9\x3b\xbb\xdc\xaf\x38"
        "\x2b\xf1\xee\x97\x2f\xbf\x99\x77\xba\xda\x89\x45\x84\x7a\x2a\x6c"
        "\x9a\xd3\x4a\x66\x75\x54\xe0\x4d\x1f\x7f\xa2\xc3\x32\x41\xbd\x8f"
        "\x01\xba\x22\x0d",
        /* plain text */
        "\x13\x1d\x43\xe0\xde\xa1\xbe\x5c\x5a\x1b\xfd\x97\x1d\x85\x2c\xbf"
        "\x71\x2d\x7b\x4f\x57\x96\x1f\xea\x32\x08\xaf\xa8\xbc\xa4\x33\xf4"
        "\x56\xad\x09\xc7\x41\x7e\x58\xbc\x69\xcf\x88\x66\xd1\x35\x3f\x74"
        "\x86\x5e\x80\x78\x1d\x20\x2d\xfb\x3e\xcf\xf7\xfc\xbc\x3b\x19\x0f"
        "\xe8\x2a\x20\x4e\xd0\xe3\x50\xfc\x0f\x6f\x26\x13\xb2\xf2\xbc\xa6"
        "\xdf\x5a\x47\x3a\x57\xa4\xa0\x0d\x98\x5e\xba\xd8\x80\xd6\xf2\x38"
        "\x64\xa0\x7b\x01",
    }, {
        0x66035492,
        0x0f,
        0,
        193,
        "\x17\x3d\x14\xba\x50\x03\x73\x1d\x7a\x60\x04\x94\x70\xf0\x0a\x29",
        /* cipher text */
        "\xa6\xc8\x5f\xc6\x6a\xfb\x85\x33\xaa\xfc\x25\x18\xdf\xe7\x84\x94"
        "\x0e\xe1\xe4\xb0\x30\x23\x8c\xc8\x00",
        /* plain text */
        "\x6c\xf6\x53\x40\x73\x55\x52\xab\x0c\x97\x52\xfa\x6f\x90\x25\xfe"
        "\x0b\xd6\x75\xd9\x00\x58\x75\xb2\x00",
    }
};


struct mac_tv mac_array[MAC_TV_NUM] = {
    {
        /* count */
        0x561eb2dd,
        /* bearer */
        0x14,
        /*key*/
        "\x47\x05\x41\x25\x56\x1e\xb2\xdd\xa9\x40\x59\xda\x05\x09\x78\x50",
        /* direction bit */
        0,
        /* input bits */
        90,
        /* input */
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        /* expected output */
        "\x67\x19\xa0\x88",
    }, {
        /* count */
        0xa94059da,
        /* bearer */
        0x0a,
        /*key*/
        "\xc9\xe6\xce\xc4\x60\x7c\x72\xdb\x00\x0a\xef\xa8\x83\x85\xab\x0a",
        /* direction bit */
        1,
        /* input bits */
        577,
        /* input */
        "\x98\x3b\x41\xd4\x7d\x78\x0c\x9e\x1a\xd1\x1d\x7e\xb7\x03\x91\xb1"
        "\xde\x0b\x35\xda\x2d\xc6\x2f\x83\xe7\xb7\x8d\x63\x06\xca\x0e\xa0"
        "\x7e\x94\x1b\x7b\xe9\x13\x48\xf9\xfc\xb1\x70\xe2\x21\x7f\xec\xd9"
        "\x7f\x9f\x68\xad\xb1\x6e\x5d\x7d\x21\xe5\x69\xd2\x80\xed\x77\x5c"
        "\xeb\xde\x3f\x40\x93\xc5\x38\x81\x00\x00\x00\x00",
        /* expected output */
        "\xfa\xe8\xff\x0b",
    }
};


int test_zuc(int dec)
{
    struct sxkeyref key;
    struct sx3gpp c;
    char *output;
    const char *expected, *input;
    int r = 0;
    size_t i = 0;
    struct tv *tv;

    for (i = 0; i < TV_ARRAY_SZ; i++) {
        tv = &array[i];
        output = dmamem;
        if (!dec) {
            input = tv->plaintext;
            expected = tv->ciphertext;
        } else {
            input = tv->ciphertext;
            expected = tv->plaintext;
        }
        key = sx_keyref_load_material(16, tv->key);
        r = sx_3gpp_create_zuc(&c, &key, tv->direction, tv->bearer, tv->count);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_crypt(&c, input, tv->bitlen, output);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_run(&c);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_wait(&c);
        if (r != SX_OK)
            return r;
        r = sx_memdiff(output, expected, (tv->bitlen + 7) / 8);
        if (r)
            return r;
    } /* end of loop */

    return 0;
}


int test_zuc_mac_generate(void)
{
    struct sxkeyref keyref;
    struct sx3gpp c;
    char *output_mac;
    int r = 0;
    struct mac_tv* tv;
    size_t i = 0;

    for (i = 0; i < MAC_TV_NUM; i++) {
        tv = &mac_array[i];
        keyref = sx_keyref_load_material(16, tv->key);
        output_mac = dmamem;
        r = sx_3gpp_mac_create_zuc(&c, &keyref, tv->direction, tv->bearer, tv->count);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_mac_feed(&c, tv->input, tv->inbitsz);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_generate(&c, output_mac);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_wait(&c);
        if (r != SX_OK)
            return r;
        r = sx_memdiff(tv->expect, output_mac, 4);
        if (r)
            return r;
    }
    return r;
}


int main(int argc, char **argv)
{
    int r = 0;

    (void)argc;
    (void)argv;
    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return 1;
    }
    /* ZUC encryption */
    r |= test_zuc(0);
    /* ZUC decryption */
    r |= test_zuc(1);
    /* ZUC authentication */
    r |= test_zuc_mac_generate();

    return r;
}

