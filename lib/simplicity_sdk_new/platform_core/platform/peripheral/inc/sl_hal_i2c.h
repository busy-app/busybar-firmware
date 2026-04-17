/***************************************************************************//**
 * @file
 * @brief Inter-Integrated Circuit (I2C) peripheral API
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

#ifndef SL_HAL_I2C_H
#define SL_HAL_I2C_H
#include "em_device.h"
#if defined(I2C_PRESENT)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "sl_assert.h"
#include "sl_status.h"
#include "sl_enum.h"
#include "sl_device_i2c.h"

/***************************************************************************//**
 * @addtogroup i2c I2C - Inter-Integrated Circuit
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/// I2C instance validation
#define SL_HAL_I2C_REF_VALID(ref)    (I2C_NUM(ref) != -1)
/// I2C IF Errors flags.
#define SL_HAL_I2C_IF_ERRORS    (I2C_IF_BUSERR | I2C_IF_ARBLOST)
/// I2C IEN Errors flags.
#define SL_HAL_I2C_IEN_ERRORS   (I2C_IEN_BUSERR | I2C_IEN_ARBLOST)

/**
 * @brief
 *   Standard mode max frequency assuming using 4:4 ratio for Nlow:Nhigh.
 * @details
 *   From I2C specification: Min Tlow = 4.7us, min Thigh = 4.0us,
 *   max Trise=1.0us, max Tfall=0.3us. Since ratio is 4:4, have to use
 *   worst case value of Tlow or Thigh as base.
 *
 *   1/(Tlow + Thigh + 1us + 0.3us) = 1/(4.7 + 4.7 + 1.3)us = 93458Hz.
 * @note
 *   Due to chip characteristics, max value is somewhat reduced.
 */
#define SL_HAL_I2C_FREQ_STANDARD_MAX   100000

/**
 * @brief
 *   Fast mode max frequency assuming using 6:3 ratio for Nlow:Nhigh.
 * @details
 *   From I2C specification: Min Tlow = 1.3us, min Thigh = 0.6us,
 *   max Trise=0.3us, max Tfall=0.3us. Since ratio is 6:3, have to use
 *   worst case value of Tlow or 2xThigh as base.
 *
 *   1/(Tlow + Thigh + 0.3us + 0.3us) = 1/(1.3 + 0.65 + 0.6)us = 392157Hz.
 */
#define SL_HAL_I2C_FREQ_FAST_MAX        392157

/**
 * @brief
 *   Fast mode+ max frequency assuming using 11:6 ratio for Nlow:Nhigh.
 * @details
 *   From I2C specification: Min Tlow = 0.5us, min Thigh = 0.26us,
 *   max Trise=0.12us, max Tfall=0.12us. Since ratio is 11:6, have to use
 *   worst case value of Tlow or (11/6)xThigh as base.
 *
 *   1/(Tlow + Thigh + 0.12us + 0.12us) = 1/(0.5 + 0.273 + 0.24)us = 987167Hz.
 */
#define SL_HAL_I2C_FREQ_FASTPLUS_MAX    987167

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/
/***************************************************************************//**
 * This function initializes the I2C module in Leader/Follower Mode.
 *
 * @param[in] i2c            A pointer to the I2C Peripheral register block.
 * @param[in] mode           Leader or Follower mode.
 ******************************************************************************/
void sl_hal_i2c_init(I2C_TypeDef *i2c,
                     sl_i2c_operating_mode_t mode);

/***************************************************************************//**
 * This function is used to send the data.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @param[in] data    Data to transmit.
 ******************************************************************************/
void sl_hal_i2c_tx(I2C_TypeDef *i2c,
                   uint8_t data);

/***************************************************************************//**
 * This function is used to receive the data.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            Data received.
 ******************************************************************************/
uint8_t sl_hal_i2c_rx(I2C_TypeDef *i2c);

