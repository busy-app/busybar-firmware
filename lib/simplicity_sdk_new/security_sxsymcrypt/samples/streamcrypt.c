/** Read a stream of input data from stdin and write the encrypted or
 * decrypted result on stdout.
  *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/hash.h>
#include <sxsymcrypt/sm4.h>
#include <sxsymcrypt/chachapoly.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/version.h>
#include <stddef.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);


#define MAX_TOTAL_SZ 4096
static char *dmamem;


static int blk_cipher_crypt(struct sxblkcipher *c, const char *in, size_t insz,
    char *out)
{
    int r;
    r = sx_blkcipher_crypt(c, in, insz, out);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(c);
    if (r != SX_OK)
        return r;

    writedata(out, insz);

    return SX_OK;
}


int aead_gcm_encrypt(size_t keysz, size_t aadsz,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key, const char *iv))
{
    const char *key, *iv, *aad, *data;
    char *tag;
    size_t sz;
    struct sxaead c;
    int r;
    struct sxkeyref k;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz > keysz + SX_GCM_IV_SZ + aadsz);
    key = dmamem;
    iv = key + keysz;
    aad = iv + SX_GCM_IV_SZ;
    data = aad + aadsz;
    sz -= keysz + SX_GCM_IV_SZ + aadsz;
    tag = dmamem + sz;
    k = sx_keyref_load_material(keysz, key);
    r = create_function(&c, &k, iv);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, data, sz, dmamem);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(dmamem, sz);
    writedata(tag, SX_GCM_TAG_SZ);

    return r;
}


int aead_gcm_decrypt(size_t keysz, size_t aadsz,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key, const char *iv))
{
    const char *key, *iv, *aad, *data, *tag;
    size_t sz;
    struct sxaead c = {0};
    int r;
    struct sxkeyref k;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz > keysz + SX_GCM_IV_SZ + aadsz + SX_GCM_TAG_SZ);
    key = dmamem;
    iv = key + keysz;
    aad = iv + SX_GCM_IV_SZ;
    data = aad + aadsz;
    sz -= keysz + SX_GCM_IV_SZ + aadsz + SX_GCM_TAG_SZ;
    tag = data + sz;
    k = sx_keyref_load_material(keysz, key);
    r = create_function(&c, &k, iv);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, data, sz, dmamem);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(dmamem, sz);

    return r;
}


int blkcipher_xts(size_t keysz,
    int (*blkcipher_create)(struct sxblkcipher*, const struct sxkeyref *k1, const struct sxkeyref *k2, const char*))
{
    size_t sz;
    struct sxblkcipher c;
    int r;
    const char *key1 = dmamem;
    const char *key2 = dmamem + keysz;
    const char *iv = key2 + keysz;
    const char *indata = iv + 16;
    char *outdata = dmamem;
    struct sxkeyref k1, k2;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz >= 2 * keysz + 16);
    k1 = sx_keyref_load_material(keysz, key1);
    k2 = sx_keyref_load_material(keysz, key2);
    r = blkcipher_create(&c, &k1, &k2, iv);
    sz -= 2 * keysz + 16;
    if (r != SX_OK)
        return r;

    return blk_cipher_crypt(&c, indata, sz, outdata);
}

/** Run blockcipher with ECB mode
 *
 * Reads input from stdin: key, data.
 *
 * @arg keysz size of key
 * @arg ecb_blkcipher_create ECB cipher create function
 *
 * @returns SX_OK or another status code on error
 */
static int blkcipher_ecb(size_t keysz,
    int (*ecb_blkcipher_create)(struct sxblkcipher*, const struct sxkeyref *k))
{
    size_t sz;
    struct sxblkcipher c;
    int r;
    const char *key = dmamem;
    const char *indata = key + keysz;
    char *outdata = dmamem;
    struct sxkeyref k;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz >= keysz);
    k = sx_keyref_load_material(keysz, key);
    r = ecb_blkcipher_create(&c, &k);
    sz -= keysz;
    if (r != SX_OK)
        return r;

    return blk_cipher_crypt(&c, indata, sz, outdata);
}

