/***************************************************************************//**
 * @file
 * @brief Commands for executing manufacturing-related tests.
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
#include "app/framework/util/af-main.h"
#include "stack/include/mfglib.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/plugin/manufacturing-library-cli/manufacturing-library-cli-plugin.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_BULB_UI_PRESENT
#include "bulb-ui.h"
#endif

// -----------------------------------------------------------------------------
// Globals
enum {
  ACTIVE_COMMAND_NONE = 0x00,
  ACTIVE_COMMAND_TONE = 0x01,
  ACTIVE_COMMAND_STREAM = 0x02,
  ACTIVE_COMMAND_RX_MODE = 0x03
};

enum {
  KICKOFF_IDLE             = 0x00,
  KICKOFF_COMMAND_RECEIVED = 0x01,
  KICKOFF_LEAVING_NETWORK  = 0x02,
  KICKOFF_MFGLIB_ENABLED   = 0x03
};

#define KICKOFF_COMMAND_RECEIVED_WAIT_MS 2000
#define KICKOFF_LEAVINGNETWORK_WAIT_MS   1000
#define KIKOFF_MFGLIB_ENABLED_WAIT_MS    2000
#define TIMEOUT_DELAY_MFGLIB_STOP        1000

static sl_zigbee_af_event_t kickoffEvent;
static void kickoffEventHandler(sl_zigbee_af_event_t * event);
static sl_zigbee_af_event_t timeoutEvent;
static void timeoutEventHandler(sl_zigbee_af_event_t * event);

static uint8_t currentActiveCommand;
static uint8_t savedChannel;
static int8_t  savedPower;
static uint16_t savedTime;
static uint8_t savedEndpoint;
static uint8_t savedOnOff;
static uint8_t kickoffState;

// Forward declarations
void sli_zigbee_af_mfglib_send_command(void);

// Callbacks and functions

void sl_zigbee_af_mfglib_cluster_server_init_cb(int8u endpoint)
{
  sl_zigbee_af_event_init(&kickoffEvent,
                          kickoffEventHandler);
  sl_zigbee_af_event_init(&timeoutEvent,
                          timeoutEventHandler);

  currentActiveCommand = ACTIVE_COMMAND_NONE;
}

static void saveStateLeaveNetwork(uint8_t channel,
                                  int8_t power,
                                  uint16_t time)
{
  savedChannel = channel;
  savedPower = power;
  savedTime = time;
  savedEndpoint = sl_zigbee_af_current_endpoint();

  sl_zigbee_af_read_server_attribute(savedEndpoint,
                                     ZCL_ON_OFF_CLUSTER_ID,
                                     ZCL_ON_OFF_ATTRIBUTE_ID,
                                     (uint8_t *)&savedOnOff,
                                     sizeof(savedOnOff));

  // set up kickoff state machine
  kickoffState = KICKOFF_COMMAND_RECEIVED;
  sl_zigbee_af_event_set_delay_ms(&kickoffEvent,
                                  KICKOFF_COMMAND_RECEIVED_WAIT_MS);
}

bool sl_zigbee_af_mfglib_cluster_tone_cb(uint8_t channel,
                                         int8_t power,
                                         uint16_t time)
{
  currentActiveCommand = ACTIVE_COMMAND_TONE;

  saveStateLeaveNetwork(channel, power, time);

  return TRUE;
}

bool sl_zigbee_af_mfglib_cluster_stream_cb(uint8_t channel,
                                           int8_t power,
                                           uint16_t time)
{
  currentActiveCommand = ACTIVE_COMMAND_STREAM;

  saveStateLeaveNetwork(channel, power, time);

  return TRUE;
}

bool sl_zigbee_af_mfglib_cluster_rx_mode_cb(uint8_t channel,
                                            int8_t power,
                                            uint16_t time)
{
  currentActiveCommand = ACTIVE_COMMAND_RX_MODE;

  saveStateLeaveNetwork(channel, power, time);

  return TRUE;
}

static void mfglibSetupGeneric(void)
{
#ifndef SL_ZIGBEE_TEST
  tokTypeMfgPhyConfig phyConfigToken, phyConfig;
  (void)sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_PHY_CONFIG), (void *)&phyConfigToken, sizeof(tokTypeMfgPhyConfig));
  phyConfig = ~phyConfigToken;
#else
  uint16_t phyConfig = 0;
#endif

  sl_zigbee_af_mfglib_start(true);
  mfglibSetChannel(savedChannel);
  mfglibSetPower(phyConfig, savedPower);
  (void) sl_zigbee_af_write_attribute(savedEndpoint,
                                      ZCL_ON_OFF_CLUSTER_ID,
                                      ZCL_ON_OFF_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      (uint8_t *)&savedOnOff,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

void sl_zigbee_af_manufacturing_library_ota_stack_status_cb(sl_status_t status)
{
  if (status != SL_STATUS_NETWORK_DOWN) {
    return;
  }

  if (currentActiveCommand == ACTIVE_COMMAND_NONE) {
    return;
  }

  sl_zigbee_af_event_set_delay_ms(&kickoffEvent,
                                  KICKOFF_LEAVINGNETWORK_WAIT_MS);
}

static void kickoffEventHandler(sl_zigbee_af_event_t * event)
{
  sl_status_t status;

  sl_zigbee_af_event_set_inactive(
    &kickoffEvent);

  switch (kickoffState) {
    case KICKOFF_IDLE:
      return;
      break;

    case KICKOFF_COMMAND_RECEIVED:
      sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);

      // Note:  I schedule the timeout after the stack status handler call.
      kickoffState = KICKOFF_LEAVING_NETWORK;
      break;

    case KICKOFF_LEAVING_NETWORK:
      mfglibSetupGeneric();
      sl_zigbee_af_event_set_delay_ms(&kickoffEvent,
                                      KIKOFF_MFGLIB_ENABLED_WAIT_MS);
      kickoffState = KICKOFF_MFGLIB_ENABLED;

      break;

    case KICKOFF_MFGLIB_ENABLED:
      switch (currentActiveCommand) {
        case ACTIVE_COMMAND_TONE:
          status = mfglibStartTone();
          sl_zigbee_af_core_println("Tone Start %02X", status);
          break;
        case ACTIVE_COMMAND_STREAM:
          status = mfglibStartStream();
          sl_zigbee_af_core_println("Stream Start %02X", status);
          break;
        case ACTIVE_COMMAND_RX_MODE:
          // no calls required for mfglib for RX mode
          sl_zigbee_af_core_println("RX Mode");
          break;
        default:
          break;
      }

      if (savedTime > 0) {
        // if saved time is 0, we want to enter mfglib mode forever.  If not,
        // we have a legit time, in milliseconds, after which we wish to exit.
        sl_zigbee_af_event_set_delay_ms(
          &timeoutEvent,
          savedTime);
      }

      break;

    default:
      break;
  }
}

static void saveRxStatistics(void)
{
  uint16_t packetsReceived;
  int8_t   savedRssi;
  uint8_t  savedLqi;
  sl_status_t status;

  sl_zigbee_af_mfglib_rx_statistics(&packetsReceived, &savedRssi, &savedLqi);

  status =
    sl_zigbee_af_write_manufacturer_specific_server_attribute(savedEndpoint,
                                                              ZCL_MFGLIB_CLUSTER_ID,
                                                              ZCL_PACKETS_RECEIVED_ATTRIBUTE_ID,
                                                              0x1002,
                                                              (uint8_t *) &packetsReceived,
                                                              ZCL_INT16U_ATTRIBUTE_TYPE);

  status =
    sl_zigbee_af_write_manufacturer_specific_server_attribute(savedEndpoint,
                                                              ZCL_MFGLIB_CLUSTER_ID,
                                                              ZCL_SAVED_RSSI_ATTRIBUTE_ID,
                                                              0x1002,
                                                              (uint8_t *) &savedRssi,
                                                              ZCL_INT8S_ATTRIBUTE_TYPE);

  status =
    sl_zigbee_af_write_manufacturer_specific_server_attribute(savedEndpoint,
                                                              ZCL_MFGLIB_CLUSTER_ID,
                                                              ZCL_SAVED_LQI_ATTRIBUTE_ID,
                                                              0x1002,
                                                              (uint8_t *) &savedLqi,
                                                              ZCL_INT8U_ATTRIBUTE_TYPE);
}

static void timeoutEventHandler(sl_zigbee_af_event_t * event)
{
  sl_zigbee_af_event_set_inactive(&timeoutEvent);

  if (currentActiveCommand == ACTIVE_COMMAND_NONE) {
    sl_zigbee_af_mfglib_stop();
    // Now time to kick off the join process, depending on the relevent UI
    // plugin
    return;
  }

  // Still have an active command.  We need to disable state and kick off a
  // delay to wait for the manufacturing library to stop.
  sl_zigbee_af_event_set_delay_ms(&timeoutEvent, TIMEOUT_DELAY_MFGLIB_STOP);

  if (currentActiveCommand == ACTIVE_COMMAND_TONE) {
    mfglibStopTone();
  }

  if (currentActiveCommand == ACTIVE_COMMAND_STREAM) {
    mfglibStopStream();
  }

  if (currentActiveCommand == ACTIVE_COMMAND_RX_MODE) {
    saveRxStatistics();
  }

  currentActiveCommand = ACTIVE_COMMAND_NONE;
}
