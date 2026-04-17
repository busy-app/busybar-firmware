/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL On Off Server plugin.
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
#include "zll-on-off-server.h"

/** @brief Off With Effect
 *
 * This callback is called by the ZLL On/Off Server plugin whenever an
 * OffWithEffect command is received. The application should implement the
 * effect and variant requested in the command and return
 * ::SL_ZIGBEE_ZCL_STATUS_SUCCESS if successful or an appropriate error status
 * otherwise.
 *
 * @param endpoint   Ver.: always
 * @param effectId   Ver.: always
 * @param effectVariant   Ver.: always
 */
WEAK(sl_zigbee_af_status_t sl_zigbee_af_zll_on_off_server_off_with_effect_cb(uint8_t endpoint,
                                                                             uint8_t effectId,
                                                                             uint8_t effectVariant))
{
  (void)endpoint;
  (void)effectId;
  (void)effectVariant;
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}
