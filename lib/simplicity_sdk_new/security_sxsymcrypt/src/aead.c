/*
 * @Copyright 2024 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/aead.h"
#include "../include/sxsymcrypt/aes.h"
#include "../include/sxsymcrypt/keyref.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "../include/sxsymcrypt/memdiff.h"
#include "keyrefdefs.h"
#include "aeaddefs.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "cmaes.h"
#include "sl_code_classification.h"

/** Mode Register value for context loading */
#define AES_AEAD_MODEID_CTX_LOAD (1u << 4)
/** Mode Register value for context saving */
#define AES_AEAD_MODEID_CTX_SAVE (1u << 5)

/** Size of AEAD block size, in bytes */
#define AEAD_BLOCK_SZ (16)
/** Size of AEAD GCM and CCM context saving state, in bytes */
#define AES_AEAD_CTX_STATE_SZ (32)
/** Size of AEAD lenAlenC, in bytes */
#define AEAD_LENA_LENC_SZ (16)

void set_nonce_gcm(struct sxaead *c);
void set_nonce_ccm(struct sxaead *c);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_GCM, SL_CODE_CLASS_SECURITY)
static int lenAlenC_aesgcm_ba411(size_t aadsz, size_t datasz, uint8_t *out);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static int lenAlenC_nop(size_t aadsz, size_t datasz, uint8_t *out);


static const struct sx_aead_cmdma_tags ba415tags = {
    .cfg = DMATAG_BA415 | DMATAG_CONFIG(0),
    .key = DMATAG_BA415 | DMATAG_CONFIG(0x10),
    .iv_or_state = DMATAG_BA415 | DMATAG_CONFIG(0x30) | DMATAG_LAST,
    .aad = DMATAG_BA415 | DMATAG_DATATYPE_HEADER,
    .data = DMATAG_BA415,
    .tag = DMATAG_BA415 | DMATAG_DATATYPE_REFERENCE
};

static const char zeros[SX_CCM_MAX_TAG_SZ] = { 0 };


static const struct sx_aead_cmdma_cfg ba415gcmcfg = {
    .decr = CM_CFG_DECRYPT,
    .mode = 0,
    .dmatags = &ba415tags,
    .verifier = zeros,
    .lenAlenC = lenAlenC_nop,
    .granularity = AEAD_BLOCK_SZ,
    .statesz = 0,
    .inputminsz = 1,
    .tagminsz = 16,
    .hwtagverif = 1
};


static const struct sx_aead_cmdma_tags ba411aeadtags = {
    .cfg = DMATAG_BA411 | DMATAG_CONFIG(0),
    .iv_or_state = DMATAG_BA411 | DMATAG_CONFIG(0x28),
    .key = DMATAG_BA411 | DMATAG_CONFIG(0x08),
    .aad = DMATAG_BA411 | DMATAG_DATATYPE_HEADER,
    .tag = DMATAG_BA411,
    .data = DMATAG_BA411
};


#define BA411_MODEID_GCM 6
static const struct sx_aead_cmdma_cfg ba411gcmcfg = {
    .decr = CM_CFG_DECRYPT,
    .mode = BA411_MODEID_GCM,
    .dmatags = &ba411aeadtags,
    .verifier = NULL,
    .lenAlenC = lenAlenC_aesgcm_ba411,
    .set_nonce = set_nonce_gcm,
    .ctxsave = AES_AEAD_MODEID_CTX_SAVE,
    .ctxload = AES_AEAD_MODEID_CTX_LOAD,
    .granularity = AEAD_BLOCK_SZ,
    .statesz = AES_AEAD_CTX_STATE_SZ,
    .inputminsz = 0,
    .tagminsz = 1,
    .hwtagverif = 0
};


