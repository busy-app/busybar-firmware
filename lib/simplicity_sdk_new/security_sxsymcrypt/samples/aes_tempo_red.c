/** Sample for AES with temporal redundancy.
 *
 * The sample computes some AES operations and validates 
 * it against the reference. 
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <stdio.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/cmac.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "../src/blkcipherdefs.h"
#include "../src/aeaddefs.h"
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 9);


static const char reference_plaintext[96] =
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a";

static const char aad[8] =
        "\x00\x01\x02\x03\x04\x05\x06\x07";

static const char nonce_ccm[13] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c";

static const char reference_gcm_ciphertext[96] =
        "\xf8\xad\x19\x2c\x48\x5b\x68\xc2"
        "\xa2\xef\x1f\x9b\x45\x30\x67\x22"
        "\xd8\xe7\xa4\x04\x7d\xad\x62\x60"
        "\x0f\x1c\x8f\x3c\x67\xd7\xb5\x46"
        "\xee\x65\x65\x15\xda\xc0\x28\x75"
        "\xa9\xc9\x77\xf6\xb3\x1a\xc0\xb8"
        "\xa7\x5e\xaa\x12\x2a\xc7\xa9\xed"
        "\xc7\x73\x0c\xc3\xd9\xdf\x01\xce"
        "\xf1\xcf\x4b\xac\x71\x0c\x55\x88"
        "\xca\x0d\x0f\x4e\x0f\xa9\x1f\x71"
        "\x03\x6d\x3b\x89\x27\xdc\x02\x54"
        "\x1f\xc0\xb9\xc6\xcf\xda\x9d\x22";

static const char reference_gcm_tag[16] =
        "\xfe\xbc\x2b\x07\xaf\xdf\xb1\x55"
        "\x22\x28\x88\xa1\x51\x35\x79\xa3";

static const char reference_ccm_ciphertext[] =
        "\x7d\xf5\x0a\x6a\x7d\x49\x65\x14"
        "\xb3\xaa\x4d\xb6\xee\xa7\x09\xf2"
        "\x88\xc1\xba\xc0\x58\x6d\xee\x70"
        "\xc5\xc2\x80\x4b\xff\xc1\xdc\x31"
        "\xbe\x94\xf1\xaa\xfb\xec\x38\xee"
        "\x90\xb0\xdb\x3d\x96\x2a\xd4\x9b"
        "\xb5\xd4\x64\x03\xf0\xaf\xd7\xe3"
        "\xc9\xb1\xcc\xdc\x35\x94\xd8\xb6"
        "\xa6\x7e\x11\xd1\xc1\x5f\xcc\xe7"
        "\x89\x40\x3f\xfb\xc6\x2f\xb7\xf0"
        "\xf8\xbe\x48\x46\xdd\x8d\xe6\xd3"
        "\x5f\xa8\xa4\xfa\x10\xc1\x62\x13";

static const char reference_ccm_tag[16] =
        "\xa7\x66\x73\x62\x81\x09\x77\x6a"
        "\xb5\x6a\xff\x6a\xbf\xa5\xf5\x30";

static const char reference_ciphertext[48] =
        "\x61\x55\xb5\x57\x6f\x2e\x6f\xd3"
        "\x18\xfe\xea\x49\xb5\xc0\xfd\x70"
        "\x69\xa2\x52\x76\x48\x58\xed\x00"
        "\x73\xe7\x83\x1e\x38\x37\x18\xf6"
        "\x71\xec\x2a\x51\x3f\x5c\x3a\x6e"
        "\x54\xff\xb6\x5c\xbf\xba\xfb\x6d";

static const char iv[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";

static const char keyref[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";

static const char reference_mac[16] =
        "\x6e\x00\x7c\xaa\xdc\xae\xd9\xb7"
        "\x9c\x37\x2f\xc4\xc0\x26\x38\x1a";


int test_aesgcm_enc(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char ciphertext[96];
    char tagout[16];
    int r = 0;

    struct sxaesparams params = {.config = 0};
    SX_AES_SET_TEMPO_REDUNDANCY(params.config);

    key = sx_keyref_load_material(16, keyref);
    r = sx_aead_create_aes_generic(&c, &key, iv, 0, 0,
            0, 0, SX_AES_ENCRYPT, AEAD_MODEID_GCM, &params);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, reference_plaintext, sizeof(reference_plaintext), ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tagout);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(ciphertext, reference_gcm_ciphertext, sizeof(reference_plaintext));
    r |= sx_memdiff(tagout, reference_gcm_tag, sizeof(reference_gcm_tag));

    return r;
}


int test_aesccm_dec(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char plaintext[96];
    int r = 0;
    struct sxaesparams params = {.config = 0};
    SX_AES_SET_TEMPO_REDUNDANCY(params.config);

    key = sx_keyref_load_material(16, keyref);
    r = sx_aead_create_aes_generic(&c, &key, nonce_ccm, sizeof(nonce_ccm), 16,
        sizeof(aad), sizeof(reference_plaintext), SX_AES_DECRYPT, AEAD_MODEID_CCM, &params);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, reference_ccm_ciphertext, sizeof(reference_plaintext), plaintext);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, reference_ccm_tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(plaintext, reference_plaintext, sizeof(reference_plaintext));

    return r;
}


int test_aesctr_enc(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char ciphertext[48];
    int r = 0;
    struct sxaesparams params = {.config = 0};
    SX_AES_SET_TEMPO_REDUNDANCY(params.config);

    key = sx_keyref_load_material(16, keyref);
    r = sx_blkcipher_create_aes_generic(&c, &key, NULL, iv, BLKCIPHER_MODEID_CTR,
            SX_AES_ENCRYPT, &params);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext, 48, ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(ciphertext, reference_ciphertext, 48);
}


int test_aescmac_generate(void)
{
    struct sxkeyref key;
    struct sxmac c;
    char output_mac[16];
    int r = 0;
    struct sxaesparams params = {.config = 0};
    SX_AES_SET_TEMPO_REDUNDANCY(params.config);

    key = sx_keyref_load_material(16, keyref);
    r = sx_mac_create_aes_generic(&c, &key, &params);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, reference_plaintext, 48);
    if (r != SX_OK)
        return r;
    r = sx_mac_generate(&c, output_mac);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(reference_mac, output_mac, 16);
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int r;

    /* Run AES operations with temporal redundancy*/
    r = test_aesgcm_enc();
    if (r)
        return r;

    r = test_aesccm_dec();
    if (r)
        return r;

    r = test_aesctr_enc();
    if (r)
        return r;
    
    r = test_aescmac_generate();
    if (r)
        return r;

    return 0;
}