/** Run blockcipher with CBC, CTR, CFB or OFB mode
 *
 * Reads input from stdin: key, iv, data.
 *
 * @arg keysz size of key
 * @arg blkcipher_create cipher create function
 *
 * @returns SX_OK or another status code on error
 */
static int blkcipher_cbc_ctr_cfb_ofb(size_t keysz,
    int (*blkcipher_create)(struct sxblkcipher*, const struct sxkeyref *k, const char*))
{
    size_t sz;
    struct sxblkcipher c;
    int r;
    const char *key = dmamem;
    const char *iv = key + keysz;
    const char *indata = iv + 16;
    char *outdata = dmamem;
    struct sxkeyref k;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz >= keysz + 16);
    k = sx_keyref_load_material(keysz, key);
    r = blkcipher_create(&c, &k, iv);
    sz -= keysz + 16;
    if (r != SX_OK)
        return r;

    return blk_cipher_crypt(&c, indata, sz, outdata);
}


int aead_ccm_encrypt(size_t keysz, size_t aadsz, size_t tagsz, size_t noncesz,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key,
        const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz, size_t datasz))
{
    const char *key, *nonce, *aad, *data;
    char *output, *tag;
    size_t sz;
    struct sxaead c;
    int r;
    struct sxkeyref k;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz >= keysz + noncesz + aadsz);

    key = dmamem;
    nonce = key + keysz;
    aad = nonce + noncesz;
    data = aad + aadsz;
    sz -= keysz + noncesz + aadsz;
    output = dmamem;
    tag = dmamem + sz;
    k = sx_keyref_load_material(keysz, key);
    r = create_function(&c, &k, nonce, noncesz, tagsz, aadsz, sz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, data, sz, output);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(output, sz);
    writedata(tag, tagsz);

    return r;
}


int aead_ccm_decrypt(size_t keysz, size_t aadsz, size_t tagsz, size_t noncesz,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key,
        const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz, size_t datasz))
{
    const char *key, *nonce, *aad, *data, *tag;
    char *output;
    size_t sz;
    struct sxaead c;
    int r;
    struct sxkeyref k;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz >= keysz + noncesz + aadsz+ tagsz);

    key = dmamem;
    nonce = key + keysz;
    aad = nonce + noncesz;
    data = aad + aadsz;
    sz -= keysz + noncesz + aadsz + tagsz;
    tag = data + sz;
    output = dmamem;
    k = sx_keyref_load_material(keysz, key);
    r = create_function(&c, &k, nonce, noncesz, tagsz, aadsz, sz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, data, sz, output);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(output, sz);

    return r;
}


#define SX_CHACHAPOLY_TAG_SZ 16u
#define SX_CHACHAPOLY_NONCE_SZ 12u
#define SX_CHACHAPOLY_KEY_SZ 32u
int chacha20poly1305_enc(int aadsz)
{
    struct sxaead c;
    int r;
    size_t fullsz, datasz;
    const char *key, *nonce, *aad, *data;
    char *msg = dmamem;
    char *output, *tag;
    struct sxkeyref k;

    fullsz = readdata(msg, MAX_TOTAL_SZ);
    key = msg;
    nonce = key + SX_CHACHAPOLY_KEY_SZ;
    aad = nonce + SX_CHACHAPOLY_NONCE_SZ;
    data = aad + aadsz;
    datasz = fullsz - SX_CHACHAPOLY_KEY_SZ - SX_CHACHAPOLY_NONCE_SZ - aadsz;
    output = dmamem;
    tag = dmamem + datasz;
    k = sx_keyref_load_material(SX_CHACHAPOLY_KEY_SZ, key);
    r = sx_aead_create_chacha20poly1305_enc(&c, &k, nonce);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, data, datasz, output);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&c, tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(output, datasz);
    writedata(tag, SX_CHACHAPOLY_TAG_SZ);

    return r;
}


