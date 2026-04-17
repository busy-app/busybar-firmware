/***************************************************************************/ /**
 * @file
 * @brief SL SI91X Crypto Utility source file
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

#include <stdlib.h>
#include "sl_si91x_crypto_utility.h"

uint8_t *pkcs7_padding(const uint8_t *data, size_t data_length, size_t block_size, size_t *padded_data_length)
{
  // Calculate the padding length
  size_t padding_length = block_size - (data_length % block_size);

  // Allocate memory for the padded data
  size_t padded_length = data_length + padding_length;

  uint8_t *padded_data = (uint8_t *)malloc(padded_length);
  if (!padded_data) {
    return NULL;
  }

  // Copy the original data to the padded buffer
  for (size_t i = 0; i < data_length; i++) {
    padded_data[i] = data[i];
  }

  // Add padding bytes with the value of the padding length
  for (size_t i = data_length; i < padded_length; i++) {
    padded_data[i] = (uint8_t)padding_length;
  }

  // Update the padded data length
  *padded_data_length = padded_length;

  return padded_data;
}

int pkcs7_unpad(const unsigned char *data, size_t data_length)
{
  if (data_length == 0) {
    return -1; // Invalid input
  }

  unsigned char padding = data[data_length - 1];

  if (padding >= data_length) {
    return -1; // Invalid padding
  }

  for (size_t i = 1; i <= padding; ++i) {
    if (data[data_length - i] != padding) {
      return -1; // Invalid padding
    }
  }

  return data_length - padding;
}
