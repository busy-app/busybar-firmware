/*
 * @Copyright 2024 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/sm4.h"
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

/* Size of the key used in SM4 */
#define SX_BLKCIPHER_SM4_KEYSZ 16

#define CMDMA_BA419_BUS_MSK (0x0F)
#define SM4_CMAC_MODEID_BA419 8
#define SM4_CMAC_BLOCK_SZ 16
#define SM4_CMAC_STATE_SZ 16
#define SM4_CMAC_MAC_SZ 16

/** Mode Register value for context loading */
#define SM4_MODEID_CTX_LOAD (1u << 4)
/** Mode Register value for context saving */
#define SM4_MODEID_CTX_SAVE (1u << 5)

/** SM4 block size, in bytes */
#define SM4_BLOCK_SZ (16)
/** SM4 GCM and CCM context saving state size, in bytes */
#define SM4_AEAD_CTX_STATE_SZ (32)
/** SM4 block cipher context saving state size, in bytes */
#define SM4_BLKCIPHER_STATE_SZ (16)

extern int sx_aead_create_ccmheader(const char *nonce, size_t noncesz,
        uint8_t tagsz, uint64_t aadsz, uint64_t datasz, uint8_t *header,
        uint8_t *headersz);
extern void set_nonce_gcm(struct sxaead *c);
extern void set_nonce_ccm(struct sxaead *c);

static int lenAlenC_nop(size_t aadsz, size_t datasz, uint8_t *out);
static int lenAlenC_sm4gcm_ba419(size_t aadsz, size_t datasz, uint8_t *out);


static const struct sx_blkcipher_cmdma_tags ba419tags = {
    .cfg = DMATAG_BA419 | DMATAG_CONFIG(0),
    .key = DMATAG_BA419 | DMATAG_CONFIG(0x08),
    .key2 = DMATAG_BA419 | DMATAG_CONFIG(0x48),
    .iv_or_state = DMATAG_BA419 | DMATAG_CONFIG(0x28),
    .data = DMATAG_BA419
};


