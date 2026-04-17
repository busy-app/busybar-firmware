/** Sample for usage of AEAD encryption and decryption in chunks.
 *
 * The sample computes a AES CCM, GCM, ChaCha20Poly1305, encryption and
 * decryption and validates the results against the references.
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/chachapoly.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define MAX_TOTAL_SZ 4*128
static char *dmamem;

#define SX_CHACHAPOLY_CTX_BLK_SZ 64

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

static const char iv[12] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b";

static const char aad[8] =
        "\x00\x01\x02\x03\x04\x05\x06\x07";

static const char nonce_ccm[13] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c";

static const char nonce_chachapoly[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";

static const char keyref[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";


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


static const char reference_chachapoly_ciphertext[96] =
        "\x0a\xf3\x49\x77\x76\xa5\x26\xa7"
        "\x24\x3e\x11\x49\x6c\x23\x33\xb7"
        "\xee\x13\x9f\x9c\xf9\x0e\x17\x66"
        "\xab\x48\xfb\xf9\x0a\xd3\x6a\xd4"
        "\xff\x7e\xf3\x47\x0a\x4d\x3e\xe9"
        "\x80\x9e\x8c\x27\x78\xcd\xdf\x92"
        "\x2f\x0a\x61\x70\x02\x1e\x73\x09"
        "\xfb\x50\x59\xd8\x59\xe7\x37\x9d"
        "\xa4\xeb\xf8\x05\xd5\x82\x5c\x42"
        "\xf5\x8b\xfd\x91\xe5\xd6\x49\x4b"
        "\xaf\xe6\x22\xbc\xa9\x56\x61\x91"
        "\x16\x3f\xc6\x85\x0f\x84\xf2\x7e";

static const char reference_chachapoly_tag[16] =
        "\xd9\x41\xba\x3c\xc9\x55\x2e\x9b"
        "\xa2\xd5\xa7\x51\x8d\x80\xcc\xe7";


int test_aesgcm_enc(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char *plaintext, *ciphertext, *iv_local, *key_local, *aad_local, *tagout;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);
    chunksz = inputsz / 3;

    plaintext = dmamem;
    ciphertext = plaintext + inputsz;
    tagout = ciphertext + inputsz;
    iv_local = tagout + 16;
    key_local = iv_local + sizeof(iv);
    aad_local = key_local + sizeof(keyref);

    memcpy(plaintext, reference_plaintext, inputsz);
    memcpy(iv_local, iv, sizeof(iv));
    memcpy(key_local, keyref, sizeof(keyref));
    memcpy(aad_local, aad, sizeof(aad));

    key = sx_keyref_load_material(16, key_local);

    /* First Chunk of message */
    r = sx_aead_create_aesgcm_enc(&c, &key, iv_local);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad_local, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext, chunksz, ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext + chunksz, chunksz, ciphertext + chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext + 2 * chunksz, chunksz,
            ciphertext + 2 * chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tagout);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(ciphertext, reference_gcm_ciphertext, inputsz);
    r |= sx_memdiff(tagout, reference_gcm_tag, sizeof(reference_gcm_tag));

    return r;
}


int test_aesgcm_dec(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char *plaintext, *ciphertext, *iv_local, *key_local, *aad_local;
    char *tagin_local;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);
    chunksz = inputsz / 3;

    ciphertext = dmamem;
    plaintext = ciphertext + inputsz;
    tagin_local = plaintext + inputsz;
    iv_local = tagin_local + 16;
    key_local = iv_local + sizeof(iv);
    aad_local = key_local + sizeof(keyref);

    memcpy(ciphertext, reference_gcm_ciphertext, inputsz);
    memset(plaintext, 0xAB, inputsz);
    memcpy(iv_local, iv, sizeof(iv));
    memcpy(key_local, keyref, sizeof(keyref));
    memcpy(aad_local, aad, sizeof(aad));
    memcpy(tagin_local, reference_gcm_tag, sizeof(reference_gcm_tag));

    key = sx_keyref_load_material(16, key_local);

    /* First Chunk of message */
    r = sx_aead_create_aesgcm_dec(&c, &key, iv_local);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad_local, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext, chunksz, plaintext);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext + chunksz, chunksz, plaintext + chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext + 2 * chunksz, chunksz,
            plaintext + 2 * chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, tagin_local);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(plaintext, reference_plaintext, inputsz);

    return r;
}


int test_aesccm_enc(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char *plaintext, *ciphertext, *nonce_local, *key_local, *aad_local;
    char *tagout;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);
    chunksz = inputsz / 3;

    plaintext = dmamem;
    ciphertext = plaintext + inputsz;
    tagout = ciphertext + inputsz;
    nonce_local = tagout + sizeof(reference_ccm_tag);
    key_local = nonce_local + sizeof(nonce_ccm);
    aad_local = key_local + sizeof(keyref);

    memcpy(plaintext, reference_plaintext, inputsz);
    memcpy(nonce_local, nonce_ccm, sizeof(nonce_ccm));
    memcpy(key_local, keyref, sizeof(keyref));
    memcpy(aad_local, aad, sizeof(aad));

    key = sx_keyref_load_material(16, key_local);

    /* First Chunk of message */
    r = sx_aead_create_aesccm_enc(&c, &key, nonce_local, sizeof(nonce_ccm), 16,
            sizeof(aad), inputsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad_local, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext, chunksz, ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext + chunksz, chunksz, ciphertext + chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext + 2 * chunksz, chunksz,
            ciphertext + 2 * chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tagout);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(ciphertext, reference_ccm_ciphertext, inputsz);
    r += sx_memdiff(tagout, reference_ccm_tag, sizeof(reference_ccm_tag));

    return r;
}


