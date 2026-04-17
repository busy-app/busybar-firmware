/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/channel.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"

/* We take in consideration also 128 bit bus */
#define BUS_MASK (0x0F)


static void sx_channel_free(struct sxchannel *c)
{
    sx_cmdma_release_hw(c->dma.regs);
    c->dma.regs = NULL;
}


int sx_channel_transform(struct sxchannel *c, const char *src, const size_t sz,
        char *dst)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (!sz)
        return SX_OK;

    ADD_INDESCA(c->dma, src, sz, DMATAG_BYPASS, BUS_MASK);
    ADD_OUTDESCA(c->dma, dst, sz, BUS_MASK);

    return SX_OK;
}


int sx_channel_run(struct sxchannel *c)
{
    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;
    if (c->dma.d == c->descs) {
        sx_channel_free(c);
        return SX_ERR_TOO_SMALL;
    }
    sx_cmdma_start(&c->dma, sizeof(c->descs), c->descs);

    return SX_OK;
}


int sx_channel_status(struct sxchannel *c)
{
    int r;

    if (!c->dma.regs)
        return SX_ERR_UNITIALIZED_OBJ;

    r = sx_cmdma_check(&c->dma);
    if (r == SX_ERR_HW_PROCESSING)
        return SX_ERR_HW_PROCESSING;

    sx_channel_free(c);

    return r;
}


int sx_channel_wait(struct sxchannel *c)
{
    int r = SX_ERR_HW_PROCESSING;

    while (r == SX_ERR_HW_PROCESSING) {
        sx_cmdma_wait(c->dma.regs);
        r = sx_channel_status(c);
    }

    return r;
}
