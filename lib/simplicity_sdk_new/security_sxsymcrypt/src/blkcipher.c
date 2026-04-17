/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/blkcipher.h"
#include "../include/sxsymcrypt/aes.h"
#include "../include/sxsymcrypt/keyref.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "blkcipherdefs.h"
#include "keyrefdefs.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "cmaes.h"


/** Mode Register value for context loading */
#define BLKCIPHER_MODEID_CTX_LOAD (1u << 4)
/** Mode Register value for context saving */
#define BLKCIPHER_MODEID_CTX_SAVE (1u << 5)

/** AES block cipher context saving state size, in bytes */
#define AES_BLKCIPHER_STATE_SZ (16)

static const struct sx_blkcipher_cmdma_tags ba411tags = {
    .cfg = DMATAG_BA411 | DMATAG_CONFIG(0),
    .iv_or_state = DMATAG_BA411 | DMATAG_CONFIG(0x28),
    .key = DMATAG_BA411 | DMATAG_CONFIG(0x08),
    .key2 = DMATAG_BA411 | DMATAG_CONFIG(0x48),
    .data = DMATAG_BA411
};

static const struct sx_blkcipher_cmdma_tags ba416tags = {
    .cfg = DMATAG_BA416 | DMATAG_CONFIG(0),
    .key = DMATAG_BA416 | DMATAG_CONFIG(0x20),
    .key2 = DMATAG_BA416 | DMATAG_CONFIG(0x40),
    .iv_or_state = DMATAG_BA416 | DMATAG_CONFIG(0x10) | DMATAG_LAST,
    .data = DMATAG_BA416 | DMATAG_LAST,
};


