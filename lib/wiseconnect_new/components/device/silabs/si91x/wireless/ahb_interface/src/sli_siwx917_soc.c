/***************************************************************************/ /**
 * @file  sli_siwx917_soc.c
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
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif /* SL_COMPONENT_CATALOG_PRESENT */
#include "system_si91x.h"
#include "rsi_error.h"
#include "rsi_ccp_common.h"
#include "sl_si91x_constants.h"
#include "rsi_ipmu.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_m4.h"
#include "sl_si91x_status.h"
#include "sli_siwx917_timer.h"
#include "sli_siwx917_soc.h"
#include "sl_constants.h"
#include "rsi_temp_sensor.h"
#include "sl_si91x_host_interface.h"
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#endif

#define RSI_HAL_MAX_WR_BUFF_LEN 4096

#define SI91X_INTERFACE_OUT_REGISTER       (*(uint32_t *)(SLI_HOST_INTF_REG_OUT))
#define SI91X_INTERFACE_IN_REGISTER        (*(uint32_t *)(SLI_HOST_INTF_REG_IN))
#define SI91X_INTERFACE_STATUS_REGISTER    (*(uint32_t *)(SLI_HOST_INTF_STATUS_REG))
#define SI91X_PING_BUFFER_ADDRESS_REGISTER (*(uint32_t *)(SLI_PING_BUFFER_ADDR))
#define SI91X_PONG_BUFFER_ADDRESS_REGISTER (*(uint32_t *)(SLI_PONG_BUFFER_ADDR))

typedef struct {
  uint8_t _[2048];
} sli_si91x_pingpong_buffer_t;

#define SI91X_PING_BUFFER ((sli_si91x_pingpong_buffer_t *)(0x19000))
#define SI91X_PONG_BUFFER ((sli_si91x_pingpong_buffer_t *)(0x1A000))

/**
 * @fn          int16_t rsi_bl_select_option(uint8_t cmd)
 * @brief       Send firmware load request to module or update default configurations.
 * @param[in]   cmd - type of configuration to be saved \n
 *                    BURN_NWP_FW                    - 0x42 \n
 *                    LOAD_NWP_FW                    - 0x31 \n
 *                    LOAD_DEFAULT_NWP_FW_ACTIVE_LOW - 0x71 \n
 * @return      0              - Success \n
 *              Non-Zero Value - Failure \n
 *                               -28 - Firmware Load or Upgrade timeout error \n
 *                               -14 - Valid Firmware not present \n
 *                               -15 - Invalid Option
 *
 */
/// @private
int16_t rsi_bl_select_option(uint8_t cmd)
{
  uint16_t boot_cmd   = 0;
  int16_t retval      = 0;
  uint16_t read_value = 0;
  sli_si91x_timer_t timer_instance;

  SI91X_INTERFACE_OUT_REGISTER = boot_cmd;

  if (cmd == BURN_NWP_FW) {
    boot_cmd = SLI_HOST_INTERACT_REG_VALID_FW | cmd;
  } else {
    boot_cmd = SLI_HOST_INTERACT_REG_VALID | cmd;
  }
  retval = sli_si91x_send_boot_instruction(SLI_REG_WRITE, &boot_cmd);
  if (retval < 0) {
    return retval;
  }

  sli_si91x_timer_init(&timer_instance, 300);

  while ((cmd != LOAD_NWP_FW) && (cmd != LOAD_DEFAULT_NWP_FW_ACTIVE_LOW)) {
    retval = sli_si91x_send_boot_instruction(SLI_REG_READ, &read_value);
    if (retval < 0) {
      return retval;
    }
    if (cmd == BURN_NWP_FW) {
      if (read_value == (SLI_HOST_INTERACT_REG_VALID | SLI_SEND_RPS_FILE)) {
        break;
      }
    }

    else if (read_value == (SLI_HOST_INTERACT_REG_VALID | cmd)) {
      break;
    }
    if (sli_si91x_timer_expired(&timer_instance)) {
      return RSI_ERROR_FW_LOAD_OR_UPGRADE_TIMEOUT;
    }
  }
  if ((cmd == LOAD_NWP_FW) || (cmd == LOAD_DEFAULT_NWP_FW_ACTIVE_LOW)) {
    sli_si91x_timer_init(&timer_instance, 3000);
    do {
      retval = sli_si91x_send_boot_instruction(SLI_REG_READ, &read_value);
      if (retval < 0) {
        return retval;
      }
      if ((read_value & 0xF000) == (SLI_HOST_INTERACT_REG_VALID_FW & 0xF000)) {
        if ((read_value & 0xFF) == SLI_VALID_FIRMWARE_NOT_PRESENT) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL4, "SLI_VALID_FIRMWARE_NOT_PRESENT\n");
#endif
          return RSI_ERROR_VALID_FIRMWARE_NOT_PRESENT;
        }
        if ((read_value & 0xFF) == SLI_INVALID_OPTION) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL4, "INVALID CMD\n");
#endif

          return RSI_ERROR_INVALID_OPTION;
        }
        if ((read_value & 0xFF) == SLI_CHECKSUM_SUCCESS) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL4, "LOAD SUCCESS\n");
