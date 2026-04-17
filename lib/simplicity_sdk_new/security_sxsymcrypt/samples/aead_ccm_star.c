/** Sample for usage of AES CCM* encryption and decryption.
 *
 * @Copyright 2024 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 10);

/* The following values are extracted from IEEE Std 802.15.4-2011, part 15.4.
 * https://www.corsi.univr.it/documenti/OccorrenzaIns/matdid/matdid342697.pdf
 */
static const char keyref[16] =
        "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf";
static const char nonce[13] =
        "\xac\xde\x48\x00\x00\x00\x00\x01\x00\x00\x00\x05\x04";
static const char aad[26] =
        "\x69\xdc\x84\x21\x43\x02\x00\x00\x00\x00\x48\xde\xac\x01\x00\x00"
        "\x00\x00\x48\xde\xac\x04\x05\x00\x00\x00";
static const char reference_plaintext[4] =
        "\x61\x62\x63\x64";
static const char reference_ciphertext[4] =
        "\xd4\x3e\x02\x2b";


int test_aes_ccm_star_enc(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char ciphertext[16];
    int r = 0;

    key = sx_keyref_load_material(16, keyref);
    r = sx_aead_create_aesccm_enc(&c, &key, nonce, sizeof(nonce), 0,
                                  sizeof(aad), sizeof(reference_plaintext));
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, reference_plaintext, sizeof(reference_plaintext), ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, NULL);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(ciphertext, reference_ciphertext, sizeof(reference_ciphertext));

    return (!r ? SX_OK : r);
}


int test_aes_ccm_star_dec(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char plaintext[16];
    int r = 0;

    key = sx_keyref_load_material(16, keyref);
    r = sx_aead_create_aesccm_dec(&c, &key, nonce, sizeof(nonce), 0,
                                  sizeof(aad), sizeof(reference_ciphertext));
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, reference_ciphertext, sizeof(reference_ciphertext), plaintext);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, NULL);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(plaintext, reference_plaintext, sizeof(reference_plaintext));

    return (!r ? SX_OK : r);
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int r;

    r = test_aes_ccm_star_enc();
    if (r != SX_OK)
        return r;

    r = test_aes_ccm_star_dec();
    if (r != SX_OK)
        return r;

    return 0;
}

