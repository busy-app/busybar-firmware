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
#include "gas-proxy-function.h"
#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"

/** @brief Non Tap Off Message Handler Completed
 *
 * Notify the application that the Non Tap Off Message (Non-TOM) have been
 * handled by the mirror endpoint. This callback is invoked when the the Non
 * Tap Off Message have been processed. The first argument is a pointer to the
 * gbz commands response buffer. The application is responsible for freeing
 * this buffer.
 *
 * @param gbzCommandsResponse   Ver.: always
 * @param gbzCommandsResponseLength   Ver.: always
 */
WEAK(void sl_zigbee_af_gas_proxy_function_non_tap_off_message_handler_completed_cb(uint8_t *gbzCommandsResponse,
                                                                                   uint16_t gbzCommandsResponseLength))
{
  sl_zigbee_af_app_println("GBCS sl_zigbee_af_gas_proxy_function_non_tap_off_message_handler_completed_cb: gbzCommandsResponseLength=0x%04X", gbzCommandsResponseLength);
  free(gbzCommandsResponse);
  gbzCommandsResponse = NULL;
}

/** @brief Alert WAN
 *
 * Notify the application of an Alert that should be sent to the WAN. The
 * second argument is a pointer to the gbz alert buffer. The application is
 * responsible for freeing this buffer.
 *
 * @param alertCode The 16 bit allert code as defined by GBCS Ver.: always
 * @param gbzAlert Buffer continaing the GBZ formatted Alert Ver.: always
 * @param gbzAlertLength The length in octets of the GBZ formatted Alert
 * Ver.: always
 */
WEAK(void sl_zigbee_af_gas_proxy_function_alert_w_a_n_cb(uint16_t alertCode,
                                                         uint8_t *gbzAlert,
                                                         uint16_t gbzAlertLength))
{
  sl_zigbee_af_app_println("GBCS sl_zigbee_af_gas_proxy_function_alert_w_a_n_cb: alertCode=0x%04X, gbzAlertLength=0x%04X", alertCode, gbzAlertLength);
  free(gbzAlert);
  gbzAlert = NULL;
}

/** @brief Tap Off Message Future Command Ignored
 *
 * Notify the application that an elemental ZCL command, embedded within the
 * Tap Off Message (TOM), has been ignored by the GPF as it is a future dated
 * command. The application can utilize the callback to store the discarded
 * command for later retrieval. When the future dated command becomes active on
 * the GSME, it will inform the CommsHub about the activation via an Alert
 * message. The CommsHub should be able to correlate the matching messages and
 * pass the previously stored ZCL commands to the TOM message handler for
 * processing.
 *
 * @param gpfMessage   Ver.: always
 * @param zclClusterCommand   Ver.: always
 */
WEAK(void sl_zigbee_af_gas_proxy_function_tap_off_message_future_command_ignored_cb(const sl_zigbee_af_gpf_message_t * gpfMessage,
                                                                                    const sl_zigbee_af_cluster_command_t * zclClusterCommand))
{
  UNUSED_VAR(gpfMessage);
  UNUSED_VAR(zclClusterCommand);
}

/** @brief Data Log Access Request
 *
 * Query the application regarding the processing of the current GBCS specific
 * command. Depending on the vendor specific information, such as the Tenancy
 * attribute in the Device Management Cluster, the callback's return value
 * determines whether the current command gets processed or not. With a return
 * value of true, the plugin will process the message. Otherwise, the plugin
 * will reject the message.
 *
 * @param gpfMessage   Ver.: always
 * @param zclClusterCommand   Ver.: always
 */
WEAK(bool sl_zigbee_af_gas_proxy_function_data_log_access_request_cb(const sl_zigbee_af_gpf_message_t * gpfMessage,
                                                                     const sl_zigbee_af_cluster_command_t * zclClusterCommand))
{
  UNUSED_VAR(gpfMessage);
  UNUSED_VAR(zclClusterCommand);

  return true;
}

/** @brief Validate Incoming Zcl Command
 *
 * Query the application to check if a specific ZCL command should be processed
 * given the UseCase # and ZCL command payload information. This function
 * should capture the behavior detailed by the GBCS Use Case documentation.
 * Please refer to sl_zigbee_af_gpf_zcl_command_permission_t enum for specific return data
 * types.
 *
 * @param cmd   Ver.: always
 * @param messageCode   Ver.: always
 */
