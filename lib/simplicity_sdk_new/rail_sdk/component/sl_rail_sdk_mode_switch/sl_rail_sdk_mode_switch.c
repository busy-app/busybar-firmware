/***************************************************************************//**
 * @file sl_rail_sdk_mode_switch.c
 * @brief RAIL Wi-SUN Mode Switch Component
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "sl_component_catalog.h"
#include "sl_rail.h"
#include "sl_rail_util_init.h"
#include "app_init.h"
#include "sl_simple_led_instances.h"
#include "app_log.h"
#include "sl_rail_util_init_inst0_config.h"
#include "rail_config.h"
#include "app_assert.h"
#include "sl_rail_sdk_mode_switch.h"
#include "sl_rail_sdk_packet_asm.h"
#include "sl_sleeptimer.h"
#include "sl_rail_sdk_mode_switch.h"
#include "sl_status.h"
#include "sl_rail_sdk_fifo_size_config.h"
#include "sl_rail_ieee802154.h"
#include "sl_code_classification.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "app_task_init.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define MSPHR_LENGTH 2U
#define MS_PACKET_LENGTH 18U

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// List of detected channel numbers
radio_info_t radio_info = {
  .mode_switch_capable_channels = 0U,
  .channel_list = { { 0U } }
};

const char* mode_switch_state_strings[] = {
  "MS_IDLE",
  "MS_REQUESTED",
  "MS_INITIATED",
  "MS_SENDING_MS_START_PACKET",
  "MS_ON_NEW_PHY",
  "MS_RETURN_TO_BASE_PHY",
  "MS_SENDING_MS_END_PACKET"
};

const char* phy_modulation_strings[] = {
  "M_OFDM",
  "M_OQPSK",
  "M_BPSK",
  "M_GFSK",
  "M_2FSK",
  "M_UNDEFINED"
};

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Mode switch packet header
static uint8_t ms_phr[MSPHR_LENGTH] = { 0, 0 };
/// Current state of the mode switch process
static volatile mode_switch_state_t ms_state = MS_IDLE;
/// Phy mode id to switch
static volatile uint8_t ms_new_phy_mode_id = 0xFFU;
/// Radio channel number after mode switch
volatile uint16_t ms_new_channel = 0xFFFFU;
/// The channel used for the upcoming transmission
static volatile uint16_t current_channel = 0U;
/// The channel the device returns to from mode switch
static volatile uint16_t base_channel = 0U;
/// Buffer of the packet to be sent
static uint8_t tx_frame_buffer[SL_RAIL_SDK_TX_FIFO_SIZE];
/// The time in seconds that the device is in the new phy during mode switch
static volatile uint32_t ms_duration = 0U;
/// Timer for the mode switch process
static sl_sleeptimer_timer_handle_t mode_switch_timer;
/// Radio power
static sl_rail_tx_power_t power = 140U;
/// WiSUN FSK packet FCS is on/off
static uint8_t fsk_fcs_type = 0U;
/// WiSUN FSK packet whitening is on/off
static uint8_t fsk_whitening = 1U;
/// WiSUN OFDM, rate specifies the data rate of the payload and is equal to the numerical value of the MCS
static uint8_t ofdm_rate = 0x00;
// rate: 5 bits wide,
//  The Rate field (RA4-RA0) specifies the data rate of the payload
//  and is equal to the numerical value of the MCS
// 0x0 BPSK, coding rate 1/2, 4 x frequency repetition
// 0x1 BPSK, coding rate 1/2, 2 x frequency repetition
// 0x2 QPSK, coding rate 1/2, 2 x frequency repetition
// 0x3 QPSK, coding rate 1/2
// 0x4 QPSK, coding rate 3/4
// 0x5 16-QAM, coding rate 1/2
// 0x6 16-QAM, coding rate 3/4
/// WiSUN OFDM, the scrambler field specifies the scrambling seed
static uint8_t ofdm_scrambler = 0x00;
/// Enable to print extra information about the received or sent packets
static bool print_packet_details = true;

/// A configuration structure for IEEE 802.15.4 in RAIL.
static const sl_rail_ieee802154_config_t config = {
  .p_addresses = NULL,
  .ack_config = {
    .enable = true,
    .ack_timeout_us = 672,
    .rx_transitions = {
      .success = SL_RAIL_RF_STATE_RX,
      .error = SL_RAIL_RF_STATE_RX
    },
    .tx_transitions = {
      .success = SL_RAIL_RF_STATE_RX,
      .error = SL_RAIL_RF_STATE_RX
    }
  },
  .timings = {
    .idle_to_tx = 110,
    .idle_to_rx = 110,
    .rx_to_tx = 192,
    // Make tx_to_rx slightly lower than desired to make sure we get to RX in time
    .tx_to_rx = 182,
    .rxsearch_timeout = 0,
    .tx_to_rxsearch_timeout = 0,
    .tx_to_tx = 0
  },
  .frames_mask = SL_RAIL_IEEE802154_ACCEPT_STANDARD_FRAMES,
  .promiscuous_mode = false,
  .is_pan_coordinator = false,
  .default_frame_pending_in_outgoing_acks = false
};

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
* Callback function for the mode switch timer
*
* @param[in] handle Pointer to the sleeptimer handle
* @param[in] data Pointer to delay flag
******************************************************************************/
SL_CODE_RAM static void ms_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * This function gets the static fcs_type value.
 *****************************************************************************/