static const struct sx_blkcipher_cmdma_cfg ba411ecbcfg = {
    .decr = CM_CFG_DECRYPT,
    .dmatags = &ba411tags,
    .statesz = 0,
    .mode = BLKCIPHER_MODEID_ECB,
    .inminsz = 16,
    .granularity = 16,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba411cbccfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = BLKCIPHER_MODEID_CTX_SAVE,
    .ctxload = BLKCIPHER_MODEID_CTX_LOAD,
    .dmatags = &ba411tags,
    .statesz = AES_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CBC,
    .inminsz = 16,
    .granularity = 16,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba411ofbcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = BLKCIPHER_MODEID_CTX_SAVE,
    .ctxload = BLKCIPHER_MODEID_CTX_LOAD,
    .dmatags = &ba411tags,
    .statesz = AES_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_OFB,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba411cfbcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = BLKCIPHER_MODEID_CTX_SAVE,
    .ctxload = BLKCIPHER_MODEID_CTX_LOAD,
    .dmatags = &ba411tags,
    .statesz = AES_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CFB,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba411ctrcfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = BLKCIPHER_MODEID_CTX_SAVE,
    .ctxload = BLKCIPHER_MODEID_CTX_LOAD,
    .dmatags = &ba411tags,
    .statesz = AES_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_CTR,
    .inminsz = 1,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba411xtscfg = {
    .decr = CM_CFG_DECRYPT,
    .ctxsave = BLKCIPHER_MODEID_CTX_SAVE,
    .ctxload = BLKCIPHER_MODEID_CTX_LOAD,
    .dmatags = &ba411tags,
    .statesz = AES_BLKCIPHER_STATE_SZ,
    .mode = BLKCIPHER_MODEID_XTS,
    .inminsz = 16,
    .granularity = 1,
    .blocksz = BLKCIPHER_BLOCK_SZ,
};

static const struct sx_blkcipher_cmdma_cfg ba416xtscfg = {
    .decr = CM_CFG_DECRYPT,
    .dmatags = &ba416tags,
    .statesz = 0,
    .inminsz = 16,
    .granularity = 1,
};


static void sx_memcpy(void* dst, const void* src, size_t length)
{
    for (size_t i = 0; i < length; i++)
        ((uint8_t*) dst)[i] = ((uint8_t*) src)[i];
}


static void sx_blkcipher_free(struct sxblkcipher *c)
{
    sx_cmdma_release_hw(c->dma.regs);
    c->dma.regs = NULL;
}


/** Returns the bitmask for key size used for verifying HW capabilities. */
static uint32_t sx_blkcipher_key_mask(size_t keysz)
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


static int sx_blkcipher_create_aesxts(struct sxblkcipher *c,
    const struct sxkeyref *key1, const struct sxkeyref *key2,
    const char *iv, const struct sxaesparams *params)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba416 = ~0u;
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba411 = ~0u;
    uint32_t keyszfld = 0;
    uint32_t mode = 0;
    unsigned int mode_compatibleba411 = ~0u;
    uint32_t hwreqs = 0;
    uint32_t hw_temp_red = 0;
    c->dma.regs = NULL;

    if (KEYREF_IS_INVALID(key1) || KEYREF_IS_INVALID(key2))
        return SX_ERR_INVALID_KEYREF;
    /* BA411 XTS does support HW keys */
    if ( (KEYREF_IS_USR(key1) != KEYREF_IS_USR(key2))
        || (!KEYREF_IS_USR(key1) && (key2->cfg != (key1->cfg+1))) )
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key1)) {
        keyszfld = sx_aes_keysz(key1->sz);
        if ( (keyszfld == ~0u) || (key1->sz != key2->sz) )
            return SX_ERR_INVALID_KEY_SZ;
        hwreqs = sx_blkcipher_key_mask(key1->sz);

        /* BA416 can be used only with user provided keys that have 16 or 32 bytes. */
        if ((key1->sz != 24) && (!CM_AES_TEMPO_REDUNDANCY_IS_SET(params->config))) {
            if (compatibleba416 == ~0u)
                compatibleba416 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA416);
            c->dma.regs = sx_cmdma_find_available(compatibleba416);
            c->cfg = &ba416xtscfg;
        }
    }

    if (!c->dma.regs) {
        if (compatibleba411 == ~0u) {
            compatibleba411 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA411);
        }

        hwreqs |= (1 << BLKCIPHER_MODEID_XTS);
        mode_compatibleba411 = sx_cmdma_filter_compatible(compatibleba411,
                REG_BA411_CAPS, hwreqs);

        if (CM_AES_TEMPO_REDUNDANCY_IS_SET(params->config)) {
            hw_temp_red = CM_TEMPORAL_REDUNDANCY_CFG_EN;
            hwreqs = CM_TEMPORAL_REDUNDANCY_CFG2_EN;
            mode_compatibleba411 = sx_cmdma_filter_compatible(mode_compatibleba411,
                REG_BA411_CTR_SZ, hwreqs);
        }

        c->dma.regs = sx_cmdma_find_available(mode_compatibleba411);
        c->cfg = &ba411xtscfg;
        mode = CMDMA_BLKCIPHER_MODE_SET(BLKCIPHER_MODEID_XTS);
        keyszfld = 0;
        c->compatible = mode_compatibleba411;
    }

    if (((!compatibleba416) || (compatibleba416 && !c->dma.regs)) &&
        (!mode_compatibleba411))
        return SX_ERR_INCOMPATIBLE_HW;

    if (!c->dma.regs)
        return SX_ERR_RETRY;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        KEYREF_AES_HWKEY_CONF(key1->cfg) | mode | keyszfld | hw_temp_red,
        c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(key1)) {
        ADD_CFGDESC(c->dma, key1->key, key1->sz, c->cfg->dmatags->key);
        ADD_CFGDESC(c->dma, key2->key, key2->sz, c->cfg->dmatags->key2);
    }
    ADD_CFGDESC(c->dma, iv, 16, c->cfg->dmatags->iv_or_state);

    c->key = key1;
    c->textsz = 0;
    c->is_multifeed = 0;
    c->extradatasz = 0;

    return SX_OK;
}


