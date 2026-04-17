/***************************************************************************//**
 * @file
 * @brief ICACHE Disable
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_icache_disable.h"
#include "em_device.h"
#include "sl_hal_bus.h"

#if !defined(L1ICACHE_PRESENT) || !defined(L2ICACHE_PRESENT) || !defined(_SILICON_LABS_32B_SERIES_3)
#error "The icache_disable component is not supported on this device."
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 **************************  GLOBAL VARIABLES   ********************************
 ******************************************************************************/

#if defined(__GNUC__)
extern const uint32_t linker_code_begin;
extern const uint32_t linker_code_end;
#endif

/*******************************************************************************
 *****************************   DEFINES   *************************************
 ******************************************************************************/
#if defined(__GNUC__)
#define FLASH_CODE_BASE               (uint32_t)&linker_code_begin
#define FLASH_CODE_LIMIT              (uint32_t)&linker_code_end
#endif

#if defined(__ICCARM__)
#pragma section = ".text"
#define FLASH_CODE_BASE       ((uint32_t)(uint32_t *)__section_begin(".text"))
#define FLASH_CODE_LIMIT      ((uint32_t)(uint32_t *)__section_end(".text"))
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 *  Clear all MPU entries.
 ******************************************************************************/
static void mpu_entry_table_clear(void)
{
  // ARM Cortex-M33
  const uint8_t num_mpu_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk)
                                  >> MPU_TYPE_DREGION_Pos;

  for ( uint8_t i = 0; i < num_mpu_regions; i++) {
    MPU->RNR = i;
    MPU->RBAR = 0U;
    MPU->RLAR = 0U;
  }
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Disable the L1 cache, as well as the L2 cache.
 *
 *  This function clears the MPU and sets attributes on FLASH memory accesses
 *  to disable caching. Other software that relies on the MPU may not behave
 *  correctly when this component is present.
 *
 * @return
 *  None
 ******************************************************************************/
void sli_icache_disable(void)
{
  // Enable L1ICACHE0 register clock.
  CMU->CLKEN1_SET = CMU_CLKEN1_ICACHE0;

  // Disable L1 Cache
  sl_hal_bus_reg_write_bit(&(L1ICACHE0->CTRL), _ICACHE_CTRL_CACHEDIS_SHIFT, 1);
  sl_hal_bus_reg_write_bit(&(L1ICACHE0->CTRL), _ICACHE_CTRL_DATACACHEDIS_SHIFT, 1);

  // Disable L2 Cache via the MPU
  ARM_MPU_Disable();
  {
    mpu_entry_table_clear();

    // Set MAIR0 memory attributes for non-transient, non-cacheable data.
    ARM_MPU_SetMemAttr(0,                                             // MAIR0
                       ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1, 0, 0, 0), // Outer
                                    ARM_MPU_ATTR_MEMORY_(1, 0, 0, 0)) // Inner
                       );
    // FLASH Code Execute with MAIR0 memory attributes
    MPU->RNR = 1U;
    MPU->RBAR = ARM_MPU_RBAR(FLASH_CODE_BASE,
                             ARM_MPU_SH_NON,
                             0,  // Read-Write
                             1,  // Non-privileged
                             0); // Executable
    MPU->RLAR = ARM_MPU_RLAR(FLASH_CODE_LIMIT,
                             0); // Attribute Index 0
  }
  ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}

#ifdef __cplusplus
}
#endif
