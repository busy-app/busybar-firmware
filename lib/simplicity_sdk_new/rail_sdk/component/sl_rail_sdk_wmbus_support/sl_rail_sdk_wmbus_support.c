/***************************************************************************//**
 * @file
 * @brief Wireless M-Bus support component
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_component_catalog.h"
#include "sl_rail_sdk_wmbus_support.h"
#include "sl_rail_sdk_wmbus_support_config.h"
#include <string.h>

#include "em_gpcrc.h"
#include "em_cmu.h"
#include "sl_common.h"

#include "mbedtls/aes.h"

//(1B length field + 255B payload + 17block * 2B CRC) * 3/2 3of6 ratio + 1B postamble
//if you need software Manchester, you should change it to (1+255+17*2)*2+1
#define WMBUS_MAX_ENCODED_LENGTH (1 + 255 + 17 * 2) * 3 / 2 + 1

//length of an encoded full block (16B+2B crc) using T mode's 3of6
#define ENC_FULLBLOCK_LENGTH_T 27
//length of an encoded first block (10B+2B crc) using T mode's 3of6
#define ENC_FIRSTBLOCK_LENGTH_T 18

//stores the current mbus Mode. Currently only used for modeT TX.
static sl_rail_sdk_wmbus_mode_t wmbusMode;

//tools for encryption
static mbedtls_aes_context aes_ctx;
SL_ALIGN(4) static uint8_t aesKey[16] SL_ATTRIBUTE_ALIGN(4); //the aes key should be stored 32-bit aligned for mbedTls

/// Constant for accessibility setting used in the app
static sl_rail_sdk_wmbus_accessibility_t wmbus_accessibility = SL_RAIL_SDK_WMBUS_ACCESSIBILITY;

/**
 * Get the current Wireless M-Bus accessibility settings
 *
 * @return the current value
 */
sl_rail_sdk_wmbus_accessibility_t sl_rail_sdk_wmbus_get_accessibility(void)
{
  return wmbus_accessibility;
}

/**
 * Set the current Wireless M-Bus accessibility settings
 *
 * @param[in] new_wmbus_accessibility to be applied
 */
void sl_rail_sdk_wmbus_set_accessibility(sl_rail_sdk_wmbus_accessibility_t new_wmbus_accessibility)
{
  wmbus_accessibility = new_wmbus_accessibility;
}

void sl_rail_sdk_wmbus_init(void)
{
  // setting mode according to configuration
  sl_rail_sdk_wmbus_set_mode(SL_RAIL_SDK_WMBUS_TYPE);
}

void sl_rail_sdk_wmbus_set_mode(sl_rail_sdk_wmbus_mode_t mode)
{
  wmbusMode = mode;
}

sl_rail_sdk_wmbus_mode_t sl_rail_sdk_wmbus_get_selected_mode(void)
{
  return wmbusMode;
}

uint32_t sl_rail_sdk_wmbus_get_meter_limited_acc_rx_start(bool slowMode)
{
  uint32_t return_value = 0UL;
  switch (wmbusMode) {
    case WMBUS_MODE_S:
    case WMBUS_MODE_R2:
      return_value = 3000UL;
      break;
    case WMBUS_MODE_T_METER:
    case WMBUS_MODE_T_COLLECTOR:
      return_value = 2000UL;
      break;
    case WMBUS_MODE_C:
    case WMBUS_MODE_F:
      if (slowMode) {
        return_value = 999500UL;
      } else {
        return_value = 99500UL;
      }
      break;
    case WMBUS_MODE_N_24:
      return_value = 2099500UL;
      break;
    case WMBUS_MODE_N_48:
    case WMBUS_MODE_N_192:
      return_value = 1099500UL;
      break;
    default:
      break;
  }
  return return_value;
}

uint32_t sl_rail_sdk_wmbus_get_meter_limited_acc_rx_stop(bool slowMode)
{
  uint32_t return_value = 0UL;
  switch (wmbusMode) {
    case WMBUS_MODE_S:
    case WMBUS_MODE_R2:
      return_value = 50000UL;
      break;
    case WMBUS_MODE_T_METER:
    case WMBUS_MODE_T_COLLECTOR:
      return_value = 3000UL;
      break;
    case WMBUS_MODE_C:
    case WMBUS_MODE_F:
      if (slowMode) {
        return_value = 1000500UL;
      } else {
        return_value = 100500UL;
      }
      break;
    case WMBUS_MODE_N_24:
      return_value = 2100500UL;
      break;
    case WMBUS_MODE_N_48:
    case WMBUS_MODE_N_192:
      return_value = 1100500UL;
      break;
    default:
      break;
  }
  return return_value;
}