#define BA411_MODEID_CCM 5
static const struct sx_aead_cmdma_cfg ba411ccmcfg = {
    .decr = CM_CFG_DECRYPT,
    .mode = BA411_MODEID_CCM,
    .dmatags = &ba411aeadtags,
    .verifier = zeros,
    .lenAlenC = lenAlenC_nop,
    .set_nonce = set_nonce_ccm,
    .ctxsave = AES_AEAD_MODEID_CTX_SAVE,
    .ctxload = AES_AEAD_MODEID_CTX_LOAD,
    .granularity = AEAD_BLOCK_SZ,
    .statesz = AES_AEAD_CTX_STATE_SZ,
    .inputminsz = 0,
    .tagminsz = 4,
    .hwtagverif = 1
};


/** Returns the bitmask for key size used for verifying HW capabilities. */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static uint32_t sx_aead_ba411_key_mask(size_t keysz)
{
    switch (keysz) {
    case 16:
        return 1 << 24;
    case 24:
        return 1 << 25;
    case 32:
        return 1 << 26;
    }

    return ~0u;
}


static int lenAlenC_nop(size_t aadsz, size_t datasz, uint8_t *out)
{
    (void)aadsz;
    (void)datasz;
    (void)out;

    return 0;
}


static int lenAlenC_aesgcm_ba411(size_t aadsz, size_t datasz, uint8_t *out)
{
    uint32_t i = 0;
    aadsz = aadsz << 3;
    datasz = datasz << 3;
    for (i = 0; i < 8; i++) {
        out[7 - i] = aadsz & 0xFF;
        aadsz >>= 8;
    }
    out += 8;
    for (i = 0; i < 8; i++) {
        out[7 - i] = datasz & 0xFF;
        datasz >>= 8;
    }

    return 1;
}


void set_nonce_gcm(struct sxaead *c)
{
    ADD_INDESC_PRIV(c->dma,
        (OFFSET_EXTRAMEM(c) + sizeof(c->extramem) - c->cfg->statesz),
        SX_GCM_IV_SZ, c->cfg->dmatags->iv_or_state);
}


