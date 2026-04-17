/*
 * @copyright Copyright 2023 Secure-IC S.A.S.
 * This file relies on Secure-IC S.A.S. software and patent portfolio.
 * This file cannot be used nor duplicated without prior approval from Secure-IC S.A.S.
 */

#ifndef HW_HEADER_FILE
#define HW_HEADER_FILE

#include <stdint.h>
#include <stddef.h>

#ifndef SX_EXTRA_IN_DESCS
#define SX_EXTRA_IN_DESCS 0
#endif

struct sx_regs;
struct sxdesc;

/** Look up the cryptomaster registers of instance 'idx' */
struct sx_regs *sx_hw_find_regs(unsigned int idx);


/** Return th instance index for the given cryptomaster registers */
int sx_hw_idx_of_regs(struct sx_regs *regs);


/** Look up the TRNG registers of instance 'idx' */
struct sx_regs *sx_hw_find_trng_regs(unsigned int idx);


/** Write value 'val' at address 'addr' of the registers 'regs' */
void sx_wrreg(struct sx_regs *regs, uint32_t addr, uint32_t val);


/** Write a pointer 'p' into register at 'regaddr' from 'regs' */
void sx_wrreg_addr(struct sx_regs *regs, uint32_t addr, struct sxdesc *p);


/** Read register at address 'addr' from registers 'regs' */
uint32_t sx_rdreg(struct sx_regs *regs, uint32_t addr);


/** Map user memory for access by DMA */
char *sx_map_usrdatain(char *s, size_t sz);
char *sx_map_usrdataout(char *s, size_t sz);

/** Map internal memory for access by DMA */
char *sx_map_internal(struct sx_regs *regs, char *dma);

/** Flush cpu accessed memory for access by the hardware */
void sx_flush_tohw(struct sx_regs *regs, char *cpumem, size_t sz);

/** Flush memory written by the hardware for access by the cpu */
void sx_flush_fromhw(struct sx_regs *regs, char *cpumem, size_t offset,
    size_t sz);


/** Return and reserve a compatible cryptomaster instance
 *
 * The 'compatible' bitmask tells which cryptomaster instances the caller
 * can work with.
 *
 * This returns the registers for the cryptomaster instance found or
 * NULL if none were available. The cryptomaster instance is then
 * reserved and should be given back after use with sx_cmdma_release_hw().
 *
 * The compatible bitmask comes from sx_cmdma_list_compatible().
 */
struct sx_regs *sx_cmdma_find_available(unsigned int compatible);


/** Give back the cryptomaster instance associated with regs */
void sx_cmdma_release_hw(struct sx_regs *regs);

/** Wait function used with interrupts
 *
 * This function is called only if interrupts are enabled.
 */
void sx_cmdma_wait(struct sx_regs *regs);

/** Trigger a CPU hard fault.
 *
 */
void sx_trigger_hardfault(void);

#endif