uint16_t sl_rail_sdk_wmbus_frame_chars_to_mfield(const char* manuf)
{
  uint16_t code = 0;

  if (manuf != NULL) {
    code = (uint16_t)((manuf[0] - 64) << 10);
    code |= (uint16_t)((manuf[1] - 64) << 5);
    code |= (uint16_t)(manuf[2] - 64);
  }
  return code;
}

void sl_rail_sdk_wmbus_frame_mfield_to_chars(const uint16_t mfield, char* manuf)
{
  if (manuf != NULL) {
    manuf[0] = ((mfield >> 10) & 0x1F) + 64;
    manuf[1] = ((mfield >> 5) & 0x1F) + 64;
    manuf[2] = ((mfield >> 0) & 0x1F) + 64;
  }
}

uint8_t sl_rail_sdk_wmbus_frame_add_payload_int8(uint8_t *buffer,
                                                 sl_rail_sdk_wmbus_frame_dib_function_t dibFunction,
                                                 uint8_t vib,
                                                 int8_t value)
{
  if (buffer == NULL) {
    return 0;
  }
  buffer[0] = (uint8_t)((dibFunction << 4) + 1);   //DIF=1 is int8
  buffer[1] = vib;
  memcpy(buffer + 2, &value, 1);
  return 3;
}

uint8_t sl_rail_sdk_wmbus_frame_add_payload_int16(uint8_t *buffer,
                                                  sl_rail_sdk_wmbus_frame_dib_function_t dibFunction,
                                                  uint8_t vib,
                                                  int16_t value)
{
  if (buffer == NULL) {
    return 0;
  }
  buffer[0] = (uint8_t)((dibFunction << 4) + 2);   //DIF=2 is int16
  buffer[1] = vib;
  memcpy(buffer + 2, &value, 2);
  return 4;
}

uint8_t sl_rail_sdk_wmbus_frame_add_payload_int32(uint8_t *buffer,
                                                  sl_rail_sdk_wmbus_frame_dib_function_t dibFunction,
                                                  uint8_t vib,
                                                  int32_t value)
{
  if (buffer == NULL) {
    return 0;
  }
  buffer[0] = (uint8_t)((dibFunction << 4) + 4);   //DIF=4 is int32
  buffer[1] = vib;
  memcpy(buffer + 2, &value, 4);
  return 6;
}

uint16_t sl_rail_sdk_wmbus_frame_add_idle_filler(uint8_t *buffer, uint16_t fillerLength)
{
  if (buffer == NULL) {
    return 0;
  }
  memset(buffer, 0x2f, fillerLength);
  return fillerLength;
}

void sl_rail_sdk_wmbus_frame_crypto5_init(void)
{
  mbedtls_aes_init(&aes_ctx);
}

void sl_rail_sdk_wmbus_frame_crypto5_set_key(const uint8_t *newKey)
{
  memcpy(aesKey, newKey, 16);
}

void sl_rail_sdk_wmbus_frame_crypto5_get_key(uint8_t *key)
{
  memcpy(key, aesKey, 16);
}

uint8_t sl_rail_sdk_wmbus_frame_crypto5_encrypt(uint8_t *input,
                                                uint8_t *output,
                                                uint8_t *iv,
                                                uint16_t length,
                                                uint8_t encryptedBlocks)
{
  if (input == NULL || output == NULL || iv == NULL) {
    return 0;
  }

  mbedtls_aes_setkey_enc(&aes_ctx, aesKey, 128);

  if (encryptedBlocks == 0) {
    encryptedBlocks = (uint8_t)(length / 16);
    if (length % 16) {
      encryptedBlocks++;
    }
  }

  //add 0x2F padding as per EN13757-3
  if ((uint16_t)encryptedBlocks * 16 > length) {
    sl_rail_sdk_wmbus_frame_add_idle_filler(input + length, (uint16_t)encryptedBlocks * 16 - length);
  }

  mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, (uint16_t)encryptedBlocks * 16, iv, input, output);

  return encryptedBlocks;
}

void sl_rail_sdk_wmbus_frame_crypto5_decrypt(uint8_t *input,
                                             uint8_t *output,
                                             uint8_t *iv,
                                             uint16_t length)
{
  if (input == NULL || output == NULL || iv == NULL) {
    return;
  }

  mbedtls_aes_setkey_dec(&aes_ctx, aesKey, 128);
  mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, length, iv, input, output);
}