void set_nonce_ccm(struct sxaead *c)
{
    (void)c;
    return;
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static void sx_memcpy(void* dst, void* src, size_t length)
{
    for (size_t i = 0; i < length; i++)
        ((uint8_t*) dst)[i] = ((uint8_t*) src)[i];
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static void sx_writebe(uint8_t *out, uint64_t data, uint16_t targetsz)
{
    uint16_t i;
    for (i = 0; i < targetsz; i++)
        out[(targetsz - 1) - i] = (data >> (i * 8)) & 0xFF;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_CCM, SL_CODE_CLASS_SECURITY)
int sx_aead_create_ccmheader(const char *nonce, size_t noncesz,
    uint8_t tagsz, uint64_t aadsz, uint64_t datasz, uint8_t *header,
    uint8_t *headersz)
{
    char flags;
    size_t m, l;

    /* RFC3610 paragraph 2.2 defines the formatting of the first block.
     * M, CCM MAC size is one of {4,6,8,10,12,14,16}, CCM* not supported
     * (MAC size 0)
     * L must be between 2 and 8.
     * Nonce size should not be between 7 and 13 bytes.
     * The first block contains:
     *  byte  [0]           the flags byte (see below)
     *  bytes [1,1+nonce.len]   nonce
     *  bytes [2+nonce.len, 16] message length
     *
     *  The flags byte has the following bit fields:
     *    [6:7] = 0
     *    [3:5] = authentication tag size, encoded as (tagsz-2)/2
     *              only multiples of 2 between 2 and 16 are allowed.
     *    [0:2] = length field size minus 1.
     **/
    l = 15 - noncesz;

    flags = (aadsz > 0) ? (1 << 6) : 0;

    /* Authentication tag size encoding. A tagsz = 0 is accepted by CCM* and
     * that will be encoded as a 0. */
    m = (tagsz > 0) ? (tagsz - 2) / 2 : 0;

    flags |= (m & 0x7) << 3;
    flags |= ((l - 1) & 0x7);
    header[0] = flags;

    sx_memcpy(&header[1], (void *) nonce, noncesz);

    sx_writebe(&(header[1 + noncesz]), datasz, l);
    /* if there's additional authentication data, encode the size */
    if (aadsz > 0) {
        if (aadsz < 0xFF00) {
            sx_writebe(&header[16], aadsz, 2);
            *headersz = 18;
        } else if (aadsz <= 0xFFFFFFFF) {
            header[16] = 0xFF;
            header[17] = 0xFE;
            sx_writebe(&header[18], aadsz, 4);
            *headersz = 22;
        } else {
            header[16] = 0xFF;
            header[17] = 0xFF;
            sx_writebe(&header[18], aadsz, 8);
            *headersz = 26;
        }
    } else {
        *headersz = 16;
    }

    return SX_OK;
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static void sx_aead_free(struct sxaead *c)
{
    sx_cmdma_release_hw(c->dma.regs);
    c->dma.regs = NULL;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_GCM, SL_CODE_CLASS_SECURITY)
static int sx_aead_create_aesgcm(struct sxaead *c, const struct sxkeyref *key,
    const char *iv, const struct sxaesparams *params)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba415 = ~0u;
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba411 = ~0u;
    uint32_t keyszfld = 0;
    unsigned int mode_compatibleba411 = ~0u;
    uint32_t hwreqs = 0;
    uint32_t hw_temp_red = 0;

    c->dma.regs = NULL;
    c->compatible = 0;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;

    if (KEYREF_IS_USR(key)) {
        keyszfld = sx_aes_keysz(key->sz);
        if (keyszfld == ~0u)
            return SX_ERR_INVALID_KEY_SZ;
        hwreqs = sx_aead_ba411_key_mask(key->sz);
    }

    /* BA415 can be used only with user provided keys that have 16 or 32 bytes
     * or with HW keys. */
    if ((key->sz != 24)  && (!CM_AES_TEMPO_REDUNDANCY_IS_SET(params->config))) {
        if (compatibleba415 == ~0u) {
            compatibleba415 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA415);
        }

        c->dma.regs = sx_cmdma_find_available(compatibleba415);
        c->cfg = &ba415gcmcfg;
    }

    if (!c->dma.regs) {
        if (compatibleba411 == ~0u) {
            compatibleba411 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA411);
        }

        hwreqs |= (1 << BA411_MODEID_GCM);
        mode_compatibleba411 = sx_cmdma_filter_compatible(compatibleba411,
                REG_BA411_CAPS, hwreqs);

        if (CM_AES_TEMPO_REDUNDANCY_IS_SET(params->config)) {
            hw_temp_red = CM_TEMPORAL_REDUNDANCY_CFG_EN;
            hwreqs = CM_TEMPORAL_REDUNDANCY_CFG2_EN;
            mode_compatibleba411 = sx_cmdma_filter_compatible(mode_compatibleba411,
                REG_BA411_CTR_SZ, hwreqs);
        }

        c->dma.regs = sx_cmdma_find_available(mode_compatibleba411);
        c->cfg = &ba411gcmcfg;
        c->compatible = mode_compatibleba411;
        keyszfld = 0;
        /* Backup the IV in case we won't call HW when context saving
         * The backup for AES GCM is done only for the case where BA411 is used.
         * This is because the HP BA415 AES GCM HP engine does not support context
         * saving, thus IV does not need to be backed-up. */
        sx_memcpy(c->extramem + sizeof(c->extramem) - c->cfg->statesz, (char *)iv, SX_GCM_IV_SZ);
    }

    if (((!compatibleba415) || (compatibleba415 && !c->dma.regs)) &&
        (!mode_compatibleba411))
        return SX_ERR_INCOMPATIBLE_HW;

    if (!c->dma.regs)
        return SX_ERR_RETRY;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
                CMDMA_AEAD_MODE_SET(c->cfg->mode) | KEYREF_AES_HWKEY_CONF(key->cfg) | keyszfld | hw_temp_red,
                c->cfg->dmatags->cfg);

    if (KEYREF_IS_USR(key))
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    ADD_CFGDESC(c->dma, iv, SX_GCM_IV_SZ, c->cfg->dmatags->iv_or_state);

    c->totalaadsz = 0;
    c->discardaadsz = 0;
    c->datainsz = 0;
    c->dataintotalsz = 0;
    c->extraaadsz = 0;
    c->extraaadptr = c->extraaadmem;
    c->no_exec = 0;
    c->tagsz = SX_GCM_TAG_SZ;
    c->expectedtag = c->cfg->verifier;
    c->key = key;

    return SX_OK;
}


