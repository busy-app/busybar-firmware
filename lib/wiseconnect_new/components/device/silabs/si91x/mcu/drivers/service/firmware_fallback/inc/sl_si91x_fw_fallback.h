/*******************************************************************************
 * @file  sl_si91x_fw_fallback.h
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

#ifndef _SLI_SI91X_FW_FALLBACK__
#define _SLI_SI91X_FW_FALLBACK__
#include "sl_status.h"
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/**
 * @addtogroup FIRMWARE-FALLBACK Firmware Fallback A and B
 * @ingroup SI91X_SERVICE_APIS
 * @{
 * 
 ******************************************************************************/

/******************************** Macros ********************************/

/** @brief Magic number located in RPS Header. */
#define SLI_SI91X_RPS_MAGIC_NO 0x900D900D ///< Magic number in RPS header

/** @brief Firmware header size in bytes. */
#define SLI_SI91X_RPS_HEADER_SIZE sizeof(sl_wifi_firmware_header_t) ///< Firmware RPS header size

/** @brief Flash memory chunk size for erase operations. */
#define SL_SI91X_CHUNK_LENGTH 4096 ///< Flash erase chunk length (4KB)

/** @brief Base address of M4 and NWP flash memory. */
#define SLI_SI91X_M4_FLASH_BASE_ADDR  0x8000000 ///< M4 flash base address
#define SLI_SI91X_NWP_FLASH_BASE_ADDR 0x4000000 ///< NWP flash base address

/** @brief Slot information flash offsets. */
#define SLI_SI91X_SLOT_INFO_FLASH_OFFSET 0x81F1000 ///< Primary slot information offset
#define SLI_SI91X_BACKUP_SLOT_INFO_FLASH_OFFSET \
  (SLI_SI91X_SLOT_INFO_FLASH_OFFSET + SL_SI91X_CHUNK_LENGTH) ///< Backup slot offset

/** @brief Image type identifiers in RPS. */
#define SL_SI91X_M4_RPS_BIT  1 ///< M4 RPS image type
#define SL_SI91X_NWP_RPS_BIT 0 ///< NWP RPS image type

/** @brief OTA image chunk size. */
#define SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE 1024 ///< OTA firmware chunk size

/** @brief Timeout for NWP responses. */
#define SL_SI91X_NWP_RESPONSE_TIMEOUT 30000 ///< NWP response timeout (30s)

/** @brief Timeout for selecting default firmware. */
#define SL_SI91X_DEFAULT_FW_SELECT_CMD_TIMEOUT 10000 ///< Timeout for selecting default firmware (10s)

/** @brief Magic word to verify slot information integrity. */
#define SLI_SI91X_AB_FW_SLOT_MAGIC_WORD 0xA5A5B5B5 ///< Slot information magic word

/** @brief CRC polynomial for slot information. */
#define CRC32_POLYNOMIAL 0x04C11DB7 ///< This macro defines the polynomial used for CRC32 calculation.

/** @brief Memory value to check for memory match. */
#define SL_SI91X_MEM_CHECK_VALUE 0xAB11 ///< Memory value to check for memory match

/** @brief M4 updater OTA flag. */
#define SL_SI91X_M4_UPDATER_OTA_FLAG 0xEF ///< M4 updater OTA flag

