/***************************************************************************//**
 * @file
 * @brief System Configuration (SYSCFG) peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include "sl_hal_syscfg.h"
#include "sl_status.h"
#include "sl_hal_bus.h"
#include "sl_assert.h"

/***************************************************************************//**
 * @addtogroup system config
 * @{
 ******************************************************************************/

/*******************************************************************************
 ******************************      DEFINES      ******************************
 ******************************************************************************/
#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) \
  || defined(_MPAHBRAM_CTRL_MASK)

#if defined(_SILICON_LABS_32B_SERIES_2)
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)

/// On Series 2 Config 1, aka EFR32XG21, ECC is supported for the
/// main DMEM RAM banks which is controlled with one ECC encoder/decoder.
#define ECC_RAM0_SYNDROMES_INIT (SYSCFG_DMEM0ECCCTRL_RAMECCEWEN)
#define ECC_RAM0_CORRECTION_EN  (SYSCFG_DMEM0ECCCTRL_RAMECCCHKEN)

#elif (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7)   \
  || (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) && !defined(_MPAHBRAM_CTRL_MASK)) \
  || (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11) && !defined(_MPAHBRAM_CTRL_MASK)))

/// On Series 2 Config 2, aka EFR32XG22, ECC is supported for the
/// main DMEM RAM banks which is controlled with one ECC encoder/decoder.
#define ECC_RAM0_SYNDROMES_INIT (SYSCFG_DMEM0ECCCTRL_RAMECCEN)
#define ECC_RAM0_CORRECTION_EN  (SYSCFG_DMEM0ECCCTRL_RAMECCEWEN)

#define ECC_IF_REG         (SYSCFG->IF)
#define ECC_IF_1BIT_ERROR  (SYSCFG_IF_RAMERR1B)

#elif defined(_MPAHBRAM_CTRL_MASK)

/// From Series 2 Config 3, aka EFR32XG23, ECC is now standalone in the
/// MPAHBRAM module.
#define ECC_RAM0_SYNDROMES_INIT (MPAHBRAM_CTRL_ECCWEN)
#define ECC_RAM0_CORRECTION_EN  (MPAHBRAM_CTRL_ECCEN)

#if defined(DMEM_COUNT) && (DMEM_COUNT == 2)
#define ECC_RAM1_SYNDROMES_INIT (MPAHBRAM_CTRL_ECCWEN)
#define ECC_RAM1_CORRECTION_EN  (MPAHBRAM_CTRL_ECCEN)
#endif

#define ECC_IF_REG         (DMEM->IF)
/// number of AHB ports is between 1 and 4.
#if defined(MPAHBRAM_IF_AHB3ERR1B)
#define ECC_IF_1BIT_ERROR       (MPAHBRAM_IF_AHB0ERR1B | MPAHBRAM_IF_AHB1ERR1B | MPAHBRAM_IF_AHB2ERR1B | MPAHBRAM_IF_AHB3ERR1B)
#elif defined(MPAHBRAM_IF_AHB2ERR1B)
#define ECC_IF_1BIT_ERROR       (MPAHBRAM_IF_AHB0ERR1B | MPAHBRAM_IF_AHB1ERR1B | MPAHBRAM_IF_AHB2ERR1B)
#elif defined(MPAHBRAM_IF_AHB1ERR1B)
#define ECC_IF_1BIT_ERROR       (MPAHBRAM_IF_AHB0ERR1B | MPAHBRAM_IF_AHB1ERR1B)
#else
#define ECC_IF_1BIT_ERROR       (MPAHBRAM_IF_AHB0ERR1B)
#endif

#else

#error "Unknown device"

#endif /* #if defined(if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1) */

#define ECC_RAM_SIZE_MAX   (RAM_MEM_SIZE)

#if defined(DMEM_COUNT) && (DMEM_COUNT == 2)

#define ECC_RAM0_MEM_BASE   (DMEM0_RAM0_RAM_MEM_BASE)
#define ECC_RAM0_MEM_SIZE   (DMEM0_RAM0_RAM_MEM_SIZE)
#define ECC_RAM1_MEM_BASE   (DMEM1_RAM0_RAM_MEM_BASE)
#define ECC_RAM1_MEM_SIZE   (DMEM1_RAM0_RAM_MEM_SIZE)