uint8_t get_fsk_fcs_type(void)
{
  return fsk_fcs_type;
}

/******************************************************************************
 * This function sets the static fcs_type value.
 *****************************************************************************/
void set_fsk_fcs_type(uint8_t new_fsk_fcs_type)
{
  if (new_fsk_fcs_type < 2) {
    fsk_fcs_type = new_fsk_fcs_type;
  } else {
    app_log_warning("Set FSK type: %d is higher then 0x01\n", new_fsk_fcs_type);
  }
}

/******************************************************************************
 * This function gets the static whitening value.
 *****************************************************************************/
uint8_t get_fsk_whitening(void)
{
  return fsk_whitening;
}

/******************************************************************************
 * This function sets the static fsk_whitening value.
 *****************************************************************************/
void set_fsk_whitening(uint8_t new_fsk_whitening)
{
  if (new_fsk_whitening < 2) {
    fsk_whitening = new_fsk_whitening;
  } else {
    app_log_warning("Set FSK whitening: %d is higher then 0x01\n", new_fsk_whitening);
  }
}

/******************************************************************************
 * This function gets the static rate value.
 *****************************************************************************/
uint8_t get_ofdm_rate(void)
{
  return ofdm_rate;
}

/******************************************************************************
 * This function sets the static rate value.
 *****************************************************************************/
void set_ofdm_rate(uint8_t new_ofdm_rate)
{
  if (new_ofdm_rate < 7) {
    ofdm_rate = new_ofdm_rate;
    if (get_phy_modulation_from_channel(current_channel) == M_OFDM) {
      ms_new_phy_mode_id = get_phy_mode_id_from_channel(current_channel);
      ms_new_phy_mode_id = ms_new_phy_mode_id + ofdm_rate;
    }
  } else {
    app_log_warning("Set OFDM rate: %d is higher then 0x06\n", new_ofdm_rate);
  }
}

/******************************************************************************
 * This function gets the static scrambler value.
 *****************************************************************************/
uint8_t get_ofdm_scrambler(void)
{
  return ofdm_scrambler;
}

/******************************************************************************
 * This function sets the static scrambler value.
 *****************************************************************************/
void set_ofdm_scrambler(uint8_t new_ofdm_scrambler)
{
  ofdm_scrambler = new_ofdm_scrambler;
}

/******************************************************************************
 * This function gets the static print_packet_details value.
 *****************************************************************************/
bool get_print_packet_details(void)
{
  return print_packet_details;
}

/******************************************************************************
 * This function sets the static print_packet_details value.
 *****************************************************************************/
