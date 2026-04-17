/***************************************************************************//**
 * @file
 * @brief Provides support features for security, such as random number generation.
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
#include "tool/random/random-number-generation.h"

sl_status_t sl_zigbee_af_generate_random_key(sl_zigbee_key_data_t* result)
{
  if (platformRandomDataFunction(sl_zigbee_key_contents(result),
                                 SL_ZIGBEE_ENCRYPTION_KEY_SIZE)) {
    return SL_STATUS_ZIGBEE_INSUFFICIENT_RANDOM_DATA;
  }
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_af_generate_random_data(uint8_t* result, uint8_t size)
{
  if (platformRandomDataFunction(result, size)) {
    return SL_STATUS_ZIGBEE_INSUFFICIENT_RANDOM_DATA;
  }
  return SL_STATUS_OK;
}
