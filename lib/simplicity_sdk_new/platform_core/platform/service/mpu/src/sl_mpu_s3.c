/***************************************************************************//**
 * @file
 * @brief Simple MPU Service - Series 3 Implementation
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

#include "em_device.h"
#include "sl_core.h"
#include "sl_mpa_manager.h"
#include "sl_mpu.h"

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

#if defined(__ICCARM__)
// iccarm
#pragma section = "text_ram"
#define RAMFUNC_SECTION_BEGIN             ((uint32_t)(uint32_t *)__section_begin("text_ram"))
#define RAMFUNC_SECTION_END               ((uint32_t)(uint32_t *)__section_end("text_ram"))
#define RAMFUNC_SECTION_SIZE              __section_size("text_ram")
#define RAMFUNC_SECTION_NON_ALIASED_BEGIN RAMFUNC_SECTION_BEGIN
#define SRAM_END                          (SRAM_BASE + SRAM_SIZE)

// In the case of IAR, the RAM code is in DMEM non-aliased.
#define RAMFUNC_SECTION_IS_IN_DMEM_ALIAS  0

#elif defined(__GNUC__)
// armgcc
extern uint32_t __vma_ramfuncs_start__;
extern uint32_t __vma_ramfuncs_end__;
extern uint32_t __ramfuncs_start__;
#define RAMFUNC_SECTION_BEGIN             ((uint32_t) &__vma_ramfuncs_start__)
#define RAMFUNC_SECTION_END               ((uint32_t) &__vma_ramfuncs_end__)
#define RAMFUNC_SECTION_SIZE              (RAMFUNC_SECTION_END - RAMFUNC_SECTION_BEGIN)
#define RAMFUNC_SECTION_NON_ALIASED_BEGIN ((uint32_t) &__ramfuncs_start__)
#define SRAM_END                          (SRAM_BASE + SRAM_SIZE)
#define SRAM_ALIAS_END                    (SRAM_ALIAS_BASE + SRAM_SIZE)

#define RAMFUNC_SECTION_IS_IN_DMEM_ALIAS  1

#endif

#if defined(SL_TRUSTZONE_SECURE)
  #define SRAM_ALIAS_ALTERNATE_TZ_BASE SRAM_ALIAS_NS_BASE
  #define SRAM_ALTERNATE_TZ_BASE       SRAM_NS_BASE
#else
  #define SRAM_ALIAS_ALTERNATE_TZ_BASE SRAM_ALIAS_S_BASE
  #define SRAM_ALTERNATE_TZ_BASE       SRAM_S_BASE
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/**************************************************************************//**
 * Configures internal SRAM as non-executable and enable MPU.
 *
 * @note  On series 3 SIXG301 devices, there are the following RAM-related
 *        address ranges.
 *        - 0x0080_0000 0x0087_FFFF (Alias to DMEM_NS) - Non-secure
 *        - 0x1080_0000 0x1087_FFFF (Alias to DMEM (execute only)) -Secure
 *        - 0x2000_0000 0x2007_FFFF (DMEM_NS) - Non-secure
 *        - 0x3000_0000 0x3007_FFFF (DMEM) - Secure
 *
 *        The MPU regions are configured differently depending on the RAM layout
 *        described in GCC and IAR linker scripts. A common characteristic
 *        to all the MPU regions for GCC and IAR is:
 *        - All MPU regions for RAM are non-cacheable.
 *
 *        The table below presents the MPU regions for series 3 SIXG301 devices.
 *        The table assumes there are RAMFunc in RAM. If there are no RAMFunc
 *        functions, then the associated region will be a single region covering
 *        the full addresses range. The table also assumes that the application
 *        is Non-Secure. If the application is Secure, the MPU regions will have
 *        the same layout, but the addresses will be in the Secure address
 *        space.
 *
 * GCC
 * ---
 *                                          MPU Region* Region Attributes              Adresses Range
 *                                          ------------------------------------------------------------------------
 * 0x0080_0000 0x0087_FFFF (RAMFUNC)*       0           shareable,     executable      RAMFunc start to RAMFunc end
 *                                          1           non shareable, non executable  RAMFunc end to range end
 * or
 * 0x0080_0000 0x0087_FFFF (NOT USED)*      x           non shareable, non executable  Entire range (if no RAMFunc)
 *
 * 0x1080_0000 0x1087_FFFF (NOT USED)*      2           non shareable, non executable  Entire range
 * 0x2000_0000 0x2007_FFFF (DATA)*          3           shareable,     non executable  Entire range
 * 0x3000_0000 0x3007_FFFF (NOT USED)*      4           non shareable, non executable  Entire range
 *
 * IAR
 * ---
 *                                          MPU Region* Region Attributes              Adresses Range
 *                                          ------------------------------------------------------------------------
 * 0x0080_0000 0x0087_FFFF (NOT USED)*      0           non shareable, non executable  Entire range
 * 0x1080_0000 0x1087_FFFF (NOT USED)*      1           non shareable, non executable  Entire range
 * 0x2000_0000 0x2007_FFFF (DATA, RAMFUNC)* 2           shareable,     non executable  Range start to RAMFunc start
 *                                          3           shareable,         executable  RAMFunc start to RAMFunc end
 *                                          4           shareable,     non executable  RAMFunc end to range end
 * or
 * 0x2000_0000 0x2007_FFFF (NOT USED)*      x           non shareable, non executable  Entire range (if no RAMFunc)
 *
 * 0x3000_0000 0x3007_FFFF (NOT USED)*      5           non shareable, non executable  Entire range

 *        *If the ARM Cortex-M33 works in non-secure, all non-secure alias/
 *        non-alias will be used by default for RAMfunc and data in RAM.
 *        Same logic if the Cortex-M33 is in secure, all secure alias/non-alias
 *        will be used by default. The MPU regions creation will adapt to
 *        the default secure or non-secure addresses. And thus the order
 *        in which the MPU regions are created can vary.
 *****************************************************************************/
