/***************************************************************************//**
 * @file
 * @brief TZ Non-Secure Execution Start-up
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "em_device.h"
#include "sl_common.h"

#if !defined(__ARM_FEATURE_CMSE) || (__ARM_FEATURE_CMSE != 3U)
  #error "The TZ Non-Secure execution code requires access to the CMSE toolchain extension to set proper SAU settings."
#endif // __ARM_FEATURE_CMSE

/*---------------------------------------------------------------------------
 * Defines
 *---------------------------------------------------------------------------*/

#define LINK_TIME_INJECTED_DATA_PATTERN   0x0DF0ADBA /* 0xBAADF00D backwards*/
#define TOTAL_INTERNAL_INTERRUPTS         (16)

#define THUMB_INSTRUCTION_BIT               0x00000001
#define VECTOR_TABLE_ENTRY_OFFSET(x)        ((uint32_t)(x) + (uint32_t)(THUMB_INSTRUCTION_BIT))

// Linker section definition.
#if defined (__GNUC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#define __ATTRIBUTE_SECURE_VECTORS        __attribute__((used, section(".secure_vectors")))
#define __ATTRIBUTE_SECURE_CONFIG_DATA    __attribute__((used, section(".secure_config_data")))
#define __ATTRIBUTE_SECURE_RESET_HANDLER  __attribute__((used, section(".secure_reset_handler")))
#define __ATTRIBUTE_SECURE_FAULT_HANDLER  __attribute__((used, section(".secure_fault_handler")))
#define __ATTRIBUTE_SECURE_VECTORS_COPY   __attribute__((used, section(".secure_vectors_copy")))
#define __NO_PROLOGUE                     __attribute__((naked))

#elif defined(__ICCARM__)

#pragma data_alignment=512
#define __ATTRIBUTE_SECURE_VECTORS        SL_ATTRIBUTE_SECTION("secure_vectors")
#define __ATTRIBUTE_SECURE_CONFIG_DATA    SL_ATTRIBUTE_SECTION("secure_config_data")
#define __ATTRIBUTE_SECURE_RESET_HANDLER  SL_ATTRIBUTE_SECTION("secure_reset_handler")
#define __ATTRIBUTE_SECURE_FAULT_HANDLER  SL_ATTRIBUTE_SECTION("secure_fault_handler")
#define __ATTRIBUTE_SECURE_VECTORS_COPY   SL_ATTRIBUTE_SECTION("secure_vectors_copy")
#define __NO_PROLOGUE                     __naked

#else
#error "Unsupported compiler."
#endif

// Initial Stack Pointer.
extern uint32_t __INITIAL_SP;

// App properties.
#if defined(SL_APP_PROPERTIES)
extern ApplicationProperties_t sl_app_properties;
#define APP_PROPERTIES_ADDR (void(*)(void)) & sl_app_properties
#else
#define APP_PROPERTIES_ADDR (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler))
#endif

// This data is injected inline with the secure app's binary blob at link-time.
typedef struct secure_config_data {
  uint32_t *secure_vector_table;
  uint32_t *non_secure_vector_table;
  uint32_t mspu_region_size;
} secure_config_data_t;

/*---------------------------------------------------------------------------
 * Internal References
 *---------------------------------------------------------------------------*/

void sli_tz_secure_reset_handler(void);
void sli_tz_secure_fault_handler(void);

#if !defined(SL_TZ_NON_SECURE_EXECUTION_USE_SOURCE)
__ATTRIBUTE_SECURE_CONFIG_DATA secure_config_data_t sl_tz_secure_config_data = {
  .secure_vector_table = (uint32_t *)LINK_TIME_INJECTED_DATA_PATTERN,
  .non_secure_vector_table = (uint32_t *)LINK_TIME_INJECTED_DATA_PATTERN,
  .mspu_region_size = LINK_TIME_INJECTED_DATA_PATTERN
};
#else
extern uint32_t __Secure_Vectors;
extern uint32_t linker_vectors_begin;
extern uint32_t __mspu_region_size__;

