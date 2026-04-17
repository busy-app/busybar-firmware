/***************************************************************************//**
 * @file sli_token_manager_format_klv.c
 * @brief klv read/write API handling implementation.
 * @version 1.0.0
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

#include "sl_device_clock.h"
#include "sl_hal_gpcrc.h"
#include "sli_token_manager_format_klv.h"
#include "sli_token_manager_internal.h"
#include "sli_token_manager_backend_nvm.h"
#if defined(SL_TOKEN_MANAGER_SECURITY)
#include "sli_token_manager_hal_crypto.h"
#include "sl_memory_manager.h"
#include "sl_token_manager_config.h"
#endif

// CRC configuration
#define CRC16_POLY          0x1021U
#define INIT_VALUE_CRC16    0xFFFFU

// Macro to calculate the security offset
#if defined(SL_TOKEN_MANAGER_SECURITY)
#define SLI_CRYPTO_TAG_SIZE            4
#define SLI_CRYPTO_RANDOM_NUMBER_SIZE  4
#define SLI_CRYPTO_NONCE_SIZE          12
#define SLI_CRYPTO_HEADER_OFFSET       0
#define SLI_CRYPTO_NONCE_OFFSET        (SLI_CRYPTO_HEADER_OFFSET + SLI_TOKEN_KLV_HEADER_SIZE)

#define SLI_TOKEN_MANAGER_GET_SECURITY_OFFSET(klv_start_address)                      \
  ((*(uint32_t *)(klv_start_address) == SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START) \
   ? (SLI_CRYPTO_RANDOM_NUMBER_SIZE + SLI_CRYPTO_TAG_SIZE) : 0)
#else
#define SLI_CRYPTO_RANDOM_NUMBER_SIZE 0
#define SLI_TOKEN_MANAGER_GET_SECURITY_OFFSET(klv_start_address) 0
#endif

/***************************************************************************//**
 * Returns the CRC16 value of the data.
 *
 * @note The CRC16-CCITT is used.
 *
 * @param[in]  data  Pointer to data for which CRC needs to be calculated.
 * @param[in]  len   Length of the data.
 *
 * @return a valid CRC16 value. zero otherwise.
 ******************************************************************************/
static uint16_t sli_token_manager_cal_checksum(uint8_t* data, uint32_t len)
{
 #if defined(_CMU_CLKEN0_MASK)
  CMU->CLKEN0_SET = CMU_CLKEN0_GPCRC0;
 #endif
  GPCRC0->EN = GPCRC_EN_EN;
  GPCRC0->CTRL = CRC16_POLY;
  GPCRC0->INIT = INIT_VALUE_CRC16;
  GPCRC0->CMD = GPCRC_CMD_INIT;

  while (len--) {
    GPCRC0->INPUTDATABYTE = *data++;
  }

  return (uint16_t)GPCRC0->DATA;
}

