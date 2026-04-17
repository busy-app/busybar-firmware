/***************************************************************************/ /**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdint.h>
#include <stddef.h>
#include "sl_si91x_status.h"
#include "sl_si91x_types.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_spi_constants.h"
#include "sl_si91x_host_interface.h"
#include "sl_status.h"
#include "sl_additional_status.h"
#include "sl_wifi_constants.h"
#include "sl_constants.h"
#include "sl_rsi_utility.h"

// This macro converts a 32-bit value from host to little-endian byte order
#define htole32(x) (x)

// This macro converts a 16-bit value from host to little-endian byte order
#define htole16(x) (x)

sl_status_t sli_submit_rx_buffer(void);

sl_status_t sli_si91x_bus_set_interrupt_mask(uint32_t mask)
{
  sl_status_t status;
  // Create a buffer to store interrupt configuration data, depending on the system's bit support
#ifdef SLI_BIT_32_SUPPORT
  uint8_t buffer[4] = { 0, 0, 0, 0 };
#else
  uint8_t buffer[2] = { 0, 0 };
#endif

  // Read the current interrupt mask configuration from memory
  status = sl_si91x_bus_read_memory(SLI_INT_MASK_REG_ADDR, sizeof(buffer), (const uint8_t *)buffer);
  SLI_VERIFY_STATUS(status);

  // Modify the interrupt configuration based on the provided mask
  buffer[1] = buffer[1] & 0xFC; // Clear lower 2 bits
  buffer[1] = buffer[1] | mask; // Set the specified interrupt mask

  // Write the updated mask value back to memory
  status = sl_si91x_bus_write_memory(SLI_INT_MASK_REG_ADDR, sizeof(buffer), buffer);
  return status;
}

// Function for clearing an interrupt
sl_status_t sli_si91x_bus_clear_interrupt(uint32_t interrupt_clear_value)
{
#ifdef SLI_BIT_32_SUPPORT
  // Write the provided interrupt clear value to the specified memory address (32-bit version)
  return sl_si91x_bus_write_memory(SLI_INT_CLR_REG_ADDR, 4, &interrupt_clear_value);
#else
  // Write the provided interrupt clear value to the specified memory address (16-bit version)
  return sl_si91x_bus_write_memory(SLI_INT_CLR_REG_ADDR, 2, (uint8_t *)&interrupt_clear_value);
#endif
}

sl_status_t sli_si91x_bus_enable_high_speed(void)
{
  // It writes specific data to a register at address 0x08 with a size of 2 bytes
  return sli_si91x_bus_write_register(0x08, 2, 0x3);
}

sl_status_t sli_si91x_req_wakeup(void)
{
  // Wake device, if needed
  sl_si91x_host_set_sleep_indicator();
  uint32_t timestamp = sl_si91x_host_get_timestamp();
  do {
    if (sl_si91x_host_get_wake_indicator()) {
      sli_si91x_ulp_wakeup_init();
      break;
    }
    if (sl_si91x_host_elapsed_time(timestamp) > 1000) {
      return SL_STATUS_TIMEOUT;
    }
  } while (1);
  return SL_STATUS_OK;
}
sl_status_t sli_submit_rx_buffer(void)
{
  return SL_STATUS_OK;
}

sl_status_t sli_si91x_boot_instruction(uint8_t type, uint16_t *data)
{
  sl_status_t retval            = 0;
  uint16_t local                = 0;
  uint32_t cmd                  = 0;
  uint16_t read_data            = 0;
  volatile int32_t loop_counter = 0;
#ifdef RSI_DEBUG_PRINT
  RSI_DPRINT(RSI_PL3, "\nBootInsn\n");
#endif

  switch (type) {
    case SLI_REG_READ:
      retval = sl_si91x_bus_read_memory(SLI_HOST_INTF_REG_OUT, 2, (const uint8_t *)&read_data);
      *data  = read_data;
      break;

    case SLI_REG_WRITE:
      retval = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)data);
      break;

    case SLI_PING_WRITE:
#ifdef SLAVE_MODE_TRANSFER
      // Bootloader expects 4KB of data.
      retval = sli_si91x_bus_write_slave(SLI_PING_PONG_CHUNK_SIZE, (uint8_t *)data);
#else
      for (uint32_t offset = 0; offset < 4096; offset += 1024) {
        retval = sl_si91x_bus_write_memory(0x51400 + offset, 1024, (uint8_t *)((uint32_t)data + offset));
        VERIFY_STATUS_AND_RETURN(retval);
      }
#endif
      local  = (SLI_PING_AVAIL | SLI_HOST_INTERACT_REG_VALID);
      retval = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&local);
      break;

    case SLI_PONG_WRITE:
#ifdef SLAVE_MODE_TRANSFER
      // Bootloader expects 4KB of data.
      retval = sli_si91x_bus_write_slave(SLI_PING_PONG_CHUNK_SIZE, (uint8_t *)data);
#else
      for (uint32_t offset = 0; offset < 4096; offset += 1024) {
        retval = sl_si91x_bus_write_memory(0x52400 + offset, 1024, (uint8_t *)((uint32_t)data + offset));
        VERIFY_STATUS_AND_RETURN(retval);
      }
#endif

      // Perform the write operation
      local  = (SLI_PONG_AVAIL | SLI_HOST_INTERACT_REG_VALID);
      retval = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&local);
      break;

    case BURN_M4_FW:
    case BURN_NWP_FW:
      cmd    = type | SLI_HOST_INTERACT_REG_VALID;
      retval = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&cmd);
      VERIFY_STATUS_AND_RETURN(retval);

      SLI_SI91X_RESET_LOOP_COUNTER(loop_counter);
      SLI_SI91X_WHILE_LOOP(loop_counter, SLI_SI91X_LOOP_COUNT_UPGRADE_IMAGE)
      {
        retval = sl_si91x_bus_read_memory(SLI_HOST_INTF_REG_OUT, 2, (const uint8_t *)&read_data);
        VERIFY_STATUS_AND_RETURN(retval);
        if (read_data == (uint16_t)(SLI_SEND_RPS_FILE | SLI_HOST_INTERACT_REG_VALID)) {
          break;
        }
      }
      SLI_SI91X_CHECK_LOOP_COUNTER(loop_counter, SLI_SI91X_LOOP_COUNT_UPGRADE_IMAGE);
      break;

    case LOAD_NWP_FW:
      cmd    = LOAD_NWP_FW | SLI_HOST_INTERACT_REG_VALID;
      retval = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&cmd);
      break;
    case LOAD_DEFAULT_NWP_FW_ACTIVE_LOW:
      cmd    = LOAD_DEFAULT_NWP_FW_ACTIVE_LOW | SLI_HOST_INTERACT_REG_VALID;
      retval = sl_si91x_bus_write_memory(SLI_HOST_INTF_REG_IN, 2, (uint8_t *)&cmd);
      break;
    default:
      retval = SL_STATUS_OK;
      break;
  }
  return retval;
}