__ATTRIBUTE_SECURE_CONFIG_DATA const secure_config_data_t sl_tz_secure_config_data = {
  .secure_vector_table = (uint32_t*) (((uint32_t)&__Secure_Vectors) /* + 0x10000000*/),
  .non_secure_vector_table = (uint32_t*) &linker_vectors_begin,
  .mspu_region_size = (uint32_t)&__mspu_region_size__
};
#endif

#if defined (__GNUC__)
#pragma GCC diagnostic pop
#endif // __GNUC__

/*---------------------------------------------------------------------------
 * Secure Reset Handler called on controller reset
 *---------------------------------------------------------------------------*/
__ATTRIBUTE_SECURE_RESET_HANDLER __NO_PROLOGUE void sli_tz_secure_reset_handler(void)
{
/*
 * This code is meant to be pre-compiled and used as a binary blob in applications
 * that require the tz_non_secure_execution component. This source serves as a
 * reference to audit the code that is contained in the binary blob.
 */

/* The Non-Secure execution code takes care of moving between the security states.
 * It does all the essential configuration to run an application in Non-Secure
 * mode without the possibility of switching back and forth.
 *
 * The code below makes sure that all the peripherals are accessed from non-secure
 * address except SMU while also configuring the core to execute in Non-Secure mode.
 * The MSPU is configured so that all region in the DMEM and all NVM regions are Non-Secure
 *  except for the one NVM region in charge of switching to Non-Secure.
 */

  // Set Secure Vector Table.
  SCB->VTOR = (uint32_t)sl_tz_secure_config_data.secure_vector_table;

  // Set Non-Secure Stack pointer.
  __TZ_set_MSP_NS(*(sl_tz_secure_config_data.non_secure_vector_table));

  // Set Non-Secure vector table.
  SCB_NS->VTOR = (uint32_t)sl_tz_secure_config_data.non_secure_vector_table;

  // Enable SecureFault
  SCB->SHCSR |= SCB_SHCSR_SECUREFAULTENA_Msk;

  // Enable BusFault, HardFault and NMI triggering in Non-Secure mode.
  // 0x5FA need to be written to the Vector Key field to write in the AIRCR register.
  SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_BFHFNMINS_Msk);

  // Enables Non-Secure access to the Floating-Point Extension.
#if defined(SCB_NSACR_CPn_Msk)
  SCB->NSACR |= (SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk | SCB_NSACR_CPn_Msk);
#else
  SCB->NSACR |= (SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk);
#endif

  // Set all Interrupt Non-Secure State (ITNS) bits. This results
  // in all IRQs being targeted at the NS world.
  __ASM volatile (

    // for (uint8_t i = 0; i < 16; i++) {
    //  NVIC->ITNS = 0xFFFFFFFF;
    // }

    "MOVS         R3, #0               \n"
    "MOV          R2, #0xFFFFFFFF      \n"
    "MOV          R1, %[itns]          \n"
    "nvic_itns_loop:                   \n"
    "STR          R2, [R1, R3, LSL #2] \n"
    "ADDS         R3, #1               \n"
    "CMP          R3, #16              \n"
    "BNE          nvic_itns_loop       \n"
    : // No outputs
    :[itns] "r" (&NVIC->ITNS)
    : "r1", "r2", "r3", "r5"
    );

  // Enable necessary clocks.
#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  CMU->CLKEN1_SET = CMU_CLKEN1_SMU | CMU_CLKEN1_L2ICACHE0 | CMU_CLKEN1_ICACHE0;
#else
  CMU->SMUCLKCTRL_SET = CMU_SMUCLKCTRL_CLKEN;
  CMU->L2ICACHE0CLKCTRL_SET = CMU_L2ICACHE0CLKCTRL_CLKEN;
  CMU->SYSCFGCLKCTRL_SET = CMU_SYSCFGCLKCTRL_CLKEN;
#endif

  // Enable Faults on BMPU and PPU access.
#if defined(_SMU_BMPURSPERRD0_MASK)
  SMU->BMPURSPERRD0_SET = _SMU_BMPURSPERRD0_MASK;
#elif defined(_SMU_BMPURSPERRD_MASK)
  SMU->BMPURSPERRD_SET = _SMU_BMPURSPERRD_MASK;
