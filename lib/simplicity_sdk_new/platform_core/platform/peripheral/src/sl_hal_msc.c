/***************************************************************************//**
 * @file
 * @brief Flash controller (MSC) Peripheral API
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

#include "sl_hal_msc.h"
#if defined(MSC_COUNT) && (MSC_COUNT > 0)
#include "sl_hal_bus.h"
#include "sl_hal_syscfg.h"
#include "sl_assert.h"
#include "sl_common.h"
#include "sl_core.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define FLASH_PAGE_MASK (~(FLASH_PAGE_SIZE - 1U))

/***************************************************************************//**
 * Get locked status of the MSC registers.
 *
 * @details MSC_IS_LOCKED() is implemented as a macro because it's used inside
 *                          functions that can be placed either in flash
 *                          or in RAM.
 ******************************************************************************/

#define MSC_IS_LOCKED()    ((MSC->STATUS & _MSC_STATUS_REGLOCK_MASK) != 0U)

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE bool sl_hal_msc_is_locked(void);
extern __INLINE void sl_hal_msc_lock(void);
extern __INLINE void sl_hal_msc_unlock(void);
extern __INLINE void sl_hal_msc_set_readctrl(uint32_t value);
extern __INLINE uint32_t sl_hal_msc_get_readctrl(void);
extern __INLINE void sl_hal_msc_lock_page(uint32_t page_number);
extern __INLINE bool sl_hal_msc_page_is_locked(uint32_t page_number);
extern __INLINE uint32_t sl_hal_msc_get_size_userdata(void);
extern __INLINE void sl_hal_msc_set_misc_lock_word(uint32_t value);
extern __INLINE uint32_t sl_hal_msc_get_misc_lock_word(void);

#if !defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
extern __INLINE void sl_hal_msc_enable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_msc_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_msc_set_pending_interrupts(uint32_t flags);
extern __INLINE void sl_hal_msc_clear_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_msc_get_pending_interrupts(void);
extern __INLINE uint32_t sl_hal_msc_get_enabled_pending_interrupts(void);
#endif

/***************************************************************************//**
 * Init MSC module.
 ******************************************************************************/
void sl_hal_msc_init(void)
{
  // Unlock MSC.
  MSC->LOCK_SET = MSC_LOCK_LOCKKEY_UNLOCK;
  // Disable flash write.
  MSC->WRITECTRL_CLR = MSC_WRITECTRL_WREN;
}

/***************************************************************************//**
 * Wait for a specified MSC status or timeout.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_wait_status(uint32_t mask,
                                           uint32_t value)
{
  uint32_t timeout = SL_HAL_MSC_PROGRAM_TIMEOUT;

  while (timeout) {
    uint32_t status = MSC->STATUS;

    // if INVADDR is asserted by MSC, BUSY will never go high, can be checked early.
    if ((status & MSC_STATUS_INVADDR) != 0) {
      return SL_HAL_MSC_INVALID_ADDR;
    }
    // if requested operation fails because flash is locked, BUSY will be high
    // for a few cycles and it's not safe to clear WRITECTRL.WREN during that
    // period. sl_hal_msc_wait_status should return only when it's safe to do so.
    // So if user is checking BUSY flag, make sure it matches user's expected
    // value and only then check the lock bits. Otherwise, do check early and
    // bail out if necessary.
    if ((!(mask & MSC_STATUS_BUSY))
        && (status & (MSC_STATUS_LOCKED | MSC_STATUS_REGLOCK))) {
      return SL_HAL_MSC_LOCKED;
    }
    if ((status & mask) == value) {
      if ((status & (MSC_STATUS_LOCKED | MSC_STATUS_REGLOCK)) != 0) {
        return SL_HAL_MSC_LOCKED;
      } else {
        return SL_HAL_MSC_OK;
      }
    }
    timeout--;
  }

  return SL_HAL_MSC_TIMEOUT;
}

/***************************************************************************//**
 * Writes data to flash memory. It is assumed that start address is word
 * aligned and that num_bytes is an integer multiple of four, and that the
 * write operation does not cross a flash page boundary.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_write_burst(uint32_t address,
                                           const uint32_t *data,
                                           uint32_t num_bytes)
{
  // Check alignment (must be aligned to words).
  EFM_ASSERT(((uint32_t)address & 0x3U) == 0);
  EFM_ASSERT(data != NULL);
  // Check number of bytes, must be divisible by four.
  EFM_ASSERT((num_bytes & 0x3U) == 0);

  sl_hal_msc_status_t ret_val;
  MSC->ADDRB = address;
  if ((MSC->STATUS & MSC_STATUS_INVADDR) != 0) {
    return SL_HAL_MSC_INVALID_ADDR;
  }

  MSC->WDATA = *data; // Write first word.
  data++; // Increment data pointer to next word.
  num_bytes  -= 4; // Decrement number of bytes to write.

  while (num_bytes) {
    ret_val = sl_hal_msc_wait_status(MSC_STATUS_WDATAREADY, MSC_STATUS_WDATAREADY);

    if (ret_val != SL_HAL_MSC_OK) {
      return ret_val;
    }

    MSC->WDATA = *data; // Write word.
    data++; // Increment data pointer to next word.
    num_bytes  -= 4; // Decrement number of bytes to write.
  }

  MSC->WRITECMD_SET = MSC_WRITECMD_WRITEEND;
  ret_val = sl_hal_msc_wait_status((MSC_STATUS_BUSY | MSC_STATUS_PENDING), 0);
  if (ret_val == SL_HAL_MSC_OK) {
    // We need to check twice to be sure.
    ret_val = sl_hal_msc_wait_status((MSC_STATUS_BUSY | MSC_STATUS_PENDING), 0);
  }

  return ret_val;
}

/***************************************************************************//**
 * Set MSC code execution configuration.
 ******************************************************************************/
