/** Sample for usage of AES GMAC with AAD fed using context saving.
 *
 * @Copyright 2023 Secure-IC S.A.S.
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


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define MAX_TOTAL_SZ 4*128

static const char iv[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";

static const char keyref[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";

static const char aad[24] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
        "\x00\x01\x02\x03\x04\x05\x06\x07";

static const char reference_gmac_tag[16] =
        "\xc4\xf4\x17\xc6\xd2\x49\x5f\xf4"
        "\x99\xef\x8f\x79\x3c\xc5\x4d\xc0";


int test_aesgmac(void)
{
    struct sxkeyref key;
    struct sxaead c;
    char tagout[16];
    int r = 0;

    key = sx_keyref_load_material(16, keyref);

    /* First Chunk of message */
    r = sx_aead_create_aesgcm_enc(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, 16);
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
    r = sx_aead_feed_aad(&c, aad + 16, 8);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tagout);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(tagout, reference_gmac_tag, sizeof(reference_gmac_tag));
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return test_aesgmac();
}

