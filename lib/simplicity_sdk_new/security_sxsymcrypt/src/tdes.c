/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/tdes.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "blkcipherdefs.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "cmaes.h"

//Size in bytes of the TDES key
#define BLKCIPHER_TDES_KEYSZ 24

//Block size in bytes of the TDES
#define BLKCIPHER_TDES_BLOCKSZ 8

//BA412E-TDES Config register -> NbKey [1:0] set to Triple DES (0b0011)
#define CMDMA_BA412_TDES_SET (3)
//BA412E-TDES Config register -> ModeOfOperation [3:2]
#define CMDMA_BA412_MODE_TDES(modeid) ((modeid) << 2)

static const struct sx_blkcipher_cmdma_tags ba412tags = {
    .cfg = DMATAG_BA412 | DMATAG_CONFIG(0),
    .key = DMATAG_BA412 | DMATAG_CONFIG(4),
    .iv_or_state = DMATAG_BA412 | DMATAG_CONFIG(28),
    .data = DMATAG_BA412
};


static const struct sx_blkcipher_cmdma_cfg ba412ecbcfg = {
    .decr = CM_CFG_DECRYPT << 4,
    .dmatags = &ba412tags,
    .statesz = 0,
    .mode = BLKCIPHER_MODEID_ECB,
    .inminsz = 8,
    .granularity = 8,
    .blocksz = BLKCIPHER_TDES_BLOCKSZ,
};

static const struct sx_blkcipher_cmdma_cfg ba412cbccfg = {
    .decr = CM_CFG_DECRYPT << 4,
    .dmatags = &ba412tags,
    .statesz = 0,
    .mode = BLKCIPHER_MODEID_CBC,
    .inminsz = 8,
    .granularity = 8,
    .blocksz = BLKCIPHER_TDES_BLOCKSZ,
};


static int sx_blkcipher_create_tdes_ba412(struct sxblkcipher *c,
    const char *key, const char *iv, const struct sx_blkcipher_cmdma_cfg *cfg,
    const uint32_t dir)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba412 = ~0u;

    if (compatibleba412 == ~0u) {
        compatibleba412 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA412);
    }
    if (!compatibleba412)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(compatibleba412);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    c->cfg = cfg;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_BA412_TDES_SET | CMDMA_BA412_MODE_TDES(cfg->mode) | dir,
        c->cfg->dmatags->cfg);
    ADD_CFGDESC(c->dma, key, BLKCIPHER_TDES_KEYSZ , c->cfg->dmatags->key);
    if (iv != NULL) {
        ADD_CFGDESC(c->dma, iv, 8, c->cfg->dmatags->iv_or_state);
    }
    c->textsz = 0;

    return SX_OK;
}


int sx_blkcipher_create_tdesecb_enc(struct sxblkcipher *c, const char *key)
{
    return sx_blkcipher_create_tdes_ba412(c, key, NULL, &ba412ecbcfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_tdesecb_dec(struct sxblkcipher *c, const char *key)
{
    return sx_blkcipher_create_tdes_ba412(c, key, NULL, &ba412ecbcfg,
        ba412ecbcfg.decr);
}


int sx_blkcipher_create_tdescbc_enc(struct sxblkcipher *c, const char *key,
    const char *iv)
{
    return sx_blkcipher_create_tdes_ba412(c, key, iv, &ba412cbccfg,
        CM_CFG_ENCRYPT);
}


int sx_blkcipher_create_tdescbc_dec(struct sxblkcipher *c, const char *key,
    const char *iv)
{
    return sx_blkcipher_create_tdes_ba412(c, key, iv, &ba412cbccfg,
        ba412cbccfg.decr);
}
