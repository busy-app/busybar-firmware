/***************************************************************************/ /**
 * @file  sl_si91x_fw_fallback.c
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

#include "rsi_debug.h"
#include "sl_si91x_fw_fallback.h"
#include "base_types.h"
#include "si91x_device.h"
#include "core_cm4.h"
#ifdef SL_SI91X_FW_FALLBACK
#include "sl_si91x_driver.h"
#include "sli_siwx917_soc.h"
#include "sl_device.h"
#include "sli_siwx917_soc.h"
#include "sl_si91x_types.h"
#endif

#define DISABLE_AB_DEBUG_LOGS 1 // Set to 1 to disable debug logs
#if (DISABLE_AB_DEBUG_LOGS == 1)
#undef DEBUGOUT
#define DEBUGOUT(...) ((void)0) // Disable logs
#endif

#ifdef SL_SI91X_FW_FALLBACK
static uint32_t ota_image_address = 0; ///<        This variable holds the address of the OTA image.
static uint32_t ota_image_type    = 0; ///<        This variable holds the type of the OTA image.
static sl_status_t sli_si91x_erase_slot_info(uint32_t flash_offset);
static sl_status_t sli_si91x_calculate_firmware_slot_crc(sl_si91x_fw_ab_slot_management_t *slot_info);
static void sli_si91x_update_crc(sl_si91x_fw_ab_slot_management_t *slot_info);
static uint8_t sl_m4_upadter_ota_flag = 0;
#endif

#define SL_SI91X_SELECT_DEFAULT_NWP_FW_IMAGE 0x35 // Command value for selecting default NWP firmware image

#define HOST_INTERFACE_OUT_REGISTER_FW_FALLBACK (*(volatile uint32_t *)(HOST_INTF_REG_OUT))

#define SI91X_INTERFACE_IN_REGISTER_FW_FALLBACK (*(volatile uint32_t *)(HOST_INTF_REG_IN))

// MBR magic word for flash initialization

#ifdef SLI_SI91X_MCU_4MB_LITE_IMAGE

#define MBR_MAGIC_WORD_FW_FALLBACK (*(volatile uint32_t *)(0x8160000))

#else

#define MBR_MAGIC_WORD_FW_FALLBACK (*(volatile uint32_t *)(0x81F0000))

#endif

#ifdef SL_SI91X_FW_FALLBACK
/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_ab_upgrade_get_rps_configs(const uint8_t *image_buffer, ota_image_info_t *ota_st)
 *  @pre         None
 *  @brief       Retrieves A/B fallback OTA image configuration from the provided image buffer.
 *               This function checks the magic number in the image buffer to validate the
 *               firmware header. If valid, it populates the ota_image_info_t structure with
 *               the image offset and size from the firmware header.
 *  @param[in]   image_buffer       Pointer to the buffer containing the firmware image.
 *  @param[out]  ota_st             Pointer to the ota_image_info_t structure to be populated.
 *  @return      sl_status_t        SL_STATUS_OK on success, or SL_STATUS_FAIL on failure.
 ******************************************************************************/
sl_status_t sl_si91x_ab_upgrade_get_rps_configs(const uint8_t *image_buffer, ota_image_info_t *ota_st)
{
  if ((image_buffer == NULL) || (ota_st == NULL)) {
    DEBUGOUT("\r\n[get_rps_cfgs] Error: NULL pointer passed!\r\n");
    return SL_STATUS_NULL_POINTER;
  }

  const sl_wifi_firmware_header_t *rps_config_t = (const sl_wifi_firmware_header_t *)image_buffer;
  DEBUGOUT("\r\n[get_rps_cfgs] Magic number: %lx\r\n", rps_config_t->magic_no);

  if (rps_config_t->magic_no == SLI_SI91X_RPS_MAGIC_NO) {

    // Extract image type from bit 0 of control_flags
    ota_st->ota_image_type   = (uint8_t)(rps_config_t->control_flags & BIT(0));
    ota_st->ota_image_offset = rps_config_t->flash_location;

    // Adjust the offset based on ota_image_type
    if ((ota_st->ota_image_type & BIT(0)) == SL_SI91X_M4_RPS_BIT) { // Ensuring bit-masking check
      ota_st->ota_image_offset += SLI_SI91X_M4_FLASH_BASE_ADDR;
    } else if ((ota_st->ota_image_type & BIT(0)) == SL_SI91X_NWP_RPS_BIT) {
      ota_st->ota_image_offset += SLI_SI91X_NWP_FLASH_BASE_ADDR;
    } else {
      DEBUGOUT("\r\n[get_rps_cfgs] Error: Unknown image type!\r\n");
      return SL_SI91X_AB_ERR_UNKNOWN_IMG;
    }

    ota_st->ota_image_size = rps_config_t->image_size + SLI_SI91X_RPS_HEADER_SIZE;

    ota_image_address = rps_config_t->flash_location;
    ota_image_type    = ota_st->ota_image_type;
    return SL_STATUS_OK;
  } else {
    DEBUGOUT("\r\n[get_rps_cfgs] Error: Magic number mismatch!\r\n");
    return SL_SI91X_AB_ERR_MAGIC_NUMBER;
  }
}

/***************************************************************************/ /**
 *  @fn          int16_t sl_si91x_select_default_nwp_fw(const uint8_t fw_image_number)
 *  @pre         None
 *  @brief       Selects the default NWP firmware image to load.
 *               This function sends the appropriate command to select which
 *               firmware image (0 for slot A, 1 for slot B) should be loaded on boot.
 *  @param[in]   fw_image_number    Firmware image number to select (0 for slot A, 1 for slot B)
 *  @return      int16_t - Error code
 ******************************************************************************/