void set_print_packet_details(bool new_print_packet_details)
{
  print_packet_details = new_print_packet_details;
}

/******************************************************************************
 * This function calibrates the radio.
 *****************************************************************************/
void calibrate_radio(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "calibrate_radio error: NULL handle\n");

  // Calibration on OFDM is sufficient for FSK as well
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  sl_rail_antenna_sel_t rf_path = SL_RAIL_ANTENNA_AUTO;

  // Finding the first channel with OFDM modulation
  for (uint8_t i = 0; i < radio_info.mode_switch_capable_channels; i++) {
    if (radio_info.channel_list[i].modulation == M_OFDM) {
      current_channel = radio_info.channel_list[i].channel_number;
      break;
    }
  }

  sl_rail_start_rx(rail_handle, current_channel, NULL);
  status = sl_rail_get_rf_path(rail_handle, &rf_path);

  if (status == SL_RAIL_STATUS_NO_ERROR) {
    sl_rail_idle(rail_handle, SL_RAIL_IDLE_ABORT, false);
    status = sl_rail_calibrate_ir(rail_handle, NULL, rf_path);
    if (status == SL_RAIL_STATUS_NO_ERROR) {
      app_log_info("IR calibration OK\n");
    } else {
      app_log_warning("IR calibration ERROR: %lu\n", status);
    }
  }
}

/******************************************************************************
 * API to init and set the IEEE802154 hardware acceleration.
 *****************************************************************************/
void init_ieee802154_for_mode_switch(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "init_ieee802154_for_mode_switch error: NULL handle\n");

  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

  status = sl_rail_ieee802154_init(rail_handle, &config);
  app_assert(status == SL_RAIL_STATUS_NO_ERROR,
             "sl_rail_ieee802154_init error: %lu\n",
             status);
}

/******************************************************************************
 * API to enable DUALSYNC in Tx and Rx in case of FSK FEC usage.
 *****************************************************************************/
void init_rx_option_for_mode_switch(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "init_rx_option_for_mode_switch error: NULL handle\n");

  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  sl_rail_rx_options_t enable_dualsync = SL_RAIL_RX_OPTION_ENABLE_DUAL_SYNC;

  status = sl_rail_config_rx_options(rail_handle,
                                     SL_RAIL_RX_OPTIONS_ALL,
                                     enable_dualsync);
  app_assert(status == SL_RAIL_STATUS_NO_ERROR,
             "sl_rail_config_rx_options error: %lu\n",
             status);
}

/******************************************************************************
 * API to enable Mode Switch and dynamic FEC.
 *****************************************************************************/
void init_ieee802154g_option_for_mode_switch(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "init_ieee802154g_option_for_mode_switch error: NULL handle\n");

  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  sl_rail_ieee802154_g_options_t ieee802154g_option =
    (1 << SL_RAIL_IEEE802154_G_OPTION_GB868_SHIFT)
    | (1 << SL_RAIL_IEEE802154_G_OPTION_DYN_FEC_SHIFT)
    | (1 << SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH_SHIFT);

  status = sl_rail_ieee802154_config_g_options(rail_handle,
                                               SL_RAIL_IEEE802154_G_OPTIONS_ALL,
                                               ieee802154g_option);
  app_assert(status == SL_RAIL_STATUS_NO_ERROR,
             "sl_rail_ieee802154_config_g_options error: %lu\n",
             status);
}

/******************************************************************************
 * API to disable MAC filtering on packet received.
 *****************************************************************************/
void enable_promiscuous_mode_for_mode_switch(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "enable_promiscuous_mode_for_mode_switch error: NULL handle\n");

  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

  status = sl_rail_ieee802154_set_promiscuous_mode(rail_handle, true);
  app_assert(status == SL_RAIL_STATUS_NO_ERROR,
             "sl_rail_ieee802154_set_promiscuous_mode error: %lu\n",
             status);
}

/******************************************************************************
 * API to enable Start and End event for Mode Switch.
 *****************************************************************************/
