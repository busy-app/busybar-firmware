/***************************************************************************//**
 * @file
 * @brief Inter-Integrated Circuit (I2C) driver API
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

#include <string.h>
#include "sl_sleeptimer.h"
#include "sl_clock_manager.h"
#include "sl_udelay.h"
#include "sl_interrupt_manager.h"
#include "sl_hal_i2c.h"
#include "sl_hal_gpio.h"
#include "sl_i2c.h"
#include "sli_i2c.h"

/*******************************************************************************
*******************************   DEFINES   ***********************************
*******************************************************************************/
/**
 * @brief Number of clock cycles for bus recovery.
 *
 * @note
 *   Set to 10 clock cycles to ensure the bus can be cleared effectively,
 *   allowing for proper recovery.
 *   This helps resume communication quickly without leaving the bus in a stalled state.
 */
#define I2C_RECOVER_NUM_CLOCKS  10

/**
 * @brief SCL hold time in microseconds.
 *
 * @note
 *   Set to 100 microseconds to ensure proper synchronization
 *   between leader and follower devices.
 *   This provides enough time for data processing on the SDA line
 *   and ensures reliable communication, even with slower devices.
 */
#ifndef I2C_SCL_HOLD_TIME_US
#define I2C_SCL_HOLD_TIME_US 100
#endif

/*******************************************************************************
*****************************   LOCAL VARIABLES   *****************************
*******************************************************************************/
/// Active transfer contexts for each I2C instance.
static sl_i2c_handle_t *i2c_handle_contexts[I2C_COUNT] = { NULL };

// Lookup tables for I2C frequency and clock high/low ratio (CLHR).
static const uint32_t i2c_max_freq_table[] = {
  [SL_I2C_FREQ_STANDARD_MODE] = SL_HAL_I2C_FREQ_STANDARD_MAX,
  [SL_I2C_FREQ_FAST_MODE]     = SL_HAL_I2C_FREQ_FAST_MAX,
  [SL_I2C_FREQ_FASTPLUS_MODE] = SL_HAL_I2C_FREQ_FASTPLUS_MAX
};

static const sl_i2c_clock_hlr_t i2c_clhr_table[] = {
  [SL_I2C_FREQ_STANDARD_MODE] = SL_I2C_CLK_HLR_STANDARD,
  [SL_I2C_FREQ_FAST_MODE]     = SL_I2C_CLK_HLR_ASYMMETRIC,
  [SL_I2C_FREQ_FASTPLUS_MODE] = SL_I2C_CLK_HLR_FAST
};

/// Temporary buffer for I2C operations.
static uint8_t temp_buffer[] = { 0xFF };

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/
static uint32_t get_current_time_ms(void);
static sl_status_t i2c_setup_blocking_transfer_interrupts(sl_i2c_handle_t *i2c_handle);
static sl_status_t i2c_leader_mode_blocking_state_machine(sl_i2c_handle_t *i2c_handle,
                                                          const uint8_t *tx_buffer,
                                                          uint32_t tx_len,
                                                          uint8_t *rx_buffer,
                                                          uint32_t rx_len,
                                                          uint32_t timeout);
static sl_status_t i2c_leader_mode_write_no_stop_blocking_state_machine(sl_i2c_handle_t *i2c_handle,
                                                                        const uint8_t *tx_buffer1,
                                                                        uint32_t tx_len1,
                                                                        const uint8_t *tx_buffer2,
                                                                        uint32_t tx_len2,
                                                                        uint32_t timeout);
static sl_status_t i2c_follower_mode_blocking_state_machine(sl_i2c_handle_t *i2c_handle,
                                                            const uint8_t *tx_buffer,
                                                            uint32_t tx_len,
                                                            uint8_t *rx_buffer,
                                                            uint32_t rx_len,
                                                            uint32_t timeout);
static sl_status_t i2c_get_dma_trigger_signals(sl_i2c_handle_t *i2c_handle,
                                               DMADRV_PeripheralSignal_t *dma_tx_trigger_source,
                                               DMADRV_PeripheralSignal_t *dma_rx_trigger_source);
static void stop_active_dma_transfers(sl_i2c_handle_t *i2c_handle);
static sl_status_t i2c_setup_leader_non_blocking_dma_transfer(sl_i2c_handle_t *i2c_handle,
                                                              const uint8_t *tx_buffer,
                                                              uint32_t tx_len,
                                                              uint8_t *rx_buffer,
                                                              uint32_t rx_len);
static sl_status_t i2c_setup_follower_non_blocking_dma_transfer(sl_i2c_handle_t *i2c_handle,
                                                                const uint8_t *tx_buffer,
                                                                uint32_t tx_len,
                                                                uint8_t *rx_buffer,
                                                                uint32_t rx_len);
static void i2c_common_irq_handler(uint8_t i2c_instance);

/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
*******************************************************************************/

/***************************************************************************//**
 * Initializes the I2C Module.
 ******************************************************************************/