int16_t sl_si91x_select_default_nwp_fw(const uint8_t fw_image_number)
{
  uint16_t boot_cmd = 0;
  if (fw_image_number != 0 && fw_image_number != 1) {
    return SL_STATUS_INVALID_PARAMETER; // Error if firmware image number is not 0 or 1
  }

  // Prepare command with firmware image number
  boot_cmd = HOST_INTERACT_REG_VALID
             | SL_SI91X_SELECT_DEFAULT_NWP_FW_IMAGE; // Command to select default NWP firmware image
  boot_cmd &= 0xF0FF;                                // Clear the lower nibble of the second byte
  boot_cmd |= (uint16_t)(fw_image_number << 8);      // Add the firmware image number

  __asm volatile("cpsid i" ::: "memory");

  // Wait for MBR to be ready
  while (MBR_MAGIC_WORD_FW_FALLBACK != 0x5A5A)
    ;

  // Write the command to the interface register
  SI91X_INTERFACE_IN_REGISTER_FW_FALLBACK = (uint32_t)boot_cmd;
  __asm volatile("dsb");
  __asm volatile("isb");

  // Wait for command acknowledgment
  while ((uint16_t)(HOST_INTERFACE_OUT_REGISTER_FW_FALLBACK) != 0xAB35) {
    ;
  }

  // Enable the NVIC interrupts.
  __asm volatile("cpsie i" ::: "memory");

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_flash_write(uint32_t address, const uint8_t *buffer, uint32_t length)
 *  @pre         None
 *  @brief       Write data to flash memory.
 *               This function writes data to the specified flash memory address.
 *               This function will work for A/B firmware only.
 *  @param[in]   address            Flash memory address to write to ( for M4: 0x8xxxxxx and for NWP: 0x4xxxxxx ).
 *  @param[in]   buffer             Pointer to the data buffer.
 *  @param[in]   length             Number of bytes to write.
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_flash_write(uint32_t address, const uint8_t *buffer, uint32_t length)
{
  sl_status_t status                        = SL_STATUS_FAIL;
  sl_si91x_fw_fallback_request_t fw_request = { 0 };
  size_t request_size                       = 0;

  // Validate length (ensure it fits within the buffer limit)
  if ((length > SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE) || (buffer == NULL) || (length == 0)) {
    return SL_STATUS_INVALID_PARAMETER; // Error if length exceeds 1024 bytes
  }

  // Populate request structure
  fw_request.sub_command  = SL_SI91X_AB_FALLBACK_WRITE_CMD;
  fw_request.chunk_type   = SL_SI91X_DATA_PACKET;
  fw_request.data_length  = length;
  fw_request.flash_offset = address;
  memcpy(fw_request.data, buffer, length);

  request_size = offsetof(sl_si91x_fw_fallback_request_t, data) + length;

  // Send firmware update command
  status = sli_si91x_driver_send_command(SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST,
                                         SLI_WIFI_COMMON_CMD,
                                         &fw_request,
                                         request_size,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_NWP_RESPONSE_TIMEOUT),
                                         NULL,
                                         NULL);

  return status;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_fw_fallback_ota_flash_write(const sl_si91x_fw_fallback_config_t *config, const uint8_t *data_buffer)
 *  @pre         None
 *  @brief       Write data to flash memory for OTA firmware updates.
 *               This function writes data to the specified flash memory address using the unified flash write API.
 *               This function will work for A/B firmware only.
 *  @param[in]   config             Pointer to the firmware fallback configuration structure.
 *  @param[in]   data_buffer        Pointer to the data buffer.
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_fw_fallback_ota_flash_write(const sl_si91x_fw_fallback_config_t *config,
                                                 const uint8_t *data_buffer)
{
  sl_status_t status                        = SL_STATUS_FAIL;
  sl_si91x_fw_fallback_request_t fw_request = { 0 };
  size_t request_size                       = 0;

  // Validate input parameters
  if (config == NULL || data_buffer == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Validate data length (ensure it fits within the buffer limit)
  if (config->ota_image_data_length > SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE || config->ota_image_data_length == 0) {
    return SL_STATUS_INVALID_PARAMETER; // Error if length exceeds 1024 bytes
  }

  // Fill the firmware fallback request structure with all necessary information
  fw_request.sub_command    = SL_SI91X_AB_FALLBACK_WRITE_CMD;
  fw_request.chunk_type     = config->ota_image_chunk_type;
  fw_request.data_length    = config->ota_image_data_length;
  fw_request.flash_offset   = config->ota_image_flash_offset;
  fw_request.m4_updater_ota = config->m4_updater_ota_flag;

  sl_m4_upadter_ota_flag = config->m4_updater_ota_flag;

  // Copy data to the structure
  memcpy(fw_request.data, data_buffer, config->ota_image_data_length);

  request_size = offsetof(sl_si91x_fw_fallback_request_t, data) + config->ota_image_data_length;

  // Send firmware update command
  status = sli_si91x_driver_send_command(SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST,
                                         SLI_WIFI_COMMON_CMD,
                                         &fw_request,
                                         request_size,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_NWP_RESPONSE_TIMEOUT),
                                         NULL,
                                         NULL);

  return status;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_verify_image(uint32_t flash_address)
 *  @pre         None
 *  @brief       Checks the integrity of the firmware for fallback.
 *               This function sends a command to check the integrity of the firmware
 *               at the specified flash memory address. It populates the request structure
 *               with the necessary parameters and sends the command to the firmware.
 *               This function will work for A/B firmware only.
 *  @param[in]   address            Flash address to check integrity( for M4: 0x8xxxxxx and for NWP: 0x4xxxxxx ).
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_verify_image(uint32_t flash_address)
{
  sl_status_t status                        = SL_STATUS_FAIL;
  sl_si91x_fw_fallback_request_t fw_request = { 0 };

  if (flash_address == 0) {
    DEBUGOUT("\r\n [verify_image] Error: Invalid flash address (0x00000000)\r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Populate request structure
  fw_request.chunk_type   = SL_SI91X_DATA_PACKET;
  fw_request.sub_command  = SL_SI91X_AB_FALLBACK_INTEGRITY_CHECK_CMD;
  fw_request.flash_offset = flash_address;
  if (sl_m4_upadter_ota_flag == SL_SI91X_M4_UPDATER_OTA_FLAG) {
    fw_request.m4_updater_ota = sl_m4_upadter_ota_flag;
  }
  DEBUGOUT("\r\n ota_flag=%X \r\n", fw_request.m4_updater_ota);
  size_t request_size = offsetof(sl_si91x_fw_fallback_request_t, data);

  // Send firmware update command
  status = sli_si91x_driver_send_command(SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST,
                                         SLI_WIFI_COMMON_CMD,
                                         &fw_request,
                                         request_size,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_NWP_RESPONSE_TIMEOUT),
                                         NULL,
                                         NULL);
  return status;
}
/***************************************************************************/ /**
 *  @fn          static sl_status_t sli_si91x_validate_ab_info(uint32_t address,
 *                                                               sl_si91x_fw_ab_slot_management_t *slot_info)
 *  @pre         None
 *  @brief       Validates the slot information and checks if the given address
 *               falls within any active slot, preventing erase operations.
 *  @param[in]   address            The flash memory address to be checked.
 *  @param[out]  slot_info          Pointer to the structure holding slot details.
 *  @return      sl_status_t        SL_STATUS_OK if the address is safe for erase,
 *                                  SL_SI91X_AB_ERR_ERASE_ACTIVE_SLOT if the address
 *                                  overlaps with an active slot, or an error code otherwise.
 ******************************************************************************/
static sl_status_t sli_si91x_validate_ab_info(uint32_t address, sl_si91x_fw_ab_slot_management_t *slot_info)
{

  // **Validate Inputs: NULL Pointer & Zero Address**
  if (slot_info == NULL || address == 0) {
    DEBUGOUT("\r\n[validate_ab_info] Error: %s. Aborting operation.\r\n",
             (slot_info == NULL) ? "Slot info pointer is NULL" : "Invalid address (0x00000000)");
    return (slot_info == NULL) ? SL_STATUS_NULL_POINTER : SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t status = sl_si91x_ab_get_slot_info(slot_info);

  // Check if retrieving slot information was successful
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n[validate_ab_info] Error: Failed to retrieve slot information. Status: %lX\r\n", status);

    // If the error is due to invalid slot info (0xFFFFFFFF), allow the erase to proceed
    if (status == SL_SI91X_AB_ERR_INVALID_SLOT_INFO) {
      DEBUGOUT(
        "\r\n[validate_ab_info] Warning: Slot info is invalid (0xFFFFFFFF). Proceeding with the operation...\r\n");
      return SL_STATUS_OK;
    }
    DEBUGOUT("\r\n [validate_ab_info] Error: Slot info is invalid \n");
    // Otherwise, return the error as erase cannot proceed
    return status;
  }

  DEBUGOUT("\r\n[validate_ab_info] Validating slot info. Address: %lX, Active M4 Slot: %u\r\n",
           address,
           slot_info->m4_slot_info.current_active_M4_slot);

  // **Check if the address falls within an active M4 Slot A range**
  if (slot_info->m4_slot_info.current_active_M4_slot == SLOT_A
      && address >= slot_info->m4_slot_info.m4_slot_A.slot_image_offset
      && address
           < (slot_info->m4_slot_info.m4_slot_A.slot_image_offset + slot_info->m4_slot_info.m4_slot_A.image_size)) {
    DEBUGOUT("\r\n [validate_ab_info] Error: Erase operation overlaps with active M4 Slot A "
             "(Offset: %lX, Size: %lu bytes). Erase aborted.\n",
             slot_info->m4_slot_info.m4_slot_A.slot_image_offset,
             slot_info->m4_slot_info.m4_slot_A.image_size);
    return SL_SI91X_AB_ERR_ERASE_ACTIVE_SLOT;
  }

  // **Check if the address falls within an active M4 Slot B range**
  if (slot_info->m4_slot_info.current_active_M4_slot == SLOT_B
      && address >= slot_info->m4_slot_info.m4_slot_B.slot_image_offset
      && address
           < (slot_info->m4_slot_info.m4_slot_B.slot_image_offset + slot_info->m4_slot_info.m4_slot_B.image_size)) {
    DEBUGOUT("\r\n [validate_ab_info] Error: Erase operation overlaps with active M4 Slot B "
             "(Offset: %lX, Size: %lu bytes). Erase aborted.\n",
             slot_info->m4_slot_info.m4_slot_B.slot_image_offset,
             slot_info->m4_slot_info.m4_slot_B.image_size);
    return SL_SI91X_AB_ERR_ERASE_ACTIVE_SLOT;
  }

  // If the address does not overlap with any active slot, return OK
  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_flash_erase(uint32_t address, uint32_t length)
 *  @pre         None
 *  @brief       Erase data from flash memory.
 *               This function will Round up to the nearest 4KB boundary and erases data from the specified flash memory address.
 *               This function will work for A/B firmware only.
 *  @param[in]   address            Flash memory address to erase( for M4: 0x8xxxxxx and for NWP: 0x4xxxxxx ).
 *  @param[in]   length             Number of bytes to erase.
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_flash_erase(uint32_t address, uint32_t length)
{
  sl_status_t status                        = SL_STATUS_FAIL;
  sl_si91x_fw_fallback_request_t fw_request = { 0 };
  sl_si91x_fw_ab_slot_management_t slot_info;
  uint32_t erase_size = 0;

  // Validate slot information and check if the address is within an active slot
  status = sli_si91x_validate_ab_info(address, &slot_info);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n [flash_erase] Error: Failed to validate the slot information: %lX\n", status);
    return status;
  }

  erase_size = (length + 4095) & ~4095; // Round up to the nearest 4KB boundary

  // Populate request structure
  fw_request.sub_command  = SL_SI91X_AB_FALLBACK_ERASE_CMD;
  fw_request.data_length  = erase_size;
  fw_request.flash_offset = address;

  size_t request_size = offsetof(sl_si91x_fw_fallback_request_t, data);

  // Send firmware update command
  status = sli_si91x_driver_send_command(SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST,
                                         SLI_WIFI_COMMON_CMD,
                                         &fw_request,
                                         request_size,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_NWP_RESPONSE_TIMEOUT),
                                         NULL,
                                         NULL);
  DEBUGOUT("\r\n [flash_erase] Erase Success %lX\n", status);
  return status;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_ab_upgrade_set_slot_info(uint32_t new_image_offset, uint32_t new_image_size, sl_si91x_ab_image_type_t image_type)
 *  @pre         None
 *  @brief       Updates slot information and switches active slots on OTA updates.
 *               This function updates the slot information and switches the active slots
 *               based on the provided parameters.
 *               This function will work for A/B firmware only.
 *  @param[in]   new_image_offset   New image base address (used only during OTA update).
 *  @param[in]   new_image_size     New image size (used only during OTA update).
 *  @param[in]   image_type         Type of the image (M4 or NWP).
 *  @return      sl_status_t        SL_STATUS_OK if successful, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_ab_upgrade_set_slot_info(uint32_t new_image_offset,
                                              uint32_t new_image_size,
                                              sl_si91x_ab_image_type_t image_type)
{
  sl_status_t status                                   = SL_STATUS_FAIL;
  sl_si91x_fw_ab_slot_management_t fw_slot_info        = { 0 };
  sl_si91x_fw_ab_slot_management_t verify_fw_slot_info = { 0 };

  // Error check for new_image_offset(for M4: 0x8xxxxxx and for NWP: 0x4xxxxxx)
  if (((new_image_offset & 0xF0000000) != (SLI_SI91X_M4_FLASH_BASE_ADDR & 0xF0000000))
      && ((new_image_offset & 0xF0000000) != (SLI_SI91X_NWP_FLASH_BASE_ADDR & 0xF0000000))) {
    DEBUGOUT(
      "\r\n [upgrade_ab_info]Error: Invalid new_image_offset parameter. Status: SL_STATUS_INVALID_PARAMETER\r\n");
    return SL_STATUS_INVALID_PARAMETER; // Invalid offset
  }

  // Read current slot info from flash
  status = sl_si91x_flash_read(SLI_SI91X_SLOT_INFO_FLASH_OFFSET,
                               (uint8_t *)&fw_slot_info,
                               sizeof(sl_si91x_fw_ab_slot_management_t));
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n [upgrade_ab_info] Error: Failed to read slot info from flash. Status: %lX\r\n", status);
    return SL_SI91X_AB_ERR_FLASH_READ;
  }

  bool need_flash_update =
    ((image_type == SL_SI91X_AB_OTA_IMAGE_TYPE_M4) || (image_type == SL_SI91X_AB_OTA_IMAGE_TYPE_NWP));

  if (need_flash_update) {
    // Handle OTA update for M4
    if (image_type == SL_SI91X_AB_OTA_IMAGE_TYPE_M4) {
      DEBUGOUT("\r\nProcessing Slot-Info update for M4 :%lX \r\n", new_image_offset);
      if (fw_slot_info.m4_slot_info.current_active_M4_slot == SLOT_A) {
        fw_slot_info.m4_slot_info.current_active_M4_slot      = SLOT_B;
        fw_slot_info.m4_slot_info.m4_slot_B.slot_image_offset = new_image_offset;
        fw_slot_info.m4_slot_info.m4_slot_B.image_size        = new_image_size;
        fw_slot_info.m4_slot_info.m4_slot_B.slot_id           = SLOT_B;
      } else {
        fw_slot_info.m4_slot_info.current_active_M4_slot      = SLOT_A;
        fw_slot_info.m4_slot_info.m4_slot_A.slot_image_offset = new_image_offset;
        fw_slot_info.m4_slot_info.m4_slot_A.image_size        = new_image_size;
        fw_slot_info.m4_slot_info.m4_slot_A.slot_id           = SLOT_A;
      }
    }

    // Handle OTA update for NWP
    if (image_type == SL_SI91X_AB_OTA_IMAGE_TYPE_NWP) {
      DEBUGOUT("\r\nProcessing Slot-Info update for NWP\r\n");
      if (fw_slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_A) {
        fw_slot_info.nwp_slot_info.current_active_nwp_slot      = SLOT_B;
        fw_slot_info.nwp_slot_info.nwp_slot_B.slot_image_offset = new_image_offset;
        fw_slot_info.nwp_slot_info.nwp_slot_B.image_size        = new_image_size;
        fw_slot_info.nwp_slot_info.nwp_slot_B.slot_id           = SLOT_B;
      } else {
        fw_slot_info.nwp_slot_info.current_active_nwp_slot      = SLOT_A;
        fw_slot_info.nwp_slot_info.nwp_slot_A.slot_image_offset = new_image_offset;
        fw_slot_info.nwp_slot_info.nwp_slot_A.image_size        = new_image_size;
        fw_slot_info.nwp_slot_info.nwp_slot_A.slot_id           = SLOT_A;
      }
    }

    // Update magic word and CRC
    fw_slot_info.slot_magic_word = SLI_SI91X_AB_FW_SLOT_MAGIC_WORD;
    sli_si91x_update_crc(&fw_slot_info);

    // Erase and write backup slot info
    status = sli_si91x_erase_slot_info(SLI_SI91X_BACKUP_SLOT_INFO_FLASH_OFFSET);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Failed to erase backup slot info. Status: %lX\r\n", status);
      return status;
    }

    status = sl_si91x_flash_write(SLI_SI91X_BACKUP_SLOT_INFO_FLASH_OFFSET,
                                  (uint8_t *)&fw_slot_info,
                                  sizeof(sl_si91x_fw_ab_slot_management_t));
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Failed to write backup slot info. Status: %lX\r\n", status);
      return status;
    }

    // Verify backup slot info
    status = sl_si91x_flash_read(SLI_SI91X_BACKUP_SLOT_INFO_FLASH_OFFSET,
                                 (uint8_t *)&verify_fw_slot_info,
                                 sizeof(sl_si91x_fw_ab_slot_management_t));
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Failed to read backup slot info. Status: %lX\r\n", status);
      return SL_SI91X_AB_ERR_FLASH_READ;
    }

    if ((sli_si91x_calculate_firmware_slot_crc(&verify_fw_slot_info) != verify_fw_slot_info.slot_struct_crc)
        || (verify_fw_slot_info.slot_magic_word != SLI_SI91X_AB_FW_SLOT_MAGIC_WORD)) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Backup slot info verification failed. Status: "
               "SL_SI91X_AB_ERR_BACKUP_VERIFY\r\n");
      return SL_SI91X_AB_ERR_BACKUP_VERIFY;
    }

    // Erase and write actual slot info
    status = sli_si91x_erase_slot_info(SLI_SI91X_SLOT_INFO_FLASH_OFFSET);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Failed to erase slot info. Status: %lX\r\n", status);
      return status;
    }

    status = sl_si91x_flash_write(SLI_SI91X_SLOT_INFO_FLASH_OFFSET,
                                  (uint8_t *)&fw_slot_info,
                                  sizeof(sl_si91x_fw_ab_slot_management_t));
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Failed to write slot info. Status: %lX\r\n", status);
      return status;
    }

    // Verify written slot info
    status = sl_si91x_flash_read(SLI_SI91X_SLOT_INFO_FLASH_OFFSET,
                                 (uint8_t *)&verify_fw_slot_info,
                                 sizeof(sl_si91x_fw_ab_slot_management_t));
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Failed to read written slot info. Status: %lX\r\n", status);
      return SL_SI91X_AB_ERR_FLASH_READ;
    }

    if ((sli_si91x_calculate_firmware_slot_crc(&verify_fw_slot_info) != verify_fw_slot_info.slot_struct_crc)
        || (verify_fw_slot_info.slot_magic_word != SLI_SI91X_AB_FW_SLOT_MAGIC_WORD)) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Written slot info verification failed. Status: "
               "SL_SI91X_AB_ERR_FLASH_VERIFY\r\n");
      return SL_SI91X_AB_ERR_FLASH_VERIFY;
    }

    if ((fw_slot_info.m4_slot_info.current_active_M4_slot != verify_fw_slot_info.m4_slot_info.current_active_M4_slot)
        || (fw_slot_info.nwp_slot_info.current_active_nwp_slot
            != verify_fw_slot_info.nwp_slot_info.current_active_nwp_slot)) {
      DEBUGOUT("\r\n [upgrade_ab_info] Error: Active slot mismatch after verification. Status: "
               "SL_SI91X_AB_ERR_FLASH_VERIFY\r\n");
      return SL_SI91X_AB_ERR_FLASH_VERIFY;
    }
  }
  DEBUGOUT("\r\n [upgrade_ab_info] Slot update successful\n");
  return status;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_ab_get_slot_info(sl_si91x_fw_ab_slot_management_t *slot_info_t)
 *  @pre         None
 *  @brief       Retrieves the current slot information.
 *               This function reads the current slot information from flash memory.
 *               This function will work for A/B firmware only.
 *  @param[out]  slot_info_t        Pointer to the sl_si91x_fw_ab_slot_management_t structure to store the slot information.
 *  @return      sl_status_t        SL_STATUS_OK if successful, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_ab_get_slot_info(sl_si91x_fw_ab_slot_management_t *ab_slot_info_t)
{
  if (ab_slot_info_t == NULL) {
    DEBUGOUT("\r\n [get_ab_info]Error: NULL pointer passed for slot_info!\n");
    return SL_STATUS_NULL_POINTER;
  }

  memset(ab_slot_info_t, 0, sizeof(sl_si91x_fw_ab_slot_management_t));

  // Read slot information from primary flash location
  sl_status_t status = sl_si91x_flash_read(SLI_SI91X_SLOT_INFO_FLASH_OFFSET,
                                           (uint8_t *)ab_slot_info_t,
                                           sizeof(sl_si91x_fw_ab_slot_management_t));

  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n [get_ab_info]Error: Failed to read slot info from flash. Status: %lX\r\n", status);
    return SL_SI91X_AB_ERR_FLASH_READ;
  }

  // Check if the magic number is 0xFF (invalid slot information)
  if (ab_slot_info_t->slot_magic_word == 0xFFFFFFFF) {
    DEBUGOUT("\r\n [get_ab_info]Error: Slot info magic number is 0xFFFFFFFF, indicating invalid slot info.\n");
    return SL_SI91X_AB_ERR_INVALID_SLOT_INFO;
  }

  // Verify CRC and Magic Word of the read-back structure
  if ((sli_si91x_calculate_firmware_slot_crc(ab_slot_info_t) != ab_slot_info_t->slot_struct_crc)
      || (ab_slot_info_t->slot_magic_word != SLI_SI91X_AB_FW_SLOT_MAGIC_WORD)) {
    DEBUGOUT("\r\n [get_ab_info]Error: CRC verification failed for primary slot info. Reading from backup...\n");

    // Clear the slot_info_t structure before reading from backup
    memset(ab_slot_info_t, 0, sizeof(sl_si91x_fw_ab_slot_management_t));

    // Read slot information from backup flash location
    status = sl_si91x_flash_read(SLI_SI91X_BACKUP_SLOT_INFO_FLASH_OFFSET,
                                 (uint8_t *)ab_slot_info_t,
                                 sizeof(sl_si91x_fw_ab_slot_management_t));

    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n [get_ab_info]Error: Failed to read backup slot info. Status: %lX\r\n", status);
      return SL_SI91X_AB_ERR_FLASH_READ;
    }

    // Check again if magic number is 0xFFFFFFFF in backup slot
    if (ab_slot_info_t->slot_magic_word == 0xFFFFFFFF) {
      DEBUGOUT("\r\n [get_ab_info]Error: Backup slot info magic number is 0xFFFFFFFF, indicating invalid slot info.\n");
      return SL_SI91X_AB_ERR_INVALID_SLOT_INFO;
    }

    // Verify CRC and Magic Word of the read-back structure from backup
    if ((sli_si91x_calculate_firmware_slot_crc(ab_slot_info_t) != ab_slot_info_t->slot_struct_crc)
        || (ab_slot_info_t->slot_magic_word != SLI_SI91X_AB_FW_SLOT_MAGIC_WORD)) {
      DEBUGOUT("\r\n [get_ab_info]Error: CRC verification failed for backup slot info\n");
      return SL_SI91X_AB_ERR_FLASH_VERIFY;
    }
  }

  DEBUGOUT("\r\n  [get_ab_info] Slot info successfully retrieved. Active M4 Slot: %X %lX, Active NWP Slot: %X \r\n",
           ab_slot_info_t->m4_slot_info.current_active_M4_slot,
           ((ab_slot_info_t->m4_slot_info.current_active_M4_slot == SLOT_A)
              ? ab_slot_info_t->m4_slot_info.m4_slot_A.slot_image_offset
              : ab_slot_info_t->m4_slot_info.m4_slot_B.slot_image_offset),
           ab_slot_info_t->nwp_slot_info.current_active_nwp_slot);

  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_toggle_slot_info(bool toggle_m4_image, bool toggle_nwp_image)
 *  @brief       Toggles the active slot for M4 and NWP firmware.
 *
 *  This function switches the active firmware slot for M4 and/or NWP between
 *  Slot A and Slot B, based on the current active slot.
 *
 *  @param[in]   toggle_m4_image  If true, toggles M4 firmware slot.
 *  @param[in]   toggle_nwp_image If true, toggles NWP firmware slot.
 *
 *  @return      sl_status_t Returns SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_toggle_slot_info(bool toggle_m4_image, bool toggle_nwp_image)
{
  sl_status_t status;
  sl_si91x_fw_ab_slot_management_t slot_info;

  // Get the current slot information
  status = sl_si91x_ab_get_slot_info(&slot_info);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n [toggle_ab_info] Failed to get slot information: %lX\r\n", status);
    return status;
  }

  // Toggle M4 slot if requested
  if (toggle_m4_image) {
    uint32_t new_image_offset = (slot_info.m4_slot_info.current_active_M4_slot == SLOT_A)
                                  ? slot_info.m4_slot_info.m4_slot_B.slot_image_offset
                                  : slot_info.m4_slot_info.m4_slot_A.slot_image_offset;
    uint32_t new_image_size   = (slot_info.m4_slot_info.current_active_M4_slot == SLOT_A)
                                  ? slot_info.m4_slot_info.m4_slot_B.image_size
                                  : slot_info.m4_slot_info.m4_slot_A.image_size;

    DEBUGOUT("\r\n  [toggle_ab_info] Switching M4 slot to %s\r\n",
             (slot_info.m4_slot_info.current_active_M4_slot == SLOT_A) ? "B" : "A");

    status = sl_si91x_ab_upgrade_set_slot_info(new_image_offset, new_image_size, SL_SI91X_AB_OTA_IMAGE_TYPE_M4);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n  [toggle_ab_info] Failed to set M4 slot information: %lX\r\n", status);
      return status;
    }
  }

  // Toggle NWP slot if requested
  if (toggle_nwp_image) {
    uint32_t new_image_offset = (slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_A)
                                  ? slot_info.nwp_slot_info.nwp_slot_B.slot_image_offset
                                  : slot_info.nwp_slot_info.nwp_slot_A.slot_image_offset;
    uint32_t new_image_size   = (slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_A)
                                  ? slot_info.nwp_slot_info.nwp_slot_B.image_size
                                  : slot_info.nwp_slot_info.nwp_slot_A.image_size;

    DEBUGOUT("\r\n  [toggle_ab_info] Switching NWP slot to %s\r\n",
             (slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_A) ? "B" : "A");

    status = sl_si91x_ab_upgrade_set_slot_info(new_image_offset, new_image_size, SL_SI91X_AB_OTA_IMAGE_TYPE_NWP);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n  [toggle_ab_info] Failed to set NWP slot information: %lX\r\n", status);
      return status;
    }
  }

  return SL_STATUS_OK;
}
/***************************************************************************/ /**
 *  @fn          sli_status_t sli_si91x_erase_slot_info(uint32_t flash_offset)
 *  @pre         None
 *  @brief       Erases the fallback slot information.
 *               This function erases the fallback slot information from flash memory.
 *               This function will work for A/B firmware only.
 *  @param[in]   flash_offset       Flash memory offset to erase.
 *  @return      sl_status_t        SL_STATUS_OK if successful, error code otherwise.
 ******************************************************************************/
