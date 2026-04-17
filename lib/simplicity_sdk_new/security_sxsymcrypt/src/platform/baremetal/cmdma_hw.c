/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../../hw.h"
#include <stddef.h>
#include <stdint.h>
#include "../../crypmasterregs.h"
#include "../../cmdma.h"
#include "../../../include/sxsymcrypt/statuscodes.h"

#if !defined SX_BAREMETAL_AVAILABILITY_CHECK || SX_BAREMETAL_AVAILABILITY_CHECK != 0
static unsigned int available_hw = ~0u;


static struct sx_regs *sx_hw_reserve(unsigned int idx)
{
    /* CUSTOMIZATION: replace with custom reservation implementation,
     * for example atomic bit clear. */
    available_hw &= ~(1u << idx);

    return sx_hw_find_regs(idx);
}


void sx_cmdma_release_hw(struct sx_regs *regs)
{
    int idx;

    idx = sx_hw_idx_of_regs(regs);
    /* CUSTOMIZATION: replace with custom reservation implementation,
     * for example atomic bit set. */
    available_hw |= (1u << idx);
}


struct sx_regs *sx_cmdma_find_available(unsigned int compatible)
{
    unsigned int i;
    unsigned int combined;

    combined = compatible & available_hw;
    for (i = 0; combined; i++, combined >>= 1) {
        if (combined & 1)
            return sx_hw_reserve(i);
    }

    return NULL;
}
#else
void sx_cmdma_release_hw(struct sx_regs *regs)
{
    (void)regs;
}


struct sx_regs *sx_cmdma_find_available(unsigned int compatible)
{
    struct sx_regs * regs;
    uint32_t dma_error = 0xFF;
    uint32_t dma_busy;

    if (compatible) {
        regs = sx_hw_find_regs(0);
        dma_error = sx_rdreg(regs, REG_INT_STATRAW);
        dma_busy = sx_rdreg(regs, REG_STATUS) & REG_STATUS_BUSY_MASK;
        if ((dma_busy) || (dma_error & (DMA_BUS_FETCHER_ERROR_MASK | DMA_BUS_PUSHER_ERROR_MASK)))
            return NULL;
        return sx_hw_find_regs(0);
    }

    return NULL;
}

#endif