/***************************************************************************//**
 * The bus frequency is only relevant when acting as Leader. The bus
 * frequency should not be set higher than the maximum frequency accepted by the
 * slowest device on the bus.
 *
 * Notice that, due to asymmetric requirements on low and high I2C clock
 * cycles in the I2C specification, the maximum frequency allowed
 * to comply with the specification may be somewhat lower than expected.
 *
 * See the reference manual, details on I2C clock generation,
 * for maximum allowed theoretical frequencies for different modes.
 *
 * @param[in] i2c          A pointer to the I2C Peripheral register block.
 * @param[in] i2c_clk      An I2C Peripheral clock frequency in Hz that will be used.
 *                         Setting it to a higher than actual configured value has the
 *                         consequence of reducing the real I2C frequency.
 * @param[in] freq_scl     A bus frequency to set (bus speed may be lower due to integer
 *                         prescaling). Safe (according to the I2C specification) maximum
 *                         frequencies for standard fast and fast+ modes are available using
 *                         SL_I2C_FREQ_ defines.(Using SL_I2C_FREQ_ defines requires corresponding
 *                         setting of @p type.) The slowest slave device on a bus must always be considered.
 * @param[in] clhr         A clock low-to-high ratio type to use. If not using SL_I2C_CLK_HLR_STANDARD, make
 *                         sure all devices on the bus support the specified mode. Using a non-standard
 *                         ratio is useful to achieve a higher bus clock in fast and fast+ modes.

 ******************************************************************************/
void sl_hal_i2c_set_clock_frequency(I2C_TypeDef *i2c,
                                    uint32_t i2c_clk,
                                    uint32_t freq_scl,
                                    sl_i2c_clock_hlr_t clhr);

/***************************************************************************//**
 * Get I2C clock frequency.
 *
 * @param[in] i2c          A pointer to the I2C Peripheral register block.
 * @param[in] i2c_clk      An I2C Peripheral clock frequency in Hz that will be used.
 *                         Setting it to a higher than actual configured value has the
 *                         consequence of reducing the real I2C frequency.Make sure to
 *                         give i2c_clk as given in sl_hal_i2c_set_clock_frequency.
 * @return                 Clock frequency value.
 ******************************************************************************/
uint32_t sl_hal_i2c_get_clock_frequency(I2C_TypeDef *i2c,
                                        uint32_t i2c_clk);

/***************************************************************************//**
 * Configure I2C to its reset state.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 ******************************************************************************/
void sl_hal_i2c_reset(I2C_TypeDef *i2c);

/***************************************************************************//**
 * This function will send a start command.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            On Success returns SL_STATUS_OK.
 *                    On Failure returns SL_STATUS_NOT_INITIALIZED.
 ******************************************************************************/
sl_status_t sl_hal_i2c_start_cmd(I2C_TypeDef *i2c);

/***************************************************************************//**
 * This function will send a stop command.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            On Success returns SL_STATUS_OK.
 *                    On Failure returns SL_STATUS_NOT_INITIALIZED.
 ******************************************************************************/
sl_status_t sl_hal_i2c_stop_cmd(I2C_TypeDef *i2c);

/***************************************************************************//**
 * This function clears the Tx , RX Buffer , Shift registers and Pending Commands.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 ******************************************************************************/
void sl_hal_i2c_flush_buffers(I2C_TypeDef *i2c);

/***************************************************************************//**
 * Wait for disabling to finish.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_i2c_wait_ready(I2C_TypeDef *i2c)
{
#if defined(_I2C_EN_DISABLING_MASK)
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  while (i2c->EN & _I2C_EN_DISABLING_MASK) {
    // Wait for disabling to finish
  }
#else
  (void)i2c;
#endif //_I2C_EN_DISABLING_MASK
}

/***************************************************************************//**
 * Wait for all synchronizations to finish.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_i2c_wait_sync(I2C_TypeDef *i2c)
{
#if defined(_I2C_SYNCBUSY_MASK) && defined(_I2C_STATUS_CLEARTXBUSY_MASK) && defined(_I2C_STATUS_CLEARRXBUSY_MASK)
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  while ( ((i2c->EN & _I2C_EN_EN_MASK) && (i2c->SYNCBUSY & _I2C_SYNCBUSY_MASK))
          || ((i2c->EN & _I2C_EN_EN_MASK) && (i2c->STATUS & (_I2C_STATUS_CLEARTXBUSY_MASK
                                                             | _I2C_STATUS_CLEARRXBUSY_MASK)))) {
    // Wait for all synchronizations to finish.
  }
#else
  (void)i2c;
#endif
}

/***************************************************************************//**
 * This function Enables I2C Peripheral.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.  *
 ******************************************************************************/