/***************************************************************************//**
 * Decodes the KLV header gets CRC, length and key information
 *
 * @note
 *
 * @param[in]  obj_adr           Address to the klv object.
 * @param[out] klv_header_info   contains CRC, length and key information
 *                               after header is decoded.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
static sl_status_t sli_token_manager_decode_klv_header(const uint8_t *obj_adr,
                                                       sl_klv_header_t *klv_header_info)
{
  if ((obj_adr == NULL) || (klv_header_info == NULL)) {
    TOKENDBG(printf("Null pointer error in sli_token_manager_decode_klv_header\n"));
    return SL_STATUS_NULL_POINTER;
  }

  const sl_klv_header_t *klv_header_data = (const sl_klv_header_t *)obj_adr;
  uint8_t obj_index = 0;
  uint32_t magic_number_and_crc_in_klv_header = 0;
  uint32_t length_and_key_in_klv_header = 0;

  // read magic number and crc
  magic_number_and_crc_in_klv_header = *((uint32_t *)(obj_adr) + obj_index);
  obj_index++;
  // read length and key
  length_and_key_in_klv_header = *((uint32_t *)(obj_adr) + obj_index);

  // Validate the KLV header
  if ((magic_number_and_crc_in_klv_header == SLI_TOKEN_KLV_ERASE_STATE)
      || (length_and_key_in_klv_header == SLI_TOKEN_KLV_ERASE_STATE)) {
    // KLV chain doesn't have KLV objects yet
    TOKENDBG(printf("KLV chain doesn't have KLV objects yet\n"));
    return SL_STATUS_NOT_FOUND;
  }

  if (klv_header_data->magic_number != SLI_TOKEN_KLV_MAGIC_NUMBER) {
    TOKENDBG(printf("Invalid magic_number\n"));
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (klv_header_data->length == 0xFFFF || klv_header_data->key == 0xFFFF) {
    TOKENDBG(printf("Invalid length or key\n"));
    return SL_STATUS_INVALID_PARAMETER;
  }

  *klv_header_info = *klv_header_data;

  TOKENDBG(printf("KLV Header : Magic Number - 0x%x, CRC - 0x%x, Length - 0x%x, Key - 0x%x\n",
                  klv_header_info->magic_number,
                  klv_header_info->crc,
                  klv_header_info->length,
                  klv_header_info->key));

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Returns the pointer to a flash address available for writing a new KLV token.
 *
 * @note
 *
 * @param[in]  key                      Token key.
 * @param[in]  klv_chain_start_address  The KLV chain start address.
 * @param[in]  klv_end_address          The KLV chain end address.
 *
 * @return a valid flash pointer if successful. NULL otherwise.
 ******************************************************************************/