#define ECC_CTRL0_REG       (DMEM0->CTRL)
#define ECC_CTRL1_REG       (DMEM1->CTRL)

#define ECC_IFC0_REG        (DMEM0->IF_CLR)
#define ECC_IFC1_REG        (DMEM1->IF_CLR)
#define ECC_IFC_MASK        (_MPAHBRAM_IF_MASK)

#define ECC_FAULT_CTRL0_REG (DMEM0->CTRL)
#define ECC_FAULT_CTRL1_REG (DMEM1->CTRL)
#define ECC_FAULT_EN        (MPAHBRAM_CTRL_ECCERRFAULTEN)

#else

#define ECC_RAM0_MEM_BASE  (SRAM_BASE)
#define ECC_RAM0_MEM_SIZE  (SRAM_SIZE)

#if (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) \
  || (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) && !defined(_MPAHBRAM_CTRL_MASK))\
  || (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11) && !defined(_MPAHBRAM_CTRL_MASK)))
#define ECC_CTRL_REG       (SYSCFG->DMEM0ECCCTRL)
#define ECC_IFC_REG        (SYSCFG->IF_CLR)
#define ECC_IFC_MASK       (SYSCFG_IF_RAMERR1B | SYSCFG_IF_RAMERR2B)
#define ECC_FAULT_CTRL_REG (SYSCFG->CTRL)
#define ECC_FAULT_EN       (SYSCFG_CTRL_RAMECCERRFAULTEN)

#elif defined(_MPAHBRAM_CTRL_MASK)
#define ECC_CTRL_REG       (DMEM->CTRL)
#define ECC_IFC_REG        (DMEM->IF_CLR)
#define ECC_IFC_MASK       (_MPAHBRAM_IF_MASK)
#define ECC_FAULT_CTRL_REG (DMEM->CTRL)
#define ECC_FAULT_EN       (MPAHBRAM_CTRL_ECCERRFAULTEN)
#endif

#endif /* defined(DMEM_COUNT) && (DMEM_COUNT == 2) */

#else
#error Unknown device.
#endif

#define ECC_DMA_MAX_XFERCNT (_LDMA_CH_CTRL_XFERCNT_MASK \
                             >> _LDMA_CH_CTRL_XFERCNT_SHIFT)
#define ECC_DMA_DESC_SIZE   ((ECC_DMA_MAX_XFERCNT + 1) * 4)  // 4 bytes units.

#define ECC_DMA_DESCS       (ECC_RAM_SIZE_MAX / ECC_DMA_DESC_SIZE)

#endif /* #if defined(_SYSCFG_DMEM0ECCCTRL_MASK) || #if defined(_MPAHBRAM_CTRL_MASK) */

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) \
  || defined(_MPAHBRAM_CTRL_MASK)
typedef struct {
  uint32_t           init_syndrome_enable;
  uint32_t           correction_enable;
  uint32_t           base;
  uint32_t           size;
} sli_hal_syscfg_ecc_bank_t;
#endif

