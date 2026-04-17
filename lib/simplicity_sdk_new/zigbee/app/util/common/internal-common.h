/***************************************************************************//**
 * @file
 * @brief declarations for common code for internal apps.
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

void sniffCommand(SL_CLI_COMMAND_ARG);
void statsCommand(SL_CLI_COMMAND_ARG);
void freeBuffersCommand(SL_CLI_COMMAND_ARG);
void neighborTableCommand(SL_CLI_COMMAND_ARG);
void reomveNeighborByNodeIdCommand(SL_CLI_COMMAND_ARG);
void broadcastTableCommand(SL_CLI_COMMAND_ARG);
void nodeIdCommand(SL_CLI_COMMAND_ARG);
void routeTableCommand(SL_CLI_COMMAND_ARG);
void discoveryTableCommand(SL_CLI_COMMAND_ARG);
void childTableCommand(SL_CLI_COMMAND_ARG);
void addressTableCommand(SL_CLI_COMMAND_ARG);
void eraseChildCommand(SL_CLI_COMMAND_ARG);
void eraseChildTableCommand(SL_CLI_COMMAND_ARG);

#define COMMON_INTERNAL_COMMANDS                                                    \
  sl_zigbee_command_entry_action("sniff", sniffCommand, "u",                        \
                                 "Turn on/off Sniffing (no address matching)"),     \
  sl_zigbee_command_entry_action("stats", statsCommand, "",                         \
                                 "Print debug stats"),                              \
  sl_zigbee_command_entry_action("buffers", freeBuffersCommand, "",                 \
                                 "Print number of free buffers"),                   \
  sl_zigbee_command_entry_action("neighbors", neighborTableCommand, "",             \
                                 "Print the neighbor table"),                       \
  sl_zigbee_command_entry_action("rm_neighbor", reomveNeighborByNodeIdCommand, "v", \
                                 "Remove neighbor by node Id"),                     \
  sl_zigbee_command_entry_action("broadcasts", broadcastTableCommand, "",           \
                                 "Print the broadcast table"),                      \
  sl_zigbee_command_entry_action("nodeid", nodeIdCommand, "",                       \
                                 "Print the current node ID"),                      \
  sl_zigbee_command_entry_action("routes", routeTableCommand, "",                   \
                                 "Print the routing table"),                        \
  sl_zigbee_command_entry_action("discoveries", discoveryTableCommand, "",          \
                                 "Print the discovery table"),                      \
  sl_zigbee_command_entry_action("children", childTableCommand, "",                 \
                                 "Print the child table"),                          \
  sl_zigbee_command_entry_action("erase-child", eraseChildCommand, "u",             \
                                 "Erase the child at the specificed index"),        \
  sl_zigbee_command_entry_action("erase-child-table", eraseChildTableCommand, "",   \
                                 "Erase the child table"),                          \
  sl_zigbee_command_entry_action("state", stateCommand, "",                         \
                                 "Print the current state"),                        \
  sl_zigbee_command_entry_action("addr_table", addressTableCommand, "",             \
                                 "Print the address table"),
