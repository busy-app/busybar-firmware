/***************************************************************************//**
 * @file app_settings.h
 * @brief Application settings handler
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <stdint.h>
#include "sl_status.h"
#include "sl_wisun_api.h"
#include "sl_wisun_cli_util.h"

#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

#define SL_WISUN_PING_PATTERN_SIZE 16

#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#define EM_MAX SL_POWER_MANAGER_EM3
#endif

typedef struct {
  char allowed_channels[APP_UTIL_PRINTABLE_DATA_MAX_LENGTH+1];
  char network_name[SL_WISUN_NETWORK_NAME_SIZE+1];
  uint8_t regulatory_domain;
  uint8_t operating_class;
  uint16_t operating_mode;
  uint8_t network_size;
  int16_t tx_power_ddbm;
  uint16_t rx_fifo_size;
  uint8_t uc_dwell_interval_ms;
  uint16_t number_of_channels;
  uint32_t ch0_frequency;
  uint16_t channel_spacing; // channel spacing in kHz
  uint8_t trace_filter[SL_WISUN_FILTER_BITFIELD_SIZE];
  uint8_t regulation;
  int8_t regulation_warning_threshold;
  int8_t regulation_alert_threshold;
  uint8_t device_type;
  uint8_t chan_plan_id;
  uint8_t phy_mode_id;
  uint8_t phy_config_type;
  uint8_t fec;
  uint8_t rx_phy_mode_ids[SL_WISUN_MAX_PHY_MODE_ID_COUNT];
  int8_t rx_phy_mode_ids_count;
  uint8_t rx_mdr_capable;
  uint16_t protocol_id;
  uint16_t channel_id;
  uint8_t lfn_profile;
  uint8_t crc_type;
  uint8_t preamble_length;
  uint8_t stf_length;
  uint8_t max_neighbor_count;
  uint8_t max_child_count;
  uint16_t max_security_neighbor_count;
  uint16_t preferred_pan_id;
  uint8_t keychain;
  uint8_t keychain_index;
  uint8_t direct_connect_pmk[SL_WISUN_PMK_LEN];
  uint8_t max_hop_count;
  uint16_t lowpan_mtu;
  uint16_t ipv6_mru;
  uint8_t max_edfe_fragment_count;
  uint16_t socket_rx_buffer_size;
  char eap_identity[SL_WISUN_EAP_IDENTITY_SIZE+1];
  uint8_t fan_tps_version;
} app_settings_wisun_t;

typedef struct {
  uint16_t identifier;
  uint16_t sequence_number;
  uint16_t packet_interval;
  uint16_t packet_length;
  char pattern[SL_WISUN_PING_PATTERN_SIZE+1];
} app_settings_ping_t;

typedef struct {
  uint8_t printable_data_as_hex;
  uint8_t printable_data_length;
  uint8_t autoconnect;
  uint8_t pti_state;
} app_settings_app_t;

typedef struct {
  uint8_t min_be;
  uint8_t max_be;
  uint16_t backoff_period_us;
  uint8_t max_cca_retries;
  uint8_t max_frame_retries;
} app_settings_mac_t;

extern app_settings_wisun_t app_settings_wisun;
extern app_settings_ping_t app_settings_ping;
extern app_settings_app_t app_settings_app;
extern app_settings_mac_t app_settings_mac;

extern const app_enum_t app_settings_wisun_join_state_enum_ffn[];
extern const app_enum_t app_settings_wisun_join_state_enum_lfn[];

#if SLI_WISUN_DISABLE_SECURITY
extern uint32_t app_security_state;
#endif

#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
extern uint32_t lfn_em_time_ms[EM_MAX + 1];
#endif

sl_status_t app_settings_get_channel_mask(const char *str, sl_wisun_channel_mask_t *channel_mask);

#endif  // APP_SETTINGS_H