// @brief Error codes for SI91x A/B slot and fallback operations.
#define SL_SI91X_AB_ERR_FLASH_READ        ((sl_status_t)0xAB001) ///< A/B fallback: Flash read operation failed
#define SL_SI91X_AB_ERR_FLASH_VERIFY      ((sl_status_t)0xAB002) ///< A/B fallback: Flash verification failure
#define SL_SI91X_AB_ERR_BACKUP_READ       ((sl_status_t)0xAB003) ///< A/B fallback: Backup slot read failure
#define SL_SI91X_AB_ERR_BACKUP_VERIFY     ((sl_status_t)0xAB004) ///< A/B fallback: CRC verification failed in backup slot
#define SL_SI91X_AB_ERR_CRC_MISMATCH      ((sl_status_t)0xAB005) ///< A/B fallback: CRC mismatch detected
#define SL_SI91X_AB_ERR_MAGIC_NUMBER      ((sl_status_t)0xAB006) ///< A/B fallback: Magic number mismatch error
#define SL_SI91X_AB_ERR_UNKNOWN_IMG       ((sl_status_t)0xAB007) ///< A/B fallback: Unknown image type
#define SL_SI91X_AB_ERR_INVALID_SLOT_INFO ((sl_status_t)0xAB008) ///< A/B fallback: Invalid slot info
#define SL_SI91X_AB_ERR_ERASE_ACTIVE_SLOT ((sl_status_t)0xAB009) ///< A/B fallback: Invalid slot info

// This enumeration is used to specify the type of OTA image being processed
// during an A/B firmware upgrade and for slot management.
/** @brief Enum for A/B OTA image types. */
typedef enum {
  SL_SI91X_AB_OTA_IMAGE_TYPE_M4  = 1, ///< Image for M4 processor
  SL_SI91X_AB_OTA_IMAGE_TYPE_NWP = 2, ///< Image for NWP processor
  SL_SI91X_AB_OTA_IMAGE_TYPE_MAX      ///< Maximum image type value
} sl_si91x_ab_image_type_t;

/** @brief Enum for AB slot identifiers */
typedef enum {
  SLOT_A = 1, ///< Primary slot (typically the active firmware slot)
  SLOT_B = 2, ///< Secondary slot (used for updates or fallback)
  SLOT_MAX    ///< Maximum number of slots (used for validation)
} sl_si91x_ab_slot_t;

/** @brief Enum for A/B chunk types(Payload / Header). */
typedef enum {
  SL_SI91X_DATA_PACKET   = 0, ///< Chunk type for Data packet
  SL_SI91X_HEADER_PACKET = 1  ///< Chunk type for Header packet
} sl_si91x_chunk_type_t;

/** @brief SI91x-specific A/B fallback command types. */
typedef enum {
  SL_SI91X_AB_FALLBACK_WRITE_CMD             = 1, ///< Write firmware data for fallback
  SL_SI91X_AB_FALLBACK_READ_CMD              = 2, ///< Read firmware data for verification
  SL_SI91X_AB_FALLBACK_ERASE_CMD             = 3, ///< Erase fallback firmware slot
  SL_SI91X_AB_FALLBACK_INTEGRITY_CHECK_CMD   = 4, ///< Perform integrity check before fallback
  SL_SI91X_AB_FALLBACK_BURN_SECURITY_VERSION = 5, ///< Burn security version
  SL_SI91X_AB_FALLBACK_LOAD_QSPI_KEYS        = 6, ///< Load QSPI keys
  SL_SI91X_AB_FALLBACK_SOFT_RESET            = 7, ///< NWP Soft reset for fallback
  SL_SI91X_AB_FALLBACK_MAX_CMD               = 8  ///< Maximum fallback command value (for validation)
} sl_si91x_ab_fallback_cmd_t;

/************************** Structures ***************************/

/** @brief Structure for OTA image information. */
typedef struct {
  uint8_t ota_image_type;    ///< OTA image type
  uint32_t ota_image_offset; ///< OTA image offset
  uint32_t ota_image_size;   ///< OTA image size
} ota_image_info_t;

/** @brief Firmware update request structure. */
typedef struct {
  uint16_t chunk_type;    ///< Header (1) or Data (0)
  uint16_t sub_command;   ///< Operation type: WRITE (1), READ (2), ERASE (3), INTEGRITY_CHECK (4)
  uint32_t data_length;   ///< Length of data (for erase, must be a multiple of 4K)
  uint32_t flash_offset;  ///< Flash memory address: M4 -> 0x8000000, NWP -> 0x4000000
  uint8_t m4_updater_ota; ///< 0xEF if the updater is M4
  uint8_t reserved1;      ///< Reserved for future use
  uint16_t reserved2;     ///< Reserved for future use
  uint32_t reserved3;     ///< Reserved for future use
  uint8_t data
    [SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE]; ///< Data buffer (used for WRITE operation, NULL for READ, ERASE, and INTEGRITY_CHECK)
} sl_si91x_fw_fallback_request_t;

