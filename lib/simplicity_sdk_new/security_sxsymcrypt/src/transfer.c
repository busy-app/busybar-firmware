/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/transfer.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"

int sx_transfer_create_copier(struct sxchannel *c)
{
    SX_COMPATIBILTY_STORAGE unsigned int cmpresent = ~0u;

    if (cmpresent == ~0u)
        cmpresent = sx_cmdma_list_compatible(~0u);

    if (!cmpresent)
        return SX_ERR_INCOMPATIBLE_HW;

    c->dma.regs = sx_cmdma_find_available(cmpresent);
    if (!c->dma.regs)
        return SX_ERR_RETRY;

    sx_cmdma_newdma(&c->dma, c->descs, sizeof(c->descs));

    return SX_OK;
}