static sl_status_t sli_si91x_erase_slot_info(uint32_t flash_offset)
{
  sl_status_t status  = SL_STATUS_FAIL;
  uint32_t erase_size = sizeof(sl_si91x_fw_ab_slot_management_t);
  DEBUGOUT("\r\n [erase_slot_info]  erase slot info offset: %lX\r\n", flash_offset);
  status = sl_si91x_flash_erase(flash_offset, erase_size);
  return status;
}
/***************************************************************************/ /**
 *  @fn          void sli_si91x_update_crc(sl_si91x_fw_ab_slot_management_t *slot_info)
 *  @pre         None
 *  @brief       Update CRC.
 *               This function updates the CRC of the given sl_si91x_fw_ab_slot_management_t structure.
 *               This function will work for A/B firmware only.
 *  @param[in]   slot_info          Pointer to the sl_si91x_fw_ab_slot_management_t structure.
 *  @return      None
 ******************************************************************************/
static void sli_si91x_update_crc(sl_si91x_fw_ab_slot_management_t *slot_info)
{
  slot_info->slot_struct_crc = sli_si91x_calculate_firmware_slot_crc(slot_info);
}
/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_burn_nwp_security_version(uint32_t flash_address)
 *  @pre         None
 *  @brief       Burns the NWP security version to the flash.
 *               This function burns the NWP security version to the flash.
 *               This function will work for A/B firmware only.
 *  @param[in]   flash_address        Flash address of the NWP firmware to burn the security version
 *  @return      sl_status_t          SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_burn_nwp_security_version(uint32_t flash_address)
{
  sl_status_t status                        = SL_STATUS_FAIL;
  sl_si91x_fw_fallback_request_t fw_request = { 0 };

  // Populate request structure
  fw_request.chunk_type   = SL_SI91X_DATA_PACKET;
  fw_request.sub_command  = SL_SI91X_AB_FALLBACK_BURN_SECURITY_VERSION;
  fw_request.flash_offset = flash_address;

  size_t request_size = offsetof(sl_si91x_fw_fallback_request_t, data);

  // DEBUGOUT("\r\n integratiy+request_size:%u %u \r\n", sizeof(sl_si91x_fw_fallback_request_t), request_size);

  // Send firmware update command
  status = sli_si91x_driver_send_command(SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST,
                                         SLI_WIFI_COMMON_CMD,
                                         &fw_request,
                                         request_size,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_NWP_RESPONSE_TIMEOUT),
                                         NULL,
                                         NULL);
  return status;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_fallback_load_qspi_keys(uint32_t image_offset)
 *  @pre         None
 *  @brief       Requests to load QSPI keys.
 *               This function requests to load QSPI keys for to enable the inline decryption for M4 slot firmware.
 *               This function will work for A/B firmware only.
 *  @param[in]   image_offset        Flash address to enable the inline decryption for M4 slot firmware
 *  @return      sl_status_t          SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/

