/** Sample for usage of KASUMI encryption, decryption, authentication, and key stream generation.
 *
 * The sample computes a Kasumi encryption, decryption, authentication and
 * key stream generation and validates it against the reference.
 *
 * By using different create functions, other modes can be used in the same
 * manner.
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
#define KEY_TV_ARRAY_SZ 6
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


struct key_tv {
    const struct sx3gppalg *alg;
    uint32_t framein;
    uint32_t outsz;
    int direction;
    int outbitsz;
    const char *key;
    const char *ciphertext;
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
        0xE28BCF7B,
        0x18,
        0, /* direction bit */
        510,
        "\xef\xa8\xb2\x22\x9e\x72\x0c\x2a\x7c\x36\xea\x55\xe9\x60\x56\x95",
        /* ciphertext */
        "\x3d\xea\xcc\x7c\x15\x82\x1c\xaa\x89\xee\xca\xde\x9b\x5b\xd3\x61"
        "\x4b\xd0\xc8\x41\x9d\x71\x03\x85\xdd\xbe\x58\x49\xef\x1b\xac\x5a"
        "\xe8\xb1\x4a\x5b\x0a\x67\x41\x52\x1e\xb4\xe0\x0b\xb9\xec\xf3\xe9"
        "\xf7\xcc\xb9\xca\xe7\x41\x52\xd7\xf4\xe2\xa0\x34\xb6\xea\x00\xec",
        /* plain text */
        "\x10\x11\x12\x31\xe0\x60\x25\x3a\x43\xfd\x3f\x57\xe3\x76\x07\xab"
        "\x28\x27\xb5\x99\xb6\xb1\xbb\xda\x37\xa8\xab\xcc\x5a\x8c\x55\x0d"
        "\x1b\xfb\x2f\x49\x46\x24\xfb\x50\x36\x7f\xa3\x6c\xe3\xbc\x68\xf1"
        "\x1c\xf9\x3b\x15\x10\x37\x6b\x02\x13\x0f\x81\x2a\x9f\xa1\x69\xd8",
    }, {
        0x398A59B4,
        0x15,
        1, /* direction bit */
        253,
        "\xD3\xC5\xD5\x92\x32\x7F\xB1\x1C\x40\x35\xC6\x68\x0A\xF8\xC6\xD1",
        /* ciphertext */
        "\xCA\x0A\x60\xB4\x29\x9E\x69\x54\xDB\xF7\x68\x6E\x46\xF4\x41\x90"
        "\xDC\x81\xB0\x74\x04\x48\x13\xB5\x0A\xB1\xFE\x46\x59\x7B\xA3\x38",
        /* plain text */
        "\x98\x1B\xA6\x82\x4C\x1B\xFB\x1A\xB4\x85\x47\x20\x29\xB7\x1D\x80"
        "\x8C\xE3\x3E\x2C\xC3\xC0\xB5\xFC\x1F\x3D\xE8\xA6\xDC\x66\xB1\xF0",
    }
};