__INLINE void sl_hal_i2c_enable(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Enable peripheral.
  i2c->EN_SET = I2C_EN_EN;
}

/***************************************************************************//**
 * This function Disables I2C Peripheral.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 ******************************************************************************/
__INLINE void sl_hal_i2c_disable(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Make sure module is enabled.
  if ((i2c->EN & _I2C_EN_EN_MASK) != I2C_EN_EN) {
    return;
  }

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Disable I2C.
  i2c->EN_CLR = I2C_EN_EN;

  // Wait for disabling to finish.
  sl_hal_i2c_wait_ready(i2c);
}

/***************************************************************************//**
 * This function is used to send the ACK on the I2C Bus.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            On Success returns SL_STATUS_OK.
 *                    On Failure returns SL_STATUS_NOT_INITIALIZED.
 ******************************************************************************/
__INLINE sl_status_t sl_hal_i2c_send_ack(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  sl_status_t status = SL_STATUS_OK;

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Make sure module is enabled.
  if ((i2c->EN & _I2C_EN_EN_MASK) != I2C_EN_EN) {
    status = SL_STATUS_NOT_INITIALIZED;
  } else {
    // Intiate ACK.
    i2c->CMD  = I2C_CMD_ACK;
  }

  return status;
}

/***************************************************************************//**
 * This function is used to enable/disable the Auto ACK feature on the I2C Bus.
 * It is used only in Leader/Follower receiver mode.
 *
 * @param[in] i2c      A pointer to the I2C Peripheral register block.
 * @param[in] enable   Enable or Disable the Auto ack.
 ******************************************************************************/
__INLINE void sl_hal_i2c_auto_ack(I2C_TypeDef *i2c,
                                  uint8_t enable)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Enable or Disable Auto Ack.
  i2c->CTRL  = (enable << _I2C_CTRL_AUTOACK_SHIFT);
}

/***************************************************************************//**
 * This function is used to send the NACK on the I2C Bus.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            On Success returns SL_STATUS_OK.
 *                    On Failure returns SL_STATUS_NOT_INITIALIZED.
 ******************************************************************************/
__INLINE sl_status_t sl_hal_i2c_send_nack(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  sl_status_t status = SL_STATUS_OK;

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Make sure module is enabled.
  if ((i2c->EN & _I2C_EN_EN_MASK) != I2C_EN_EN) {
    status = SL_STATUS_NOT_INITIALIZED;
  } else {
    // Intiate NACK.
    i2c->CMD  = I2C_CMD_NACK;
  }

  return status;
}

/***************************************************************************//**
 * This function is used to get the I2C STATUS register.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            STATUS register value.
 ******************************************************************************/
__INLINE uint32_t sl_hal_i2c_get_status(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  return i2c->STATUS;
}

/***************************************************************************//**
 * Enables one or more I2C interrupts.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @param[in] flags   I2C interrupt sources to enable.
 *                    Use a set of interrupt flags OR-ed together to set
 *                    multiple interrupt sources.
 ******************************************************************************/
__INLINE void sl_hal_i2c_enable_interrupts(I2C_TypeDef *i2c,
                                           uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  i2c->IEN_SET = flags;
}

/***************************************************************************//**
 * Disables one or more I2C interrupts.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @param[in] flags   I2C interrupt sources to disable.
 *                    Use a set of interrupt flags OR-ed together to set
 *                    multiple interrupt sources.
 ******************************************************************************/
__INLINE void sl_hal_i2c_disable_interrupts(I2C_TypeDef *i2c,
                                            uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  i2c->IEN_CLR = flags;
}

/***************************************************************************//**
 * Sets one or more pending I2C interrupts from Software.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @param[in] flags   I2C interrupt sources to set to pending.
 *                    Use a set of interrupt flags OR-ed together to set
 *                    multiple interrupt sources.
 ******************************************************************************/