void sl_mpu_disable_execute_from_ram(void)
{
#if !defined(SL_RAM_LINKER)
  sl_status_t status = SL_STATUS_OK;
  sl_mpa_manager_region_t* temp_region_handle = NULL;

  #if (RAMFUNC_SECTION_IS_IN_DMEM_ALIAS == 1)
  if ( RAMFUNC_SECTION_SIZE > 0 ) {
    // Configure address range before the RAMFUNC section as non-executable.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)SRAM_ALIAS_BASE,
                                             (size_t)(RAMFUNC_SECTION_BEGIN - SRAM_ALIAS_BASE),
                                             SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
    EFM_ASSERT(status == SL_STATUS_OK);

    // Configure the RAMFUNC section as read-only in the non-aliased DMEM, since
    // it is possible to modify RAM functions from that address space to
    // conduct code injection attacks.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)RAMFUNC_SECTION_NON_ALIASED_BEGIN,
                                             (size_t)RAMFUNC_SECTION_SIZE,
                                             SL_MPA_MANAGER_ATTRIBUTE_READ_ONLY);
    EFM_ASSERT(status == SL_STATUS_OK);

    // Configure address range after the RAMFUNC section as non-executable.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)RAMFUNC_SECTION_END,
                                             (size_t)(SRAM_ALIAS_END - RAMFUNC_SECTION_END),
                                             SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
    EFM_ASSERT(status == SL_STATUS_OK);
  } else {
    // When there are no RAM functions, the entirety of the DMEM_ALIAS is
    // configured to be non-executable.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)SRAM_ALIAS_BASE,
                                             (size_t)SRAM_SIZE,
                                             SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
    EFM_ASSERT(status == SL_STATUS_OK);
  }

  // Configure DMEM as fully non-executable.
  status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_mpa_manager_configure_region(temp_region_handle,
                                           (void*)SRAM_BASE,
                                           (size_t)SRAM_SIZE,
                                           SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
  EFM_ASSERT(status == SL_STATUS_OK);

  #else
  if ( RAMFUNC_SECTION_SIZE > 0 ) {
    // Configure address range before the RAMFUNC section as non-executable.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)SRAM_BASE,
                                             (size_t)(RAMFUNC_SECTION_BEGIN - SRAM_BASE),
                                             SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
    EFM_ASSERT(status == SL_STATUS_OK);

    // Configure the RAMFUNC section as read-only in the non-aliased DMEM, since
    // it is possible to modify RAM functions from that address space to
    // conduct code injection attacks.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)RAMFUNC_SECTION_NON_ALIASED_BEGIN,
                                             (size_t)RAMFUNC_SECTION_SIZE,
                                             SL_MPA_MANAGER_ATTRIBUTE_READ_ONLY);
    EFM_ASSERT(status == SL_STATUS_OK);

    // Configure address range after the RAMFUNC section as non-executable.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)RAMFUNC_SECTION_END,
                                             (size_t)(SRAM_END - RAMFUNC_SECTION_END),
                                             SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
    EFM_ASSERT(status == SL_STATUS_OK);
  } else {
    // When there are no RAM functions, the entirety of the non-aliased DMEM is
    // configured to be non-executable.
    status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
    EFM_ASSERT(status == SL_STATUS_OK);
    status = sl_mpa_manager_configure_region(temp_region_handle,
                                             (void*)SRAM_BASE,
                                             (size_t)SRAM_SIZE,
                                             SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
    EFM_ASSERT(status == SL_STATUS_OK);
  }

  // Configure DMEM_ALIAS as fully non-executable.
  status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_mpa_manager_configure_region(temp_region_handle,
                                           (void*)SRAM_ALIAS_BASE,
                                           (size_t)SRAM_SIZE,
                                           SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
  EFM_ASSERT(status == SL_STATUS_OK);
  #endif /* RAMFUNC_SECTION_IS_IN_DMEM_ALIAS == 1 */

  // Configure alternate TrustZone address space of DMEM as fully
  // non-executable.
  status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_mpa_manager_configure_region(temp_region_handle,
                                           (void*)SRAM_ALTERNATE_TZ_BASE,
                                           (size_t)SRAM_SIZE,
                                           SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
  EFM_ASSERT(status == SL_STATUS_OK);

  // Configure alternate TrustZone address space of DMEM_ALIAS as fully
  // non-executable.
  status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
  EFM_ASSERT(status == SL_STATUS_OK);
  status = sl_mpa_manager_configure_region(temp_region_handle,
                                           (void*)SRAM_ALIAS_ALTERNATE_TZ_BASE,
                                           (size_t)SRAM_SIZE,
                                           SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
  EFM_ASSERT(status == SL_STATUS_OK);
#endif /* !defined(SL_RAM_LINKER) */
}

/**************************************************************************//**
 * Enables simplified MPU driver. Configures memory address as non-executable.
 *****************************************************************************/
sl_status_t sl_mpu_disable_execute(uint32_t address_begin,
                                   uint32_t address_end,
                                   uint32_t size)
{
  (void)address_end;
  sl_status_t status = SL_STATUS_OK;
  sl_mpa_manager_region_t* temp_region_handle = NULL;

  // Round inside the memory region, if address is not align on 32 bytes.
  uint32_t rounded_base = ((address_begin % 32u) == 0u) ? address_begin
                          : (address_begin + (32u - (address_begin % 32u)));

  // Round inside the memory region, if address is not align on 32 bytes.
  uint32_t rounded_size = ((size % 32u) == 0u) ? size
                          : (size  - (size % 32u));

  status = sl_mpa_manager_alloc_region_handle(&temp_region_handle);
  if ( status != SL_STATUS_OK ) {
    return status;
  }

  status = sl_mpa_manager_configure_region(temp_region_handle,
                                           (void*)rounded_base,
                                           (size_t)rounded_size,
                                           SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE);
  if ( status != SL_STATUS_OK ) {
    return status;
  }

  return SL_STATUS_OK;
}

#if __CORTEX_M != (0u)
/**************************************************************************//**
 * MemManage default exception handler. Reset target.
 *****************************************************************************/
__WEAK void mpu_fault_handler(void)
{
  // Force fail assert to trigger reset
  __NVIC_SystemReset();
}

/**************************************************************************//**
 * MemManage exception handler.
 *****************************************************************************/
void MemManage_Handler(void)
{
  mpu_fault_handler();
}
#endif