int chacha20poly1305_dec(int aadsz)
{
    struct sxaead c;
    int r;
    size_t fullsz, datasz;
    const char *key, *nonce, *aad, *data, *tag;
    char *msg = dmamem;
    char *output;
    struct sxkeyref k;

    fullsz = readdata(msg, MAX_TOTAL_SZ);
    key = msg;
    nonce = key + SX_CHACHAPOLY_KEY_SZ;
    aad = nonce + SX_CHACHAPOLY_NONCE_SZ;
    data = aad + aadsz;
    datasz = fullsz - SX_CHACHAPOLY_KEY_SZ - SX_CHACHAPOLY_NONCE_SZ - aadsz -
            SX_CHACHAPOLY_TAG_SZ;
    tag = data + datasz;
    output = dmamem;
    k = sx_keyref_load_material(SX_CHACHAPOLY_KEY_SZ, key);
    r = sx_aead_create_chacha20poly1305_dec(&c, &k, nonce);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&c, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&c, data, datasz, output);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&c, tag);
    if (r != SX_OK)
        return r;
    r = sx_aead_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(output, datasz);

    return r;
}


int sm4_xts_context_saving(const int dir)
{
    struct sxblkcipher c;
    int r;
    size_t i;
    size_t sz;
    const char *key1 = dmamem;
    const char *key2 = dmamem + 16;
    const char *iv = key2 + 16;
    const char *indata = iv + 16;
    char *outdata = dmamem + 16 + 16 + 16;
    struct sxkeyref k1, k2;

    sz = readdata(dmamem, MAX_TOTAL_SZ);
    assert(sz >= 2 * 16 + 16);

    k1 = sx_keyref_load_material(16, key1);
    k2 = sx_keyref_load_material(16, key2);
    if(dir)
        r = sx_blkcipher_create_sm4xts_enc(&c, &k1, &k2, iv);
    else
        r = sx_blkcipher_create_sm4xts_dec(&c, &k1, &k2, iv);
    if (r != SX_OK)
        return r;
    sz -= 2 * 16 + 16;
    /* For testing context saving, minimum indata size must be 17, otherwise
     * we cannot split indata in 2 buffers, first buffer a multiple of 16 bytes
     */
    assert(sz > 16);

    for (i = 0; i < (sz - 16); i += 16) {
        r = sx_blkcipher_crypt(&c, indata + i, 16, outdata + i);
        if (r != SX_OK)
            return r;
        r = sx_blkcipher_save_state(&c);
        if (r != SX_OK)
            return r;
        r = sx_blkcipher_wait(&c);
        if (r != SX_OK)
            return r;
        r = sx_blkcipher_resume_state(&c);
        if (r != SX_OK)
            return r;
    }

    r = sx_blkcipher_crypt(&c, indata + i, sz - i, outdata + i);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&c);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_wait(&c);
    if (r != SX_OK)
        return r;

    writedata(outdata, sz);

    return SX_OK;
}