int test_aesccm_dec(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char *plaintext, *ciphertext, *nonce_local, *key_local, *aad_local;
    char *tagin_local;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);
    chunksz = inputsz / 3;

    ciphertext = dmamem;
    plaintext = ciphertext + inputsz;
    tagin_local = plaintext + inputsz;
    nonce_local = tagin_local + sizeof(reference_ccm_tag);
    key_local = nonce_local + sizeof(nonce_ccm);
    aad_local = key_local + sizeof(keyref);

    memcpy(ciphertext, reference_ccm_ciphertext, inputsz);
    memset(plaintext, 0xAB, inputsz);
    memcpy(nonce_local, nonce_ccm, sizeof(nonce_ccm));
    memcpy(key_local, keyref, sizeof(keyref));
    memcpy(aad_local, aad, sizeof(aad));
    memcpy(tagin_local, reference_ccm_tag, sizeof(reference_ccm_tag));

    key = sx_keyref_load_material(16, key_local);

    /* First Chunk of message */
    r = sx_aead_create_aesccm_dec(&c, &key, nonce_local, sizeof(nonce_ccm), 16,
            sizeof(aad), inputsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad_local, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext, chunksz, plaintext);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext + chunksz, chunksz, plaintext + chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext + 2 * chunksz, chunksz,
            plaintext + 2 * chunksz);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, tagin_local);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(plaintext, reference_plaintext, inputsz);

    return r;
}


int test_chachapoly_enc(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char *plaintext, *ciphertext, *nonce_local, *key_local, *aad_local, *tagout;
    int inputsz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);

    plaintext = dmamem;
    ciphertext = plaintext + inputsz;
    tagout = ciphertext + inputsz;
    nonce_local = tagout + 16;
    key_local = nonce_local + sizeof(nonce_chachapoly);
    aad_local = key_local + 2 * sizeof(keyref);

    memcpy(plaintext, reference_plaintext, inputsz);
    memcpy(nonce_local, nonce_chachapoly, sizeof(nonce_chachapoly));
    memcpy(key_local, keyref, sizeof(keyref));
    memcpy(key_local + sizeof(keyref), keyref, sizeof(keyref));
    memcpy(aad_local, aad, sizeof(aad));

    key = sx_keyref_load_material(32, key_local);

    /* First Chunk of message */
    r = sx_aead_create_chacha20poly1305_enc(&c, &key, nonce_local);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad_local, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext, SX_CHACHAPOLY_CTX_BLK_SZ, ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, plaintext + SX_CHACHAPOLY_CTX_BLK_SZ,
            sizeof(reference_plaintext) - SX_CHACHAPOLY_CTX_BLK_SZ,
            ciphertext + SX_CHACHAPOLY_CTX_BLK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tagout);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(ciphertext, reference_chachapoly_ciphertext, inputsz);
    r += sx_memdiff(tagout, reference_chachapoly_tag,
            sizeof(reference_chachapoly_tag));

    return r;
}


int test_chachapoly_dec(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char *plaintext, *ciphertext, *nonce_local, *key_local, *aad_local;
    char *tagin_local;
    int inputsz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);

    ciphertext = dmamem;
    plaintext = ciphertext + inputsz;
    tagin_local = plaintext + inputsz;
    nonce_local = tagin_local + sizeof(reference_chachapoly_tag);
    key_local = nonce_local + sizeof(nonce_chachapoly);
    aad_local = key_local + 2 * sizeof(keyref);

    memcpy(ciphertext, reference_chachapoly_ciphertext, inputsz);
    memset(plaintext, 0xAB, inputsz);
    memcpy(nonce_local, nonce_chachapoly, sizeof(nonce_chachapoly));
    memcpy(key_local, keyref, sizeof(keyref));
    memcpy(key_local + sizeof(keyref), keyref, sizeof(keyref));
    memcpy(aad_local, aad, sizeof(aad));
    memcpy(tagin_local, reference_chachapoly_tag, sizeof(reference_chachapoly_tag));

    key = sx_keyref_load_material(32, key_local);

    /* First Chunk of message */
    r = sx_aead_create_chacha20poly1305_dec(&c, &key, nonce_local);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad_local, sizeof(aad));
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext, SX_CHACHAPOLY_CTX_BLK_SZ, plaintext);
    if (r != SX_OK)
        return r;
    r = sx_aead_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_aead_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, ciphertext + SX_CHACHAPOLY_CTX_BLK_SZ,
            sizeof(reference_plaintext) - SX_CHACHAPOLY_CTX_BLK_SZ,
            plaintext + SX_CHACHAPOLY_CTX_BLK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, (const char *)tagin_local);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(plaintext, reference_plaintext, inputsz);
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int r;

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return 1;
    }

    /* Run AES GCM encryption using context saving */
    r = test_aesgcm_enc();
    if (r)
        return r;

    /* Run AES GCM decryption using context saving */
    r = test_aesgcm_dec();
    if (r)
        return r;

    r = test_aesccm_enc();
    if (r)
        return r;

    r = test_aesccm_dec();
    if (r)
        return r;

    r = test_chachapoly_enc();
    if (r)
        return r;

    r = test_chachapoly_dec();
    if (r)
        return r;

    return 0;
}

