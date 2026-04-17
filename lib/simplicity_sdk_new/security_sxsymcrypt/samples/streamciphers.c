/** Sample for usage of AES stream ciphers encryption and decryption in chunks.
 *
 * The sample computes AES CTR, OFB, CFB encryption and decryption and
 * validates it against the reference.
 *
 * @Copyright 2024 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 11);


#define CHUNK_SZ (15)
#define MSG_SZ (48)

static const char reference_plaintext[MSG_SZ] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};

static const char reference_ciphertext_ctr[MSG_SZ] = {
        0x61, 0x55, 0xb5, 0x57, 0x6f, 0x2e, 0x6f, 0xd3,
        0x18, 0xfe, 0xea, 0x49, 0xb5, 0xc0, 0xfd, 0x70,
        0x69, 0xa2, 0x52, 0x76, 0x48, 0x58, 0xed, 0x00,
        0x73, 0xe7, 0x83, 0x1e, 0x38, 0x37, 0x18, 0xf6,
        0x71, 0xec, 0x2a, 0x51, 0x3f, 0x5c, 0x3a, 0x6e,
        0x54, 0xff, 0xb6, 0x5c, 0xbf, 0xba, 0xfb, 0x6d};

static const char reference_ciphertext_ofb[MSG_SZ] = {
        0x61, 0x55, 0xb5, 0x57, 0x6f, 0x2e, 0x6f, 0xd3,
        0x18, 0xfe, 0xea, 0x49, 0xb5, 0xc0, 0xfd, 0x70,
        0xc5, 0x26, 0xa0, 0x47, 0x6f, 0x97, 0x31, 0xdd,
        0x02, 0x5d, 0xc0, 0xdd, 0x2a, 0xac, 0xa1, 0x49,
        0xeb, 0x72, 0x47, 0x98, 0xee, 0x1e, 0x1c, 0x2c,
        0x0a, 0x99, 0x3f, 0x79, 0x35, 0x75, 0x57, 0x3f};

static const char reference_ciphertext_cfb[MSG_SZ] = {
        0x61, 0x55, 0xb5, 0x57, 0x6f, 0x2e, 0x6f, 0xd3,
        0x18, 0xfe, 0xea, 0x49, 0xb5, 0xc0, 0xfd, 0x70,
        0x7b, 0xc9, 0xc9, 0xc4, 0x77, 0xe0, 0xb4, 0x6a,
        0x11, 0x6d, 0x84, 0xb8, 0x06, 0x6a, 0xb3, 0xe0,
        0x42, 0xac, 0x9f, 0x08, 0x28, 0x86, 0xc0, 0x12,
        0xad, 0xa5, 0x89, 0x7b, 0xc9, 0x5c, 0xfa, 0xd8};

static const char iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static const char keyref[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};


/* This functions runs AES CTR encryption using context saving.
 * For test purpose, the reference plaintext will be splitted into 3 parts of
 * CHUNK_SZ bytes for first two, and the remaining as last chunk. */
static int test_aesctr_encryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char ciphertext[MSG_SZ] = {0};
    int r = 0;

    key = sx_keyref_load_material(16, keyref);

    /* First Chunk of message */
    r = sx_blkcipher_create_aesctr_enc(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext, CHUNK_SZ, ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext + CHUNK_SZ, CHUNK_SZ,
            ciphertext + CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext + 2 * CHUNK_SZ,
                           MSG_SZ - 2 * CHUNK_SZ,
                           ciphertext + 2 * CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(ciphertext, reference_ciphertext_ctr, MSG_SZ);
}


/* This functions runs AES CTR decryption using context saving.
 * For test purpose, the reference ciphertext will be splitted into 3 parts of
 * CHUNK_SZ bytes for first two, and the remaining as last chunk. */
static int test_aesctr_decryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char plaintext[MSG_SZ] = {0};
    int r = 0;

    key = sx_keyref_load_material(16, keyref);

    /* First Chunk of message */
    r = sx_blkcipher_create_aesctr_dec(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_ctr, CHUNK_SZ, plaintext);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_ctr + CHUNK_SZ, CHUNK_SZ,
            plaintext + CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_ctr + 2 * CHUNK_SZ,
                           MSG_SZ - 2 * CHUNK_SZ,
                           plaintext + 2 * CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(plaintext, reference_plaintext, MSG_SZ);
}


