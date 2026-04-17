/***************************************************************************//**
 * @file
 * @brief Silicon Labs Secure Engine Manager API.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_SE_MANAGER_EXTMEM_H
#define SL_SE_MANAGER_EXTMEM_H

#include "sl_common.h"
#include "sli_se_manager_features.h"
#include "sl_se_manager_types.h"
#include "sl_status.h"
#include "sl_enum.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_SILICON_LABS_32B_SERIES_3) && defined(SLI_MAILBOX_COMMAND_SUPPORTED)

/// @addtogroup sl_se_manager
/// @{

/***************************************************************************//**
 * @addtogroup sl_se_manager_extmem External memory support
 *
 * @brief
 *   API for managing and accessing the external memory.
 *
 * @details
 *   The API for the external memory includes functions for managing
 *   external flash memory on Series-3 devices. The exerrnal flash
 *   memory is partitioned in a number of Code Regions and one Data region.
 *   The number of Code Regions is defined by @ref SL_SE_MAX_CODE_REGIONS and
 *   varies on different devices.
 *   The APIs include functions for
 *     - Code region configuration, erase and write
 *     - Data region get location, erase and write
 *
 * @{
 ******************************************************************************/

/// @addtogroup sl_se_memory_region_utils Memory Region commands
/// @{

/// Security level of a memory (code or data) region
typedef enum {
  SL_SE_MEMORY_REGION_SECURITY_LEVEL_PLAINTEXT = 0,
  SL_SE_MEMORY_REGION_SECURITY_LEVEL_ENC_ONLY,
  SL_SE_MEMORY_REGION_SECURITY_LEVEL_ENC_AUTH,
} sl_se_memory_region_security_level_t;

/// Code region configuration
typedef struct {
  unsigned int region_idx;         ///< Index of code region
  unsigned int region_size;        ///< Size of code region
  sl_se_memory_region_security_level_t security_level;   ///< Security level of region
  bool bank_swapping_enabled;      ///< Bank swapping enabled (if true)
  bool locked;                     ///< Region is locked (if true)
} sl_se_code_region_config_t;

/// @} (end addtogroup sl_se_memory_region_utils)

/// QSPI FLPLL reference clock selection
typedef enum {
  SLI_SE_QSPI_REF_CLOCK_HFXO = 0x00,
  SLI_SE_QSPI_REF_CLOCK_HFRCODPLL = 0x02,
} sli_se_qspi_ref_clock_t;

/// Target range of SCLK frequency, to select TRIM bits
typedef enum {
  SLI_SE_QSPI_PLLCLKFREQSEL_SCLK120MHZ = 0, ///< 0: SCLK 120MHz (FLPLL Freq / 2)
  SLI_SE_QSPI_PLLCLKFREQSEL_SCLK140MHZ = 1, ///< 1: SCLK 140MHz (FLPLL Freq / 2)
  SLI_SE_QSPI_PLLCLKFREQSEL_SCLK160MHZ = 2, ///< 2: SCLK 160MHz (FLPLL Freq / 2)
  SLI_SE_QSPI_PLLCLKFREQSEL_SCLK180MHZ = 3, ///< 3: SCLK 180MHz (FLPLL Freq / 2)
  SLI_SE_QSPI_PLLCLKFREQSEL_SCLK200MHZ = 4, ///< 4: SCLK 200MHz (FLPLL Freq / 2)
  SLI_SE_QSPI_PLLCLKFREQSEL_SCLK220MHZ = 5  ///< 5: SCLK 220MHz (FLPLL Freq / 2)
} sli_se_qspi_pllclkfreqsel_t;

/// Integer prescalar for deriving divided version of clk_per
typedef enum {
  SLI_SE_QSPI_CLKPERPRESC_DIV1 = 0,  ///< 0: DIV1
  SLI_SE_QSPI_CLKPERPRESC_DIV2 = 1,  ///< 1: DIV2
  SLI_SE_QSPI_CLKPERPRESC_DIV3 = 2,  ///< 2: DIV3
  SLI_SE_QSPI_CLKPERPRESC_DIV4 = 3   ///< 3: DIV4
} sli_se_qspi_clkperpresc_t;