static const struct sx_blkcipher_cmdma_cfg ba419ecbcfg = {
    .decr = CM_CFG_DECRYPT,
    .dmatags = &ba419tags,
    .statesz = 0,
    .mode = BLKCIPHER_MODEID_ECB,
    .inminsz = 16,
    .granularity = 16,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba419cbccfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = SM4_MODEID_CTX_SAVE,
    .ctxload = SM4_MODEID_CTX_LOAD,
    .dmatags = &ba419tags,
    .statesz = SM4_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CBC,
    .inminsz = 16,
    .granularity = 16,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba419ofbcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = SM4_MODEID_CTX_SAVE,
    .ctxload = SM4_MODEID_CTX_LOAD,
    .dmatags = &ba419tags,
    .statesz = SM4_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_OFB,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba419cfbcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = SM4_MODEID_CTX_SAVE,
    .ctxload = SM4_MODEID_CTX_LOAD,
    .dmatags = &ba419tags,
    .statesz = SM4_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CFB,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba419ctrcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = SM4_MODEID_CTX_SAVE,
    .ctxload = SM4_MODEID_CTX_LOAD,
    .dmatags = &ba419tags,
    .statesz = SM4_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CTR,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba419xtscfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = SM4_MODEID_CTX_SAVE,
    .ctxload = SM4_MODEID_CTX_LOAD,
    .dmatags = &ba419tags,
    .statesz = SM4_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_XTS,
    .inminsz = 16,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_aead_cmdma_tags ba419aeadtags = {
    .cfg = DMATAG_BA419 | DMATAG_CONFIG(0),
    .iv_or_state = DMATAG_BA419 | DMATAG_CONFIG(0x28),
    .key = DMATAG_BA419 | DMATAG_CONFIG(0x08),
    .aad = DMATAG_BA419 | DMATAG_DATATYPE_HEADER,
    .tag = DMATAG_BA419,
    .data = DMATAG_BA419
};

#define SM4_MODEID_GCM 6
static const struct sx_aead_cmdma_cfg ba419gcmcfg = {
    .decr = CM_CFG_DECRYPT,
    .mode = SM4_MODEID_GCM,
    .dmatags = &ba419aeadtags,
    .verifier = NULL,
    .lenAlenC = lenAlenC_sm4gcm_ba419,
    .set_nonce = set_nonce_gcm,
    .ctxsave = SM4_MODEID_CTX_SAVE,
    .ctxload = SM4_MODEID_CTX_LOAD,
    .granularity = SM4_BLOCK_SZ,
    .statesz = SM4_AEAD_CTX_STATE_SZ,
    .inputminsz = 0,
    .tagminsz = 1,
    .hwtagverif = 0
};


#define SM4_MODEID_CCM 5
const struct sx_aead_cmdma_cfg ba419ccmcfg = {
    .decr = CM_CFG_DECRYPT,
    .mode = SM4_MODEID_CCM,
    .dmatags = &ba419aeadtags,
    .verifier = NULL,
    .lenAlenC = lenAlenC_nop,
    .set_nonce = set_nonce_ccm,
    .ctxsave = SM4_MODEID_CTX_SAVE,
    .ctxload = SM4_MODEID_CTX_LOAD,
    .granularity = SM4_BLOCK_SZ,
    .statesz = SM4_AEAD_CTX_STATE_SZ,
    .inputminsz = 0,
    .tagminsz = 4,
    .hwtagverif = 0
};


static const struct sx_mac_cmdma_tags ba419tags_cmac = {
    .cfg = DMATAG_BA419 | DMATAG_CONFIG(0),
    .state = DMATAG_BA419 | DMATAG_CONFIG(0x28),
    .key = DMATAG_BA419 | DMATAG_CONFIG(0x08),
    .data = DMATAG_BA419
};


static const struct sx_mac_cmdma_cfg ba419cfg_cmac = {
    .cmdma_mask = CMDMA_BA419_BUS_MSK,
    .granularity = SM4_CMAC_BLOCK_SZ,
    .blocksz = SM4_CMAC_BLOCK_SZ,
    .statesz = SM4_CMAC_STATE_SZ,
    .savestate = SM4_MODEID_CTX_SAVE,
    .loadstate = SM4_MODEID_CTX_LOAD,
    .dmatags = &ba419tags_cmac,
};

int lenAlenC_nop(size_t aadsz, size_t datasz, uint8_t *out)
{
    (void)aadsz;
    (void)datasz;
    (void)out;

    return 0;
}


static void sx_memcpy(void* dst, void* src, size_t length)
{
    for (size_t i = 0; i < length; i++)
        ((uint8_t*) dst)[i] = ((uint8_t*) src)[i];
}


static int lenAlenC_sm4gcm_ba419(size_t aadsz, size_t datasz, uint8_t *out)
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


static int sx_blkcipher_newcmd(struct sxblkcipher *c,
    const struct sxkeyref *key, const struct sx_blkcipher_cmdma_cfg *cfg,
    const uint32_t dir)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba419 = ~0u;
    unsigned int mode_compatibleba419;

    if (compatibleba419 == ~0u)
        compatibleba419 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA419);

    mode_compatibleba419 = sx_cmdma_filter_compatible(compatibleba419,
            REG_BA419_CAPS, 1 << cfg->mode);
    if (!mode_compatibleba419)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba419);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = cfg;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_BLKCIPHER_MODE_SET(cfg->mode) | dir |
        KEYREF_SM4_HWKEY_CONF(key->cfg),
        c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(key))
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);

    c->key = key;
    c->compatible = mode_compatibleba419;
    c->textsz = 0;
    c->is_multifeed = 0;
    c->extradatasz = 0;
    c->extradataptr = c->extradata;
    c->outputsz_bkp = 0;
    c->iv_loaded = 0;

    return SX_OK;
}


static int sx_blkcipher_create_sm4_ba419(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv,
    const struct sx_blkcipher_cmdma_cfg *cfg, const uint32_t dir)
{
    int r;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key))
        if (key->sz != SX_BLKCIPHER_SM4_KEYSZ)
            return SX_ERR_INVALID_KEY_SZ;

    r = sx_blkcipher_newcmd(c, key, cfg, dir);
    if (r != SX_OK)
        return r;

    if (iv != NULL)
        ADD_CFGDESC(c->dma, iv, 16, c->cfg->dmatags->iv_or_state);

    return SX_OK;
}


