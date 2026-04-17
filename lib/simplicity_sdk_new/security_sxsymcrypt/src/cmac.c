/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/cmac.h"
#include "../include/sxsymcrypt/aes.h"
#include "../include/sxsymcrypt/keyref.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "keyrefdefs.h"
#include "macdefs.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "cmaes.h"

#define CMDMA_BA411_BUS_MSK (0x0F)
#define CMAC_MODEID_AES 8
#define CMAC_BLOCK_SZ 16
#define CMAC_STATE_SZ 16
#define CMAC_MAC_SZ 16

/** Mode Register value for context loading */
#define CMAC_MODEID_CTX_LOAD (1u << 4)
/** Mode Register value for context saving */
#define CMAC_MODEID_CTX_SAVE (1u << 5)


static const struct sx_mac_cmdma_tags ba411tags = {
    .cfg = DMATAG_BA411 | DMATAG_CONFIG(0),
    .state = DMATAG_BA411 | DMATAG_CONFIG(0x28),
    .key = DMATAG_BA411 | DMATAG_CONFIG(0x08),
    .data = DMATAG_BA411
};


static const struct sx_mac_cmdma_cfg ba411cfg = {
    .cmdma_mask = CMDMA_BA411_BUS_MSK,
    .granularity = CMAC_BLOCK_SZ,
    .blocksz = CMAC_BLOCK_SZ,
    .savestate = CMAC_MODEID_CTX_SAVE,
    .statesz = CMAC_STATE_SZ,
    .loadstate = CMAC_MODEID_CTX_LOAD,
    .dmatags = &ba411tags,
};


static int sx_cmac_create_aes_ba411(struct sxmac *c, const struct sxkeyref *key,
    const struct sxaesparams* params);

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


static int sx_cmac_create_aes_ba411(struct sxmac *c, const struct sxkeyref *key,
    const struct sxaesparams *params)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba411 = ~0u;
    unsigned int mode_compatibleba411;
    uint32_t hwreqs = 0;
    uint32_t hw_temp_red = 0;

    if (KEYREF_IS_INVALID(key))
        return SX_ERR_INVALID_KEYREF;
    if (KEYREF_IS_USR(key)) {
        if (sx_aes_keysz((key)->sz) == ~0u)
            return SX_ERR_INVALID_KEY_SZ;
        hwreqs = sx_cmac_key_mask(key->sz);
    }

    if (compatibleba411 == ~0u) {
        compatibleba411 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA411);
    }

    hwreqs |= (1 << CMAC_MODEID_AES);
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

    c->cfg = &ba411cfg;
    c->cntindescs = 1;
    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs),
        CMDMA_CMAC_MODE_SET(CMAC_MODEID_AES) | KEYREF_AES_HWKEY_CONF(key->cfg) | hw_temp_red,
        c->cfg->dmatags->cfg);
    if (KEYREF_IS_USR(key)) {
        ADD_CFGDESC(c->dma, key->key, key->sz, c->cfg->dmatags->key);
        c->cntindescs++;
    }
    c->feedsz = 0;
    c->macsz = CMAC_MAC_SZ;
    c->key = key;
    c->compatible = mode_compatibleba411;

    return SX_OK;
}


int sx_mac_create_aescmac(struct sxmac *c, const struct sxkeyref *key)
{
    struct sxaesparams params = {.config = 0};

    return sx_cmac_create_aes_ba411(c, key, &params);
}

int sx_mac_create_aes_generic(struct sxmac *c, const struct sxkeyref *key,
    const struct sxaesparams *params)
{
    return sx_cmac_create_aes_ba411(c, key, params);
}