/** @brief Configuration structure for firmware fallback requests. */
typedef struct {
  uint32_t ota_image_data_length;  ///< Length of data (for erase, must be a multiple of 4K)
  uint32_t ota_image_flash_offset; ///< Flash memory address: M4 -> 0x8000000, NWP -> 0x4000000
  uint16_t ota_image_chunk_type;   ///< Header (1) or Data (0)
  uint8_t m4_updater_ota_flag;     ///< 0xEF if the M4 updater is used for OTA update
  uint8_t ota_image_type;          ///< Image type: 0=application, 1=updater
  uint8_t reserved1;               ///< Reserved for future use
  uint16_t reserved2;              ///< Reserved for future use
  uint32_t reserved3;              ///< Reserved for future use
} sl_si91x_fw_fallback_config_t;

/** @brief Structure for slot information. */
typedef struct {
  uint8_t slot_id;            ///< Slot Identifier (e.g., SLOT_A, SLOT_B)
  uint32_t slot_image_offset; ///< Base address of the slot in flash memory
  uint32_t image_size;        ///< Size of the firmware image in bytes
} SlotInfo_t;

/** @brief M4 slot management structure. */
typedef struct {
  SlotInfo_t m4_slot_A;           ///< Information for Slot A
  SlotInfo_t m4_slot_B;           ///< Information for Slot B
  uint8_t current_active_M4_slot; ///< Currently active slot (A or B)
} M4_SlotManagement_t;

/** @brief NWP slot management structure. */
typedef struct {
  SlotInfo_t nwp_slot_A;           ///< Information for NWP Slot A
  SlotInfo_t nwp_slot_B;           ///< Information for NWP Slot B
  uint8_t current_active_nwp_slot; ///< Currently active NWP slot (A or B)
} NWP_SlotManagement_t;

/** @brief Firmware slot management structure. */
typedef struct {
  uint32_t slot_magic_word;           ///< Magic word for slot integrity verification (0xA5A5B5B5)
  M4_SlotManagement_t m4_slot_info;   ///< Slot information for M4
  NWP_SlotManagement_t nwp_slot_info; ///< Slot information for NWP
  uint32_t slot_struct_crc;           ///< CRC for the slot information structure
} sl_si91x_fw_ab_slot_management_t;

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
  * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  ******************************************************************************/
sl_status_t sl_si91x_ab_upgrade_get_rps_configs(const uint8_t *image_buffer, ota_image_info_t *ota_st);

/***************************************************************************/ /**
  *  @fn          sl_status_t sl_si91x_flash_write(uint32_t address, const uint8_t *buffer, uint32_t length)
  *  @pre         None
  *  @brief       Write data to flash memory.
  *               This function writes data to the specified flash memory address.
  *               This function will work for A/B firmware only.
  *  @param[in]   address            Flash memory address to write to.
  *  @param[in]   buffer             Pointer to the data buffer.
  *  @param[in]   length             Number of bytes to write (min 0 max 4096).
  *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
  * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  ******************************************************************************/