/*******************************************************************************
 **************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) \
  || defined(_MPAHBRAM_CTRL_MASK)
static const sli_hal_syscfg_ecc_bank_t ecc_bank_tbl[SL_HAL_SYSCFG_ECC_BANKS] =
{
  {
    ECC_RAM0_SYNDROMES_INIT, ECC_RAM0_CORRECTION_EN,
    ECC_RAM0_MEM_BASE, ECC_RAM0_MEM_SIZE
  },
#if SL_HAL_SYSCFG_ECC_BANKS > 1
  {
    ECC_RAM1_SYNDROMES_INIT, ECC_RAM1_CORRECTION_EN,
    ECC_RAM1_MEM_BASE, ECC_RAM1_MEM_SIZE
  },
#if SL_HAL_SYSCFG_ECC_BANKS > 2
  {
    ECC_RAM2_SYNDROMES_INIT, ECC_RAM2_CORRECTION_EN,
    ECC_RAM2_MEM_BASE, ECC_RAM2_MEM_SIZE
  },
#endif
#endif
};
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) || defined(_MPAHBRAM_CTRL_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11) \
  || defined(_MPAHBRAM_CTRL_MASK)
static void sli_hal_syscfg_ecc_read_write_existing_pio(const sli_hal_syscfg_ecc_bank_t *ecc_bank);
#else
static void sli_hal_syscfg_ecc_read_write_existing_dma(uint32_t start,
                                                       uint32_t size,
                                                       const uint32_t channels[2]);
#endif
static void sli_hal_syscfg_ecc_init_bank(const sli_hal_syscfg_ecc_bank_t *ecc_bank,
                                         const uint32_t dma_channels[2]);
static void sli_hal_syscfg_ecc_disable_bank(const sli_hal_syscfg_ecc_bank_t *ecc_bank);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
#if !defined(SL_TRUSTZONE_NONSECURE)
#if defined(_SYSCFG_CHIPREV_FAMILY_MASK) || defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)
extern __INLINE uint32_t sl_hal_syscfg_read_chip_rev(void);
#endif

#if defined(_SYSCFG_CFGSYSTIC_MASK)
extern __INLINE void sl_hal_syscfg_set_systicextclken_cfgsystic(void);
extern __INLINE void sl_hal_syscfg_clear_systicextclken_cfgsystic(void);
#endif

#if defined(_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK)
extern __INLINE void sl_hal_syscfg_set_dmem0ramctrl_ramwsen_bit(void);
extern __INLINE void sl_hal_syscfg_clear_dmem0ramctrl_ramwsen_bit(void);
extern __INLINE uint32_t sl_hal_syscfg_get_dmem0ramctrl_ramwsen_bit(void);
#endif
#if defined(_SYSCFG_DMEM0RETNCTRL_MASK)
extern __INLINE uint32_t sl_hal_syscfg_read_dmem0retnctrl(void);
extern __INLINE void sl_hal_syscfg_mask_dmem0retnctrl(uint32_t mask);
extern __INLINE void sl_hal_syscfg_zero_dmem0retnctrl(void);
#endif
#endif

#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) \
  || defined(_MPAHBRAM_CTRL_MASK)

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11) \
  || defined(_MPAHBRAM_CTRL_MASK)

/***************************************************************************//**
 * Read and write existing values in RAM (for ECC initialization).
 *
 * @param[in] ecc_bank Pointer to ECC RAM bank (sli_hal_syscfg_ecc_bank_t)
 *
 * @details This function uses core to load and store the existing data
 *    values in the given RAM bank.
 ******************************************************************************/
static void sli_hal_syscfg_ecc_read_write_existing_pio(const sli_hal_syscfg_ecc_bank_t *ecc_bank)
{
  EFM_ASSERT(ecc_bank != NULL);
  EFM_ASSERT(ecc_bank->base < (ecc_bank->base + ecc_bank->size));

  volatile const uint32_t *ram_ptr = (volatile const uint32_t *) ecc_bank->base;
  const uint32_t *end_ptr = (const uint32_t *) (ecc_bank->base + ecc_bank->size);
  volatile const uint32_t *ctrl_reg;
  uint32_t enable_ecc;

#if defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  if (ecc_bank->base == ECC_RAM0_MEM_BASE) {
    ctrl_reg = &ECC_CTRL0_REG;
  } else if (ecc_bank->base == ECC_RAM1_MEM_BASE) {
    ctrl_reg = &ECC_CTRL1_REG;
  } else {
    EFM_ASSERT(0);
    return;
  }
#else
  ctrl_reg = &ECC_CTRL_REG;
#endif /* defined(DMEM_COUNT) && (DMEM_COUNT == 2) */

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) \
  || (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) && !defined(_MPAHBRAM_CTRL_MASK)) \
  || (defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11) && !defined(_MPAHBRAM_CTRL_MASK))
  enable_ecc = ecc_bank->init_syndrome_enable;
#elif defined(_MPAHBRAM_CTRL_MASK)
  // MPAHBRAM ECC requires both ECCEN and ECCWEN to be set for the syndromes
  // to be written in ECC.
  enable_ecc = ecc_bank->correction_enable;

