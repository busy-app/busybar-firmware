/** Run external patterns and output the results.
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
#include <sxsymcrypt/sha1.h>
#include <sxsymcrypt/sha2.h>
#include <sxsymcrypt/sha3.h>
#include <sxsymcrypt/sm3.h>
#include <sxsymcrypt/hmac.h>
#include <sxsymcrypt/sm4.h>
#include <sxsymcrypt/tdes.h>
#include <sxsymcrypt/chachapoly.h>
#include <sxsymcrypt/mac.h>
#include <sxsymcrypt/cmac.h>
#include <sxsymcrypt/aria.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include <stddef.h>
#include "env/io.h"

SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 2);

#ifndef CFG_MAX_JOBS
#define CFG_MAX_JOBS 1u
#endif

#ifdef CFG_WRITE_RAW_ON_STDOUT
#define write_raw writedata
#else
#define write_raw(data, sz)
#endif

#define JOB_DMAMEM_SZ 0x4000
#define MAX_TOTAL_SZ (CFG_MAX_JOBS * JOB_DMAMEM_SZ)
#define STEP_STATUS_EOF -1

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(*x))
static char *dmamem;
static int failures;
static int processed;

struct ptnparams {
    uint32_t jobtype;
    uint32_t opsz[7];
};

struct job;
struct job {
    char *dmem;
    union {
        struct sxblkcipher blkciph;
        struct sxaead aead;
        struct sxhash hash;
        struct sxmac mac;
    } op;
    int (*next)(struct job* j);
    int verification;
    size_t keysz;
    union {
        size_t ivsz;
        size_t noncesz;
    } initsz;
    size_t aadsz;
    size_t sz;
    union {
        size_t tagsz;
        size_t macsz;
    } auth;
};


void readoperands(struct job *j, struct ptnparams *ptn)
{
    size_t totalsz = 0;
    int i;
    size_t r;

    for (i = 0; i < 7; i++)
        totalsz += ptn->opsz[i];
    r = readdata(j->dmem, totalsz);
    assert(r == totalsz);

    j->verification = ptn->jobtype >> 28;
}


int compare_digest(struct job *j)
{
    int r = 0;
    int diff;
    const char *expected = j->dmem + j->sz;
    const char *digest = expected + j->auth.tagsz;

    r = sx_hash_wait(&j->op.hash);
    if (r != SX_OK)
        return r;
    j->next = NULL;
    write_raw(digest, j->auth.tagsz);
    diff = sx_memdiff(digest, expected, j->auth.tagsz);
    failures += !!diff;

    return diff;
}


int start_sha(const struct sxhashalg *alg,
    struct job *j, struct ptnparams *ptn)
{
    int r = 0;
    size_t sz = ptn->opsz[0];
    size_t digestsz = ptn->opsz[1];

    const char *msg = j->dmem;
    char *digest = j->dmem + sz + digestsz;
    assert(sz + 2 * digestsz <= JOB_DMAMEM_SZ);

    readoperands(j, ptn);

    r = sx_hash_create(&j->op.hash, alg, sizeof(j->op.hash));
    if (r != SX_OK)
        return r;
    r = sx_hash_feed(&j->op.hash, msg, sz);
    if (r != SX_OK)
        return r;
    r = sx_hash_digest(&j->op.hash, digest);
    if (r != SX_OK)
        return r;
    j->next = compare_digest;
    j->sz = sz;
    j->auth.tagsz = digestsz;
    return r;
}


int start_sha256(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha2_256, j, ptn);
}


int start_sha224(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha2_224, j, ptn);
}


int start_sha384(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha2_384, j, ptn);
}


int start_sha512(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha2_512, j, ptn);
}


int start_sha1(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha1, j, ptn);
}

int start_sha3_224(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha3_224, j, ptn);
}

int start_sha3_256(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha3_256, j, ptn);
}

int start_sha3_384(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha3_384, j, ptn);
}

int start_sha3_512(struct job *j, struct ptnparams *ptn)
{
   return start_sha(&sxhashalg_sha3_512, j, ptn);
}


int compare_mac(struct job *j)
{
    int r = 0;
    int diff;
    const char *expected = j->dmem + j->keysz + j->sz;
    const char *mac = expected + j->auth.tagsz;

    r = sx_mac_wait(&j->op.mac);
    if (r != SX_OK)
        return r;

    j->next = NULL;
    write_raw(mac, j->auth.tagsz);
    diff = sx_memdiff(mac, expected, j->auth.tagsz);
    failures += !!diff;

    return diff;
}

int start_hmac_sha(int(*sx_mac_create_hmac_mode)(struct sxmac *, struct sxkeyref *keyref),
        struct job *j, struct ptnparams *ptn)
{
    int r = 0;
    size_t ksz = ptn->opsz[0];
    size_t sz = ptn->opsz[1];
    size_t macsz = ptn->opsz[2];
    const char *hmackey = j->dmem;
    const char *msg = j->dmem + ksz;
    char *mac = j->dmem + ksz + sz + macsz;

    struct sxkeyref keyref = sx_keyref_load_material(ksz, hmackey);

    assert(ksz + sz + macsz * 2 <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    r = sx_mac_create_hmac_mode(&j->op.mac, &keyref);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&j->op.mac, msg, sz);
    if (r != SX_OK)
        return r;
    r = sx_mac_generate(&j->op.mac, mac);
    if (r != SX_OK)
        return r;
    j->next = compare_mac;
    j->keysz = ksz;
    j->sz = sz;
    j->auth.tagsz = macsz;

    return r;
}


int start_hmac_sha1(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha1, j, ptn);
}


int start_hmac_sha224(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha2_224, j, ptn);
}


int start_hmac_sha256(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha2_256, j, ptn);
}


int start_hmac_sha384(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha2_384, j, ptn);
}


int start_hmac_sha512(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha2_512, j, ptn);
}


int start_hmac_sha3_224(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha3_224, j, ptn);
}


int start_hmac_sha3_256(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha3_256, j, ptn);
}


int start_hmac_sha3_384(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha3_384, j, ptn);
}


int start_hmac_sha3_512(struct job *j, struct ptnparams *ptn)
{
   return start_hmac_sha(sx_mac_create_hmac_sha3_512, j, ptn);
}


int finish_aesgcm_decrypt(struct job *j)
{
    int r;
    const char *ref = j->dmem + j->keysz + SX_GCM_IV_SZ + j->aadsz + j->sz + j->auth.tagsz;
    const char *pt = j->dmem + j->keysz + SX_GCM_IV_SZ + j->aadsz + 2*j->sz + j->auth.tagsz;
    int diff;

    r = sx_aead_wait(&j->op.aead);
    switch (j->verification) {
    case 0:
        assert(r == SX_OK);
        diff = sx_memdiff(ref, pt, j->sz);
        failures += !!diff;
        REPORT_PROGRESS(" AES GCM decrypted matches refs", diff);
        break;
    case 1:
        assert(r == SX_ERR_INVALID_TAG);
        break;
    case 2:
        assert(r == SX_OK || r == SX_ERR_INVALID_TAG);
        break;
    }
    write_raw(pt, j->sz);
    j->next = NULL;

    return r;
}


static int start_aead_gcm_decr(struct job *j, struct ptnparams *ptn,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key, const char *iv))
{
    size_t keysz = ptn->opsz[0];
    size_t aadsz = ptn->opsz[2];
    size_t sz  = ptn->opsz[3];
    size_t tagsz  = ptn->opsz[4];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *aad = iv + SX_GCM_IV_SZ;
    const char *ct = aad + aadsz;
    const char *tag = ct + sz;
    char *pt = j->dmem + keysz + SX_GCM_IV_SZ + aadsz + 2*sz + tagsz;
    int r;
    int totalsz = keysz + SX_GCM_IV_SZ + aadsz + 2*sz + tagsz;
    struct sxkeyref k;

    j->keysz = keysz;
    j->aadsz = aadsz;
    j->sz = sz;
    j->auth.tagsz = tagsz;

    assert(totalsz + tagsz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    r = create_function(&j->op.aead, &k, iv);
    if (r != SX_OK)
        return r;
    r = sx_aead_truncate_tag(&j->op.aead, tagsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&j->op.aead, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&j->op.aead, ct, sz, pt);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&j->op.aead, tag);
    if (r != SX_OK)
        return r;

    j->next = finish_aesgcm_decrypt;

    return SX_OK;
}


int finish_aesgcm_encrypt(struct job *j)
{
    int r;
    char *ct = j->dmem + j->keysz + SX_GCM_IV_SZ + j->aadsz;
    char *tag = ct + 2 * j->sz + j->auth.tagsz;
    const char *ref = j->dmem + j->keysz + SX_GCM_IV_SZ + j->aadsz + j->sz;
    const char *reftag = ref + j->sz;
    int diff;

    r = sx_aead_wait(&j->op.aead);
    assert(r == SX_OK);
    write_raw(ct, j->sz);
    write_raw(tag, SX_GCM_TAG_SZ);
    diff = sx_memdiff(ref, ct, j->sz);
    diff |= sx_memdiff(reftag, tag, j->auth.tagsz);
    failures += !!diff;
    REPORT_PROGRESS(" AES GCM crypted & tag match refs", diff);
    j->next = NULL;

    return r;
}


static int start_aead_gcm_encr(struct job *j, struct ptnparams *ptn,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key, const char *iv))
{
    size_t keysz = ptn->opsz[0];
    size_t aadsz = ptn->opsz[2];
    size_t sz  = ptn->opsz[3];
    size_t tagsz  = ptn->opsz[5];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *aad = iv + SX_GCM_IV_SZ;
    const char *msg = aad + aadsz;
    char *ct = j->dmem + keysz + SX_GCM_IV_SZ + aadsz;
    char *tag = ct + sz + sz + tagsz;
    int r;
    int totalsz = keysz + SX_GCM_IV_SZ + aadsz + 2*sz + tagsz;
    struct sxkeyref k;

    j->keysz = keysz;
    j->aadsz = aadsz;
    j->sz = sz;
    j->auth.tagsz = tagsz;

    assert(totalsz + tagsz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    r = create_function(&j->op.aead, &k, iv);
    if (r != SX_OK)
        return r;
    r = sx_aead_truncate_tag(&j->op.aead, tagsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&j->op.aead, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&j->op.aead, msg, sz, ct);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&j->op.aead, tag);
    if (r != SX_OK)
        return r;

    j->next = finish_aesgcm_encrypt;

    return SX_OK;
}


int finish_blkcipher(struct job *j)
{
    int r = 0;

    r = sx_blkcipher_wait(&j->op.blkciph);
    write_raw(j->dmem, j->sz);
    j->next = NULL;

    return r;
}


int check_blkcipher_matches_expected(struct job *j)
{
    int diff;
    int r;
    const char *expected = j->dmem + j->keysz + 16 + j->sz;

    r = finish_blkcipher(j);
    assert(r == SX_OK);
    diff = sx_memdiff(j->dmem, expected, j->sz);
    failures += !!diff;
    j->next = NULL;

    return r | diff;
}


int check_blkcipher_ecb_matches_expected(struct job *j)
{
    int diff;
    int r;
    const char *expected = j->dmem + j->keysz + j->sz;

    r = finish_blkcipher(j);
    assert(r == SX_OK);
    diff = sx_memdiff(j->dmem, expected, j->sz);
    failures += !!diff;
    j->next = NULL;

    return r | diff;
}


int check_ccm_matches_expected_decrypt(struct job *j)
{
    int r;
    const char *ref = j->dmem + j->keysz + j->initsz.noncesz + j->aadsz + j->sz + j->auth.tagsz;
    const char *output = j->dmem;
    int diff;

    r = sx_aead_wait(&j->op.aead);

    switch (j->verification) {
    case 0:
        assert(r == SX_OK);
        diff = sx_memdiff(ref, output, j->sz);
        failures += !!diff;
        break;
    case 1:
        assert(r == SX_ERR_INVALID_TAG);
        break;
    }
    write_raw(ref, j->sz);
    j->next = NULL;

    return r;
}


int check_ccm_matches_expected_encrypt(struct job *j)
{
    int r;
    char *refct = j->dmem + j->keysz + j->initsz.noncesz + j->aadsz + j->sz;
    char *reftag = refct + j->sz;
    char *outputct = j->dmem;
    char *outputag = outputct + j->sz;
    int diff;

    r = sx_aead_wait(&j->op.aead);
    assert(r == SX_OK);
    write_raw(refct, j->sz);
    write_raw(refct, j->auth.tagsz);
    diff = sx_memdiff(refct, outputct, j->sz);
    diff |= sx_memdiff(reftag, outputag, j->auth.tagsz);
    failures += !!diff;
    j->next = NULL;

    return r;
}


int encdec_blkcipher(struct job *j, const char *indata, char *outdata)
{
    int r = 0;

    r = sx_blkcipher_crypt(&j->op.blkciph, indata, j->sz, outdata);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&j->op.blkciph);

    return r;
}


int start_aesxts(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key1 = j->dmem;
    const char *key2 = j->dmem + keysz / 2;
    const char *iv = key2 + keysz / 2;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k1, k2;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);

    k1 = sx_keyref_load_material(keysz / 2, key1);
    k2 = sx_keyref_load_material(keysz / 2, key2);
    if (direction)
        r = sx_blkcipher_create_aesxts_enc(&j->op.blkciph, &k1, &k2, iv);
    else
        r = sx_blkcipher_create_aesxts_dec(&j->op.blkciph, &k1, &k2, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aesxtsencr(struct job *j, struct ptnparams *ptn)
{
    return start_aesxts(j, 1, ptn);
}


int start_aesxtsdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aesxts(j, 0, ptn);
}


int start_aes_ecb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *indata = key + keysz;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_aesecb_enc(&j->op.blkciph, &k);
    else
        r = sx_blkcipher_create_aesecb_dec(&j->op.blkciph, &k);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_ecb_matches_expected;

    return r;
}


int start_aesecbencr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_ecb(j, 1, ptn);
}


int start_aesecbdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_ecb(j, 0, ptn);
}


int start_aes_cbc(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_aescbc_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_aescbc_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aes_cfb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_aescfb_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_aescfb_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aes_ofb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_aesofb_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_aesofb_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aes_ctr(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_aesctr_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_aesctr_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aescbcencr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_cbc(j, 1, ptn);
}

int start_aescbcdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_cbc(j, 0, ptn);
}


int start_aescfbencr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_cfb(j, 1, ptn);
}


int start_aescfbdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_cfb(j, 0, ptn);
}


int start_aesofbencr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_ofb(j, 1, ptn);
}


int start_aesofbdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_ofb(j, 0, ptn);
}


int start_aead_ccm_encr(struct job *j, struct ptnparams *ptn,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key,
                           const char *nonce, size_t noncesz, size_t tagsz,
                           size_t aadsz, size_t datasz))
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t noncesz = ptn->opsz[1];
    size_t aadsz = ptn->opsz[2];
    size_t sz = ptn->opsz[3];
    size_t tagsz = ptn->opsz[5];
    const char *key = j->dmem;
    const char *nonce = key + keysz;
    const char *aad = nonce + noncesz;
    const char *indata = aad + aadsz;
    char *outdata = j->dmem;
    char *tag = outdata + sz;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    j->initsz.noncesz = noncesz;
    j->auth.tagsz = tagsz;
    j->aadsz = aadsz;
    assert(keysz + noncesz + aadsz + 2 * sz + tagsz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);

    k = sx_keyref_load_material(keysz, key);
    r = create_function(&j->op.aead, &k, nonce, noncesz, j->auth.tagsz, aadsz, j->sz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&j->op.aead, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&j->op.aead, indata, j->sz, outdata);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&j->op.aead, tag);
    if (r != SX_OK)
        return r;

    j->next = check_ccm_matches_expected_encrypt;

    return r;
}


int start_aead_ccm_decr(struct job *j, struct ptnparams *ptn,
    int (*create_function)(struct sxaead *c, const struct sxkeyref *key,
                           const char *nonce, size_t noncesz, size_t tagsz,
                           size_t aadsz, size_t datasz))
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t noncesz = ptn->opsz[1];
    size_t aadsz = ptn->opsz[2];
    size_t sz = ptn->opsz[3];
    size_t tagsz = ptn->opsz[4];
    const char *key = j->dmem;
    const char *nonce = key + keysz;
    const char *aad = nonce + noncesz;
    const char *indata = aad + aadsz;
    const char *tag = indata + sz;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    j->initsz.noncesz = noncesz;
    j->auth.tagsz = tagsz;
    j->aadsz = aadsz;
    assert(keysz + noncesz + aadsz + 2 * sz + tagsz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);

    k = sx_keyref_load_material(keysz, key);
    r = create_function(&j->op.aead, &k, nonce, noncesz, tagsz, aadsz, j->sz);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&j->op.aead, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&j->op.aead, indata, j->sz, outdata);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&j->op.aead, tag);
    if (r != SX_OK)
        return r;

    j->next = check_ccm_matches_expected_decrypt;
    assert(r == SX_OK);

    return r;
}


int start_aesctrencr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_ctr(j, 1, ptn);
}


int start_aesctrdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aes_ctr(j, 0, ptn);
}


int check_chachapoly_enc_output(struct job *j)
{
    int r;
    const char *outputref = j->dmem + j->keysz + j->initsz.noncesz + j->aadsz + j->sz;
    const char *tagref = outputref + j->sz;
    const char *output = j->dmem;
    const char *tag = output + j->sz;
    int diff;

    r = sx_aead_wait(&j->op.aead);
    assert(r == SX_OK);
    diff = sx_memdiff(outputref, output, j->sz);
    diff += sx_memdiff(tagref, tag, j->auth.tagsz);
    failures += !!diff;
    write_raw(output, j->sz);
    write_raw(tag, j->auth.tagsz);
    j->next = NULL;

    return r;
}


int check_chachapoly_dec_output(struct job *j)
{
    int r;
    const char *outputref = j->dmem + j->keysz + j->initsz.noncesz + j->aadsz + j->sz +
            j->auth.tagsz;
    const char *output = j->dmem;
    int diff;

    r = sx_aead_wait(&j->op.aead);
    assert(r == SX_OK);
    diff = sx_memdiff(outputref, output, j->sz);
    failures += !!diff;
    write_raw(output, j->sz);
    j->next = NULL;

    return r;
}


int start_chachapoly(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t ksz = ptn->opsz[0];
    size_t noncesz = ptn->opsz[1];
    size_t aadsz = ptn->opsz[2];
    size_t datasz = ptn->opsz[3];
    size_t tagsz = (direction ? ptn->opsz[5] : ptn->opsz[4]);
    const char *key = j->dmem;
    const char *nonce = key + ksz;
    const char *aad = nonce + noncesz;
    const char *data = aad + aadsz;
    const char *tag = data + datasz;
    char *out = j->dmem;
    char *tagout = out + datasz;
    struct sxkeyref k;

    assert(tagsz == 16);
    assert(ksz + noncesz + aadsz + datasz + tagsz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(ksz, key);
    if (direction)
        r = sx_aead_create_chacha20poly1305_enc(&j->op.aead, &k, nonce);
    else
        r = sx_aead_create_chacha20poly1305_dec(&j->op.aead, &k, nonce);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&j->op.aead, aad, aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&j->op.aead, data, datasz, out);
    if (r != SX_OK)
        return r;
    if (direction)
        r = sx_aead_produce_tag(&j->op.aead, tagout);
    else
        r = sx_aead_verify_tag(&j->op.aead, tag);
    if (r != SX_OK)
        return r;
    j->next = (direction ?
            check_chachapoly_enc_output : check_chachapoly_dec_output);
    j->keysz = ksz;
    j->initsz.noncesz = noncesz;
    j->aadsz = aadsz;
    j->sz = datasz;
    j->auth.tagsz = tagsz;

    return r;
}

int start_chachapoly_enc(struct job *j, struct ptnparams *ptn)
{
    return start_chachapoly(j, 1, ptn);
}

int start_chachapoly_dec(struct job *j, struct ptnparams *ptn)
{
    return start_chachapoly(j, 0, ptn);
}


int start_sm4_ecb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *indata = key + keysz;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_sm4ecb_enc(&j->op.blkciph, &k);
    else
        r = sx_blkcipher_create_sm4ecb_dec(&j->op.blkciph, &k);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_ecb_matches_expected;

    return r;
}

int start_sm4_cbc(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_sm4cbc_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_sm4cbc_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}

int start_sm4_cfb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_sm4cfb_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_sm4cfb_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}

int start_sm4_ofb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_sm4ofb_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_sm4ofb_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}

int start_sm4_ctr(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_sm4ctr_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_sm4ctr_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}

int start_sm4ecbenc(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_ecb(j, 1, ptn);
}

int start_sm4ecbdec(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_ecb(j, 0, ptn);
}

int start_sm4cbcenc(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_cbc(j, 1, ptn);
}

int start_sm4cbcdec(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_cbc(j, 0, ptn);
}

int start_sm4ofbenc(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_ofb(j, 1, ptn);
}

int start_sm4ofbdec(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_ofb(j, 0, ptn);
}

int start_sm4cfbenc(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_cfb(j, 1, ptn);
}

int start_sm4cfbdec(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_cfb(j, 0, ptn);
}

int start_sm4ctrenc(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_ctr(j, 1, ptn);
}

int start_sm4ctrdec(struct job *j, struct ptnparams *ptn)
{
    return start_sm4_ctr(j, 0, ptn);
}


int start_tdes_ecb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[1];
    const char *key = j->dmem;
    const char *indata = key + keysz;
    char *outdata = j->dmem;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz == 24);
    assert(!(sz & 0x07));
    assert(keysz + 3 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    if (direction)
        r = sx_blkcipher_create_tdesecb_enc(&j->op.blkciph, key);
    else
        r = sx_blkcipher_create_tdesecb_dec(&j->op.blkciph, key);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_ecb_matches_expected;

    return r;
}


int start_tdesecbencr(struct job *j, struct ptnparams *ptn)
{
    return start_tdes_ecb(j, 1, ptn);
}


int start_tdesecbdecr(struct job *j, struct ptnparams *ptn)
{
    return start_tdes_ecb(j, 0, ptn);
}


int check_tdes_matches_expected(struct job *j)
{
    int diff;
    int r;
    const char *expected = j->dmem + j->keysz + j->initsz.ivsz + j->sz;

    r = finish_blkcipher(j);
    assert(r == SX_OK);
    diff = sx_memdiff(expected + j->sz, expected, j->sz);
    failures += !!diff;
    j->next = NULL;

    return r | diff;
}


int start_tdes_cbc(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t ivsz = ptn->opsz[1];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + ivsz;
    char *outdata = j->dmem + keysz + ivsz + sz + sz;

    j->keysz = keysz;
    j->sz = sz;
    j->initsz.ivsz = ivsz;
    assert(keysz == 24);
    assert(ivsz == 8);
    assert(!(sz & 0x07));
    assert(keysz + ivsz + 3 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    if (direction)
        r = sx_blkcipher_create_tdescbc_enc(&j->op.blkciph, key, iv);
    else
        r = sx_blkcipher_create_tdescbc_dec(&j->op.blkciph, key, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_tdes_matches_expected;

    return r;
}


int start_tdescbcencr(struct job *j, struct ptnparams *ptn)
{
    return start_tdes_cbc(j, 1, ptn);
}


int start_tdescbcdecr(struct job *j, struct ptnparams *ptn)
{
    return start_tdes_cbc(j, 0, ptn);
}


int check_aescmac_matches_expected(struct job *j)
{
    int diff;
    int r;
    const char *reference_mac = j->dmem + j->keysz + j->sz;
    const char *output_mac = j->dmem;

    j->next = NULL;
    r = sx_mac_wait(&j->op.mac);
    if (r != SX_OK)
        return r;
    diff = sx_memdiff(reference_mac, output_mac, j->auth.macsz);
    failures += !!diff;
    write_raw(j->dmem, j->sz);

    return diff;
}


int start_mac_cmacgen(struct job *j, struct ptnparams *ptn,
    int (*create_function)(struct sxmac *c, const struct sxkeyref *key))
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[1];
    size_t macsz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *indata = key + keysz;
    char *macout = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    j->auth.macsz = macsz;
    assert(keysz + sz + macsz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);

    k = sx_keyref_load_material(keysz, key);
    r = create_function(&j->op.mac, &k);
    if (r != SX_OK)
        return r;

    r = sx_mac_feed(&j->op.mac, indata, j->sz/2);
    if (r != SX_OK)
        return r;
    r = sx_mac_feed(&j->op.mac, indata+j->sz/2, j->sz-(j->sz/2));
    if (r != SX_OK)
        return r;
    r = sx_mac_generate(&j->op.mac, macout);
    if (r != SX_OK)
        return r;
    j->next = check_aescmac_matches_expected;

    return SX_OK;
}


int start_aescmacgen(struct job *j, struct ptnparams *ptn)
{
    return start_mac_cmacgen(j, ptn, sx_mac_create_aescmac);
}


int start_aesgcmencr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_gcm_encr(j, ptn, sx_aead_create_aesgcm_enc);
}


int start_aesgcmdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_gcm_decr(j, ptn, sx_aead_create_aesgcm_dec);
}


int start_sm4gcmencr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_gcm_encr(j, ptn, sx_aead_create_sm4gcm_enc);
}


int start_sm4gcmdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_gcm_decr(j, ptn, sx_aead_create_sm4gcm_dec);
}


int start_aesccmencr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_ccm_encr(j, ptn, sx_aead_create_aesccm_enc);
}


int start_aesccmdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_ccm_decr(j, ptn, sx_aead_create_aesccm_dec);
}


int start_sm4ccmencr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_ccm_encr(j, ptn, sx_aead_create_sm4ccm_enc);
}


int start_sm4ccmdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_ccm_decr(j, ptn, sx_aead_create_sm4ccm_dec);
}


int start_ariaccmencr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_ccm_encr(j, ptn, sx_aead_create_ariaccm_enc);
}


int start_ariaccmdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_ccm_decr(j, ptn, sx_aead_create_ariaccm_dec);
}


int start_aria_ecb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *indata = key + keysz;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_ariaecb_enc(&j->op.blkciph, &k);
    else
        r = sx_blkcipher_create_ariaecb_dec(&j->op.blkciph, &k);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_ecb_matches_expected;

    return r;
}


int start_ariaecbencr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_ecb(j, 1, ptn);
}


int start_ariaecbdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_ecb(j, 0, ptn);
}


int start_aria_cbc(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_ariacbc_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_ariacbc_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aria_cfb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_ariacfb_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_ariacfb_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aria_ofb(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_ariaofb_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_ariaofb_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_aria_ctr(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[2];
    const char *key = j->dmem;
    const char *iv = j->dmem + keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem;
    struct sxkeyref k;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 16 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_ariactr_enc(&j->op.blkciph, &k, iv);
    else
        r = sx_blkcipher_create_ariactr_dec(&j->op.blkciph, &k, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_ariagcmencr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_gcm_encr(j, ptn, sx_aead_create_ariagcm_enc);
}


int start_ariagcmdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aead_gcm_decr(j, ptn, sx_aead_create_ariagcm_dec);
}


int start_ariacbcencr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_cbc(j, 1, ptn);
}

int start_ariacbcdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_cbc(j, 0, ptn);
}


int start_ariacfbencr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_cfb(j, 1, ptn);
}


int start_ariacfbdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_cfb(j, 0, ptn);
}


int start_ariaofbencr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_ofb(j, 1, ptn);
}


int start_ariaofbdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_ofb(j, 0, ptn);
}


int start_ariactrencr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_ctr(j, 1, ptn);
}


int start_ariactrdecr(struct job *j, struct ptnparams *ptn)
{
    return start_aria_ctr(j, 0, ptn);
}


int start_ariacmacgen(struct job *j, struct ptnparams *ptn)
{
    return start_mac_cmacgen(j, ptn, sx_mac_create_ariacmac);
}


int start_chacha20(struct job *j, int direction, struct ptnparams *ptn)
{
    int r = 0;
    size_t keysz = ptn->opsz[0];
    size_t sz = ptn->opsz[3];
    const char *key = j->dmem;
    const char *counter = key + keysz;
    const char *iv = counter + 4;
    const char *indata = iv + 12;
    char *outdata = j->dmem;
    struct sxkeyref k;
    uint32_t counter_val;

    j->keysz = keysz;
    j->sz = sz;
    assert(keysz + 4 + 12 + 2 * sz <= JOB_DMAMEM_SZ);
    readoperands(j, ptn);

    counter_val = (uint8_t)counter[0] +
            ((uint8_t)counter[1] << 8) +
            ((uint8_t)counter[2] << 16) +
            ((uint8_t)counter[3] << 24);
    k = sx_keyref_load_material(keysz, key);
    if (direction)
        r = sx_blkcipher_create_chacha20_enc(&j->op.blkciph, &k, counter_val, iv);
    else
        r = sx_blkcipher_create_chacha20_dec(&j->op.blkciph, &k, counter_val, iv);
    if (r != SX_OK)
        return r;

    r = encdec_blkcipher(j, indata, outdata);
    assert(r == SX_OK);
    j->next = check_blkcipher_matches_expected;

    return r;
}


int start_chacha20encr(struct job *j, struct ptnparams *ptn)
{
    return start_chacha20(j, 1, ptn);
}


int start_chacha20decr(struct job *j, struct ptnparams *ptn)
{
    return start_chacha20(j, 0, ptn);
}


typedef int(*jobstartfunc)(struct job *j, struct ptnparams *ptn);


jobstartfunc jobdefs[] = {
    start_aesxtsencr,
    start_aesxtsdecr,
    start_aesgcmencr,
    start_aesgcmdecr,
    start_aesecbencr,
    start_aesecbdecr,
    start_aescbcencr,
    start_aescbcdecr,
    start_aescfbencr,
    start_aescfbdecr,
    start_aesofbencr,
    start_aesofbdecr,
    start_aesccmencr,
    start_aesccmdecr,
    start_aesctrencr,
    start_aesctrdecr,
    start_sha256,
    start_sha224,
    start_sha384,
    start_sha512,
    start_sha1,
    start_ariagcmencr,
    start_ariagcmdecr,
    start_ariaecbencr,
    start_ariaecbdecr,
    start_ariacbcencr,
    start_ariacbcdecr,
    start_ariacfbencr,
    start_ariacfbdecr,
    start_ariaofbencr,
    start_ariaofbdecr,
    start_ariaccmencr,
    start_ariaccmdecr,
    start_ariactrencr,
    start_ariactrdecr,
    start_hmac_sha1,
    start_hmac_sha224,
    start_hmac_sha256,
    start_hmac_sha384,
    start_hmac_sha512,
    start_sha3_224,
    start_sha3_256,
    start_sha3_384,
    start_sha3_512,
    start_chachapoly_enc,
    start_chachapoly_dec,
    start_sm4ecbenc,
    start_sm4ecbdec,
    start_sm4cbcenc,
    start_sm4cbcdec,
    start_sm4ofbenc,
    start_sm4ofbdec,
    start_sm4cfbenc,
    start_sm4cfbdec,
    start_sm4ctrenc,
    start_sm4ctrdec,
    start_tdesecbencr,
    start_tdesecbdecr,
    start_tdescbcencr,
    start_tdescbcdecr,
    start_aescmacgen,
    start_sm4gcmencr,
    start_sm4gcmdecr,
    start_sm4ccmencr,
    start_sm4ccmdecr,
    start_ariacmacgen,
    start_chacha20encr,
    start_chacha20decr,
    start_hmac_sha3_224,
    start_hmac_sha3_256,
    start_hmac_sha3_384,
    start_hmac_sha3_512,
};


int nextjob(struct job *j)
{
    size_t r;
    int ret_job;
    struct ptnparams ptn = {0};
    unsigned int jobtype;

    r = readdata(&ptn, sizeof(ptn));
    if (r != sizeof(ptn))
        return STEP_STATUS_EOF;
    jobtype = ptn.jobtype & 0xFFFF;
    assert(jobtype >= 8000);
    jobtype -= 8000;
    assert(jobtype < sizeof(jobdefs)/sizeof(*jobdefs));
    ret_job = jobdefs[jobtype](j, &ptn);
    if(ret_job != SX_OK)
        REPORT_PROGRESS(" ret_job", ret_job != SX_OK);
    assert(ret_job == SX_OK);

    return 0;
}


int runmany(unsigned int maxpending)
{
    struct job jobs[CFG_MAX_JOBS] = {{0}};
    unsigned int i;
    int r = 0;
    int pending = 0;
    int incoming = 1;

    if (maxpending > ARRAY_COUNT(jobs))
        maxpending = ARRAY_COUNT(jobs);

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return -1;
    }
    for (i = 0; i < ARRAY_COUNT(jobs); i++)
        jobs[i].dmem = dmamem + JOB_DMAMEM_SZ * i;

    /* start as many jobs as possible */
    for (i = 0; i < maxpending; i++) {
        nextjob(&jobs[i]);
        pending++;
    }
    /* wait for the end of operations, starting new ones when possible */
    while (pending) {
        pending = 0;
        for (i = 0; i < ARRAY_COUNT(jobs); i++) {
            if (!jobs[i].next) {
                if (!incoming)
                    continue;
                r = nextjob(&jobs[i]);
                if (r == STEP_STATUS_EOF) {
                    incoming = 0;
                    continue;
                }
                pending++;
            }
            pending++;
            r |= jobs[i].next(&jobs[i]);
            processed++;
        }
    }

    return r;
}


int main()
{
    int r;

    r = runmany(CFG_MAX_JOBS);
    REPORT_SUMMARY(processed, failures);

    if (r == STEP_STATUS_EOF)
        r = failures;

    return r;
}