__INLINE void sl_hal_i2c_set_interrupts(I2C_TypeDef *i2c,
                                        uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  i2c->IF_SET = flags;
}

/***************************************************************************//**
 * Clears one or more pending I2C interrupts from Software.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @param[in] flags   I2C interrupt sources to clear.
 *                    Use a set of interrupt flags OR-ed together to clear
 *                    multiple interrupt sources.
 ******************************************************************************/
__INLINE void sl_hal_i2c_clear_interrupts(I2C_TypeDef *i2c,
                                          uint32_t flags)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  i2c->IF_CLR = flags;
}

/***************************************************************************//**
 * Get pending I2C interrupt flags.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            Pending I2C interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_i2c_get_pending_interrupts(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  return i2c->IF;
}

/***************************************************************************//**
 * Get enabled I2C interrupt flags.
 * Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            Enabled I2C interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_i2c_get_enabled_interrupts(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  return i2c->IEN;
}

/***************************************************************************//**
 * Get enabled and Pending I2C interrupt flags.
 * Useful for handling more interrupt sources in the same interrupt handler.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            Pending and enabled I2C interrupt sources.
 ******************************************************************************/
__INLINE uint32_t sl_hal_i2c_get_enabled_pending_interrupts(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  uint32_t enabled_interrupts;

  // Store I2C->IEN in enabled_interrupts variable.
  enabled_interrupts = i2c->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return i2c->IF & enabled_interrupts;
}

/***************************************************************************//**
 * Set Follower address to use for I2C peripheral (when operating in Follower mode).
 *
 * @param[in] i2c            A pointer to the I2C Peripheral register block.
 * @param[in] addr           I2C Follower address to use. The 7 most significant bits define the actual
 *                           address, the least significant bit is reserved and always set to 0.
 * @param[in] is_10bit_addr  7bit or 10bit address configuration.
 ******************************************************************************/
__INLINE void sl_hal_i2c_set_follower_address(I2C_TypeDef *i2c,
                                              uint16_t addr,
                                              bool is_10bit_addr)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  // Check for 10bit or 7bit address.
  if (is_10bit_addr) {
    i2c->SADDR = (((uint32_t)(addr) >> 8) & 0x06) | SL_I2C_FIRST_BYTE_10BIT_ADDR_MASK;
  } else {
    i2c->SADDR = (uint32_t)addr & _I2C_SADDR_MASK;
  }
}

/***************************************************************************//**
 * Get Follower address used for I2C peripheral (when operating in Follower mode).
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            I2C Follower address in use. The 7 most significant bits define the actual
 *                    address, the least significant bit is reserved and always returned as 0.
 ******************************************************************************/
__INLINE uint8_t sl_hal_i2c_get_follower_address(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  return ((uint8_t)(i2c->SADDR));
}

/***************************************************************************//**
 * Set Follower address to use for I2C peripheral (when operating in Follower mode).
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @param[in] mask    Specifies the significant bits of the follower address. Setting the mask to 0x00 will match all addresses, while setting it to
                      0x7F will only match the exact address specified by ADDR.
 ******************************************************************************/
__INLINE void sl_hal_i2c_set_follower_mask_address(I2C_TypeDef *i2c,
                                                   uint8_t mask)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  i2c->SADDRMASK = (uint32_t)mask & _I2C_SADDRMASK_MASK;
}

/***************************************************************************//**
 * Get Follower address mask used for I2C peripheral (when operating in Follower mode).
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            I2C Target address mask in use. The 7 most significant bits define the
 *                    actual address mask, the least significant bit is reserved and always returned as 0.
 ******************************************************************************/
__INLINE uint8_t sl_hal_i2c_get_follower_mask_address(I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Wait for synchronization to complete.
  sl_hal_i2c_wait_sync(i2c);

  return ((uint8_t)(i2c->SADDRMASK));
}

#if defined(_I2C_CTRL_TXFIL_SHIFT)
/***************************************************************************//**
 * Sets the TX FIFO Threshold.
 *
 * @param[in] i2c             A pointer to the I2C Peripheral register block.
 * @param[in] threshold_value Tx fifo Threshold.
 ******************************************************************************/