#if defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  // Enable ECC syndrome write.
  if (ecc_bank->base == ECC_RAM0_MEM_BASE) {
    ECC_CTRL0_REG |= ecc_bank->init_syndrome_enable;
    ECC_IFC0_REG = ECC_IFC_MASK;
  } else if (ecc_bank->base == ECC_RAM1_MEM_BASE) {
    ECC_CTRL1_REG |= ecc_bank->init_syndrome_enable;
    ECC_IFC1_REG = ECC_IFC_MASK;
  }
#else
  ECC_CTRL_REG |= ecc_bank->init_syndrome_enable;
  ECC_IFC_REG = ECC_IFC_MASK;
#endif /* defined(DMEM_COUNT) && (DMEM_COUNT == 2) */

#endif

#ifndef __GNUC__
#define __asm__        asm
#endif

  // Performs a read and write of all RAM address to initialize
  // ECC syndromes. ECC is initialized by reading a RAM address
  // while ECC is disabled and write it back while ECC is enabled.
  //
  // HardFault could occur if we try to read values from RAM while ECC
  // is enabled and not initialized. In this case, ECC tries to correct the
  // value and ended giving the wrong value which could be sometimes an
  // non-existing address.
  //
  // So for ECC initialization to work properly, this must ensures that while
  // ECC is enabled, RAM will be accessed only through writes, no reads shall
  // occur. It's hard to have such guarantee with C code, because the C
  // compiler with optimization settings, can get in the way
  // and do some unwanted reads while ECC is enabled. Assembly allows such
  // guarantee and let ECC be initialized without triggering errors.

  __asm__ volatile (
    "1:\n\t"                              // Define label 1.
    "LDR r1, [%[ram_ptr]]\n\t"            // Load content of ram_ptr into R1, ECC
                                          //   is disabled to get a correct value.
    "LDR r0, [%[ctrl_reg]]\n\t"           // Load ctrl_reg content into R0
    "ORR r0, r0, %[enable_ecc]\n\t"       // OR R0 and enable_ecc, and store result
                                          //   in R0
    "STR r0, [%[ctrl_reg]]\n\t"           // Write R0 into ctrl_reg, ECC is
                                          //   enabled from now on
    "STR r1, [%[ram_ptr]]\n\t"            // Write back ram content where it was,
                                          //  syndrome will be written in ECC
    "BIC r0, r0, %[enable_ecc]\n\t"       // Bit clear enable_ecc from R0, and store
                                          //   result in R0.
    "STR r0, [%[ctrl_reg]]\n\t"           // Write R0 into ctrl_reg, ECC is
                                          //   disabled.
    "ADDS %[ram_ptr], %[ram_ptr], #4\n\t" // Increment ram_ptr by 4 (size of
                                          //   a word).
    "CMP %[ram_ptr], %[end_ptr]\n\t"      // Compare ram_ptr and end_ptr...
    "BCC 1b\n\t"                          //... and jump back to label 1 if Carrry.
                                          // Clear (meaning ram_ptr < end_ptr).
    "ORR r0, r0, %[enable_ecc]\n\t"       // And re-enable ECC ASAP to be sure no.
    "STR r0, [%[ctrl_reg]]\n\t"           // STR occurs with ECC disabled.
    :[ram_ptr] "+r" (ram_ptr)
    :[end_ptr] "r" (end_ptr),
    [ctrl_reg] "r" (ctrl_reg),
    [enable_ecc] "r" (enable_ecc)
    : "r0", "r1",                         // R0 and R1 used as temporary registers.
    "memory"                              // Memory pointed by ram_ptr is modified.
    );
}

#else

/***************************************************************************//**
 * DMA read and write existing values (for ECC initialization).
 *
 * @param[in] start Start address of address range in RAM to read/write.
 * @param[in] size Size of address range in RAM to read/write.
 * @param[in] channels[2] Array of 2 DMA channels to use.
 *
 * @details This function uses DMA to read and write the existing data values in
 *          the RAM region specified by start and size. The function will use the
 *          2 DMA channels specified by the channels[2] array.
 ******************************************************************************/