void sl_hal_msc_set_exec_config(const sl_hal_msc_exec_config_t *exec_config)
{
  EFM_ASSERT(exec_config != NULL);

  uint32_t msc_read_ctrl;

#if defined(MSC_RDATACTRL_DOUTBUFEN)
  msc_read_ctrl = MSC->RDATACTRL & ~MSC_RDATACTRL_DOUTBUFEN;

  if (exec_config->dout_buf_en) {
    msc_read_ctrl |= MSC_RDATACTRL_DOUTBUFEN;
  }

  MSC->RDATACTRL = msc_read_ctrl;
#elif defined(MSC_READCTRL_DOUTBUFEN)
  msc_read_ctrl = MSC->READCTRL & ~MSC_READCTRL_DOUTBUFEN;

  if (exec_config->dout_buf_en) {
    msc_read_ctrl |= MSC_READCTRL_DOUTBUFEN;
  }
  MSC->READCTRL = msc_read_ctrl;
#endif
}

/***************************************************************************//**
 * Erases a page in flash memory.
 * For IAR Embedded Workbench, Simplicity Studio and GCC this will be achieved
 * automatically by using attributes in the function proctype. For Keil
 * uVision you must define a section called "ram_code" and place this manually
 * in your project's scatter file.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_erase_page(uint32_t *start_address)
{
  EFM_ASSERT(start_address != NULL);
  // Address must be aligned to page boundary.
  EFM_ASSERT((((uint32_t)start_address) & (FLASH_PAGE_SIZE - 1U)) == 0);

  sl_hal_msc_status_t ret_val;
  bool was_locked;

  was_locked = MSC_IS_LOCKED();
  MSC->LOCK = MSC_LOCK_LOCKKEY_UNLOCK;

  MSC->WRITECTRL_SET = MSC_WRITECTRL_WREN;
  MSC->ADDRB         = (uint32_t)start_address;
  MSC->WRITECMD_SET  = MSC_WRITECMD_ERASEPAGE;

  ret_val = sl_hal_msc_wait_status((MSC_STATUS_BUSY | MSC_STATUS_PENDING), 0);

  if (ret_val == SL_HAL_MSC_OK) {
    // We need to check twice to be sure.
    ret_val = sl_hal_msc_wait_status((MSC_STATUS_BUSY | MSC_STATUS_PENDING), 0);
  }
  MSC->WRITECTRL_CLR = MSC_WRITECTRL_WREN;
  if (was_locked) {
    MSC->LOCK = MSC_LOCK_LOCKKEY_LOCK;
  }

  return ret_val;
}

/***************************************************************************//**
 * Writes data to flash memory. Write data must be aligned to words and
 * contain a number of bytes that is divisible by four.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_write_word(uint32_t *address,
                                          void const *data,
                                          uint32_t num_bytes)
{
  EFM_ASSERT(address != NULL);
  EFM_ASSERT(((uint32_t)address & 0x3U) == 0); // Check alignment (must be aligned to words).
  EFM_ASSERT(data != NULL);
  EFM_ASSERT((num_bytes & 0x3U) == 0); // Check number of bytes, must be divisible by four.

  uint32_t addr;
  const uint8_t  *p_data;
  uint32_t burst_len;
  sl_hal_msc_status_t ret_val = SL_HAL_MSC_OK;
  bool was_locked;

  was_locked = MSC_IS_LOCKED();
  MSC->LOCK = MSC_LOCK_LOCKKEY_UNLOCK;

  // Enable flash write.
  MSC->WRITECTRL_SET = MSC_WRITECTRL_WREN;

  addr  = (uint32_t)address;
  p_data = (const uint8_t*)data;

  while (num_bytes) {
    // Max burst length is up to next flash page boundary.
    burst_len = SL_MIN(num_bytes,
                       ((addr + FLASH_PAGE_SIZE) & FLASH_PAGE_MASK) - addr);
    ret_val = sl_hal_msc_write_burst(addr, (const uint32_t*)p_data, burst_len);
    if (ret_val != SL_HAL_MSC_OK) {
      break;
    }
    addr      += burst_len;
    p_data    += burst_len;
    num_bytes -= burst_len;
  }
  // Disable flash write.
  MSC->WRITECTRL_CLR = MSC_WRITECTRL_WREN;
  if (was_locked) {
    MSC->LOCK = MSC_LOCK_LOCKKEY_LOCK;
  }

  return ret_val;
}

/***************************************************************************//**
 * Erase the entire Flash in one operation.
 ******************************************************************************/
