/** Sample for usage of AEAD encryption and decryption in chunks.
 * In this particular example, AAD is provided through multiple context
 * savings/loadings, having any size (not mandatory the size of a chunk,
 * 16 bytes).
 *
 * The sample computes a AES GCM / CCM, encryption and
 * decryption and validates the results against the references.
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

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 11);

#define MSG_SZ   96
#define IV_SZ    12
#define NONCE_SZ 13
#define AAD_SZ   40
#define KEY_SZ   16
#define TAG_SZ   16

static const char reference_plaintext[MSG_SZ] =
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

static const char iv[IV_SZ] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b";

static const char aad[AAD_SZ] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
        "\x20\x21\x22\x23\x24\x25\x26\x27";

static const char nonce[NONCE_SZ] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c";

static const char keyref[KEY_SZ] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";

/* Reference outputs */
static const char reference_gcm_ciphertext[MSG_SZ] =
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
static const char reference_gcm_tag[TAG_SZ] =
        "\x58\x40\x72\x2d\x74\xe1\xe4\x6a"
        "\xd0\x3f\x44\x65\xff\xbe\x4c\x37";

static const char reference_ccm_ciphertext[MSG_SZ] =
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
static const char reference_ccm_tag[TAG_SZ] =
        "\x69\xf3\x96\x17\xb4\x2b\x8c\xfb"
        "\xc4\x73\xcf\x6c\x08\x8c\xe6\xdf";


static int test_aesgcm_enc()
{
    int r;
    struct sxaead c;
    struct sxkeyref key;
    char ciphertext[MSG_SZ];
    char tagout[TAG_SZ];

    key = sx_keyref_load_material(KEY_SZ, keyref);
    r = sx_aead_create_aesgcm_enc(&c, &key, iv);
    if (r != SX_OK)
        return r;

    /* Feed AAD */
    r = sx_aead_feed_aad(&c, aad, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 6, 8);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 14, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 20, 20);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    /* Feed data */
    r = sx_aead_crypt(&c, reference_plaintext, MSG_SZ, ciphertext);
    if (r != SX_OK)
        return r;

    /* Call HW and wait for result */
    r = sx_aead_produce_tag(&c, tagout);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(ciphertext, reference_gcm_ciphertext, MSG_SZ);
    r |= sx_memdiff(tagout, reference_gcm_tag, TAG_SZ);
    return r;
}


static int test_aesgcm_dec()
{
    int r;
    struct sxaead c;
    struct sxkeyref key;
    char plaintext[MSG_SZ];

    key = sx_keyref_load_material(KEY_SZ, keyref);
    r = sx_aead_create_aesgcm_dec(&c, &key, iv);
    if (r != SX_OK)
        return r;

    /* Feed AAD */
    r = sx_aead_feed_aad(&c, aad, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 6, 8);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 14, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 20, 20);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    /* Feed data */
    r = sx_aead_crypt(&c, reference_gcm_ciphertext, MSG_SZ, plaintext);
    if (r != SX_OK)
        return r;

    /* Call HW and wait for result */
    r = sx_aead_verify_tag(&c, reference_gcm_tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(plaintext, reference_plaintext, MSG_SZ);
}


static int test_aesccm_enc()
{
    int r;
    struct sxaead c;
    struct sxkeyref key;
    char ciphertext[MSG_SZ];
    char tagout[TAG_SZ];

    key = sx_keyref_load_material(KEY_SZ, keyref);
    r = sx_aead_create_aesccm_enc(&c, &key, nonce, NONCE_SZ, TAG_SZ, AAD_SZ, MSG_SZ);
    if (r != SX_OK)
        return r;

    /* Feed AAD */
    r = sx_aead_feed_aad(&c, aad, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 6, 8);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 14, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 20, 20);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    /* Feed data */
    r = sx_aead_crypt(&c, reference_plaintext, MSG_SZ, ciphertext);
    if (r != SX_OK)
        return r;

    /* Call HW and wait for result */
    r = sx_aead_produce_tag(&c, tagout);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(ciphertext, reference_ccm_ciphertext, MSG_SZ);
    r |= sx_memdiff(tagout, reference_ccm_tag, TAG_SZ);
    return r;
}


static int test_aesccm_dec()
{
    int r;
    struct sxaead c;
    struct sxkeyref key;
    char plaintext[MSG_SZ];

    key = sx_keyref_load_material(KEY_SZ, keyref);
   r = sx_aead_create_aesccm_dec(&c, &key, nonce, NONCE_SZ, TAG_SZ, AAD_SZ, MSG_SZ);
    if (r != SX_OK)
        return r;

    /* Feed AAD */
    r = sx_aead_feed_aad(&c, aad, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 6, 8);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 14, 6);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    r = sx_aead_feed_aad(&c, aad + 20, 20);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;

    /* Feed data */
    r = sx_aead_crypt(&c, reference_ccm_ciphertext, MSG_SZ, plaintext);
    if (r != SX_OK)
        return r;

    /* Call HW and wait for result */
    r = sx_aead_verify_tag(&c, reference_ccm_tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(plaintext, reference_plaintext, MSG_SZ);
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int r;

    /* Run AES GCM encryption using context saving with arbitrary AAD size */
    r = test_aesgcm_enc();
    if (r)
        return r;

    /* Run AES GCM decryption using context saving with arbitrary AAD size */
    r = test_aesgcm_dec();
    if (r)
        return r;

    /* Run AES CCM encryption using context saving with arbitrary AAD size */
    r = test_aesccm_enc();
    if (r)
        return r;

    /* Run AES CCM decryption using context saving with arbitrary AAD size */
    r = test_aesccm_dec();
    if (r)
        return r;

    return 0;
}