int sx_aead_create_aesgcm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *iv)
{
    int r;
    struct sxaesparams params = {.config = 0};

    r = sx_aead_create_aesgcm(c, key, iv, &params);
    if (r != SX_OK)
        return r;

    return SX_OK;
}


int sx_aead_create_aesgcm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *iv)
{
    int r;
    struct sxaesparams params = {.config = 0};

    r = sx_aead_create_aesgcm(c, key, iv, &params);
    if (r != SX_OK)
        return r;

    c->dma.dmamem.cfg |= c->cfg->decr;

    return SX_OK;
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_CCM, SL_CODE_CLASS_SECURITY)
static int sx_aead_create_aesccm(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz, size_t datasz,
    const uint32_t dir, const struct sxaesparams *params)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba411 = ~0u;
    unsigned int mode_compatibleba411 = ~0u;
    uint32_t hwreqs = 0;
    uint32_t hw_temp_red = 0;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (sx_aes_keysz((key)->sz) == ~0u)
            return SX_ERR_INVALID_KEY_SZ;
        hwreqs = sx_aead_ba411_key_mask(key->sz);
    }
    if ((tagsz & 1) || (tagsz == 2) || (tagsz > 16))
        return SX_ERR_INVALID_TAG_SIZE;

    if ((noncesz < 7) || (noncesz > 13))
        return SX_ERR_INVALID_NONCE_SIZE;

    /* datasz must ensure  0 <= datasz < 2^(8L) */
    uint8_t l = 15 - noncesz;
    if ((l < 8U) && (datasz >= (1ULL << (l * 8)))) {
        /* message too long to encode the size in the CCM header */
        return SX_ERR_TOO_BIG;
    }

    if (compatibleba411 == ~0u) {
        compatibleba411 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA411);
    }

    hwreqs |= (1 << BA411_MODEID_CCM);
    mode_compatibleba411 = sx_cmdma_filter_compatible(compatibleba411,
            REG_BA411_CAPS, hwreqs);

    if (CM_AES_TEMPO_REDUNDANCY_IS_SET(params->config)) {
        hw_temp_red = CM_TEMPORAL_REDUNDANCY_CFG_EN;
        hwreqs = CM_TEMPORAL_REDUNDANCY_CFG2_EN;
        mode_compatibleba411 = sx_cmdma_filter_compatible(mode_compatibleba411,
            REG_BA411_CTR_SZ, hwreqs);
    }
    if (!mode_compatibleba411)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba411);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba411ccmcfg;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
            CMDMA_AEAD_MODE_SET(c->cfg->mode) | KEYREF_AES_HWKEY_CONF(key->cfg) | dir | hw_temp_red,
            c->cfg->dmatags->cfg);

    if (KEYREF_IS_USR(key))
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);

    //create header
    uint8_t *header = c->extramem;
    uint8_t headersz;
    int r = sx_aead_create_ccmheader(nonce, noncesz, tagsz, aadsz, datasz,
            header, &headersz);
    if (r != SX_OK)
        return r;

    /* Add first block of header as AAD */
    ADD_INDESC_PRIV_RAW(c->dma, OFFSET_EXTRAMEM(c), AEAD_BLOCK_SZ, c->cfg->dmatags->aad);

    c->totalaadsz = AEAD_BLOCK_SZ;
    c->discardaadsz = AEAD_BLOCK_SZ;
    c->datainsz = 0;
    c->dataintotalsz = 0;
    c->extraaadsz = 0;
    c->extraaadptr = c->extraaadmem;
    c->no_exec = 0;
    c->extraaadptr = c->extraaadmem;
    c->tagsz = tagsz;
    c->key = key;
    c->compatible = mode_compatibleba411;
    /* For CCM decryption, BA411 engine will compute the output tag as
     * tagInputed ^ tagComputed. If inputed tag and computed tag are
     * identical, the outputted tag will be an array of zeros with tagsz
     * length. For encryption, expectedtag will be set to NULL by
     * sx_aead_crypt() to disable verification.
     */
    c->expectedtag = c->cfg->verifier;

    /* If remaining header, write it in extraadmem */
    headersz -= AEAD_BLOCK_SZ;
    if (headersz) {
        sx_memcpy(c->extraaadptr, header + AEAD_BLOCK_SZ, headersz);
        c->extraaadsz = headersz;
    }

    return SX_OK;
}


