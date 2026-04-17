/** Sample for usage of block cipher encryption and decryption in chunks.
 *
 * The sample computes a AES CTR encryption and decryption and
 * validates it against the reference.
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
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define MAX_TOTAL_SZ 4*128
static char *dmamem;


static const char reference_plaintext[48] =
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a";

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

/* This functions runs AES CTR encryption using context saving.
 * For test purpose, the reference plaintext will be splitted into 3 parts of
 * 16 bytes each. */
int test_aesctr_encryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char *plaintext, *ciphertext, *local_iv, *local_key;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);
    chunksz = inputsz / 3;

    plaintext = dmamem;
    ciphertext = plaintext + inputsz;
    local_iv = ciphertext + inputsz;
    local_key = local_iv + 16;

    memcpy(plaintext, reference_plaintext, inputsz);
    memcpy(local_iv, iv, sizeof(iv));
    memcpy(local_key, keyref, sizeof(keyref));

    key = sx_keyref_load_material(16, local_key);

    /* First Chunk of message */
    r = sx_blkcipher_create_aesctr_enc(&c, &key, local_iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, plaintext, chunksz, ciphertext);
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
    r = sx_blkcipher_crypt(&c, plaintext + chunksz, chunksz,
            ciphertext + chunksz);
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
    r = sx_blkcipher_crypt(&c, plaintext + 2 * chunksz, chunksz,
            ciphertext + 2 * chunksz);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(ciphertext, reference_ciphertext, inputsz);
}


/* This functions runs AES CTR decryption using context saving.
 * For test purpose, the reference ciphertext will be splitted into 3 parts of
 * 16 bytes each. */
int test_aesctr_decryption(void)
{
    struct sxkeyref key;
    struct sxblkcipher c;
    char *plaintext, *ciphertext, *local_iv, *local_key;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(reference_plaintext);
    chunksz = inputsz / 3;

    plaintext = dmamem;
    ciphertext = plaintext + inputsz;
    local_iv = ciphertext + inputsz;
    local_key = local_iv + 16;

    memcpy(ciphertext, reference_ciphertext, inputsz);
    memcpy(local_iv, iv, sizeof(iv));
    memcpy(local_key, keyref, sizeof(keyref));

    key = sx_keyref_load_material(16, local_key);

    /* First Chunk of message */
    r = sx_blkcipher_create_aesctr_dec(&c, &key, local_iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_crypt(&c, ciphertext, chunksz, plaintext);
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
    r = sx_blkcipher_crypt(&c, ciphertext + chunksz, chunksz,
            plaintext + chunksz);
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
    r = sx_blkcipher_crypt(&c, ciphertext + 2 * chunksz, chunksz,
            plaintext + 2 * chunksz);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
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

    /* Run AES CTR encryption using context saving */
    r = test_aesctr_encryption();
    if (r)
        return r;

    /* Run AES CTR decryption using context saving */
    r = test_aesctr_decryption();

    return r;
}

