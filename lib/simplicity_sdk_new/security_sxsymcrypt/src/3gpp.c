/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/3gpp.h"
#include "../include/sxsymcrypt/keyref.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include <stddef.h>
#include "keyrefdefs.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "cmaes.h"


#define SNOW3G_CONFIDENTIALITY_MODEID (0u)
#define ZUC_CONFIDENTIALITY_MODEID    (0u)
#define KASUMI_CONFIDENTIALITY_MODEID (0u)

#define SNOW3G_INTEGRITY_MODEID (1u)
#define ZUC_INTEGRITY_MODEID    (1u)
#define KASUMI_INTEGRITY_MODEID (4u)

#define BA423_CFG_DIRECTION_OFFSET 1
#define BA423_CFG_KEY_SIZE 16
#define BA421_CFG_DIRECTION_OFFSET 1
#define BA421_CFG_KEY_SIZE 16
#define BA422_CFG_DIRECTION_OFFSET 3
#define BA422_CFG_KEY_SIZE 16


struct sx_3gpp_cmdma_tags {
    uint32_t cfg;
    uint32_t key;
    uint32_t data;
    uint32_t bf;   /* bearer, fresh or outsz */
    uint32_t cnt;  /* frame dependent input */
    uint32_t szmask;
};


struct sx3gppalg
{
    const struct sx_3gpp_cmdma_tags *dmatags;
    unsigned int mode;
    uint32_t outsz;
};


#define OFFSET_PARAMS(c) (sizeof((c)->dma.dmamem) + sizeof((c)->descs))
#define ADD_PARAMDESC(c, idx, tag) ADD_INDESC_PRIV((c)->dma, OFFSET_PARAMS(c) + (idx) * sizeof(uint32_t), sizeof(uint32_t), (tag))


static const struct sx_3gpp_cmdma_tags ba423tags = {
    .cfg = DMATAG_BA423 | DMATAG_CONFIG(0),
    .key = DMATAG_BA423 | DMATAG_CONFIG(0x04),
    .bf = DMATAG_BA423 | DMATAG_CONFIG(0x14),
    .cnt = DMATAG_BA423 | DMATAG_CONFIG(0x18),
    .data = DMATAG_BA423,
    .szmask = 0x3,
};


static const struct sx3gppalg ba423cfg = {
    .dmatags = &ba423tags,
};


static const struct sx_3gpp_cmdma_tags ba421tags = {
    .cfg = DMATAG_BA421 | DMATAG_CONFIG(0),
    .key = DMATAG_BA421 | DMATAG_CONFIG(0x04),
    .bf = DMATAG_BA421 | DMATAG_CONFIG(0x14),
    .cnt = DMATAG_BA421 | DMATAG_CONFIG(0x18),
    .data = DMATAG_BA421,
    .szmask = 0x3,
};


static const struct sx3gppalg ba421cfg = {
    .dmatags = &ba421tags,
};


static const struct sx_3gpp_cmdma_tags ba422tags = {
    .cfg = DMATAG_BA422 | DMATAG_CONFIG(0),
    .key = DMATAG_BA422 | DMATAG_CONFIG(0x08),
    .bf = DMATAG_BA422 | DMATAG_CONFIG(0x18),
    .cnt = DMATAG_BA422 | DMATAG_CONFIG(0x20),
    .data = DMATAG_BA422,
    .szmask = 0x7,
};

/* The hardware is inconsistent and for GEA modes it expects
 * frame dependent input with the tag for framein (0x28) instead of the
 * tag for cnt (0x20) as in ALL other modes.
 * And to make matters worse, it expects the output size (in bytes)
 * with the cnt tag (0x20). */
static const struct sx_3gpp_cmdma_tags ba422geatags = {
    .cfg = DMATAG_BA422 | DMATAG_CONFIG(0),
    .key = DMATAG_BA422 | DMATAG_CONFIG(0x08),
    .bf = DMATAG_BA422 | DMATAG_CONFIG(0x20),
    .cnt = DMATAG_BA422 | DMATAG_CONFIG(0x28),
    .data = DMATAG_BA422,
    .szmask = 0x7,
};

static const struct sx3gppalg ba422cfg = {
    .dmatags = &ba422tags,
};