#endif
  SMU->PPURSPERRD_SET = _SMU_PPURSPERRD_MASK;

  // Set all Bus Master to Non-Secure.
  SMU->BMPUSATD0_CLR = _SMU_BMPUSATD0_MASK;

  // Set NVM MSPU region's size based on actual region's size in the linker.
  switch (sl_tz_secure_config_data.mspu_region_size) {
    case 0x4000:
#if defined(SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_16KB)
      SMU->MSPUNVMREGIONSIZE_SET = SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_16KB;
#elif defined(SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_16KB)
      SMU->MSPUOSPI0CTRL_SET = SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_16KB;
#endif
      break;
    case 0x8000:
#if defined(SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_32KB)
      SMU->MSPUNVMREGIONSIZE_SET = SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_32KB;
#elif defined(SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_32KB)
      SMU->MSPUOSPI0CTRL_SET = SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_32KB;
#endif
      break;
    case 0x10000:
#if defined(SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_64KB)
      SMU->MSPUNVMREGIONSIZE_SET = SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_64KB;
#elif defined(SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_64KB)
      SMU->MSPUOSPI0CTRL_SET = SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_64KB;
#endif
      break;
    case 0x20000:
#if defined(SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_128KB)
      SMU->MSPUNVMREGIONSIZE_SET = SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_128KB;
#elif defined(SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_128KB)
      SMU->MSPUOSPI0CTRL_SET = SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_128KB;
#endif
      break;
    case 0x40000:
#if defined(SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_256KB)
      SMU->MSPUNVMREGIONSIZE_SET = SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_256KB;
#elif defined(SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_256KB)
      SMU->MSPUOSPI0CTRL_SET = SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_256KB;
#endif
      break;
    case 0x80000:
#if defined(SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_512KB)
      SMU->MSPUNVMREGIONSIZE_SET = SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_512KB;
#elif defined(SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_512KB)
      SMU->MSPUOSPI0CTRL_SET = SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_512KB;
#endif
      break;
    default:
#if defined(SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_16KB)
      SMU->MSPUNVMREGIONSIZE_SET = SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_16KB;
#elif defined(SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_16KB)
      SMU->MSPUOSPI0CTRL_SET = SMU_MSPUOSPI0CTRL_SECTORSIZE_MSPU_16KB;
#endif
  }

  // Set all RAM regions to Non-Secure.
#if defined(_SMU_MSPUDMEMNSREGIONFLAG0_MASK)
  SMU->MSPUDMEMNSREGIONFLAG0_SET = _SMU_MSPUDMEMNSREGIONFLAG0_MASK;
#elif defined(_SMU_MSPUDMEMSATD0_MASK)
  SMU->MSPUDMEMSATD0_CLR = _SMU_MSPUDMEMSATD0_MASK;
  SMU->MSPUDMEMSATD1_CLR = _SMU_MSPUDMEMSATD1_MASK;
#endif

  // Set all PSRAM regions to Non-Secure.
#if defined(_SMU_MSPUOSPI1SATD0_MASK)
  SMU->MSPUOSPI1SATD0_CLR = _SMU_MSPUOSPI1SATD0_MASK;
  SMU->MSPUOSPI1SATD1_CLR = _SMU_MSPUOSPI1SATD1_MASK;
  SMU->MSPUOSPI1SATD2_CLR = _SMU_MSPUOSPI1SATD2_MASK;
  SMU->MSPUOSPI1SATD3_CLR = _SMU_MSPUOSPI1SATD3_MASK;
#endif

  // Set all ITCM and DTCM regions to Non-Secure.
#if defined(MEMSYSCTL_ITGU_CFG_PRESENT_Msk) || defined(MEMSYSCTL_DTGU_CFG_PRESENT_Msk)
  for (uint32_t i = 0; i < 16; i++) {
  #if defined(MEMSYSCTL_ITGU_CFG_PRESENT_Msk)
    MEMSYSCTL->ITGU_LUT[i] = 0xFFFFFFFF;
  #endif
  #if defined(MEMSYSCTL_DTGU_CFG_PRESENT_Msk)
    MEMSYSCTL->DTGU_LUT[i] = 0xFFFFFFFF;
  #endif
  }