#endif
          break;
        }
      }
      if (sli_si91x_timer_expired(&timer_instance)) {
        return RSI_ERROR_FW_LOAD_OR_UPGRADE_TIMEOUT;
      }

    } while (1);
  }
  return retval;
}

/**
 * @fn          int16_t sli_si91x_send_boot_instruction(uint8_t type, uint16_t *data)
 * @brief       Send boot instructions to module.
 * @param[in]   type - type of the insruction to perform \n
 *                     0xD1 - SLI_REG_READ \n
 *                     0xD2 - SLI_REG_WRITE \n
 *                     0xD5 - SLI_PING_WRITE \n
 *                     0xD4 - SLI_PONG_WRITE \n
 *                     0x42 - BURN_NWP_FW \n
 *                     0x31 - LOAD_NWP_FW \n
 *                     0x71 - LOAD_DEFAULT_NWP_FW_ACTIVE_LOW
 * @param[in]   data - pointer to data which is to be read/write \n
 * @return      0              - Success \n
 *              Non-Zero Value - Failure \n
 *                               -28       - Firmware Load or Upgrade timeout error \n
 *                                -2       - Invalid Parameter \n
 *                                -1 or -2 - SPI Failure
 * @note        This is a proprietry API and it is not recommended to be used by the user directly.
 */
/// @private
int16_t sli_si91x_send_boot_instruction(uint8_t type, uint16_t *data)
{
  int16_t retval     = 0;
  uint32_t cmd       = 0;
  uint16_t read_data = 0;
  sli_si91x_timer_t timer_instance;

  switch (type) {
    case SLI_REG_READ:
      *data = (uint16_t)SI91X_INTERFACE_OUT_REGISTER;
      break;

    case SLI_REG_WRITE:
      SI91X_INTERFACE_IN_REGISTER = *data;
      break;

    case BURN_NWP_FW:
      cmd = BURN_NWP_FW | SLI_HOST_INTERACT_REG_VALID;

      SI91X_INTERFACE_IN_REGISTER = cmd;

      sli_si91x_timer_init(&timer_instance, 300);

      do {
        read_data = (uint16_t)SI91X_INTERFACE_OUT_REGISTER;
        if (sli_si91x_timer_expired(&timer_instance)) {
          return RSI_ERROR_FW_LOAD_OR_UPGRADE_TIMEOUT;
        }
      } while (read_data != (SLI_SEND_RPS_FILE | SLI_HOST_INTERACT_REG_VALID));
      break;

    case LOAD_NWP_FW:
      SI91X_INTERFACE_IN_REGISTER = LOAD_NWP_FW | SLI_HOST_INTERACT_REG_VALID;
      break;

    case LOAD_DEFAULT_NWP_FW_ACTIVE_LOW:
      SI91X_INTERFACE_IN_REGISTER = LOAD_DEFAULT_NWP_FW_ACTIVE_LOW | SLI_HOST_INTERACT_REG_VALID;
      break;

    default:
      retval = RSI_ERROR_INVALID_PARAM;
      break;
  }
  return retval;
}

/**
 * @fn          int16 rsi_waitfor_boardready(void)
 * @brief       Waits to receive board ready from WiFi module
 * @param[in]   none
 * @param[out]  none
 * @return      errCode
 *              0   = SUCCESS
 *              < 0 = Failure
 *              -7  = Error in OS operation
 *              -9  = Bootup options last configuration not saved
 *              -10 = Bootup options checksum failed
 *              -11 = Bootloader version mismatch
 *              -12 = Board ready not received
 * @section description
 * This API is used to check board ready from WiFi module.
 */
