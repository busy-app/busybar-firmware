/***************************************************************************//**
 * @file
 * @brief CLI for the Identify plugin.
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

// *******************************************************************
// * identify-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "identify.h"

// plugin identify print
void sli_zigbee_af_identify_cli_print(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint8_t i;
  for (i = 0; i < sl_zigbee_af_endpoint_count(); ++i) {
    uint8_t endpoint = sl_zigbee_af_endpoint_from_index(i);
    sl_zigbee_af_identify_cluster_println("Endpoint 0x%02X is identifying: %s",
                                          endpoint,
                                          (sl_zigbee_af_is_device_identifying(endpoint)
                                           ? "true"
                                           : "false"));
  }
}