#endif

  // Fetch the Non-Secure Reset Handler's address and save it in a register.
  __ASM volatile (
    "MOV            R3, %[address_ns]   \n"
    "LDR            R4, [R3, #4]        \n"
    : // No outputs
    :[address_ns] "r" (sl_tz_secure_config_data.non_secure_vector_table)
    : "r3", "r4", "r5"
    );

  // Flush both Cache before switching to Non-Secure.
  __ASM volatile (
#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301) // There seems to be a bug with the L1ICACHE0 invalidation
    // L1ICACHE0->CMD = ICACHE_CMD_INVALIDATE;
    "MOVS           R2, %[L1icache_flush] \n"
    "MOV            R3, %[L1icache_cmd]   \n"
    "STR            R2, [R3]              \n"
#endif

    // L2ICACHE0->FLUSHCMD = L2CACHE_FLUSHCMD_FLUSHALL;
    "MOVS           R2, %[L2icache_flush]     \n"
    "MOV            R3, %[L2icache_flushcmd]  \n"
    "STR            R2, [R3]									\n"

    // while (L2ICACHE0->STATUS & L2CACHE_STATUS_FLUSHRUNNING) ;
    "l2cache_flushrunning_loop:               \n"
    "LDR            R2, [R3]                  \n"
    "AND            R2, R2, %[L2icache_flush] \n"
    "CMP            R2, #0                    \n"
    "BNE            l2cache_flushrunning_loop \n"

    : // No outputs
    :
#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
    [L1icache_cmd] "r" (&L1ICACHE0->CMD_SET),
    [L1icache_flush] "r" (ICACHE_CMD_INVALIDATE),
#endif
#if defined(L2CACHE_FLUSHCMD_FLUSHALL)
    [L2icache_flushcmd] "r" (&L2ICACHE0->FLUSHCMD_SET),
    [L2icache_flush] "r" (L2CACHE_FLUSHCMD_FLUSHALL),
    [L2icache_status] "r" (&L2ICACHE0->STATUS),
    [L2icache_flushrunning] "r" (L2CACHE_STATUS_FLUSHRUNNING)
#else
    [L2icache_flushcmd] "r" (&L2ICACHE0->FLUSHCMD_SET),
    [L2icache_flush] "r" (L2ICACHE_FLUSHCMD_FLUSHALL),
    [L2icache_status] "r" (&L2ICACHE0->STATUS),
    [L2icache_flushrunning] "r" (L2ICACHE_STATUS_FLUSHRUNNING)
#endif
    : "r2", "r3", "r4", "r5"
    );

  // Disable the clock for the Caches' modules.
#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
  __ASM volatile (
    // CMU->CLKEN1_CLR = CMU_CLKEN1_L2ICACHE0 | CMU_CLKEN1_ICACHE0;
    "MOV      R3, %[clken1_clr]         \n"
    "MOV      R2, %[mask]               \n"
    "STR      R2, [R3]                  \n"
    : // No outputs.
    :[clken1_clr] "r" (&CMU->CLKEN1_CLR),
    [mask] "r" (CMU_CLKEN1_L2ICACHE0 | CMU_CLKEN1_ICACHE0)
    : "r2", "r3", "r4"
    );
#else
  __ASM volatile (
    // CMU->L2ICACHE0CLKCTRL_CLR = CMU_L2ICACHE0CLKCTRL_CLKEN;
    "MOV      R3, %[l2icache_clr]       \n"
    "MOV      R2, %[l2icache_mask]      \n"
    "STR      R2, [R3]                  \n"

    // CMU->SYSCFGCLKCTRL_CLR = CMU_SYSCFGCLKCTRL_CLKEN;
    "MOV      R3, %[syscfg_clr]       \n"
    "MOV      R2, %[syscfg_mask]      \n"
    "STR      R2, [R3]                  \n"
    : // No outputs.
    :[l2icache_clr] "r" (&CMU->L2ICACHE0CLKCTRL_CLR),
    [l2icache_mask] "r" (CMU_L2ICACHE0CLKCTRL_CLKEN),
    [syscfg_clr] "r" (&CMU->SYSCFGCLKCTRL_CLR),
    [syscfg_mask] "r" (CMU_SYSCFGCLKCTRL_CLKEN)
    : "r2", "r3", "r4"
    );
