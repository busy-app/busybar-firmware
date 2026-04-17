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

/** @brief CheckInTimeout
 *
 * This function is called by the Poll Control Server plugin after a threshold value of poll control
 * check in messages are sent to a trust center and no responses are received. The existence of this
 * callback provides an opportunity for the application to implement its own rejoin algorithm or logic.
 */
WEAK(void sl_zigbee_af_poll_control_server_check_in_timeout_cb(void))
{
  return;
}
