/***************************************************************************//**
 * @file
 * @brief
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

void multiprotocolSimStartCommand(void);

#define MULTIPROTOCOL_SIM_COMMANDS                                                                 \
  /* test number, param1, param2 param3 */                                                         \
  sl_zigbee_command_entry_action("multiprotocol_sim_start", multiprotocolSimStartCommand, "uwwww", \
                                 "Start simulating activity on a second protocol"),