/* This functions runs AES OFB encryption using context saving.
 * For test purpose, the reference ciphertext will be splitted into 3 parts of
 * CHUNK_SZ bytes for first two, and the remaining as last chunk. */
static int test_aesofb_encryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char ciphertext[MSG_SZ] = {0};
    int r = 0;

    key = sx_keyref_load_material(16, keyref);

    /* First Chunk of message */
    r = sx_blkcipher_create_aesofb_enc(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext, CHUNK_SZ, ciphertext);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext + CHUNK_SZ, CHUNK_SZ,
            ciphertext + CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext + 2 * CHUNK_SZ,
                           MSG_SZ - 2 * CHUNK_SZ,
                           ciphertext + 2 * CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(ciphertext, reference_ciphertext_ofb, MSG_SZ);
}


/* This functions runs AES OFB decryption using context saving.
 * For test purpose, the reference ciphertext will be splitted into 3 parts of
 * CHUNK_SZ bytes for first two, and the remaining as last chunk. */
static int test_aesofb_decryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char plaintext[MSG_SZ] = {0};
    int r = 0;

    key = sx_keyref_load_material(16, keyref);

    /* First Chunk of message */
    r = sx_blkcipher_create_aesofb_dec(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_ofb, CHUNK_SZ, plaintext);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_ofb + CHUNK_SZ, CHUNK_SZ,
            plaintext + CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_ofb + 2 * CHUNK_SZ,
                           MSG_SZ - 2 * CHUNK_SZ,
                           plaintext + 2 * CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(plaintext, reference_plaintext, MSG_SZ);
}


/* This functions runs AES CFB encryption using context saving.
 * For test purpose, the reference ciphertext will be splitted into 3 parts of
 * CHUNK_SZ bytes for first two, and the remaining as last chunk. */
static int test_aescfb_encryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char ciphertext[MSG_SZ] = {0};
    int r = 0;

    key = sx_keyref_load_material(16, keyref);

    /* First Chunk of message */
    r = sx_blkcipher_create_aescfb_enc(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext, CHUNK_SZ, ciphertext);
    if (r != SX_OK)
        return r;

    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext + CHUNK_SZ, CHUNK_SZ,
            ciphertext + CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_plaintext + 2 * CHUNK_SZ,
                           MSG_SZ - 2 * CHUNK_SZ,
                           ciphertext + 2 * CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(ciphertext, reference_ciphertext_cfb, MSG_SZ);
}


/* This functions runs AES CFB decryption using context saving.
 * For test purpose, the reference ciphertext will be splitted into 3 parts of
 * CHUNK_SZ bytes for first two, and the remaining as last chunk. */
static int test_aescfb_decryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char plaintext[MSG_SZ] = {0};
    int r = 0;

    key = sx_keyref_load_material(16, keyref);

    /* First Chunk of message */
    r = sx_blkcipher_create_aescfb_dec(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_cfb, CHUNK_SZ, plaintext);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_cfb + CHUNK_SZ, CHUNK_SZ,
            plaintext + CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_blkcipher_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, reference_ciphertext_cfb + 2 * CHUNK_SZ,
                           MSG_SZ - 2 * CHUNK_SZ,
                           plaintext + 2 * CHUNK_SZ);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(plaintext, reference_plaintext, MSG_SZ);
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int r;

    /* Run AES CTR encryption using context saving */
    r = test_aesctr_encryption();
    if (r)
        return r;

    /* Run AES CTR decryption using context saving */
    r = test_aesctr_decryption();
    if (r)
        return r;

    /* Run AES OFB encryption using context saving */
    r = test_aesofb_encryption();
    if (r)
        return r;

    /* Run AES OFB decryption using context saving */
    r = test_aesofb_decryption();
    if (r)
        return r;

    /* Run AES CFB encryption using context saving */
    r = test_aescfb_encryption();
    if (r)
        return r;

    /* Run AES CFB decryption using context saving */
    r = test_aescfb_decryption();

    return r;
}

