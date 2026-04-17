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

/** @brief Status
 *
 * This callback is fired when the Update Link Key exchange process is updated
 * with a status from the stack. Implementations will know that the Update TC
 * Link Key plugin has completed its link key request when the keyStatus
 * parameter is SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS.
 *
 * @param keyStatus An ::sl_zigbee_key_status_t value describing the success or failure
 * of the key exchange process. Ver.: always
 */
WEAK(void sl_zigbee_af_update_tc_link_key_status_cb(sl_zigbee_key_status_t keyStatus))
{
  (void)keyStatus;
  return;
}