int sx_blkcipher_create_aesxts_enc(struct sxblkcipher *c,
    const struct sxkeyref *key1, const struct sxkeyref *key2, const char *iv)
{
    int r;
    struct sxaesparams params = {.config = 0};
    r = sx_blkcipher_create_aesxts(c, key1, key2, iv, &params);
    if (r != SX_OK)
        return r;

    c->dma.dmamem.cfg |= CM_CFG_ENCRYPT;

    return SX_OK;
}


int sx_blkcipher_create_aesxts_dec(struct sxblkcipher *c,
    const struct sxkeyref *key1, const struct sxkeyref *key2, const char *iv)
{
    int r;
    struct sxaesparams params = {.config = 0};
    r = sx_blkcipher_create_aesxts(c, key1, key2, iv, &params);
    if (r != SX_OK)
        return r;

    c->dma.dmamem.cfg |= c->cfg->decr;

    return SX_OK;
}


static int sx_blkcipher_create_aes_ba411(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv,
    const struct sx_blkcipher_cmdma_cfg *cfg,
    const uint32_t dir, const struct sxaesparams *params)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba411 = ~0u;
    unsigned int mode_compatibleba411;
    uint32_t hw_temp_red = 0;
    uint32_t hwreqs = 0;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (sx_aes_keysz((key)->sz) == ~0u)
            return SX_ERR_INVALID_KEY_SZ;
        hwreqs = sx_blkcipher_key_mask(key->sz);
    }

    if (compatibleba411 == ~0u) {
        compatibleba411 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA411);
    }

    hwreqs |= (1 << cfg->mode);
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

    c->cfg = cfg;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_BLKCIPHER_MODE_SET(cfg->mode) | KEYREF_AES_HWKEY_CONF(key->cfg) | dir | hw_temp_red,
        c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(key))
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    if (iv != NULL)
        ADD_CFGDESC(c->dma, iv, 16, c->cfg->dmatags->iv_or_state);
    c->key = key;
    c->compatible = mode_compatibleba411;
    c->textsz = 0;
    c->is_multifeed = 0;
    c->outputsz_bkp = 0;
    c->extradatasz = 0;
    c->extradataptr = c->extradata;
    c->iv_loaded = 0;

    if ((c->cfg->inminsz == 1) || (c->cfg->granularity == 1))
        sx_memcpy(c->state_bkp, iv, 16);

    return SX_OK;
}


int sx_blkcipher_create_aesctr_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411ctrcfg,
        CM_CFG_ENCRYPT, &params);
}


int sx_blkcipher_create_aesctr_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411ctrcfg,
        ba411ctrcfg.decr, &params);
}


int sx_blkcipher_create_aesecb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, NULL, &ba411ecbcfg,
        CM_CFG_ENCRYPT, &params);
}


int sx_blkcipher_create_aesecb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, NULL, &ba411ecbcfg,
        ba411ecbcfg.decr, &params);
}


int sx_blkcipher_create_aescbc_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411cbccfg,
        CM_CFG_ENCRYPT, &params);
}


int sx_blkcipher_create_aescbc_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411cbccfg,
        ba411cbccfg.decr, &params);
}


int sx_blkcipher_create_aescfb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411cfbcfg,
        CM_CFG_ENCRYPT, &params);
}


int sx_blkcipher_create_aescfb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411cfbcfg,
        ba411cfbcfg.decr, &params);
}


int sx_blkcipher_create_aesofb_enc(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411ofbcfg,
        CM_CFG_ENCRYPT, &params);
}


int sx_blkcipher_create_aesofb_dec(struct sxblkcipher *c,
    const struct sxkeyref *key, const char *iv)
{
    struct sxaesparams params = {.config = 0};
    return sx_blkcipher_create_aes_ba411(c, key, iv, &ba411ofbcfg,
        ba411ofbcfg.decr, &params);
}