#endif

  // Configure all peripherals to Non-Secure except for SMU.
  __ASM volatile (

    // SMU->PPUSATD0_CLR = _SMU_PPUSATD0_MASK;
    "MOV      R3, %[smu_ppusatd0_clear]   \n"
    "MOV      R2, %[ppusatd0_mask]        \n"
    "STR      R2, [R3]                    \n"

    // SMU->PPUSATD1_CLR = _SMU_PPUSATD1_MASK & (~SMU_PPUSATD1_SMU);
    "MOV      R3, %[smu_ppusatd1_clear]   \n"
    "MOV      R2, %[ppusatd1_mask]        \n"
    "STR      R2, [R3]                    \n"

#if defined(_SMU_PPUSATD2_MASK)
    // SMU->PPUSATD2_CLR = _SMU_PPUSATD2_MASK;
    "MOV      R3, %[smu_ppusatd2_clear]   \n"
    "MOV      R2, %[ppusatd2_mask]        \n"
    "STR      R2, [R3]                    \n"

    // SMU->PPUSATD3_CLR = _SMU_PPUSATD3_MASK;
    "MOV      R3, %[smu_ppusatd3_clear]   \n"
    "MOV      R2, %[ppusatd3_mask]        \n"
    "STR      R2, [R3]                    \n"
#endif
    : // No outputs.
    :[smu_ppusatd0_clear] "r" (&SMU->PPUSATD0_CLR),
    [ppusatd0_mask] "r" (_SMU_PPUSATD0_MASK),
    [smu_ppusatd1_clear] "r" (&SMU->PPUSATD1_CLR),
    [ppusatd1_mask] "r" (_SMU_PPUSATD1_MASK & (~SMU_PPUSATD1_SMU))
#if defined(_SMU_PPUSATD2_MASK)
    ,
    [smu_ppusatd2_clear] "r" (&SMU->PPUSATD2_CLR),
    [ppusatd2_mask] "r" (_SMU_PPUSATD2_MASK),
    [smu_ppusatd3_clear] "r" (&SMU->PPUSATD3_CLR),
    [ppusatd3_mask] "r" (_SMU_PPUSATD3_MASK)
#endif
    : "r2", "r3", "r4", "r5"
    );

  // Set all MSPU region as Non-Secure except for the one containing the
  // secure code necessary for configuring and switching the core to Non-Secure.
  __ASM volatile (

    // uint32_t *reg = flash_region_flag0;
    // uint32_t offset = (((uintptr_t)&__SECURE_VECTOR_TABLE & 0xFFFFFF) / (uintptr_t)&__secure_flash_size__);
    // do {
    //   if (offset >= 32) {
    //     *reg = 0xFFFFFFFF;
    //     offset -= 32;
    //   } else {
    //     *reg = ~(1 << offset);
    //     offset = 0xFFFFFFFF;
    //   }
    //   reg++;
    // } while (reg <= flash_region_flag3);

    "MOV            R1, %[flash_region_flag0]         \n"
    "BIC            R3, %[secure_vector], #0xFF000000 \n"
    "UDIV           R3, R3, %[secure_flash_size]      \n"

    "flash_region_reg_loop:                           \n"
    "CMP            R3, #32                           \n"
    "BLS            flash_region_reg_with_secure_region \n"
    "SUB            R3, #32                           \n"
    "MOV            R2, #0xFFFFFFFF                   \n"
    "STR            R2, [R1]                          \n"
    "B              flash_region_reg_loop_exit        \n"

    "flash_region_reg_with_secure_region:             \n"
    "MOV            R2, #1                            \n"
    "LSLS           R2, R3                            \n"
    "MVN            R2, R2                            \n"
    "STR            R2, [R1]                          \n"
    "MOV            R3, #0xFFFFFFFF                   \n"
    "B              flash_region_reg_loop_exit        \n"

    "flash_region_reg_loop_exit:                      \n"
    "ADD            R1, #4                            \n"
    "CMP            R1, %[flash_region_flag3]         \n"
    "BLE            flash_region_reg_loop             \n"
    : // No outputs.
    :[secure_vector] "r" (sl_tz_secure_config_data.secure_vector_table),
    [secure_flash_size] "r" (sl_tz_secure_config_data.mspu_region_size),
#if defined(_SMU_MSPUNVMNSREGIONFLAG0_MASK)
    [flash_region_flag0] "r" (&SMU->MSPUNVMNSREGIONFLAG0_SET),
    [flash_region_flag3] "r" (&SMU->MSPUNVMNSREGIONFLAG3_SET)
#elif defined(_SMU_MSPUOSPI0SATD0_MASK)
    [flash_region_flag0] "r" (&SMU->MSPUOSPI0SATD0_CLR),
    [flash_region_flag3] "r" (&SMU->MSPUOSPI0SATD3_CLR)
#endif
    : "r1", "r2", "r3", "r4"
    );

  // Enables the ESAU.
  __ASM volatile (

    // SMU->ESAURTYPES_SET = SMU_ESAURTYPES_ESAUEN;
    "MOV      R3, %[smu_esaurtypes_set]   \n"
    "MOVS     R2, %[esauen]               \n"
    "STR      R2, [R3]                    \n"
    : // No outputs.
    :[smu_esaurtypes_set] "r" (&SMU->ESAURTYPES_SET),
    [esauen] "r" (SMU_ESAURTYPES_ESAUEN)
    : "r2", "r3", "r4", "r5"
    );

  // Sets the core to Non-Secure using the SAU and calls the non-secure reset handler.
  // Should not return.
  __ASM volatile (
    "LSRS           R4, R4, #1          \n"
    "LSLS           R4, R4, #1          \n"

    // SAU->CTRL = SAU_CTRL_ALLNS_Msk;
    "MOVS           R2, #2              \n"
    "MOV            R3, %[SauCtrl]      \n"
    "STR            R2, [R3]            \n"

    // Invalidate all registers.
    "MOVS           R0, #0              \n"
    "MOVS           R1, #0              \n"
    "MOVS           R2, #0              \n"
    "MOVS           R3, #0              \n"
    "MOVS           R5, #0              \n"
    "MOVS           R6, #0              \n"
    "MOVS           R7, #0              \n"
    "MOVS           R8, #0              \n"
    "MOVS           R9, #0              \n"
    "MOVS           R10, #0             \n"
    "MOVS           R11, #0             \n"
    "MOVS           R12, #0             \n"
    "DSB                                \n"

    // Jump to Non-Secure Reset Handler (Application).
    "BXNS           R4                  \n"
    : // No outputs
    :[SauCtrl] "r" (&SAU->CTRL)
    : "r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
    );
}