sl_status_t sl_si91x_flash_write(uint32_t address, const uint8_t *buffer, uint32_t length);

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_fw_fallback_ota_flash_write(const sl_si91x_fw_fallback_config_t *config, const uint8_t *data_buffer)
 *  @pre         1. Flash region must be erased before writing.
 *               2. Data chunk size must not exceed 1024 bytes.
 *  @brief       Writes firmware data to flash during OTA updates.
 *               Call this function when receiving each chunk of OTA firmware data.
 *               Must be called after erasing flash and before verifying the image.
 *               Works only with A/B firmware configuration.
 *
 *  @param[in]   config             Pointer to the firmware fallback configuration structure.
 *  @param[in]   data_buffer        Pointer to the data buffer to write.
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_fw_fallback_ota_flash_write(const sl_si91x_fw_fallback_config_t *config,
                                                 const uint8_t *data_buffer);

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
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_flash_read(uint32_t address, uint8_t *buffer, uint32_t length);

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
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_ab_upgrade_set_slot_info(uint32_t new_image_offset,
                                              uint32_t new_image_size,
                                              sl_si91x_ab_image_type_t image_type);

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_flash_erase(uint32_t address, uint32_t length)
 *  @pre         None
 *  @brief       Erases data from flash memory sector-wise.
 *
 *  This function erases data from the specified flash memory address in
 *  sector-sized blocks. The erase operation will only work for A/B firmware
 *  slots and ensures that no partial sector erases occur.
 *
 *  @note        The erase operation is performed in full sectors. If the
 *               provided address or length does not align with sector boundaries,
 *               the function will round to the nearest sector boundary.
 *
 *  @param[in]   address  Starting flash memory address to erase (must be sector-aligned).
 *  @param[in]   length   Number of bytes to erase (will be rounded up to sector size).
 *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status). 
 *******************************************************************************/
sl_status_t sl_si91x_flash_erase(uint32_t address, uint32_t length);

/***************************************************************************/ /**
  *  @fn          sl_status_t sl_si91x_ab_get_slot_info(sl_si91x_fw_ab_slot_management_t *slot_info_t)
  *  @pre         None
  *  @brief       Retrieves the current slot information.
  *               This function reads the current slot information from flash memory.
  *               This function will work for A/B firmware only.
  *  @param[out]  slot_info_t        Pointer to the sl_si91x_fw_ab_slot_management_t structure to store the slot information.
  *  @return      sl_status_t        SL_STATUS_OK if successful, error code otherwise.
  ******************************************************************************/
sl_status_t sl_si91x_ab_get_slot_info(sl_si91x_fw_ab_slot_management_t *slot_info_t);

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_toggle_slot_info(bool toggle_m4_image, bool toggle_nwp_image)
 *  @pre         None
 *  @brief       Toggles the active slot information for M4 or NWP.
 *               This function retrieves the current slot information, checks which slot is active,
 *               and switches to the other slot by calling the sl_si91x_set_slot_info API.
 *  @param[in]   toggle_m4_image    Set to 1 to toggle M4 slot, 0 otherwise.
 *  @param[in]   toggle_nwp_image   Set to 1 to toggle NWP slot, 0 otherwise.
 *  @return      sl_status_t        SL_STATUS_OK if successful, error code otherwise.
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_toggle_slot_info(bool toggle_m4_image, bool toggle_nwp_image);

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
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_get_m4_app_addr(uint32_t *app_addr);

/***************************************************************************/ /**
 *  @fn          void sl_si91x_jump_to_m4_application(uint32_t app_addr)
 *  @brief       Jumps to the M4 application.
 *               This function disables interrupts, fetches the stack pointer and reset handler
 *               from the application's vector table, sets the VTOR to point to the new
 *               application vector table, and branches to the reset handler.
 *               This function will work for A/B firmware only.
 *  @param[in]   app_addr           Address of the application to jump to.
 ******************************************************************************/
void sl_si91x_jump_to_m4_application(uint32_t app_addr);

/***************************************************************************/ /**
  *  @fn          sl_status_t sl_si91x_verify_image(uint32_t address)
  *  @pre         None
  *  @brief       Checks the integrity of the firmware for Fallback.
  *               This function sends a command to check the integrity of the firmware
  *               at the specified flash memory address. It populates the request structure
  *               with the necessary parameters and sends the command to the firmware.
  *               This function will work for A/B firmware only.
  *  @param[in]   address            Flash memory address to check integrity.
  *  @return      sl_status_t        SL_STATUS_OK on success, error code otherwise.
  * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
  ******************************************************************************/
