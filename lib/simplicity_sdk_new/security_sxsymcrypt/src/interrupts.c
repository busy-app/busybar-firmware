/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/interrupts.h"
#include "../include/sxsymcrypt/statuscodes.h"
#include "crypmasterregs.h"
#include "cmdma.h"
#include "hw.h"


#define MAX_IRQ_INSTANCES 64u

/* Enable interrupts showing that an operation finished or aborted.
 * For that, we're interested in :
 *     # Fetcher DMA error
 *     # Pusher DMA error
 *     # Pusher DMA stop
 *  */
#define CMDMA_INTMASK_EN ((1 << 2) | (1 << 5) | (1 << 4))


int sx_interrupts_enable(void)
{
    unsigned int i;

    for (i = 0; i < MAX_IRQ_INSTANCES; i++) {
        struct sx_regs *regs;
        uint32_t r;

        regs = sx_hw_find_regs(i);
        if (!regs)
            break;
        r = sx_rdreg(regs, REG_HW_PRESENCE);
        if (!r)
            break;
        /* to start from a known good state, reset everything */
        sx_wrreg(regs, REG_INT_EN, 0);
        sx_wrreg(regs, REG_INT_STATCLR, ~0);
        sx_wrreg(regs, REG_INT_EN, CMDMA_INTMASK_EN);
    }

    return SX_OK;
}


int sx_interrupts_disable(void)
{
    unsigned int i;

    for (i = 0; i < MAX_IRQ_INSTANCES; i++) {
        struct sx_regs *regs;
        uint32_t r;

        regs = sx_hw_find_regs(i);
        if (!regs)
            break;
        r = sx_rdreg(regs, REG_HW_PRESENCE);
        if (!r)
            break;
        sx_wrreg(regs, REG_INT_EN, 0);
        sx_wrreg(regs, REG_INT_STATCLR, ~0);
    }

    return SX_OK;
}