int16_t rsi_waitfor_boardready(void)
{
  int16_t retval      = 0;
  uint16_t read_value = 0;

  retval = rsi_boot_insn(REG_READ, &read_value);

  if (retval < 0) {
    return retval;
  }
  if (read_value == 0) {
    return RSI_ERROR_IN_OS_OPERATION;
  }
  if ((read_value & 0xFF00) == (HOST_INTERACT_REG_VALID_READ & 0xFF00)) {
    if ((read_value & 0xFF) == SLI_BOOTUP_OPTIONS_LAST_CONFIG_NOT_SAVED) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTUP OPTIOINS LAST CONFIGURATION NOT SAVED\n");
#endif
      return RSI_ERROR_BOOTUP_OPTIONS_NOT_SAVED;
    } else if ((read_value & 0xFF) == SLI_BOOTUP_OPTIONS_CHECKSUM_FAIL) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTUP OPTIONS CHECKSUM FAIL\n");
#endif
      return RSI_ERROR_BOOTUP_OPTIONS_CHECKSUM_FAIL;
    }
#if defined(BOOTLOADER_VERSION_CHECK) && (BOOTLOADER_VERSION_CHECK == 1)
    else if ((read_value & 0xFF) == BOOTLOADER_VERSION) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTLOADER VERSION CORRECT\n");
#endif
    } else {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTLOADER VERSION NOT MATCHING\n");
#endif

      return RSI_ERROR_BOOTLOADER_VERSION_NOT_MATCHING;
    }
#endif

#ifdef RSI_DEBUG_PRINT
    RSI_DPRINT(RSI_PL3, "RECIEVED BOARD READY\n");
#endif
    return RSI_ERROR_NONE;
  }

#ifdef RSI_DEBUG_PRINT
  RSI_DPRINT(RSI_PL3, "WAITING FOR BOARD READY\n");
#endif
  return RSI_ERROR_WAITING_FOR_BOARD_READY;
}
/**
 * @fn          int16_t rsi_select_option(uint8_t cmd, uint8_t fw_image_number)
 * @brief       Sends a command to select an option to load or update configuration.
 * @param[in]   cmd - Type of configuration to be saved or loaded.
 *                    Possible values:
 *                    - LOAD_NWP_FW (0x31)
 *                    - LOAD_DEFAULT_NWP_FW_ACTIVE_LOW (0x71)
 *                    - SL_SI91X_LOAD_NWP_FW_WITH_IMAGE_NUMBER (0x41)
 * @param[in]   fw_image_number - Firmware image number to be loaded (used with SL_SI91X_LOAD_NWP_FW_WITH_IMAGE_NUMBER).
 * @return      int16_t - Error code.
 *                    - < 0 : Command issue failed.
 *                    -   0 : SUCCESS.
 *                    - -28 : Firmware Load or Upgrade timeout error.
 *                    - -14 : Valid Firmware not present.
 *                    - -15 : Invalid Option.
 * @section description
 * This API is used to send a firmware load request to the WiFi module or update default configurations.
 */
int16_t rsi_select_option(uint8_t cmd, uint8_t fw_image_number)
{
  uint16_t boot_cmd             = 0;
  int16_t retval                = 0;
  uint16_t read_value           = 0;
  volatile int32_t loop_counter = 0;

  boot_cmd = HOST_INTERACT_REG_VALID | cmd;
  if (cmd == CHECK_NWP_INTEGRITY) {
    boot_cmd &= 0xF0FF;
    boot_cmd = boot_cmd | (uint16_t)(fw_image_number << 8);
  }

  // If command is 0x41, clear the lower nibble of the second byte and add firmware image number
  if (cmd == SL_SI91X_LOAD_NWP_FW_WITH_IMAGE_NUMBER) {
    boot_cmd &= 0xF0FF;                           // Clear the lower nibble of the second byte
    boot_cmd |= (uint16_t)(fw_image_number << 8); // Add the firmware image number
  }

  retval = rsi_boot_insn(REG_WRITE, &boot_cmd);
  if (retval < 0) {
    return retval;
  }

  if ((cmd != LOAD_NWP_FW) && (cmd != LOAD_DEFAULT_NWP_FW_ACTIVE_LOW) && (cmd != RSI_JUMP_TO_PC)
      && (cmd != SL_SI91X_LOAD_NWP_FW_WITH_IMAGE_NUMBER)) {
    RSI_RESET_LOOP_COUNTER(loop_counter);
    RSI_WHILE_LOOP((uint32_t)loop_counter, RSI_LOOP_COUNT_SELECT_OPTION)
    {
      retval = rsi_boot_insn(REG_READ, &read_value);
      if (retval < 0) {
        return retval;
      }
      if (cmd == CHECK_NWP_INTEGRITY) {
        if ((read_value & 0xFF) == SLI_CHECKSUM_SUCCESS) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL3, "CHECKSUM SUCCESS\n");
#endif
        } else if (read_value == SLI_CHECKSUM_FAILURE) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL3, "CHECKSUM FAIL\n");
#endif
        } else if (read_value == SLI_CHECKSUM_INVALID_ADDRESS) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL3, "Invalid Address \n");