int sx_blkcipher_crypt(struct sxblkcipher *c, const char *datain, size_t sz,
    char *dataout)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (sz >= DMA_MAX_SZ) {
        sx_blkcipher_free(c);
        return SX_ERR_TOO_BIG;
    }
    if (c->is_multifeed) {
        sx_blkcipher_free(c);
        return SX_ERR_FEED_AFTER_FEED;
    }

    c->textsz += sz;
    c->datain_bkp = datain;
    c->dataout_bkp = dataout;
    /* The general approach is to add Input and then Output descriptors.
     * In this particular case we do it the other way around because
     * the SET_LAST_DESC_IGN sets the last descriptor from the list. Also,
     * the descriptors list is constructed in the order of insertion,
     * so it will have a mix of Input/Output descriptors. By doing
     * this swap we ensure the last descriptor is the Input that needs
     * to be altered. */
    ADD_RAW_OUTDESC(c->dma, dataout, sz);
    ADD_RAW_INDESC(c->dma, datain, sz, c->cfg->dmatags->data);

    return SX_OK;
}


int sx_blkcipher_multifeed_crypt(struct sxblkcipher *c,
                                 const struct sxdataref *datain, size_t countdatain,
                                 const struct sxdataref *dataout, size_t countdataout)
{
    size_t outputsz = 0;
    size_t extrablocksz;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if ((c->cfg->dmatags->cfg & DMATAG_BA416) == DMATAG_BA416) {
        sx_blkcipher_free(c);
        return SX_ERR_INCOMPATIBLE_HW;
    }
    if (c->textsz) {
        sx_blkcipher_free(c);
        return SX_ERR_FEED_AFTER_FEED;
    }
    if ((countdatain > SX_BLKCIPHER_EXTRA_DESC_IN + 1) ||
        (countdataout > SX_BLKCIPHER_EXTRA_DESC_OUT + 1)) {
        sx_blkcipher_free(c);
        return SX_ERR_FEED_COUNT_EXCEEDED;
    }
    if ((!countdatain) || (!countdataout)) {
        sx_blkcipher_free(c);
        return SX_ERR_TOO_SMALL;
    }

    /* Set input descriptors */
    for (size_t i = 0; i < countdatain; i++) {
        if (datain[i].sz) {
            if (datain[i].sz >= DMA_MAX_SZ) {
                sx_blkcipher_free(c);
                return SX_ERR_TOO_BIG;
            }
            ADD_RAW_INDESC(c->dma, datain[i].data, datain[i].sz, c->cfg->dmatags->data);
            c->textsz += datain[i].sz;
        }
    }
    SET_LAST_DESC_IGN(c->dma, c->textsz, 0x0F);

    /* Set output descriptors */
    for (size_t i = 0; i < countdataout; i++) {
        if (dataout[i].sz) {
            if (dataout[i].sz >= DMA_MAX_SZ) {
                sx_blkcipher_free(c);
                return SX_ERR_TOO_BIG;
            }
            ADD_RAW_OUTDESC(c->dma, dataout[i].data, dataout[i].sz);
            outputsz += dataout[i].sz;
        }
    }
    extrablocksz = outputsz & 0xF;
    if (extrablocksz)
        ADD_DISCARDDESC(c->dma, 0x10 - extrablocksz);

    /* Check that input and output size match */
    if (outputsz != c->textsz) {
        sx_blkcipher_free(c);
        return SX_ERR_FEED_UNBALANCED;
    }

    c->is_multifeed = 1;

    return SX_OK;
}


int sx_blkcipher_run(struct sxblkcipher *c)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if (c->textsz < c->cfg->inminsz) {
        sx_blkcipher_free(c);
        return SX_ERR_TOO_SMALL;
    }
    if (c->textsz % c->cfg->granularity) {
        sx_blkcipher_free(c);
        return SX_ERR_WRONG_SIZE_GRANULARITY;
    }

    if (c->dma.dmamem.cfg & c->cfg->ctxsave)
        c->dma.dmamem.cfg &= ~c->cfg->ctxsave;

    if (!c->is_multifeed) {
        /* We need to set the ignore. If multifeed was used,
         * this is already done. */
        if (ALIGN_SZA(c->textsz, 0xf) - c->textsz) {
            SET_LAST_DESC_IGN(c->dma, c->textsz, 0x0F);
            ADD_DISCARDDESC(c->dma, ALIGN_SZA(c->textsz, 0xf) - c->textsz);
        }
    }
    sx_cmdma_start(&c->dma,  sizeof(c->descs) + sizeof(c->extramem), c->descs);

    return SX_OK;
}


