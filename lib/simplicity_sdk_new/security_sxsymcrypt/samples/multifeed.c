/** Sample for usage of operations with multifeed.
 *
 * The sample computes:
 * - an AES CMAC
 * - an AES CTR encryption and decryption
 * - an AES CCM encryption
 *
 * and validates against references.
 *
 * By using different create functions, other modes can be used in the same
 * manner.
 *
 * @Copyright 2024 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */
#include <string.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/cmac.h>
#include <sxsymcrypt/mac.h>
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include "env/io.h"

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 11);

#define MSG_SZ 48
#define MAC_SZ 16
#define KEY_SZ 16
#define IV_SZ 16
#define AAD_SZ 32
#define TAG_SZ 16
#define NONCE_SZ 12

static const char reference_plaintext[MSG_SZ] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};

static const char keyref[KEY_SZ] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static const char iv[IV_SZ] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static const char nonce[NONCE_SZ] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b};

static const char aad[AAD_SZ] = {
        0x18, 0xfe, 0xea, 0x49, 0xb5, 0xc0, 0xfd, 0x70,
        0x69, 0xa2, 0x52, 0x76, 0x48, 0x58, 0xed, 0x00,
        0x73, 0xe7, 0x83, 0x1e, 0x38, 0x37, 0x18, 0xf6,
        0x71, 0xec, 0x2a, 0x51, 0x3f, 0x5c, 0x3a, 0x6e};

static const char reference_sha3256_mac[MAC_SZ] = {
        0x6e, 0x00, 0x7c, 0xaa, 0xdc, 0xae, 0xd9, 0xb7,
        0x9c, 0x37, 0x2f, 0xc4, 0xc0, 0x26, 0x38, 0x1a};

static const char reference_ctr_resultbuffer[MSG_SZ] = {
        0x61, 0x55, 0xb5, 0x57, 0x6f, 0x2e, 0x6f, 0xd3,
        0x18, 0xfe, 0xea, 0x49, 0xb5, 0xc0, 0xfd, 0x70,
        0x69, 0xa2, 0x52, 0x76, 0x48, 0x58, 0xed, 0x00,
        0x73, 0xe7, 0x83, 0x1e, 0x38, 0x37, 0x18, 0xf6,
        0x71, 0xec, 0x2a, 0x51, 0x3f, 0x5c, 0x3a, 0x6e,
        0x54, 0xff, 0xb6, 0x5c, 0xbf, 0xba, 0xfb, 0x6d};

static const char reference_ccm_resultbuffer[MSG_SZ] = {
        0x58, 0xd4, 0x4d, 0x85, 0xf2, 0xc0, 0x5b, 0x27,
        0x98, 0x2e, 0xb7, 0xf1, 0x71, 0x79, 0x92, 0xaa,
        0x07, 0x97, 0xa7, 0xd0, 0xe5, 0x44, 0x28, 0xae,
        0xa7, 0x5a, 0x32, 0x8c, 0x83, 0xb3, 0xbc, 0x9b,
        0x99, 0x84, 0xf7, 0xff, 0x3a, 0xde, 0x91, 0x9b,
        0x86, 0x0d, 0x31, 0xcb, 0xc7, 0xa6, 0xfc, 0xf3};
static const char reference_ccm_tag[TAG_SZ] = {
        0xc4, 0x31, 0xa9, 0x92, 0x7b, 0xe7, 0xe3, 0x58,
        0x4d, 0x05, 0x3b, 0xb9, 0xa6, 0x28, 0xa1, 0x74};


int mac_computation(void)
{
    int r;
    struct sxmac c;
    struct sxkeyref key;
    struct sxdataref datain[2];
    char mac[MAC_SZ];

    datain[0].data = reference_plaintext;
    datain[0].sz = MSG_SZ - 12;
    datain[1].data = reference_plaintext + datain[0].sz;
    datain[1].sz = MSG_SZ - datain[0].sz;

    key = sx_keyref_load_material(KEY_SZ, keyref);
    r = sx_mac_create_aescmac(&c, &key);
    if (r != SX_OK)
        return r;
    r = sx_mac_multifeed(&c, datain, sizeof(datain) / sizeof(struct sxdataref));
    if (r != SX_OK)
        return r;
    r = sx_mac_generate(&c, mac);
    if (r != SX_OK)
        return r;
    r = sx_mac_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(mac, reference_sha3256_mac, MAC_SZ);
    return r;
}


int blkcipher_encryption(void)
{
    int r;
    struct sxkeyref key;
    struct sxblkcipher c;
    struct sxdataref datain[2];
    struct sxdataref dataout[3];
    char resultbuffer[MSG_SZ];

    /* Prepare IN data */
    datain[0].data = reference_plaintext;
    datain[0].sz = 15;
    datain[1].data = reference_plaintext + datain[0].sz;
    datain[1].sz = MSG_SZ - datain[0].sz;

    /* Prepare OUT data */
    dataout[0].data = resultbuffer;
    dataout[0].sz = 12;
    dataout[1].data = resultbuffer + dataout[0].sz;
    dataout[1].sz = 14;
    dataout[2].data = dataout[1].data + dataout[1].sz;
    dataout[2].sz = MSG_SZ - dataout[0].sz - dataout[1].sz;

    key = sx_keyref_load_material(KEY_SZ, keyref);
    r = sx_blkcipher_create_aesctr_enc(&c, &key, iv);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_multifeed_crypt(&c, datain, 2, dataout, 3);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    return sx_memdiff(resultbuffer, reference_ctr_resultbuffer, MSG_SZ);
}


int aead_encryption(void)
{
    int r;
    struct sxaead c;
    struct sxkeyref key;
    char resultbuffer[MSG_SZ];
    char tag[TAG_SZ];
    struct sxdataref datain[2];
    struct sxdataref dataout[3];
    struct sxdataref aadin[2];

    datain[0].data = reference_plaintext;
    datain[0].sz = MSG_SZ - 1;
    datain[1].data = reference_plaintext + datain[0].sz;
    datain[1].sz = MSG_SZ - datain[0].sz;

    dataout[0].data = resultbuffer;
    dataout[0].sz = 1;
    dataout[1].data = resultbuffer + dataout[0].sz;
    dataout[1].sz = 17;
    dataout[2].data = dataout[1].data + dataout[1].sz;
    dataout[2].sz = MSG_SZ - dataout[0].sz - dataout[1].sz;

    aadin[0].data = aad;
    aadin[0].sz = AAD_SZ - 14;
    aadin[1].data = aad + aadin[0].sz;
    aadin[1].sz = AAD_SZ - aadin[0].sz;

    key = sx_keyref_load_material(KEY_SZ, keyref);
    r = sx_aead_create_aesccm_enc(&c, &key, nonce, NONCE_SZ,
            TAG_SZ, AAD_SZ, MSG_SZ);
    if (r != SX_OK)
        return r;
    r = sx_aead_multifeed_aad(&c, aadin, 2);
    if (r != SX_OK)
        return r;
    r = sx_aead_multifeed_crypt(&c, datain, 2, dataout, 3);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    r = sx_memdiff(resultbuffer, reference_ccm_resultbuffer, MSG_SZ);
    r |= sx_memdiff(tag, reference_ccm_tag, TAG_SZ);
    return r;
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int r;

    r = mac_computation();
    if (r)
        return r;

    r = blkcipher_encryption();
    if (r)
        return r;

    r = aead_encryption();
    if (r)
        return r;

    return r;
}