#endif
        }
      }
      if (read_value == (HOST_INTERACT_REG_VALID | cmd)) {
        break;
      }
    }
    RSI_CHECK_LOOP_COUNTER(loop_counter, RSI_LOOP_COUNT_SELECT_OPTION);
  } else if ((cmd == LOAD_NWP_FW) || (cmd == LOAD_DEFAULT_NWP_FW_ACTIVE_LOW) || (cmd == RSI_JUMP_TO_PC)
             || (cmd == SL_SI91X_LOAD_NWP_FW_WITH_IMAGE_NUMBER)) {
    retval = rsi_boot_insn(REG_READ, &read_value);
    if (retval < 0) {
      return retval;
    }
    if ((read_value & 0xFF) == SLI_VALID_FIRMWARE_NOT_PRESENT) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "SLI_VALID_FIRMWARE_NOT_PRESENT\n");
#endif
      return RSI_ERROR_VALID_FIRMWARE_NOT_PRESENT;
    }
    if ((read_value & 0xFF) == SLI_INVALID_OPTION) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "INVALID CMD\n");
#endif
      return RSI_ERROR_COMMAND_NOT_SUPPORTED;
    }
  }
  return retval;
}

/**
 * @fn          int16 rsi_boot_insn(uint8 type, uint16 *data)
 * @brief       Sends boot instructions to WiFi module
 * @param[in]   uint8 type, type of the insruction to perform
 * @param[in]   uint32 *data, pointer to data which is to be read/write
 * @param[out]  none
 * @return      errCode
 *              < 0  = Command issued failure/Invalid command
 *                0  = SUCCESS
 *              > 0  = Read value
 * @section description
 * This API is used to send boot instructions to WiFi module.
 */

int16_t rsi_boot_insn(uint8_t type, uint16_t *data)
{
  int16_t retval                = 0;
  uint16_t read_data            = 0;
  volatile int32_t loop_counter = 0;
#ifdef RSI_DEBUG_PRINT
  RSI_DPRINT(RSI_PL3, "\nBootInsn\n");
#endif

  switch (type) {
    case REG_READ:
      *data = (uint16_t)SI91X_INTERFACE_OUT_REGISTER;
      break;

    case REG_WRITE:
      SI91X_INTERFACE_IN_REGISTER = *data;
      break;

    case PING_WRITE:
      memcpy(SI91X_PING_BUFFER, data, sizeof(sli_si91x_pingpong_buffer_t));
      SI91X_INTERFACE_IN_REGISTER = 0xab49;
      break;

    case PONG_WRITE:
      memcpy(SI91X_PONG_BUFFER, data, sizeof(sli_si91x_pingpong_buffer_t));
      SI91X_INTERFACE_IN_REGISTER = 0xab4f;
      break;

    case BURN_NWP_FW:
      SI91X_INTERFACE_IN_REGISTER = BURN_NWP_FW | HOST_INTERACT_REG_VALID;

      RSI_RESET_LOOP_COUNTER(loop_counter);
      RSI_WHILE_LOOP((uint32_t)loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE)
      {
        read_data = (uint16_t)SI91X_INTERFACE_OUT_REGISTER;
        if (read_data == (SLI_SEND_RPS_FILE | HOST_INTERACT_REG_VALID)) {
          break;
        }
      }
      RSI_CHECK_LOOP_COUNTER(loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE);
      break;

    case LOAD_NWP_FW:
      SI91X_INTERFACE_IN_REGISTER = LOAD_NWP_FW | HOST_INTERACT_REG_VALID;
      break;
    case LOAD_DEFAULT_NWP_FW_ACTIVE_LOW:
      SI91X_INTERFACE_IN_REGISTER = LOAD_DEFAULT_NWP_FW_ACTIVE_LOW | HOST_INTERACT_REG_VALID;
      break;
    case RSI_UPGRADE_BL:
      SI91X_INTERFACE_IN_REGISTER = RSI_UPGRADE_BL | HOST_INTERACT_REG_VALID;
      RSI_RESET_LOOP_COUNTER(loop_counter);
      RSI_WHILE_LOOP((uint32_t)loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE)
      {
        read_data = (uint16_t)SI91X_INTERFACE_OUT_REGISTER;
        if (read_data == (SLI_SEND_RPS_FILE | HOST_INTERACT_REG_VALID)) {
          break;
        }
      }
      RSI_CHECK_LOOP_COUNTER(loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE);
      break;
    default:
      retval = -2;
      break;
  }
  return retval;
}

