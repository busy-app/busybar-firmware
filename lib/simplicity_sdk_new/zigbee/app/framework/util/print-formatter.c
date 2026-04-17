/***************************************************************************//**
 * @file
 * @brief Utilities for printing in common formats: buffers, EUI64s, keys
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"

//------------------------------------------------------------------------------

static void sli_zigbee_af_print_buffer(uint16_t area,
                                       const uint8_t *buffer,
                                       uint16_t bufferLen,
                                       const char * formatString)
{
  if (sl_zigbee_af_print_enabled(area)) {
    uint16_t index = 0;
    for (; index < bufferLen; index++) {
      sl_zigbee_af_print(area, formatString, buffer[index]);
    }
  }
}

void sl_zigbee_af_print_buffer(uint16_t area,
                               const uint8_t *buffer,
                               uint16_t bufferLen,
                               bool withSpace)
{
  sli_zigbee_af_print_buffer(area, buffer, bufferLen, (withSpace ? "%02X " : "%02X"));
}

void sl_zigbee_af_print_string(uint16_t area, const uint8_t *buffer)
{
  sli_zigbee_af_print_buffer(area, buffer + 1, sl_zigbee_af_string_length(buffer), "%c");
}

void sl_zigbee_af_print_long_string(uint16_t area, const uint8_t *buffer)
{
  sli_zigbee_af_print_buffer(area, buffer + 2, sl_zigbee_af_long_string_length(buffer), "%c");
}

void sl_zigbee_af_print_little_endian_eui64(const sl_802154_long_addr_t eui64)
{
  sl_zigbee_af_print(sl_zigbee_af_print_active_area,
                     "(%c)%02X%02X%02X%02X%02X%02X%02X%02X",
                     '<',
                     eui64[0],
                     eui64[1],
                     eui64[2],
                     eui64[3],
                     eui64[4],
                     eui64[5],
                     eui64[6],
                     eui64[7]);
}

void sl_zigbee_af_print_big_endian_eui64(const sl_802154_long_addr_t eui64)
{
  sl_zigbee_af_print(sl_zigbee_af_print_active_area,
                     "(%c)%02X%02X%02X%02X%02X%02X%02X%02X",
                     '>',
                     eui64[7],
                     eui64[6],
                     eui64[5],
                     eui64[4],
                     eui64[3],
                     eui64[2],
                     eui64[1],
                     eui64[0]);
}

void sl_zigbee_af_print_zigbee_key(const uint8_t *key)
{
  // Zigbee Keys are 16 bytes long
  sl_zigbee_af_print8_byte_blocks(2, key, false);
}

void sl_zigbee_af_print_cert(const uint8_t *cert)
{
  // Certificates are 48 bytes long
  sl_zigbee_af_print8_byte_blocks(6, cert, true);
}

void sl_zigbee_af_print_key(bool publicKey, const uint8_t *key)
{
  // ECC Public Keys are 22 bytes
  // ECC Private Keys are 21 bytes

  sl_zigbee_af_print_zigbee_key(key);
  sl_zigbee_af_print_buffer(sl_zigbee_af_print_active_area, key + 16, 5, true);
  sl_zigbee_af_println(sl_zigbee_af_print_active_area, (publicKey ? "%02X" : ""), key[21]);
}

void sl_zigbee_af_print_key_283k1(bool publicKey, const uint8_t *key)
{
  // ECC Public 283k1 Keys are 37 bytes
  // ECC Private 283k1 Keys are 36 bytes
  sl_zigbee_af_print8_byte_blocks(4, key, true);
  sl_zigbee_af_print_buffer(sl_zigbee_af_print_active_area, key + 32, 4, true);
  sl_zigbee_af_println(sl_zigbee_af_print_active_area, (publicKey ? "%02X" : ""), key[36]);
}

void sl_zigbee_af_print_cert_283k1(const uint8_t *cert)
{
  // ECC 283k1 certificates are 74 bytes long
  sl_zigbee_af_print8_byte_blocks(9, cert, true);
  sl_zigbee_af_println(sl_zigbee_af_print_active_area, "%02X %02X", cert[72], cert[73]);
}

void sl_zigbee_af_print_ieee_line(const sl_802154_long_addr_t ieee)
{
  sl_zigbee_af_print_big_endian_eui64(ieee);
  sl_zigbee_af_println(sl_zigbee_af_print_active_area, "");
}

void sl_zigbee_af_print_text_line(const char * text)
{
  sl_zigbee_af_println(sl_zigbee_af_print_active_area, "%s", text);
}

void sl_zigbee_af_print8_byte_blocks(uint8_t numBlocks,
                                     const uint8_t *block,
                                     bool crBetweenBlocks)
{
  uint8_t i;
  for (i = 0; i < numBlocks; i++) {
    sl_zigbee_af_print_buffer(sl_zigbee_af_print_active_area, block + 8 * i, 8, true);
    // By moving the '%s' to a separate function call, we can
    // save CONST space.  The above string is duplicated elsewhere in the
    // code and therefore will be deadstripped.
    sl_zigbee_af_print(sl_zigbee_af_print_active_area,
                       " %s",
                       (crBetweenBlocks || ((i + 1) == numBlocks) ? "\r\n" : ""));
  }
}

void sl_zigbee_af_print_issuer(const uint8_t* issuer)
{
  // The issuer field in the certificate is in big-endian form.
  sl_zigbee_af_print(sl_zigbee_af_print_active_area, "(>) ");
  sl_zigbee_af_print8_byte_blocks(1, issuer, true);
}

void sl_zigbee_af_print_channel_list_from_mask(uint32_t channelMask)
{
  if (sl_zigbee_af_print_enabled(sl_zigbee_af_print_active_area)) {
    uint8_t i;
    bool firstPrint = true;
    for (i = 0;
         i < SL_ZIGBEE_MAX_CHANNELS_PER_PAGE;
         i++) {
      if ((channelMask & 0x01UL) != 0U) {
        if (!firstPrint) {
          sl_zigbee_af_print(sl_zigbee_af_print_active_area, ", ");
        }
        sl_zigbee_af_print(sl_zigbee_af_print_active_area, "%d", i);
        firstPrint = false;
      }
      channelMask >>= 1;
    }
  }
}
