/***************************************************************************//**
 * @file
 * @brief Flash Controller (MSC) Peripheral API
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

#ifndef SL_HAL_MSC_H
#define SL_HAL_MSC_H

#include "em_device.h"
#if defined(MSC_COUNT) && (MSC_COUNT > 0)

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "sl_code_classification.h"
#include "sl_assert.h"
#include "sl_enum.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
#include "sli_tz_ns_interface.h"
#include "sli_tz_service_msc.h"
#include "sli_tz_s_interface.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup msc MSC - Memory System Controller
 * @brief Memory System Controller API
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/***************************************************************************//**
 * Timeout used while waiting for Flash to become ready after a write.
 * This number indicates the number of iterations to perform before
 * issuing a timeout.
 *
 * @note Timeout is set very large (in the order of 100x longer than
 *       necessary). This is to avoid any corner case.
 ******************************************************************************/
#define SL_HAL_MSC_PROGRAM_TIMEOUT             10000000UL

/// SL_HAL_MSC_CODE_RAM is used to place functions in RAM.
#if !defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
#define SL_HAL_MSC_CODE_RAM           SL_CODE_RAM
#else
#define SL_HAL_MSC_CODE_RAM
#endif

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// Return codes for writing/erasing Flash.
SL_ENUM(sl_hal_msc_status_t) {
  SL_HAL_MSC_OK              =  0, ///< Flash write/erase successful.
  SL_HAL_MSC_INVALID_ADDR    =  1, ///< Invalid address. Write to an address that is not Flash.
  SL_HAL_MSC_LOCKED          =  2, ///< Flash address is locked.
  SL_HAL_MSC_TIMEOUT         =  3, ///< Timeout while writing to Flash.
  SL_HAL_MSC_UNALIGNED       =  4  ///< Unaligned access to Flash.
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

#if defined(MSC_READCTRL_DOUTBUFEN) || defined(MSC_RDATACTRL_DOUTBUFEN)
/// Code execution configuration.
typedef struct {
  bool dout_buf_en;       ///< Flash dout pipeline buffer enable.
} sl_hal_msc_exec_config_t;

/// Default MSC exec_config initialization.
#define SL_HAL_MSC_EXEC_CONFIG_DEFAULT \
  {                                    \
    false,                             \
  }

#else
/// Code execution configuration.
typedef struct {
  bool scbt_en;          ///< Enable Suppressed Conditional Branch Target Prefetch.
  bool prefetch_en;      ///< Enable MSC prefetching.
  bool ifc_dis;          ///< Disable instruction cache.
  bool ai_dis;           ///< Disable automatic cache invalidation on write or erase.
  bool icc_dis;          ///< Disable automatic caching of fetches in interrupt context.
  bool use_hprot;        ///< Use ahb_hprot to determine if the instruction is cacheable or not.
} sl_hal_msc_exec_config_t;

/// Default MSC exec_config initialization.
#define SL_HAL_MSC_EXEC_CONFIG_DEFAULT \
  {                                    \
    false,                             \
    true,                              \
    false,                             \
    false,                             \
    false,                             \
    false,                             \
  }
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * Enable MSC module. Puts MSC hardware in a known state.
 ******************************************************************************/
void sl_hal_msc_init(void);

/***************************************************************************//**
 * Wait for a specified MSC status or timeout.
 *
 * @param[in] mask MSC->STATUS register mask to apply when testing for specified
 *                 status.
 * @param[in] value The value the MSC->STATUS test is waiting to see.
 *
 * @return Returns the status of a write or erase operation, @ref sl_hal_msc_status_t
 *         SL_HAL_MSC_OK - Specified status criteria fulfilled.
 *         SL_HAL_MSC_INVALID_ADDR - Operation tried to write or erase a
 *                                   non-flash area.
 *         SL_HAL_MSC_LOCKED - MSC registers are locked or the operation
 *                                    tried to write or erase a locked area of the
 *                                    flash.
 *         SL_HAL_MSC_TIMEOUT - Operation timed out.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_wait_status(uint32_t mask,
                                           uint32_t value);

/***************************************************************************//**
 * Writes data to flash memory. It is assumed that start address is word
 * aligned and that num_bytes is an integer multiple of four, and that the
 * write operation does not cross a flash page boundary.
 *
 * @param[in] address Pointer to the flash word to write to. Must be aligned
 *                    to words.
 * @param[in] data Data to write to flash.
 * @param[in] num_bytes Number of bytes to write to flash. NB: Must be divisable
 *                      by four.
 *
 * @return Returns the status of the write operation, @ref sl_hal_msc_status_t
 *         SL_HAL_MSC_OK - Operation completed successfully.
 *         SL_HAL_MSC_INVALID_ADDR - Operation tried to write to a non-flash
 *                                   area.
 *         SL_HAL_MSC_LOCKED - MSC registers are locked or the operation
 *                                    tried to program a locked area of the flash.
 *         SL_HAL_MSC_TIMEOUT - Operation timed out.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_write_burst(uint32_t address,
                                           const uint32_t *data,
                                           uint32_t num_bytes);

#if !defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
/***************************************************************************//**
 * Set MSC code execution configuration.
 *
 * @param[in] exec_config Code execution configuration.
 ******************************************************************************/
void sl_hal_msc_set_exec_config(const sl_hal_msc_exec_config_t *exec_config);

#if defined(MSC_WRITECMD_ERASEMAIN0)
/***************************************************************************//**
 * Erase the entire Flash in one operation.
 *
 * @return Returns the status of the operation.
 *
 * @note This command will erase the entire contents of the device.
 *       Use with care, both a debug session and all contents of the flash will
 *       be lost. The lock bit, MLW will prevent this operation from executing
 *       and might prevent a successful mass erase.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_mass_erase(void);
#endif
#endif /* !SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT */

/***************************************************************************//**
 * Erases a page in flash memory.
 *
 * @param[in] start_address Pointer to the flash page to erase. Must be aligned
 *                          to beginning of page boundary.
 *
 * @return Returns the status of erase operation, @ref sl_hal_msc_status_t
 *         SL_HAL_MSC_OK - Operation completed successfully.
 *         SL_HAL_MSC_INVALID_ADDR - Operation tried to erase a non-flash
 *                                   area.
 *         SL_HAL_MSC_LOCKED - MSC registers are locked or the operation
 *                                    tried to erase a locked area of the flash.
 *         SL_HAL_MSC_TIMEOUT - Operation timed out.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_erase_page(uint32_t *start_address);

/***************************************************************************//**
 * Writes data to flash memory. Write data must be aligned to words and
 * contain a number of bytes that is divisible by four.
 *
 * @param[in] address Pointer to the flash word to write to. Must be aligned to words.
 * @param[in] data Data to write to flash.
 * @param[in] num_bytes Number of bytes to write to flash. NB: Must be divisable by four.
 *
 * @return Returns the status of the write operation, @ref sl_hal_msc_status_t
 *         SL_HAL_MSC_OK - Operation completed successfully.
 *         SL_HAL_MSC_INVALID_ADDR - Operation tried to write to a non-flash area.
 *         SL_HAL_MSC_LOCKED - MSC registers are locked or the operation tried to
 *                             program a locked area of the flash.
 *         SL_HAL_MSC_TIMEOUT - Operation timed out.
 *
 * @note It is recommended to erase the flash page before performing a write.
 *       For IAR Embedded Workbench, Simplicity Studio and GCC this will be achieved
 *       automatically by using attributes in the function proctype. For Keil
 *       uVision you must define a section called "ram_code" and place this manually
 *       in your project's scatter file.
 *       The Flash memory is organized into 64-bit wide double-words.
 *       Each 64-bit double-word can be written only twice using burst write
 *       operation between erasing cycles. The user's application must store data in
 *       RAM to sustain burst write operation.
 *       EFR32XG21 RevC is not able to program every word twice before the next erase.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_write_word(uint32_t *address,
                                          void const *data,
                                          uint32_t num_bytes);

/***************************************************************************//**
 * Writes data to flash memory using the DMA.
 *
 * @param[in] channel DMA channel to use.
 * @param[in] address A pointer to the flash word to write to. Must be aligned to
 *                    words.
 * @param[in] data Data to write to flash and be aligned to words.
 * @param[in] num_bytes A number of bytes to write from flash. NB: Must be
 *                      divisible by four.
 *
 * @return Returns the status of the write operation.
 *         SL_HAL_MSC_OK - The operation completed successfully.
 *         SL_HAL_MSC_INVALID_ADDR -The operation tried to erase a non-flash area.
 *
 * @details This function uses the LDMA to write data to the internal flash memory.
 *          This is the fastest way to write data to the flash and should be used
 *          when the application wants to achieve write speeds like they are reported
 *          in the datasheet.
 *          Note that copying data from flash to flash will be slower than copying
 *          from RAM to flash. So the source data must be in RAM in order to
 *          see the write speeds similar to the datasheet numbers.
 *
 * @note This function requires that the LDMA and LDMAXBAR clock is enabled
 *       user can use sl_clock_manager_enable_bus_clock() to enable the clock.
 ******************************************************************************/
sl_hal_msc_status_t sl_hal_msc_write_word_dma(uint32_t channel,
                                              uint32_t *address,
                                              const void *data,
                                              uint32_t num_bytes);

/***************************************************************************//**
 * Get the status of the MSC register lock.
 *
 * @return Boolean true if register lock is applied, false otherwise.
 ******************************************************************************/
__INLINE bool sl_hal_msc_is_locked(void)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  return (bool)sli_tz_ns_interface_dispatch_simple_noarg((sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
                                                         SLI_TZ_MSC_GET_LOCKED_SID);
#else

  return (MSC->STATUS & _MSC_STATUS_REGLOCK_MASK) != MSC_STATUS_REGLOCK_UNLOCKED;
#endif
}

/***************************************************************************//**
 * Set the MSC register lock to a locked state.
 ******************************************************************************/
__INLINE void sl_hal_msc_lock(void)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  (void)sli_tz_ns_interface_dispatch_simple_noarg((sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
                                                  SLI_TZ_MSC_SET_LOCKED_SID);
#else
  MSC->LOCK = MSC_LOCK_LOCKKEY_LOCK;
#endif
}

/***************************************************************************//**
 * Set the MSC register lock to an unlocked state.
 ******************************************************************************/
__INLINE void sl_hal_msc_unlock(void)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  (void)sli_tz_ns_interface_dispatch_simple_noarg((sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
                                                  SLI_TZ_MSC_SET_UNLOCKED_SID);
#else
  MSC->LOCK = MSC_LOCK_LOCKKEY_UNLOCK;
#endif
}

/***************************************************************************//**
 * Write a value to the read control register (MSC_READCTRL).
 *
 * @param[in] value The 32-bit value to write to the MSC_READCTRL register.
 ******************************************************************************/
__INLINE void sl_hal_msc_set_readctrl(uint32_t value)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  (void)sli_tz_ns_interface_dispatch_simple((sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
                                            SLI_TZ_MSC_SET_READCTRL_SID, value);

#else
  MSC->READCTRL = value;
#endif
}

/***************************************************************************//**
 * Get the current value of the read control register (MSC_READCTRL).
 *
 * @return The 32-bit value read from the MSC_READCTRL register.
 ******************************************************************************/
__INLINE uint32_t sl_hal_msc_get_readctrl(void)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  return sli_tz_ns_interface_dispatch_simple_noarg((sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
                                                   SLI_TZ_MSC_GET_READCTRL_SID);
#else
  return MSC->READCTRL;
#endif
}

/***************************************************************************//**
 * Set the lockbit for a flash page in order to prevent page writes/erases to
 * the corresponding page.
 *
 * @param[in] page_number The index of the page to apply the pagelock to.
 *                        Must be in the range [0, (flash_size / page_size) - 1].
 ******************************************************************************/
__INLINE void sl_hal_msc_lock_page(uint32_t page_number)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  (void)sli_tz_ns_interface_dispatch_simple((sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
                                            SLI_TZ_MSC_SET_PAGELOCK_SID, page_number);
#else
  EFM_ASSERT(page_number < (FLASH_SIZE / FLASH_PAGE_SIZE));

  volatile uint32_t *pagelock_registers = &MSC->PAGELOCK0;
  pagelock_registers[page_number / 32] |= (1 << (page_number % 32));
#endif
}

/***************************************************************************//**
 * Get the value of the lockbit for a flash page.
 *
 * @param[in] page_number The index of the page to get the lockbit value from.
 *                         Must be in the range [0, (flash_size / page_size) - 1].
 *
 * @return Boolean true if the page is locked, false otherwise.
 ******************************************************************************/
__INLINE bool sl_hal_msc_page_is_locked(uint32_t page_number)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  return (bool)sli_tz_ns_interface_dispatch_simple((sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
                                                   SLI_TZ_MSC_GET_PAGELOCK_SID, page_number);
#else
  EFM_ASSERT(page_number < (FLASH_SIZE / FLASH_PAGE_SIZE));
  const volatile uint32_t *pagelock_registers = &MSC->PAGELOCK0;

  return pagelock_registers[page_number / 32] & (1 << (page_number % 32));
#endif
}

/***************************************************************************//**
 * Get the size of the user data region in flash.
 *
 * @return The size of the user data region divided by 256.
 ******************************************************************************/
__INLINE uint32_t sl_hal_msc_get_size_userdata(void)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  return sli_tz_ns_interface_dispatch_simple_noarg((sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
                                                   SLI_TZ_MSC_GET_USERDATA_SIZE_SID);
#else
  return MSC->USERDATASIZE;
#endif
}

/***************************************************************************//**
 * Write a value to the mass erase and user data page lock word
 * (MSC_MISCLOCKWORD).
 *
 * @param[in] value The 32-bit value to write to the MSC_MISCLOCKWORD register.
 ******************************************************************************/
__INLINE void sl_hal_msc_set_misc_lock_word(uint32_t value)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  (void)sli_tz_ns_interface_dispatch_simple((sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
                                            SLI_TZ_MSC_SET_MISCLOCKWORD_SID, value);
#else
  MSC->MISCLOCKWORD = value;
#endif
}

/***************************************************************************//**
 * Get the current value of the mass erase and user data page lock word
 *   (MSC_MISCLOCKWORD).
 *
 * @return The 32-bit value read from the MSC_MISCLOCKWORD register.
 ******************************************************************************/
__INLINE uint32_t sl_hal_msc_get_misc_lock_word(void)
{
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
  return sli_tz_ns_interface_dispatch_simple_noarg((sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
                                                   SLI_TZ_MSC_GET_MISCLOCKWORD_SID);

#else
  return MSC->MISCLOCKWORD;
#endif
}

#if !defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
/***************************************************************************//**
 * Enable one or more MSC interrupts.
 *
 * @param[in] flags MSC interrupt sources to enable. Use a bitwise logic OR
 *                  combination of valid interrupt flags for the MSC module
 *                  (MSC_IF_nnn).
 *
 * @note Depending on the use, a pending interrupt may already be set prior to
 *       enabling the interrupt. To ignore a pending interrupt, consider using
 *       sl_hal_msc_clear_interrupts() prior to enabling the interrupt.
 ******************************************************************************/
__INLINE void sl_hal_msc_enable_interrupts(uint32_t flags)
{
  MSC->IEN_SET = flags;
}

/***************************************************************************//**
 * Disable one or more MSC interrupts.
 *
 * @param[in] flags MSC interrupt sources to disable. Use a bitwise logic OR
 *                  combination of valid interrupt flags for the MSC module
 *                  (MSC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_msc_disable_interrupts(uint32_t flags)
{
  MSC->IEN_CLR = flags;
}

/***************************************************************************//**
 * Set one or more pending MSC interrupts from SW.
 *
 * @param[in] flags MSC interrupt sources to set to pending. Use a bitwise logic
 *                  OR combination of valid interrupt flags for the MSC module
 *                  (MSC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_msc_set_pending_interrupts(uint32_t flags)
{
  MSC->IF_SET = flags;
}

/***************************************************************************//**
 * Clear one or more pending MSC interrupts.
 *
 * @param[in] flags Pending MSC interrupt source to clear. Use a bitwise logic
 *                  OR combination of valid interrupt flags for the MSC module
 *                  (MSC_IF_nnn).
 ******************************************************************************/
__INLINE void sl_hal_msc_clear_interrupts(uint32_t flags)
{
  MSC->IF_CLR = flags;
}

/***************************************************************************//**
 * Get pending MSC interrupt flags.
 *
 * @return MSC interrupt sources pending. A bitwise logic OR combination of valid
 *         interrupt flags for the MSC module (MSC_IF_nnn).
 *
 * @note The event bits are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_msc_get_pending_interrupts(void)
{
  return MSC->IF;
}

/***************************************************************************//**
 * Get enabled and pending MSC interrupt flags.
 *
 * @return Pending and enabled MSC interrupt sources.
 *         The return value is the bitwise AND of
 *         - the enabled interrupt sources in MSC_IEN and
 *         - the pending interrupt flags MSC_IF
 *
 * @note Interrupt flags are not cleared by the use of this function.
 ******************************************************************************/
__INLINE uint32_t sl_hal_msc_get_enabled_pending_interrupts(void)
{
  uint32_t ien;
  ien = MSC->IEN;
  return MSC->IF & ien;
}

#endif /* !SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT */
/** @} (end addtogroup msc) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/***************************************************************************//**
 * @addtogroup msc MSC - Memory System Controller
 *
 * @brief Memory System Controller API
 *
 * @details
 * Contains functions to control the MSC, primarily the Flash.
 * Users can perform Flash memory write and erase operations, as well as
 * optimization of the CPU instruction fetch interface for the application.
 * Available instruction fetch features depends on the MCU or SoC family, but
 * features such as instruction pre-fetch, cache, and configurable branch prediction
 * are typically available.
 *
 *
 * Flash erase may add ms of delay to interrupt latency if executing from Flash.
 * Flash write and erase operations are supported by @ref sl_hal_msc_write_word(),
 * @ref sl_hal_msc_erase_page(), and sl_hal_msc_mass_erase().
 * Mass erase is supported for MCU and SoC families with larger Flash sizes.
 *
 * @ref sl_hal_msc_init() must be called prior to any Flash write or erase operation.
 * The following steps are necessary to perform a page erase and write:
 *
 * @code
 * uint32_t *user_data_page = (uint32_t *) USERDATA_BASE;
 * uint32_t user_data = 0x05060708;
 * #if defined(_CMU_CLKEN1_MSC_MASK)
 *   sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_MSC);
 * #endif
 * // Initialize MSC module.
 * sl_hal_msc_init();
 * // Erase user data page.
 * sl_hal_msc_erase_page(user_data_page);
 * // Write value to user data page.
 * sl_hal_msc_write_word(user_data_page, &user_data, sizeof(user_data));
 * @endcode
 *
 * @} (end addtogroup msc)
 ******************************************************************************/
/* *INDENT-ON* */

#endif /* defined(MSC_COUNT) && (MSC_COUNT > 0) */
#endif /* SL_HAL_MSC_H */
