/***************************************************************************//**
 * @file sl_wisun_rf_test_tools.c
 * @brief Wi-SUN RF test helper utilities
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <string.h>
#include <stdio.h>
#include "sl_wisun_rf_test_tools.h"
#include "sl_wisun_trace_api.h"
#include "sl_wisun_regdb.h"
#include "common/endian.h"
#include "em_core.h"

#define RF_SUN_FSK_PHR_FCS_TYPE_MASK 0x0800
#define RF_SUN_FSK_PHR_FCS_TYPE_SHIFT 11
#define RF_SUN_FSK_PHR_DATA_WHITENING_MASK 0x1000
#define RF_SUN_FSK_PHR_DATA_WHITENING_SHIFT 12
#define RF_SUN_FSK_SET_PHR_FCS_TYPE(__value__) (((__value__) << RF_SUN_FSK_PHR_FCS_TYPE_SHIFT) & RF_SUN_FSK_PHR_FCS_TYPE_MASK)
#define RF_SUN_FSK_SET_PHR_DATA_WHITENING(__value__) (((__value__) << RF_SUN_FSK_PHR_DATA_WHITENING_SHIFT) & RF_SUN_FSK_PHR_DATA_WHITENING_MASK)

// SUN OFDM bitmasks
#define RF_SUN_OFDM_PHR_RATE_MASK 0x1f
#define RF_SUN_OFDM_PHR_RATE_SHIFT 19
#define RF_SUN_OFDM_PHR_SCRAMBLER_MASK 0x3
#define RF_SUN_OFDM_PHR_SCRAMBLER_SHIFT 3
#define RF_SUN_OFDM_PHR_DATA_LENGTH_MASK 0x7FF
#define RF_SUN_OFDM_PHR_DATA_LENGTH_SHIFT 7

// SUN OQPSK bitmask
#define RF_SUN_OQPSK_PHR_SPREADING_MODE_SHIFT 15
#define RF_SUN_OQPSK_PHR_RATE_MODE_SHIFT 13
#define RF_SUN_OQPSK_PHR_DATA_LENGTH_SHIFT 0

#define LEGACY_FSK_PHR_LENGTH 2
#define OFDM_PHR_LENGTH 4
#define OQPSK_PHR_LENGTH 4

extern const sl_rail_channel_config_t * channelConfigs[];

static uint8_t rf_test_reverse_bits8(uint8_t value)
{
  value = (uint8_t)((value >> 4) | (value << 4));
  value = (uint8_t)(((value & 0xCC) >> 2) | ((value & 0x33) << 2));
  value = (uint8_t)(((value & 0xAA) >> 1) | ((value & 0x55) << 1));
  return value;
}

static uint16_t rf_test_fsk_phr_data_length(uint16_t length)
{
  uint16_t lo = rf_test_reverse_bits8((uint8_t)(length & 0xFF));
  uint16_t hi = rf_test_reverse_bits8((uint8_t)((length >> 8) & 0xFF));
  return (uint16_t)(lo | (hi << 8));
}

uint8_t rf_test_op_mode_to_phy_mode(uint8_t op_mode, uint8_t fec)
{
  uint8_t phy_mode = 0;

  switch (op_mode) {
    case 0x1a: phy_mode = 1; break;
    case 0x1b: phy_mode = 2; break;
    case 0x2a: phy_mode = 3; break;
    case 0x2b: phy_mode = 4; break;
    case 0x3:  phy_mode = 5; break;
    case 0x4a: phy_mode = 6; break;
    case 0x4b: phy_mode = 7; break;
    case 0x5:  phy_mode = 8; break;
    default: return 0;
  }

  uint8_t phy_type = fec ? 1 : 0;
  return (phy_type << 4) | phy_mode;
}

bool rf_test_entry_matches_phy(const sl_rail_channel_config_entry_t *entry,
                               uint32_t channel_0_center_frequency_hz,
                               uint32_t channel_spacing_hz,
                               uint16_t number_of_channels,
                               uint8_t phy_mode_id,
                               uint8_t reg_domain,
                               uint8_t phy_version)
{
  const uint16_t entry_number_of_channels = (uint16_t)(entry->channel_number_end - entry->channel_number_start + 1);

  if (channel_0_center_frequency_hz != entry->base_frequency_hz) {
    return false;
  }
  if (channel_spacing_hz != entry->channel_spacing_hz) {
    return false;
  }
  if (number_of_channels != entry_number_of_channels) {
    return false;
  }
  if (GET_PHY_TYPE(phy_mode_id) == PHY_TYPE_FSK || GET_PHY_TYPE(phy_mode_id) == PHY_TYPE_FSK_FEC) {
    if (phy_mode_id != entry->p_stack_info[STACK_INFO_FIELD_PHY_MODE_ID]) {
      return false;
    }
  } else if (IS_OFDM(phy_mode_id) || IS_OQPSK(phy_mode_id)) {
    if (GET_PHY_TYPE(phy_mode_id) != GET_PHY_TYPE(entry->p_stack_info[STACK_INFO_FIELD_PHY_MODE_ID])) {
      return false;
    }
  }
  if ((reg_domain != STACK_INFO_REG_DOMAIN_UNUSED) &&
      (reg_domain != entry->p_stack_info[STACK_INFO_FIELD_REG_DOMAIN])) {
    return false;
  }
  if ((phy_version == SL_WISUN_PHY_CONFIG_FAN11) &&
      (phy_version != entry->p_stack_info[STACK_INFO_FIELD_VERSION])) {
    return false;
  }

  return true;
}

uint16_t rf_test_get_buffer_len(uint16_t length)
{
  for (uint8_t i = 6; i < 14; i++) {
    if ((uint16_t)(1 << i) > length) {
      return (uint16_t)(1 << i);
    }
  }

  return 0;
}

uint8_t rf_test_build_fsk_phr(uint8_t *dst, uint16_t frame_length, uint8_t crc_length)
{
  uint16_t phr = rf_test_fsk_phr_data_length(frame_length);

  phr |= RF_SUN_FSK_SET_PHR_DATA_WHITENING(1);
  phr |= RF_SUN_FSK_SET_PHR_FCS_TYPE(crc_length == 2 ? 1 : 0);

  dst[0] = (uint8_t)(phr >> 8);
  dst[1] = (uint8_t)(phr & 0xFF);
  return LEGACY_FSK_PHR_LENGTH;
}

uint8_t rf_test_build_ofdm_phr(uint8_t *dst,
                               uint16_t frame_length,
                               uint8_t mcs,
                               uint8_t scrambler)
{
  uint32_t phr = ((uint32_t)(mcs & RF_SUN_OFDM_PHR_RATE_MASK)
                 << RF_SUN_OFDM_PHR_RATE_SHIFT) |
                 ((uint32_t)(frame_length & RF_SUN_OFDM_PHR_DATA_LENGTH_MASK)
                 << RF_SUN_OFDM_PHR_DATA_LENGTH_SHIFT) |
                 ((uint32_t)(scrambler & RF_SUN_OFDM_PHR_SCRAMBLER_MASK)
                 << RF_SUN_OFDM_PHR_SCRAMBLER_SHIFT);

  phr = __RBIT(phr);
  write_le32(dst, phr);
  return OFDM_PHR_LENGTH;
}

uint8_t rf_test_build_oqpsk_phr(uint8_t *dst,
                                uint16_t frame_length,
                                uint8_t phy_mode_id)
{
  uint32_t phr;
  uint8_t spreading_mode = (phy_mode_id & 0x4) >> 2;
  uint8_t rate_mode = (phy_mode_id & 0x3);

  phr = ((uint32_t)spreading_mode << RF_SUN_OQPSK_PHR_SPREADING_MODE_SHIFT) |
        ((uint32_t)rate_mode << RF_SUN_OQPSK_PHR_RATE_MODE_SHIFT) |
        ((uint32_t)frame_length << RF_SUN_OQPSK_PHR_DATA_LENGTH_SHIFT);

  phr = __RBIT(phr);
  write_le32(dst, phr);
  return OQPSK_PHR_LENGTH;
}

bool rf_test_prepare_tx_buffer(uint8_t phy_mode_id,
                               uint16_t data_length,
                               const uint8_t *data,
                               bool use_phr,
                               uint8_t crc_length,
                               uint8_t *tx_fifo,
                               uint16_t fifo_capacity,
                               uint16_t *fifo_size_bytes,
                               uint8_t *phr_length,
                               uint16_t *init_bytes)
{
  if (data_length == 0) {
    return false;
  }

  uint16_t frame_length = data_length + crc_length;
  uint8_t local_phr_length = 0;

  if (use_phr) {
    if (IS_OFDM(phy_mode_id)) {
      uint8_t mcs = GET_PHY_MODE(phy_mode_id);
      local_phr_length = rf_test_build_ofdm_phr(tx_fifo, frame_length, mcs, 0);
    } else if (IS_OQPSK(phy_mode_id)) {
      local_phr_length = rf_test_build_oqpsk_phr(tx_fifo, frame_length, phy_mode_id);
    } else {
      local_phr_length = rf_test_build_fsk_phr(tx_fifo, frame_length, crc_length);
    }
  }

  uint16_t total_length = (uint16_t)(data_length + local_phr_length);
  if (total_length > MAX_PACKET_LENGTH) {
    sl_wisun_trace_error("rf_test: length %u is greater than %u", total_length, MAX_PACKET_LENGTH);
    return false;
  }
  uint16_t fifo_size = rf_test_get_buffer_len(total_length);
  if (fifo_size == 0 || fifo_size > fifo_capacity) {
    return false;
  }

  uint8_t *dst = tx_fifo + local_phr_length;
  if (data != NULL) {
    memcpy(dst, data, data_length);
  } else {
    for (uint16_t i = 0; i < data_length; i++) {
      dst[i] = (uint8_t)i;
    }
  }

  *fifo_size_bytes = fifo_size;
  *phr_length = local_phr_length;
  *init_bytes = total_length;
  return true;
}


sl_status_t rf_test_phy_config_to_chan_config(sl_wisun_phy_config_t *phy_config,
                                              sl_rail_channel_config_entry_t *chan_config,
                                              uint8_t *phy_mode_id,
                                              uint8_t *reg_domain,
                                              uint16_t *physical_channel_offset,
                                              uint16_t *channel_start,
                                              uint16_t *channel_end)
{
  sl_status_t status = SL_STATUS_OK;
  sl_rail_handle_t rail_handle;
  const sl_rail_channel_config_t *iter;
  const sl_wisun_chan_params_t *chan_params = NULL;
  int index = 0;
  bool found = false;
  uint32_t channel_0_center_frequency_hz = 0;
  uint32_t channel_spacing_hz = 0;
  uint16_t number_of_channels = 0;
  uint8_t phy_version = STACK_INFO_VERSION_UNUSED;

  switch (phy_config->type) {
    case SL_WISUN_PHY_CONFIG_FAN10:
      *phy_mode_id = rf_test_op_mode_to_phy_mode(phy_config->config.fan10.op_mode,
                                                phy_config->config.fan10.fec);
      if (*phy_mode_id == 0) {
        sl_wisun_trace_error("rf_test: invalid FAN10 configuration");
        return SL_STATUS_INVALID_PARAMETER;
      }
      *reg_domain = phy_config->config.fan10.reg_domain;
      phy_version = STACK_INFO_VERSION_FAN10;
      chan_params = ws_regdb_chan_params(phy_config->config.fan10.reg_domain, 0, phy_config->config.fan10.op_class);
      if (chan_params != NULL) {
        channel_0_center_frequency_hz = chan_params->chan0_freq_hz;
        channel_spacing_hz = chan_params->chan_spacing_hz;
        number_of_channels = chan_params->chan_count;
      }
      break;

    case SL_WISUN_PHY_CONFIG_FAN11:
      *phy_mode_id = phy_config->config.fan11.phy_mode_id;
      *reg_domain = phy_config->config.fan11.reg_domain;
      phy_version = STACK_INFO_VERSION_FAN11;
      chan_params = ws_regdb_chan_params(phy_config->config.fan11.reg_domain, phy_config->config.fan11.chan_plan_id, 0);
      if (chan_params != NULL) {
        channel_0_center_frequency_hz = chan_params->chan0_freq_hz;
        channel_spacing_hz = chan_params->chan_spacing_hz;
        number_of_channels = chan_params->chan_count;
      }
      break;

    case SL_WISUN_PHY_CONFIG_EXPLICIT:
      *phy_mode_id = phy_config->config.explicit_plan.phy_mode_id;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      channel_0_center_frequency_hz = phy_config->config.explicit_plan.ch0_frequency_khz * 1000;
      number_of_channels = phy_config->config.explicit_plan.number_of_channels;
      channel_spacing_hz = ws_regdb_chan_spacing_value(phy_config->config.explicit_plan.channel_spacing);
      phy_version = STACK_INFO_VERSION_UNUSED;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      break;
    case SL_WISUN_PHY_CONFIG_CUSTOM_FSK:
      *phy_mode_id = phy_config->config.custom_fsk.phy_mode_id;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      channel_0_center_frequency_hz = phy_config->config.custom_fsk.ch0_frequency_khz * 1000;
      number_of_channels = phy_config->config.custom_fsk.number_of_channels;
      channel_spacing_hz = phy_config->config.custom_fsk.channel_spacing_khz * 1000;
      phy_version = STACK_INFO_VERSION_UNUSED;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      break;
    case SL_WISUN_PHY_CONFIG_CUSTOM_OFDM:
      *phy_mode_id = phy_config->config.custom_ofdm.phy_mode_id;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      channel_0_center_frequency_hz = phy_config->config.custom_ofdm.ch0_frequency_khz * 1000;
      number_of_channels = phy_config->config.custom_ofdm.number_of_channels;
      channel_spacing_hz = phy_config->config.custom_ofdm.channel_spacing_khz * 1000;
      phy_version = STACK_INFO_VERSION_UNUSED;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      break;
    case SL_WISUN_PHY_CONFIG_CUSTOM_OQPSK:
      *phy_mode_id = phy_config->config.custom_oqpsk.phy_mode_id;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      channel_0_center_frequency_hz = phy_config->config.custom_oqpsk.ch0_frequency_khz * 1000;
      number_of_channels = phy_config->config.custom_oqpsk.number_of_channels;
      channel_spacing_hz = phy_config->config.custom_oqpsk.channel_spacing_khz * 1000;
      phy_version = STACK_INFO_VERSION_UNUSED;
      *reg_domain = STACK_INFO_REG_DOMAIN_UNUSED;
      break;

    case SL_WISUN_PHY_CONFIG_IDS:
      status = sli_wisun_get_rail_handle(&rail_handle);
      if (status != SL_STATUS_OK) {
        sl_wisun_trace_error("rf_test: failed to get rail handle");
        return status;
      }
      int proto_index = 0;
      while (channelConfigs[proto_index] != NULL && proto_index < phy_config->config.ids.protocol_id) {
        proto_index++;
      }
      if (channelConfigs[proto_index] == NULL) {
        sl_wisun_trace_error("rf_test: IDS protocol_id not found\r\n");
        return SL_STATUS_INVALID_PARAMETER;
      }
      iter = channelConfigs[proto_index];

      sl_rail_config_channels(rail_handle, iter, NULL);

      if (phy_config->config.ids.channel_id < iter->number_of_entries) {
        const sl_rail_channel_config_entry_t *entry = &iter->p_entries[phy_config->config.ids.channel_id];
        if (entry->p_stack_info) {
          *reg_domain = entry->p_stack_info[STACK_INFO_FIELD_REG_DOMAIN];
          *phy_mode_id = phy_config->config.ids.phy_mode_id;
          *channel_start = chan_config->channel_number_start;
          *channel_end = chan_config->channel_number_end;
          *physical_channel_offset = entry->physical_channel_offset;
          if (entry->p_stack_info[STACK_INFO_FIELD_PHY_MODE_ID] == *phy_mode_id) {
            memcpy(chan_config, entry, sizeof(sl_rail_channel_config_entry_t));
            return SL_STATUS_OK;
          }
        }
      }
      sl_wisun_trace_error("rf_test: IDS config did not match entry");
      return SL_STATUS_INVALID_PARAMETER;

    default:
      sl_wisun_trace_error("rf_test: unknown phy_config type %u",
                          (unsigned int)phy_config->type);
      return SL_STATUS_INVALID_PARAMETER;
  }

  status = sli_wisun_get_rail_handle(&rail_handle);
  if (status != SL_STATUS_OK) {
    return status;
  }

  iter = channelConfigs[index];
  while (iter && !found) {
    sl_rail_config_channels(rail_handle, iter, NULL);

    for (uint32_t entry_num = 0; entry_num < iter->number_of_entries; entry_num++) {
      const sl_rail_channel_config_entry_t *entry = &iter->p_entries[entry_num];

      if (!entry->p_stack_info) {
        continue;
      }

      if (rf_test_entry_matches_phy(entry,
                                    channel_0_center_frequency_hz,
                                    channel_spacing_hz,
                                    number_of_channels,
                                    *phy_mode_id,
                                    *reg_domain,
                                    phy_version)) {
        memcpy(chan_config, entry, sizeof(sl_rail_channel_config_entry_t));
        *phy_mode_id = entry->p_stack_info[STACK_INFO_FIELD_PHY_MODE_ID];
        *reg_domain = entry->p_stack_info[STACK_INFO_FIELD_REG_DOMAIN];
        *physical_channel_offset = entry->physical_channel_offset;
        *channel_start = chan_config->channel_number_start;
        *channel_end = chan_config->channel_number_end;
        found = true;
        break;
      }
    }

    if (!found) {
      iter = channelConfigs[++index];
    }
  }

  if (!found) {
    sl_wisun_trace_error("rf_test: no matching entry found");
    *physical_channel_offset = 0;
    *phy_mode_id = 0;
    *reg_domain = 0;
    *channel_start = 0;
    *channel_end = 0;
    status = SL_STATUS_NOT_FOUND;
  }
  return status;
}