int sx_blkcipher_resume_state(struct sxblkcipher *c)
{
    if (c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if (c->cfg->statesz == 0)
        return SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED;

    c->dma.regs = sx_cmdma_find_available(c->compatible);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->dma.dmamem.cfg &= ~(c->cfg->ctxsave);
    c->dma.dmamem.cfg |= c->cfg->ctxload;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs), c->dma.dmamem.cfg,
        c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(c->key))
        ADD_CFGDESC(c->dma, c->key->key, c->key->sz, c->cfg->dmatags->key);
    /* Context will be transfered in the same place as the IV. However,
     * we cannot use same approach as for IV because context is stored in c
     * and needs to be added using ADD_INDESC_PRIV() */
    if (c->iv_loaded)
        ADD_INDESC_PRIV(c->dma, OFFSET_EXTRAMEM(c), 16, c->cfg->dmatags->iv_or_state);
    else
        ADD_CFGDESC(c->dma, c->state_bkp, 16, c->cfg->dmatags->iv_or_state);

    c->textsz = 0;
    c->is_multifeed = 0;
    c->outputsz_bkp = 0;

    /* Depending if we still have remaining data from previous call, we feed it now.
     * This data is discarded as it was already processed. */
    if (c->extradatasz) {
        ADD_RAW_INDESC(c->dma, c->extradataptr, c->extradatasz, c->cfg->dmatags->data);
        ADD_RAW_OUTDESC(c->dma, c->output_bkp, c->extradatasz);
        c->outputsz_bkp = c->extradatasz;
        c->textsz += c->extradatasz;
    }

    return SX_OK;
}


int sx_blkcipher_save_state(struct sxblkcipher *c)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if (c->cfg->statesz == 0) {
        sx_blkcipher_free(c);
        return SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED;
    }

    if ((c->cfg->inminsz != 1) || (c->cfg->granularity != 1) ||
        (c->cfg->blocksz == 64)) {
        /* If we get here it means we are not dealing with streamcipher or
         * we are doing a ChaCha20 which does not support context saving with
         * any size*/
        if (c->textsz < c->cfg->blocksz) {
            sx_blkcipher_free(c);
            return SX_ERR_TOO_SMALL;
        }
        if (c->textsz & (c->cfg->blocksz - 1)) {
            sx_blkcipher_free(c);
            return SX_ERR_WRONG_SIZE_GRANULARITY;
        }
        c->iv_loaded = 1;
    } else {
        /* If we get here it means we are dealing with streamcipher */
        if (c->textsz & (c->cfg->blocksz - 1)) {
            /* Total data fed not multiple of block size */
            size_t discardsz = ALIGN_SZA(c->textsz, 0xf) - c->textsz;
            /* Fill the buffer up to block size and discard the output for it */
            ADD_RAW_INDESC(c->dma, c->extradata, discardsz, c->cfg->dmatags->data);
            ADD_DISCARDDESC(c->dma, discardsz);

            size_t insz = c->textsz - c->extradatasz;

            /* Backup extra data to be used with the next calls */
            if (c->textsz < c->cfg->blocksz) {
                /* We have not passed a block yet, need to append */
                sx_memcpy(&c->extradataptr[c->extradatasz], c->datain_bkp, insz);
                c->extradatasz += insz;
                /* backup state */
                if (c->iv_loaded)
                    sx_memcpy(c->state_bkp, c->extramem, c->cfg->statesz);
            } else {
                /* If we get here it means at least one full block was processed at any time. */
                c->iv_loaded = 1;

                /* Toggle extradata memory pointer as we need previously stored data to be
                 * valid until HW call */
                c->extradataptr += SX_BLKCIPHER_PRIV_SZ - 2 * (c->extradataptr - c->extradata);
                c->extradatasz = c->textsz & 0xF;
                c->offset = insz - c->extradatasz;

                sx_memcpy(&c->extradataptr[0], c->datain_bkp + c->offset, c->extradatasz);
            }
        } else {
            /* If we get here it means at least one full block was processed at any time. */
            c->iv_loaded = 1;
            c->extradatasz = 0;
        }
    }
    c->dma.dmamem.cfg |= c->cfg->ctxsave;

    /* Saving the state */
    ADD_OUTDESC_PRIV(c->dma, OFFSET_EXTRAMEM(c), 16, 0x0F);

    sx_cmdma_start(&c->dma,  sizeof(c->descs) + sizeof(c->extramem), c->descs);

    return SX_OK;
}