sl_status_t sl_si91x_fallback_load_qspi_keys(uint32_t image_offset)
{
  sl_status_t status                        = SL_STATUS_FAIL;
  sl_si91x_fw_fallback_request_t fw_request = { 0 };

  // Populate request structure
  fw_request.chunk_type   = SL_SI91X_DATA_PACKET;
  fw_request.sub_command  = SL_SI91X_AB_FALLBACK_LOAD_QSPI_KEYS;
  fw_request.flash_offset = image_offset;

  size_t request_size = offsetof(sl_si91x_fw_fallback_request_t, data);

  // Send firmware update command
  status = sli_si91x_driver_send_command(SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST,
                                         SLI_WIFI_COMMON_CMD,
                                         &fw_request,
                                         request_size,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(SL_SI91X_NWP_RESPONSE_TIMEOUT),
                                         NULL,
                                         NULL);
  return status;
}

/***************************************************************************/ /**
 *  @fn          void sl_si91x_nwp_soft_reset_from_updater(const uint32_t m4_slot_image_offset)
 *  @pre         None
 *  @brief       Performs a soft reset of the NWP firmware.
 *               This function sends a soft reset command to the NWP firmware for fallback and keep the NWP
 *               firmware in the boot mode.
 *               This function will work for A/B firmware only.
 *  @param[in]   m4_slot_image_offset  Offset of the M4 slot image for fallback
 *  @return      None
 ******************************************************************************/