SL_HAL_MSC_CODE_RAM
sl_hal_msc_status_t sl_hal_msc_mass_erase(void)
{
  sl_hal_msc_status_t ret_val;

  if (MSC_IS_LOCKED()) {
    return SL_HAL_MSC_LOCKED;
  }

  MSC->WRITECTRL_SET    = MSC_WRITECTRL_WREN;                 // Set write enable bit.
  MSC->MISCLOCKWORD_CLR = MSC_MISCLOCKWORD_MELOCKBIT;         // Enable Write ctrl access.
  MSC->WRITECMD_SET     = MSC_WRITECMD_ERASEMAIN0;            // Start Mass erase procedure.
  ret_val               = sl_hal_msc_wait_status(MSC_STATUS_BUSY, 0);   // Wait for end of busy flag or a problem (INVADDR, LOCK, REGLOCK, TIMEOUT).
  MSC->MISCLOCKWORD_SET = MSC_MISCLOCKWORD_MELOCKBIT;         // Reenable mass erase lock bit.
  MSC->WRITECTRL_CLR    = MSC_WRITECTRL_WREN;                 // Disable Write ctrl access.

  return ret_val;
}

/***************************************************************************//**
 * Writes data to flash memory using the DMA.
 ******************************************************************************/
sl_hal_msc_status_t sl_hal_msc_write_word_dma(uint32_t channel,
                                              uint32_t *address,
                                              const void *data,
                                              uint32_t num_bytes)
{
  EFM_ASSERT(channel < DMA_CHAN_COUNT);
  EFM_ASSERT(address != NULL);
  EFM_ASSERT(data != NULL);
  EFM_ASSERT((num_bytes & 0x3U) == 0); // Check number of bytes, must be divisible by four.

  uint32_t words;
  uint32_t burst_len;
  uint32_t src = (uint32_t) data;
  uint32_t dst = (uint32_t) address;
  bool was_locked;

  LDMA->EN_SET = 0x1;
  LDMAXBAR->CH[channel].REQSEL = LDMAXBAR_CH_REQSEL_SOURCESEL_MSC
                                 | LDMAXBAR_CH_REQSEL_SIGSEL_MSCWDATA;
  LDMA->CH[channel].CFG = _LDMA_CH_CFG_RESETVALUE;
  LDMA->CH[channel].LOOP = _LDMA_CH_LOOP_RESETVALUE;
  LDMA->CH[channel].LINK = _LDMA_CH_LINK_RESETVALUE;

  // Unlock MSC.
  was_locked = MSC_IS_LOCKED();
  MSC->LOCK = MSC_LOCK_LOCKKEY_UNLOCK;

  // Enable writing to the MSC module.
  MSC->WRITECTRL_SET = MSC_WRITECTRL_WREN;

  while (num_bytes) {
    // Max burst length is up to next flash page boundary.
    burst_len = SL_MIN(num_bytes,
                       ((dst + FLASH_PAGE_SIZE) & FLASH_PAGE_MASK) - dst);
    words = burst_len / 4;
    // Load the address.
    MSC->ADDRB = dst;
    // Check for an invalid address.
    if ((MSC->STATUS & MSC_STATUS_INVADDR) != 0) {
      return SL_HAL_MSC_INVALID_ADDR;
    }

    LDMA->CH[channel].CTRL = LDMA_CH_CTRL_DSTINC_NONE
                             | LDMA_CH_CTRL_SIZE_WORD
                             | ((words - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT);
    LDMA->CH[channel].SRC = src;
    LDMA->CH[channel].DST = (uint32_t)&MSC->WDATA;

    // Enable channel.
    LDMA->CHEN_SET = (0x1 << channel);
    while ((LDMA->CHDONE & (0x1 << channel)) == 0x0) {
      // Wait for the DMA channel to complete
    }
    LDMA->CHDONE_CLR = (0x1 << channel);
    LDMA->CHDIS_SET  = (0x1 << channel);

    MSC->WRITECMD_SET = MSC_WRITECMD_WRITEEND;

    dst       += burst_len;
    src       += burst_len;
    num_bytes -= burst_len;
  }
  // Disable writing to the MSC module.
  MSC->WRITECTRL_CLR = MSC_WRITECTRL_WREN;
  if (was_locked) {
    MSC->LOCK = MSC_LOCK_LOCKKEY_LOCK;
  }

  return SL_HAL_MSC_OK;
}

/** @} (end addtogroup msc) */
#endif /* defined(MSC_COUNT) && (MSC_COUNT > 0) */