static void sli_hal_syscfg_ecc_read_write_existing_dma(uint32_t start,
                                                       uint32_t size,
                                                       const uint32_t channels[2])
{
  // Assert that the 2 DMA channel numbers are different.
  EFM_ASSERT(channels[0] != channels[1]);
  // Make sure ECC_RAM_SIZE_MAX is a multiple of ECC_DMA_DESC_SIZE in order
  // to match the total xfer size of the descriptor chain with the largest
  // ECC RAM bank.
  EFM_ASSERT((ECC_RAM_SIZE_MAX % ECC_DMA_DESC_SIZE) == 0);

  uint32_t desc_cnt = 0;
  volatile uint32_t dma_desc[ECC_DMA_DESCS][4];
  uint32_t ch_mask = (1 << channels[0]) | (1 << channels[1]);

  // Initialize LDMA descriptor chain.
  do {
    dma_desc[desc_cnt][0] = (LDMA_CH_CTRL_STRUCTTYPE_TRANSFER  // DMA desc CTRL word.
                             | LDMA_CH_CTRL_STRUCTREQ
                             | _LDMA_CH_CTRL_XFERCNT_MASK
                             | LDMA_CH_CTRL_BLOCKSIZE_ALL
                             | LDMA_CH_CTRL_REQMODE_ALL
                             | LDMA_CH_CTRL_SRCINC_ONE
                             | LDMA_CH_CTRL_SIZE_WORD
                             | LDMA_CH_CTRL_DSTINC_ONE);

    // source and destination address.
    dma_desc[desc_cnt][1] = start;
    dma_desc[desc_cnt][2] = start;
    // link to next descriptor.
    dma_desc[desc_cnt][3] = LDMA_CH_LINK_LINK
                            | (((uint32_t) &dma_desc[desc_cnt + 1][0])
                               & _LDMA_CH_LINK_LINKADDR_MASK);

    start += ECC_DMA_DESC_SIZE;
    size  -= ECC_DMA_DESC_SIZE;
    desc_cnt++;
  } while (size);

  // Make sure desc_cnt is valid to avoid out-of-bounds access when writing to
  // dma_desc array.
  if ((desc_cnt < 2) || (desc_cnt > ECC_DMA_DESCS)) {
    while (true) {
      EFM_ASSERT(false);
    }
  }
  // Now, divide the descriptor list in two parts, one for each channel,
  // by setting the link bit and address 0 of the descriptor in the middle
  // to 0.
  dma_desc[(desc_cnt / 2) - 1][3] = 0;

  // Set last descriptor link bit and address to 0.
  dma_desc[desc_cnt - 1][3] = 0;
  // Round robin scheduling for all channels (0 = no fixed priority channels).
  LDMA->CTRL    = 0 << _LDMA_CTRL_NUMFIXED_SHIFT;
  LDMA->EN      = LDMA_EN_EN;
  LDMA->CHEN    = 0;
  LDMA->DBGHALT = 0;
  LDMA->REQDIS  = 0;

  // Disable LDMA interrupts, and clear interrupt status.
  LDMA->IEN = 0;
  LDMA->IF_CLR = ch_mask;

  // Disable looping.
  LDMA->CH[channels[0]].LOOP = 0;
  LDMA->CH[channels[1]].LOOP = 0;

  // Set descriptor address for first channel.
  LDMA->CH[channels[0]].LINK = ((uint32_t)&dma_desc[0][0])
                               & _LDMA_CH_LINK_LINKADDR_MASK;
  // Set descriptor address for second channel.
  LDMA->CH[channels[1]].LINK = ((uint32_t)&dma_desc[desc_cnt / 2][0])
                               & _LDMA_CH_LINK_LINKADDR_MASK;
  // Clear the channel done flags.
  sl_hal_bus_reg_clear_mask(&LDMA->CHDONE, ch_mask);

  // Start transfer by loading descriptors.
  LDMA->LINKLOAD = ch_mask;
  while (!(((LDMA->CHSTATUS & ch_mask) == 0)
           && ((LDMA->CHDONE & ch_mask) == ch_mask))) {
    // Wait until finished.
  }
}
#endif /* #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2) || defined(_MPAHBRAM_CTRL_MASK) */

