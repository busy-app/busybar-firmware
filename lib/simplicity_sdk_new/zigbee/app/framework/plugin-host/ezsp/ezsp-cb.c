/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

/** @brief Generates a random key
 *
 * This function is called when the Host needs to generate a random key.
 *
 * @param result: key pointer to be filled with random data Ver.: always
 */
WEAK(sl_status_t sl_zigbee_af_generate_random_key(sl_zigbee_key_data_t* result))
{
  uint16_t data;
  uint8_t* keyPtr = sl_zigbee_key_contents(result);
  uint8_t i;

  // Since our EZSP command only generates a random 16-bit number,
  // we must call it repeatedly to get a 128-bit random number.

  for ( i = 0; i < 8; i++ ) {
    sl_status_t status = sl_zigbee_ezsp_get_random_number(&data);

    if ( status != SL_STATUS_OK ) {
      return status;
    }

    keyPtr[0] = LOW_BYTE(data);
    keyPtr[1] = HIGH_BYTE(data);
    keyPtr += 2;
  }

  return SL_STATUS_OK;
}

/** @brief Generates random data for the specified size
 *
 * This function is called when the Host needs to generate random data.
 *
 * @param result: key pointer to be filled with random data Ver.: always
 * @param result: size of the array to be filled with random data Ver.: always
 */
WEAK(sl_status_t sl_zigbee_af_generate_random_data(uint8_t* result, uint8_t size))
{
  UNUSED_VAR(result);
  UNUSED_VAR(size);
  return SL_STATUS_NOT_AVAILABLE;
}
