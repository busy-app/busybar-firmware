/***************************************************************************//**
 * @file
 * @brief CLI for the Sub-Ghz Server plugin.
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
#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "stack/include/sl_zigbee_duty_cycle.h"
#include "sub-ghz-server.h"

//-----------------------------------------------------------------------------
// Forward Declarations

//-----------------------------------------------------------------------------
// Globals Declarations

const char * sli_zigbee_af_sub_ghz_server_cli_channel_change_setArguments[] = {
  "Channel change attribute (bit mask)",
  NULL
};

const char * sli_zigbee_af_sub_ghz_server_cli_channel_mask_set_arguments[] = {
  "Channel 28 mask",
  "Channel 29 mask",
  "Channel 30 mask",
  "Channel 31 mask",
  NULL
};

const char * sli_zigbee_af_sub_ghz_server_cli_duty_cycle_limits_set_arguments[] = {
  "'Limited' Duty Cycle state threshold in % * 100 (i.e. 150 = 1.5%, 10000 = 100.00%)",
  "'Critical' Duty Cycle state threshold in % * 100",
  "'Suspended' Duty Cycle state threshold in % * 100",
  NULL
};

const char * sli_zigbee_af_sub_ghz_server_cli_suspend_zcl_messagesArguments[] = {
  "Node ID of the given client",
  "Endpoint of the given client",
  "Period in minutes",
  NULL
};

const char * sli_zigbee_af_sub_ghz_server_cli_client_statusArguments[] = {
  "Node ID of the given client",
  NULL
};

//-----------------------------------------------------------------------------
// Private functions

/** @brief Print "Success" or "Error"
 *
 * Simples.
 */
static void printSuccessOrError(sl_status_t status)
{
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_core_println("OK");
  } else {
    sl_zigbee_af_core_println("Error %d (0x%02X)", status, status);
  }
}

/** @brief Read and print a set of attributes
 *
 * We are lucky that all the attributes are in the server and they are all the same type.
 * It simplifies the implementation.
 */
static void printAttributes(const uint16_t attributeIds[], int n)
{
  const uint8_t endpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
  uint32_t      value;
  uint8_t       type;
  int           i;

  for (i = 0; i < n; ++i) {
    const sl_zigbee_af_status_t status = sl_zigbee_af_read_attribute(endpoint,
                                                                     ZCL_SUB_GHZ_CLUSTER_ID,
                                                                     attributeIds[i],
                                                                     CLUSTER_MASK_SERVER,
                                                                     (uint8_t*)&value,
                                                                     sizeof value,
                                                                     &type);
    if (status == (sl_zigbee_af_status_t)SL_STATUS_OK) {
      sl_zigbee_af_core_println("0x%08X", value);
    } else {
      sl_zigbee_af_core_println("Error reading attribute");
    }
  }
}

/** @brief Write a set of attributes read from the command line
 *
 * All the attributes are in the server and they are all the same type.
 */
static void setAttributes(sl_cli_command_arg_t *arguments, const uint16_t attributeIds[], int n)
{
  const uint8_t endpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
  uint32_t      value;
  int           i;

  for (i = 0; i < n; ++i) {
    value = sl_cli_get_argument_uint32(arguments, i);
    const sl_zigbee_af_status_t status = sl_zigbee_af_write_attribute(endpoint,
                                                                      ZCL_SUB_GHZ_CLUSTER_ID,
                                                                      attributeIds[i],
                                                                      CLUSTER_MASK_SERVER,
                                                                      (uint8_t*)&value,
                                                                      ZCL_BITMAP32_ATTRIBUTE_TYPE);
    if (status != (sl_zigbee_af_status_t)SL_STATUS_OK) {
      sl_zigbee_af_core_println("Error writing attribute");
    }
  }
}

/** @brief Print a value supplied in hectopercent (i.e % * 100)
 *
 * For example, 1.5% = 150 hectopercent, 99.99% = 9999 hectopercent.
 */
static void printHectoPercent(sl_zigbee_duty_cycle_hecto_pct_t value)
{
  sl_zigbee_af_core_print("%d.%d%d%%", value / 100, (value % 100) / 10, value % 10);
}

//-----------------------------------------------------------------------------
// Public functions

/** @brief A set of definition and fuctions to print/set the Sub-GHz cluster's
 * 'Channel Change' attributes.
 */
static const uint16_t ChannelChangeAttribs[] = {
  ZCL_SUB_GHZ_CLUSTER_CHANNEL_CHANGE_ATTRIBUTE_ID
};

void sli_zigbee_af_sub_ghz_server_cli_channel_change_print(sl_cli_command_arg_t *arguments)
{
  printAttributes(ChannelChangeAttribs,
                  sizeof ChannelChangeAttribs / sizeof ChannelChangeAttribs[0]);
}
void sli_zigbee_af_sub_ghz_server_cli_channel_change_set(sl_cli_command_arg_t *arguments)
{
  setAttributes(arguments, ChannelChangeAttribs,
                sizeof ChannelChangeAttribs / sizeof ChannelChangeAttribs[0]);
}

/** @brief A set of definition and fuctions to print/set the Sub-GHz cluster's
 * 'Channel Mask' attributes.
 */
