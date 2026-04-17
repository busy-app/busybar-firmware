/***************************************************************************//**
 * @brief Bluetooth Link Layer configuration API
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef _SL_BTCTRL_LINKLAYER_DEFS_H_
#define _SL_BTCTRL_LINKLAYER_DEFS_H_

#include "sl_status.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * Link Layer scheduler priorities. */
typedef struct {
  uint8_t scan_min;
  uint8_t scan_max;
  uint8_t adv_min;
  uint8_t adv_max;
  uint8_t conn_min;
  uint8_t conn_max;
  uint8_t init_min;
  uint8_t init_max;
  uint8_t rail_mapping_offset;
  uint8_t rail_mapping_range;
  uint8_t rfu;
  uint8_t adv_step;
  uint8_t scan_step;
  uint8_t pawr_tx_min;
  uint8_t pawr_tx_max;
  uint8_t pawr_rx_min;
  uint8_t pawr_rx_max;
} sl_btctrl_ll_priorities;

// The sl_bt_bluetooth_ll_priorities struct was renamed sl_btctrl_ll_priorities in BG-16720.
// This typedef provides backward compatibility, and will be removed when the
// linklayer_priorities struct pointer is removed from the sl_bt_stack_config_t struct.
typedef sl_btctrl_ll_priorities sl_bt_bluetooth_ll_priorities;

/**
 * Highest limit of the (SCA) Sleep Clock Accuracy in ppm
 */
#define SL_BTCTRL_SLEEP_CLOCK_ACCURACY_LIMIT 500

/**
 * Basic BLE controller configuration. */
struct sl_btctrl_config {
  uint32_t buffer_memory; // Amount of controller buffer memory to allocate
  uint16_t sleepClockAccuracy;
  uint32_t flags;
  sl_btctrl_ll_priorities *priorities;
  int16_t txGain;
  int16_t rxGain;
  int16_t tx_power_min;
  int16_t tx_power_max;
  uint8_t linklayer_irq_priority;
  uint8_t radio_irq_priority;
  uint16_t conn_ce_length_min;
  bool rtos_enabled;
};

/**
 * Channel map configuration flags. */
enum sl_btctrl_channelmap_flags{
  SL_BTCTRL_CHANNELMAP_FLAG_ACTIVE_ADAPTIVITY = 0x01,
  SL_BTCTRL_CHANNELMAP_FLAG_PASSIVE_ADAPTIVITY= 0x02,
};

/**
 * Configuration flags for the controller. */
enum sl_btctrl_config_flags {
  SL_BTCTRL_CONFIG_FLAG_ENABLE_CONN_EVENT_LENGTH_EXTENSION = 0x20000,
  SL_BTCTRL_CONFIG_FLAG_SCANNER_RECEPTION_EARLY_ABORT = 0x40000,
  SL_BTCTRL_CONFIG_FLAG_USE_INACCURATE_SLEEP_CLOCK = 0x80000,
  SL_BTCTRL_CONFIG_FLAG_SYNCHRONIZE_TO_SLEEP_CLOCK = 0x100000,
};

enum sl_btctrl_advertiser_config_flags {
  SL_BTCTRL_CONFIG_FLAG_FORCE_PUBLIC_ADDRESS = 0x01,
  SL_BTCTRL_CONFIG_FLAG_PRIMARY_EXT_PACKET_ADDRESS = 0x02,
  SL_BTCTRL_CONFIG_FLAG_PRIMARY_EXT_PACKET_TX_POWER = 0x04,
};

/**
 * Configuration for Periodic Advertising with Responses. */
struct sl_btctrl_pawr_advertiser_config {
  /**
   * Number of advertising sets supporting PAwR.
   * If set to zero, previously allocated PAwR sets are only freed. */
  uint8_t max_pawr_sets;
  /**
   * Hint to the controller what will be the maximum advertised data length.
   * The value does not prevent using longer advertising data. Value zero means
   * that maximum data length can expected to be up to the length of Periodic
   * Advertising Delay. */
  uint8_t max_advertised_data_length_hint;
  /**
   * The number of subevent advertising packets requested from the host
   * at once. */
  uint8_t subevent_data_request_count;
  /**
   * How many subevents before airing a subevent its data is requested from
   * the host. */
  uint8_t subevent_data_request_advance;
};

/**
 * Configuration of synchronizer for Periodic Advertising with Responses. */
struct sl_btctrl_pawr_synchronizer_config {
  /**
   * Number of advertising sets supporting PArR.
   * If set to zero, previously allocated PArR sets are only freed. */
  uint8_t max_pawr_sets;
};

/**
 * Radio counters for troubleshooting purposes. */
struct sl_btctrl_radio_counters {
  uint16_t tx_packets; ///< Number of transmitted packets
  uint16_t rx_packets; ///< Number of received packets
  uint16_t crc_errors; ///< Number of received packets with CRC error
  uint16_t failures;   ///< Number of radio failures
};

/**
 * Channel sounding configuration. */
struct sl_btctrl_cs_config {
  /** number of channel sounding configurations per connection */
  uint8_t configs_per_connection;
  /** number of simultaneous channel sounding procedures */
  uint8_t procedures;
  /** max number of antennas used in channel sounding sync packets */
  uint8_t cs_sync_antennas_max;
};

struct sl_btctrl_adv_config {
  enum sl_btctrl_advertiser_config_flags flags;
};

#endif // _SL_BTCTRL_LINKLAYER_DEFS_H_
