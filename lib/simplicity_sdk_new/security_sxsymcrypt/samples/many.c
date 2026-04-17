/** Run encryption-decryption cycles with internal patterns.
 *
 *  It first starts multiple encryptions asynchronously. When the
 * encryptions complete, a corresponding decryption is started
 *  asynchronously. When the decryptions finish, the decrypted text is
 * compared with the original plaintext validating the
 * encryption-decryption cycle.
 *
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include <sxsymcrypt/aead.h>
#include <sxsymcrypt/blkcipher.h>
#include <sxsymcrypt/aes.h>
#include <sxsymcrypt/keyref.h>
#include <sxsymcrypt/statuscodes.h>
#include <sxsymcrypt/dmamem.h>
#include <sxsymcrypt/version.h>
#include <sxsymcrypt/memdiff.h>
#include <stddef.h>
#include "env/io.h"


SXSYMCRYPT_API_ASSERT_COMPATIBLE(4, 0);

#ifndef CFG_MAX_JOBS
#define CFG_MAX_JOBS 2u
#endif

#ifndef CFG_REPORT_LVL
#define CFG_REPORT_LVL 1
#endif

#if CFG_REPORT_LVL > 0
#define REPORT_VALIDATIONS(cnt) display("validated cycles :", cnt)
#else
#define REPORT_VALIDATIONS(cnt) do {} while (0)
#endif

void write_raw(const char *data, size_t sz)
{
    (void)data;
    (void)sz;
#ifdef CFG_WRITE_RAW_ON_STDOUT
    writedata(data, sz);
#endif
}

#define JOB_DMAMEM_SZ 256
#define MAX_TOTAL_SZ (CFG_MAX_JOBS * JOB_DMAMEM_SZ)


#define ARRAY_COUNT(x) (sizeof(x)/sizeof(*x))
static char *dmamem;
static int validations;

struct job;

struct action {
    size_t keysz;
    size_t aadsz;
    size_t sz;
    const char *in;
    int (*start)(struct job* j);
};

struct job {
    char *dmem;
    union {
        struct sxblkcipher blkciph;
        struct sxaead aead;
    } op;
    struct action *x;
    int (*next)(struct job* j);
};


void loadjob(struct job *j, size_t sz)
{
    char *dst = j->dmem;
    const char *src = j->x->in;

    while (sz--) {
        *dst = *src;
        dst++;
        src++;
    }
}

int finish_aesgcm_encrypt(struct job *j)
{
    int r;
    const char *t = j->dmem + j->x->keysz + SX_GCM_IV_SZ + j->x->aadsz;
    const char *tag = t + j->x->sz;

    r = sx_aead_wait(&j->op.aead);
    write_raw(t, j->x->sz);
    write_raw(tag, SX_GCM_TAG_SZ);
    j->next = NULL;

    return r;
}

int check_aesgcm_matches_original(struct job *j)
{
    int r;
    int diff;
    const char *t = j->dmem + j->x->keysz + SX_GCM_IV_SZ + j->x->aadsz;

    r = sx_aead_wait(&j->op.aead);
    write_raw(t, j->x->sz);
    if (r != SX_OK)
        return r;

    diff = sx_memdiff(t,
        j->x->in + j->x->keysz + SX_GCM_IV_SZ + j->x->aadsz,
        j->x->sz);
    REPORT_PROGRESS("GCM D(E(k)) == k ?", diff);

    validations++;
    j->next = NULL;

    return diff;
}


int decrypt_aesgcm_after_encrypt(struct job *j)
{
    int r;
    const char *aeskey = j->dmem;
    const char *iv = j->dmem + j->x->keysz;
    const char *aad = iv + SX_GCM_IV_SZ;
    char *ct = (char*)aad + j->x->aadsz;
    const char *tag = ct + j->x->sz;
    struct sxkeyref k;

    r = finish_aesgcm_encrypt(j);
    if (r != SX_OK)
        return r;

    k = sx_keyref_load_material(j->x->keysz, aeskey);

    /* no loadjob() as we want to decrypt already encrypted data in j->dmem*/
    r = sx_aead_create_aesgcm_dec(&j->op.aead, &k, iv);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&j->op.aead, aad, j->x->aadsz);
    if (r != SX_OK)
        return r;
    r = sx_aead_crypt(&j->op.aead, ct, j->x->sz, ct);
    if (r != SX_OK)
        return r;
    r = sx_aead_verify_tag(&j->op.aead, tag);
    if (r != SX_OK)
        return r;
    j->next = check_aesgcm_matches_original;

    return 0;
}


int start_aesgcm_encrypt(struct job *j)
{
    const char *aeskey = j->dmem;
    const char *iv = j->dmem + j->x->keysz;
    const char *aad = iv + SX_GCM_IV_SZ;
    const char *msg = aad + j->x->aadsz;
    char *ct = (char*)aad + j->x->aadsz;
    char *tag = ct + j->x->sz;
    int r;
    struct sxkeyref k;

    loadjob(j, j->x->keysz + SX_GCM_IV_SZ + j->x->aadsz + j->x->sz);
    k = sx_keyref_load_material(j->x->keysz, aeskey);
    r = sx_aead_create_aesgcm_enc(&j->op.aead, &k, iv);
    if (r != SX_OK)
        return r;
    r = sx_aead_feed_aad(&j->op.aead, aad, j->x->aadsz);
       if (r != SX_OK)
           return r;
    r = sx_aead_crypt(&j->op.aead, msg, j->x->sz, ct);
    if (r != SX_OK)
        return r;
    r = sx_aead_produce_tag(&j->op.aead, tag);
    if (r != SX_OK)
        return r;
    j->next = decrypt_aesgcm_after_encrypt;

    return 0;
}


