/***************************************************************************//**
 * @file
 * @brief Routines for the Groups Server plugin, the server implementation of
 *        the Groups cluster.
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
#include "app/framework/plugin/groups-server/groups-server.h"

/** @brief Get Group Name
 *
 * This function returns the name of a group with the provided group ID, should
 * it exist.
 *
 * @param endpoint Endpoint Ver.: always
 * @param groupId Group ID Ver.: always
 * @param groupName Group Name Ver.: always
 */
WEAK(void sl_zigbee_af_groups_server_get_group_name_cb(uint8_t endpoint,
                                                       uint16_t groupId,
                                                       uint8_t *groupName))
{
  (void)endpoint;
  (void)groupId;
  (void)groupName;
}

/** @brief Set Group Name
 *
 * This function sets the name of a group with the provided group ID.
 *
 * @param endpoint Endpoint Ver.: always
 * @param groupId Group ID Ver.: always
 * @param groupName Group Name Ver.: always
 */
WEAK(void sl_zigbee_af_groups_server_set_group_name_cb(uint8_t endpoint,
                                                       uint16_t groupId,
                                                       uint8_t *groupName))
{
  (void)endpoint;
  (void)groupId;
  (void)groupName;
}

/** @brief Group Names Supported
 *
 * This function returns whether or not group names are supported.
 *
 * @param endpoint Endpoint Ver.: always
 */
WEAK(bool sl_zigbee_af_groups_server_group_names_supported_cb(uint8_t endpoint))
{
  (void)endpoint;
  return false;
}
