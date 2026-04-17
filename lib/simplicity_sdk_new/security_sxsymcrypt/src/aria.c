/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/aria.h"
#include "../include/sxsymcrypt/aead.h"
#include "../include/sxsymcrypt/mac.h"
#include "../include/sxsymcrypt/keyref.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "blkcipherdefs.h"
#include "aeaddefs.h"
#include "keyrefdefs.h"
#include "macdefs.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "cmaes.h"

/* Size of the key used in ARIA */
#define SX_BLKCIPHER_ARIA_KEYSZ 16

#define CMDMA_BA424_BUS_MSK (0x0F)
#define ARIA_CMAC_MODEID_BA424 8
#define ARIA_CMAC_BLOCK_SZ 16
#define ARIA_CMAC_STATE_SZ 16
#define ARIA_CMAC_MAC_SZ 16

/** Mode Register value for context loading */
#define ARIA_MODEID_CTX_LOAD (1u << 4)
/** Mode Register value for context saving */
#define ARIA_MODEID_CTX_SAVE (1u << 5)

/** ARIA block size, in bytes */
#define ARIA_BLOCK_SZ (16)
/** ARIA GCM and CCM context saving state size, in bytes */
#define ARIA_AEAD_CTX_STATE_SZ (32)
/** AES block cipher context saving state size, in bytes */
#define ARIA_BLKCIPHER_STATE_SZ (16)

extern int sx_aead_create_ccmheader(const char *nonce, size_t noncesz,
        uint8_t tagsz, uint64_t aadsz, uint64_t datasz, uint8_t *header,
        uint8_t *headersz);
extern void set_nonce_gcm(struct sxaead *c);
extern void set_nonce_ccm(struct sxaead *c);

static int lenAlenC_nop(size_t aadsz, size_t datasz, uint8_t *out);
static int lenAlenC_ariagcm_ba424(size_t aadsz, size_t datasz, uint8_t *out);


static const struct sx_blkcipher_cmdma_tags ba424tags = {
    .cfg = DMATAG_BA424 | DMATAG_CONFIG(0),
    .key = DMATAG_BA424 | DMATAG_CONFIG(0x08),
    .iv_or_state = DMATAG_BA424 | DMATAG_CONFIG(0x28),
    .data = DMATAG_BA424
};