/***************************************************************************//**
 * This function initializes ECC for a given memory bank which is specified
 * with the sli_hal_syscfg_ecc_bank_t structure input parameter.
 *
 * @param[in] ecc_bank ECC memory bank device structure.
 * @param[in] dma_channels Array of 2 DMA channels that may be used during ECC
 *                         initialization.
 ******************************************************************************/
static void sli_hal_syscfg_ecc_init_bank(const sli_hal_syscfg_ecc_bank_t *ecc_bank,
                                         const uint32_t dma_channels[2])
{
  EFM_ASSERT(ecc_bank != NULL);
  EFM_ASSERT(dma_channels[0] != dma_channels[1]);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)  \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11) \
  || defined(_MPAHBRAM_CTRL_MASK)

  (void) dma_channels;
#if !defined(_MPAHBRAM_CTRL_MASK)
  // Disable ECC write.
  ECC_CTRL_REG &= ~ecc_bank->init_syndrome_enable;
#endif
  // Initialize ECC syndromes by using core cpu to load and store the existing
  // data values in RAM.
  sli_hal_syscfg_ecc_read_write_existing_pio(ecc_bank);
#else
  // Enable ECC write.
  ECC_CTRL_REG |= ecc_bank->init_syndrome_enable;
  // Initialize ECC syndromes by using DMA to read and write the existing
  // data values in RAM.
  sli_hal_syscfg_ecc_read_write_existing_dma(ecc_bank->base, ecc_bank->size, dma_channels);
#endif

  // Clear any ECC errors that may have been reported before or during
  // initialization.
#if defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  if (ecc_bank->base == ECC_RAM0_MEM_BASE) {
    ECC_IFC0_REG = ECC_IFC_MASK;
  } else if (ecc_bank->base == ECC_RAM1_MEM_BASE) {
    ECC_IFC1_REG = ECC_IFC_MASK;
  }
#else
  ECC_IFC_REG = ECC_IFC_MASK;
#endif /* defined(DMEM_COUNT) && (DMEM_COUNT == 2) */

#if !defined(_MPAHBRAM_CTRL_MASK)
  // Enable ECC decoder to detect and report ECC errors.
  ECC_CTRL_REG |= ecc_bank->correction_enable;
#endif

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * Disable ECC for a given memory bank.
 *
 * @param[in] ecc_bank ECC memory bank device structure.
 ******************************************************************************/
static void sli_hal_syscfg_ecc_disable_bank(const sli_hal_syscfg_ecc_bank_t *ecc_bank)
{
  EFM_ASSERT(ecc_bank != NULL);

#if defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  // Disable ECC write (encoder) and checking (decoder).
  if (ecc_bank->base == ECC_RAM0_MEM_BASE) {
    ECC_CTRL0_REG &= ~(ecc_bank->init_syndrome_enable | ecc_bank->correction_enable);
  } else if (ecc_bank->base == ECC_RAM1_MEM_BASE) {
    ECC_CTRL1_REG &= ~(ecc_bank->init_syndrome_enable | ecc_bank->correction_enable);
  }
#else
  ECC_CTRL_REG &= ~(ecc_bank->init_syndrome_enable | ecc_bank->correction_enable);
#endif /* defined(DMEM_COUNT) && (DMEM_COUNT == 2) */
}

/***************************************************************************//**
 * Configure Error Correcting Code (ECC).
 ******************************************************************************/
void sl_hal_syscfg_ecc_set_config(const sl_hal_syscfg_ecc_config_t *ecc_config)
{
  EFM_ASSERT(ecc_config != NULL);

#if defined(ECC_FAULT_CTRL_REG)
  uint32_t fault_ctrl_reg = ECC_FAULT_CTRL_REG;
  // Disable ECC faults if ecc fault ctrl register is defined.
  fault_ctrl_reg &= ~ECC_FAULT_EN;
  ECC_FAULT_CTRL_REG = fault_ctrl_reg;
#endif

  // Loop through the ECC banks array, enable or disable according to
  // the ecc_config->enable_ecc_bank array.
  for (uint32_t cnt = 0; cnt < SL_HAL_SYSCFG_ECC_BANKS; cnt++) {
    if (ecc_config->enable_ecc_bank[cnt]) {
      sli_hal_syscfg_ecc_init_bank(&ecc_bank_tbl[cnt], ecc_config->dma_channels);
    } else {
      sli_hal_syscfg_ecc_disable_bank(&ecc_bank_tbl[cnt]);
    }
  }
}
#endif /* #if defined(_SYSCFG_DMEM0ECCCTRL_MASK) || defined(_MPAHBRAM_CTRL_MASK) */

