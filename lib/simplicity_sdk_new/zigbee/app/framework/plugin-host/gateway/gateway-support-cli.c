/***************************************************************************//**
 * @file
 * @brief Gateway specific CLI behavior for a host application.
 *   In this case we assume our application is running on
 *   a PC with Unix library support, connected to an NCP via serial uart.
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

// common include file
#include "app/framework/util/common.h"

#include <time.h>

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

// The difference in seconds between the ZigBee Epoch: January 1st, 2000
// and the Unix Epoch: January 1st 1970.
#define UNIX_ZIGBEE_EPOCH_DELTA (uint32_t)946684800UL

void sl_zigbee_af_gateway_support_time_sync_local(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  time_t unixTime = time(NULL);
  unixTime -= UNIX_ZIGBEE_EPOCH_DELTA;
  sl_zigbee_af_set_time(unixTime);
  sl_zigbee_af_print_time(sl_zigbee_af_get_current_time());
}
