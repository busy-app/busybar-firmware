/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/cmmask.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"


int sx_cm_load_mask(struct sxcmmask *c, uint32_t value)
{
    SX_COMPATIBILTY_STORAGE unsigned int compatibleba411 = ~0u;

    if (compatibleba411 == ~0u)
        compatibleba411 = sx_cmdma_list_compatible(REG_HW_PRESENT_BA411);
    if (!compatibleba411)
        return SX_ERR_INCOMPATIBLE_HW;

    c->channel.dma.regs = sx_cmdma_find_available(compatibleba411);
    if (!c->channel.dma.regs)
        return SX_ERR_RETRY;

    /* This is a special case where "cfg" is used to transmit the random
     * value instead of a command word. */
    sx_cmdma_newcmd(&c->channel.dma, c->channel.descs, sizeof(c->channel.descs),
        value, DMATAG_BA411 | DMATAG_CONFIG(0x68));

    ADD_OUTDESCA(c->channel.dma, NULL, 0, 0xf);

    return sx_channel_run(&c->channel);
}
