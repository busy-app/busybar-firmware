/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Commissioning Network plugin.
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
#include "app/util/common/form-and-join.h"
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"

#include "zigbee_stack_callback_dispatcher.h"
#include "zll-commissioning-network-config.h"

#define WAITING_BIT 0x80

enum {
  ZLL_NETWORK_INITIAL= 0x00,
  UNUSED_PRIMARY     = 0x01,
  JOINABLE_PRIMARY   = 0x02,
  JOINABLE_SECONDARY = 0x03,
  WAITING_PRIMARY    = WAITING_BIT | JOINABLE_PRIMARY,
  WAITING_SECONDARY  = WAITING_BIT | JOINABLE_SECONDARY,
};
static uint8_t state = ZLL_NETWORK_INITIAL;
static sl_zigbee_af_event_t networkEvent;
static void networkEventHandler(sl_zigbee_af_event_t * event);
uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE] = SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_NETWORK_EXTENDED_PAN_ID;

// The exponent of the number of scan periods, where a scan period is 960
// symbols, and a symbol is 16 miscroseconds.  The scan will occur for
// ((2^duration) + 1) scan periods.  The ZLL specification requires routers to
// scan for joinable networks using a duration of 4.
#define SCAN_DURATION 4

// Private ZLL commissioning functions
sl_zigbee_node_type_t sli_zigbee_af_zll_get_logical_node_type(void);
sl_status_t sli_zigbee_af_zll_form_network(uint8_t channel, int8_t power, sl_802154_pan_id_t panId);

void sli_zigbee_af_zll_commissioning_network_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(&networkEvent,
                          networkEventHandler);
}
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_af_zll_scan_for_joinable_network(void)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
  if (state == ZLL_NETWORK_INITIAL) {
    // TODO: The scan duration is hardcoded to 3 for the joinable part of the
    // form and join library, but ZLL specifies the duration should be 4.
    status = sl_zigbee_scan_for_joinable_network(sl_zigbee_get_zll_primary_channel_mask(),
                                                 extendedPanId);
    if (status == SL_STATUS_OK) {
      state = JOINABLE_PRIMARY;
    } else {
      sl_zigbee_af_app_println("Error: %s: 0x%02X",
                               "could not scan for joinable network",
                               status);
    }
  }
  return status;
}

sl_status_t sl_zigbee_af_zll_scan_for_unused_pan_id(void)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
  if (state == ZLL_NETWORK_INITIAL) {
    status = sl_zigbee_scan_for_unused_pan_id(sl_zigbee_get_zll_primary_channel_mask(),
                                              SCAN_DURATION);
    if (status == SL_STATUS_OK) {
      state = UNUSED_PRIMARY;
    } else {
      sl_zigbee_af_app_println("Error: %s: 0x%02X",
                               "could not scan for unused network",
                               status);
    }
  }
  return status;
}

void sl_zigbee_af_joinable_network_found_cb(sl_zigbee_zigbee_network_t *networkFound,
                                            uint8_t lqi,
                                            int8_t rssi)
{
  sl_status_t status = SL_STATUS_FAIL;
  if (sl_zigbee_af_zll_commissioning_network_join_cb(networkFound, lqi, rssi)) {
    sl_zigbee_network_parameters_t parameters;
    memset(&parameters, 0, sizeof(sl_zigbee_network_parameters_t));
    memmove(parameters.extendedPanId,
            networkFound->extendedPanId,
            EXTENDED_PAN_ID_SIZE);
    parameters.panId = networkFound->panId;
    parameters.radioTxPower = SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER;
    parameters.radioChannel = networkFound->channel;

    sl_zigbee_af_zll_set_initial_security_state();
    status = sl_zigbee_af_join_network(&parameters);
  }

  // Note: If the application wants to skip this network or if the join fails,
  // we cannot call sl_zigbee_scan_for_next_joinable_network to continue the scan
  // because we would be recursing.  Instead, schedule an event to fire at the
  // next tick and restart from there.
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("Error: %s: 0x%02X", "could not join network", status);
    sl_zigbee_af_event_set_active(&networkEvent);
  }
}

void sl_zigbee_af_form_and_join_network_found_cb(sl_zigbee_zigbee_network_t *networkFound,
                                                 uint8_t lqi,
                                                 int8_t rssi)
{
  sl_zigbee_af_joinable_network_found_cb(networkFound, lqi, rssi);
}

