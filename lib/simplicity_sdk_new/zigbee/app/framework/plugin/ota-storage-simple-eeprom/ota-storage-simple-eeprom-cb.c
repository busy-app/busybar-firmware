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

/** @brief Erase Complete
 *
 * This is called when an EEPROM erase operation has completed.
 *
 * @param success The result of the erase operation. Ver.: always
 */
WEAK(void sl_zigbee_af_ota_storage_simple_eeprom_erase_complete_cb(bool success))
{
  UNUSED_VAR(success);
}