void usage(void)
{
    displaymsg("Usage: sample <op> <args>\n");
    displaymsg("  op = 0 for AES 128b GCM encryption; args = aadsz\n");
    displaymsg("  op = 1 for AES 128b GCM decryption; args = aadsz\n");
    displaymsg("  op = 2 for AES 256b GCM encryption; args = aadsz\n");
    displaymsg("  op = 3 for AES 256b GCM decryption; args = aadsz\n");
    displaymsg("  op = 4 for AES 128b XTS encryption\n");
    displaymsg("  op = 5 for AES 128b XTS decryption\n");
    displaymsg("  op = 6 for AES 256b XTS encryption\n");
    displaymsg("  op = 7 for AES 256b XTS decryption\n");
    displaymsg("  op =14 for AES 128b CBC encryption\n");
    displaymsg("  op =15 for AES 128b CBC decryption\n");
    displaymsg("  op =16 for AES 256b CBC encryption\n");
    displaymsg("  op =17 for AES 256b CBC decryption\n");
    displaymsg("  op =18 for AES 128b ECB encryption\n");
    displaymsg("  op =19 for AES 128b ECB decryption\n");
    displaymsg("  op =20 for AES 256b ECB encryption\n");
    displaymsg("  op =21 for AES 256b ECB decryption\n");
    displaymsg("  op =22 for AES 128b CTR encryption\n");
    displaymsg("  op =23 for AES 128b CTR decryption\n");
    displaymsg("  op =24 for AES 256b CTR encryption\n");
    displaymsg("  op =25 for AES 256b CTR decryption\n");
    displaymsg("  op =26 for AES 128b CFB encryption\n");
    displaymsg("  op =27 for AES 128b CFB decryption\n");
    displaymsg("  op =28 for AES 256b CFB encryption\n");
    displaymsg("  op =29 for AES 256b CFB decryption\n");
    displaymsg("  op =30 for AES 128b OFB encryption\n");
    displaymsg("  op =31 for AES 128b OFB decryption\n");
    displaymsg("  op =32 for AES 256b OFB encryption\n");
    displaymsg("  op =33 for AES 256b OFB decryption\n");
    displaymsg("  op =40 for AES 128b CCM encryption; args = aadsz tagsz noncesz\n");
    displaymsg("  op =41 for AES 128b CCM decryption; args = aadsz tagsz noncesz\n");
    displaymsg("  op =42 for AES 256b CCM encryption; args = aadsz tagsz noncesz\n");
    displaymsg("  op =43 for AES 256b CCM decryption; args = aadsz tagsz noncesz\n");
    displaymsg("  op =45 for ChaCha20-Poly1305 encryption; args = aadsz\n");
    displaymsg("  op =46 for ChaCha20-Poly1305 decryption; args = aadsz\n");
    //SM4
    displaymsg("  op =47 for SM4 CBC encryption\n");
    displaymsg("  op =48 for SM4 CBC decryption\n");
    displaymsg("  op =49 for SM4 ECB encryption\n");
    displaymsg("  op =50 for SM4 ECB decryption\n");
    displaymsg("  op =51 for SM4 CTR encryption\n");
    displaymsg("  op =52 for SM4 CTR decryption\n");
    displaymsg("  op =53 for SM4 CFB encryption\n");
    displaymsg("  op =54 for SM4 CFB decryption\n");
    displaymsg("  op =55 for SM4 OFB encryption\n");
    displaymsg("  op =56 for SM4 OFB decryption\n");
    displaymsg("  op =58 for SM4 GCM encryption\n");
    displaymsg("  op =59 for SM4 GCM decryption\n");
    displaymsg("  op =60 for SM4 CCM encryption\n");
    displaymsg("  op =61 for SM4 CCM decryption\n");
    displaymsg("  op =62 for SM4 XTS encryption\n");
    displaymsg("  op =63 for SM4 XTS decryption\n");
    displaymsg("  op =64 for SM4 XTS encryption with context saving\n");
    displaymsg("  op =65 for SM4 XTS decryption with context saving\n");
}


int main(int argc, char **argv)
{
    int op;
    int r = 1;
    int aadsz = 0;
    int noncesz = 0;
    int tagsz = 0;

    if (argc < 2) {
        usage();
        return -1;
    }

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return -1;
    }
    op = sx_atoi(argv[1]);
    if (argc > 2)
        aadsz = sx_atoi(argv[2]);

    if (argc > 4) {
        tagsz = sx_atoi(argv[3]);
        noncesz = sx_atoi(argv[4]);
    }
    switch (op) {
    case 0:
        r = aead_gcm_encrypt(16, aadsz, sx_aead_create_aesgcm_enc);
        break;
    case 1:
        r = aead_gcm_decrypt(16, aadsz, sx_aead_create_aesgcm_dec);
        break;
    case 2:
        r = aead_gcm_encrypt(32, aadsz, sx_aead_create_aesgcm_enc);
        break;
    case 3:
        r = aead_gcm_decrypt(32, aadsz, sx_aead_create_aesgcm_dec);
        break;
    case 4:
        r = blkcipher_xts(16, sx_blkcipher_create_aesxts_enc);
        break;
    case 5:
        r = blkcipher_xts(16, sx_blkcipher_create_aesxts_dec);
        break;
    case 6:
        r = blkcipher_xts(32, sx_blkcipher_create_aesxts_enc);
        break;
    case 7:
        r = blkcipher_xts(32, sx_blkcipher_create_aesxts_dec);
        break;
//---AES
//-----CBC
    case 14:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aescbc_enc);
        break;
    case 15: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aescbc_dec);
        break;
    case 16:
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aescbc_enc);
        break;
    case 17: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aescbc_dec);
        break;