int sx_blkcipher_create_sm4ctr_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419ctrcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_sm4ctr_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419ctrcfg,
        ba419ctrcfg.decr);
}


int sx_blkcipher_create_sm4ecb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key)
{
    return sx_blkcipher_create_sm4_ba419(c, key, NULL, &ba419ecbcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_sm4ecb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key)
{
    return sx_blkcipher_create_sm4_ba419(c, key, NULL, &ba419ecbcfg,
        ba419ecbcfg.decr);
}


int sx_blkcipher_create_sm4cbc_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419cbccfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_sm4cbc_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419cbccfg,
        ba419cbccfg.decr);
}


int sx_blkcipher_create_sm4cfb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419cfbcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_sm4cfb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419cfbcfg,
        ba419cfbcfg.decr);
}


int sx_blkcipher_create_sm4ofb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419ofbcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_sm4ofb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    return sx_blkcipher_create_sm4_ba419(c, key, iv, &ba419ofbcfg,
        ba419ofbcfg.decr);
}


int sx_aead_create_sm4gcm(struct sxaead *c, const struct sxkeyref *key,
    const char *iv, const int dir)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba419 = ~0u;
    unsigned int mode_compatibleba419 = ~0u;
    uint32_t hwreqs;

    c->dma.regs = NULL;
    c->compatible = 0;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key))
        if (key->sz != SX_BLKCIPHER_SM4_KEYSZ)
            return SX_ERR_INVALID_KEY_SZ;

    if (compatibleba419 == ~0u) {
        compatibleba419 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA419);
    }

    hwreqs = (1 << SM4_MODEID_GCM);
    mode_compatibleba419 = sx_cmdma_filter_compatible(compatibleba419,
            REG_BA419_CAPS, hwreqs);
    if (!mode_compatibleba419)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba419);
    c->cfg = &ba419gcmcfg;
    c->compatible = mode_compatibleba419;

    if (!c->dma.regs)
        return SX_ERR_RETRY;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_AEAD_MODE_SET(c->cfg->mode) | dir |
        KEYREF_SM4_HWKEY_CONF(key->cfg),
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


int sx_aead_create_sm4gcm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *iv)
{
    return sx_aead_create_sm4gcm(c, key, iv, 0);
}


int sx_aead_create_sm4gcm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *iv)
{
    return sx_aead_create_sm4gcm(c, key, iv, ba419gcmcfg.decr);
}


static int sx_aead_create_sm4ccm(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz, size_t datasz,
    const uint32_t dir)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba419 = ~0u;
    unsigned int mode_compatibleba419 = ~0u;
    uint32_t hwreqs = 0;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key))
        if (key->sz != SX_BLKCIPHER_SM4_KEYSZ)
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

    if (compatibleba419 == ~0u) {
        compatibleba419 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA419);
    }

    hwreqs |= (1 << SM4_MODEID_CCM);
    mode_compatibleba419 = sx_cmdma_filter_compatible(compatibleba419,
            REG_BA419_CAPS, hwreqs);
    if (!mode_compatibleba419)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba419);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba419ccmcfg;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_AEAD_MODE_SET(c->cfg->mode) | dir |
        KEYREF_SM4_HWKEY_CONF(key->cfg),
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
    ADD_INDESC_PRIV_RAW(c->dma, OFFSET_EXTRAMEM(c), SM4_BLOCK_SZ, c->cfg->dmatags->aad);

    c->totalaadsz = SM4_BLOCK_SZ;
    c->discardaadsz = SM4_BLOCK_SZ;
    c->datainsz = 0;
    c->dataintotalsz = 0;
    c->extraaadsz = 0;
    c->no_exec = 0;
    c->extraaadptr = c->extraaadmem;
    c->tagsz = tagsz;
    c->key = key;
    c->compatible = mode_compatibleba419;
    c->expectedtag = c->cfg->verifier;

    /* If remaining header, write it in extraadmem */
    headersz -= SM4_BLOCK_SZ;
    if (headersz) {
        sx_memcpy(c->extraaadptr, header + SM4_BLOCK_SZ, headersz);
        c->extraaadsz = headersz;
    }

    return SX_OK;
}


