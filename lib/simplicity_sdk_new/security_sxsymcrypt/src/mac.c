/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/mac.h"
#include "../include/sxsymcrypt/keyref.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "keyrefdefs.h"
#include "macdefs.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "cmaes.h"


static void sx_mac_free(struct sxmac *c)
{
    sx_cmdma_release_hw(c->dma.regs);
    c->dma.regs = NULL;
}


static int sx_mac_feed_data(struct sxmac *c, const char *datain, size_t sz)
{
    if (sz >= DMA_MAX_SZ) {
        sx_mac_free(c);
        return SX_ERR_TOO_BIG;
    }

    if (sz != 0) {
        if (!sx_cmdma_has_room(&c->dma, c->cntindescs, 2)) {
            sx_mac_free(c);
            return SX_ERR_FEED_COUNT_EXCEEDED;
        }

        ADD_RAW_INDESC(c->dma, datain, sz, c->cfg->dmatags->data);
        c->cntindescs++;
        c->feedsz += sz;
    }

    return SX_OK;
}


int sx_mac_feed(struct sxmac *c, const char *datain, size_t sz)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    return sx_mac_feed_data(c, datain, sz);
}


int sx_mac_multifeed(struct sxmac *c, const struct sxdataref *datain, size_t countdatain)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    for (size_t i = 0; i < countdatain; i++) {
        int r = sx_mac_feed_data(c, datain[i].data, datain[i].sz);
        if (r != SX_OK)
            return r;
    }

    return SX_OK;
}


static int sx_mac_run(struct sxmac *c)
{
    if ((c->feedsz == 0) && (c->dma.dmamem.cfg & c->cfg->loadstate)) {
        sx_mac_free(c);
        return SX_ERR_TOO_SMALL;
    }
    sx_cmdma_start(&c->dma, sizeof(c->descs), c->descs);

    return SX_OK;
}


int sx_mac_generate(struct sxmac *c, char *mac)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if(c->feedsz == 0)
        ADD_EMPTY_INDESC(c->dma, (c->cfg->cmdma_mask + 1), c->cfg->dmatags->data);
    SET_LAST_DESC_IGN(c->dma, c->feedsz, c->cfg->cmdma_mask);

    ADD_OUTDESCA(c->dma, mac, c->macsz, c->cfg->cmdma_mask);

    c->dma.dmamem.cfg &= ~c->cfg->savestate;

    return sx_mac_run(c);
}


int sx_mac_resume_state(struct sxmac *c)
{
    if (c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    if (!c->cfg->statesz)
        return SX_ERR_CONTEXT_SAVING_NOT_SUPPORTED;

    c->dma.regs = sx_cmdma_find_available(c->compatible);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    sx_cmdma_newcmd(&c->dma, c->descs, sizeof(c->descs), c->dma.dmamem.cfg,
        c->cfg->dmatags->cfg);
    c->cntindescs = 1;
    if (KEYREF_IS_USR(c->key)) {
        ADD_CFGDESC(c->dma, c->key->key, c->key->sz, c->cfg->dmatags->key);
        c->cntindescs++;
    }
    ADD_INDESC_PRIV(c->dma, OFFSET_EXTRAMEM(c), c->cfg->statesz,
        c->cfg->dmatags->state);
    c->cntindescs++;
    c->dma.dmamem.cfg |= c->cfg->loadstate;
    c->feedsz = 0;

    return SX_OK;
}


int sx_mac_save_state(struct sxmac *c)
{
    uint32_t sz;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    sz = c->feedsz;

    if (sz < c->cfg->blocksz) {
        sx_mac_free(c);
        return SX_ERR_TOO_SMALL;
    }
    if (sz % c->cfg->granularity) {
        sx_mac_free(c);
        return SX_ERR_WRONG_SIZE_GRANULARITY;
    }

    c->dma.dmamem.cfg |= c->cfg->savestate;

    ADD_OUTDESC_PRIV(c->dma, OFFSET_EXTRAMEM(c), c->cfg->statesz, 0x0F);

    return sx_mac_run(c);
}


int sx_mac_status(struct sxmac *c)
{
    int r;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    r = sx_cmdma_check(&c->dma);
    if (r == SX_ERR_HW_PROCESSING) {
        return r;
    }

    if (r == SX_OK)
        sx_flush_fromhw(c->dma.regs, (char*) &c->extramem, OFFSET_EXTRAMEM(c),
            sizeof(c->extramem));

    sx_mac_free(c);

    return r;
}


int sx_mac_wait(struct sxmac *c)
{
    int r = SX_ERR_HW_PROCESSING;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    while (r == SX_ERR_HW_PROCESSING) {
        sx_cmdma_wait(c->dma.regs);
        r = sx_mac_status(c);
    }

    return r;
}