static uint32_t sli_token_manager_check_token_in_klv_list(uint16_t key,
                                                          uint32_t *klv_chain_start_address,
                                                          uint32_t *klv_end_address)
{
  if ((klv_chain_start_address == NULL) || (klv_end_address == NULL)) {
    TOKENDBG(printf("Null pointer error in sli_token_manager_check_token_in_klv_list\n"));
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t security_offset = SLI_TOKEN_MANAGER_GET_SECURITY_OFFSET(klv_chain_start_address);
  // Skip the first 4 bytes, that represents KLV chain start
  uint8_t *obj_adr = (uint8_t *)((uint32_t *)klv_chain_start_address + 1);
  sl_klv_header_t klv_header_info;

  while ((uint32_t *)obj_adr < (uint32_t *)klv_end_address
         && *(uint32_t *)obj_adr != SLI_TOKEN_KLV_ERASE_STATE) {
    if (sli_token_manager_decode_klv_header(obj_adr, &klv_header_info) != SL_STATUS_OK) {
      return (uint32_t)obj_adr;
    }

    if (key == klv_header_info.key) {
      return (uint32_t)obj_adr;
    }

    obj_adr += SLI_TOKEN_KLV_HEADER_SIZE
               + SL_CEILING(klv_header_info.length, sizeof(uint32_t))
               + security_offset;
  }

  return (uint32_t)obj_adr;
}

 #if defined(SL_TOKEN_MANAGER_SECURITY)
/***************************************************************************//**
 * Create nonce for encryption.
 *
 * @param[in]  objAdr        Address to the klv object.
 * @param[in]  nonce_val     A pointer to nonce buffer which is updated by this function.
 * @param[in]  klv_headers   A pointer to the KLV header information.
 *
 * @return SL_STATUS_OK if successful or SL_STATUS_SECURITY_RANDOM_NUM_GEN_ERROR on failure.
 ******************************************************************************/
static sl_status_t sli_token_manager_create_nonce(uint32_t objAdr, uint8_t *nonce_val, sl_klv_header_t *klv_headers)
{
  sl_status_t status;
  uint8_t nonce[SLI_CRYPTO_NONCE_SIZE];
  // Construct nonce from random number
  status = hal_crypto_handle.gen_rand_num(&nonce[0], SLI_CRYPTO_RANDOM_NUMBER_SIZE);
  if (status != SL_STATUS_OK) {
    TOKENDBG(printf("random number generation failed, status=0x%lx.\n", status));
    return status;
  }

  nonce[4] = (objAdr & 0xFF000000) >> 24;
  nonce[5] = (objAdr & 0x00FF0000) >> 16;
  nonce[6] = (objAdr & 0x0000FF00) >> 8;
  nonce[7] = (objAdr & 0x000000FF);

  nonce[8] = (klv_headers->key & 0xFF00) >> 8;
  nonce[9] = (klv_headers->key & 0x00FF);
  nonce[10] = (klv_headers->length & 0xFF00) >> 8;
  nonce[11] = (klv_headers->length & 0x00FF);

  memcpy(&nonce_val[0], &nonce[0], sizeof(nonce));

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Get nonce for decryption.
 *
 * @param[in]  objAdr        Address to the klv object.
 * @param[in]  nonce_val     A pointer to nonce buffer which is updated by this function.
 * @param[in]  klv_headers   A pointer to the KLV header information.
 ******************************************************************************/
static void sli_token_manager_get_nonce(uint32_t objAdr, uint8_t *nonce_val, sl_klv_header_t *klv_headers)
{
  uint8_t nonce[SLI_CRYPTO_NONCE_SIZE];

  (void)sli_token_manager_flash_read(objAdr, &nonce[0], 4);

  nonce[4] = (objAdr & 0xFF000000) >> 24;
  nonce[5] = (objAdr & 0x00FF0000) >> 16;
  nonce[6] = (objAdr & 0x0000FF00) >> 8;
  nonce[7] = (objAdr & 0x000000FF);

  nonce[8] = (klv_headers->key & 0xFF00) >> 8;
  nonce[9] = (klv_headers->key & 0x00FF);
  nonce[10] = (klv_headers->length & 0xFF00) >> 8;
  nonce[11] = (klv_headers->length & 0x00FF);

  memcpy(&nonce_val[0], &nonce[0], sizeof(nonce));
}

/*****************************************************************************************************************//**
 * This function returns crypto algorithm w.r.t klv security type configured.
 *
 * @param[in]  sec_type     klv security type.
 *
 * @return Returns remapped crypto algorithm.
 ********************************************************************************************************************/
static hal_crypto_algo_t sli_token_manager_get_crypto_algo(sl_klv_security_type_t sec_type)
{
  hal_crypto_algo_t crypto_algo;

  switch (sec_type) {
    case SL_TOKEN_MANAGER_KLV_SECURITY_TYPE_NONE:
      crypto_algo = HAL_CRYPTO_ALGO_NONE;
      break;
    case SL_TOKEN_MANAGER_KLV_SECURITY_TYPE_AEAD:
      crypto_algo = HAL_CRYPTO_ALGO_AEAD;
      break;
    case SL_TOKEN_MANAGER_KLV_SECURITY_TYPE_ENC:
      crypto_algo = HAL_CRYPTO_ALGO_ENC;
      break;
    default:
      crypto_algo = HAL_CRYPTO_ALGO_AEAD;
      break;
  }

  return crypto_algo;
}

 #endif

/***************************************************************************//**
 * @brief
 *  Returns the size of a static token from the KLV header
 *
 * @param[in]  klv_start_address   The KLV start address of a KLV chain.
 * @param[in]  klv_end_address     The KLV end address of a KLV chain.
 * @param[in]  token               32-bit token identifier
 * @param[out] size_out            A pointer to where the token data size should be placed.
 *
 * @return SL_STATUS_OK if successful, an error code otherwise.
 ******************************************************************************/
sl_status_t sli_token_manager_get_size_from_klv(uint32_t klv_start_address,
                                                uint32_t klv_end_address,
                                                uint32_t token,
                                                uint32_t *size_out)
{
  if (((uint32_t *)klv_start_address == NULL) || ((uint32_t *)klv_end_address == NULL) || (size_out == NULL)) {
    TOKENDBG(printf("Null pointer error in sli_token_manager_get_size_from_klv\n"));
    return SL_STATUS_NULL_POINTER;
  }

  uint32_t klv_main_start = *((uint32_t *)klv_start_address);

  // Validate the KLV_MAIN_START
  if ((klv_main_start != SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START)
      && (klv_main_start != SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START)) {
    TOKENDBG(printf("Invalid KLV chain start 0x%lx\n", klv_main_start));
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint8_t *obj_adr = (uint8_t *)klv_start_address + sizeof(uint32_t);
  uint32_t security_offset = SLI_TOKEN_MANAGER_GET_SECURITY_OFFSET(klv_start_address);

  while ((uint32_t *)obj_adr < (uint32_t *)klv_end_address) {
    sl_klv_header_t klv_header_info;
    if (sli_token_manager_decode_klv_header(obj_adr, &klv_header_info) != SL_STATUS_OK) {
      return SL_STATUS_INVALID_PARAMETER;
    }

    if (klv_header_info.key == (token & 0xFFFFU)) {
      *size_out = klv_header_info.length;
      return SL_STATUS_OK;
    }

    obj_adr += SLI_TOKEN_KLV_HEADER_SIZE + SL_CEILING(klv_header_info.length, sizeof(uint32_t)) + security_offset;
  }

  return SL_STATUS_NOT_FOUND;
}

/***************************************************************************//**
 * Reads data from a klv object chain.
 *
 * @note
 *
 * @param[in]  klv_handle        A pointer to the sl_klv_handle_t,
 *                               containing length and key info
 * @param[in]  klv_start_address The KLV chain start address.
 * @param[in]  klv_end_address   The KLV chain end address.
 * @param[out] data              A pointer to where the token data should be placed.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sli_read_klv_object(sl_klv_handle_t const *klv_handle,
                                uint32_t *klv_start_address,
                                uint32_t *klv_end_address,
                                void *data)
{
  if ((klv_handle == NULL) || (klv_start_address == NULL) || (klv_end_address == NULL) || (data == NULL)) {
    TOKENDBG(printf("Null pointer error in sli_read_klv_object\n"));
    return SL_STATUS_NULL_POINTER;
  }

  if ((klv_handle->length == 0) || (klv_handle->length == 0xFFFF) || (klv_handle->key == 0xFFFF)) {
    TOKENDBG(printf("Invalid KLV handle parameters\n"));
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint32_t klv_main_start = *(uint32_t *)klv_start_address;

  if (klv_main_start != SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START
      && klv_main_start != SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START) {
    TOKENDBG(printf("Invalid KLV chain start\n"));
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint8_t *obj_adr = (uint8_t *)klv_start_address + sizeof(uint32_t);
  uint32_t security_offset = SLI_TOKEN_MANAGER_GET_SECURITY_OFFSET(klv_start_address);
  sl_status_t status = SL_STATUS_OK;

  while ((uint32_t *)obj_adr < (uint32_t *)klv_end_address) {
    sl_klv_header_t klv_header_info;

    status = sli_token_manager_decode_klv_header(obj_adr, &klv_header_info);

    if (status != SL_STATUS_OK) {
      return status;
    }

    if (klv_header_info.key == (klv_handle->key & 0xFFFFU)) {
      // Requested length from the data offset shouldn't exceed the total length
      if (klv_header_info.length < (klv_handle->offset + klv_handle->length)) {
        TOKENDBG(printf("Requested length exceeds available data length\n"));
        return SL_STATUS_INVALID_PARAMETER;
      }

      const uint8_t *ptr_src = (const uint8_t *)(obj_adr + SLI_TOKEN_KLV_HEADER_SIZE);

 #if defined(SL_TOKEN_MANAGER_SECURITY)
      uint8_t *ptr_dec = NULL;
      if (klv_main_start == SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START) {
        uint8_t nonce[SLI_CRYPTO_NONCE_SIZE] = { 0 };
        uint8_t tag[SLI_CRYPTO_TAG_SIZE];

        // Allocate memory for decryption
        // TODO: Optimize memory allocation
        ptr_dec = (uint8_t *)sl_malloc(klv_header_info.length);
        if (ptr_dec == NULL) {
          TOKENDBG(printf("Memory allocation failed\n"));
          return SL_STATUS_ALLOCATION_FAILED;
        }

        // Get nonce and tag
        sli_token_manager_get_nonce((uint32_t)ptr_src, nonce, &klv_header_info);
        memcpy(tag, ptr_src + SLI_CRYPTO_RANDOM_NUMBER_SIZE + SL_CEILING(klv_header_info.length, sizeof(uint32_t)), SLI_CRYPTO_TAG_SIZE);

        // Decrypt data
        status = hal_crypto_handle.decrypt(nonce,
                                           (uint8_t *)&klv_header_info,
                                           sizeof(klv_header_info),
                                           ptr_src + SLI_CRYPTO_RANDOM_NUMBER_SIZE,
                                           klv_header_info.length,
                                           ptr_dec,
                                           tag,
                                           sli_token_manager_get_crypto_algo(SL_TOKEN_MANAGER_KLV_SECURITY_TYPE));
        if (status != SL_STATUS_OK) {
          TOKENDBG(printf("Decryption failed, status=0x%lx\n", status));
          sl_free(ptr_dec);
          return status;
        }
        ptr_src = ptr_dec;
      }
 #endif

      // Verify CRC for only static device tokens.
      // Static secure token data is encrypted using authenticated encryption (AEAD)
      // and therefore CRC is not verified.
      if (klv_main_start == SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START) {
        // Validate CRC
        uint16_t calculated_checksum = sli_token_manager_cal_checksum((uint8_t *)ptr_src, klv_header_info.length);
        if (calculated_checksum != klv_header_info.crc) {
          TOKENDBG(printf("CRC check failed\n"));
          return SL_STATUS_INVALID_PARAMETER;
        }
      }

      // Copy data to the user buffer
      memcpy(data, ptr_src + klv_handle->offset, klv_handle->length);

 #if defined(SL_TOKEN_MANAGER_SECURITY)
      if (ptr_dec != NULL) {
        sl_free(ptr_dec);
      }
 #endif
      return SL_STATUS_OK;
    }

    obj_adr += SLI_TOKEN_KLV_HEADER_SIZE + SL_CEILING(klv_header_info.length, sizeof(uint32_t)) + security_offset;
  }

  return SL_STATUS_NOT_FOUND;
}

/***************************************************************************//**
 * Writes a klv object into the chain.
 *
 * @note This function checks for a available space and then write
 * the klv object at the tail of the chain.
 *
 * A KLV objects are stored in sequence as shown below:
 * KLV chain representation:
 * ------------------------------------------------------------------
 * KLV_CHAIN_START | KLV_OBJECT_1 | KLV_OBJECT_2 | KLV_OBJECT_2 |...
 * ------------------------------------------------------------------
 * KLV_CHAIN_START is a 4-byte unique value that indicates the start of the
 * respective KLV chain. The KLV chain start address is aligned to 4 bytes.
 *
 * Each KLV object is stored as follows:
 * Static device token - stored in the SE MTP and therefore are not encrypted.
 * used for storing small token data:
 * -------------------------------------------
 * KLV Header (8 bytes) |Data (length bytes) |
 * -------------------------------------------
 *
 * Static secure token - the data is encrypted using authenticated encryption (AEAD):
 * -----------------------------------------------------------------------------
 * KLV Header (8 bytes) | Nonce (4 bytes) | Data (length bytes) | Tag (4 bytes) |
 * -----------------------------------------------------------------------------
 * KLV Header :
 *   The KLV header is a 8-byte structure that contains the following fields:
 *    - Magic number (2 bytes) : A unique identifier for the KLV object.
 *    - CRC (2 bytes)          : A checksum value for the data.
 *    - Length (2 bytes)       : The length of the data.
 *    - Key (2 bytes)          : A unique key for the KLV object.
 * Nonce :
 *    The nonce is a 4-bytes value generated randomly during encryption.
 *    Even though the stored nonce is 4-bytes, the actual nonce used for
 *    encryption is 12 bytes. The nonce is generated by concatenating
 *    the 4-byte random value with the 4-byte KLV header (CRC and length)
 *    and 4 bytes flash address.
 *
 * Data:
 *  The data is the actual content of the KLV object. The length of the data
 *  is specified in the KLV header.
 *
 * Tag :
 *    The tag is a 4-byte value that is generated during encryption.
 *
 * @param[in]  klv_handle        A pointer to the sl_klv_handle_t,
 *                               containing length and key info
 * @param[in]  klv_start_address The KLV chain start address.
 * @param[in]  klv_end_address   The KLV chain end address.
 * @param[out] data              A pointer to the data being written.
 *
 * @return SL_STATUS_OK if successful. Error code otherwise.
 ******************************************************************************/
sl_status_t sli_write_klv_object(sl_klv_handle_t const *klv_handle,
                                 uint32_t klv_start_address,
                                 uint32_t klv_end_address,
                                 void const *data)
{
  // Validate input parameters
  if ((klv_handle == NULL) || ((uint32_t *)klv_start_address == NULL) || ((uint32_t *)klv_end_address == NULL) || (data == NULL)) {
    TOKENDBG(printf("Null pointer error in sli_write_klv_object\n"));
    return SL_STATUS_NULL_POINTER;
  }

  if ((klv_handle->length == 0) || (klv_handle->length == 0xFFFF) || klv_handle->key == 0xFFFF) {
    TOKENDBG(printf("Invalid KLV handle parameters\n"));
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((klv_start_address % 4 != 0) || (klv_end_address % 4 != 0) || (klv_start_address >= klv_end_address)) {
    TOKENDBG(printf("Invalid address alignment or range\n"));
    return SL_STATUS_INVALID_RANGE;
  }

  uint32_t klv_main_start = *((uint32_t *)klv_start_address);
  if ((klv_main_start != SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START)
      && (klv_main_start != SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START)) {
    // KLV chain is not initialized, need to initialize it.
    TOKENDBG(printf("KLV chain is not initialized, need to initialize it\n"));
    if (klv_start_address == (uint32_t)SLI_TOKEN_STATIC_SECURE_TOKEN_BASE) {
      // Initialize KLV chain for secure token
      klv_main_start = SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START;
      // Write the initialized value to the secure token's KLV start address.
      if (sli_token_manager_flash_write(klv_start_address, &klv_main_start, sizeof(klv_main_start)) != SL_STATUS_OK) {
        TOKENDBG(printf("Failed to initialize KLV chain start\n"));
        return SL_STATUS_INITIALIZATION;
      }
    } else {
      // Initialize KLV chain for device token
      klv_main_start = SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START;
      // Copy the initialized value to the device token's KLV start address.
      memcpy((void *)klv_start_address, &klv_main_start, sizeof(klv_main_start));
    }
  }

  uint32_t *current_pointer =
    (uint32_t *)sli_token_manager_check_token_in_klv_list((klv_handle->key & 0xFFFF),
                                                          (uint32_t *)klv_start_address,
                                                          (uint32_t *)klv_end_address);

  if (current_pointer >= (uint32_t *)klv_end_address) {
    return SL_STATUS_FULL;
  }

  if (*current_pointer != SLI_TOKEN_KLV_ERASE_STATE) {
    if (klv_main_start == SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START) {
      // Allow static device token overwrite
      sl_klv_header_t existing_header;
      if (sli_token_manager_decode_klv_header((uint8_t *)current_pointer, &existing_header) != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }

      if (existing_header.length != klv_handle->length) {
        // Size mismatch
        return SL_STATUS_INVALID_PARAMETER;
      }

      // Check if the existing data matches the new data
      if (memcmp(data, ((uint8_t *)current_pointer + sizeof(sl_klv_header_t)), existing_header.length) == 0) {
        // Already exists
        return SL_STATUS_ALREADY_EXISTS;
      }
    } else {
      return SL_STATUS_ALREADY_EXISTS;
    }
  }

  const uint8_t *ptr_src = (const uint8_t *)data;
  uint32_t security_offset = SLI_TOKEN_MANAGER_GET_SECURITY_OFFSET((uint32_t *)klv_start_address);
  // Must be a multiple of 4 bytes
  uint16_t write_length_bytes = SL_CEILING(klv_handle->length, sizeof(uint32_t));
  if ((uint32_t *)((uint8_t *)current_pointer + sizeof(sl_klv_header_t) + write_length_bytes + security_offset)
      >= (uint32_t *)klv_end_address) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  sl_klv_header_t klv_headers = {
    .crc = 0,
    .magic_number = SLI_TOKEN_KLV_MAGIC_NUMBER,
    .key = klv_handle->key & 0xFFFF,
    .length = klv_handle->length
  };

  // Calculate CRC for only Static device token.
  // Static secure token data is encrypted using authenticated encryption (AEAD)
  // and therefore CRC is not calculated.
  if (klv_main_start == SLI_TOKEN_KLV_STATIC_DEVICE_TOKEN_MAIN_START) {
    klv_headers.crc = sli_token_manager_cal_checksum((uint8_t *)data, klv_handle->length);
  }

  const uint8_t *buffer_ptr = (const uint8_t *)&klv_headers;
  if (klv_main_start == SLI_TOKEN_KLV_STATIC_SECURE_TOKEN_MAIN_START) {
#if defined(SL_TOKEN_MANAGER_SECURITY)
    uint8_t buffer[12];
    uint8_t nonce_val[SLI_CRYPTO_NONCE_SIZE];
    uint8_t tag[SLI_CRYPTO_TAG_SIZE];
    sl_status_t status;
    uint8_t *ptr_enc = NULL;

    // Allocate memory for storing encrypted data
    // TODO: Optimize memory allocation
    ptr_enc = (uint8_t *)sl_malloc(klv_headers.length);
    if (ptr_enc == NULL) {
      TOKENDBG(printf("Memory allocation failed\n"));
      return SL_STATUS_ALLOCATION_FAILED;
    }
    buffer_ptr = (const uint8_t *)buffer;
    // Create nonce
    sli_token_manager_create_nonce((uint32_t)(((uint8_t *)current_pointer) + sizeof(klv_headers)), &nonce_val[0], &klv_headers);
    // Using the source buffer to hold the encrypted data
    status = hal_crypto_handle.encrypt(&nonce_val[0],
                                       (uint8_t *)&klv_headers,
                                       sizeof(klv_headers),
                                       data,
                                       klv_headers.length,
                                       ptr_enc,
                                       tag,
                                       sli_token_manager_get_crypto_algo(SL_TOKEN_MANAGER_KLV_SECURITY_TYPE));
    if (status == SL_STATUS_OK) {
      // Write klv header and nonce to the buffer
      memcpy(&buffer[SLI_CRYPTO_HEADER_OFFSET], &klv_headers, sizeof(klv_headers));
      memcpy(&buffer[SLI_CRYPTO_NONCE_OFFSET], &nonce_val[0], SLI_CRYPTO_RANDOM_NUMBER_SIZE);
      // write the tag to the flash
      status = sli_token_manager_flash_write((uint32_t)(((uint8_t *)current_pointer) + sizeof(klv_headers) + SLI_CRYPTO_RANDOM_NUMBER_SIZE + write_length_bytes), tag, SLI_CRYPTO_TAG_SIZE);
    }
    if (status != SL_STATUS_OK) {
      sl_free(ptr_enc);
      return status;
    }

    ptr_src = (const uint8_t *)ptr_enc;
#endif
    if ((sli_token_manager_flash_write((uint32_t)current_pointer, buffer_ptr, sizeof(klv_headers) + SLI_CRYPTO_RANDOM_NUMBER_SIZE) != SL_STATUS_OK)
        || (sli_token_manager_flash_write((uint32_t)(((uint8_t *)current_pointer) + sizeof(klv_headers) + SLI_CRYPTO_RANDOM_NUMBER_SIZE), ptr_src, write_length_bytes) != SL_STATUS_OK)) {
      #if defined(SL_TOKEN_MANAGER_SECURITY)
      sl_free(ptr_enc);
      #endif
      return SL_STATUS_FAIL;
    }
#if defined(SL_TOKEN_MANAGER_SECURITY)
    sl_free(ptr_enc);
#endif
  } else {
    memcpy(current_pointer, &klv_headers, sizeof(klv_headers));
    memcpy((uint8_t *)current_pointer + sizeof(klv_headers), data, write_length_bytes);
  }

  return SL_STATUS_OK;
}
