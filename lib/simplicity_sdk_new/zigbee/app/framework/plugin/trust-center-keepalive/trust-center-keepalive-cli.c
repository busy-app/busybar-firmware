/***************************************************************************//**
 * @file
 * @brief CLI for the TC Keepalive plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "trust-center-keepalive.h"

void sl_zigbee_af_send_keepalive_now_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  (void)sli_zigbee_af_send_keepalive_now();
}

void sl_zigbee_af_send_keepalive_halt_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_trust_center_keepalive_disable();
}