void enable_mode_switch_events(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "enable_mode_switch_events error: NULL handle\n");

  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  sl_rail_events_t enable_modeswitch = SL_RAIL_UTIL_INIT_EVENT_INST0_MASK
                                       | SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START
                                       | SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END;

  status = sl_rail_config_events(rail_handle, SL_RAIL_EVENTS_ALL, enable_modeswitch);
  app_assert(status == SL_RAIL_STATUS_NO_ERROR,
             "sl_rail_config_events error: %lu\n",
             status);
}

/******************************************************************************
 * API to initialize the power amplifier.
 *****************************************************************************/
void init_rail_pa_settings(sl_rail_handle_t rail_handle)
{
  uint16_t fsk_channel = 0;
  uint16_t ofdm_channel = 0;
  bool fsk_found = false;
  bool ofdm_found = false;
  sl_rail_status_t status = 0;

  app_log_info("Channel info for PA: %d\n", radio_info.mode_switch_capable_channels);

  for (uint8_t i = 0; i < radio_info.mode_switch_capable_channels; i++) {
    if (radio_info.channel_list[i].modulation == M_OFDM && !ofdm_found) {
      ofdm_channel = radio_info.channel_list[i].channel_number;
      ofdm_found = true;
    }
    if (radio_info.channel_list[i].modulation != M_OFDM && !fsk_found) {
      fsk_channel = radio_info.channel_list[i].channel_number;
      fsk_found = true;
    }
    if (fsk_found && ofdm_found) {
      break;
    }
  }

  if (ofdm_found) {
    status = sl_rail_prepare_channel(rail_handle, ofdm_channel);
    app_assert(status == SL_RAIL_STATUS_NO_ERROR, "Prepare channel failed");
    status = sl_rail_set_tx_power_dbm(rail_handle, power);
    app_assert(status == SL_RAIL_STATUS_NO_ERROR, "PA setting failed");
    app_log_info("Applied OFDM PA to channel %d\n", ofdm_channel);
  }
  if (fsk_found) {
    status = sl_rail_prepare_channel(rail_handle, fsk_channel);
    app_assert(status == SL_RAIL_STATUS_NO_ERROR, "Prepare channel failed");
    status = sl_rail_set_tx_power_dbm(rail_handle, power);
    app_assert(status == SL_RAIL_STATUS_NO_ERROR, "PA setting failed");
    app_log_info("Applied Subgiga PA to channel %d\n", fsk_channel);
  }
}

/******************************************************************************
 * API to update the power amplifier settings.
 *****************************************************************************/
void update_rail_pa_settings(sl_rail_handle_t rail_handle)
{
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  status = sl_rail_set_tx_power_dbm(rail_handle, power);
  app_assert(status == SL_RAIL_STATUS_NO_ERROR, "PA setting failed");
}

/******************************************************************************
 * This function configures the basic parameters for the mode switch process.
 *****************************************************************************/
void init_mode_switch(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "init_mode_switch error: NULL handle\n");

  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

  // Radio calibration
  calibrate_radio(rail_handle);

  base_channel = channelConfigs[0]->configs[0].channelNumberStart;
  current_channel = base_channel;
  set_new_phy_mode_id(radio_info.channel_list[0].phy_mode_id);

  sl_rail_start_rx(rail_handle, current_channel, NULL);

  // Set Radio to Idle to be able to set the following settings
  sl_rail_idle(rail_handle, SL_RAIL_IDLE, true);

  // Init and set the IEEE802154 hardware acceleration
  init_ieee802154_for_mode_switch(rail_handle);

  // Enable DUALSYNC in Tx and Rx in case of FSK FEC usage
  // in the channel-based multi-PHY
  init_rx_option_for_mode_switch(rail_handle);

  // Enable Mode Switch and dynamic FEC
  init_ieee802154g_option_for_mode_switch(rail_handle);

  // Avoid MAC filtering on packet received
  enable_promiscuous_mode_for_mode_switch(rail_handle);

  // Enable Start and End event for Mode Switch
  enable_mode_switch_events(rail_handle);

  // Save available channels locally
  update_channel_list(rail_handle);

  // Init the PA setting for Mode Switch
  init_rail_pa_settings(rail_handle);

  status = sl_rail_start_rx(rail_handle, current_channel, NULL);
  app_assert(status == SL_RAIL_STATUS_NO_ERROR,
             "sl_rail_start_rx error: %lu\n",
             status);
}

