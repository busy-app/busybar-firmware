/***************************************************************************//**
 * @file
 * @brief CLI for the BDB 3.1 Rejoin Algorithm plugin.
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
#include "zigbee-bdb-3dot1-rejoin-algorithm.h"

extern sl_zigbee_af_bdb_rejoin_state_t bdb_rejoin_state;

void sl_zigbee_af_bdb_3dot1_rejoin_algorithm_enable_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  bdb_rejoin_state.enabled = (bool)sl_cli_get_argument_uint8(arguments, 0);
}

void sl_zigbee_af_bdb_3dot1_rejoin_algorithm_status_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_core_println("BDB Rejoin Algorithm Status");
  sl_zigbee_af_core_println("Enabled:                      %s", bdb_rejoin_state.enabled ? "y" : "n");
  sl_zigbee_af_core_println("State:                        %d", bdb_rejoin_state.state);
  sl_zigbee_af_core_println("TC Keepalive failure:         %s", bdb_rejoin_state.tc_keepalive_failure ? "y" : "n");
  sl_zigbee_af_core_println("Poll Control checkin failure: %s", bdb_rejoin_state.poll_control_failure ? "y" : "n");
  sl_zigbee_af_core_println("Rejoins attempted:            %d", bdb_rejoin_state.rejoin_attempts);
}
