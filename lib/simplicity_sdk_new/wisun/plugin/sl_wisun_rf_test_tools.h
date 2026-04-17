/***************************************************************************//**
 * @file sl_wisun_rf_test_tools.h
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

#ifndef SL_WISUN_RF_TEST_TOOLS_H
#define SL_WISUN_RF_TEST_TOOLS_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_rail.h"
#include "sl_wisun_types.h"
#include "sli_wisun_internal_api.h"

#define PHY_TYPE_FSK        0x0
#define PHY_TYPE_FSK_FEC    0x1
#define PHY_TYPE_OFDM1      0x2
#define PHY_TYPE_OFDM2      0x3
#define PHY_TYPE_OFDM3      0x4
#define PHY_TYPE_OFDM4      0x5
#define PHY_TYPE_OQPSK_100  0x6
#define PHY_TYPE_OQPSK_1000 0x7
#define PHY_TYPE_OQPSK_2000 0x8

#define GET_PHY_TYPE(_phy_mode_id) (((_phy_mode_id) >> 4) & 0xf)
#define GET_PHY_MODE(_phy_mode_id) ((_phy_mode_id) & 0xf)
#define IS_OFDM(_phy_mode_id) ((GET_PHY_TYPE(_phy_mode_id) >= PHY_TYPE_OFDM1) && (GET_PHY_TYPE(_phy_mode_id) <= PHY_TYPE_OFDM4))
#define IS_OQPSK(_phy_mode_id) ((GET_PHY_TYPE(_phy_mode_id) >= PHY_TYPE_OQPSK_100) && (GET_PHY_TYPE(_phy_mode_id) <= PHY_TYPE_OQPSK_2000))

// Stack info field indices (matching sli_wisun_driver.h definitions)
#define STACK_INFO_FIELD_PROTOCOL      0
#define STACK_INFO_FIELD_PHY_MODE_ID   1
#define STACK_INFO_FIELD_VERSION       2
#define STACK_INFO_FIELD_CHAN_PLAN_ID  3  // If version == 1
#define STACK_INFO_FIELD_OP_CLASS      3  // If version == 0
#define STACK_INFO_FIELD_REG_DOMAIN    4

#define STACK_INFO_VERSION_FAN10       0
#define STACK_INFO_VERSION_FAN11       1
#define STACK_INFO_VERSION_UNUSED    255
#define STACK_INFO_PROTOCOL_WISUN      7

#define STACK_INFO_REG_DOMAIN_UNUSED 255

#define RF_PRIORITY_PROTECTED 0
#define RF_PRIORITY_BACKGROUND 255

#define MAX_PACKET_LENGTH 2047

// RAIL events enabled (matching sli_wisun_driver.c definition)
#define MAC_TASK_RAIL_EVENTS (SL_RAIL_EVENT_RX_PACKET_RECEIVED          \
    | SL_RAIL_EVENT_RX_FRAME_ERROR             \
    | SL_RAIL_EVENT_RX_FIFO_OVERFLOW           \
    | SL_RAIL_EVENT_RX_ADDRESS_FILTERED        \
    | SL_RAIL_EVENT_RX_PACKET_ABORTED          \
    | SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED     \
    | SL_RAIL_EVENT_TX_PACKET_SENT             \
    | SL_RAIL_EVENT_TX_CHANNEL_CLEAR           \
    | SL_RAIL_EVENT_TX_CHANNEL_BUSY            \
    | SL_RAIL_EVENT_TX_UNDERFLOW               \
    | SL_RAIL_EVENT_TXACK_UNDERFLOW            \
    | SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END \
    | SL_RAIL_EVENT_RX_SYNC_0_DETECT           \
    | SL_RAIL_EVENT_RX_SYNC_1_DETECT)

typedef enum {
  RF_TEST_OFF,
  RF_TEST_TX_ACTIVE,
  RF_TEST_RX_ACTIVE,
  RF_TEST_TONE,
  RF_TEST_STREAM,
} rf_test_state_t;


uint8_t rf_test_op_mode_to_phy_mode(uint8_t op_mode, uint8_t fec);

bool rf_test_entry_matches_phy(const sl_rail_channel_config_entry_t *entry,
                               uint32_t channel_0_center_frequency_hz,
                               uint32_t channel_spacing_hz,
                               uint16_t number_of_channels,
                               uint8_t phy_mode_id,
                               uint8_t reg_domain,
                               uint8_t phy_version);

uint16_t rf_test_get_buffer_len(uint16_t length);

uint8_t rf_test_build_fsk_phr(uint8_t *dst, uint16_t frame_length, uint8_t crc_length);

uint8_t rf_test_build_ofdm_phr(uint8_t *dst,
                               uint16_t frame_length,
                               uint8_t mcs,
                               uint8_t scrambler);

uint8_t rf_test_build_oqpsk_phr(uint8_t *dst,
                                uint16_t frame_length,
                                uint8_t phy_mode_id);

bool rf_test_prepare_tx_buffer(uint8_t phy_mode_id,
                               uint16_t data_length,
                               const uint8_t *data,
                               bool use_phr,
                               uint8_t crc_length,
                               uint8_t *tx_fifo,
                               uint16_t fifo_capacity,
                               uint16_t *fifo_size_bytes,
                               uint8_t *phr_length,
                               uint16_t *init_bytes);

sl_status_t rf_test_phy_config_to_chan_config(sl_wisun_phy_config_t *phy_config,
                                              sl_rail_channel_config_entry_t *chan_config,
                                              uint8_t *phy_mode_id,
                                              uint8_t *reg_domain,
                                              uint16_t *physical_channel_offset,
                                              uint16_t *channel_start,
                                              uint16_t *channel_end);

#endif // SL_WISUN_RF_TEST_TOOLS_H
