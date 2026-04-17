/** Sample for usage of AES CMAC in chunks.
 *
 * The sample computes a AES CMAC message authentication code and validates it
 * against the reference.
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/mac.h>
#include <sxsymcrypt/cmac.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define MAX_TOTAL_SZ 4*128
static char *dmamem;


static const char msg[48] =
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96"
        "\xe9\x3d\x7e\x11\x73\x93\x17\x2a";

static const char reference_mac[16] =
        "\x6e\x00\x7c\xaa\xdc\xae\xd9\xb7"
        "\x9c\x37\x2f\xc4\xc0\x26\x38\x1a";

static const char keyref[16] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";

/* This function runs AES CMAC generate using context saving. For test purpose,
 * the message will be split into 3 parts of 16 bytes each. */
int test_aescmac_generate(void)
{
    struct sxkeyref key;
    struct sxmac c;
    char *local_msg, *local_key, *output_mac;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(msg);
    chunksz = inputsz / 3;

    local_msg = dmamem;
    local_key = local_msg + inputsz;
    output_mac = local_key + sizeof(keyref);

    memcpy(local_msg, msg, inputsz);
    memcpy(local_key, keyref, sizeof(keyref));

    key = sx_keyref_load_material(16, local_key);

    /* First Chunk of message */
    r = sx_mac_create_aescmac(&c, &key);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, local_msg, chunksz);
    if (r != SX_OK)
        return r;
    r = sx_mac_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    /* Second Chunk of message */
    r = sx_mac_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, local_msg + chunksz, chunksz);
    if (r != SX_OK)
        return r;
    r = sx_mac_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_mac_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, local_msg + 2 * chunksz, chunksz);
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

int test_aescmac_multifeed_generate(void)
{
    struct sxkeyref key;
    struct sxmac c;
    char *local_msg, *local_key, *output_mac;
    int inputsz, chunksz;
    int r = 0;

    inputsz = sizeof(msg);
    chunksz = inputsz / 3;

    local_msg = dmamem;
    local_key = local_msg + inputsz;
    output_mac = local_key + sizeof(keyref);

    memcpy(local_msg, msg, inputsz);
    memcpy(local_key, keyref, sizeof(keyref));

    key = sx_keyref_load_material(16, local_key);

    /* First Chunk of message */
    r = sx_mac_create_aescmac(&c, &key);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, local_msg, chunksz / 2);
    if (r != SX_OK)
        return r;
     r = sx_mac_feed(&c, local_msg + (chunksz / 2) , chunksz - (chunksz / 2));
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, local_msg + chunksz, chunksz);
    if (r != SX_OK)
        return r;
    r = sx_mac_save_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    /* Last Chunk of message */
    r = sx_mac_resume_state(&c);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&c, local_msg + (2 * chunksz), chunksz);
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
    int r = 0;

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return 1;
    }
    r |= test_aescmac_generate();
    r |= test_aescmac_multifeed_generate();

    return r;
}