/******************************************************************************
 * API to calculate modulation from PHY mode id.
 *****************************************************************************/
phy_modulation_e calculate_modulation_from_phy_mode_id(const uint8_t phy_mode_id)
{
  phy_modulation_e modulation = M_UNDEFINED;

  if ((phy_mode_id >> 4) < 2) {
    modulation = M_2FSK;
  } else {
    modulation = M_OFDM;
  }

  return modulation;
}

/******************************************************************************
 * API to update channel configurations locally.
 *****************************************************************************/
void update_channel_list(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "update_channel_list error: NULL handle\n");

  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  uint8_t next = 0U;
  uint16_t channel = CHANNEL_DOES_NOT_EXIST;
  phy_modulation_e modulation = M_UNDEFINED;

  memset(&radio_info, 0, sizeof(radio_info_t));
  for (uint8_t i = 0; i < WISUN_MODESWITCHPHRS_ARRAY_SIZE; i++) {
    channel = CHANNEL_DOES_NOT_EXIST;
    status = sl_rail_ieee802154_compute_channel_from_phy_mode_id(rail_handle,
                                                                 wisun_modeSwitchPhrs[i].phyModeId,
                                                                 &channel);
    if (status != SL_RAIL_STATUS_NO_ERROR) {
      // it fails because it is the selected channel
      (void) sl_rail_get_channel(rail_handle, &channel);
    }
    if (channel != CHANNEL_DOES_NOT_EXIST) {
      if (next >= MAX_SELECTABLE_CHANNEL) {
        break;
      }
      for (uint8_t j = 0; j < channelConfigs[0]->length; j++) {
        if (channel == channelConfigs[0]->configs[j].channelNumberStart) {
          modulation = calculate_modulation_from_phy_mode_id(wisun_modeSwitchPhrs[i].phyModeId);
          radio_info.channel_list[next].channel_number = channel;
          radio_info.channel_list[next].phy_mode_id = wisun_modeSwitchPhrs[i].phyModeId;
          radio_info.channel_list[next].modulation = modulation;
          radio_info.channel_list[next++].phy_mode_index = i;
          radio_info.mode_switch_capable_channels++;
          break;
        }
      }
    }
  }
  print_channel_list();
}

/******************************************************************************
 * API to print channel configurations.
 *****************************************************************************/
void print_channel_list(void)
{
  app_log_info("Found %d channels\n",
               radio_info.mode_switch_capable_channels);
  for (uint8_t i = 0; i < radio_info.mode_switch_capable_channels; i++) {
    if (radio_info.channel_list[i].modulation == M_OFDM) {
      app_log_info("  Ch number: %5d phyModeId: %3d modulation: %s rate: %1d\n",
                   radio_info.channel_list[i].channel_number,
                   radio_info.channel_list[i].phy_mode_id,
                   phy_modulation_strings[(uint8_t)radio_info.channel_list[i].modulation],
                   (radio_info.channel_list[i].phy_mode_id & 0x0F));
    } else {
      app_log_info("  Ch number: %5d phyModeId: %3d modulation: %s\n",
                   radio_info.channel_list[i].channel_number,
                   radio_info.channel_list[i].phy_mode_id,
                   phy_modulation_strings[(uint8_t)radio_info.channel_list[i].modulation]);
    }
  }
  app_log_info("NOTE: Although multiple channel configs may be available,\n"
               "      Config 0 is assumed for use.\n");
}

/******************************************************************************
 * API to trigger mode switch transmission.
 *****************************************************************************/