static const struct sx_blkcipher_cmdma_cfg ba424ecbcfg = {
    .decr = CM_CFG_DECRYPT,
    .dmatags = &ba424tags,
    .statesz = 0,
    .mode = BLKCIPHER_MODEID_ECB,
    .inminsz = 16,
    .granularity = 16,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba424cbccfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = ARIA_MODEID_CTX_SAVE,
    .ctxload = ARIA_MODEID_CTX_LOAD,
    .dmatags = &ba424tags,
    .statesz = ARIA_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CBC,
    .inminsz = 16,
    .granularity = 16,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba424ofbcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = ARIA_MODEID_CTX_SAVE,
    .ctxload = ARIA_MODEID_CTX_LOAD,
    .dmatags = &ba424tags,
    .statesz = ARIA_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_OFB,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba424cfbcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = ARIA_MODEID_CTX_SAVE,
    .ctxload = ARIA_MODEID_CTX_LOAD,
    .dmatags = &ba424tags,
    .statesz = ARIA_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CFB,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba424ctrcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = ARIA_MODEID_CTX_SAVE,
    .ctxload = ARIA_MODEID_CTX_LOAD,
    .dmatags = &ba424tags,
    .statesz = ARIA_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CTR,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_aead_cmdma_tags ba424aeadtags = {
    .cfg = DMATAG_BA424 | DMATAG_CONFIG(0),
    .iv_or_state = DMATAG_BA424 | DMATAG_CONFIG(0x28),
    .key = DMATAG_BA424 | DMATAG_CONFIG(0x08),
    .aad = DMATAG_BA424 | DMATAG_DATATYPE_HEADER,
    .tag = 0,
    .data = DMATAG_BA424
};

#define ARIA_MODEID_GCM 6
static const struct sx_aead_cmdma_cfg ba424gcmcfg = {
    .decr = CM_CFG_DECRYPT,
    .mode = ARIA_MODEID_GCM,
    .dmatags = &ba424aeadtags,
    .verifier = NULL,
    .lenAlenC = lenAlenC_ariagcm_ba424,
    .set_nonce = set_nonce_gcm,
    .ctxsave = ARIA_MODEID_CTX_SAVE,
    .ctxload = ARIA_MODEID_CTX_LOAD,
    .granularity = ARIA_BLOCK_SZ,
    .statesz = ARIA_AEAD_CTX_STATE_SZ,
    .inputminsz = 0,
    .tagminsz = 1,
    .hwtagverif = 0
};


#define ARIA_MODEID_CCM 5
const struct sx_aead_cmdma_cfg ba424ccmcfg = {
    .decr = CM_CFG_DECRYPT,
    .mode = ARIA_MODEID_CCM,
    .dmatags = &ba424aeadtags,
    .verifier = NULL,
    .lenAlenC = lenAlenC_nop,
    .set_nonce = set_nonce_ccm,
    .ctxsave = ARIA_MODEID_CTX_SAVE,
    .ctxload = ARIA_MODEID_CTX_LOAD,
    .granularity = ARIA_BLOCK_SZ,
    .statesz = ARIA_AEAD_CTX_STATE_SZ,
    .inputminsz = 0,
    .tagminsz = 4,
    .hwtagverif = 0
};


static const struct sx_mac_cmdma_tags ba424tags_cmac = {
    .cfg = DMATAG_BA424 | DMATAG_CONFIG(0),
    .state = DMATAG_BA424 | DMATAG_CONFIG(0x28),
    .key = DMATAG_BA424 | DMATAG_CONFIG(0x08),
    .data = DMATAG_BA424
};


static const struct sx_mac_cmdma_cfg ba424cfg_cmac = {
    .cmdma_mask = CMDMA_BA424_BUS_MSK,
    .granularity = ARIA_CMAC_BLOCK_SZ,
    .blocksz = ARIA_CMAC_BLOCK_SZ,
    .statesz = ARIA_CMAC_STATE_SZ,
    .savestate = ARIA_MODEID_CTX_SAVE,
    .loadstate = ARIA_MODEID_CTX_LOAD,
    .dmatags = &ba424tags_cmac,
};


/** Returns the bitmask for key size used for verifying HW capabilities. */
static uint32_t sx_cmac_key_mask(size_t keysz)
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


static void sx_memcpy(void* dst, void* src, size_t length)
{
    for (size_t i = 0; i < length; i++)
        ((uint8_t*) dst)[i] = ((uint8_t*) src)[i];
}


int lenAlenC_nop(size_t aadsz, size_t datasz, uint8_t *out)
{
    (void)aadsz;
    (void)datasz;
    (void)out;

    return 0;
}

static int lenAlenC_ariagcm_ba424(size_t aadsz, size_t datasz, uint8_t *out)
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


static int sx_blkcipher_create_aria_ba424(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv,
    const struct sx_blkcipher_cmdma_cfg *cfg, const uint32_t dir)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba424 = ~0u;
    unsigned int mode_compatibleba424;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key))
        if (sx_aes_keysz((key)->sz) == ~0u)
            return SX_ERR_INVALID_KEY_SZ;

    if (compatibleba424 == ~0u)
        compatibleba424 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA424);

    mode_compatibleba424 = sx_cmdma_filter_compatible(compatibleba424,
            REG_BA424_CAPS, 1 << cfg->mode);
    if (!mode_compatibleba424)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba424);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = cfg;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
    CMDMA_BLKCIPHER_MODE_SET(cfg->mode) | key->cfg | dir, c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(key))
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    if (iv != NULL)
        ADD_CFGDESC(c->dma, iv, 16, c->cfg->dmatags->iv_or_state);

    c->key = key;
    c->compatible = mode_compatibleba424;
    c->textsz = 0;
    c->is_multifeed = 0;
    c->extradatasz = 0;
    c->extradataptr = c->extradata;
    c->outputsz_bkp = 0;
    c->iv_loaded = 0;

    return SX_OK;
}


int sx_blkcipher_create_ariactr_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424ctrcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_ariactr_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424ctrcfg,
        ba424ctrcfg.decr);
}


int sx_blkcipher_create_ariaecb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key)
{
    return sx_blkcipher_create_aria_ba424(c, key, NULL, &ba424ecbcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_ariaecb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key)
{
    return sx_blkcipher_create_aria_ba424(c, key, NULL, &ba424ecbcfg,
        ba424ecbcfg.decr);
}


int sx_blkcipher_create_ariacbc_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424cbccfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_ariacbc_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424cbccfg,
        ba424cbccfg.decr);
}


int sx_blkcipher_create_ariacfb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424cfbcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_ariacfb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424cfbcfg,
        ba424cfbcfg.decr);
}


int sx_blkcipher_create_ariaofb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424ofbcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_ariaofb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_aria_ba424(c, key, iv, &ba424ofbcfg,
        ba424ofbcfg.decr);
}


static int sx_aead_create_ariagcm(struct sxaead *c, const struct sxkeyref *key,
    const char *iv, const int dir)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba424 = ~0u;
    unsigned int mode_compatibleba424 = ~0u;
    uint32_t hwreqs;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key))
        if (sx_aes_keysz((key)->sz) == ~0u)
            return SX_ERR_INVALID_KEY_SZ;

    if (compatibleba424 == ~0u)
        compatibleba424 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA424);

    hwreqs = (1 << ARIA_MODEID_GCM);
    mode_compatibleba424 = sx_cmdma_filter_compatible(compatibleba424,
            REG_BA424_CAPS, hwreqs);
    if (!mode_compatibleba424)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba424);
    c->cfg = &ba424gcmcfg;
    c->compatible = mode_compatibleba424;

    if (!c->dma.regs)
        return SX_ERR_RETRY;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
                CMDMA_AEAD_MODE_SET(c->cfg->mode) | key->cfg | dir,
                c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(key))
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    ADD_CFGDESC(c->dma, iv, SX_GCM_IV_SZ, c->cfg->dmatags->iv_or_state);

    /* Backup the IV in case we won't call HW when context saving */
    sx_memcpy(c->extramem + sizeof(c->extramem) - c->cfg->statesz, (char *)iv, SX_GCM_IV_SZ);

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