const struct sx3gppalg sx3gppalg_kasumi_gsm_a53 = {
    .dmatags = &ba422tags,
    .mode = 1u,
    .outsz = 29u, /* 228 bits */
};

const struct sx3gppalg sx3gppalg_kasumi_gsm_a54 = {
    .dmatags = &ba422tags,
    .mode = 5u,
    .outsz = 29u, /* 228 bits */
};

const struct sx3gppalg sx3gppalg_kasumi_ecsd_a53 = {
    .dmatags = &ba422tags,
    .mode = 2u,
    .outsz = 87u, /* 696 bits */
};

const struct sx3gppalg sx3gppalg_kasumi_ecsd_a54 = {
    .dmatags = &ba422tags,
    .mode = 6u,
    .outsz = 87u, /* 696 bits */
};

const struct sx3gppalg sx3gppalg_kasumi_gea3 = {
    .dmatags = &ba422geatags,
    .mode = 3u,
};

const struct sx3gppalg sx3gppalg_kasumi_gea4 = {
    .dmatags = &ba422geatags,
    .mode = 7u,
};

static uint32_t u32tobe(uint32_t v)
{
    /* This is a very minimalistic implementation. Customize if needed.*/
    static const uint32_t ref = 0x01020304;
    static const char *pref = (char*)&ref;

    if (*pref == 1) {
        /* big endian */
        return v;
    } else {
        /* little endian */
        return ((v >> 24) & 0xFF)
            | ((v >> 8) & 0xFF00)
            | ((v & 0xFF00 ) << 8)
            | ((v & 0xFF) << 24);
    }
}


int sx_3gpp_create_snow3g(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir, uint32_t bearer,
    uint32_t count)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba423 = ~0u;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;

    if (KEYREF_IS_USR(key)) {
        if (key->sz != BA423_CFG_KEY_SIZE)
            return SX_ERR_INVALID_KEY_SZ;
    } else {
        /* HW key is not supported */
        return SX_ERR_HW_KEY_NOT_SUPPORTED;
    }

    if (compatibleba423 == ~0u)
        compatibleba423 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA423);
    if (!compatibleba423)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(compatibleba423);

    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba423cfg;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        SNOW3G_CONFIDENTIALITY_MODEID | (!!dir << BA423_CFG_DIRECTION_OFFSET),
        c->cfg->dmatags->cfg);

    ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    c->params[0] = u32tobe(count);
    c->params[1] = u32tobe(bearer);
    ADD_PARAMDESC(c, 0, c->cfg->dmatags->cnt);
    ADD_PARAMDESC(c, 1, c->cfg->dmatags->bf);

    c->outsz = 0;
    c->insz = 0;

    return SX_OK;
}


int sx_3gpp_mac_create_snow3g(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir, uint32_t fresh, uint32_t count)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba423 = ~0u;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (key->sz != BA423_CFG_KEY_SIZE)
            return SX_ERR_INVALID_KEY_SZ;
    } else {
        /* HW key is not supported */
        return SX_ERR_HW_KEY_NOT_SUPPORTED;
    }

    if (compatibleba423 == ~0u)
        compatibleba423 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA423);

    if (!compatibleba423)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(compatibleba423);
    if (!c->dma.regs)
        return SX_ERR_RETRY;
    c->cfg = &ba423cfg;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        SNOW3G_INTEGRITY_MODEID | (!!dir << BA423_CFG_DIRECTION_OFFSET),
        c->cfg->dmatags->cfg);

    ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    c->params[0] = u32tobe(count);
    c->params[1] = u32tobe(fresh);
    ADD_PARAMDESC(c, 0, c->cfg->dmatags->cnt);
    ADD_PARAMDESC(c, 1, c->cfg->dmatags->bf);

    c->insz = 0;
    c->outsz = 4;

    return SX_OK;
}