sl_status_t sl_si91x_verify_image(uint32_t flash_address);

/***************************************************************************/ /**
 *  @fn          int16_t sl_si91x_select_default_nwp_fw(const uint8_t fw_image_number)
 *  @pre         None
 *  @brief       Selects the default NWP firmware image to load.
 *               This function sends the appropriate command to select which
 *               firmware image (0 for slot A, 1 for slot B) should be loaded on boot.
 *  @param[in]   fw_image_number    Firmware image number to select (0 for slot A, 1 for slot B)
 *  @return      int16_t - Error code
 *                    - 0       : SUCCESS
 *                    - < 0     : Command issue failed
 *                    - -14     : Valid Firmware not present
 *                    - -15     : Invalid Option/Command not supported
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
int16_t sl_si91x_select_default_nwp_fw(const uint8_t fw_image_number);

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_burn_nwp_security_version(uint32_t flash_address)
 *  @pre         1. Valid NWP firmware must be present at the specified flash address.
 *               2. OTP memory must not be programmed with a higher security version.
 *  @brief       Burns the NWP security version to OTP memory to prevent firmware rollback.
 *               Call this function after validating a new NWP firmware version and before
 *               using it. This operation is irreversible - once burned, only firmware
 *               with equal or higher security versions can be loaded.
 *
 *  @param[in]   flash_address        Base address of NWP firmware in flash memory.
 *  @return      sl_status_t          SL_STATUS_OK on success, error code otherwise.
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).  
 ******************************************************************************/
sl_status_t sl_si91x_burn_nwp_security_version(uint32_t flash_address);

/***************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_fallback_load_qspi_keys(uint32_t image_offset)
 *  @pre         1. Firmware encryption must be enabled.
 *               2. Valid firmware must exist at the specified offset.
 *  @brief       Loads QSPI keys for encrypted firmware execution.
 *               Call this function:
 *               - After system reset when using encrypted firmware
 *               - Before switching to a different encrypted firmware slot
 *               - Before performing NWP soft reset with encrypted firmware
 *
 *  @param[in]   image_offset         Base address of encrypted firmware in flash.
 *  @return      sl_status_t          SL_STATUS_OK on success, error code otherwise.
 * For more information on status codes, refer to [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).  
 ******************************************************************************/
sl_status_t sl_si91x_fallback_load_qspi_keys(uint32_t image_offset);

/***************************************************************************/ /**
 *  @fn          void sl_si91x_nwp_soft_reset_from_updater(const uint32_t m4_slot_image_offset)
 *  @pre         1. Device must be configured with A/B firmware slots.
 *               2. M4 updater firmware must be running.
 *               3. Active slot information must be retrieved.
 *               4. Image verification must be successful.
 *               5. QSPI keys must be loaded.
 *  @brief       Performs a soft reset of the NWP firmware during A/B firmware fallback operations.
 *               This function is a critical part of the firmware update process that:
 *               - Ensures safe transition between firmware versions
 *               - Maintains system stability during updates
 *               - Prepares NWP for firmware switch
 *               - Enables fallback to previous version if needed
 *               
 *               The function operates in a dual-bank system where two firmware slots
 *               (A and B) are maintained. This design allows one slot to remain as a
 *               backup while the other is being updated, ensuring system recovery
 *               capability if an update fails.
 *
 *  @param[in]   m4_slot_image_offset  Base address of M4 firmware in active slot (A or B).
 *                                     This address points to the firmware image that will
 *                                     be used after the reset.
 *  @post        1. After this API call, the updater application must branch to the active slot application.
 *                  No other operations should not be performed after this API call.
 *               2. No NWP operations allowed between reset and M4 updater
 ******************************************************************************/

void sl_si91x_nwp_soft_reset_from_updater(const uint32_t m4_slot_image_offset);

