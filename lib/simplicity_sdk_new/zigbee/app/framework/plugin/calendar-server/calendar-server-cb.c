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

/** @brief Publish Info
 *
 * This function is called by the calendar-server plugin after receiving any of
 * the following commands and just before it starts publishing the response:
 * GetCalendar, GetDayProfiles, GetSeasons, GetSpecialDays, and
 * GetWeekProfiles.
 *
 * @param publishCommandId ZCL command to be published Ver.: always
 * @param clientNodeId Destination nodeId Ver.: always
 * @param clientEndpoint Destination endpoint Ver.: always
 * @param totalCommands Total number of publish commands to be sent
 * Ver.: always
 */
WEAK(void sl_zigbee_af_calendar_server_publish_info_cb(uint8_t publishCommandId,
                                                       sl_802154_short_addr_t clientNodeId,
                                                       uint8_t clientEndpoint,
                                                       uint8_t totalCommands))
{
  UNUSED_VAR(publishCommandId);
  UNUSED_VAR(clientNodeId);
  UNUSED_VAR(clientEndpoint);
  UNUSED_VAR(totalCommands);
}
