/***************************************************************************//**
 * @file
 * @brief Data store and fetch routines.
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

#include PLATFORM_HEADER
#include "sl_code_classification.h"

// Copy bytes from source to destination, reversing the order.
void sl_util_reverse_mem_copy(uint8_t* dest, const uint8_t* src, uint16_t length)
{
  uint8_t i;
  uint8_t j = (length - 1);

  for ( i = 0; i < length; i++) {
    dest[i] = src[j];
    j--;
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BYTE_UTILITIES, SL_CODE_CLASS_TIME_CRITICAL)
uint16_t sl_util_fetch_low_high_int16u(const uint8_t *contents)
{
  return HIGH_LOW_TO_INT(contents[1], contents[0]);
}

uint16_t sl_util_fetch_high_low_int16u(const uint8_t *contents)
{
  return HIGH_LOW_TO_INT(contents[0], contents[1]);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_BYTE_UTILITIES, SL_CODE_CLASS_TIME_CRITICAL)
void sl_util_store_low_high_int16u(uint8_t *contents, uint16_t value)
{
  contents[0] = LOW_BYTE(value);
  contents[1] = HIGH_BYTE(value);
}

void sl_util_store_high_low_int16u(uint8_t *contents, uint16_t value)
{
  contents[0] = HIGH_BYTE(value);
  contents[1] = LOW_BYTE(value);
}

void sli_util_store_int32u(bool lowHigh, uint8_t* contents, uint32_t value)
{
  uint8_t ii;
  for (ii = 0; ii < 4; ii++) {
    uint8_t index = (lowHigh ? ii : 3 - ii);
    contents[index] = (uint8_t)(value & 0xFF);
    value = (value >> 8);
  }
}

uint32_t sli_util_fetch_int32u(bool lowHigh, const uint8_t* contents)
{
  uint8_t b0;
  uint8_t b1;
  uint8_t b2;
  uint8_t b3;

  if (lowHigh) {
    b0 = contents[3];
    b1 = contents[2];
    b2 = contents[1];
    b3 = contents[0];
  } else {
    b0 = contents[0];
    b1 = contents[1];
    b2 = contents[2];
    b3 = contents[3];
  }
  return ((((uint32_t) ((uint16_t) (((uint16_t) b0) << 8) | ((uint16_t) b1)))
           << 16)
          | (uint32_t) ((uint16_t) (((uint16_t) b2) << 8) | ((uint16_t) b3)));
}