__INLINE void sl_hal_i2c_set_tx_fifo_threshold(I2C_TypeDef *i2c,
                                               sl_i2c_fifo_threshold_t threshold_value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Configure the TX FIFO.
  i2c->CTRL_SET = (threshold_value << _I2C_CTRL_TXFIL_SHIFT);
}

/***************************************************************************//**
 * Gets the TX FIFO Threshold.
 *
 * @param[in] i2c   A pointer to the I2C Peripheral register block.
 * @return          Returns the configured tx fifo threshold.
 ******************************************************************************/
__INLINE sl_i2c_fifo_threshold_t sl_hal_i2c_get_tx_fifo_threshold(const I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Return the TX FIFO value.
  return (sl_i2c_fifo_threshold_t)((i2c->CTRL >> _I2C_CTRL_TXFIL_SHIFT) & 0x0F);
}
#endif //_I2C_CTRL_TXFIL_SHIFT

#if defined(_I2C_CTRL_RXFIL_SHIFT)
/***************************************************************************//**
 * Sets the RX FIFO Threshold.
 *
 * @param[in] i2c             A pointer to the I2C Peripheral register block.
 * @param[in] threshold_value Rx fifo threshold.
 ******************************************************************************/
__INLINE void sl_hal_i2c_set_rx_fifo_threshold(I2C_TypeDef *i2c,
                                               sl_i2c_fifo_threshold_t threshold_value)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Configure the TX FIFO.
  i2c->CTRL_SET = (threshold_value << _I2C_CTRL_RXFIL_SHIFT);
}

/***************************************************************************//**
 * Gets the RX FIFO Threshold.
 *
 * @param[in] i2c     A pointer to the I2C Peripheral register block.
 * @return            Returns the configured tx fifo threshold.
 ******************************************************************************/
__INLINE sl_i2c_fifo_threshold_t sl_hal_i2c_get_rx_fifo_threshold(const I2C_TypeDef *i2c)
{
  // Make sure the module exists on the selected chip.
  EFM_ASSERT(SL_HAL_I2C_REF_VALID(i2c));

  // Return the TX FIFO value.
  return (sl_i2c_fifo_threshold_t)((i2c->CTRL >> _I2C_CTRL_RXFIL_SHIFT) & 0x0F);
}
#endif //_I2C_CTRL_RXFIL_SHIFT

/** @} (end addtogroup i2c) */

#ifdef __cplusplus
}
#endif

/* *INDENT-OFF* */
/***************************************************************************//**
 * @addtogroup i2c I2C - Inter-Integrated Circuit
 * @{
 * @li @ref i2c_intro
 * @li @ref i2c_example
 *
 *@n @section i2c_intro Introduction
 *  This module contains functions to control the I2C peripheral of Silicon Labs 32-bit MCUs and SoCs.
 *  The I2C interface allows communication on I2C buses with the lowest energy consumption possible.
 *
 *@n @section i2c_example Example
 *
 *  I2C TX example:
 *  @code{.c}
 *  {
 *    // Enable bus clocks for I2C0 and GPIO
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_I2C0);
 *    sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
 *
 *    // I2C init.
 *    sl_hal_i2c_init(i2c_peripheral, SL_I2C_LEADER_MODE);
 *
 *    // Set the clock frequency.
 *    sl_hal_i2c_set_clock_frequency(i2c_peripheral, i2c_clk, freq_scl, clhr);
 *
 *    // Enable i2c.
 *    sl_hal_i2c_enable(i2c_peripheral);
 *
 *    follower_addr &= 0xfe;
 *
 *    // Initiate start command.
 *    sl_hal_i2c_start_cmd(i2c_peripheral);
 *
 *   // Transmit follower address with write mode.
 *   sl_hal_i2c_tx(i2c_peripheral, follower_addr);
 *  }
 *
 *  @endcode
 *
 * @}  (end addtogroup i2c)
 ******************************************************************************/
/* *INDENT-ON* */

#endif /* defined(I2C_PRESENT) */
#endif /* #ifndef SL_HAL_I2C_H */