void unmask_ta_interrupt(uint32_t interrupt_no)
{
  TASS_P2P_INTR_MASK_CLR = interrupt_no;
}

void sli_m4_ta_interrupt_init(void)
{
#ifdef SLI_SI917
  //! Unmask the interrupt
  unmask_ta_interrupt(TX_PKT_TRANSFER_DONE_INTERRUPT | RX_PKT_TRANSFER_DONE_INTERRUPT | TA_WRITING_ON_COMM_FLASH
                      | NWP_DEINIT_IN_COMM_FLASH
#ifdef SLI_SI91X_MCU_FW_UPGRADE_OTA_DUAL_FLASH
                      | M4_IMAGE_UPGRADATION_PENDING_INTERRUPT
#endif
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
                      | SIDE_BAND_CRYPTO_DONE
#endif
  );
#else
  //! Unmask the interrupt
  unmask_ta_interrupt(TX_PKT_TRANSFER_DONE_INTERRUPT | RX_PKT_TRANSFER_DONE_INTERRUPT);
#endif
  P2P_STATUS_REG |= M4_is_active;

  *(volatile uint32_t *)0xE000E108 = 0x00000400;

  //! Set P2P Intr priority
  NVIC_SetPriority(TASS_P2P_IRQn, TASS_P2P_INTR_PRI);

  return;
}

void sli_si91x_ulp_wakeup_init(void)
{
  // for compilation
}

/***************************************************************************/ /**
 *  @fn          int16_t sli_si91x_set_default_nwp_fw(const uint8_t fw_image_number)
 *  @pre         None
 *  @brief       Selects the default NWP firmware image to load.
 *               This function sends the appropriate command to select which
 *               firmware image (0 or 1) should be loaded on boot.
 *  @param[in]   fw_image_number    Firmware image number to select (0 or 1)
 *  @return      errCode
 *               0  = SUCCESS - RSI_ERROR_NONE
 *               -1 = Timeout waiting for acknowledgment
 *               -2 = Invalid firmware image number - RSI_ERROR_INVALID_PARAM
 * @section description
 * This API is used to set a default NWP firmware image during boot up.
 ******************************************************************************/
int16_t sli_si91x_set_default_nwp_fw(const uint8_t fw_image_number)
{
  uint16_t boot_cmd              = 0;
  volatile uint32_t loop_counter = 0;
  uint16_t out_reg_val           = 0;

  if (fw_image_number > 1) {
    return RSI_ERROR_INVALID_PARAM; // Error if firmware image number is not 0 or 1
  }
  // Prepare command with firmware image number
  boot_cmd = HOST_INTERACT_REG_VALID
             | SLI_SELECT_DEFAULT_NWP_FW_IMAGE_NUMBER; // Command to select default NWP firmware image

  boot_cmd &= 0xF0FF;                           // Clear the lower nibble of the second byte
  boot_cmd |= (uint16_t)(fw_image_number << 8); // Add the firmware image number

  // Interrupts are being disabled but this API is expected to be called during bootup
  __asm volatile("cpsid i" ::: "memory");

  // Write the command to the interface register
  SI91X_INTERFACE_IN_REGISTER = (uint32_t)boot_cmd;
  __asm volatile("dsb");
  __asm volatile("isb");

  RSI_RESET_LOOP_COUNTER(loop_counter);
  RSI_WHILE_LOOP((uint32_t)loop_counter, RSI_LOOP_COUNT_SET_IMAGE_WAIT)
  {
    out_reg_val = (*(volatile uint16_t *)(SLI_HOST_INTF_REG_OUT));
    if (out_reg_val == (uint16_t)(SLI_SELECT_DEFAULT_NWP_FW_IMAGE_NUMBER | HOST_INTERACT_REG_VALID)) {
      break;
    }
  }
  // Enable the interrupts.
  __asm volatile("cpsie i" ::: "memory");

  // Return -1 if timedout waiting for response
  RSI_CHECK_LOOP_COUNTER(loop_counter, RSI_LOOP_COUNT_SET_IMAGE_WAIT);

  return RSI_ERROR_NONE;
}