#define QSPI_AES_SEC_SEG_LS_ADDR_2 0x120000EC                                 // QSPI AES SEC SEGMENT_LS address 2
#define QSPI_AES_SEC_SEG_MS_ADDR_2 0x120000F0                                 // QSPI AES SEC SEGMENT_MS address 2
#define OCTASPI_BUS_CONTROLLER_2   0x120000C4                                 // OCTASPI BUS CONTROLLER 2
void sl_si91x_nwp_soft_reset_from_updater(const uint32_t m4_slot_image_offset)
{
  sl_status_t status                        = SL_STATUS_OK;
  sl_si91x_fw_fallback_request_t fw_request = { 0 };
  volatile uint32_t *addr                   = (volatile uint32_t *)HOST_INTF_REG_OUT;

  // Populate request structure
  fw_request.chunk_type  = SL_SI91X_DATA_PACKET;
  fw_request.sub_command = SL_SI91X_AB_FALLBACK_SOFT_RESET;

  size_t request_size = offsetof(sl_si91x_fw_fallback_request_t, data);

  // Send firmware update command
  status = sli_si91x_driver_send_command(SLI_SI91X_FW_FALLBACK_REQ_FROM_HOST,
                                         SLI_WIFI_COMMON_CMD,
                                         &fw_request,
                                         request_size,
                                         SLI_WIFI_WAIT_FOR_RESPONSE(3000),
                                         NULL,
                                         NULL);
  (void)status;
  // Wait for the memory to match the expected value(SL_SI91X_MEM_CHECK_VALUE) for the NWP firmware
  while ((*addr != SL_SI91X_MEM_CHECK_VALUE)) {
    __NOP();
    // Continue checking until a match is found
  }

  //Clearing the RX_Buffer valid bit
  M4SS_P2P_INTR_CLR_REG = RX_BUFFER_VALID;
  M4SS_P2P_INTR_CLR_REG;

  const sl_wifi_firmware_header_t *m4_rps_configs = (const sl_wifi_firmware_header_t *)m4_slot_image_offset;
  *(volatile uint32_t *)OCTASPI_BUS_CONTROLLER_2 |=
    BIT(13); // Enable bit 13 to enable the next QSPI banks in OCTASPI BUS CONTROLLER 2
  *(volatile uint32_t *)QSPI_AES_SEC_SEG_LS_ADDR_2 =
    (m4_rps_configs->flash_location + SLI_SI91X_RPS_HEADER_SIZE); // Store the M4 app start address in ULP RAM

  *(volatile uint32_t *)QSPI_AES_SEC_SEG_MS_ADDR_2 =
    ((m4_rps_configs->flash_location + SLI_SI91X_RPS_HEADER_SIZE + m4_rps_configs->image_size)
     - 1); // Store the (M4 app start address + Size of m4 application) in ULP RAM
}
#endif
#ifdef SL_SI91X_FW_FALLBACK_UPDATER
/***************************************************************************/ /**
 *  @fn          uint32_t sli_si91x_calculate_crc32(const void *data, size_t length)
 *  @pre         None
 *  @brief       Calculate CRC32.
 *               This function calculates the CRC32 for the given data.
 *  @param[in]   data               Pointer to the data.
 *  @param[in]   length             Length of the data.
 *  @return      uint32_t           Calculated CRC32 value.
 ******************************************************************************/