sl_status_t trig_mode_switch_tx(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "trig_mode_switch_tx error: NULL handle\n");

  sl_rail_status_t rail_status = SL_RAIL_STATUS_NO_ERROR;
  sl_status_t status = SL_STATUS_OK;
  uint32_t phr = 0U;
  uint32_t duration_in_sec = ms_duration * 1000U;
  uint16_t channel = CHANNEL_DOES_NOT_EXIST;
  uint16_t i = count_phy_mode_id_index(ms_new_phy_mode_id);
  uint8_t phy_mode_id = ms_new_phy_mode_id;

  if (i == PHY_MODE_ID_DOES_NOT_EXIST) {
    status = SL_STATUS_INVALID_PARAMETER;
  }

  if (status == SL_STATUS_OK) {
    rail_status = sl_rail_ieee802154_compute_channel_from_phy_mode_id(rail_handle,
                                                                      phy_mode_id,
                                                                      &channel);
    if (rail_status == SL_RAIL_STATUS_NO_ERROR) {
      ms_new_channel = channel;
      status = SL_STATUS_OK;
    } else {
      status = SL_STATUS_INVALID_PARAMETER;
    }
  }

  if (status == SL_STATUS_OK) {
    phr = wisun_modeSwitchPhrs[i].phr;
    phr = __RBIT(phr);
    phr >>= 16;
    memcpy(ms_phr, &(wisun_modeSwitchPhrs[i].phr), MSPHR_LENGTH);
    if (ms_duration) {
      status = sl_sleeptimer_start_timer_ms(&mode_switch_timer,
                                            duration_in_sec,
                                            ms_timer_callback,
                                            NULL,
                                            0,
                                            0);
    }
  }

  if (status == SL_STATUS_OK) {
    ms_state = MS_INITIATED;
    if (radio_info.channel_list[i].modulation ==  M_OFDM) {
      ofdm_rate = phy_mode_id & 0x0F;
    }
  } else {
    ms_state = MS_IDLE;
  }

#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif

  return status;
}

/******************************************************************************
 * The API to get the payload from the radio frame.
 *****************************************************************************/
uint16_t unpack_packet(sl_rail_handle_t rail_handle,
                       uint8_t *rx_destination,
                       const sl_rail_rx_packet_info_t *packet_information,
                       uint8_t **start_of_payload,
                       phy_modulation_e modulation)
{
  if ((rx_destination == NULL)
      || (packet_information == NULL)
      || (start_of_payload == NULL)) {
    return 0;
  }

  uint16_t payload_size = 0U;

  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
#if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
#endif
  }
  if (modulation == M_2FSK) {
    *start_of_payload
      = sl_rail_sdk_802154_packet_unpack_sunfsk_2byte_data_frame(packet_information,
                                                                 &fsk_fcs_type,
                                                                 &fsk_whitening,
                                                                 &payload_size,
                                                                 rx_destination);
  } else if (modulation == M_OFDM) {
    *start_of_payload
      = sl_rail_sdk_802154_packet_unpack_ofdm_data_frame(packet_information,
                                                         &ofdm_rate,
                                                         &ofdm_scrambler,
                                                         &payload_size,
                                                         rx_destination);
  } else {
    app_log_warning("Unkown modulation\n");
  }

  ms_new_phy_mode_id = get_phy_mode_id_from_channel(current_channel);
  if (modulation == M_OFDM) {
    ms_new_phy_mode_id += ofdm_rate;
  }

  return payload_size;
}

/******************************************************************************
 * API to prepare the packet for sending and load it into the RAIL TX FIFO.
 *****************************************************************************/