typedef struct SL_ATTRIBUTE_PACKED {
  uint8_t int_div;                                ///< Integer divider, min val = 6, max val = 11
  uint16_t frac_div;                              ///< Fractional divider, min_val = 0, max val = 2047
  sli_se_qspi_pllclkfreqsel_t pll_clk_freq_sel;   ///< Target range of the SCLK frequency
  sli_se_qspi_clkperpresc_t clk_per_presc;        ///< How much to divide the FLPLL output down
  sli_se_qspi_ref_clock_t ref_clock;              ///< Reference clock for the FLPLL
} sli_se_qspi_flpll_config_t;

/// QSPI registers
typedef enum {
  SLI_SE_QSPI_FLPADDRVST = 0x00,
  SLI_SE_QSPI_FLPADCTRL1 = 0x01,
  SLI_SE_EMU_FLREG = 0x02,
  SLI_SE_QSPI_CALCFG0 = 0x03,
  SLI_SE_QSPI_CALCFG1 = 0x04,
  SLI_SE_QSPI_CSN0_AUTOCFG0 = 0x05,
  SLI_SE_QSPI_CSN0_AUTOCFG1 = 0x06,
} sli_se_qspi_reg_t;

/// Flash operation state.
SL_ENUM(sli_se_flash_state_t) {
  FLASH_IDLE = 0,
  FLASH_BUSY = 1,
  FLASH_ERROR = 2
};

/// Flash Status.
typedef union {
  struct SL_ATTRIBUTE_PACKED {
    sli_se_flash_state_t status;      ///< Status of the QSPI flash operation.
    uint8_t reserved[3];              ///< Reserved bytes.
  };

  uint32_t raw;                       ///< Raw status.
} sli_se_flash_status_t;

// SPI device command type.
SL_ENUM_GENERIC(sli_se_spi_command_t, uint8_t) {
  SLI_SE_SPI_COMMAND_READ = 0,
  SLI_SE_SPI_COMMAND_WRITE = 1,
};

// -----------------------------------------------------------------------------
// Prototypes

/// @addtogroup sl_se_memory_region_utils
/// @{