//-----ECB
    case 18:
        r = blkcipher_ecb(16, sx_blkcipher_create_aesecb_enc);
        break;
    case 19: //decrypt
        r = blkcipher_ecb(16, sx_blkcipher_create_aesecb_dec);
        break;
    case 20:
        r = blkcipher_ecb(32, sx_blkcipher_create_aesecb_enc);
        break;
    case 21: //decrypt
        r = blkcipher_ecb(32, sx_blkcipher_create_aesecb_dec);
        break;
//-----CTR
    case 22:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aesctr_enc);
        break;
    case 23: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aesctr_dec);
        break;
    case 24:
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aesctr_enc);
        break;
    case 25: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aesctr_dec);
        break;
//-----CFB
    case 26:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aescfb_enc);
        break;
    case 27: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aescfb_dec);
        break;
    case 28:
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aescfb_enc);
        break;
    case 29: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aescfb_dec);
        break;
//-----OFB
    case 30:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aesofb_enc);
        break;
    case 31: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_aesofb_dec);
        break;
    case 32:
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aesofb_enc);
        break;
    case 33: //decrypt
        r = blkcipher_cbc_ctr_cfb_ofb(32, sx_blkcipher_create_aesofb_dec);
        break;
    case 40:
        r = aead_ccm_encrypt(16, aadsz, tagsz, noncesz, sx_aead_create_aesccm_enc);
        break;
    case 41:
        r = aead_ccm_decrypt(16, aadsz, tagsz, noncesz, sx_aead_create_aesccm_dec);
        break;
    case 42:
        r = aead_ccm_encrypt(32, aadsz, tagsz, noncesz, sx_aead_create_aesccm_enc);
        break;
    case 43:
        r = aead_ccm_decrypt(32, aadsz, tagsz, noncesz, sx_aead_create_aesccm_dec);
        break;
    case 45:
        r = chacha20poly1305_enc(aadsz);
        break;
    case 46:
        r = chacha20poly1305_dec(aadsz);
        break;
//---SM4
//-----CBC
    case 47:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4cbc_enc);
        break;
    case 48:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4cbc_dec);
        break;
//-----ECB
    case 49:
        r = blkcipher_ecb(16, sx_blkcipher_create_sm4ecb_enc);
        break;
    case 50:
        r = blkcipher_ecb(16, sx_blkcipher_create_sm4ecb_dec);
        break;
//-----CTR
    case 51:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4ctr_enc);
        break;
    case 52:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4ctr_dec);
        break;
//-----CFB
    case 53:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4cfb_enc);
        break;
    case 54:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4cfb_dec);
        break;
//-----OFB
    case 55:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4ofb_enc);
        break;
    case 56:
        r = blkcipher_cbc_ctr_cfb_ofb(16, sx_blkcipher_create_sm4ofb_dec);
        break;
    case 58:
        r = aead_gcm_encrypt(16, aadsz, sx_aead_create_sm4gcm_enc);
        break;
    case 59:
        r = aead_gcm_decrypt(16, aadsz, sx_aead_create_sm4gcm_dec);
        break;
    case 60:
        r = aead_ccm_encrypt(16, aadsz, tagsz, noncesz, sx_aead_create_sm4ccm_enc);
        break;
    case 61:
        r = aead_ccm_decrypt(16, aadsz, tagsz, noncesz, sx_aead_create_sm4ccm_dec);
        break;
    case 62:
        r = blkcipher_xts(16, sx_blkcipher_create_sm4xts_enc);
        break;
    case 63:
        r = blkcipher_xts(16, sx_blkcipher_create_sm4xts_dec);
        break;
    case 64:
        r = sm4_xts_context_saving(1);
        break;
    case 65:
        r = sm4_xts_context_saving(0);
        break;
    }

    if (r != SX_OK)
        return r;

    return 0;
}