int finish_aesxts(struct job *j)
{
    int r = 0;

    r = sx_blkcipher_wait(&j->op.blkciph);
    write_raw(j->dmem + 2 * j->x->keysz + 16, j->x->sz);
    j->next = NULL;

    return r;
}


int start_aesxts(struct job *j, int direction)
{
    int r = 0;
    const char *key1 = j->dmem;
    const char *key2 = j->dmem + j->x->keysz;
    const char *iv = key2 + j->x->keysz;
    const char *indata = iv + 16;
    char *outdata = j->dmem + 2 * j->x->keysz + 16;
    struct sxkeyref k1, k2;

    k1 = sx_keyref_load_material(j->x->keysz, key1);
    k2 = sx_keyref_load_material(j->x->keysz, key2);

    if (direction)
        r = sx_blkcipher_create_aesxts_enc(&j->op.blkciph, &k1, &k2, iv);
    else
        r = sx_blkcipher_create_aesxts_dec(&j->op.blkciph, &k1, &k2, iv);
    if (r != SX_OK)
        return r;

    r = sx_blkcipher_crypt(&j->op.blkciph, indata, j->x->sz, outdata);
    if (r != SX_OK)
        return r;
    r = sx_blkcipher_run(&j->op.blkciph);
    j->next = finish_aesxts;

    return 0;
}


int check_aesxts_matches_original(struct job *j)
{
    int diff;
    int r;

    r = finish_aesxts(j);
    if (r != SX_OK)
        return r;

    diff = sx_memdiff(j->dmem + j->x->keysz*2 + 16,
        j->x->in + j->x->keysz*2 + 16,
        j->x->sz);
    REPORT_PROGRESS("XTS D(E(k)) == k ?", diff);
    validations++;
    j->next = NULL;

    return diff;
}

int decrypt_after_encrypt(struct job *j)
{
    int r = finish_aesxts(j);

    if (r != SX_OK)
        return r;
    /* no loadjob() as we want to decrypt already encrypted data in j->dmem*/
    r = start_aesxts(j, 0);
    j->next = check_aesxts_matches_original;
    return r;
}

int start_aesxts_encrypt(struct job *j)
{
    int r;

    loadjob(j, 2*j->x->keysz + 16 + j->x->sz);
    r = start_aesxts(j, 1);
    j->next = decrypt_after_encrypt;
    return r;
}



int start_aesxts_decrypt(struct job *j)
{
    int r;

    loadjob(j, 2*j->x->keysz + 16 + j->x->sz);
    r = start_aesxts(j, 0);

    return r;
}


struct action patterns[] = {
    {
        .keysz = 32,
        .aadsz = 39,
        .sz = 17,
        .in =
            "0123456789abcdef0123456789abcdef"
            "\0\0\0\0\0\0\0\0\0\0\0\0"
            "authenticated but not encrypted payload"
            "a secret message!",
        .start = start_aesgcm_encrypt
    },
    {
        .keysz = 32,
        .sz = 64,
        .in =
            "\x27\x18\x28\x18\x28\x45\x90\x45\x23\x53\x60\x28\x74\x71\x35\x26"
            "\x62\x49\x77\x57\x24\x70\x93\x69\x99\x59\x57\x49\x66\x96\x76\x27"
            "\x31\x41\x59\x26\x53\x58\x97\x93\x23\x84\x62\x64\x33\x83\x27\x95"
            "\x02\x88\x41\x97\x16\x93\x99\x37\x51\x05\x82\x09\x74\x94\x45\x92"
            "\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
            "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
            "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f",
        .start = start_aesxts_encrypt
    },
    {
        .keysz = 16,
        .aadsz = 39,
        .sz = 17,
        .in =
            "0123456789abcdef"
            "\0\0\0\0\0\0\0\0\0\0\0\0"
            "authenticated but not encrypted payload"
            "a secret message!",
        .start = start_aesgcm_encrypt
    },
    {
        .keysz = 16,
        .sz = 25,
        .in =
            "\x27\x18\x28\x18\x28\x45\x90\x45\x23\x53\x60\x28\x74\x71\x35\x26"
            "\x31\x41\x59\x26\x53\x58\x97\x93\x23\x84\x62\x64\x33\x83\x27\x95"
            "\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
            "\x20\x21\x22\x23\x24\x25\x26\x27\x28",
        .start = start_aesxts_encrypt
    },
};


int runmany(unsigned int maxpending)
{
    struct job jobs[CFG_MAX_JOBS] = {{0}};
    unsigned int i;
    int r = 0;
    int pending = 0;

    if (maxpending > ARRAY_COUNT(jobs))
        maxpending = ARRAY_COUNT(jobs);

    dmamem = sx_alloc_global_dmamem(MAX_TOTAL_SZ);
    if (!dmamem) {
        DISPLAY_ERROR("Could not allocate DMA memory\n");
        return -1;
    }

    for (i = 0; i < ARRAY_COUNT(jobs); i++)
        jobs[i].dmem = dmamem + JOB_DMAMEM_SZ * i;

    /* start as much jobs as possible, each with a sample pattern */
    for (i = 0; i < ARRAY_COUNT(patterns) && i < maxpending && !r; i++) {
        struct action *x = &patterns[i];
        jobs[i].x = x;
        r = x->start(&jobs[i]);
        pending++;
    }
    /* wait for the end of all encryption operations,
     * and decryptions of the encryptions */
    while (pending && !r) {
        pending = 0;
        for (i = 0; i < ARRAY_COUNT(jobs); i++) {
            if (!jobs[i].next)
                continue;
            pending++;
            r |= jobs[i].next(&jobs[i]);
        }
    }

    return r;
}

int main()
{
    int r;

    r = runmany(CFG_MAX_JOBS);
    REPORT_VALIDATIONS(validations);

    return r;
}
