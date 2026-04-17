/*
 * @Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#include "../../hw.h"
#include "../../cmdma.h"
#include <stddef.h>
#include <stdint.h>
#include "../../membarriers.h"

#ifndef SX_CM_REGS_ADDR
#define SX_CM_REGS_ADDR 0x000C0000
#endif
#ifndef SX_CM_REGS_STRIDE
#define SX_CM_REGS_STRIDE  0x4000
#endif
#ifndef SX_ADDR2BUS
#define SX_ADDR2BUS 0x00000000
#endif
#ifndef SX_TRNG_REGS_OFFSET
#define SX_TRNG_REGS_OFFSET 0x1000
#endif

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

#ifndef SX_DMAMEM_RESERVE_MEM_SZ
#define SX_DMAMEM_RESERVE_MEM_SZ 2048
#endif

#if SX_DMAMEM_RESERVE_MEM_SZ != 0
static char global_dmamem[SX_DMAMEM_RESERVE_MEM_SZ];
#endif

struct sx_regs {
    char *devmem;
    int slotidx;
};


static const struct sx_regs hwregs[] = {
    {
        .devmem = (char*)(SX_CM_REGS_ADDR),
        .slotidx = 0,
    },
    /* CUSTOMIZATION: For more than one instance of CryptoMaster,
     * add entries with the base address of the registers and
     * incremented slot index. */
};


static const struct sx_regs trngregs[] = {
    {
        .devmem = (char*)(SX_CM_REGS_ADDR) + (SX_TRNG_REGS_OFFSET),
    },
};

static void sx_wrregx(struct sx_regs *regs, uint32_t addr, uint32_t val)
{
    volatile uint32_t *p = (uint32_t*)(regs->devmem + addr);
    wmb();
    *p = val;
    rmb();
}

static void sx_wrregx_addr(struct sx_regs *regs, uint32_t addr, size_t p)
{
    volatile size_t *d = (volatile size_t*)(regs->devmem + addr);
    wmb();
    *d = p;
    rmb();
}


static uint32_t sx_rdregx(struct sx_regs *regs, uint32_t addr)
{
    volatile uint32_t *p = (uint32_t*)(regs->devmem + addr);
    uint32_t v;
    wmb();
    v = *p;
    rmb();
    return v;
}


struct sx_regs *sx_hw_find_regs(unsigned int idx)
{
    if (idx >= ARRAY_COUNT(hwregs))
        return NULL;

    return (struct sx_regs *) &hwregs[idx];
}

int sx_hw_idx_of_regs(struct sx_regs *regs)
{
    return regs - hwregs;
}


struct sx_regs *sx_hw_find_trng_regs(unsigned int idx)
{
    if (idx >= ARRAY_COUNT(trngregs))
        return NULL;

    return (struct sx_regs *) &trngregs[idx];
}


void sx_wrreg(struct sx_regs *regs, uint32_t addr, uint32_t val)
{
    sx_wrregx(regs, addr, val);
}


void sx_wrreg_addr(struct sx_regs *regs, uint32_t addr, struct sxdesc *p)
{
    sx_wrregx_addr(regs, addr, (size_t)p);
}


char *sx_map_internal(struct sx_regs *regs, char *dma)
{
    (void)regs;
    return (char*)(dma) + (SX_ADDR2BUS);
}


char *sx_map_usrdatain(char *s, size_t sz)
{
    (void) sz;
    return s + (SX_ADDR2BUS);
}

char *sx_map_usrdataout(char *s, size_t sz)
{
    (void) sz;
    return s + (SX_ADDR2BUS);
}

void sx_flush_tohw(struct sx_regs *regs, char *cpumem, size_t sz)
{
    (void)regs;
    (void)cpumem;
    (void)sz;
    /* CUSTOMIZATION: on non-coherent architectures, the CPU caches
     * should be flushed such that the hardware can see the memory as
     * written by the CPU. */
}


void sx_flush_fromhw(struct sx_regs *regs, char *cpumem, size_t offset,
    size_t sz)
{
    (void)regs;
    (void)cpumem;
    (void)offset;
    (void)sz;
    /* CUSTOMIZATION: on non-coherent architectures, the CPU caches
     * should be flushed such that it sees the memory as written by the
     * hardware. */
}


uint32_t sx_rdreg(struct sx_regs *regs, uint32_t addr)
{
    return sx_rdregx(regs, addr);
}

#if SX_DMAMEM_RESERVE_MEM_SZ != 0
char *sx_alloc_global_dmamem(size_t sz)
{
    if (sz > sizeof(global_dmamem))
        return NULL;

    return global_dmamem;
}
#endif


void sx_cmdma_wait(struct sx_regs *regs)
{
    (void)regs;
    /* CUSTOMIZATION: write custom wait on interrupts here. This is also
     * where custom code can be included to move the CPU to a lower power
     * mode. */
}