#if defined(_SYSCFG_DMEM0PORTMAPSEL_MASK)
/***************************************************************************//**
 * Set MPAHBRAM port to use to access DMEM.
 ******************************************************************************/
void sl_hal_syscfg_set_dmem_port_map(sl_hal_syscfg_dmem_master_t master,
                                     uint8_t port)
{
  // make sure master is within the mask of port map that can be changed
  // and make sure port is a sensible value.
  EFM_ASSERT(((1 << master) & _SYSCFG_DMEM0PORTMAPSEL_MASK) != 0x0);
#if defined(DMEM_COUNT) && (DMEM_COUNT == 1)
  EFM_ASSERT(port < DMEM_NUM_PORTS);
#elif defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  EFM_ASSERT(port < DMEM0_NUM_PORTS);
#endif

#if defined(DMEM_COUNT) && (DMEM_COUNT == 1)
  uint32_t bit_field_mask = DMEM_NUM_PORTS - 1;
#elif defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  uint32_t bit_field_mask = DMEM0_NUM_PORTS - 1;
#endif

  sl_hal_bus_reg_write_mask(&SYSCFG->DMEM0PORTMAPSEL,
                            bit_field_mask << master,
                            (uint32_t)port << master);
}
#endif

#if defined(_MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK)
/***************************************************************************//**
 * Set MPAHBRAM port priority for arbitration when multiple concurrent
 * transactions to DMEM.
 ******************************************************************************/
void sl_hal_syscfg_set_port_priority(sl_hal_syscfg_port_priority_t port_priority)
{
#if defined(DMEM_COUNT) && (DMEM_COUNT == 1)
  EFM_ASSERT(port_priority < ((DMEM_NUM_PORTS + 1) << _MPAHBRAM_CTRL_AHBPORTPRIORITY_SHIFT));

  sl_hal_bus_reg_write_mask(&DMEM->CTRL,
                            _MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK,
                            (uint32_t)port_priority << _MPAHBRAM_CTRL_AHBPORTPRIORITY_SHIFT);
#elif defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  EFM_ASSERT(port_priority < ((DMEM0_NUM_PORTS + 1) << _MPAHBRAM_CTRL_AHBPORTPRIORITY_SHIFT));

  sl_hal_bus_reg_write_mask(&DMEM0->CTRL,
                            _MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK,
                            (uint32_t)port_priority << _MPAHBRAM_CTRL_AHBPORTPRIORITY_SHIFT);
  sl_hal_bus_reg_write_mask(&DMEM1->CTRL,
                            _MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK,
                            (uint32_t)port_priority << _MPAHBRAM_CTRL_AHBPORTPRIORITY_SHIFT);
#endif
}

/***************************************************************************//**
 * Get MPAHBRAM port arbitration priority selection.
 ******************************************************************************/
sl_hal_syscfg_port_priority_t sl_hal_syscfg_get_port_priority(void)
{
  uint32_t port = 0;

#if defined(DMEM_COUNT) && (DMEM_COUNT == 1)
  port = sl_hal_bus_reg_read_mask(&DMEM->CTRL,
                                  _MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK);
#elif defined(DMEM_COUNT) && (DMEM_COUNT == 2)
  port = sl_hal_bus_reg_read_mask(&DMEM0->CTRL,
                                  _MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK);
#endif

  return (sl_hal_syscfg_port_priority_t)(port >> _MPAHBRAM_CTRL_AHBPORTPRIORITY_SHIFT);
}
#endif /* if defined(_MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK) */

/** @} (end addtogroup system config) */