/*----------------------------------------------------------------------------
 * Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
__ATTRIBUTE_SECURE_FAULT_HANDLER __NO_PROLOGUE void sli_tz_secure_fault_handler(void)
{
  // Falling here means that an unauthorized bus access was tried triggering
  // a SecureFault. Care should be taken to not access Secure ressources.
  // Secure ressources are the SMU, the Secure aliases and the flash
  // MSPU region in charge of switching the core to Non-Secure.
  while (1) {
  }
}

#if defined(SL_TZ_NON_SECURE_EXECUTION_USE_SOURCE)
// Non-Secure Execution Vector Table.
#define SECURE_VECTOR_TABLE {                                                                                              \
    { .topOfStack = &__INITIAL_SP },                                             /*      Initial Stack Pointer          */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_reset_handler)) }, /*      sli_tz_secure_reset_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { APP_PROPERTIES_ADDR },                                                     /*      Application properties         */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
    { (void(*)(void))(VECTOR_TABLE_ENTRY_OFFSET(sli_tz_secure_fault_handler)) }, /*      sli_tz_secure_fault_handler    */ \
}

// Secure Vector Table.
__ATTRIBUTE_SECURE_VECTORS const tVectorEntry sli_tz_secure_vectors[TOTAL_INTERNAL_INTERRUPTS] = SECURE_VECTOR_TABLE;

// When a bootloader is present, the Non-Secure Execution Start-up might need to be pushed further in memory to
// make sure the start address is aligned with a MSPU region. In that case, another instance of the Secure Vector Table is
// flashed after the bootloader where it would be expected to be.
__ATTRIBUTE_SECURE_VECTORS_COPY const tVectorEntry sli_tz_secure_vectors_copy[TOTAL_INTERNAL_INTERRUPTS] = SECURE_VECTOR_TABLE;
#endif