static uint32_t sli_si91x_calculate_crc32(const void *data, size_t length)
{
  uint32_t crc         = 0xFFFFFFFF;
  const uint8_t *bytes = (const uint8_t *)data;

  for (size_t i = 0; i < length; i++) {
    crc ^= (uint32_t)bytes[i] << 24;

    for (int j = 0; j < 8; j++) {
      if (crc & 0x80000000)
        crc = (crc << 1) ^ CRC32_POLYNOMIAL;
      else
        crc <<= 1;
    }
  }
  return ~crc; // Final XOR for CRC32
}
/***************************************************************************/ /**
 *  @fn          sl_status_t sli_si91x_calculate_firmware_slot_crc(sl_si91x_fw_ab_slot_management_t *slot_info)
 *  @pre         None
 *  @brief       Calculate CRC of sl_si91x_fw_ab_slot_management_t.
 *               This function calculates the CRC of the given sl_si91x_fw_ab_slot_management_t structure.
 *               This function will work for A/B firmware only.
 *  @param[in]   slot_info          Pointer to the sl_si91x_fw_ab_slot_management_t structure.
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
static sl_status_t sli_si91x_calculate_firmware_slot_crc(sl_si91x_fw_ab_slot_management_t *slot_info)
{
  return sli_si91x_calculate_crc32(slot_info, offsetof(sl_si91x_fw_ab_slot_management_t, slot_struct_crc));
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_flash_read(uint32_t address, uint8_t *buffer, uint32_t length)
 *  @pre         None
 *  @brief       Read data from flash memory.
 *               This function reads data from the specified flash memory address.
 *               This function will work for A/B firmware only.
 *  @param[in]   address            Flash memory address to read from M4.
 *  @param[in]   buffer             Pointer to the buffer to store read data.
 *  @param[in]   length             Number of bytes to read.
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 ******************************************************************************/
sl_status_t sl_si91x_flash_read(uint32_t address, uint8_t *buffer, uint32_t length)
{
  if (buffer == NULL || length == 0) {
    DEBUGOUT("\r\nError: Invalid parameters passed to sl_si91x_flash_read. Address: %lX, Length: %lu\r\n",
             address,
             length);
    return SL_STATUS_INVALID_PARAMETER; // Prevent invalid buffer usage
  }
  // Directly copy from the fixed address (assuming it's always readable)
  memcpy(buffer, (const void *)address, length);
  return SL_STATUS_OK;
}

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_get_m4_app_addr(uint32_t *app_addr)
 *  @pre         None
 *  @brief       Retrieves the M4 application address from the active firmware slot.
 *               This function reads firmware slot information from flash memory,
 *               verifies its integrity using a magic word and CRC check, and determines
 *               the application address for the currently active M4 slot.
 *               This function will work for A/B firmware only.
 * 
 *               If a CRC failure is detected in the primary slot information,  
 *               the function attempts to retrieve the slot information from a backup slot.
 *  @param[out]  app_addr           Pointer to store the retrieved application address.
 *  @return      SL_STATUS_OK        If the operation is successful.
 *  @return      SL_SI91X_AB_ERR_FLASH_READ If reading slot information from flash fails.
 *  @return      SL_SI91X_AB_ERR_MAGIC_NUMBER If the magic word does not match the expected value.
 *  @return      SL_SI91X_AB_ERR_CRC_MISMATCH If the CRC verification fails.
 *  @return      SL_STATUS_INVALID_PARAMETER If the active slot is invalid.
 ******************************************************************************/