void sli_zigbee_af_zll_commissioning_network_unused_pan_id_found_callback(sl_802154_pan_id_t panId, uint8_t channel)
{
  sl_status_t status = sli_zigbee_af_zll_form_network(channel,
                                                      SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_COMMON_RADIO_TX_POWER,
                                                      panId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("Error: %s: 0x%02X", "could not form network", status);
    sli_zigbee_af_zll_commissioning_network_scan_error_callback(status);
  }
}

void sl_zigbee_af_form_and_join_unused_pan_id_found_cb(sl_802154_pan_id_t panId, uint8_t channel)
{
  sli_zigbee_af_zll_commissioning_network_unused_pan_id_found_callback(panId, channel);
}

void sli_zigbee_af_zll_commissioning_network_scan_error_callback(sl_status_t status)
{
#ifdef SCAN_SECONDARY_CHANNELS
  if (status == SL_STATUS_NO_BEACONS
      && state == JOINABLE_PRIMARY
      && sl_zigbee_get_zll_secondary_channel_mask() != 0) {
    state = JOINABLE_SECONDARY;
    sl_zigbee_af_event_set_active(&networkEvent);
    return;
  }
#endif
  sl_zigbee_af_app_println("Error: %s: 0x%02X",
                           (state == UNUSED_PRIMARY
                            ? "could not find unused network"
                            : "could not find joinable network"),
                           status);
  state = ZLL_NETWORK_INITIAL;
}

void sl_zigbee_af_get_form_and_join_extended_pan_id_cb(uint8_t *resultLocation)
{
  memmove(resultLocation, extendedPanId, EXTENDED_PAN_ID_SIZE);
}

void sl_zigbee_af_set_form_and_join_extended_pan_id_cb(const uint8_t *extPanId)
{
  // Note, this function will also be used by the 'network extpanid' CLI command.
  memmove(extendedPanId, extPanId, EXTENDED_PAN_ID_SIZE);
}

static void networkEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  sl_status_t status = SL_STATUS_FAIL;
  sl_zigbee_af_event_set_inactive(&networkEvent);
  if ((state == JOINABLE_PRIMARY || state == JOINABLE_SECONDARY)
      && sl_zigbee_form_and_join_can_continue_joinable_network_scan()) {
    status = sl_zigbee_scan_for_next_joinable_network();
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_app_println("Error: %s: 0x%02X",
                               "could not continue scan for joinable network",
                               status);
    }
#ifdef SCAN_SECONDARY_CHANNELS
  } else if (state == JOINABLE_SECONDARY) {
    // TODO: The scan duration is hardcoded to 3 for the joinable part of the
    // form and join library, but ZLL specifies the duration should be 4.
    status = sl_zigbee_scan_for_joinable_network(sl_zigbee_get_zll_secondary_channel_mask(),
                                                 extendedPanId);
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_app_println("Error: %s: 0x%02X",
                               "could not scan for joinable network",
                               status);
    }
#endif
  }

  if (status != SL_STATUS_OK) {
    state = ZLL_NETWORK_INITIAL;
    sl_zigbee_form_and_join_cleanup(SL_STATUS_OK);
  }
}

void sli_zigbee_af_zll_commissioning_network_stack_status_callback(sl_status_t status)
{
  uint8_t delayMinutes = MAX_INT8U_VALUE;

  if (status == SL_STATUS_NETWORK_UP) {
    if (state == UNUSED_PRIMARY) {
      delayMinutes = 0;
    } else if (state == JOINABLE_PRIMARY || state == JOINABLE_SECONDARY) {
      // The ZLL state is now updated in emberAfPluginZllCommissioningCommonStackStatusCallback():
      // this callback just takes care of the network module state.
      state |= WAITING_BIT; // JOINABLE_XXX to WAITING_XXX
      delayMinutes = SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_NETWORK_JOINABLE_SCAN_TIMEOUT_MINUTES;
    }
  } else if (JOINABLE_PRIMARY <= state) {
    state &= ~WAITING_BIT; // JOINABLE_XXX or WAITING_XXX to JOINABLE_XXX
    delayMinutes = 0;
  }

  if (delayMinutes == 0) {
    networkEventHandler(&networkEvent);
  } else if (delayMinutes != MAX_INT8U_VALUE) {
    sl_zigbee_af_event_set_delay_minutes(&networkEvent,
                                         delayMinutes);
  }
}