/// @} (end addtogroup Firmware_Fallback) */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/***************************************************************************/
/***************************************************************************/
/**
 * @addtogroup FIRMWARE-FALLBACK Firmware Fallback A and B
 * @{
 *
 * @details
 *
 * @section Firmware_Fallback_Intro Introduction
 *
 * The Firmware Fallback feature provides mechanisms for managing A/B firmware upgrades
 * and fallback operations in Si91x devices. It ensures system reliability by maintaining
 * two firmware slots and enabling fallback to a stable firmware version in case of
 * an upgrade failure Additionally, this functionality requires a specific MBR configuration to work.
 *
 * @section Firmware_Fallback_Features Features
 *
 * - **A/B Slot Management**: Maintains two firmware slots (Slot A and Slot B) for active and backup firmware.
 * - **Integrity Checks**: Verifies firmware integrity using CRC and magic word validation.
 * - **Fallback Mechanism**: Switches to a backup firmware slot in case of failure.
 * - **OTA Support**: Supports Over-The-Air (OTA) firmware upgrades with slot management.
 *
 * @section Firmware_Fallback_Usage Usage
 *
 * The following steps outline the typical usage of the Firmware Fallback feature, providing clear guidance on how and when to use each API in the firmware update process:
 *
 * 1. **At OTA initiation**:  
 *    Use @ref sl_si91x_ab_upgrade_get_rps_configs to parse the incoming firmware image header. 
 *    This step extracts critical metadata, such as image size and type, 
 *    and is typically performed when the first chunk of OTA data arrives. 
 *    This information helps determine the type of update being performed 
 *    and allocate the necessary resources for the update process.
 *
 * 2. **Preparing the inactive slot for new firmware**:  
 *    Before writing the new firmware, use @ref sl_si91x_flash_erase to clear the inactive slot. 
 *    This ensures the slot is ready to receive the new firmware image 
 *    and prevents flash corruption. The inactive slot can be identified 
 *    using @ref sl_si91x_ab_get_slot_info, which retrieves the current slot configuration. 
 *
 * 3. **Writing firmware during OTA download**:  
 *    Use @ref sl_si91x_flash_write to store the received firmware chunks in the inactive 
 *    slot as they arrive from the OTA source. This function should be called for 
 *    each chunk of data received during the OTA session, ensuring proper addressing 
 *    and offset management. This step ensures that the new firmware is written correctly 
 *    to the designated slot.
 *
 * 4. **Validating the downloaded firmware**:  
 *    After the firmware download is complete, use @ref sl_si91x_verify_image to 
 *    validate the integrity of the downloaded firmware image. 
 *    This step ensures that only properly formed and uncorrupted firmware is 
 *    considered for activation.
 * 
 * 5. **Updating slot metadata after successful validation**:  
 *    Once the new firmware is verified, use @ref sl_si91x_ab_upgrade_set_slot_info to 
 *    update the slot metadata with information about the new firmware. 
 *    This prepares the slot for activation and ensures the slot information 
 *    accurately reflects the valid firmware version. 
 *    This step is essential to mark the new firmware as ready for execution.
 *
 * 6. **Retrieving current slot information**:  
 *    Use @ref sl_si91x_ab_get_slot_info to determine which slot (A or B) is currently 
 *    active for both M4 and NWP processors. This function also retrieves information 
 *    about firmware locations and sizes in flash memory. It is typically called before 
 *    performing any operation that depends on knowing the current slot configuration. 
 *    This step is useful for diagnostics and ensuring the correct slot is being used.
 *
 * 7. **Switching slots manually or for recovery**:  
 *    Use @ref sl_si91x_toggle_slot_info only in specific scenarios, such as recovering 
 *    from a failed firmware update by switching back to the previously working slot or 
 *    manually switching between existing firmware versions. After calling this function, 
 *    the system must be reset to load the newly activated firmware. 
 *    This function should not be part of the regular update process and 
 *    should only be used when both slots contain valid firmware images.
 * 
 * 
 * 8. **Burning NWP security version**:  
 *    Use @ref sl_si91x_burn_nwp_security_version to burn the NWP security version to OTP memory.
 *    This step is critical for enforcing firmware security on the NWP (Network Wireless Processor).
 *    Burning the security version to OTP (One-Time Programmable) memory ensures that only firmware images
 *    with a security version equal to or greater than the burned value can be executed. This mechanism
 *    prevents rollback attacks by disallowing the loading of older, potentially vulnerable firmware versions.
 *    This operation is irreversible and should be performed with caution, typically after a successful
 *    firmware update or when upgrading the device's security requirements.
 * 
 * 9. **Loading QSPI keys for inline decryption**:  
 *    Use @ref sl_si91x_fallback_load_qspi_keys to load the QSPI keys for
 *    enabling inline decryption for M4 slot applications during fallback.
 *    This function is essential when firmware images are stored in encrypted form in QSPI flash.
 *    Loading the QSPI keys allows the hardware to transparently decrypt firmware as it is read and executed,
 *    ensuring both security and performance. This step should be performed before attempting to boot or
 *    validate encrypted firmware images, and the keys must be securely provisioned and managed.
 * 
 * 10. **Soft reset for NWP firmware**:  
 *    Use @ref sl_si91x_nwp_soft_reset_from_updater to perform a soft reset of the NWP firmware for fallback
 *    and keep the NWP firmware in boot mode.
 *    This function is typically used after updating or recovering the NWP firmware, ensuring that the NWP
 *    restarts cleanly and enters a known state. Keeping the NWP in boot mode allows for further maintenance
 *    operations, such as additional firmware updates, diagnostics, or recovery actions, before normal
 *    operation resumes. This step is important for robust fallback and recovery workflows.
 * 
 * 11. **Default NWP firmware selection**:  
 *    Use @ref sl_si91x_select_default_nwp_fw to select the default NWP firmware image to load.
 *    This API allows you to specify which NWP (Network Wireless Processor) firmware slot (A or B)
 *    should be used as the default on the next boot. The function takes a firmware image number
 *    as input (0 for slot A, 1 for slot B) and updates the boot configuration accordingly.
 *    This is particularly useful for scenarios where you want to manually control which firmware
 *    version is active, such as after a successful OTA update, for testing a new firmware image,
 *    or for recovery purposes if the current firmware is not functioning as expected.
 *    The function returns 0 on success, a negative value if the command fails.
 *
 * 12. **Executing the firmware from the active slot**:  
 *    Use @ref sl_si91x_jump_to_m4_application to execute the firmware from the currently 
 *    active slot. This function is primarily used by the M4 Updater to branch 
 *    to the appropriate application after reading slot information. 
 *    It transfers control from the updater to the main application, completing the boot process. This step ensures the system boots into the correct firmware version.
 *
 * The following three APIs are required only when full encryption is enabled:
 *  - @ref sl_si91x_burn_nwp_security_version
 *  - @ref sl_si91x_fallback_load_qspi_keys
 *  - @ref sl_si91x_nwp_soft_reset_from_updater
 * 
 * By following these steps, the Firmware Fallback feature ensures a seamless and reliable firmware update process, with mechanisms for recovery and fallback in case of failures. This design minimizes downtime and ensures the device remains operational even in the event of update issues.
 *
 * @section Firmware_Fallback_Benefits Benefits
 *
 * - Ensures system reliability with robust fallback mechanisms.
 * - Simplifies OTA firmware upgrades with A/B slot management.
 * - Eliminates the need to transfer the image from the download region to the execution region, enabling faster updates.
 * - Provides APIs for seamless integration into application firmware.
 * - Optimizes flash memory usage for firmware storage.
 *
 * @} (end addtogroup FIRMWARE-FALLBACK)
 */
/***************************************************************************/

#ifdef __cplusplus
}
#endif
#endif // _SLI_SI91X_FW_FALLBACK__