static const uint16_t ChannelMaskAttribs[] = {
  ZCL_SUB_GHZ_CLUSTER_PAGE_28_CHANNEL_MASK_ATTRIBUTE_ID,
  ZCL_SUB_GHZ_CLUSTER_PAGE_29_CHANNEL_MASK_ATTRIBUTE_ID,
  ZCL_SUB_GHZ_CLUSTER_PAGE_30_CHANNEL_MASK_ATTRIBUTE_ID,
  ZCL_SUB_GHZ_CLUSTER_PAGE_31_CHANNEL_MASK_ATTRIBUTE_ID
};
void sli_zigbee_af_sub_ghz_server_cli_channel_mask_print(sl_cli_command_arg_t *arguments)
{
  printAttributes(ChannelMaskAttribs,
                  sizeof ChannelMaskAttribs / sizeof ChannelMaskAttribs[0]);
}
void sli_zigbee_af_sub_ghz_server_cli_channel_mask_set(sl_cli_command_arg_t *arguments)
{
  setAttributes(arguments, ChannelMaskAttribs,
                sizeof ChannelMaskAttribs / sizeof ChannelMaskAttribs[0]);
}

/** @brief Print the current Duty Cycle limits
 */
void sli_zigbee_af_sub_ghz_server_cli_duty_cycle_limits_print(sl_cli_command_arg_t *arguments)
{
  static const char* const dcStateStr[] = {
    "Off",      // SL_ZIGBEE_DUTY_CYCLE_TRACKING_OFF                    = 0,
    "Normal",   // SL_ZIGBEE_DUTY_CYCLE_LBT_NORMAL                      = 1,
    "Limited",  // SL_ZIGBEE_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED   = 2,
    "Critical", // SL_ZIGBEE_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED  = 3,
    "Suspended" // SL_ZIGBEE_DUTY_CYCLE_LBT_SUSPEND_LIMIT_REACHED       = 4,
  };

  sl_zigbee_duty_cycle_limits_t limits;
  sl_zigbee_per_device_duty_cycle_t arrayOfDutyCycles[SL_ZIGBEE_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR + 1];
  sl_zigbee_duty_cycle_state_t dcState;

  if (sl_zigbee_get_duty_cycle_limits(&limits) == SL_STATUS_OK) {
    sl_zigbee_af_core_print("Limited : ");
    printHectoPercent(limits.limitThresh);
    sl_zigbee_af_core_println("");

    sl_zigbee_af_core_print("Critical: ");
    printHectoPercent(limits.critThresh);
    sl_zigbee_af_core_println("");

    sl_zigbee_af_core_print("Suspend : ");
    printHectoPercent(limits.suspLimit);
    sl_zigbee_af_core_println("");
  } else {
    sl_zigbee_af_core_println("Could not get %s%s%s", "", "duty cycle", " limits");
  }

  if (sl_zigbee_get_current_duty_cycle(sizeof arrayOfDutyCycles / sizeof arrayOfDutyCycles[0],
                                       arrayOfDutyCycles) == SL_STATUS_OK) {
    uint8_t i;

    sl_zigbee_af_core_print("Consumed: ");
    printHectoPercent(arrayOfDutyCycles[0].dutyCycleConsumed);
    sl_zigbee_af_core_println("");

    for (i = 1; i < sizeof arrayOfDutyCycles / sizeof arrayOfDutyCycles[0]; i++) {
      sl_zigbee_af_core_print("  (%04X: ", arrayOfDutyCycles[i].nodeId);
      printHectoPercent(arrayOfDutyCycles[i].dutyCycleConsumed);
      sl_zigbee_af_core_println(")");
    }
  } else {
    sl_zigbee_af_core_println("Could not get %s%s%s", "consumed ", "duty cycle", "s");
  }

  if (sl_zigbee_get_duty_cycle_state(&dcState) == SL_STATUS_OK) {
    sl_zigbee_af_core_println("DC state: %s", dcStateStr[dcState]);
  } else {
    sl_zigbee_af_core_println("Could not get %s%s%s", "current ", "duty cycle", " state");
  }
}

/** @brief Set the Duty Cycle limits
 */
void sli_zigbee_af_sub_ghz_server_cli_duty_cycle_limits_set(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_duty_cycle_limits_t limits = { 0 };
  sl_status_t          status;

  limits.limitThresh = sl_cli_get_argument_uint16(arguments, 0);
  limits.critThresh  = sl_cli_get_argument_uint16(arguments, 1);
  limits.suspLimit   = sl_cli_get_argument_uint16(arguments, 2);

  status = sl_zigbee_set_duty_cycle_limits_in_stack(&limits);
  printSuccessOrError(status);
}

/** @brief Send the 'Suspend ZCL Messages' command to a given client.
 *
 * @param nodeId .... client's node ID
 * @param endpoint .. client's endpoint
 * @param period .... suspension perion in minutes
 */
void sli_zigbee_af_sub_ghz_server_cli_suspend_zcl_messages(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;

  sl_802154_short_addr_t nodeId = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t period = sl_cli_get_argument_uint8(arguments, 2);

  status = sl_zigbee_af_sub_ghz_server_send_suspend_zcl_messages_command(nodeId,
                                                                         endpoint,
                                                                         period);
  printSuccessOrError(status);
}

/** @brief Print the given Sub-GHZ given client's current status.
 *
 * @param nodeId .... client's node ID
 */
void sli_zigbee_af_sub_ghz_server_cli_client_status(sl_cli_command_arg_t *arguments)
{
  sl_802154_short_addr_t nodeId = sl_cli_get_argument_uint16(arguments, 0);
  uint16_t t = sl_zigbee_af_sub_ghz_server_suspend_zcl_messages_status(nodeId);

  if (t > 0) {
    sl_zigbee_af_core_println("%04X: %s for %d:%d%d", nodeId, "suspended", t / 60, (t % 60) / 10, t % 10);
  } else {
    sl_zigbee_af_core_println("%04X: not %s", nodeId, "suspended");
  }
}
