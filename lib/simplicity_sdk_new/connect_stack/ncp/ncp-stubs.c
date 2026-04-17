/***************************************************************************//**
 * @brief NCP stubs
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

#include "stack/include/ember.h"

EmberStatus emApiSetNcpSecurityKeyPersistent(uint8_t *keyContents, uint8_t keyLength, uint32_t key_id)
{
  (void)keyContents;
  (void)keyLength;
  (void)key_id;
  return EMBER_INVALID_CALL;
}

EmberStatus emApiSetNcpSecurityKey(uint8_t *keyContents, uint8_t keyLength)
{
  (void)keyContents;
  (void)keyLength;
  return EMBER_INVALID_CALL;
}