int sx_3gpp_create_zuc(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir,
    uint32_t bearer, uint32_t count)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba421 = ~0u;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;

    if (KEYREF_IS_USR(key)) {
        if (key->sz != BA421_CFG_KEY_SIZE)
            return SX_ERR_INVALID_KEY_SZ;
    } else {
        /* HW key is not supported */
        return SX_ERR_HW_KEY_NOT_SUPPORTED;
    }

    if (compatibleba421 == ~0u)
        compatibleba421 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA421);
    if (!compatibleba421)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(compatibleba421);

    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba421cfg;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        ZUC_CONFIDENTIALITY_MODEID | (!!dir << BA421_CFG_DIRECTION_OFFSET),
        c->cfg->dmatags->cfg);

    ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    c->params[0] = u32tobe(count);
    c->params[1] = u32tobe(bearer);
    ADD_PARAMDESC(c, 0, c->cfg->dmatags->cnt);
    ADD_PARAMDESC(c, 1, c->cfg->dmatags->bf);

    c->outsz = 0;
    c->insz = 0;

    return SX_OK;
}


int sx_3gpp_mac_create_zuc(struct sx3gpp *c, const struct sxkeyref *key,
    const uint32_t dir, uint32_t bearer, uint32_t count)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba421 = ~0u;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (key->sz != BA421_CFG_KEY_SIZE)
            return SX_ERR_INVALID_KEY_SZ;
    } else {
        /* HW key is not supported */
        return SX_ERR_HW_KEY_NOT_SUPPORTED;
    }

    if (compatibleba421 == ~0u)
        compatibleba421 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA421);

    if (!compatibleba421) {
        return SX_ERR_INCOMPATIBLE_HW;
    }

    c->dma.regs = sx_cmdma_find_available(compatibleba421);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba421cfg;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        ZUC_INTEGRITY_MODEID |
        (!!dir << BA421_CFG_DIRECTION_OFFSET), c->cfg->dmatags->cfg);

    ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    c->params[0] = u32tobe(count);
    c->params[1] = u32tobe(bearer);
    ADD_PARAMDESC(c, 0, c->cfg->dmatags->cnt);
    ADD_PARAMDESC(c, 1, c->cfg->dmatags->bf);

    c->insz = 0;
    c->outsz = 4;

    return SX_OK;
}


int sx_3gpp_create_kasumi(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir, uint32_t bearer,
    uint32_t count)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba422 = ~0u;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;

    if (KEYREF_IS_USR(key)) {
        if (key->sz != BA422_CFG_KEY_SIZE)
            return SX_ERR_INVALID_KEY_SZ;
    } else {
        /* HW key is not supported */
        return SX_ERR_HW_KEY_NOT_SUPPORTED;
    }

    if (compatibleba422 == ~0u)
        compatibleba422 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA422);
    if (!compatibleba422)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(compatibleba422);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba422cfg;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        KASUMI_CONFIDENTIALITY_MODEID |
        (!!dir << BA422_CFG_DIRECTION_OFFSET),
        c->cfg->dmatags->cfg);

    ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    c->params[0] = u32tobe(count);
    c->params[1] = u32tobe(bearer);
    ADD_PARAMDESC(c, 0, c->cfg->dmatags->cnt);
    ADD_PARAMDESC(c, 1, c->cfg->dmatags->bf);

    c->outsz = 0;
    c->insz = 0;

    return SX_OK;
}


int sx_3gpp_mac_create_kasumi(struct sx3gpp *c,
    const struct sxkeyref *key, const uint32_t dir, uint32_t fresh, uint32_t count)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba422 = ~0u;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (key->sz != BA422_CFG_KEY_SIZE)
            return SX_ERR_INVALID_KEY_SZ;
    } else {
        /* HW key is not supported */
        return SX_ERR_HW_KEY_NOT_SUPPORTED;
    }

    if (compatibleba422 == ~0u)
        compatibleba422 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA422);

    if (!compatibleba422)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(compatibleba422);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = &ba422cfg;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        KASUMI_INTEGRITY_MODEID |
        (!!dir << BA422_CFG_DIRECTION_OFFSET), c->cfg->dmatags->cfg);

    ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
    c->params[0] = u32tobe(count);
    c->params[1] = u32tobe(fresh);
    ADD_PARAMDESC(c, 0, c->cfg->dmatags->cnt);
    ADD_PARAMDESC(c, 1, c->cfg->dmatags->bf);

    c->insz = 0;
    c->outsz = 8;

    return SX_OK;
}


