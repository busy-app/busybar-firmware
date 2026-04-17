/***************************************************************************//**
 * @file
 * @brief EM4 GPIO Retention implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "btl_em4_gpio_retention.h"
#include "em_cmu.h"
#include "em_burtc.h"

/***************************************************************************//**
 * Validate and restore the EM4 GPIO retention configuration stored in BURAM.
 *
 * This function reads the BURAM registers, validates the stored configuration
 * by checking the signature and ensuring the data is consistent, and restores
 * the GPIO pin configurations.
 ******************************************************************************/
int32_t btl_em4GpioRetentionValidateAndRestore(void)
{
  // Read the header from first BURAM register (16-bit header)
  uint32_t header_data = BURAM->RET[0].REG;

  // Extract header fields (16-bit header) - header is in the first 2 bytes (least significant bytes)
  uint16_t header_word = (header_data & 0xFFFF);
  uint8_t signature = header_word & 0x0F;           // Bits 0-3 (4-bit signature)
  uint8_t num_pins = (header_word >> 12) & 0x0F;    // Bits 12-15 (4-bit number of pins)

  // Check signature (can be either BURAM signature or counter signature)
  if (signature != BURAM_SIGNATURE_VALID && signature != BURAM_RESET_COUNTER_SIGNATURE) {
    return BOOTLOADER_ERROR_GPIO_RETENTION_INVALID;
  }

  // Check number of pins (must be 1-16)
  if (num_pins == 0 || num_pins > MAX_EM4_RETAINED_PINS) {
    return BOOTLOADER_ERROR_GPIO_RETENTION_INVALID;
  }

  // Calculate how many BURAM registers we need to read
  // Header takes 2 bytes, each pin takes 1 byte
  // Total bytes = 2 + num_pins
  uint8_t total_bytes_needed = 2 + num_pins;
  uint8_t registers_needed = (total_bytes_needed + 3) / 4; // Round up to 32-bit registers

  // Check that we don't exceed BURAM size (32 registers)
  if (registers_needed > 32) {
    return BOOTLOADER_ERROR_GPIO_RETENTION_INVALID;
  }

  // Validate all pin configurations
  for (uint8_t i = 0; i < num_pins; i++) {
    uint32_t reg_index = (i + 2) / 4; // +2 because header uses first 2 bytes
    uint8_t byte_offset = (i + 2) % 4; // Which byte within the register
    uint8_t pin_config;

    // Extract the pin configuration byte
    pin_config = (uint8_t)((BURAM->RET[reg_index].REG >> (byte_offset * 8)) & 0xFF);

    // Extract pin configuration fields (8-bit structure)
    uint8_t port = pin_config & 0x07;              // Bits 0-2 (LSB) - supports up to 8 ports
    uint8_t pin = (pin_config >> 3) & 0x0F;        // Bits 3-6

    // Check port (0-3 for ports A-D)
    if (port > 3) {
      return BOOTLOADER_ERROR_GPIO_RETENTION_INVALID;
    }

    // Check pin number (0-15)
    if (pin > 15) {
      return BOOTLOADER_ERROR_GPIO_RETENTION_INVALID;
    }
  }

  // All validations passed, restore the GPIO configurations
  // Enable GPIO clock
  CMU_ClockEnable(cmuClock_GPIO, true);

  for (uint8_t i = 0; i < num_pins; i++) {
    uint32_t reg_index = (i + 2) / 4;   // +2 because header uses first 2 bytes
    uint8_t byte_offset = (i + 2) % 4;   // Which byte within the register
    uint8_t pin_config;

    // Extract the pin configuration byte
    pin_config = (uint8_t)((BURAM->RET[reg_index].REG >> (byte_offset * 8)) & 0xFF);

    // Extract pin configuration fields (8-bit structure)
    // Bit field layout: dout(7) | pin(3-6) | port(0-2)
    uint8_t dout = (pin_config >> 7) & 0x01;         // Bit 7
    uint8_t pin = (pin_config >> 3) & 0x0F;          // Bits 3-6
    uint8_t port = pin_config & 0x07;                // Bits 0-2 (LSB)

    // Get GPIO port pointer
    GPIO_Port_TypeDef gpio_port;
    switch (port) {
      case 0: gpio_port = gpioPortA; break;
      case 1: gpio_port = gpioPortB; break;
      case 2: gpio_port = gpioPortC; break;
      case 3: gpio_port = gpioPortD; break;
      default: continue;   // Skip invalid port
    }

    // Configure GPIO pin in push-pull mode
    GPIO_PinModeSet(gpio_port, pin, gpioModePushPull, dout);
  }
  // Always unlatch EM4 GPIO retention
  btl_em4GpioRetentionUnlatch();

  return BOOTLOADER_OK; // Success
}

/***************************************************************************//**
 * Unlatch EM4 GPIO retention.
 ******************************************************************************/
void btl_em4GpioRetentionUnlatch(void)
{
  // Unlatch EM4 GPIO retention
  EMU->CMD_SET = EMU_CMD_EM4UNLATCH;
}

/***************************************************************************//**
 * Increment BURAM reset counter.
 *
 * This function increments the reset counter in the BURAM header.
 * The counter is saturated at BURAM_RESET_COUNTER_MAX.
 ******************************************************************************/