void prepare_packet(sl_rail_handle_t rail_handle,
                    uint8_t *out_data,
                    uint16_t length,
                    phy_modulation_e modulation)
{
  app_assert(rail_handle != NULL,
             "prepare_packet error: NULL handle\n");

  if (out_data == NULL) {
    return;
  }

  uint16_t bytes_written_in_fifo = 0U;
  uint16_t packet_size = 0U;

  if (ms_state == MS_INITIATED) {
    memcpy(tx_frame_buffer, &ms_phr, sizeof(ms_phr));
    packet_size = MS_PACKET_LENGTH;
  } else {
    if (modulation == M_2FSK) {
      sl_rail_sdk_802154_packet_pack_sunfsk_2bytes_data_frame(fsk_fcs_type,
                                                              fsk_whitening,
                                                              length,
                                                              out_data,
                                                              &packet_size,
                                                              tx_frame_buffer);
    } else if (modulation == M_OFDM) {
      sl_rail_sdk_802154_packet_pack_ofdm_data_frame(ofdm_rate,
                                                     ofdm_scrambler,
                                                     length,
                                                     out_data,
                                                     &packet_size,
                                                     tx_frame_buffer);
    } else {
      app_log_warning("Unkown modulation\n");
    }
  }
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle,
                                                tx_frame_buffer,
                                                packet_size,
                                                true);
  app_assert(bytes_written_in_fifo == packet_size,
             "sl_rail_write_tx_fifo() failed to write in fifo"
             "(%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             packet_size);
}

/******************************************************************************
 * API to get the current state of the mode switch process.
 *****************************************************************************/
mode_switch_state_t get_ms_state(void)
{
  return ms_state;
}

/******************************************************************************
 * API to get the text equivalent of the current mode switch state.
 *****************************************************************************/
const char* get_ms_state_str(void)
{
  return mode_switch_state_strings[(uint8_t)ms_state];
}

/******************************************************************************
 * API to set the current state of the mode switch process.
 *****************************************************************************/
void set_ms_state(mode_switch_state_t new_state)
{
  ms_state = new_state;
}

/******************************************************************************
 * API to get the text equivalent of the modulation of the current channel.
 *****************************************************************************/
const char* get_phy_modulation_str(void)
{
  phy_modulation_e phy_mod = get_phy_modulation_from_channel(current_channel);

  return phy_modulation_strings[(uint8_t)phy_mod];
}

/******************************************************************************
 * API to request mode switch.
 *****************************************************************************/
sl_status_t request_mode_switch(void)
{
  sl_status_t status = SL_STATUS_OK;
  phy_modulation_e phy_mod = get_phy_modulation_from_channel(current_channel);

  if (ms_state == MS_IDLE && phy_mod == M_2FSK) {
    ms_state = MS_REQUESTED;
  } else {
    status = SL_STATUS_INVALID_STATE;
  }

  return status;
}

/******************************************************************************
 * API to count the index of the given phy_mode_id.
 *****************************************************************************/
uint16_t count_phy_mode_id_index(const uint8_t phy_mode_id)
{
  uint16_t index = PHY_MODE_ID_DOES_NOT_EXIST;

  for (uint16_t i = 0; i < WISUN_MODESWITCHPHRS_ARRAY_SIZE; i++) {
    if (wisun_modeSwitchPhrs[i].phyModeId == phy_mode_id) {
      index = i;
      break;
    }
  }

  return index;
}

/******************************************************************************
 * API to get the index of the given channel.
 *****************************************************************************/
uint16_t get_channel_index(const uint16_t new_channel)
{
  uint16_t index = CHANNEL_DOES_NOT_EXIST;

  for (uint16_t i = 0; i < radio_info.mode_switch_capable_channels; i++) {
    if (radio_info.channel_list[i].channel_number == new_channel) {
      index = i;
      break;
    }
  }

  return index;
}

/******************************************************************************
 * API to set new phy mode id for mode switch.
 *****************************************************************************/
sl_status_t set_new_phy_mode_id(const uint8_t new_phy_mode_id)
{
  sl_status_t status = SL_STATUS_OK;
  uint16_t phy_mode_id_index = count_phy_mode_id_index(new_phy_mode_id);

  if (phy_mode_id_index == PHY_MODE_ID_DOES_NOT_EXIST) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    ms_new_phy_mode_id = new_phy_mode_id;
  }

  return status;
}

/******************************************************************************
 * API to get the currently used phy mode id.
 *****************************************************************************/