static void sx_blkcipher_ctr_decrease(uint8_t *ctr)
{
    for (size_t i = SX_BLKCIPHER_PRIV_SZ - 1; i > 0; i--) {
        if (--ctr[i] != 0xFF)
            break;
    }
}


static void sx_blkcipher_xor(uint8_t *dst, const uint8_t *a, const uint8_t *b, size_t sz)
{
    for (size_t i = 0; i < sz; i++)
        dst[i] = a[i] ^ b[i];
}


static int sx_blkcipher_reload_state(struct sxblkcipher *c)
{
    int r = SX_OK;

    /* Each mode has it's unique way of reloading the state:
     * - CTR - counter--
     * - OFB - ciphertext(n) xor plaintext(n)
     * - CFB - ciphertext(n)
     */
    switch (c->cfg->mode) {
    case BLKCIPHER_MODEID_CTR:
        sx_blkcipher_ctr_decrease((uint8_t *)c->extramem);
        break;
    case BLKCIPHER_MODEID_OFB:
        if ((c->iv_loaded)) {
            if (c->textsz > 2 * c->cfg->blocksz) {
                sx_blkcipher_xor((uint8_t *)c->extramem, (uint8_t *)c->datain_bkp + c->offset - 16,
                                 (uint8_t *)c->dataout_bkp + c->offset - 16, SX_BLKCIPHER_PRIV_SZ);
            } else if (c->textsz > c->cfg->blocksz) {
                /* Toggle extradata memory pointer as we need previously stored data to be
                 * valid until HW call */
                char* extradataptr = c->extradataptr + SX_BLKCIPHER_PRIV_SZ - 2 * (c->extradataptr - c->extradata);
                sx_blkcipher_xor((uint8_t *)c->extramem, (uint8_t *)c->output_bkp,
                                 (uint8_t *)extradataptr, c->outputsz_bkp);
                sx_blkcipher_xor((uint8_t *)c->extramem + c->outputsz_bkp, (uint8_t *)c->dataout_bkp,
                                 (uint8_t *)c->datain_bkp, SX_BLKCIPHER_PRIV_SZ - c->outputsz_bkp);
            } else {
                sx_memcpy((uint8_t *)c->extramem, (uint8_t *)c->state_bkp, 16);
            }
        }
        break;
    case BLKCIPHER_MODEID_CFB:
        if (c->iv_loaded) {
            const char *ciphertext;
            if ((c->dma.dmamem.cfg & 0x01) == CM_CFG_DECRYPT)
                ciphertext = c->datain_bkp;
            else
                ciphertext = c->dataout_bkp;
            if (c->textsz > 2 * c->cfg->blocksz) {
                sx_memcpy((uint8_t *)c->extramem,
                          (uint8_t *)ciphertext + c->offset - 16,
                          SX_BLKCIPHER_PRIV_SZ);
            } else if (c->textsz > c->cfg->blocksz) {
                if ((c->dma.dmamem.cfg & 0x01) == CM_CFG_DECRYPT) {
                    char *extradataptr = c->extradataptr + SX_BLKCIPHER_PRIV_SZ -
                                         2 * (c->extradataptr - c->extradata);
                    sx_memcpy((uint8_t *)c->extramem, (uint8_t *)extradataptr, c->outputsz_bkp);
                } else {
                    sx_memcpy((uint8_t *)c->extramem, (uint8_t *)c->output_bkp, c->outputsz_bkp);
                }
                sx_memcpy((uint8_t *)c->extramem + c->outputsz_bkp,
                          (uint8_t *)ciphertext,
                          SX_BLKCIPHER_PRIV_SZ - c->outputsz_bkp);
            } else {
                sx_memcpy((uint8_t *)c->extramem, (uint8_t *)c->state_bkp, 16);
            }
        }
        break;
    default:
        r = SX_ERR_INCOMPATIBLE_HW;
        break;
    }

    return r;
}