int sx_aead_create_sm4ccm_enc(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz,
    size_t datasz)
{
    return sx_aead_create_sm4ccm(c, key, nonce, noncesz, tagsz, aadsz, datasz,
            0);
}


int sx_aead_create_sm4ccm_dec(struct sxaead *c, const struct sxkeyref *key,
    const char *nonce, size_t noncesz, size_t tagsz, size_t aadsz,
    size_t datasz)
{
    return sx_aead_create_sm4ccm(c, key, nonce, noncesz, tagsz, aadsz, datasz,
            ba419ccmcfg.decr);
}


static int sx_blkcipher_create_sm4xts(struct sxblkcipher *c,
    const struct sxkeyref *key1, const struct sxkeyref *key2, const char *iv,
    const uint32_t dir)
{
    int r;

    if (KEYREF_IS_INVALID(key1) || KEYREF_IS_INVALID(key2))
        return SX_ERR_INVALID_KEYREF;
    if ((KEYREF_IS_USR(key1) != KEYREF_IS_USR(key2)) ||
        (!KEYREF_IS_USR(key1) && (key2->cfg != (key1->cfg+1))))
        return SX_ERR_INVALID_KEYREF;
    if ((KEYREF_IS_USR(key1)) &&
        ((key1->sz != key2->sz) || (key1->sz != SX_BLKCIPHER_SM4_KEYSZ)))
        return SX_ERR_INVALID_KEY_SZ;

    r = sx_blkcipher_newcmd(c, key1, &ba419xtscfg, dir);
    if (r != SX_OK)
        return r;
    if (KEYREF_IS_USR(key2))
        ADD_CFGDESC(c->dma, key2->key, key2->sz, c->cfg->dmatags->key2);

    ADD_CFGDESC(c->dma, iv, 16, c->cfg->dmatags->iv_or_state);

    return SX_OK;
}


int sx_blkcipher_create_sm4xts_enc(struct sxblkcipher *c,
    const struct sxkeyref *key1, const struct sxkeyref *key2, const char *iv)
{
    return sx_blkcipher_create_sm4xts(c, key1, key2, iv, CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_sm4xts_dec(struct sxblkcipher *c,
    const struct sxkeyref *key1, const struct sxkeyref *key2, const char *iv)
{
    return sx_blkcipher_create_sm4xts(c, key1, key2, iv, ba419xtscfg.decr);
}


static int sx_cmac_create_sm4_ba419(struct sxmac *c, const struct sxkeyref *key)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba419 = ~0u;
    unsigned int mode_compatibleba419;
    uint32_t hwreqs = 0;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (key->sz != SX_BLKCIPHER_SM4_KEYSZ)
            return SX_ERR_INVALID_KEY_SZ;
    }

    if (compatibleba419 == ~0u) {
        compatibleba419 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA419);
    }

    hwreqs |= (1 << SM4_CMAC_MODEID_BA419);
    mode_compatibleba419 = sx_cmdma_filter_compatible(compatibleba419,
            REG_BA419_CAPS, hwreqs);
    if (!mode_compatibleba419)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(mode_compatibleba419);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba419cfg_cmac;
    c->cntindescs = 1;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_CMAC_MODE_SET(SM4_CMAC_MODEID_BA419) |
        KEYREF_SM4_HWKEY_CONF(key->cfg),
        c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(key)) {
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
        c->cntindescs++;
    }
    c->feedsz = 0;
    c->macsz = SM4_CMAC_MAC_SZ;
    c->key = key;
    c->compatible = mode_compatibleba419;

    return SX_OK;
}


int sx_mac_create_sm4cmac(struct sxmac *c, const struct sxkeyref *key)
{
    return sx_cmac_create_sm4_ba419(c, key);
}