struct key_tv key_tv_array[KEY_TV_ARRAY_SZ] = {
    {
        &sx3gppalg_kasumi_gsm_a53,
        0x0024F20F,
        0,
        0, /* direction bit */
        228, /* output bit size */
        "\x2B\xD6\x45\x9F\x82\xC5\xBC\x00\x2B\xD6\x45\x9F\x82\xC5\xBC\x00",
        /* output */ /* 228 bits */
        "\x88\x9E\xEA\xAF\x9E\xD1\xBA\x1A\xBB\xD8\x43\x62\x32\xE4\x57\x28"
        "\xD0\x1A\xA8\x91\x33\xDA\x73\xC1\x1E\xAB\x68\xB7\xD0\x00\x00\x00",
    }, {
        &sx3gppalg_kasumi_ecsd_a53,
        0x0024F20F,
        0,
        0, /* direction bit */
        696, /* output bit size */
        "\x2B\xD6\x45\x9F\x82\xC5\xBC\x00\x2B\xD6\x45\x9F\x82\xC5\xBC\x00",
        /* output */ /* 696 bits */
        "\xF7\x5E\x66\x3A\xCE\xA2\x1E\xC9\xD0\xBD\xE9\x8B\x6C\x33\xB8\x19"
        "\x29\x9E\x83\x0A\x1A\x2E\x2F\x91\x43\x26\xBE\xF5\x15\x08\x9B\x6D"
        "\xB0\xF2\x71\xAF\xB9\x60\x9F\x90\x52\x02\xCD\xCF\x51\x42\x6D\x17"
        "\x2D\xB4\x7B\xFE\xD3\xE6\xD8\x3D\x14\xF4\x87\x63\x66\xCC\xCD\x5B"
        "\xFA\xE8\x5B\x27\xC9\xB4\x9F\x2F\x77\x75\xB0\xB5\x04\x90\x5F\x27"
        "\xB5\xAE\x62\xB8\x26\x9E\xA9\x00",
    }, {
        &sx3gppalg_kasumi_gea3,
        0x8E9421A3,
        0x40,
        0, /* direction bit */
        (8*0x40), /* output 8*M bit size */
        "\x2B\xD6\x45\x9F\x82\xC5\xBC\x00\x2B\xD6\x45\x9F\x82\xC5\xBC\x00",
        /* output */ /* 8*M bits */
        "\x5F\x35\x97\x09\xDE\x95\x0D\x01\x05\xB1\x7B\x6C\x90\x19\x42\x80"
        "\xF8\x80\xB4\x8D\xCC\xDC\x2A\xFE\xED\x41\x5D\xBE\xF4\x35\x4E\xEB"
        "\xB2\x1D\x07\x3C\xCB\xBF\xB2\xD7\x06\xBD\x7A\xFF\xD3\x71\xFC\x96"
        "\xE3\x97\x0D\x14\x3D\xCB\x26\x24\x05\x48\x26\xDE\xB7\x7D\x41\x6E",
    }, {
        &sx3gppalg_kasumi_gsm_a54,
        0x0024F20F,
        0,
        /* direction bit */
        0,
        /* output bits */
        228,
        /*key*/
        "\x2B\xD6\x45\x9F\x82\xC5\xBC\x00\x2B\xD6\x45\x9F\x82\xC5\xBC\x00",
        /* expected output */
        "\x88\x9E\xEA\xAF\x9E\xD1\xBA\x1A\xBB\xD8\x43\x62\x32\xE4\x57\x28"
        "\xD0\x1A\xA8\x91\x33\xDA\x73\xC1\x1E\xAB\x68\xB7\xD0\x00\x00\x00",
    }, {
        &sx3gppalg_kasumi_ecsd_a54,
        0x0024F20F,
        0,
        /* direction bit */
        0,
        /* output bits */
        696,
        /*key*/
        "\x2B\xD6\x45\x9F\x82\xC5\xBC\x00\x2B\xD6\x45\x9F\x82\xC5\xBC\x00",
        /* expected output */
        "\xF7\x5E\x66\x3A\xCE\xA2\x1E\xC9\xD0\xBD\xE9\x8B\x6C\x33\xB8\x19"
        "\x29\x9E\x83\x0A\x1A\x2E\x2F\x91\x43\x26\xBE\xF5\x15\x08\x9B\x6D"
        "\xB0\xF2\x71\xAF\xB9\x60\x9F\x90\x52\x02\xCD\xCF\x51\x42\x6D\x17"
        "\x2D\xB4\x7B\xFE\xD3\xE6\xD8\x3D\x14\xF4\x87\x63\x66\xCC\xCD\x5B"
        "\xFA\xE8\x5B\x27\xC9\xB4\x9F\x2F\x77\x75\xB0\xB5\x04\x90\x5F\x27"
        "\xB5\xAE\x62\xB8\x26\x9E\xA9\x00",
    }, {
        &sx3gppalg_kasumi_gea4,
        0x8E9421A3,
        0x40,
        /* direction bit */
        0,
        /* output bits */
        8*0x40,
        /*key*/
        "\x2B\xD6\x45\x9F\x82\xC5\xBC\x00\x2B\xD6\x45\x9F\x82\xC5\xBC\x00",
        /* expected output */
        "\x5F\x35\x97\x09\xDE\x95\x0D\x01\x05\xB1\x7B\x6C\x90\x19\x42\x80"
        "\xF8\x80\xB4\x8D\xCC\xDC\x2A\xFE\xED\x41\x5D\xBE\xF4\x35\x4E\xEB"
        "\xB2\x1D\x07\x3C\xCB\xBF\xB2\xD7\x06\xBD\x7A\xFF\xD3\x71\xFC\x96"
        "\xE3\x97\x0D\x14\x3D\xCB\x26\x24\x05\x48\x26\xDE\xB7\x7D\x41\x6E",
    },
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
        "\x33\x32\x34\x62\x63\x39\x38\x61\x37\x34\x79",
        /* expected output */
        "\x00\x00\x00\x00\x46\xE0\x0D\x4B",
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
        "\x00\x00\x00\x00\x2B\xEE\xF3\xAC",
    }
};


int test_kasumi(int dec)
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
        r = sx_3gpp_create_kasumi(&c, &key, tv->direction, tv->bearer, tv->count);
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


int test_kasumi_key_stream_generation(void)
{
    struct sxkeyref key;
    struct sx3gpp c;
    char *output;
    const char *expected;
    int r = 0;
    struct key_tv *tv;
    size_t i = 0;

    for (i = 0; i < KEY_TV_ARRAY_SZ; i++) {
        tv = &key_tv_array[i];
        output = dmamem;
        expected = tv->ciphertext;
        key = sx_keyref_load_material(16, tv->key);
        r = sx_3gpp_create_kasumi_keystream(&c, &key, tv->alg, tv->framein, tv->outsz);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_generate(&c, output);
        if (r != SX_OK)
            return r;
        r = sx_3gpp_wait(&c);
        if (r != SX_OK)
            return r;
        r = sx_memdiff(output, expected, (tv->outbitsz + 7) / 8);
        if (r)
            return r;
    }

    return 0;
}


int test_kasumi_mac_generate(void)
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
        r = sx_3gpp_mac_create_kasumi(&c, &keyref, tv->direction, tv->fresh, tv->count);
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
        r = sx_memdiff(output_mac, tv->expect, 8);
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
    r |= test_kasumi(0);
    r |= test_kasumi(1);
    r |= test_kasumi_key_stream_generation();
    r |= test_kasumi_mac_generate();

    return r;
}
