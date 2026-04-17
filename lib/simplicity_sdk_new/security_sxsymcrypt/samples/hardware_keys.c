/** Sample for usage of hardware keys with block cipher.
 *
 * This sample uses the hardware key at index 0 to encrypt the reference
 * plaintext, then to decrypt the result and verify that the decrypted message
 * is the same as the reference plaintext.
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


static const char reference_plaintext[32] =
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a";
static const char iv[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";


int blkcipher_aescbc_hwkeys_encrypt(struct sxkeyref *k, char *iv, char *input,
    char *output, int inputsz)
{
    struct sxblkcipher c;
    int r;

    r = sx_blkcipher_create_aescbc_enc(&c, k, iv);
    if (r != SX_OK)
        return r;

    r = sx_blkcipher_crypt(&c, input, inputsz, output);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);

    return r;
}


int blkcipher_aescbc_hwkeys_decrypt(struct sxkeyref *k, char *iv ,char *input,
    char *output, int inputsz)
{
    struct sxblkcipher c;
    int r;

    r = sx_blkcipher_create_aescbc_dec(&c, k, iv);
    if (r != SX_OK)
        return r;

    r = sx_blkcipher_crypt(&c, input,inputsz, output);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);

    return r;
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct sxkeyref key;
    char *plaintext, *ciphertext, *result, *local_iv;
    int inputsz;
    int r = 0;

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return 1;
    }
    inputsz = sizeof(reference_plaintext);
    plaintext = dmamem;
    ciphertext = dmamem;
    result = dmamem;
    local_iv = dmamem + inputsz;

    memcpy(plaintext, reference_plaintext, inputsz);
    memcpy(local_iv, iv, sizeof(iv));

    key = sx_keyref_load_by_id(0);
    r = blkcipher_aescbc_hwkeys_encrypt(&key, local_iv, plaintext, ciphertext,
            inputsz);
    if (r != SX_OK)
        return r;

    r = blkcipher_aescbc_hwkeys_decrypt(&key, local_iv, ciphertext, result,
            inputsz);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(result, reference_plaintext, sizeof(reference_plaintext));

    return r;
}