int sx_aead_create_aesccm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz,
    size_t datasz)
{
    struct sxaesparams params = {.config = 0};

    return sx_aead_create_aesccm(c, key, nonce, noncesz, tagsz, aadsz, datasz,
            0, &params);
}


int sx_aead_create_aesccm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz,
    size_t datasz)
{
    struct sxaesparams params = {.config = 0};

    return sx_aead_create_aesccm(c, key, nonce, noncesz, tagsz, aadsz, datasz,
            ba411ccmcfg.decr, &params);
}


int sx_aead_feed_aad(struct sxaead *c, const char *aad, size_t aadsz)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (aadsz >= DMA_MAX_SZ) {
        sx_aead_free(c);
        return SX_ERR_TOO_BIG;
    }
    if (c->dataintotalsz) {
        sx_aead_free(c);
        return SX_ERR_FEED_AFTER_DATA;
    }

    /* Store data at the end of previous buffer unprocessed. */
    if (c->extraaadsz + aadsz < AEAD_BLOCK_SZ) {
        sx_memcpy(c->extraaadptr + c->extraaadsz, (char *)aad, aadsz);
        c->extraaadsz += aadsz;
        return SX_OK;
    }

    /* If we get here, there is at least one block to process */
    if (c->extraaadsz) {
        ADD_RAW_INDESC(c->dma, c->extraaadptr, c->extraaadsz, c->cfg->dmatags->aad);
        c->discardaadsz += c->extraaadsz;
        c->totalaadsz += c->extraaadsz;
        c->extraaadsz = 0;
    }

    /* Add AAD size that will be a multiple of block size when combined with previously
     * internally stored data added just above */
    size_t feedsz = ((c->discardaadsz + aadsz) & (~(AEAD_BLOCK_SZ - 1))) - c->discardaadsz;
    ADD_RAW_INDESC(c->dma, aad, feedsz, c->cfg->dmatags->aad);
    c->discardaadsz += feedsz;
    c->totalaadsz += feedsz;

    /* Store remaining bytes internally */
    if (aadsz != feedsz) {
        /* Toggle extraaad memory pointer as we need previously stored data to be
         * valid until HW call */
        c->extraaadptr += SX_BLKCIPHER_PRIV_SZ - 2 * (c->extraaadptr - c->extraaadmem);
        size_t extraaadsz = aadsz - feedsz;
        sx_memcpy(c->extraaadptr, (char *)aad + feedsz, extraaadsz);
        c->extraaadsz = extraaadsz;
    }

    return SX_OK;
}