int sx_3gpp_create_kasumi_keystream(struct sx3gpp *c,
    const struct sxkeyref *key, const struct sx3gppalg *alg,
    uint32_t framein, uint32_t outsz)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba422 = ~0u;

    if (alg->outsz == 0 && outsz == 0)
        return SX_ERR_UNITIALIZED_OBJ;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;

    if (KEYREF_IS_USR(key)) {
        if (key->sz != BA422_CFG_KEY_SIZE)
            return SX_ERR_INVALID_KEY_SZ;
    } else {
        /* HW key is not supported */
        return SX_ERR_HW_KEY_NOT_SUPPORTED;
    }

    if (compatibleba422 == ~0u)
        compatibleba422 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA422);
    if (!compatibleba422)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(compatibleba422);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = alg;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        alg->mode | (0 << BA422_CFG_DIRECTION_OFFSET), c->cfg->dmatags->cfg);

    ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);

    c->params[0] = u32tobe(framein);
    ADD_PARAMDESC(c, 0, c->cfg->dmatags->cnt);
    c->outsz = alg->outsz;
    if (alg->outsz == 0) {
        c->outsz = outsz;
        c->params[1] = u32tobe(outsz);
        ADD_PARAMDESC(c, 1, c->cfg->dmatags->bf);
    }


    /* The hardware cannot output the (key) cipher stream without some
     * initial 8 bytes of random input. The content does not matter.
     * Here we use the key data as it's at least 8 bytes big. */
    ADD_INDESC_BITS(c->dma, key->key, 8, c->cfg->dmatags->data,
            c->cfg->dmatags->szmask, 8 * 8);
    c->insz = 8; /* No real data input for this one. Needed to keep run() happy*/

    return SX_OK;
}


static void sx_3gpp_free(struct sx3gpp *c)
{
    sx_cmdma_release_hw(c->dma.regs);
    c->dma.regs = NULL;
}


int sx_3gpp_crypt(struct sx3gpp *c, const char *datain, size_t inbitsz,
    char *dataout)
{
    size_t insz = 0;
    size_t outsz = 0;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if (!inbitsz) {
        sx_3gpp_free(c);
        return SX_ERR_TOO_SMALL;
    }

    insz = (inbitsz + 7) >> 3;
    c->insz = insz;
    outsz = insz;

    if (insz >= DMA_MAX_SZ) {
        sx_3gpp_free(c);
        return SX_ERR_TOO_BIG;
    }

    ADD_INDESC_BITS(c->dma, datain, insz, c->cfg->dmatags->data,
            c->cfg->dmatags->szmask, inbitsz);
    ADD_OUTDESCA(c->dma, dataout, outsz, c->cfg->dmatags->szmask);

    return SX_OK;
}


int sx_3gpp_mac_feed(struct sx3gpp *c, const char *datain, size_t inbitsz)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (!inbitsz) {
        sx_3gpp_free(c);
        return SX_ERR_TOO_SMALL;
    }
    c->insz = (inbitsz + 7) >> 3;

    if (c->insz >= DMA_MAX_SZ) {
        sx_3gpp_free(c);
        return SX_ERR_TOO_BIG;
    }

    ADD_INDESC_BITS(c->dma, datain, c->insz, c->cfg->dmatags->data,
            c->cfg->dmatags->szmask, inbitsz);

    return SX_OK;
}


int sx_3gpp_run(struct sx3gpp *c)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (c->insz == 0) {
        sx_3gpp_free(c);
        return SX_ERR_TOO_SMALL;
    }
    sx_cmdma_start(&c->dma,  sizeof(c->descs) + sizeof(c->params), c->descs);

    return SX_OK;
}


int sx_3gpp_generate(struct sx3gpp *c, char *output)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    ADD_OUTDESCA(c->dma, output, c->outsz, c->cfg->dmatags->szmask);

    return sx_3gpp_run(c);
}


int sx_3gpp_status(struct sx3gpp *c)
{
    int r;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    r = sx_cmdma_check(&c->dma);
    if (r != SX_ERR_HW_PROCESSING)
        sx_3gpp_free(c);

    return r;
}


int sx_3gpp_wait(struct sx3gpp *c)
{
    int r = SX_ERR_HW_PROCESSING;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    while (r == SX_ERR_HW_PROCESSING) {
        sx_cmdma_wait(c->dma.regs);
        r = sx_3gpp_status(c);
    }

    return r;
}