sl_status_t sl_i2c_init(sl_i2c_handle_t *i2c_handle,
                        const sl_i2c_init_params_t *init_params)
{
  CORE_DECLARE_IRQ_STATE;
  I2C_TypeDef *i2c_base_addr;
  sl_bus_clock_t i2c_bus_clk;
  int8_t i2c_instance_num;

  // Parameter validation
  if (i2c_handle == NULL || init_params == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Validate I2C peripheral instance
  i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(init_params->i2c_peripheral);
  i2c_instance_num = I2C_NUM(i2c_base_addr);
  if (!SL_HAL_I2C_REF_VALID(i2c_base_addr)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  // Validate operating mode
  if (!SL_I2C_OPERATING_MODE_IS_VALID(init_params->operating_mode)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate frequency mode
  if (!SL_I2C_FREQ_MODE_IS_VALID(init_params->frequency_mode)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Check if the I2C instance is already initialized
  if (i2c_handle_contexts[i2c_instance_num] != NULL) {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_ALREADY_INITIALIZED;
  }

  // Enable clocks
  i2c_bus_clk = sl_device_peripheral_get_bus_clock(init_params->i2c_peripheral);
  sl_clock_manager_enable_bus_clock(i2c_bus_clk);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);

  // GPIO Configuration
  sl_hal_gpio_set_pin_mode(&(init_params->scl_gpio), SL_GPIO_MODE_WIRED_AND_PULLUP, 1);
  sl_hal_gpio_set_pin_mode(&(init_params->sda_gpio), SL_GPIO_MODE_WIRED_AND_PULLUP, 1);

  // Generate 9 clock pulses to recover the follower device from an unknown state.
  // Refer to the 'Bus Lock-Up' section in the I2C part of the reference manual.
  if (init_params->operating_mode == SL_I2C_LEADER_MODE) {
    for (int clk_pulses = 0; clk_pulses < I2C_RECOVER_NUM_CLOCKS; clk_pulses++) {
      sl_hal_gpio_clear_pin(&(init_params->scl_gpio));
      sl_udelay_wait(I2C_SCL_HOLD_TIME_US);
      sl_hal_gpio_set_pin(&(init_params->scl_gpio));
      sl_udelay_wait(I2C_SCL_HOLD_TIME_US);
    }
  }

  // GPIO Routing
  GPIO->I2CROUTE[i2c_instance_num].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;
  GPIO->I2CROUTE[i2c_instance_num].SCLROUTE = (uint32_t)((init_params->scl_gpio.pin << _GPIO_I2C_SCLROUTE_PIN_SHIFT)
                                                         | (init_params->scl_gpio.port << _GPIO_I2C_SCLROUTE_PORT_SHIFT));
  GPIO->I2CROUTE[i2c_instance_num].SDAROUTE = (uint32_t)((init_params->sda_gpio.pin << _GPIO_I2C_SDAROUTE_PIN_SHIFT)
                                                         | (init_params->sda_gpio.port << _GPIO_I2C_SDAROUTE_PORT_SHIFT));

  // Fill I2C handle structure with initialization parameters
  i2c_handle->i2c_peripheral = init_params->i2c_peripheral;
  i2c_handle->operating_mode = init_params->operating_mode;
  i2c_handle->frequency_mode = init_params->frequency_mode;
  i2c_handle->scl_gpio = init_params->scl_gpio;
  i2c_handle->sda_gpio = init_params->sda_gpio;

  // DMA Configuration
  DMADRV_Init();
  if (DMADRV_AllocateChannel(&(i2c_handle->dma_channel.dma_tx_channel), NULL) != ECODE_EMDRV_DMADRV_OK) {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_ALLOCATION_FAILED;
  }
  if (DMADRV_AllocateChannel(&(i2c_handle->dma_channel.dma_rx_channel), NULL) != ECODE_EMDRV_DMADRV_OK) {
    DMADRV_FreeChannel(i2c_handle->dma_channel.dma_tx_channel);
    CORE_EXIT_ATOMIC();
    return SL_STATUS_ALLOCATION_FAILED;
  }

  // I2C instance Configuration
  sli_i2c_init_core(i2c_handle);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Deinitializes and resets the I2C instance.
 ******************************************************************************/
sl_status_t sl_i2c_deinit(sl_i2c_handle_t *i2c_handle)
{
  CORE_DECLARE_IRQ_STATE;
  sl_bus_clock_t i2c_bus_clk;
  I2C_TypeDef *i2c_base_addr;
  int8_t i2c_instance_num;

  // Validate handle
  if (i2c_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  // Get the I2C instance and base address from the handle
  i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  i2c_instance_num = I2C_NUM(i2c_base_addr);

  // De-Allocate DMA channels
  DMADRV_FreeChannel(i2c_handle->dma_channel.dma_tx_channel);
  DMADRV_FreeChannel(i2c_handle->dma_channel.dma_rx_channel);

  // Clear follower address if in follower mode
  if (i2c_handle->operating_mode == SL_I2C_FOLLOWER_MODE) {
    sl_hal_i2c_set_follower_address(i2c_base_addr, 0x00, false);
  }

  // Flush buffers and reset I2C peripheral
  sl_hal_i2c_flush_buffers(i2c_base_addr);
  sl_hal_i2c_reset(i2c_base_addr);

  // Disable i2c peripheral
  sl_hal_i2c_disable(i2c_base_addr);

  // Reset GPIO configuration
  GPIO->I2CROUTE[i2c_instance_num].ROUTEEN = _GPIO_I2C_ROUTEEN_RESETVALUE;
  GPIO->I2CROUTE[i2c_instance_num].SCLROUTE = _GPIO_I2C_SCLROUTE_RESETVALUE;
  GPIO->I2CROUTE[i2c_instance_num].SDAROUTE = _GPIO_I2C_SDAROUTE_RESETVALUE;

  sl_hal_gpio_set_pin_mode(&(i2c_handle->scl_gpio), SL_GPIO_MODE_DISABLED, 0);
  sl_hal_gpio_set_pin_mode(&(i2c_handle->sda_gpio), SL_GPIO_MODE_DISABLED, 0);

  // Disable the clock for the I2C peripheral
  i2c_bus_clk = sl_device_peripheral_get_bus_clock(i2c_handle->i2c_peripheral);
  sl_clock_manager_disable_bus_clock(i2c_bus_clk);

  // Clear the initialization flag
  i2c_handle_contexts[i2c_instance_num] = NULL;

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets the frequency of the I2C bus.
 ******************************************************************************/
sl_status_t sl_i2c_set_frequency(sl_i2c_handle_t *i2c_handle,
                                 sl_i2c_freq_mode_t frequency_mode)
{
  CORE_DECLARE_IRQ_STATE;
  uint32_t freq = 0, max_freq = 0;
  sl_i2c_clock_hlr_t clhr = 0;
  sl_clock_branch_t i2c_clk_branch;

  // Validate handle and frequency mode
  if (i2c_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  if (!SL_I2C_FREQ_MODE_IS_VALID(frequency_mode)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  i2c_clk_branch = sl_device_peripheral_get_clock_branch(i2c_handle->i2c_peripheral);
  sl_clock_manager_get_clock_branch_frequency(i2c_clk_branch, &freq);

  max_freq = i2c_max_freq_table[frequency_mode];
  clhr     = i2c_clhr_table[frequency_mode];

  sl_hal_i2c_set_clock_frequency(i2c_base_addr, freq, max_freq, clhr);

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Gets the current configured frequency of the I2C module.
 ******************************************************************************/
sl_status_t sl_i2c_get_frequency(sl_i2c_handle_t *i2c_handle,
                                 sl_i2c_freq_mode_t *frequency_mode)
{
  CORE_DECLARE_IRQ_STATE;
  uint32_t clock_frequency = 0, freq = 0;
  sl_clock_branch_t i2c_clk_branch;

  if (i2c_handle == NULL || frequency_mode == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  i2c_clk_branch = sl_device_peripheral_get_clock_branch(i2c_handle->i2c_peripheral);
  sl_clock_manager_get_clock_branch_frequency(i2c_clk_branch, &clock_frequency);

  freq = sl_hal_i2c_get_clock_frequency(i2c_base_addr, clock_frequency);

  if (freq <= SL_HAL_I2C_FREQ_STANDARD_MAX) {
    *frequency_mode = SL_I2C_FREQ_STANDARD_MODE;
  } else if (freq <= SL_HAL_I2C_FREQ_FAST_MAX) {
    *frequency_mode = SL_I2C_FREQ_FAST_MODE;
  } else if (freq <= SL_HAL_I2C_FREQ_FASTPLUS_MAX) {
    *frequency_mode = SL_I2C_FREQ_FASTPLUS_MODE;
  } else {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set the follower device address for I2C communication.
 ******************************************************************************/
sl_status_t sl_i2c_set_follower_address(sl_i2c_handle_t *i2c_handle,
                                        uint16_t address,
                                        bool is_10bit_addr)
{
  CORE_DECLARE_IRQ_STATE;

  // Null pointer validation
  if (i2c_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Operating mode validation
  if (i2c_handle->operating_mode != SL_I2C_FOLLOWER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Follower address validation
  if (address > 0x3FF) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Add reserved address validation for 7-bit addresses
  if (!is_10bit_addr) {
    // Check for reserved 7-bit addresses (0x00-0x07 and 0x78-0x7F)
    if ((address <= 0x07) || (address >= 0x78)) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  CORE_ENTER_ATOMIC();

  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);

  // Set the follower address in the I2C peripheral
  i2c_handle->follower_address = address;
  sl_hal_i2c_set_follower_address(i2c_base_addr, (address << 1), is_10bit_addr);
  sl_hal_i2c_set_follower_mask_address(i2c_base_addr, 0xFE); // 0xFE means exact address match

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Configures the DMA channels for the I2C instance.
 ******************************************************************************/
sl_status_t sl_i2c_configure_dma(sl_i2c_handle_t *i2c_handle,
                                 sl_i2c_dma_channel_info_t *dma_channel)
{
  CORE_DECLARE_IRQ_STATE;

  if (i2c_handle == NULL || dma_channel == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  if (i2c_handle->dma_channel.dma_tx_channel != dma_channel->dma_tx_channel) {
    if (DMADRV_AllocateChannel(&dma_channel->dma_tx_channel, NULL) != ECODE_EMDRV_DMADRV_OK) {
      CORE_EXIT_ATOMIC();
      return SL_STATUS_ALLOCATION_FAILED;
    }

    DMADRV_FreeChannel(i2c_handle->dma_channel.dma_tx_channel);
    i2c_handle->dma_channel.dma_tx_channel = dma_channel->dma_tx_channel;
  }

  if (i2c_handle->dma_channel.dma_rx_channel != dma_channel->dma_rx_channel) {
    if (DMADRV_AllocateChannel(&dma_channel->dma_rx_channel, NULL) != ECODE_EMDRV_DMADRV_OK) {
      DMADRV_FreeChannel(i2c_handle->dma_channel.dma_tx_channel);
      CORE_EXIT_ATOMIC();
      return SL_STATUS_ALLOCATION_FAILED;
    }

    DMADRV_FreeChannel(i2c_handle->dma_channel.dma_rx_channel);
    i2c_handle->dma_channel.dma_rx_channel = dma_channel->dma_rx_channel;
  }

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Leader Mode: Send data to the specified follower device (blocking).
 ******************************************************************************/
sl_status_t sl_i2c_leader_send_blocking(sl_i2c_handle_t *i2c_handle,
                                        uint16_t address,
                                        const uint8_t *tx_buffer,
                                        uint32_t tx_len,
                                        uint32_t timeout)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || tx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow leader mode for this API
  if (i2c_handle->operating_mode != SL_I2C_LEADER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the length of data to send
  if (tx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate follower address range
  if (address > 0x3FF) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->follower_address = address;
  i2c_handle->tx_offset = 0;
  i2c_handle->transfer_direction = SL_I2C_WRITE;

  // Initialize the I2C transfer
  i2c_setup_blocking_transfer_interrupts(i2c_handle);

  status = i2c_leader_mode_blocking_state_machine(i2c_handle, tx_buffer, tx_len, NULL, 0, timeout);

  CORE_EXIT_ATOMIC();
  return status;
}

/***************************************************************************//**
 * Leader Mode: Receive data from the specified follower device (blocking).
 ******************************************************************************/
sl_status_t sl_i2c_leader_receive_blocking(sl_i2c_handle_t *i2c_handle,
                                           uint16_t address,
                                           uint8_t *rx_buffer,
                                           uint32_t rx_len,
                                           uint32_t timeout)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || rx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow leader mode for this API
  if (i2c_handle->operating_mode != SL_I2C_LEADER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the length of data to send
  if (rx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate follower address range
  if (address > 0x3FF) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->follower_address = address;
  i2c_handle->rx_offset = 0;
  i2c_handle->transfer_direction = SL_I2C_READ;

  // Initialize the I2C transfer
  i2c_setup_blocking_transfer_interrupts(i2c_handle);

  status = i2c_leader_mode_blocking_state_machine(i2c_handle, NULL, 0, rx_buffer, rx_len, timeout);

  CORE_EXIT_ATOMIC();
  return status;
}

/***************************************************************************//**
 * Leader Mode: Send then receive data from the specified follower device (blocking).
 ******************************************************************************/
sl_status_t sl_i2c_leader_transfer_blocking(sl_i2c_handle_t *i2c_handle,
                                            uint16_t address,
                                            const uint8_t *tx_buffer,
                                            uint32_t tx_len,
                                            uint8_t *rx_buffer,
                                            uint32_t rx_len,
                                            uint32_t timeout)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || tx_buffer == NULL || rx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow leader mode for this API
  if (i2c_handle->operating_mode != SL_I2C_LEADER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the input and length of data to send and receive
  if (tx_len == 0 || rx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validate follower address range
  if (address > 0x3FF) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->follower_address = address;
  i2c_handle->tx_offset = 0;
  i2c_handle->rx_offset = 0;
  i2c_handle->transfer_direction = SL_I2C_WRITE_READ;

  // Initialize the I2C transfer
  i2c_setup_blocking_transfer_interrupts(i2c_handle);

  status = i2c_leader_mode_blocking_state_machine(i2c_handle, tx_buffer, tx_len, rx_buffer, rx_len, timeout);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Leader Mode: Send data to the specified follower device (non-blocking).
 ******************************************************************************/
sl_status_t sl_i2c_leader_send_non_blocking(sl_i2c_handle_t *i2c_handle,
                                            uint16_t address,
                                            const uint8_t *tx_buffer,
                                            uint32_t tx_len,
                                            void *user_data)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || tx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow leader mode for this API
  if (i2c_handle->operating_mode != SL_I2C_LEADER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the length of data to receive
  if (tx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->follower_address = address;
  i2c_handle->transfer_direction = SL_I2C_WRITE;
  i2c_handle->event = SL_I2C_EVENT_IDLE;
  i2c_handle->user_data = user_data;

  status = i2c_setup_leader_non_blocking_dma_transfer(i2c_handle, tx_buffer, tx_len, NULL, 0);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Leader Mode: Receive data from the specified follower device (non-blocking).
 ******************************************************************************/
sl_status_t sl_i2c_leader_receive_non_blocking(sl_i2c_handle_t *i2c_handle,
                                               uint16_t address,
                                               uint8_t *rx_buffer,
                                               uint32_t rx_len,
                                               void *user_data)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || rx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow leader mode for this API
  if (i2c_handle->operating_mode != SL_I2C_LEADER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the length of data to receive
  if (rx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->follower_address = address;
  i2c_handle->transfer_direction = SL_I2C_READ;
  i2c_handle->event = SL_I2C_EVENT_IDLE;
  i2c_handle->user_data = user_data;

  status = i2c_setup_leader_non_blocking_dma_transfer(i2c_handle, NULL, 0, rx_buffer, rx_len);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Leader Mode: Send then receive data from the specified follower device (non-blocking).
 ******************************************************************************/
sl_status_t sl_i2c_leader_transfer_non_blocking(sl_i2c_handle_t *i2c_handle,
                                                uint16_t address,
                                                const uint8_t *tx_buffer,
                                                uint32_t tx_len,
                                                uint8_t *rx_buffer,
                                                uint32_t rx_len,
                                                void *user_data)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Null pointer validation
  if (i2c_handle == NULL || tx_buffer == NULL || rx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Operating mode validation
  if (i2c_handle->operating_mode != SL_I2C_LEADER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Tx and Rx length validation
  if (tx_len == 0 || rx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize i2c instance structure
  i2c_handle->follower_address = address;
  i2c_handle->transfer_direction = SL_I2C_WRITE_READ;
  i2c_handle->event = SL_I2C_EVENT_IDLE;
  i2c_handle->user_data = user_data;

  status = i2c_setup_leader_non_blocking_dma_transfer(i2c_handle, tx_buffer, tx_len, rx_buffer, rx_len);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Follower Mode: Send data to the I2C leader (blocking).
 ******************************************************************************/
sl_status_t sl_i2c_follower_send_blocking(sl_i2c_handle_t *i2c_handle,
                                          const uint8_t *tx_buffer,
                                          uint32_t tx_len,
                                          uint32_t timeout)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || tx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow follower mode for this API
  if (i2c_handle->operating_mode != SL_I2C_FOLLOWER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the input and length of data to send and receive
  if (tx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->tx_offset = 0;
  i2c_handle->transfer_direction = SL_I2C_WRITE;

  // Initialize the I2C transfer
  i2c_setup_blocking_transfer_interrupts(i2c_handle);

  status = i2c_follower_mode_blocking_state_machine(i2c_handle, tx_buffer, tx_len, NULL, 0, timeout);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Follower Mode: Receive data from the I2C leader (blocking).
 ******************************************************************************/
sl_status_t sl_i2c_follower_receive_blocking(sl_i2c_handle_t *i2c_handle,
                                             uint8_t *rx_buffer,
                                             uint32_t rx_len,
                                             uint32_t timeout)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || rx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow follower mode for this API
  if (i2c_handle->operating_mode != SL_I2C_FOLLOWER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the input and length of data to send and receive
  if (rx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->rx_offset = 0;
  i2c_handle->transfer_direction = SL_I2C_READ;

  // Initialize the I2C transfer
  i2c_setup_blocking_transfer_interrupts(i2c_handle);

  status = i2c_follower_mode_blocking_state_machine(i2c_handle, NULL, 0, rx_buffer, rx_len, timeout);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * Follower Mode: Send data to the I2C leader (non-blocking).
 ******************************************************************************/
sl_status_t sl_i2c_follower_send_non_blocking(sl_i2c_handle_t *i2c_handle,
                                              const uint8_t *tx_buffer,
                                              uint32_t tx_len,
                                              void *user_data)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Null pointer validation
  if (i2c_handle == NULL || tx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Operating mode validation
  if (i2c_handle->operating_mode != SL_I2C_FOLLOWER_MODE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Tx length validation
  if (tx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize the I2C instance structure
  i2c_handle->transfer_direction = SL_I2C_WRITE;
  i2c_handle->event = SL_I2C_EVENT_IDLE;
  i2c_handle->user_data = user_data;

  status = i2c_setup_follower_non_blocking_dma_transfer(i2c_handle, tx_buffer, tx_len, NULL, 0);

  CORE_EXIT_ATOMIC();
  return status;
}

/***************************************************************************//**
 * Follower Mode: Receive data from the I2C leader (non-blocking).
 ******************************************************************************/
sl_status_t sl_i2c_follower_receive_non_blocking(sl_i2c_handle_t *i2c_handle,
                                                 uint8_t *rx_buffer,
                                                 uint32_t rx_len,
                                                 void *user_data)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Null pointer validation
  if (i2c_handle == NULL || rx_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Operating mode validation
  if (i2c_handle->operating_mode != SL_I2C_FOLLOWER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Rx length validation
  if (rx_len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize the I2C instance structure
  i2c_handle->transfer_direction = SL_I2C_READ;
  i2c_handle->user_data = user_data;

  status = i2c_setup_follower_non_blocking_dma_transfer(i2c_handle, NULL, 0, rx_buffer, rx_len);

  CORE_EXIT_ATOMIC();
  return status;
}

/***************************************************************************//**
 * Register a transfer complete callback for non-blocking APIs.
 ******************************************************************************/
sl_status_t sl_i2c_set_transfer_complete_callback(sl_i2c_handle_t *i2c_handle,
                                                  sl_i2c_transfer_complete_callback_t callback)
{
  CORE_DECLARE_IRQ_STATE;

  // Validate input parameters
  if (i2c_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  i2c_handle->transfer_complete_callback = callback;

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Register an event callback for non-blocking APIs.
 ******************************************************************************/
sl_status_t sl_i2c_set_event_callback(sl_i2c_handle_t *i2c_handle,
                                      sl_i2c_event_callback_t callback)
{
  CORE_DECLARE_IRQ_STATE;

  // Validate input parameters
  if (i2c_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_ENTER_ATOMIC();

  i2c_handle->event_callback = callback;

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/*******************************************************************************
 **************************   INTERNAL FUNCTIONS   *****************************
 ******************************************************************************/

/***************************************************************************//**
 * Configure the I2C instance with initialization parameters.
 ******************************************************************************/
sl_status_t sli_i2c_init_core(sl_i2c_handle_t *i2c_handle)
{
  if (i2c_handle == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t ref_freq, max_freq = 0;
  sl_i2c_clock_hlr_t clhr = 0;
  sl_clock_branch_t i2c_clk_branch;
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  if (!SL_HAL_I2C_REF_VALID(i2c_base_addr)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Retrieve the I2C peripheral clock frequency
  i2c_clk_branch = sl_device_peripheral_get_clock_branch(i2c_handle->i2c_peripheral);
  if (sl_clock_manager_get_clock_branch_frequency(i2c_clk_branch, &ref_freq) != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  // Reset the I2C instance
  sl_hal_i2c_reset(i2c_base_addr);

  // Clear and disable all I2C interrupts to ensure clean state
  sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IF_MASK);
  sl_hal_i2c_disable_interrupts(i2c_base_addr, _I2C_IEN_MASK);

  // Initialize the I2C module with the appropriate operating mode (leader or follower)
  sl_hal_i2c_init(i2c_base_addr, i2c_handle->operating_mode);

  // Configure the I2C frequency based on the operating mode and the frequency mode
  if (i2c_handle->operating_mode == SL_I2C_LEADER_MODE) {
    max_freq = i2c_max_freq_table[i2c_handle->frequency_mode];
    clhr = i2c_clhr_table[i2c_handle->frequency_mode];
    sl_hal_i2c_set_clock_frequency(i2c_base_addr, ref_freq, max_freq, clhr);
  }

  // Enable the I2C peripheral
  sl_hal_i2c_enable(i2c_base_addr);

  // Register this I2C handle in the global context table.
  i2c_handle_contexts[I2C_NUM(i2c_base_addr)] = i2c_handle;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Dispatch interrupt handler for the I2C Leader
 ******************************************************************************/
void sli_i2c_leader_dispatch_interrupt(sl_i2c_handle_t *i2c_handle)
{
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  uint32_t pending_irq = sl_hal_i2c_get_enabled_pending_interrupts(i2c_base_addr);

  if (pending_irq & I2C_IF_ACK) {
    sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
    switch (i2c_handle->state) {
      case SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK:
        if (i2c_handle->follower_address < 0x7F) {
          i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
          if (i2c_handle->transfer_direction != SL_I2C_READ) {
            i2c_handle->state = SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK;
          }
        } else {
          i2c_handle->state = SL_I2C_STATE_ADDR_2ND_BYTE_10BIT_WAIT_FOR_ACK_OR_NACK;
        }
        break;

      case SL_I2C_STATE_ADDR_2ND_BYTE_10BIT_WAIT_FOR_ACK_OR_NACK:
        if (i2c_handle->transfer_direction != SL_I2C_READ) {
          i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
          i2c_handle->state = SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK;
        } else {
          sl_hal_i2c_start_cmd(i2c_base_addr);
          sl_hal_i2c_tx(i2c_base_addr, i2c_handle->addr_buffer[0] | 1);
          i2c_handle->state = SL_I2C_STATE_REPEATED_ADDR_WAIT_FOR_ACK_OR_NACK;
        }
        break;

      case SL_I2C_STATE_REPEATED_ADDR_WAIT_FOR_ACK_OR_NACK:
        i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
        break;
    }
  } else if (pending_irq & I2C_IF_TXC) {
    sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_TXC);
    sl_hal_i2c_disable_interrupts(i2c_base_addr, I2C_IEN_TXC);
    sl_hal_i2c_start_cmd(i2c_base_addr);
    sl_hal_i2c_tx(i2c_base_addr, i2c_handle->addr_buffer[0] | 1);
  } else if (pending_irq & I2C_IF_NACK) {
    sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);
    switch (i2c_handle->state) {
      case SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK:
      case SL_I2C_STATE_ADDR_2ND_BYTE_10BIT_WAIT_FOR_ACK_OR_NACK:
      case SL_I2C_STATE_REPEATED_ADDR_WAIT_FOR_ACK_OR_NACK:
        i2c_handle->event = SL_I2C_EVENT_INVALID_ADDR;
        break;
      case SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK:
        i2c_handle->event = SL_I2C_EVENT_DATA_NACK;
        break;
    }
    i2c_handle->state = SL_I2C_STATE_WAIT_FOR_STOP;
    sl_hal_i2c_stop_cmd(i2c_base_addr);
  } else if (pending_irq & I2C_IF_MSTOP) {
    sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    sl_hal_i2c_disable_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    stop_active_dma_transfers(i2c_handle);
    i2c_base_addr->CTRL = _I2C_CTRL_RESETVALUE;
    if (i2c_handle->event == SL_I2C_EVENT_IN_PROGRESS) {
      i2c_handle->event = SL_I2C_EVENT_COMPLETED;
    }
  } else if (pending_irq & SL_HAL_I2C_IF_ERRORS) {
    sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    sl_hal_i2c_disable_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    stop_active_dma_transfers(i2c_handle);
    i2c_base_addr->CMD = I2C_CMD_ABORT;
    if (pending_irq & I2C_IF_ARBLOST) {
      i2c_handle->event = SL_I2C_EVENT_ARBITRATION_LOST;
    } else if (pending_irq & I2C_IF_BUSERR) {
      i2c_handle->event = SL_I2C_EVENT_BUS_ERROR;
    }
  }
}

/***************************************************************************//**
 * Dispatch interrupt handler for the I2C Follower
 ******************************************************************************/
void sli_i2c_follower_dispatch_interrupt(sl_i2c_handle_t *i2c_handle)
{
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  uint32_t pending_irq = sl_hal_i2c_get_enabled_pending_interrupts(i2c_base_addr);

  if (pending_irq & I2C_IF_ADDR) {
    sl_hal_i2c_send_ack(i2c_base_addr);
    sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ADDR | I2C_IF_RXDATAV | I2C_IF_SSTOP);
    sl_hal_i2c_enable_interrupts(i2c_base_addr, (I2C_IEN_NACK | I2C_IEN_SSTOP));

    switch (i2c_handle->state) {
      case SL_I2C_STATE_ADDRESS_MATCH:
        if (i2c_handle->follower_address < 0x7F) {
          i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
          if (i2c_handle->transfer_direction == SL_I2C_READ) {
            i2c_base_addr->CTRL_SET = I2C_CTRL_AUTOACK;
          }
        } else {
          sl_hal_i2c_enable_interrupts(i2c_base_addr, I2C_IEN_RXDATAV);
          i2c_handle->state = SL_I2C_STATE_10BIT_ADDRESS_MATCH;
        }
        break;

      case SL_I2C_STATE_REP_ADDR_MATCH:
        sl_hal_i2c_disable_interrupts(i2c_base_addr, I2C_IEN_ADDR);
        i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
        break;
    }
  } else if (pending_irq & I2C_IF_RXDATAV) {
    sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_RXDATAV);
    sl_hal_i2c_disable_interrupts(i2c_base_addr, I2C_IEN_RXDATAV);

    if (i2c_handle->state == SL_I2C_STATE_10BIT_ADDRESS_MATCH) {
      if (((i2c_handle->follower_address << 1) & 0xFF) == i2c_handle->addr_buffer[1]) {
        sl_hal_i2c_send_ack(i2c_base_addr);
        if (i2c_handle->transfer_direction == SL_I2C_READ) {
          i2c_base_addr->CTRL_SET = I2C_CTRL_AUTOACK;
          i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
        } else {
          i2c_handle->state = SL_I2C_STATE_REP_ADDR_MATCH;
        }
      } else {
        sl_hal_i2c_send_nack(i2c_base_addr);
        i2c_handle->event = SL_I2C_EVENT_INVALID_ADDR;
      }
    } else {
      sl_hal_i2c_flush_buffers(i2c_base_addr);
      sl_hal_i2c_send_nack(i2c_base_addr);
    }
  } else if (pending_irq & I2C_IF_NACK) {
    sl_hal_i2c_disable_interrupts(i2c_base_addr, I2C_IEN_NACK);
  } else if (pending_irq & I2C_IF_SSTOP) {
    sl_hal_i2c_disable_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    stop_active_dma_transfers(i2c_handle);
    i2c_base_addr->CTRL = _I2C_CTRL_RESETVALUE;
    if (i2c_handle->event == SL_I2C_EVENT_IN_PROGRESS) {
      i2c_handle->event = SL_I2C_EVENT_COMPLETED;
    }
  } else if (pending_irq & SL_HAL_I2C_IF_ERRORS) {
    sl_hal_i2c_disable_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IEN_MASK);
    stop_active_dma_transfers(i2c_handle);
    i2c_base_addr->CMD = I2C_CMD_ABORT;
    if (pending_irq & I2C_IF_ARBLOST) {
      i2c_handle->event = SL_I2C_EVENT_ARBITRATION_LOST;
    } else if (pending_irq & I2C_IF_BUSERR) {
      i2c_handle->event = SL_I2C_EVENT_BUS_ERROR;
    }
  }
}

/***************************************************************************//**
 * Leader Mode: Send data from two buffers in a single I2C transaction.
 ******************************************************************************/
sl_status_t sli_i2c_leader_write_no_stop_blocking(sl_i2c_handle_t *i2c_handle,
                                                  uint16_t address,
                                                  const uint8_t *tx_buffer1,
                                                  uint32_t tx_len1,
                                                  const uint8_t *tx_buffer2,
                                                  uint32_t tx_len2,
                                                  uint32_t timeout)
{
  CORE_DECLARE_IRQ_STATE;
  sl_status_t status = SL_STATUS_OK;

  // Validate input parameters
  if (i2c_handle == NULL || tx_buffer1 == NULL || tx_buffer2 == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Only allow leader mode for this API
  if (i2c_handle->operating_mode != SL_I2C_LEADER_MODE) {
    return SL_STATUS_INVALID_MODE;
  }
  // Validate the input and length of data to send and receive
  if (tx_len1 == 0 || tx_len2 == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Only support 7-bit addresses for write-write
  if (address > 0x7F) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_ATOMIC();

  // Initialize transaction parameters
  i2c_handle->follower_address = address;
  i2c_handle->tx_offset = 0;
  i2c_handle->rx_offset = 0;
  i2c_handle->transfer_direction = SL_I2C_WRITE;

  // Initialize the I2C transfer
  i2c_setup_blocking_transfer_interrupts(i2c_handle);

  status = i2c_leader_mode_write_no_stop_blocking_state_machine(i2c_handle, tx_buffer1, tx_len1, tx_buffer2, tx_len2, timeout);

  CORE_EXIT_ATOMIC();

  return status;
}

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Get the time in milli seconds.
 *
 * @return
 *   Current time in milliseconds.
 ******************************************************************************/
static uint32_t get_current_time_ms(void)
{
  return sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
}

/***************************************************************************//**
 * Enables I2C transfer-related interrupts for the specified I2C handle.
 *
 * @details
 *   Flushes I2C buffers, clears all pending interrupts, and enables the necessary
 *   interrupt sources based on the operating mode (Leader or Follower) and transfer
 *   direction. Configures the interrupt mask to include ACK, NACK, error interrupts,
 *   and other relevant interrupts such as STOP, RXDATAV, ADDR, etc.
 *
 * @param[in] i2c_handle Pointer to the I2C handle structure.
 *
 * @return return status.
 ******************************************************************************/
static sl_status_t i2c_setup_blocking_transfer_interrupts(sl_i2c_handle_t *i2c_handle)
{
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  IRQn_Type i2c_irq = (IRQn_Type)0;

  // Disable I2C IQR Handler
  switch (I2C_NUM(i2c_base_addr)) {
#if defined(I2C0)
    case 0: i2c_irq = I2C0_IRQn; break;
#endif
#if defined(I2C1)
    case 1: i2c_irq = I2C1_IRQn; break;
#endif
#if defined(I2C2)
    case 2: i2c_irq = I2C2_IRQn; break;
#endif
#if defined(I2C3)
    case 3: i2c_irq = I2C3_IRQn; break;
#endif
    default:
      return SL_STATUS_INVALID_CONFIGURATION;
  }

  sl_interrupt_manager_disable_irq(i2c_irq);

  // Flush and clear
  sl_hal_i2c_flush_buffers(i2c_base_addr);
  sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IF_MASK);

  // Build interrupt mask
  uint32_t ien_mask = I2C_IEN_ACK | I2C_IEN_NACK | SL_HAL_I2C_IEN_ERRORS;

  if (i2c_handle->operating_mode == SL_I2C_LEADER_MODE) {
    ien_mask |= I2C_IEN_MSTOP;
    if (i2c_handle->transfer_direction != SL_I2C_WRITE) {
      ien_mask |= I2C_IEN_RXDATAV;
    }
  } else { // Follower mode
    ien_mask |= I2C_IEN_SSTOP | I2C_IEN_RXDATAV | I2C_IEN_ADDR;
  }

  // Enable all interrupts at once
  sl_hal_i2c_enable_interrupts(i2c_base_addr, ien_mask);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * State machine for leader mode in blocking I2C transfer.
 *
 * @details
 *   Implements a blocking state machine for I2C leader (master) mode
 *   transfers. It manages the sequence of sending the follower (slave) address,
 *   handling 7-bit and 10-bit addressing, transmitting and/or receiving data,
 *   and processing all relevant I2C protocol events (ACK, NACK, STOP, errors).
 *   The function blocks until the transfer is complete, an error occurs, or the
 *   specified timeout is reached.
 *
 * @param[in] i2c_handle  Pointer to the I2C handle structure.
 * @param[in] tx_buffer   Pointer to the transmit buffer (can be NULL if not transmitting).
 * @param[in] tx_len      Number of bytes to transmit.
 * @param[out] rx_buffer  Pointer to the receive buffer (can be NULL if not receiving).
 * @param[in] rx_len      Number of bytes to receive.
 * @param[in] timeout     Timeout duration for the blocking transfer, in milliseconds (0 = no timeout).
 *
 * @return
 *   SL_STATUS_OK on success, or appropriate error code on failure:
 *   - SL_STATUS_TIMEOUT if the operation timed out.
 *   - SL_STATUS_NOT_FOUND if address NACKed.
 *   - SL_STATUS_ABORT if data NACKed.
 *   - SL_STATUS_TRANSMIT if arbitration lost.
 *   - SL_STATUS_IO if bus error.
 *   - SL_STATUS_FAIL for software fault.
 *
 * @note
 *   Error Handling:
 *   - Arbitration Lost: Occurs if another leader takes control of the bus.
 *   - Bus Error: Occurs if an unexpected bus condition is detected.
 *   - Timeout: Occurs if the transfer does not complete within the specified time.
 ******************************************************************************/
static sl_status_t i2c_leader_mode_blocking_state_machine(sl_i2c_handle_t *i2c_handle,
                                                          const uint8_t *tx_buffer,
                                                          uint32_t tx_len,
                                                          uint8_t *rx_buffer,
                                                          uint32_t rx_len,
                                                          uint32_t timeout)
{
  uint32_t pending_irq;
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);

  i2c_handle->state = SL_I2C_STATE_SEND_START_AND_ADDR;

  bool is_10bit_addr = (i2c_handle->follower_address > 0x7F);
  uint16_t actual_follower_addr = (i2c_handle->follower_address << 1);
  uint8_t first_addr_byte, second_addr_byte;
  if (is_10bit_addr) {
    first_addr_byte = (((actual_follower_addr >> 8) & 0x06) | SL_I2C_FIRST_BYTE_10BIT_ADDR_MASK);
    second_addr_byte = (actual_follower_addr & 0xFF);
  } else {
    first_addr_byte = (actual_follower_addr & SL_I2C_7BIT_FOLLOWER_ADDRESS_MASK);
    if (i2c_handle->transfer_direction == SL_I2C_READ) {
      first_addr_byte |= 1;
    }
  }

  uint32_t start_time = get_current_time_ms();

  while (true) {
    if (timeout > 0 && (get_current_time_ms() - start_time >= timeout)) {
      i2c_base_addr->CMD = I2C_CMD_ABORT;
      i2c_handle->event = SL_I2C_EVENT_TIMEOUT;
      return SL_STATUS_TIMEOUT;
    }

    pending_irq = sl_hal_i2c_get_enabled_pending_interrupts(i2c_base_addr);

    // Handle errors first - fastest path for error conditions
    if (pending_irq & SL_HAL_I2C_IF_ERRORS) {
      sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IF_MASK);
      i2c_base_addr->CMD = I2C_CMD_ABORT;
      i2c_handle->state = SL_I2C_STATE_ERROR;

      if (pending_irq & I2C_IF_ARBLOST) {
        i2c_handle->event = SL_I2C_EVENT_ARBITRATION_LOST;
        return SL_STATUS_TRANSMIT;
      } else {
        i2c_handle->event = SL_I2C_EVENT_BUS_ERROR;
        return SL_STATUS_IO;
      }
    }

    switch (i2c_handle->state) {
      case SL_I2C_STATE_SEND_START_AND_ADDR:
        sl_hal_i2c_start_cmd(i2c_base_addr);
        sl_hal_i2c_tx(i2c_base_addr, first_addr_byte);
        i2c_handle->state = SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK;
        i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
        break;

      case SL_I2C_STATE_SEND_REPEATED_START_AND_ADDR:
        sl_hal_i2c_start_cmd(i2c_base_addr);
        sl_hal_i2c_tx(i2c_base_addr, (first_addr_byte | 1));
        i2c_handle->state = SL_I2C_STATE_REPEATED_ADDR_WAIT_FOR_ACK_OR_NACK;
        break;

      case SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK:
        if (pending_irq & I2C_IF_ACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
          if (is_10bit_addr) {
            sl_hal_i2c_tx(i2c_base_addr, second_addr_byte);
            i2c_handle->state = SL_I2C_STATE_ADDR_2ND_BYTE_10BIT_WAIT_FOR_ACK_OR_NACK;
          } else {
            if (i2c_handle->transfer_direction == SL_I2C_READ) {
              i2c_handle->state = SL_I2C_STATE_RECEIVE_DATA;
            } else {  // WRITE or WRITE-READ
              i2c_handle->state = SL_I2C_STATE_SEND_DATA;
            }
          }
        } else if (pending_irq & I2C_IF_NACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);
          sl_hal_i2c_stop_cmd(i2c_base_addr);
          i2c_handle->event = SL_I2C_EVENT_ADDR_NACK;
          i2c_handle->state = SL_I2C_STATE_SEND_STOP;
        }
        break;

      case SL_I2C_STATE_ADDR_2ND_BYTE_10BIT_WAIT_FOR_ACK_OR_NACK:
        if (pending_irq & I2C_IF_ACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
          if (i2c_handle->transfer_direction == SL_I2C_READ) {
            sl_hal_i2c_start_cmd(i2c_base_addr);
            sl_hal_i2c_tx(i2c_base_addr, (first_addr_byte | 1));
            i2c_handle->state = SL_I2C_STATE_REPEATED_ADDR_WAIT_FOR_ACK_OR_NACK;
          } else {
            i2c_handle->state = SL_I2C_STATE_SEND_DATA;
          }
        } else if (pending_irq & I2C_IF_NACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);
          sl_hal_i2c_stop_cmd(i2c_base_addr);
          i2c_handle->event = SL_I2C_EVENT_ADDR_NACK;
          i2c_handle->state = SL_I2C_STATE_SEND_STOP;
        }
        break;

      case SL_I2C_STATE_REPEATED_ADDR_WAIT_FOR_ACK_OR_NACK:
        if (pending_irq & I2C_IF_ACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
          i2c_handle->state = SL_I2C_STATE_RECEIVE_DATA;
        } else if (pending_irq & I2C_IF_NACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);
          sl_hal_i2c_stop_cmd(i2c_base_addr);
          i2c_handle->event = SL_I2C_EVENT_ADDR_NACK;
          i2c_handle->state = SL_I2C_STATE_SEND_STOP;
        }
        break;

      case SL_I2C_STATE_SEND_DATA:
        if (i2c_handle->tx_offset < tx_len) {
          sl_hal_i2c_tx(i2c_base_addr, tx_buffer[i2c_handle->tx_offset++]);
          i2c_handle->state = SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK;
        } else {
          if (i2c_handle->transfer_direction == SL_I2C_WRITE_READ) {
            i2c_handle->state = SL_I2C_STATE_SEND_REPEATED_START_AND_ADDR;
          } else {
            sl_hal_i2c_stop_cmd(i2c_base_addr);
            i2c_handle->state = SL_I2C_STATE_SEND_STOP;
          }
        }
        break;

      case SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK:
        if (pending_irq & I2C_IF_ACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
          i2c_handle->state = SL_I2C_STATE_SEND_DATA;
        } else if (pending_irq & I2C_IF_NACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);

          sl_hal_i2c_stop_cmd(i2c_base_addr);
          i2c_handle->state = SL_I2C_STATE_SEND_STOP;
          i2c_handle->event = SL_I2C_EVENT_DATA_NACK;
        }
        break;

      case SL_I2C_STATE_RECEIVE_DATA:
        if (pending_irq & I2C_IF_RXDATAV) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_RXDATAV);

          if (i2c_handle->rx_offset < rx_len) {
            rx_buffer[i2c_handle->rx_offset++] = sl_hal_i2c_rx(i2c_base_addr);
          }

          if (i2c_handle->rx_offset == rx_len) {
            sl_hal_i2c_send_nack(i2c_base_addr);
            sl_hal_i2c_stop_cmd(i2c_base_addr);
            i2c_handle->state = SL_I2C_STATE_SEND_STOP;
          } else {
            sl_hal_i2c_send_ack(i2c_base_addr);
          }
        }
        break;

      case SL_I2C_STATE_SEND_STOP:
        if (pending_irq & I2C_IF_MSTOP) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_MSTOP);
          if (i2c_handle->event == SL_I2C_EVENT_ADDR_NACK) {
            return SL_STATUS_NOT_FOUND;
          } else if (i2c_handle->event == SL_I2C_EVENT_DATA_NACK) {
            return SL_STATUS_ABORT;
          } else {
            i2c_handle->event = SL_I2C_EVENT_COMPLETED;
            return SL_STATUS_OK;
          }
        }
        break;

      default:
        i2c_handle->event = SL_I2C_EVENT_SW_FAULT;
        i2c_handle->state = SL_I2C_STATE_ERROR;
        return SL_STATUS_FAIL;
    }
  }
}

/***************************************************************************//**
 * State machine for write-write operation in blocking I2C transfer.
 *
 * @details
 *   Implements a blocking state machine for I2C write-write operations that
 *   transmits two separate buffers consecutively in a single atomic I2C transaction.
 *   The transaction format is: S+ADDR(W)+DATA1+DATA2+P.
 *   This ensures atomicity and is primarily designed for I2CSPM compatibility.
 *
 * @param[in] i2c_handle  Pointer to the I2C handle structure.
 * @param[in] tx_buffer1  Pointer to the first transmit buffer.
 * @param[in] tx_len1     Number of bytes in the first buffer.
 * @param[in] tx_buffer2  Pointer to the second transmit buffer.
 * @param[in] tx_len2     Number of bytes in the second buffer.
 * @param[in] timeout     Timeout duration in milliseconds (0 = no timeout).
 *
 * @return
 *   SL_STATUS_OK on success, or appropriate error code on failure.
 *
 * @note
 *   This function supports I2CSPM write_write compatibility, enabling seamless
 *   migration from I2CSPM to I2C driver for applications requiring atomic
 *   two-buffer write operations.
 *   @deprecated This function will be removed when I2CSPM is deprecated.
 ******************************************************************************/
static sl_status_t i2c_leader_mode_write_no_stop_blocking_state_machine(sl_i2c_handle_t *i2c_handle,
                                                                        const uint8_t *tx_buffer1,
                                                                        uint32_t tx_len1,
                                                                        const uint8_t *tx_buffer2,
                                                                        uint32_t tx_len2,
                                                                        uint32_t timeout)
{
  uint32_t pending_irq;
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);

  i2c_handle->state = SL_I2C_STATE_SEND_START_AND_ADDR;

  uint16_t actual_follower_addr = (i2c_handle->follower_address << 1);
  uint8_t first_addr_byte;
  first_addr_byte = (actual_follower_addr & SL_I2C_7BIT_FOLLOWER_ADDRESS_MASK);

  uint32_t start_time = get_current_time_ms();

  // Local variables to track current buffer and state
  const uint8_t *current_tx_buffer = tx_buffer1;
  uint32_t current_tx_len = tx_len1;
  bool is_second_buffer = false;

  while (true) {
    if (timeout > 0 && (get_current_time_ms() - start_time >= timeout)) {
      i2c_base_addr->CMD = I2C_CMD_ABORT;
      i2c_handle->event = SL_I2C_EVENT_TIMEOUT;
      return SL_STATUS_TIMEOUT;
    }

    pending_irq = sl_hal_i2c_get_enabled_pending_interrupts(i2c_base_addr);

    // Handle errors first - fastest path for error conditions
    if (pending_irq & SL_HAL_I2C_IF_ERRORS) {
      sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IF_MASK);
      i2c_base_addr->CMD = I2C_CMD_ABORT;
      i2c_handle->state = SL_I2C_STATE_ERROR;

      if (pending_irq & I2C_IF_ARBLOST) {
        i2c_handle->event = SL_I2C_EVENT_ARBITRATION_LOST;
        return SL_STATUS_TRANSMIT;
      } else {
        i2c_handle->event = SL_I2C_EVENT_BUS_ERROR;
        return SL_STATUS_IO;
      }
    }

    switch (i2c_handle->state) {
      case SL_I2C_STATE_SEND_START_AND_ADDR:
        sl_hal_i2c_start_cmd(i2c_base_addr);
        sl_hal_i2c_tx(i2c_base_addr, first_addr_byte);
        i2c_handle->state = SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK;
        i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
        break;

      case SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK:
        if (pending_irq & I2C_IF_ACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
          i2c_handle->state = SL_I2C_STATE_SEND_DATA;
        } else if (pending_irq & I2C_IF_NACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);
          sl_hal_i2c_stop_cmd(i2c_base_addr);
          i2c_handle->event = SL_I2C_EVENT_ADDR_NACK;
          i2c_handle->state = SL_I2C_STATE_SEND_STOP;
        }
        break;

      case SL_I2C_STATE_SEND_DATA:
        if (i2c_handle->tx_offset < current_tx_len) {
          sl_hal_i2c_tx(i2c_base_addr, current_tx_buffer[i2c_handle->tx_offset++]);
          i2c_handle->state = SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK;
        } else {
          // Current buffer complete
          if (!is_second_buffer) {
            // First buffer complete - switch to second buffer without STOP
            current_tx_buffer = tx_buffer2;
            current_tx_len = tx_len2;
            i2c_handle->tx_offset = 0;
            is_second_buffer = true;
            // Stay in SL_I2C_STATE_SEND_DATA to continue with second buffer
          } else {
            // Second buffer complete - send STOP
            sl_hal_i2c_stop_cmd(i2c_base_addr);
            i2c_handle->state = SL_I2C_STATE_SEND_STOP;
          }
        }
        break;

      case SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK:
        if (pending_irq & I2C_IF_ACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
          i2c_handle->state = SL_I2C_STATE_SEND_DATA;
        } else if (pending_irq & I2C_IF_NACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);

          sl_hal_i2c_stop_cmd(i2c_base_addr);
          i2c_handle->state = SL_I2C_STATE_SEND_STOP;
          i2c_handle->event = SL_I2C_EVENT_DATA_NACK;
        }
        break;

      case SL_I2C_STATE_SEND_STOP:
        if (pending_irq & I2C_IF_MSTOP) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_MSTOP);
          if (i2c_handle->event == SL_I2C_EVENT_ADDR_NACK) {
            return SL_STATUS_NOT_FOUND;
          } else if (i2c_handle->event == SL_I2C_EVENT_DATA_NACK) {
            return SL_STATUS_ABORT;
          } else {
            i2c_handle->event = SL_I2C_EVENT_COMPLETED;
            return SL_STATUS_OK;
          }
        }
        break;

      default:
        i2c_handle->event = SL_I2C_EVENT_SW_FAULT;
        i2c_handle->state = SL_I2C_STATE_ERROR;
        return SL_STATUS_FAIL;
    }
  }
}

/***************************************************************************//**
 * State machine for follower mode in blocking I2C transfer.
 *
 * @details
 *   Implements a blocking state machine for I2C follower (slave) mode transfers.
 *   Handles 7-bit and 10-bit addressing, manages the sequence of address matching,
 *   data transmission and reception, and processes all relevant I2C protocol events
 *   (ACK, NACK, STOP, errors). The function blocks until the transfer is complete,
 *   an error occurs, or the specified timeout is reached.
 *
 * @param[in]  i2c_handle  Pointer to the I2C handle structure.
 * @param[in]  tx_buffer   Pointer to the transmit buffer (can be NULL if not transmitting).
 * @param[in]  tx_len      Number of bytes to transmit.
 * @param[out] rx_buffer   Pointer to the receive buffer (can be NULL if not receiving).
 * @param[in]  rx_len      Number of bytes to receive.
 * @param[in]  timeout     Timeout duration for the blocking transfer, in milliseconds (0 = no timeout).
 *
 * @return
 *   SL_STATUS_OK on success, or appropriate error code on failure:
 *   - SL_STATUS_TIMEOUT if the operation timed out.
 *   - SL_STATUS_NOT_FOUND if address NACKed.
 *   - SL_STATUS_FULL if receive buffer is full.
 *   - SL_STATUS_TRANSMIT if arbitration lost.
 *   - SL_STATUS_IO if bus error.
 *   - SL_STATUS_FAIL for software fault.
 *
 * @note
 *   Error Handling:
 *   - Arbitration Lost: Occurs if another leader takes control of the bus.
 *   - Bus Error: Occurs if an unexpected bus condition is detected.
 *   - Timeout: Occurs if the transfer does not complete within the specified time.
 ******************************************************************************/
static sl_status_t i2c_follower_mode_blocking_state_machine(sl_i2c_handle_t *i2c_handle,
                                                            const uint8_t *tx_buffer,
                                                            uint32_t tx_len,
                                                            uint8_t *rx_buffer,
                                                            uint32_t rx_len,
                                                            uint32_t timeout)
{
  uint32_t pending_irq, second_addr_byte = 0;
  uint8_t rx_data;
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  bool is_10bit_addr = (i2c_handle->follower_address > 0x7F);

  i2c_handle->state = SL_I2C_STATE_IDLE;

  if (is_10bit_addr) {
    second_addr_byte = ((i2c_handle->follower_address << 1) & 0xFF);
  }

  uint32_t start_time = get_current_time_ms();
  while (true) {
    if (timeout > 0 && (get_current_time_ms() - start_time >= timeout)) {
      i2c_base_addr->CMD = I2C_CMD_ABORT;
      i2c_handle->event = SL_I2C_EVENT_TIMEOUT;
      return SL_STATUS_TIMEOUT;
    }

    pending_irq = sl_hal_i2c_get_enabled_pending_interrupts(i2c_base_addr);

    // Error handling
    if (pending_irq & SL_HAL_I2C_IF_ERRORS) {
      sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IF_MASK);
      i2c_base_addr->CMD = I2C_CMD_ABORT;
      i2c_handle->state = SL_I2C_STATE_ERROR;

      if (pending_irq & I2C_IF_ARBLOST) {
        i2c_handle->event = SL_I2C_EVENT_ARBITRATION_LOST;
        return SL_STATUS_TRANSMIT;
      } else {
        i2c_handle->event = SL_I2C_EVENT_BUS_ERROR;
        return SL_STATUS_IO;
      }
    }

    switch (i2c_handle->state) {
      case SL_I2C_STATE_IDLE:
        if (pending_irq & I2C_IF_ADDR) {
          rx_data = sl_hal_i2c_rx(i2c_base_addr);
          sl_hal_i2c_clear_interrupts(i2c_base_addr, (I2C_IF_ADDR | I2C_IF_RXDATAV | I2C_IF_SSTOP));
          sl_hal_i2c_send_ack(i2c_base_addr);

          if (is_10bit_addr) {
            i2c_handle->state = SL_I2C_STATE_10BIT_ADDRESS_MATCH;
          } else {
            if (rx_data & 0x01) {
              i2c_handle->state = SL_I2C_STATE_SEND_DATA;
            } else {
              i2c_handle->state = SL_I2C_STATE_RECEIVE_DATA;
            }
          }
          i2c_handle->event = SL_I2C_EVENT_IN_PROGRESS;
        }
        break;

      case SL_I2C_STATE_10BIT_ADDRESS_MATCH:
        if (pending_irq & I2C_IF_RXDATAV) {
          rx_data = sl_hal_i2c_rx(i2c_base_addr);
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_RXDATAV);

          if (second_addr_byte == rx_data) {
            sl_hal_i2c_send_ack(i2c_base_addr);
            if (i2c_handle->transfer_direction == SL_I2C_READ) {
              i2c_handle->state = SL_I2C_STATE_RECEIVE_DATA;
            } else {  // SL_I2C_WRITE
              i2c_handle->state = SL_I2C_STATE_REP_ADDR_MATCH;
            }
          }
        }
        break;

      case SL_I2C_STATE_REP_ADDR_MATCH:
        if (pending_irq & I2C_IF_ADDR) {
          rx_data = sl_hal_i2c_rx(i2c_base_addr);
          sl_hal_i2c_clear_interrupts(i2c_base_addr, (I2C_IF_ADDR | I2C_IF_RXDATAV));
          sl_hal_i2c_send_ack(i2c_base_addr);
          if (rx_data & 0x01) {
            i2c_handle->state = SL_I2C_STATE_SEND_DATA;
          } else {
            sl_hal_i2c_send_nack(i2c_base_addr);
            i2c_handle->event = SL_I2C_EVENT_ADDR_NACK;
            i2c_handle->state = SL_I2C_STATE_WAIT_FOR_STOP;
          }
        }
        break;

      case SL_I2C_STATE_SEND_DATA:
        if (i2c_handle->tx_offset < tx_len) {
          sl_hal_i2c_tx(i2c_base_addr, tx_buffer[i2c_handle->tx_offset++]);
        } else {
          sl_hal_i2c_tx(i2c_base_addr, 0xFF);
        }
        i2c_handle->state = SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK;
        break;

      case SL_I2C_STATE_DATA_WAIT_FOR_ACK_OR_NACK:
        if (pending_irq & I2C_IF_ACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_ACK);
          i2c_handle->state = SL_I2C_STATE_SEND_DATA;
        } else if (pending_irq & I2C_IF_NACK) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_NACK);
          i2c_handle->state = SL_I2C_STATE_WAIT_FOR_STOP;
        }
        break;

      case SL_I2C_STATE_RECEIVE_DATA:
        if (pending_irq & I2C_IF_RXDATAV) {
          rx_data = sl_hal_i2c_rx(i2c_base_addr);
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_RXDATAV);

          if (i2c_handle->rx_offset < rx_len) {
            rx_buffer[i2c_handle->rx_offset++] = rx_data;
            sl_hal_i2c_send_ack(i2c_base_addr);
          } else {
            sl_hal_i2c_send_nack(i2c_base_addr);
            i2c_handle->event = SL_I2C_EVENT_BUFFER_FULL;
            i2c_handle->state = SL_I2C_STATE_WAIT_FOR_STOP;
          }
        } else if (pending_irq & I2C_IF_SSTOP) {
          i2c_handle->state = SL_I2C_STATE_WAIT_FOR_STOP;
        }
        break;

      case SL_I2C_STATE_WAIT_FOR_STOP:
        if (pending_irq & I2C_IF_SSTOP) {
          sl_hal_i2c_clear_interrupts(i2c_base_addr, I2C_IF_SSTOP);
          if (i2c_handle->event == SL_I2C_EVENT_ADDR_NACK) {
            return SL_STATUS_NOT_FOUND;
          } else if (i2c_handle->event == SL_I2C_EVENT_BUFFER_FULL) {
            return SL_STATUS_FULL;
          } else {
            i2c_handle->event = SL_I2C_EVENT_COMPLETED;
            return SL_STATUS_OK;
          }
        }
        break;

      default:
        i2c_handle->event = SL_I2C_EVENT_SW_FAULT;
        i2c_handle->state = SL_I2C_STATE_ERROR;
        return SL_STATUS_FAIL;
    }
  }
}

/***************************************************************************//**
 * Get DMA trigger sources for I2C TX and RX operations.
 *
 * @details
 *   Determines the appropriate DMA peripheral signals for the given I2C instance.
 *   Maps the I2C base address to the corresponding DMA trigger sources and enables
 *   the appropriate I2C interrupt.
 *
 * @param[in]  i2c_base_addr        Pointer to the I2C peripheral base address.
 * @param[out] dma_tx_trigger_source Pointer to store the TX DMA trigger source.
 * @param[out] dma_rx_trigger_source Pointer to store the RX DMA trigger source.
 *
 * @note This function also:
 *       - Clears any pending interrupts for the I2C instance
 *       - Enables the I2C interrupt in the interrupt manager
 *       - Function is used internally by DMA initialization routines
 *
 * @return Return Status code.
 ******************************************************************************/
static sl_status_t i2c_get_dma_trigger_signals(sl_i2c_handle_t *i2c_handle,
                                               DMADRV_PeripheralSignal_t *dma_tx_trigger_source,
                                               DMADRV_PeripheralSignal_t *dma_rx_trigger_source)
{
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  IRQn_Type i2c_irq;

  switch (I2C_NUM(i2c_base_addr)) {
#if defined(LDMAXBAR_CH_REQSEL_SIGSEL_I2C0TXBL) && defined(LDMAXBAR_CH_REQSEL_SIGSEL_I2C0RXDATAV) \
    || defined(LDMAXBAR0_CH_REQSEL_SIGSEL_I2C0TXBL) && defined(LDMAXBAR0_CH_REQSEL_SIGSEL_I2C0RXDATAV)
    case 0:
      *dma_tx_trigger_source = dmadrvPeripheralSignal_I2C0_TXBL;
      *dma_rx_trigger_source = dmadrvPeripheralSignal_I2C0_RXDATAV;
      i2c_irq = I2C0_IRQn;
      break;
#endif

#if defined(LDMAXBAR_CH_REQSEL_SIGSEL_I2C1TXBL) && defined(LDMAXBAR_CH_REQSEL_SIGSEL_I2C1RXDATAV) \
    || defined(LDMAXBAR0_CH_REQSEL_SIGSEL_I2C1TXBL) && defined(LDMAXBAR0_CH_REQSEL_SIGSEL_I2C1RXDATAV)
    case 1:
      *dma_tx_trigger_source = dmadrvPeripheralSignal_I2C1_TXBL;
      *dma_rx_trigger_source = dmadrvPeripheralSignal_I2C1_RXDATAV;
      i2c_irq = I2C1_IRQn;
      break;
#endif

#if defined(LDMAXBAR0_CH_REQSEL_SIGSEL_I2C2TXBL) && defined(LDMAXBAR0_CH_REQSEL_SIGSEL_I2C2RXDATAV)
    case 2:
      *dma_tx_trigger_source = dmadrvPeripheralSignal_I2C2_TXBL;
      *dma_rx_trigger_source = dmadrvPeripheralSignal_I2C2_RXDATAV;
      i2c_irq = I2C2_IRQn;
      break;
#endif

    default:
      return SL_STATUS_INVALID_CONFIGURATION;
  }

  sl_interrupt_manager_clear_irq_pending(i2c_irq);
  sl_interrupt_manager_enable_irq(i2c_irq);
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Safely stops all active DMA transfers associated with the given I2C handle.
 *
 * @details
 *    This static helper function checks if there are any ongoing DMA transfers
 *    (both TX and RX) for the specified I2C handle. If active transfers are found,
 *    it stops them to ensure a clean state for subsequent operations or shutdown.
 *
 * @param[in] i2c_handle Pointer to the I2C handle structure containing DMA channel information.
 ******************************************************************************/
static void stop_active_dma_transfers(sl_i2c_handle_t *i2c_handle)
{
  bool tx_active = false, rx_active = false;

  // Check and stop TX DMA if active
  if (DMADRV_TransferActive(i2c_handle->dma_channel.dma_tx_channel, &tx_active) == ECODE_EMDRV_DMADRV_OK) {
    if (tx_active) {
      DMADRV_StopTransfer(i2c_handle->dma_channel.dma_tx_channel);
    }
  }

  // Check and stop RX DMA if active
  if (DMADRV_TransferActive(i2c_handle->dma_channel.dma_rx_channel, &rx_active) == ECODE_EMDRV_DMADRV_OK) {
    if (rx_active) {
      DMADRV_StopTransfer(i2c_handle->dma_channel.dma_rx_channel);
    }
  }
}

/***************************************************************************//**
 * Setup DMA transfer for I2C leader mode non-blocking operations.
 *
 * @details
 *   Configures DMA channels and descriptors specifically for I2C
 *   leader mode non-blocking transfers. It handles leader-initiated write and
 *   read operations to communicate with follower devices on the I2C bus.
 *
 *   Leader mode operations supported:
 *   - Leader write: Send data to follower device
 *   - Leader read: Request and receive data from follower device
 *   - Both 7-bit and 10-bit follower addressing
 *
 * @param[in] i2c_handle Pointer to the I2C handle structure containing:
 *                       - I2C peripheral instance (must be in leader mode)
 *                       - Transfer direction (SL_I2C_WRITE or SL_I2C_READ)
 *                       - Follower address (7-bit or 10-bit)
 *                       - Buffer information (TX/RX buffers and lengths)
 *                       - DMA channel allocation
 *
 * @return Status code indicating the result of the operation.
 *
 * @note
 *   - This function is ONLY for I2C leader mode operations
 *   - Leader initiates communication by sending follower address
 *   - DMA channel interrupts are disabled; completion handled via I2C interrupts
 *
 * @warning
 *   - Must be called with i2c_handle->operating_mode == SL_I2C_LEADER_MODE
 *   - DMA channels must be pre-allocated before calling this function
 ******************************************************************************/
static sl_status_t i2c_setup_leader_non_blocking_dma_transfer(sl_i2c_handle_t *i2c_handle,
                                                              const uint8_t *tx_buffer,
                                                              uint32_t tx_len,
                                                              uint8_t *rx_buffer,
                                                              uint32_t rx_len)
{
  sl_status_t status;
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  DMADRV_PeripheralSignal_t dma_tx_trigger_source, dma_rx_trigger_source;
  sl_i2c_dma_channel_info_t *dma_channel = &i2c_handle->dma_channel;

  status = i2c_get_dma_trigger_signals(i2c_handle, &dma_tx_trigger_source, &dma_rx_trigger_source);
  if (status != SL_STATUS_OK) {
    return status;
  }

  bool is_10bit_addr = (i2c_handle->follower_address > 0x7F);
  uint16_t follower_address = (i2c_handle->follower_address << 1);
  uint8_t addr_buffer_count = 0;

  if (is_10bit_addr) {
    // 10-bit address
    i2c_handle->addr_buffer[0] = ((follower_address >> 8) & 0x06) | SL_I2C_FIRST_BYTE_10BIT_ADDR_MASK;
    i2c_handle->addr_buffer[1] = (uint8_t)(follower_address & 0xFF);
    addr_buffer_count = 2;
  } else {
    // 7-bit address
    uint8_t addr = follower_address & SL_I2C_7BIT_FOLLOWER_ADDRESS_MASK;
    if (i2c_handle->transfer_direction == SL_I2C_READ) {
      addr |= 1;
    }
    i2c_handle->addr_buffer[0] = addr;
    addr_buffer_count = 1;
  }

  sl_hal_i2c_flush_buffers(i2c_base_addr);
  sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IF_MASK);
  sl_hal_i2c_enable_interrupts(i2c_base_addr, (I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_MSTOP | SL_HAL_I2C_IEN_ERRORS));
  i2c_handle->state = SL_I2C_STATE_ADDR_WAIT_FOR_ACK_OR_NACK;
  i2c_handle->event = SL_I2C_EVENT_IDLE;

#if defined(EMDRV_DMADRV_LDMA)
  LDMA_TransferCfg_t tx_cfg = LDMA_TRANSFER_CFG_PERIPHERAL(dma_tx_trigger_source);
  LDMA_TransferCfg_t rx_cfg = LDMA_TRANSFER_CFG_PERIPHERAL(dma_rx_trigger_source);

  // Series 2: Disable specific channel interrupts only
  LDMA_IntDisable(1 << dma_channel->dma_tx_channel);
  LDMA_IntDisable(1 << dma_channel->dma_rx_channel);

  if (i2c_handle->transfer_direction == SL_I2C_WRITE || i2c_handle->transfer_direction == SL_I2C_WRITE_READ) {
    if (i2c_handle->transfer_direction == SL_I2C_WRITE) {
      i2c_base_addr->CTRL_SET = I2C_CTRL_AUTOSE;
    } else {
      sl_hal_i2c_enable_interrupts(i2c_base_addr, I2C_IEN_TXC);
    }
    i2c_handle->dma_desc.tx_desc[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2P_BYTE((void*)(i2c_handle->addr_buffer), &((i2c_base_addr)->TXDATA), addr_buffer_count, 1);
    i2c_handle->dma_desc.tx_desc[1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE((void*)(tx_buffer), &((i2c_base_addr)->TXDATA), tx_len);
  }
  if (i2c_handle->transfer_direction == SL_I2C_READ || i2c_handle->transfer_direction == SL_I2C_WRITE_READ) {
    if (i2c_handle->transfer_direction == SL_I2C_READ) {
      i2c_handle->dma_desc.tx_desc[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE((void*)(i2c_handle->addr_buffer), &((i2c_base_addr)->TXDATA), addr_buffer_count);
    }
    if (rx_len > 1) {
      i2c_base_addr->CTRL_SET = I2C_CTRL_AUTOACK;
      i2c_handle->dma_desc.rx_desc[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&i2c_base_addr->RXDATA, (void*)(rx_buffer), rx_len - 1, 1);
      i2c_handle->dma_desc.rx_desc[1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_WRITE(I2C_CTRL_AUTOACK, (uint32_t)&i2c_base_addr->CTRL_CLR, 1);
      i2c_handle->dma_desc.rx_desc[2] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&i2c_base_addr->RXDATA, (void*)(rx_buffer + rx_len - 1), 1, 1);
      i2c_handle->dma_desc.rx_desc[3] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_WRITE(I2C_CMD_NACK | I2C_CMD_STOP, (uint32_t)&i2c_base_addr->CMD);
    } else {
      i2c_handle->dma_desc.rx_desc[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&i2c_base_addr->RXDATA, (void*)(rx_buffer), 1, 1);
      i2c_handle->dma_desc.rx_desc[1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_WRITE((I2C_CMD_NACK | I2C_CMD_STOP), (uint32_t)(&i2c_base_addr->CMD));
    }
  }
#elif defined(EMDRV_DMADRV_LDMA_S3)
  sl_hal_ldma_transfer_config_t tx_cfg = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(dma_tx_trigger_source);
  sl_hal_ldma_transfer_config_t rx_cfg = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(dma_rx_trigger_source);

  // Series 3: Disable specific channel interrupts only
  sl_hal_ldma_disable_interrupts(LDMA0, (1 << dma_channel->dma_tx_channel));
  sl_hal_ldma_disable_interrupts(LDMA0, (1 << dma_channel->dma_rx_channel));

  if (i2c_handle->transfer_direction == SL_I2C_WRITE || i2c_handle->transfer_direction == SL_I2C_WRITE_READ) {
    if (i2c_handle->transfer_direction == SL_I2C_WRITE) {
      i2c_base_addr->CTRL_SET = I2C_CTRL_AUTOSE;
    } else {
      sl_hal_i2c_enable_interrupts(i2c_base_addr, I2C_IEN_TXC);
    }
    i2c_handle->dma_desc.tx_desc[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, (void*)(i2c_handle->addr_buffer), &((i2c_base_addr)->TXDATA), addr_buffer_count, 1);
    i2c_handle->dma_desc.tx_desc[1] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, (void*)(tx_buffer), &((i2c_base_addr)->TXDATA), tx_len);
  }
  if (i2c_handle->transfer_direction == SL_I2C_READ || i2c_handle->transfer_direction == SL_I2C_WRITE_READ) {
    if (i2c_handle->transfer_direction == SL_I2C_READ) {
      i2c_handle->dma_desc.tx_desc[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, (void*)(i2c_handle->addr_buffer), &((i2c_base_addr)->TXDATA), addr_buffer_count);
    }

    if (rx_len > 1) {
      i2c_base_addr->CTRL_SET = I2C_CTRL_AUTOACK;
      i2c_handle->dma_desc.rx_desc[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, &i2c_base_addr->RXDATA, (void*)(rx_buffer), rx_len - 1, 1);
      i2c_handle->dma_desc.rx_desc[1] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_WRITE(I2C_CTRL_AUTOACK, (uint32_t)&i2c_base_addr->CTRL_CLR, 1);
      i2c_handle->dma_desc.rx_desc[2] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, &i2c_base_addr->RXDATA, (void*)(rx_buffer + rx_len - 1), 1, 1);
      i2c_handle->dma_desc.rx_desc[3] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_WRITE(I2C_CMD_NACK | I2C_CMD_STOP, (uint32_t)&i2c_base_addr->CMD);
    } else {
      i2c_handle->dma_desc.rx_desc[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, &i2c_base_addr->RXDATA, (void*)(rx_buffer), 1, 1);
      i2c_handle->dma_desc.rx_desc[1] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_WRITE((I2C_CMD_NACK | I2C_CMD_STOP), (uint32_t)(&i2c_base_addr->CMD));
    }
  }
#endif

  DMADRV_LdmaStartTransfer(dma_channel->dma_tx_channel,
                           &tx_cfg,
                           (void*)&(i2c_handle->dma_desc.tx_desc),
                           NULL, NULL);
  if (i2c_handle->transfer_direction == SL_I2C_READ || i2c_handle->transfer_direction == SL_I2C_WRITE_READ) {
    DMADRV_LdmaStartTransfer(dma_channel->dma_rx_channel,
                             &rx_cfg,
                             (void*)&(i2c_handle->dma_desc.rx_desc),
                             NULL, NULL);
  }

  sl_hal_i2c_start_cmd(i2c_base_addr);
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Sets up a non-blocking DMA transfer for the I2C follower device.
 *
 * @details
 *   Configures the I2C peripheral to perform a DMA-based data transfer
 *   in follower (slave) mode without blocking the CPU. Prepares the necessary DMA
 *   channels and I2C settings to enable efficient data movement between the I2C bus
 *   and memory.
 *
 * @param[in] i2c_handle Pointer to the I2C handle structure containing configuration
 *                       and state information for the I2C peripheral.
 *
 * @return
 *   - SL_STATUS_OK if the setup was successful.
 *   - Appropriate error code otherwise.
 *
 * @note
 *   - The caller must ensure that the DMA and I2C peripherals are properly initialized
 *     before calling this function.
 *   - This function does not start the transfer; it only sets up the necessary resources.
 **********************************************************************************/
static sl_status_t i2c_setup_follower_non_blocking_dma_transfer(sl_i2c_handle_t *i2c_handle,
                                                                const uint8_t *tx_buffer,
                                                                uint32_t tx_len,
                                                                uint8_t *rx_buffer,
                                                                uint32_t rx_len)
{
  sl_status_t status;
  I2C_TypeDef *i2c_base_addr = sl_device_peripheral_i2c_get_base_addr(i2c_handle->i2c_peripheral);
  uint8_t *addr_buffer = i2c_handle->addr_buffer, addr_buffer_count = 0;
  bool is_10bit_addr = (i2c_handle->follower_address > 0x7F);

  DMADRV_PeripheralSignal_t dma_tx_trigger_source, dma_rx_trigger_source;
  sl_i2c_dma_channel_info_t *dma_channel = &i2c_handle->dma_channel;
  status = i2c_get_dma_trigger_signals(i2c_handle, &dma_tx_trigger_source, &dma_rx_trigger_source);
  if (status != SL_STATUS_OK) {
    return status;
  }

  sl_hal_i2c_flush_buffers(i2c_base_addr);
  sl_hal_i2c_clear_interrupts(i2c_base_addr, _I2C_IF_MASK);
  sl_hal_i2c_enable_interrupts(i2c_base_addr, (I2C_IEN_ADDR | SL_HAL_I2C_IEN_ERRORS));
  i2c_handle->state = SL_I2C_STATE_ADDRESS_MATCH;
  i2c_handle->event = SL_I2C_EVENT_IDLE;

#if defined(EMDRV_DMADRV_LDMA)
  LDMA_TransferCfg_t tx_cfg = LDMA_TRANSFER_CFG_PERIPHERAL(dma_tx_trigger_source);
  LDMA_TransferCfg_t rx_cfg = LDMA_TRANSFER_CFG_PERIPHERAL(dma_rx_trigger_source);

  // Series 2: Disable specific channel interrupts only
  LDMA_IntDisable(1 << dma_channel->dma_tx_channel);
  LDMA_IntDisable(1 << dma_channel->dma_rx_channel);

  if (i2c_handle->transfer_direction == SL_I2C_READ) {
    addr_buffer_count = is_10bit_addr ? 2 : 1;

    i2c_handle->dma_desc.rx_desc[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&((i2c_base_addr)->RXDATA), (void*)(addr_buffer), addr_buffer_count, 1);
    i2c_handle->dma_desc.rx_desc[1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_BYTE(&((i2c_base_addr)->RXDATA), (void*)rx_buffer, rx_len, 1);
    i2c_handle->dma_desc.rx_desc[2] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_WRITE(I2C_CTRL_AUTOACK, (uint32_t)(&(i2c_base_addr)->CTRL_CLR), 1);
    i2c_handle->dma_desc.rx_desc[3] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_WRITE(I2C_CMD_NACK, (uint32_t)(&(i2c_base_addr)->CMD));
  } else {  // WRITE
    addr_buffer_count = is_10bit_addr ? 3 : 1;

    i2c_handle->dma_desc.rx_desc[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&((i2c_base_addr)->RXDATA), (void*)(addr_buffer), addr_buffer_count);
    i2c_handle->dma_desc.tx_desc[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2P_BYTE((void*)tx_buffer, &((i2c_base_addr)->TXDATA), tx_len, 1);
    i2c_handle->dma_desc.tx_desc[1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2P_BYTE((void*)temp_buffer, &((i2c_base_addr)->TXDATA), 1, 0);
  }
#elif defined(EMDRV_DMADRV_LDMA_S3)
  sl_hal_ldma_transfer_config_t tx_cfg = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(dma_tx_trigger_source);
  sl_hal_ldma_transfer_config_t rx_cfg = SL_HAL_LDMA_TRANSFER_CFG_PERIPHERAL(dma_rx_trigger_source);

  // Series 3: Disable specific channel interrupts only
  sl_hal_ldma_disable_interrupts(LDMA0, (1 << dma_channel->dma_tx_channel));
  sl_hal_ldma_disable_interrupts(LDMA0, (1 << dma_channel->dma_rx_channel));

  if (i2c_handle->transfer_direction == SL_I2C_READ) {
    addr_buffer_count = is_10bit_addr ? 2 : 1;

    i2c_handle->dma_desc.rx_desc[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, &((i2c_base_addr)->RXDATA), (void*)(addr_buffer), addr_buffer_count, 1);
    i2c_handle->dma_desc.rx_desc[1] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, &((i2c_base_addr)->RXDATA), (void*)rx_buffer, rx_len, 1);
    i2c_handle->dma_desc.rx_desc[2] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_WRITE(I2C_CTRL_AUTOACK, (uint32_t)(&(i2c_base_addr)->CTRL_CLR), 1);
    i2c_handle->dma_desc.rx_desc[3] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_WRITE(I2C_CMD_NACK, (uint32_t)(&(i2c_base_addr)->CMD));
  } else {  // WRITE
    addr_buffer_count = is_10bit_addr ? 3 : 1;

    i2c_handle->dma_desc.rx_desc[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_SINGLE_P2M(SL_HAL_LDMA_CTRL_SIZE_BYTE, &((i2c_base_addr)->RXDATA), (void*)(addr_buffer), addr_buffer_count);
    i2c_handle->dma_desc.tx_desc[0] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, (void*)tx_buffer, &((i2c_base_addr)->TXDATA), tx_len, 1);
    i2c_handle->dma_desc.tx_desc[1] = (sl_hal_ldma_descriptor_t)SL_HAL_LDMA_DESCRIPTOR_LINKREL_M2P(SL_HAL_LDMA_CTRL_SIZE_BYTE, (void*)temp_buffer, &((i2c_base_addr)->TXDATA), 1, 0);
  }
#endif

  DMADRV_LdmaStartTransfer(dma_channel->dma_rx_channel,
                           &rx_cfg,
                           (void*)&(i2c_handle->dma_desc.rx_desc),
                           NULL, NULL);
  if (i2c_handle->transfer_direction == SL_I2C_WRITE) {
    DMADRV_LdmaStartTransfer(dma_channel->dma_tx_channel,
                             &tx_cfg,
                             (void*)&(i2c_handle->dma_desc.tx_desc),
                             NULL, NULL);
  }
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Function called by IRQ handlers to process I2C interrupts.
 *
 * @details Handles interrupts for I2C. It dispatches the interrupt handling
 *          based on the operating mode (leader or follower).
 *
 * @param[in] i2c_peripheral I2C Inst.
 ******************************************************************************/
static void i2c_common_irq_handler(uint8_t i2c_instance)
{
  sl_i2c_handle_t *i2c_handle = i2c_handle_contexts[i2c_instance];

  if (i2c_handle->operating_mode == SL_I2C_LEADER_MODE) {
    sli_i2c_leader_dispatch_interrupt(i2c_handle);
  } else {
    sli_i2c_follower_dispatch_interrupt(i2c_handle);
  }

  if (i2c_handle->event == SL_I2C_EVENT_COMPLETED) {
    if (i2c_handle->transfer_complete_callback) {
      i2c_handle->transfer_complete_callback(i2c_handle, i2c_handle->user_data);
    }
  } else if (i2c_handle->event != SL_I2C_EVENT_IN_PROGRESS && i2c_handle->state == SL_I2C_STATE_WAIT_FOR_STOP) {
    i2c_handle->state = SL_I2C_STATE_DONE;
    if (i2c_handle->event_callback) {
      i2c_handle->event_callback(i2c_handle, i2c_handle->event, i2c_handle->user_data);
    }
  }
}

#if defined(I2C0)
/***************************************************************************//**
 * @brief
 *   Interrupt handler for I2C0 peripheral.
 *
 * @details
 *   Invoked when an interrupt is triggered by the I2C0 peripheral.
 *   It retrieves the I2C0 device instance and passes it to the common I2C IRQ handler.
 ******************************************************************************/
void I2C0_IRQHandler(void)
{
  i2c_common_irq_handler(0);
}
#endif  // I2C0

#if defined(I2C1)
/***************************************************************************//**
 * @brief
 *   Interrupt handler for I2C1 peripheral.
 *
 * @details
 *   Invoked when an interrupt is triggered by the I2C1 peripheral.
 *   It retrieves the I2C1 device instance and passes it to the common I2C IRQ handler.
 ******************************************************************************/
void I2C1_IRQHandler(void)
{
  i2c_common_irq_handler(1);
}
#endif  // I2C1

#if defined(I2C2)
/***************************************************************************//**
 * @brief
 *   Interrupt handler for I2C2 peripheral.
 *
 * @details
 *   Invoked when an interrupt is triggered by the I2C2 peripheral.
 *   It retrieves the I2C2 device instance and passes it to the common I2C IRQ handler.
 ******************************************************************************/
void I2C2_IRQHandler(void)
{
  i2c_common_irq_handler(2);
}
#endif  // I2C2