int sx_aead_multifeed_aad(struct sxaead *c, struct sxdataref *aadin, size_t countaadin)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (c->dataintotalsz) {
        sx_aead_free(c);
        return SX_ERR_FEED_AFTER_DATA;
    }
    if (countaadin == 0) {
        sx_aead_free(c);
        return SX_ERR_TOO_SMALL;
    }
    if (countaadin > SX_AEAD_AAD_EXTRA_DESC_IN + 1) {
        sx_aead_free(c);
        return SX_ERR_FEED_COUNT_EXCEEDED;
    }

    /* For CCM, feed potential remaining bytes of header */
    if (c->extraaadsz) {
        ADD_RAW_INDESC(c->dma, c->extraaadptr, c->extraaadsz, c->cfg->dmatags->aad);
        c->discardaadsz += c->extraaadsz;
        c->totalaadsz += c->extraaadsz;
        c->extraaadsz = 0;
    }

    /* Set input descriptors */
    for (size_t i = 0; i < countaadin; i++) {
        if (aadin[i].sz) {
            if (aadin[i].sz >= DMA_MAX_SZ) {
                sx_aead_free(c);
                return SX_ERR_TOO_BIG;
            }
            ADD_RAW_INDESC(c->dma, aadin[i].data, aadin[i].sz, c->cfg->dmatags->aad);
            c->discardaadsz += aadin[i].sz;
            c->totalaadsz += aadin[i].sz;
        }
    }
    SET_LAST_DESC_IGN(c->dma, c->totalaadsz, 0x0F);

    return SX_OK;
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static void sx_aead_discard_aad(struct sxaead *c)
{
    if (c->discardaadsz) {
        ADD_DISCARDDESC(c->dma, ALIGN_SZA(c->discardaadsz, 0xf));
        c->discardaadsz = 0;
    }
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static void sx_aead_finish_and_discard_aad(struct sxaead *c)
{
    /* Add extraaad remaining as no more AAD data can be
     * provided when reaching here */
    if (c->extraaadsz) {
        ADD_INDESCA(c->dma, c->extraaadptr, c->extraaadsz, c->cfg->dmatags->aad, 0xf);
        c->discardaadsz += c->extraaadsz;
        c->totalaadsz += c->extraaadsz;
        c->extraaadsz = 0;
    }

    sx_aead_discard_aad(c);
}


int sx_aead_crypt(struct sxaead *c, const char *datain, size_t datainsz,
    char *dataout)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (datainsz >= DMA_MAX_SZ) {
        sx_aead_free(c);
        return SX_ERR_TOO_BIG;
    }
    if (c->datainsz) {
        sx_aead_free(c);
        return SX_ERR_FEED_AFTER_FEED;
    }

    sx_aead_finish_and_discard_aad(c);

    if (datainsz) {
        ADD_INDESCA(c->dma, datain, datainsz, c->cfg->dmatags->data, 0xf);
        c->dataintotalsz += datainsz;
        c->datainsz = datainsz;
        ADD_OUTDESCA(c->dma, dataout, datainsz, 0xf);
    }

    return SX_OK;
}


int sx_aead_multifeed_crypt(struct sxaead *c,
    const struct sxdataref *datain, size_t countdatain,
    const struct sxdataref *dataout, size_t countdataout)
{
    size_t outputsz = 0;
    size_t extrablocksz;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (c->datainsz) {
        sx_aead_free(c);
        return SX_ERR_FEED_AFTER_FEED;
    }
    if ((countdatain == 0) || (countdataout == 0)) {
        sx_aead_free(c);
        return SX_ERR_TOO_SMALL;
    }
    if ((countdatain > SX_AEAD_DATA_EXTRA_DESC_IN + 1) ||
        (countdataout > SX_AEAD_DATA_EXTRA_DESC_OUT + 1)) {
        sx_aead_free(c);
        return SX_ERR_FEED_COUNT_EXCEEDED;
    }

    sx_aead_finish_and_discard_aad(c);

    /* Set input descriptors */
    for (size_t i = 0; i < countdatain; i++) {
        if (datain[i].sz) {
            if (datain[i].sz >= DMA_MAX_SZ) {
                sx_aead_free(c);
                return SX_ERR_TOO_BIG;
            }
            ADD_RAW_INDESC(c->dma, datain[i].data, datain[i].sz, c->cfg->dmatags->data);
            c->datainsz += datain[i].sz;
            c->dataintotalsz += datain[i].sz;
        }
    }
    SET_LAST_DESC_IGN(c->dma, c->datainsz, 0x0F);

    /* Set output descriptors */
    for (size_t i = 0; i < countdataout; i++) {
        if (dataout[i].sz) {
            if (dataout[i].sz >= DMA_MAX_SZ) {
                sx_aead_free(c);
                return SX_ERR_TOO_BIG;
            }
            ADD_RAW_OUTDESC(c->dma, dataout[i].data, dataout[i].sz);
            outputsz += dataout[i].sz;
        }
    }
    extrablocksz = c->datainsz & 0x0F;
    if (extrablocksz)
        ADD_DISCARDDESC(c->dma, 0x10 - extrablocksz);

    /* Check that input and output size match */
    if (outputsz != c->datainsz) {
        sx_aead_free(c);
        return SX_ERR_FEED_UNBALANCED;
    }

    return SX_OK;
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static int sx_aead_run(struct sxaead *c)
{
    sx_cmdma_start(&c->dma, sizeof(c->descs) + sizeof(c->extramem), c->descs);

    return SX_OK;
}


int sx_aead_produce_tag(struct sxaead *c, char *tagout)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    sx_aead_finish_and_discard_aad(c);

    if ((c->dataintotalsz + c->totalaadsz) < c->cfg->inputminsz) {
        sx_aead_free(c);
        return SX_ERR_INCOMPATIBLE_HW;
    }

    if (c->cfg->lenAlenC(c->totalaadsz, c->dataintotalsz, &c->extramem[0]))
        ADD_INDESC_PRIV(c->dma, OFFSET_EXTRAMEM(c), AEAD_LENA_LENC_SZ,
                c->cfg->dmatags->data);

    ADD_OUTDESCA(c->dma, tagout, c->tagsz, 0xf);

    c->expectedtag = NULL;

    return sx_aead_run(c);
}