int sx_blkcipher_status(struct sxblkcipher *c)
{
    int r;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    r = sx_cmdma_check(&c->dma);
    if (r != SX_ERR_HW_PROCESSING) {
        sx_flush_fromhw(c->dma.regs, (char*) &c->extramem, OFFSET_EXTRAMEM(c),
            c->cfg->statesz);
        if (c->extradatasz)
            r = sx_blkcipher_reload_state(c);

        sx_blkcipher_free(c);
    }

    return r;
}


int sx_blkcipher_wait(struct sxblkcipher *c)
{
    int r = SX_ERR_HW_PROCESSING;

    while (r == SX_ERR_HW_PROCESSING) {
        sx_cmdma_wait(c->dma.regs);
        r = sx_blkcipher_status(c);
    }

    return r;
}


int sx_blkcipher_create_aes_generic(struct sxblkcipher *c,
    const struct sxkeyref *key1, const struct sxkeyref *key2,
    const char *iv, const char mode, const uint32_t dir,
    const struct sxaesparams *params)
{
    int r = SX_OK;
    uint32_t enc_dec;

    switch (mode) {
    case BLKCIPHER_MODEID_ECB:
        enc_dec = (dir == CM_CFG_ENCRYPT) ? CM_CFG_ENCRYPT : ba411ecbcfg.decr;
        r = sx_blkcipher_create_aes_ba411(c, key1, iv, &ba411ecbcfg,
                enc_dec, params);
        break;
    case BLKCIPHER_MODEID_CBC:
        enc_dec = (dir == CM_CFG_ENCRYPT) ? CM_CFG_ENCRYPT : ba411cbccfg.decr;
        r = sx_blkcipher_create_aes_ba411(c, key1, iv, &ba411cbccfg,
                enc_dec, params);
        break;
    case BLKCIPHER_MODEID_CTR:
        enc_dec = (dir == CM_CFG_ENCRYPT) ? CM_CFG_ENCRYPT : ba411ctrcfg.decr;
        r = sx_blkcipher_create_aes_ba411(c, key1, iv, &ba411ctrcfg,
                enc_dec, params);
        break;
    case BLKCIPHER_MODEID_CFB:
        enc_dec = (dir == CM_CFG_ENCRYPT) ? CM_CFG_ENCRYPT : ba411cfbcfg.decr;
        r = sx_blkcipher_create_aes_ba411(c, key1, iv, &ba411cfbcfg,
                enc_dec, params);
        break;
    case BLKCIPHER_MODEID_OFB:
        enc_dec = (dir == CM_CFG_ENCRYPT) ? CM_CFG_ENCRYPT : ba411ofbcfg.decr;
        r = sx_blkcipher_create_aes_ba411(c, key1, iv, &ba411ofbcfg,
                enc_dec, params);
        break;
    case BLKCIPHER_MODEID_XTS:
        r = sx_blkcipher_create_aesxts(c, key1, key2, iv, params);
        if (dir == CM_CFG_ENCRYPT)
            c->dma.dmamem.cfg |= dir;
        else
            c->dma.dmamem.cfg |= c->cfg->decr;
        break;
    default:
        r = SX_ERR_INCOMPATIBLE_HW;
        break;
    }

    return r;
}