/***************************************************************************//**
 * @brief
 *   Get code region configuration
 *
 * @details
 *   This function returns the configurations of the code regions in the
 *   range given by start_region_idx and region_array_size.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[out] regions_array
 *   Pointer to array of code region config elements where to return existing
 *   configuration.
 * @param[in] start_region_idx
 *   Starting region index to get configuration for
 * @param[in] region_array_size
 *   Array size and number of region elements to read (from start_region_idx)
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_code_region_get_config(sl_se_command_context_t *cmd_ctx,
                                         sl_se_code_region_config_t *regions_array,
                                         unsigned int start_region_idx,
                                         unsigned int region_array_size);

/***************************************************************************//**
 * @brief
 *   Apply/write code region configuration
 *
 * @details
 *   This function applies the settings from the given @ref sl_se_code_region_config_t
 *   array to persistent SE memory.
 *
 * @note
 *   After calling this function a system reboot is required to enable the new
 *   region configurations.
 *
 *   This function can not be used to apply new configurations to a code region
 *   that has been closed, and will return an error. To reopen a code region,
 *   please refer \ref sl_se_code_region_erase.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] regions_array
 *   Pointer to array of code region config elements
 * @param[in] start_region_idx
 *   Starting region index to apply configuration for
 * @param[in] region_array_size
 *   Array size and number of region elements to apply (from start_region_idx)
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_code_region_apply_config(sl_se_command_context_t *cmd_ctx,
                                           sl_se_code_region_config_t *regions_array,
                                           unsigned int start_region_idx,
                                           unsigned int region_array_size);

/***************************************************************************//**
 * @brief
 *   Erase a host code region.
 *
 * @details
 *   This function erase all sectors in the specified code region.
 *
 * @note The device may hard fault if attempting to read from an empty code region
 *   that is configured to be in AXiP mode. The application should refrain from
 *   reading an empty, AXiP enabled code region before \ref sl_se_code_region_write
 *   has been called to fill the code region with valid encrypted data again.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @param[in] region_idx
 *   Index of code region to erase
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_code_region_erase(sl_se_command_context_t *cmd_ctx,
                                    unsigned int region_idx);

/***************************************************************************//**
 * @brief
 *   Partially erase a host code region.
 *
 * @details
 *   This function erases a specified number of 32KB blocks in a specified code region.
 *   Available from SE version 3.2.0.
 *
 * @note As with \ref sl_se_code_region_erase, the device may hard fault if attempting to
 *   read from an empty code region that is configured to be in AXiP mode. The device
 *   will also hard fault if attempting to read from non-erased blocks of a partially
 *   erased code region, given that SL_SE_LIFECYCLE_EVENT_AXIP_NONCE_ROLL_DISABLED is NOT set.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @param[in] region_idx
 *   Index of code region to erase
 *
 * @param[in] block_offset
 *   32KB block offset into the code region
 *
 * @param[in] num_blocks
 *   Number of 32KB blocks to erase
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_code_region_partial_erase(sl_se_command_context_t *cmd_ctx,
                                            unsigned int region_idx,
                                            uint32_t block_offset,
                                            uint32_t num_blocks);

/***************************************************************************//**
 * @brief
 *   Write data to a host code region.
 *
 * @details
 *   This function writes data to the specified code region.
 *
 *   <b>Not currently implemented</b>: If the input data is encrypted, this function
 *   may optionally decrypt to plaintext before writing to the code region.
 *
 *   <b>Not currently implemented</b>:  In order to guard against data errors (random bit
 *   errors or FI glitches) during upgrade, this function can optionally take a hash of
 *   the data buffer, e.g. SHA-256, in order to verify the integrity.
 *
 *   It is recommended to erase the code region by calling
 *   sl_se_code_region_erase before writing a new version of the code image.
 *   The data buffer can be located in RAM or memory-mapped flash.
 *   The size of the data buffer cannot be longer than the code region.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] region_idx
 *   Index of code region to write to
 * @param[in] offset
 *   Address offset (relative to the start address of the given code region)
 *   where data should be written
 * @param[in] data
 *   Data buffer to write to the code region.
 * @param[in] num_bytes
 *   Number of bytes to write to the code region.
 * @param[in] decryption_info
 *   <b>Not currently implemented</b>: Pointer to an sl_se_crypto_operation_t descriptor to use for optional
 *   decryption of the data. Only decryption algorithms (ciphers and AEADs)
 *   are allowed. If NULL, this function will not decrypt the data.
 * @param[in] integrity_check
 *   <b>Not currently implemented</b>: Pointer to an sl_se_crypto_operation_t descriptor to use for optional
 *   data integrity verification. Only hash algorithms are allowed. If an
 *   AEAD algorithm is specified for decryption_info then integrity_check
 *   must be NULL. If integrity_check is NULL and decryption_info is not an AEAD, this
 *   function will not attempt to verify the integrity of the data.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_code_region_write(sl_se_command_context_t *cmd_ctx,
                                    unsigned int region_idx,
                                    uint32_t offset,
                                    const void *data,
                                    uint32_t num_bytes,
                                    sl_se_crypto_operation_t *decryption_info,
                                    sl_se_crypto_operation_t *integrity_check);

/***************************************************************************//**
 * @brief
 *   Close code region.
 *
 * @details
 *   Close the code region for further writes and store the given version number
 *   in SE NVM, such that the version number used during writing can later be
 *   retrieved/inspected by polling the SE.
 *
 *   The SE will set a write-once write-protect bit corresponding to the code
 *   region, disallowing writes to that region by the host and/or SE subsystem
 *   until the next POR reset.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] region_idx
 *   Index of code region to finish
 * @param[in] version
 *   Optional version number of written code.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_code_region_close(sl_se_command_context_t *cmd_ctx,
                                    unsigned int region_idx,
                                    uint32_t version);

/***************************************************************************//**
 * @brief
 *   Get version of the current content in a code region.
 *
 * @details
 *   Returns the version number passed previously with sl_se_close_code_region
 *   for the requested region, as long as the region has not been erased since.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] region_idx
 *   Index of code region
 * @param[out] version
 *   Pointer to location where to return the version number of code in region
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_code_region_get_version(sl_se_command_context_t *cmd_ctx,
                                          unsigned int region_idx,
                                          uint32_t *version);

/***************************************************************************//**
 * @brief
 *   Erase a host code region. This is a non-blocking (asynchronous)
 *   version of @ref sl_se_code_region_erase().
 *
 * @details
 *   This function returns immediately. Upon return the erase will be ongoing
 *   so the core will effectively be stalled if executing code residing in
 *   flash memory. An alternative would be to execute code from RAM.
 *   Check erasure progress by calling @ref sli_se_flash_get_status().
 *   When a call to @ref sli_se_flash_get_status() return FLASH_IDLE, the
 *   erasure is complete and flash memory bookeeping metadata is updated.
 *   This is mandatory step, at least one call of @ref sli_se_flash_get_status()
 *   with FLASH_IDLE result is required to ensure memory integrity.
 *
 * @note
 *   Refer to @ref sl_se_code_region_erase() for details.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @param[in] region_idx
 *   Index of code region to erase
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_code_region_erase_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  unsigned int region_idx);

/***************************************************************************//**
 * @brief
 *   Partially erase a host code region. This is a non-blocking (asynchronous)
 *   version of @ref sl_se_code_region_partial_erase().
 *
 * @details
 *   This function returns immediately. Upon return the erase will be ongoing
 *   so the core will effectively be stalled if executing code residing in
 *   flash memory. An alternative would be to execute code from RAM.
 *   Check erasure progress by calling @ref sli_se_flash_get_status().
 *   When a call to @ref sli_se_flash_get_status() return FLASH_IDLE, the
 *   erasure is complete and flash memory bookeeping metadata is updated.
 *   This is mandatory step, at least one call of @ref sli_se_flash_get_status()
 *   with FLASH_IDLE result is required to ensure memory integrity.
 *
 * @note
 *   Refer to @ref sl_se_code_region_partial_erase() for details.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @param[in] region_idx
 *   Index of code region to erase
 *
 * @param[in] block_offset
 *   32KB block offset into the code region
 *
 * @param[in] num_block
 *   Number of 32KB blocks to erase
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_code_region_partial_erase_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                          unsigned int region_idx,
                                                          uint32_t block_offset,
                                                          uint32_t num_blocks);

/***************************************************************************//**
 * @brief
 *   Write data to a host code region. This is a non-blocking (asynchronous)
 *   version of @ref sl_se_code_region_write().
 *
 * @details
 *   This function returns immediately. Upon return the write will be ongoing
 *   so the core will effectively be stalled if executing code residing in
 *   flash memory. An alternative would be to execute code from RAM.
 *   Check write progress by calling @ref sli_se_flash_get_status().
 *   When a call to @ref sli_se_flash_get_status() return FLASH_IDLE, the
 *   write has completed.
 *   Data buffer can NOT be located flash memory.
 *
 * @note
 *   Refer to @ref sl_se_code_region_write() for details.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] region_idx
 *   Index of code region to write to
 * @param[in] offset
 *   Address offset (relative to the start address of the given code region)
 *   where data should be written
 * @param[in] data
 *   Data buffer to write to the code region.
 * @param[in] num_bytes
 *   Number of bytes to write to the code region.
 * @param[in] decryption_info
 *   <b>Not currently implemented</b>: Pointer to an sl_se_crypto_operation_t descriptor to use for optional
 *   decryption of the data. Only decryption algorithms (ciphers and AEADs)
 *   are allowed. If NULL, this function will not decrypt the data.
 * @param[in] integrity_check
 *   <b>Not currently implemented</b>: Pointer to an sl_se_crypto_operation_t descriptor to use for optional
 *   data integrity verification. Only hash algorithms are allowed. If an
 *   AEAD algorithm is specified for decryption_info then integrity_check
 *   must be NULL. If integrity_check is NULL and decryption_info is not an AEAD, this
 *   function will not attempt to verify the integrity of the data.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_code_region_write_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  unsigned int region_idx,
                                                  uint32_t offset,
                                                  const void *data,
                                                  uint32_t num_bytes,
                                                  sl_se_crypto_operation_t *decryption_info,
                                                  sl_se_crypto_operation_t *integrity_check);

/***************************************************************************//**
 * @brief
 *   Get location of data region
 *
 * @details
 *   This function returns the location of the data region including the start
 *   address and size. The data region is defined as the rest of the external
 *   flash, after the last code region. Therefore the start of the data region
 *   is the same as the end of the last code region ( code partition ). The
 *   size can be computed like this:
 *   size = FLASH_SIZE - ( end_of_code_partion -  FLASH_BASE )
 *   NOTE:
 *   Since the size of the code partition may be changed across device reset
 *   cycles, the start address and size of the of the data region will be
 *   changed accordingly.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] address
 *   Pointer to pointer where the start address of the data region will be
 *   written.
 * @param[in] size
 *   Pointer to 32-bit word where the size of the data region will be written.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_data_region_get_location(sl_se_command_context_t *cmd_ctx,
                                           void  **address,
                                           size_t *size);

/***************************************************************************//**
 * @brief
 *   Writes data to a data region in exernal memory.
 *
 * @details
 *   This function writes data to a data region in an external memory
 *   instance, e.g. Flash or PSRAM.
 *   Buffer can be located in RAM or memory-mapped flash.
 *   Buffer cannot go out of the data region boundaries.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] address
 *   Address of location to write to in host memory address map. Must map to a
 *   data region in external memory.
 * @param[in] data
 *   Pointer to data to write.
 * @param[in] num_bytes
 *   Number of bytes to write to data region.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
sl_status_t sl_se_data_region_write(sl_se_command_context_t *cmd_ctx,
                                    void *address,
                                    const void *data,
                                    size_t num_bytes);

/***************************************************************************//**
 * @brief
 *   Erase flash sector in the data region.
 *
 * @details
 *   The start address needs to be sector-aligned (4KiB) and part of the data
 *   partition.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] start_address
 *   Start address (mapped in host memory address map) of sector to erase.
 *   Must map to a data region in external flash.
 * @param[in] num_sectors
 *   Number of sectors (4KiB) to erase.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sl_se_data_region_erase(sl_se_command_context_t *cmd_ctx,
                                    void *start_address,
                                    size_t num_sectors);

/***************************************************************************//**
 * @brief
 *   Query the SE Manager to determine if a command that modifies the L1CACHE
 *   enabled/disabled state is in progress.
 *
 * @details
 *   This is used to determine if a command that modifies the L1CACHE
 *   enabled/disabled state is in progress.
 *
 * @warning
 *   In a multi-threaded context, another thread might start/end a command that
 *   modifies the L1CACHE enabled/disabled state just after/while this function
 *   executes. This function must be executed within a critical section to
 *   guarantee thread-safe decision-making. Any conditional logic such as
 *   invoking a function based on the return value must also occur within the
 *   critical section.
 *
 * @return
 *   true if a command that modifies the L1CACHE enabled/disabled state is
 *   in progress, false otherwise.
 ******************************************************************************/
 SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
 bool sli_se_extmem_command_with_cachedis_in_progress(void);