int sx_aead_create_ariagcm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *iv)
{
    return sx_aead_create_ariagcm(c, key, iv, 0);
}


int sx_aead_create_ariagcm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *iv)
{
    return sx_aead_create_ariagcm(c, key, iv, ba424gcmcfg.decr);
}


static int sx_aead_create_ariaccm(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz, size_t datasz,
    const uint32_t dir)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba424 = ~0u;
    unsigned int mode_compatibleba424 = ~0u;
    uint32_t hwreqs;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key))
        if (sx_aes_keysz((key)->sz) == ~0u)
            return SX_ERR_INVALID_KEY_SZ;
    if ((tagsz & 1) || (tagsz < 4) || (tagsz > 16))
        return SX_ERR_INVALID_TAG_SIZE;
    if ((noncesz < 7) || (noncesz > 13))
        return SX_ERR_INVALID_NONCE_SIZE;

    /* datasz must ensure  0 <= datasz < 2^(8L) */
    uint8_t l = 15 - noncesz;
    if ((l < 8U) && (datasz >= (1ULL << (l * 8)))) {
        /* message too long to encode the size in the CCM header */
        return SX_ERR_TOO_BIG;
    }

    if (compatibleba424 == ~0u)
        compatibleba424 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA424);

    hwreqs = (1 << ARIA_MODEID_CCM);
    mode_compatibleba424 = sx_cmdma_filter_compatible(compatibleba424,
            REG_BA424_CAPS, hwreqs);
    if (!mode_compatibleba424)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba424);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba424ccmcfg;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
            CMDMA_AEAD_MODE_SET(c->cfg->mode) | key->cfg | dir,
            c->cfg->dmatags->cfg);

    if (KEYREF_IS_USR(key))
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);

    // create header
    uint8_t *header = c->extramem;
    uint8_t headersz;
    int r = sx_aead_create_ccmheader(nonce, noncesz, tagsz, aadsz, datasz,
            header, &headersz);
    if (r != SX_OK)
        return r;

    /* Add first block of header as AAD */
    ADD_INDESC_PRIV_RAW(c->dma, OFFSET_EXTRAMEM(c), ARIA_BLOCK_SZ, c->cfg->dmatags->aad);

    c->totalaadsz = ARIA_BLOCK_SZ;
    c->discardaadsz = ARIA_BLOCK_SZ;
    c->datainsz = 0;
    c->dataintotalsz = 0;
    c->extraaadsz = 0;
    c->no_exec = 0;
    c->extraaadptr = c->extraaadmem;
    c->tagsz = tagsz;
    c->key = key;
    c->compatible = mode_compatibleba424;
    c->expectedtag = c->cfg->verifier;

    /* If remaining header, write it in extraadmem */
    headersz -= ARIA_BLOCK_SZ;
    if (headersz) {
        sx_memcpy(c->extraaadptr, header + ARIA_BLOCK_SZ, headersz);
        c->extraaadsz = headersz;
    }

    return SX_OK;
}


int sx_aead_create_ariaccm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz,
    size_t datasz)
{
    return sx_aead_create_ariaccm(c, key, nonce, noncesz, tagsz, aadsz, datasz,
            0);
}


int sx_aead_create_ariaccm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz,
    size_t datasz)
{
    return sx_aead_create_ariaccm(c, key, nonce, noncesz, tagsz, aadsz, datasz,
            ba424ccmcfg.decr);
}


static int sx_cmac_create_aria_ba424(struct sxmac *c, const struct sxkeyref *key)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba424 = ~0u;
    unsigned int mode_compatibleba424;
    uint32_t hwreqs;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (sx_aes_keysz((key)->sz) == ~0u)
            return SX_ERR_INVALID_KEY_SZ;
        hwreqs = sx_cmac_key_mask(key->sz);
    }

    if (compatibleba424 == ~0u)
        compatibleba424 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA419);

    hwreqs = (1 << ARIA_CMAC_MODEID_BA424);
    mode_compatibleba424 = sx_cmdma_filter_compatible(compatibleba424,
            REG_BA424_CAPS, hwreqs);
    if (!mode_compatibleba424)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba424);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba424cfg_cmac;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_CMAC_MODE_SET(ARIA_CMAC_MODEID_BA424) | key->cfg, c->cfg->dmatags->cfg);
    c->cntindescs = 1;
    if (KEYREF_IS_USR(key)) {
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
        c->cntindescs++;
    }
    c->feedsz = 0;
    c->macsz = ARIA_CMAC_MAC_SZ;
    c->key = key;
    c->compatible = mode_compatibleba424;

    return SX_OK;
}


int sx_mac_create_ariacmac(struct sxmac *c, const struct sxkeyref *key)
{
    return sx_cmac_create_aria_ba424(c, key);
}