sl_status_t sl_si91x_get_m4_app_addr(uint32_t *app_addr)
{
  sl_status_t status                              = SL_STATUS_FAIL;
  sl_si91x_fw_ab_slot_management_t fw_slot_info_t = { 0 };

  // Read current slot info from flash
  status = sl_si91x_flash_read(SLI_SI91X_SLOT_INFO_FLASH_OFFSET,
                               (uint8_t *)&fw_slot_info_t,
                               sizeof(sl_si91x_fw_ab_slot_management_t));

  if (status != SL_STATUS_OK) {
    return SL_SI91X_AB_ERR_FLASH_READ; // Flash read error
  }

  // Verify CRC and Magic Word
  if ((fw_slot_info_t.slot_magic_word != SLI_SI91X_AB_FW_SLOT_MAGIC_WORD)
      || (sli_si91x_calculate_firmware_slot_crc(&fw_slot_info_t) != fw_slot_info_t.slot_struct_crc)) {

    DEBUGOUT("\r\nSlot info corrupted, reading from backup...\n");

    memset(&fw_slot_info_t, 0, sizeof(sl_si91x_fw_ab_slot_management_t)); // Clear memory again before backup read
    // Read from backup location
    status = sl_si91x_flash_read(SLI_SI91X_BACKUP_SLOT_INFO_FLASH_OFFSET,
                                 (uint8_t *)&fw_slot_info_t,
                                 sizeof(sl_si91x_fw_ab_slot_management_t));

    if (status != SL_STATUS_OK) {
      return SL_SI91X_AB_ERR_FLASH_READ; // Backup read error
    }

    // Validate backup CRC and Magic Word
    if ((fw_slot_info_t.slot_magic_word != SLI_SI91X_AB_FW_SLOT_MAGIC_WORD)) {
      return SL_SI91X_AB_ERR_MAGIC_NUMBER; // Magic number mismatch
    }
    if (sli_si91x_calculate_firmware_slot_crc(&fw_slot_info_t) != fw_slot_info_t.slot_struct_crc) {
      return SL_SI91X_AB_ERR_CRC_MISMATCH; // CRC verification failed
    }
  }
  // Determine active slot for M4 and fetch the application address
  if (fw_slot_info_t.m4_slot_info.current_active_M4_slot == SLOT_A) {
    *app_addr = fw_slot_info_t.m4_slot_info.m4_slot_A.slot_image_offset;
  } else if (fw_slot_info_t.m4_slot_info.current_active_M4_slot == SLOT_B) {
    *app_addr = fw_slot_info_t.m4_slot_info.m4_slot_B.slot_image_offset;
  } else {
    // Invalid slot; handle error
    return SL_STATUS_INVALID_PARAMETER; // Invalid active slot
  }

  // Convert offset to actual address
  (*app_addr) += SL_SI91X_CHUNK_LENGTH;
  DEBUGOUT("\r\n [m4_app_addr] app_addr:%lX Current_M4_Slot:%X Current Slot Offset:%lX\r\n",
           *app_addr,
           fw_slot_info_t.m4_slot_info.current_active_M4_slot,
           (fw_slot_info_t.m4_slot_info.current_active_M4_slot == SLOT_A)
             ? fw_slot_info_t.m4_slot_info.m4_slot_A.slot_image_offset
             : fw_slot_info_t.m4_slot_info.m4_slot_B.slot_image_offset);

  // This point is unreachable, but we add a return to satisfy the compiler
  return SL_STATUS_OK;
}
/***************************************************************************/ /**
 *  @fn          void sl_si91x_jump_to_m4_application(uint32_t app_addr)
 *  @pre         None
 *  @brief       Jumps to the M4 application.
 *               This function disables interrupts, fetches the stack pointer and reset handler
 *               from the application's vector table, sets the VTOR to point to the new
 *               application vector table, and branches to the reset handler.
 *               This function will work for A/B firmware only.
 *  @param[in]   app_addr           Address of the application to jump to.
 *  @return      None
 ******************************************************************************/
void sl_si91x_jump_to_m4_application(uint32_t app_addr)
{
  volatile uint32_t stack_pointer;
  volatile uint32_t reset_handler;

  // Disable interrupts and prepare to branch
  __asm volatile("cpsid i" ::: "memory"); // Disable interrupts
  __asm volatile("dsb");                  // Data Synchronization Barrier
  __asm volatile("isb");                  // Instruction Synchronization Barrier

  // Fetch stack pointer and reset handler from the application's vector table
  stack_pointer = *(uint32_t *)(app_addr + 0xB8); // Stack pointer
  reset_handler = *(uint32_t *)(app_addr + 0xC0); // Reset handler

  // Set VTOR to point to the new application vector table
  SCB->VTOR = (uint32_t)app_addr;

  //! Call RSI reset handler */
  __asm("ldr r1, %0" ::"m"(reset_handler) : "r1", "memory"); // Loads reset handler in r1 register

  //! load stack pointer
  __asm("ldr sp, %0" ::"m"(stack_pointer) : "memory"); // Loads stack pointer in sp

  __asm(" bx r1 "); // branch to r1
}

#endif