uint8_t get_phy_mode_id(void)
{
  uint8_t phy_mode_id = ms_new_phy_mode_id;

  return phy_mode_id;
}

/******************************************************************************
 * API to set time the device spends on the new channel.
 *****************************************************************************/
sl_status_t set_mode_switch_duration(const uint32_t duration)
{
  sl_status_t status = SL_STATUS_OK;

  if (duration > MODE_SWITCH_DURATION_MAX) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    ms_duration = duration;
  }
  return status;
}

/******************************************************************************
 * API to set the currently used channel.
 *****************************************************************************/
sl_rail_status_t set_channel(const uint16_t new_channel)
{
  sl_rail_handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

  status = sl_rail_start_rx(rail_handle, new_channel, NULL);
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    current_channel = new_channel;
    app_log_info("Channel is set to %d\n", current_channel);
  }

  ms_new_phy_mode_id = get_phy_mode_id_from_channel(current_channel);
  if (get_phy_modulation_from_channel(current_channel) == M_OFDM) {
    ms_new_phy_mode_id = ms_new_phy_mode_id + ofdm_rate;
  }

  return status;
}

/******************************************************************************
 * API to get the currently used channel.
 *****************************************************************************/
uint16_t get_channel(void)
{
  return current_channel;
}

/******************************************************************************
 * API to get the new channel to be set with the mode switch process.
 *****************************************************************************/
uint16_t get_ms_new_channel(void)
{
  return ms_new_channel;
}

/******************************************************************************
 * API to get the channel to be set after returning from mode switch.
 *****************************************************************************/
uint16_t get_base_channel(void)
{
  return base_channel;
}

/******************************************************************************
 * API to switch the radio channel during the mode switch process.
 *****************************************************************************/
sl_rail_status_t switch_to_ms_channel(sl_rail_handle_t rail_handle)
{
  app_assert(rail_handle != NULL,
             "switch_to_ms_channel error: NULL handle\n");

  uint16_t ms_channel = 0U;
  sl_rail_status_t status = sl_rail_get_channel(rail_handle, &ms_channel);

  if (status == SL_RAIL_STATUS_NO_ERROR) {
    base_channel = current_channel;
    status = set_channel(ms_channel);
    update_rail_pa_settings(rail_handle);
  }

  return status;
}

/******************************************************************************
 * API to set the radio back to the base channel from the mode switch channel.
 *****************************************************************************/
sl_rail_status_t return_to_base_channel(void)
{
  sl_rail_status_t status = set_channel(base_channel);

  if (status == SL_RAIL_STATUS_NO_ERROR) {
    ms_state = MS_IDLE;
  }
  ms_new_phy_mode_id = get_phy_mode_id_from_channel(current_channel);

  return status;
}

/******************************************************************************
 * API to get the PHY mode id of the given channel.
 *****************************************************************************/
uint8_t get_phy_mode_id_from_channel(const uint16_t channel)
{
  uint8_t phy_mode_id = 0;

  for (uint8_t i = 0; i < radio_info.mode_switch_capable_channels; i++) {
    if (radio_info.channel_list[i].channel_number == channel) {
      phy_mode_id = radio_info.channel_list[i].phy_mode_id;
      break;
    }
  }

  return phy_mode_id;
}

/******************************************************************************
 * API to get the PHY modulation of the given channel.
 *****************************************************************************/
phy_modulation_e get_phy_modulation_from_channel(const uint16_t channel)
{
  phy_modulation_e phy_modulation = M_UNDEFINED;

  for (uint8_t i = 0; i < radio_info.mode_switch_capable_channels; i++) {
    if (radio_info.channel_list[i].channel_number == channel) {
      phy_modulation = radio_info.channel_list[i].modulation;
      break;
    }
  }

  return phy_modulation;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * Callback function for the mode switch timer.
 *****************************************************************************/
SL_CODE_RAM static void ms_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                          void *data)
{
  (void) data;
  (void) handle;

  ms_state = MS_RETURN_TO_BASE_PHY;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  app_task_notify();
#endif
}