int sx_aead_verify_tag(struct sxaead *c, const char *tagin)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (c->cfg->mode == BA411_MODEID_CCM) {
        if ((c->dma.dmamem.cfg & c->cfg->ctxload) && (c->datainsz == 0)) {
            sx_aead_free(c);
            return SX_ERR_TOO_SMALL;
        }
    }

    sx_aead_finish_and_discard_aad(c);

    if ((c->dataintotalsz + c->totalaadsz) < c->cfg->inputminsz) {
        sx_aead_free(c);
        return SX_ERR_INCOMPATIBLE_HW;
    }

    if (c->cfg->lenAlenC(c->totalaadsz, c->dataintotalsz, &c->extramem[0])) {
        ADD_INDESC_PRIV(c->dma, OFFSET_EXTRAMEM(c), AEAD_LENA_LENC_SZ,
                c->cfg->dmatags->data);
        c->expectedtag = tagin;
    } else if (!c->cfg->hwtagverif) {
        c->expectedtag = tagin;
    } else {
        if (DMATAG_DATATYPE_REFERENCE ==
            (c->cfg->dmatags->tag & DMATAG_DATATYPE_REFERENCE))
            UPDATE_LASTDESC_TAG(c->dma, DMATAG_LAST);
        if (c->tagsz)
            ADD_INDESCA(c->dma, tagin, c->tagsz, c->cfg->dmatags->tag, 0xf);
    }

    ADD_OUTDESC_PRIV(c->dma, OFFSET_EXTRAMEM(c), c->tagsz, 0xf);

    return sx_aead_run(c);
}


int sx_aead_resume_state(struct sxaead *c)
{
    if (c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if (!c->compatible)
        return SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED;

    c->dma.regs = sx_cmdma_find_available(c->compatible);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->dma.dmamem.cfg &= ~(c->cfg->ctxsave);

    if (c->totalaadsz != 0 || c->dataintotalsz != 0) {
        /* Some HW call just occured when saving context, then restore
         * context as usual */
        c->dma.dmamem.cfg |= c->cfg->ctxload;
    }

    /* Add new command */
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
            c->dma.dmamem.cfg , c->cfg->dmatags->cfg);

    /* Add key if needed */
    if (KEYREF_IS_USR(c->key))
        ADD_CFGDESC(c->dma, c->key->key, c->key->sz, c->cfg->dmatags->key);

    /* If context saving has already been performed at least once, load context.
     * Otherwise, load as if it were an initial context */
    if (c->dma.dmamem.cfg & c->cfg->ctxload) {
        ADD_INDESC_PRIV(c->dma,
            (OFFSET_EXTRAMEM(c) + sizeof(c->extramem) - c->cfg->statesz),
            c->cfg->statesz, c->cfg->dmatags->iv_or_state);
    } else {
        c->cfg->set_nonce(c);
    }

    c->datainsz = 0;
    c->discardaadsz = 0;
    c->no_exec = 0;

    return SX_OK;
}


