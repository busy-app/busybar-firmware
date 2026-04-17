/** Sample for usage of SNOW 3G encryption, decryption and authentication.
 *
 * The sample computes a Snow3G encryption, decryption, and authentication and
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
    uint32_t bearer;
    int direction;
    int bitlen;
    const char *key;
    const char *ciphertext;
    const char *plaintext;
};


struct mac_tv {
    uint32_t count;
    uint32_t fresh;
    const char *key;
    int direction;
    int inbitsz;
    const char *input;
    const char *expect;
};


struct tv array[TV_ARRAY_SZ] = {
    {
        0xC675A64B,
        0x0C,
        1,
        798,
        /* key size is always 16 bytes in Snow3G */
        "\x2B\xD6\x45\x9F\x82\xC4\x40\xE0\x95\x2C\x49\x10\x48\x05\xFF\x48",
        /* ciphertext */
        "\x3F\x67\x85\x07\x14\xB8\xDA\x69\xEF\xB7\x27\xED\x7A\x6C\x0C\x50"
        "\x71\x4A\xD7\x36\xC4\xF5\x60\x00\x06\xE3\x52\x5B\xE8\x07\xC4\x67"
        "\xC6\x77\xFF\x86\x4A\xF4\x5F\xBA\x09\xC2\x7C\xDE\x38\xF8\x7A\x1F"
        "\x84\xD5\x9A\xB2\x55\x40\x8F\x2C\x7B\x82\xF9\xEA\xD4\x1A\x1F\xE6"
        "\x5E\xAB\xEB\xFB\xC1\xF3\xA4\xC5\x6C\x9A\x26\xFC\xF7\xB3\xD6\x6D"
        "\x02\x20\xEE\x47\x75\xBC\x58\x17\x0A\x2B\x12\xF3\x43\x1D\x11\xB3"
        "\x44\xD6\xE3\x6C",
        /* plain text */
        "\x7E\xC6\x12\x72\x74\x3B\xF1\x61\x47\x26\x44\x6A\x6C\x38\xCE\xD1"
        "\x66\xF6\xCA\x76\xEB\x54\x30\x04\x42\x86\x34\x6C\xEF\x13\x0F\x92"
        "\x92\x2B\x03\x45\x0D\x3A\x99\x75\xE5\xBD\x2E\xA0\xEB\x55\xAD\x8E"
        "\x1B\x19\x9E\x3E\xC4\x31\x60\x20\xE9\xA1\xB2\x85\xE7\x62\x79\x53"
        "\x59\xB7\xBD\xFD\x39\xBE\xF4\xB2\x48\x45\x83\xD5\xAF\xE0\x82\xAE"
        "\xE6\x38\xBF\x5F\xD5\xA6\x06\x19\x39\x01\xA0\x8F\x4A\xB4\x1A\xAB"
        "\x9B\x13\x48\x80",
    }, {
        0x544D49CD,
        0x04,
        0,
        310,
        /* key size is always 16 bytes in Snow3G */
        "\x0A\x8B\x6B\xD8\xD9\xB0\x8B\x08\xD6\x4E\x32\xD1\x81\x77\x77\xFB",
        /* ciphertext */
        "\x48\x14\x8E\x54\x52\xA2\x10\xC0\x5F\x46\xBC\x80\xDC\x6F\x73\x49"
        "\x5B\x02\x04\x8C\x1B\x95\x8B\x02\x61\x02\xCA\x97\x28\x02\x79\xA4"
        "\xC1\x8D\x2E\xE3\x08\x92\x1C",
        /* plain text */
        "\xFD\x40\xA4\x1D\x37\x0A\x1F\x65\x74\x50\x95\x68\x7D\x47\xBA\x1D"
        "\x36\xD2\x34\x9E\x23\xF6\x44\x39\x2C\x8E\xA9\xC4\x9D\x40\xC1\x32"
        "\x71\xAF\xF2\x64\xD0\xF2\x48",
    }
};


struct mac_tv mac_array[MAC_TV_NUM] = {
    {
        /* count */
        0x38A6F056,
        /* fresh */
        0xB8AEFDA9,
        /*key*/
        "\x2B\xD6\x45\x9F\x82\xC5\xB3\x00\x95\x2C\x49\x10\x48\x81\xFF\x48",
        /* direction bit */
        0,
        /* input bits */
        88,
        /* input */
        "\x33\x32\x34\x62\x63\x39\x38\x61"
        "\x37\x34\x79",
        /* expected output */
        "\xEE\x41\x9E\x0D",
    }, {
        /* count */
        0x36AF6144,
        /* fresh */
        0x9838F03A,
        /*key*/
        "\x7E\x5E\x94\x43\x1E\x11\xD7\x38\x28\xD7\x39\xCC\x6C\xED\x45\x73",
        /* direction bit */
        1,
        /* input bits */
        254,
        /* input */
        "\xB3\xD3\xC9\x17\x0A\x4E\x16\x32\xF6\x0F\x86\x10\x13\xD2\x2D\x84"
        "\xB7\x26\xB6\xA2\x78\xD8\x02\xD1\xEE\xAF\x13\x21\xBA\x59\x29\xDC",
        /* expected output */
        "\x92\xF2\xA4\x53",
    }
};


int test_snow3g(int dec)
{
    struct sxkeyref key;
    struct sx3gpp c;
    char *output;
    const char *expected, *input;
    int r = 0;
    struct tv *tv;
    size_t i = 0;

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
        r = sx_3gpp_create_snow3g(&c, &key, tv->direction, tv->bearer, tv->count);
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
    }

    return 0;
}

int test_snow3g_mac_generate(void)
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
        r = sx_3gpp_mac_create_snow3g(&c, &keyref, tv->direction, tv->fresh, tv->count);
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

    return 0;
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
    /* test snow3g encryption*/
    r |= test_snow3g(0);
    /* test snow3g decryption*/
    r |= test_snow3g(1);
    /* test snow3g authentication*/
    r |= test_snow3g_mac_generate();

   return r;
}