/***************************************************************************//**
 * @brief
 *   Erase flash sector in the data region. This is a non-blocking (asynchronous)
 *   version of @ref sl_se_data_region_erase().
 *
 * @details
 *   This function returns immediately. Upon return the erase will be ongoing
 *   so the core will effectively be stalled if executing code residing in
 *   flash memory. An alternative would be to execute code from RAM.
 *   Check erasure progress by calling @ref sli_se_flash_get_status().
 *   When a call to @ref sli_se_flash_get_status() return FLASH_IDLE, the
 *   erasure is complete and flash memory bookeeping metadata is updated.
 *   This is mandatory step, at least one call of @ref sli_se_flash_get_status()
 *   with FLASH_IDLE result is required to ensure memory integrity.
 *
 * @note
 *   Refer to @ref sl_se_data_region_erase() for details.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] start_address
 *   Start address (mapped in host memory address map) of sector to erase.
 *   Must map to a data region in external flash.
 * @param[in] num_sectors
 *   Number of sectors (4KiB) to erase.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_data_region_erase_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  void *start_address,
                                                  size_t num_sectors);

/***************************************************************************//**
 * @brief
 *   Writes data to a data region in exernal memory. This is a non-blocking
 *   (asynchronous) version of @ref sl_se_data_region_write().
 *
 * @details
 *   This function returns immediately. Upon return the write will be ongoing
 *   so the core will effectively be stalled if executing code residing in
 *   flash memory. An alternative would be to execute code from RAM.
 *   Check write progress by calling @ref sli_se_flash_get_status().
 *   When a call to @ref sli_se_flash_get_status() return FLASH_IDLE, the
 *   write has completed.
 *   Data buffer can NOT be located flash memory.
 *
 * @note
 *   Refer to @ref sl_se_data_region_write() for details.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] address
 *   Address of location to write to in host memory address map. Must map to a
 *   data region in external memory.
 * @param[in] data
 *   Data buffer to write to the data region.
 * @param[in] num_bytes
 *   Number of bytes to write to data region.
 *
 * @return
 *   SL_STATUS_OK when the function was successfully, or else, a status code
 *   of type sl_status_t that indicates why the command was not successful,
 *   @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_data_region_write_non_blocking(sl_se_command_context_t *cmd_ctx,
                                                  void *address,
                                                  const void *data,
                                                  size_t num_bytes);

/***************************************************************************//**
 * @brief
 *   Erase the entire host flash.
 *
 * @details
 *   This will erase the entire host flash. Calling it from flash will break
 *   the application. Available from SE version 3.0.1.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
SL_CODE_RAM
sl_status_t sli_se_erase_host_region(sl_se_command_context_t *cmd_ctx);

/// @} (end addtogroup sl_se_memory_region_utils)

/***************************************************************************//**
 * @brief
 *   Utility function to check if the flash is busy with an ongoing operation.
 *
 * @details
 *   Check if a long-running operation (page/region erase or bulk write) is
 *   currently being executed on the flash
 *
 * @return
 *   true if the flash is busy (flash status is FLASH_BUSY).
 *   false if mailbox command failed or flash status is FLASH_IDLE
 *   or FLASH_ERROR.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
bool sli_se_flash_is_busy(void);

/***************************************************************************//**
 * @brief
 *   Utility function to check if the flash is idle.
 *
 * @return
 *   true if the flash is idle (flash status is FLASH_IDLE).
 *   false if mailbox command failed or flash status is FLASH_BUSY
 *   or FLASH_ERROR.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
bool sli_se_flash_is_idle(void);

/***************************************************************************//**
 * @brief
 *   Get flash status.
 *
 * @details
 *   Returns a @ref sli_se_flash_state_t data structure.
 *   Structure member @ref sli_se_flash_state_t status will have value
 *   FLASH_IDLE, FLASH_BUSY or FLASH_ERROR.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[out] status
 *   Pointer to an @ref sli_se_flash_state_t object.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_flash_get_status(sl_se_command_context_t *cmd_ctx,
                                    sli_se_flash_status_t *status);

/***************************************************************************//**
 * @brief
 *   Pause an ongoing flash operation.
 *
 * @details
 *   Pause an ongoing long-running flash operation.
 *   Pausing which frees the auto-port such that code can run again.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_flash_pause(sl_se_command_context_t *cmd_ctx);

/***************************************************************************//**
 * @brief
 *   Resume a paused flash operation.
 *
 * @details
 *   Resume the previously paused long-running flash operation.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_flash_resume(sl_se_command_context_t *cmd_ctx);

#if defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)
/***************************************************************************//**
 * @brief
 *   Selects FSRCO as QSPI clock source and disables FLPLL
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_qspi_configure_clock_fsrco(sl_se_command_context_t *cmd_ctx);

/***************************************************************************//**
 * @brief
 *   Selects the reference clock for and configures the high speed FLPLL qspi clock.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] ref_clock_sel
 *   Selects clock, \ref sli_se_qspi_ref_clock_t, ref clock freq should be ~40MHz.
 * @param[in] flpll_config
 *   FLPLL configuration, \ref sli_se_qspi_flpll_config_t
 *
 * @details
 * The fractional divider is used to fine tune the final FLPLL frequency by
 * the following formula
 *
 * FLPLL_freq = ((int_div + (frac_div / 2048)) * ref_clock_freq )
 *
 * The final FLPLL frequency should be in the <240MHz-440MHz> range.
 * pll_clk_freq_sel in \ref sli_se_qspi_flpll_config_t should be selected
 * depending on the targeted FLPLL frequency range
 *
 * The FLPLL frequency is then divided by 2 before being output to the
 * QSPI. The FLPLL frequency can further be divided down by setting the
 * clk_per_presc value in \ref sli_se_qspi_flpll_config_t to obtain the
 * QSPI controller clock:
 *
 * QSPI Controller Clock = (FLPLL_freq / 2) / CLK_PER_PRESC
 *
 * Example: To achieve a QSPI controller clock of 100MHz, with a 40MHz reference
 * clock:
 *
 * Select int_div = 10, frac_div = 853 to achieve an FLPLL frequency of
 * 200MHz => pll_clk_freq_sel = 4. Set clk_per_presc = 1 (DIV2) to achieve
 * the QSPI controller clock of 100MHz.
 *
 * @note The caller is responsible for configuring a valid QSPI clock frequency
 * for the given external flash. No checks are performed to ensure a valid
 * frequency.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_se_qspi_configure_clock_flpll(sl_se_command_context_t *cmd_ctx,
                                              const sli_se_qspi_flpll_config_t *flpll_cfg);

/***************************************************************************//**
 * @brief
 *   Retrieve FLPLL configurations currently set in the QSPI registers
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[out] flpll_config
 *   FLPLL configuration, \ref sli_se_qspi_flpll_config_t
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_qspi_get_flpll_config(sl_se_command_context_t *cmd_ctx,
                                         sli_se_qspi_flpll_config_t *flpll_cfg);

/***************************************************************************//**
 * @brief
 *   Configure a QSPI/FLASH register
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] reg
 *   Register to modify, one of \ref sli_se_qspi_reg_t
 * @param[in] mask
 *   Mask to modify, all bits set to 1 will overwrite entire register
 * @param[in] val
 *   Values to write to register, together with mask
 * @param[out] result
 *   Optional, register value after write operation
 *
 * @details
 *   This function writes a value specified by a mask to one of the registers specified
 *   in \ref sli_se_qspi_reg_t. Note that no verification is done to ensure that valid
 *   values are set for the given register. No warning or error is issued if attempting
 *   to write to non-writeable bits in the register
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_qspi_configure_reg(sl_se_command_context_t *cmd_ctx,
                                      sli_se_qspi_reg_t reg,
                                      uint32_t mask,
                                      uint32_t val,
                                      uint32_t *result);

/***************************************************************************//**
 * @brief
 *   Read out the value of a QSPI/FLASH register
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] reg
 *   Register to modify, one of \ref sli_se_qspi_reg_t
 * @param[out] register_val
 *   The register value
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in @file sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_qspi_get_reg(sl_se_command_context_t *cmd_ctx,
                                sli_se_qspi_reg_t reg,
                                uint32_t *register_val);

#endif // defined(_SILICON_LABS_32B_SERIES_3_CONFIG_301)

/***************************************************************************//**
 * @brief
 *   Send a command to an external memory device connected to a QSPI peripheral
 *   instance. The command can send or receive data to/from the device or be
 *   a command without data phase.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] spi_instance:
 *   SPI instance number, currently only '1' (QSPI1) is supported.
 * @param[in] command_type
 *   Command type to send to device. Can be a read or write command.
 * @param[in] command_code
 *   Command code to send to device. Can be a byte or a half word (two bytes).
 * @param[in] command_size
 *   Size of command code in bytes. Can be 1 or 2.
 * @param[in] address
 *   Address to send to device. Set to 0 if no address phase is needed.
 * @param[in] address_size
 *   Size of address in bytes.
 * @param[in,out] data
 *   Pointer to data buffer. For write commands, data to send to device.
 *   For read commands, buffer to store received data.
 * @param[in] data_size
 *   Size of data buffer in bytes [0..8].
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_spi_device_command(sl_se_command_context_t *cmd_ctx,
                                      uint32_t spi_instance,
                                      sli_se_spi_command_t command_type,
                                      uint16_t command_code,
                                      uint8_t  command_size,
                                      uint32_t address,
                                      uint8_t  address_size,
                                      uint8_t  *data,
                                      uint8_t  data_size);

/***************************************************************************//**
 * @brief
 *   Write to a SPI peripheral register. Used to configure a QSPI interface
 *   for a specific SPI memory device.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] spi_instance:
 *   SPI instance number, currently only '1' (QSPI1) is supported.
 * @param[in] offset
 *   Register offset.
 * @param[in] value
 *   Value to write to register.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_write_spi_register(sl_se_command_context_t *cmd_ctx,
                                      uint32_t spi_instance,
                                      uint32_t offset,
                                      uint32_t value);

/***************************************************************************//**
 * @brief
 *   Write to several SPI peripheral registers. Used to configure a QSPI
 *   interface for a specific SPI memory device.
 *   Multiple writes is performed by using a table of offset-value pairs.
 *
 * @param[in] cmd_ctx
 *   Pointer to an SE command context object.
 * @param[in] spi_instance:
 *   SPI instance number, currently only '1' (QSPI1) is supported.
 * @param[in] table
 *   Table of offset-value pairs.
 * @param[in] count
 *   Size of table, i.e. number of register writes to perform.
 *
 * @return SL_STATUS_OK if the operation is successful, or error code
 *         defined in sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_write_spi_registers(sl_se_command_context_t *cmd_ctx,
                                       uint32_t spi_instance,
                                       uint32_t *table,
                                       uint32_t count);

#ifdef __cplusplus
}
#endif

/// @} (end addtogroup sl_se_manager_extmem)
/// @} (end addtogroup sl_se_manager)

// Deprecated Types and Functions Below

// Use sl_se_memory_region_security_level_t instead
typedef sl_se_memory_region_security_level_t sl_se_code_region_security_level_t SL_DEPRECATED_API_SDK_2025_6;

// #ifdef SL_SUPPRESS_DEPRECATION_WARNINGS_SDK_2025_6
#define SL_SE_CODE_REGION_SECURITY_LEVEL_PLAINTEXT SL_SE_MEMORY_REGION_SECURITY_LEVEL_PLAINTEXT
#define SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_ONLY SL_SE_MEMORY_REGION_SECURITY_LEVEL_ENC_ONLY
#define SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_AUTH SL_SE_MEMORY_REGION_SECURITY_LEVEL_ENC_AUTH
// #else
// #define SL_SE_CODE_REGION_SECURITY_LEVEL_PLAINTEXT _Pragma ("GCC warning \"'SL_SE_CODE_REGION_SECURITY_LEVEL_PLAINTEXT' macro is deprecated\"") SL_SE_MEMORY_REGION_SECURITY_LEVEL_PLAINTEXT
// #define SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_ONLY  _Pragma ("GCC warning \"'SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_ONLY' macro is deprecated\"") SL_SE_MEMORY_REGION_SECURITY_LEVEL_ENC_ONLY
// #define SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_AUTH _Pragma ("GCC warning \"'SL_SE_CODE_REGION_SECURITY_LEVEL_ENC_AUTH' macro is deprecated\"") SL_SE_MEMORY_REGION_SECURITY_LEVEL_ENC_AUTH
// #endif

// Use sl_code_region_config_t instead
typedef sl_se_code_region_config_t sl_code_region_config_t SL_DEPRECATED_API_SDK_2025_6;

#endif // defined(_SILICON_LABS_32B_SERIES_3) && defined(SLI_MAILBOX_COMMAND_SUPPORTED)

#endif // SL_SE_MANAGER_EXTMEM_H