int sx_aead_save_state(struct sxaead *c)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if (c->cfg->statesz == 0) {
        sx_aead_free(c);
        return SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED;
    }

    if (c->dataintotalsz % c->cfg->granularity) {
        sx_aead_free(c);
        return SX_ERR_WRONG_SIZE_GRANULARITY;
    }

    if (!c->discardaadsz && !c->datainsz) {
        c->no_exec = 1;
        return SX_OK;
    }

    sx_aead_discard_aad(c);

    ADD_OUTDESC_PRIV(c->dma,
            (OFFSET_EXTRAMEM(c) + sizeof(c->extramem) - c->cfg->statesz),
            c->cfg->statesz, 0x0F);

    c->dma.dmamem.cfg |= c->cfg->ctxsave;

    sx_cmdma_start(&c->dma,  sizeof(c->descs) + sizeof(c->extramem),
        c->descs);

    return SX_OK;
}


int sx_aead_status(struct sxaead *c)
{
    int r;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    r = sx_cmdma_check(&c->dma);
    if (r == SX_ERR_HW_PROCESSING)
        return r;
    if (r != SX_OK) {
        sx_aead_free(c);
        return r;
    }

    if (!c->no_exec) {
        if (c->dma.dmamem.cfg & c->cfg->ctxsave) {
            sx_flush_fromhw(c->dma.regs,
                    (char *)&c->extramem + sizeof(c->extramem) - c->cfg->statesz,
                    OFFSET_EXTRAMEM(c) + sizeof(c->extramem) - c->cfg->statesz,
                    c->cfg->statesz);
        } else if (c->expectedtag != NULL) {
            sx_flush_fromhw(c->dma.regs, (char *)&c->extramem,
                OFFSET_EXTRAMEM(c), sizeof(c->extramem));
            r = sx_memdiff(c->expectedtag, (const char *) c->extramem,
                    c->tagsz) ? SX_ERR_INVALID_TAG : SX_OK;
        } else {
            sx_flush_fromhw(c->dma.regs, (char *)&c->extramem, OFFSET_EXTRAMEM(c), 0);
        }
    }

    sx_aead_free(c);

    return r;
}


int sx_aead_wait(struct sxaead *c)
{
    int r = SX_ERR_HW_PROCESSING;

    while (r == SX_ERR_HW_PROCESSING) {
        sx_cmdma_wait(c->dma.regs);
        r = sx_aead_status(c);
    }

    return r;
}


int sx_aead_truncate_tag(struct sxaead *c, const size_t tagsz)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (c->cfg->mode == AEAD_MODEID_CCM) {
        if (tagsz != c->tagsz) {
            sx_aead_free(c);
            return SX_ERR_INVALID_TAG_SIZE;
        }
    }
    if (tagsz < c->cfg->tagminsz) {
        sx_aead_free(c);
        return SX_ERR_INVALID_TAG_SIZE;
    }

    c->tagsz = tagsz;

    return SX_OK;
}


int sx_aead_create_aes_generic(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce_or_iv, size_t noncesz, size_t tagsz, size_t aadsz, size_t datasz,
    const uint32_t dir, const char mode, const struct sxaesparams *params)
{
    int r = SX_OK;
    uint32_t enc_dec;

    switch (mode) {
    case AEAD_MODEID_CCM:
        enc_dec = (dir == CM_CFG_ENCRYPT) ? CM_CFG_ENCRYPT : ba411ccmcfg.decr;
        r = sx_aead_create_aesccm(c, key, nonce_or_iv, noncesz, tagsz, aadsz, datasz,
                enc_dec, params);
        break;
    case AEAD_MODEID_GCM:
        r = sx_aead_create_aesgcm(c, key, nonce_or_iv, params);
        if (dir == CM_CFG_DECRYPT)
            c->dma.dmamem.cfg |= c->cfg->decr;
        break;
    default:
        r = SX_ERR_INCOMPATIBLE_HW;
        break;
    }

    return r;
}
