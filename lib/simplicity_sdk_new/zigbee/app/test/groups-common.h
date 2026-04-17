/***************************************************************************//**
 * @file
 * @brief Functionality for manipulating APS/NWK groups that is common to all
 * platforms (250, 260, 2420).
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

void displayGroupsCommand(SL_CLI_COMMAND_ARG);
void clearGroupsCommand(SL_CLI_COMMAND_ARG);
void addEndpointToGroupCommand(SL_CLI_COMMAND_ARG);
void removeEndpointFromGroupCommand(SL_CLI_COMMAND_ARG);

// Platform specific implementations
uint8_t getGroupTableSize(void);           // returns 0xFF on error
bool initializeGroupsTable(void);
bool eraseGroup(uint8_t index);
bool getGroup(uint8_t index, sl_zigbee_multicast_table_entry_t* returnData);
bool setGroup(uint8_t index, sl_zigbee_multicast_table_entry_t* data);

//------------------------------------------------------------------------------

#define GROUPS_COMMON_COMMANDS                                                         \
  /* Displaying, adding, and removing endpoints to group mappings */                   \
  sl_zigbee_command_entry_action("groups", displayGroupsCommand, "",                   \
                                 "Display APS groups"),                                \
  sl_zigbee_command_entry_action("add_endpoint", addEndpointToGroupCommand, "uv",      \
                                 "Add endpoint to the specified group"),               \
  sl_zigbee_command_entry_action("del_endpoint", removeEndpointFromGroupCommand, "uv", \
                                 "Delete an endpoint from specified group"),           \
  sl_zigbee_command_entry_action("clear_groups", clearGroupsCommand, "",               \
                                 "Clear all groups"),
