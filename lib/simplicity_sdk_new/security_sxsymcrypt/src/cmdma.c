/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../include/sxsymcrypt/statuscodes.h"
#include "../include/sxsymcrypt/internal.h"
#include "crypmasterregs.h"
#include "hw.h"
#include "cmdma.h"
#include "sl_code_classification.h"


void sx_cmdma_newdma(struct sx_dmactl *dma, union sxdescmem *d, size_t dsz)
{
    dma->d = d;
    dma->dsz = dsz;

    dma->mapped = sx_map_internal(dma->regs, (char*)&dma->dmamem);
}


void sx_cmdma_newcmd(struct sx_dmactl *dma, union sxdescmem *d, size_t dsz,
    uint32_t cmd, uint32_t tag)
{
    sx_cmdma_newdma(dma, d, dsz);
    dma->dmamem.cfg = cmd;
    ADD_INDESC_PRIV(*dma, offsetof(struct sx_dmaslot, cfg),
        sizeof(dma->dmamem.cfg), tag);
}

void sx_add_indesc(struct sx_dmactl *dma, char *baddr, size_t bsz, uint32_t tag)
{
    dma->d->addr = (uintptr_t)(baddr);
    dma->d++;
    dma->d->addr = 0;
    dma->d++;
    uint32_t *v = &dma->d->v;
    *v = (uint32_t)(bsz); v++;
    *v = tag;
    dma->d++;
    if (sizeof(uint32_t) == sizeof(uintptr_t)) dma->d++;
}


void sx_add_outdesc(struct sx_dmactl *dmactl, char *p, size_t bsz)
{
    dmactl->d->addr = (uintptr_t)(p);
    dmactl->d++;
    dmactl->d->addr = 1;
    dmactl->d++;
    dmactl->d->v = (uint32_t)(bsz);
    dmactl->d++;
    if (sizeof(uint32_t) == sizeof(uintptr_t))
        dmactl->d++;
}


SL_CODE_CLASSIFY(SL_CODE_COMPONENT_AES_COMMON, SL_CODE_CLASS_SECURITY)
static struct sxdesc *sx_cmdma_finalize_descs(struct sxdesc *start,
    struct sxdesc *end, struct sxdesc *mappeddesc)
{
    struct sxdesc *lastout;
    struct sxdesc *lastin;
    struct sxdesc firstout = { 0 };
    struct sxdesc *d;

    lastin = start;
    lastout = &firstout;
    for (d = start; d <= end; d++) {
#ifdef DMA_FIFO_ADDR
        if (d->addr == (char*)DMA_FIFO_ADDR)
            d->sz |= DMA_CONST_ADDR;
#endif
        if (d->next) {
            /* input marker => link output */
            lastout->next = mappeddesc;
            lastout = d;
        } else {
            lastin->next = mappeddesc;
            lastin = d;
        }
        mappeddesc++;
    }
    lastin->next = DMA_LAST_DESCRIPTOR;
    lastin->dmatag |= DMATAG_LAST;
    lastin->sz |= DMA_REALIGN;
    lastout->next = DMA_LAST_DESCRIPTOR;
    lastout->sz |= DMA_REALIGN;

    return firstout.next;
}


void sx_cmdma_start(struct sx_dmactl *dma, size_t privsz, union sxdescmem *d)
{
    struct sxdesc *m;
    struct sxdesc *mappedout;
    struct sxdesc *descs = (struct sxdesc *)d;

    m = (struct sxdesc *)(dma->mapped + sizeof(struct sx_dmaslot));
    mappedout = sx_cmdma_finalize_descs(descs, (struct sxdesc *)dma->d - 1, m);
    sx_flush_tohw(dma->regs, (char*)&dma->dmamem, sizeof(dma->dmamem) + privsz);
    sx_wrreg_addr(dma->regs, REG_FETCH_ADDR, m);
    sx_wrreg_addr(dma->regs, REG_PUSH_ADDR, mappedout);
    sx_wrreg(dma->regs, REG_CONFIG, REG_CONFIG_SG);
    sx_wrreg(dma->regs, REG_START, REG_START_ALL);
}


int sx_cmdma_check(struct sx_dmactl *dma)
{
    uint32_t r = 0xFF;
    uint32_t busy;

    r = sx_rdreg(dma->regs, REG_INT_STATRAW);
    busy = sx_rdreg(dma->regs, REG_STATUS) & REG_STATUS_BUSY_MASK;

    if (r & (DMA_BUS_FETCHER_ERROR_MASK | DMA_BUS_PUSHER_ERROR_MASK)) {
        sx_trigger_hardfault();
        sx_cmdma_reset(dma->regs);
        return SX_ERR_DMA_FAILED;
    }
    if (busy)
        return SX_ERR_HW_PROCESSING;

    sx_wrreg(dma->regs, REG_INT_STATCLR, ~0);

    return SX_OK;
}


unsigned int sx_cmdma_list_compatible(unsigned int mask)
{
    unsigned int i;
    unsigned int found = 0;

    for (i = 0; i < sizeof(found) * 8; i++) {
        struct sx_regs *regs;
        uint32_t r;

        regs = sx_hw_find_regs(i);
        if (!regs)
            break;
        r = sx_rdreg(regs, REG_HW_PRESENCE);
        if (r & mask)
            found |= 1u << i;
    }

    return found;
}


unsigned int sx_cmdma_filter_compatible(unsigned int compatible,
    uint32_t capsaddr, uint32_t capsmask)
{
    unsigned int i;
    unsigned int usable = 0;
    uint32_t hwcaps;
    struct sx_regs *regs = NULL;

    for (i = 0; compatible; i++, compatible >>= 1) {
        if (compatible & 1) {
            regs = sx_hw_find_regs(i);
            hwcaps = sx_rdreg(regs, capsaddr);
            if ((hwcaps & capsmask) == capsmask)
                usable |= 1u << i;
        }
    }

    return usable;
}


void sx_cmdma_reset(struct sx_regs *regs)
{
    uint32_t intrmask = sx_rdreg(regs, REG_INT_EN);
    sx_wrreg(regs, REG_CONFIG, REG_SOFT_RESET_ENABLE);
    sx_wrreg(regs, REG_CONFIG, 0); //clear SW reset

    /* Wait for soft-reset to end */
    while (sx_rdreg(regs, REG_STATUS) & REG_SOFT_RESET_BUSY);

    if (intrmask)
        sx_wrreg(regs, REG_INT_EN, intrmask);
}
