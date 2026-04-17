/***************************************************************************//**
 * @file
 * @brief CLI for the Simple Metering Client plugin.
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

#include "../../include/af.h"
#include "../../util/common.h"

#include "sl_cli.h"

void sli_zigbee_af_simple_metering_client_cli_sch_snapshot(sl_cli_command_arg_t *arguments)
{
  uint8_t payload[13];
  sl_802154_short_addr_t dstAddr = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t srcEndpoint =  sl_cli_get_argument_uint8(arguments, 1);
  uint8_t dstEndpoint =  sl_cli_get_argument_uint8(arguments, 2);
  uint32_t issuerId = sl_cli_get_argument_uint32(arguments, 3);
  uint8_t commandIndex = sl_cli_get_argument_uint8(arguments, 4);
  uint8_t numberOfCommands = sl_cli_get_argument_uint8(arguments, 5);
  uint8_t snapshotScheduleId = sl_cli_get_argument_uint8(arguments, 6);
  uint32_t startTime = sl_cli_get_argument_uint32(arguments, 7);
  uint32_t snapshotSchedule = sl_cli_get_argument_uint32(arguments, 8);
  uint8_t snapshotType = sl_cli_get_argument_uint8(arguments, 9);
  uint32_t snapshotCause = sl_cli_get_argument_uint32(arguments, 10);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  payload[0] = snapshotScheduleId;
  sl_zigbee_af_copy_int32u((uint8_t *)payload, 1, startTime);
  sl_zigbee_af_copy_int24u((uint8_t *)payload, 5, snapshotSchedule);
  payload[8] = snapshotType;
  sl_zigbee_af_copy_int32u((uint8_t *)payload, 9, snapshotCause);

  sl_zigbee_af_fill_command_simple_metering_cluster_schedule_snapshot(issuerId,
                                                                      commandIndex,
                                                                      numberOfCommands,
                                                                      payload,
                                                                      13);

  (void)sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);
}

void sli_zigbee_af_simple_metering_client_cli_start_sampling(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t dstAddr = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t srcEndpoint =  sl_cli_get_argument_uint8(arguments, 1);
  uint8_t dstEndpoint =  sl_cli_get_argument_uint8(arguments, 2);
  uint16_t issuerId = sl_cli_get_argument_uint16(arguments, 3);
  uint32_t startTime = sl_cli_get_argument_uint32(arguments, 4);
  uint8_t sampleType = sl_cli_get_argument_uint8(arguments, 5);
  uint16_t sampleRequestInterval = sl_cli_get_argument_uint16(arguments, 6);
  uint16_t maxNumberOfSamples = sl_cli_get_argument_uint16(arguments, 7);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,         \
                                           ZCL_START_SAMPLING_COMMAND_ID,          \
                                           "wwuvv",                                \
                                           issuerId,                               \
                                           startTime,                              \
                                           sampleType,                             \
                                           sampleRequestInterval,                  \
                                           maxNumberOfSamples);
  // sl_zigbee_af_fill_command_simple_metering_cluster_start_sampling(issuerId,
  //                                                      startTime,
  //                                                      sampleType,
  //                                                      sampleRequestInterval,
  //                                                      maxNumberOfSamples);
  (void)sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);
}

void sli_zigbee_af_simple_metering_client_cli_get_sampled_data(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t dstAddr = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t srcEndpoint =  sl_cli_get_argument_uint8(arguments, 1);
  uint8_t dstEndpoint =  sl_cli_get_argument_uint8(arguments, 2);
  uint16_t sampleId = sl_cli_get_argument_uint16(arguments, 3);
  uint32_t startTime = sl_cli_get_argument_uint32(arguments, 4);
  uint8_t sampleType = sl_cli_get_argument_uint8(arguments, 5);
  uint16_t numberOfSamples = sl_cli_get_argument_uint16(arguments, 6);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                            | ZCL_DISABLE_DEFAULT_RESPONSE_MASK), \
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,        \
                                           ZCL_GET_SAMPLED_DATA_COMMAND_ID,       \
                                           "vwuv",                                \
                                           sampleId,                              \
                                           startTime,                             \
                                           sampleType,                            \
                                           numberOfSamples);
  (void)sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);
}

void sli_zigbee_af_simple_metering_client_cli_local_change_supply(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t dstAddr = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t srcEndpoint =  sl_cli_get_argument_uint8(arguments, 1);
  uint8_t dstEndpoint =  sl_cli_get_argument_uint8(arguments, 2);
  uint8_t proposedChange = sl_cli_get_argument_uint8(arguments, 3);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);

  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
                                            | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                            | ZCL_DISABLE_DEFAULT_RESPONSE_MASK), \
                                           ZCL_SIMPLE_METERING_CLUSTER_ID,        \
                                           ZCL_LOCAL_CHANGE_SUPPLY_COMMAND_ID,    \
                                           "u",                                   \
                                           proposedChange);

  (void)sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);
}