WEAK(sl_zigbee_af_gpf_zcl_command_permission_t sl_zigbee_af_gas_proxy_function_validate_incoming_zcl_command_cb(const sl_zigbee_af_cluster_command_t * cmd,
                                                                                                                uint16_t messageCode))
{
  if ((messageCode == TEST_ENCRYPTED_MESSAGE_CODE)
      || (messageCode == TEST_MESSAGE_CODE)) {
    return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED;
  }

  if (cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT) {
    if (cmd->apsFrame->clusterId == ZCL_CALENDAR_CLUSTER_ID) {
      if ((cmd->commandId == ZCL_PUBLISH_CALENDAR_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_DAY_PROFILE_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_WEEK_PROFILE_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_SEASONS_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_SPECIAL_DAYS_COMMAND_ID)) {
        return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED;
      }
    } else if (cmd->apsFrame->clusterId == ZCL_PRICE_CLUSTER_ID) {
      if ((cmd->commandId == ZCL_PUBLISH_TARIFF_INFORMATION_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_BLOCK_THRESHOLDS_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_PRICE_MATRIX_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_CONVERSION_FACTOR_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_CALORIFIC_VALUE_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID)
          || (cmd->commandId == ZCL_PUBLISH_BLOCK_PERIOD_COMMAND_ID)) {
        return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED;
      }
    } else if (cmd->apsFrame->clusterId == ZCL_MESSAGING_CLUSTER_ID) {
      if (cmd->commandId == ZCL_DISPLAY_MESSAGE_COMMAND_ID) {
        return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED;
      }
    } else if (cmd->apsFrame->clusterId == ZCL_DEVICE_MANAGEMENT_CLUSTER_ID) {
      if (cmd->commandId == ZCL_PUBLISH_CHANGE_OF_SUPPLIER_COMMAND_ID) {
        return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED;
      }
    }
  } else {   // client to server
    if (cmd->apsFrame->clusterId == ZCL_EVENTS_CLUSTER_ID) {
      if (cmd->commandId == ZCL_CLEAR_EVENT_LOG_REQUEST_COMMAND_ID) {
        return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_ALLOWED;
      }
    } else if (cmd->apsFrame->clusterId == ZCL_PREPAYMENT_CLUSTER_ID) {
      if ((cmd->commandId == ZCL_EMERGENCY_CREDIT_SETUP_COMMAND_ID)
          || (cmd->commandId == ZCL_SET_OVERALL_DEBT_CAP_COMMAND_ID)
          || (cmd->commandId == ZCL_SET_LOW_CREDIT_WARNING_LEVEL_COMMAND_ID)
          || (cmd->commandId == ZCL_SET_MAXIMUM_CREDIT_LIMIT_COMMAND_ID)) {
        return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_IGNORED;
      }
    }
  }

  return SL_ZIGBEE_AF_GPF_ZCL_COMMAND_PERMISSION_NOT_ALLOWED;
}

/** @brief Unknown Season Week Id
 *
 * This callback is called when the GPF receives a PublishSeasons command with
 * an unknown week ID in one of the seasons. The callback contains a mask,
 * unknownWeekIdSeasonsMask, with bits set corresponding to season entries that
 * were not published because of an unknown week ID reference.
 *
 * @param issuerCalendarId The calendar ID associated with these seasons.
 * Ver.: always
 * @param seasonEntries The season entries for this publish command.
 * Ver.: always
 * @param seasonEntriesLength The length of the season entries. Ver.: always
 * @param unknownWeekIdSeasonsMask A mask where bit N indicates that the Nth
 * season entry contains an unknown week ID reference. Ver.: always
 */
WEAK(void sl_zigbee_af_gas_proxy_function_unknown_season_week_id_cb(uint32_t issuerCalendarId,
                                                                    uint8_t *seasonEntries,
                                                                    uint8_t seasonEntriesLength,
                                                                    uint8_t unknownWeekIdSeasonsMask))
{
  UNUSED_VAR(issuerCalendarId);
  UNUSED_VAR(seasonEntries);
  UNUSED_VAR(seasonEntriesLength);

  sl_zigbee_af_app_println("GBCS: sl_zigbee_af_gas_proxy_function_unknown_season_week_id_cb: 0x%02X", unknownWeekIdSeasonsMask);
}

/** @brief Unknown Special Days Day Id
 *
 * This callback is called when the GPF receives a PublishSpecialDays command
 * with an unknown day ID for one of the special days. The callback contains a
 * mask, unknownDayIdMask, with bits set corresponding to special day entries
 * that were not published because of an unknown day ID reference.
 *
 * @param issuerCalendarId The calendar ID associated with these special days.
 * Ver.: always
 * @param specialDayEntries The special day entries for this publish command.
 * Ver.: always
 * @param specialDayEntriesLength The length of the special day entries.
 * Ver.: always
 * @param unknownDayIdMask A mask where bit N indicates that the Nth special
 * day entry contains an unknown day ID reference. Ver.: always
 */
WEAK(void sl_zigbee_af_gas_proxy_function_unknown_special_days_day_id_cb(uint32_t issuerCalendarId,
                                                                         uint8_t *specialDayEntries,
                                                                         uint8_t specialDayEntriesLength,
                                                                         uint8_t unknownDayIdMask))
{
  UNUSED_VAR(issuerCalendarId);
  UNUSED_VAR(specialDayEntries);
  UNUSED_VAR(specialDayEntriesLength);

  sl_zigbee_af_app_println("GBCS: sl_zigbee_af_gas_proxy_function_unknown_special_days_day_id_cb: 0x%02X", unknownDayIdMask);
}