void btl_em4GpioRetentionCounterIncrement(void)
{
  // Read current header
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Extract current reset counter (bits 8-11)
  uint8_t current_counter = (header_word >> 8) & 0x0F;

  // Increment counter, but saturate at maximum
  if (current_counter < BURAM_RESET_COUNTER_MAX) {
    current_counter++;
  }

  // Create new header with updated counter (preserve other fields)
  uint16_t new_header = (header_word & 0xF0FF) | (current_counter << 8);

  // Write back to BURAM, preserving upper 16 bits
  BURAM->RET[0].REG = (header_data & 0xFFFF0000) | new_header;
}

/***************************************************************************//**
 * Get current BURAM reset counter value.
 *
 * @return Current reset counter value (0-15).
 ******************************************************************************/
uint8_t btl_em4GpioRetentionCounterGet(void)
{
  // Read header from first BURAM register
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Extract reset counter (bits 8-11)
  uint8_t counter = (header_word >> 8) & 0x0F;

  return counter;
}

/***************************************************************************//**
 * Check if BURAM reset counter is enabled.
 *
 * @return true if reset counter is enabled, false otherwise.
 ******************************************************************************/
bool btl_em4GpioRetentionCounterEnabled(void)
{
  // Read the header from first BURAM register
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Check if signature is valid (indicates counter is initialized)
  uint8_t signature = header_word & 0x0F;
  return (signature == BURAM_RESET_COUNTER_SIGNATURE);
}

/***************************************************************************//**
 * Enable BURAM reset counter.
 *
 * This function initializes the BURAM reset counter by setting up the header
 * with a valid signature and reset counter set to 0.
 ******************************************************************************/
void btl_em4GpioRetentionCounterEnable(void)
{
  // Read current header
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Preserve existing fields and set valid signature
  uint16_t new_header = (header_word & 0xFFF0) | BURAM_RESET_COUNTER_SIGNATURE;

  // Write back to BURAM, preserving upper 16 bits
  BURAM->RET[0].REG = (header_data & 0xFFFF0000) | new_header;
}

/***************************************************************************//**
 * Invalidate BURAM reset counter.
 *
 * This function clears the BURAM reset counter by setting the signature to invalid
 * and clearing the reset counter.
 ******************************************************************************/
void btl_em4GpioRetentionCounterInvalidate(void)
{
  // Read current header
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Clear signature and reset counter (bits 0-3 for signature, bits 8-11 for counter)
  uint16_t new_header = header_word & 0xF0F0; // Clear signature (bits 0-3) and counter (bits 8-11)

  // Write back to BURAM, preserving upper 16 bits
  BURAM->RET[0].REG = (header_data & 0xFFFF0000) | new_header;
}

/***************************************************************************//**
 * Check if BURAM signature is valid and reset reason indicates bootloader entry.
 *
 * This function checks if the BURAM signature is valid and if the reset reason
 * is one of the 4 specific reasons that indicate bootloader entry should occur.
 *
 * @return true if signature is valid and reset reason indicates bootloader entry.
 ******************************************************************************/
bool btl_em4GpioRetentionEnterBootloader(void)
{
  // Read the header from first BURAM register
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Check if signature is valid
  uint8_t signature = header_word & 0x0F;
  if (signature != BURAM_SIGNATURE_VALID) {
    return false;
  }

  // Extract reset reason (bits 4-7)
  uint8_t reset_reason = (header_word >> 4) & 0x0F;

  // Check if reset reason indicates bootloader entry should occur
  switch (reset_reason) {
    case BURAM_RESET_REASON_BOOTLOAD:
    case BURAM_RESET_REASON_BADIMAGE:
    case BURAM_RESET_REASON_BADAPP:
      // Reset reason indicates bootloader entry
      return true;
    default:
      break;
  }

  return false;
}

/***************************************************************************//**
 * Check if device woke up from EM4 sleep.
 *
 * This function checks if the device woke up from EM4 sleep by examining
 * the reset cause register (RMU or EMU depending on device).
 *
 * @return true if device woke up from EM4, false otherwise.
 ******************************************************************************/
bool btl_em4GpioRetentionIsWakeup(void)
{
  return (EMU->RSTCAUSE & EMU_RSTCAUSE_EM4);
}

/***************************************************************************//**
 * Set BURAM reset reason.
 *
 * This function sets the reset reason in the BURAM header.
 *
 * @param reason  The reset reason to set.
 ******************************************************************************/
void btl_em4GpioRetentionSetResetReason(uint8_t reason)
{
  // Read current header
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Extract current signature (bits 0-3)
  uint8_t current_signature = header_word & 0x0F;

  // Update reset reason (bits 4-7) while preserving other fields
  uint16_t new_header = (header_word & 0xFF0F) | ((reason & 0x0F) << 4);

  // Set signature to BURAM_SIGNATURE_VALID if current signature is not counter signature
  if (current_signature != BURAM_RESET_COUNTER_SIGNATURE) {
    new_header = (new_header & 0xFFF0) | BURAM_SIGNATURE_VALID;
  }

  // Write back to BURAM, preserving upper 16 bits
  BURAM->RET[0].REG = (header_data & 0xFFFF0000) | new_header;
}

/***************************************************************************//**
 * Get BURAM reset reason.
 *
 * @return The current reset reason from BURAM header.
 ******************************************************************************/
uint8_t btl_em4GpioRetentionGetResetReason(void)
{
  // Read header from first BURAM register
  uint32_t header_data = BURAM->RET[0].REG;
  uint16_t header_word = (uint16_t)(header_data & 0xFFFF);

  // Extract reset reason (bits 4-7)
  uint8_t reset_reason = (header_word >> 4) & 0x0F;

  return reset_reason;
}
