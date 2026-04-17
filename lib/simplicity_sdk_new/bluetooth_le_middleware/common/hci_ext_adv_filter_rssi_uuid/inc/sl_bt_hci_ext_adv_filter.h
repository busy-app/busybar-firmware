/***************************************************************************//**
 * @file
 * @brief Provides API to create and configure HCI event filter for filtering
 * Extended Advertisment Reports using RSSI and UUID.
 *
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BT_HCI_EXT_ADV_FILTER_H
#define SL_BT_HCI_EXT_ADV_FILTER_H

/***********************************************************************************************//**
 * @addtogroup hci_ext_adv_filter_rssi_uuid
 * @{
 **************************************************************************************************/

#include <stdint.h>
#include "sl_status.h"
#include "sl_common.h"

/// Enable RSSI filtering.
/// The bit position in the filter bitmap to enable RSSI filtering
#define SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE 0
#define SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE_MASK \
  (1 << SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE)

/// Maximum RSSI threshold value for RSSI filtering
#define SL_BT_HCI_FILTER_RSSI_MAX -30

/// Minumum RSSI threshold value for RSSI filtering
#define SL_BT_HCI_FILTER_RSSI_MIN -120

/// The bit position in the filter bitmap to enable 16-bit Service Data UUIDs
/// filtering
#define SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_16_BIT_ENABLE 1
#define SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_16_BIT_ENABLE_MASK \
  (1 << SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_16_BIT_ENABLE)

///The bit position in the filter bitmap to enable filtering of Incomplete
/// Lists of 16-bit Service Class UUIDs
#define SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_16_BIT_ENABLE 2
#define SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_16_BIT_ENABLE_MASK \
  (1 << SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_16_BIT_ENABLE)

/// The Bit position in the filter bitmap to enable filtering of Complete lists
/// of 16-bit Service Class UUIDs
#define SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_16_BIT_ENABLE 3
#define SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_16_BIT_ENABLE_MASK \
  (1 << SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_16_BIT_ENABLE)

/// The bit position in the filter bitmap to enable 32-bit Service Data UUID
/// filtering
#define SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_32_BIT_ENABLE 4
#define SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_32_BIT_ENABLE_MASK \
  (1 << SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_32_BIT_ENABLE)

/// The bit position in the filter bitmap to enable filtering of Incomplete
/// Lists of 32-bit Service Class UUIDs
#define SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_32_BIT_ENABLE 5
#define SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_32_BIT_ENABLE_MASK \
  (1 << SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_32_BIT_ENABLE)

/// The bit position in the filter bitmap to enable filtering of Complete
/// Lists of 32-bit Service Class UUIDs
#define SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_32_BIT_ENABLE 6
#define SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_32_BIT_ENABLE_MASK \
  (1 << SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_32_BIT_ENABLE)

#define SL_BT_HCI_FILTER_CONFIG_MASK                              \
  (SL_BT_HCI_FILTER_CONFIG_RSSI_ENABLE_MASK                       \
   | SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_16_BIT_ENABLE_MASK \
   | SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_16_BIT_ENABLE_MASK   \
   | SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_16_BIT_ENABLE_MASK     \
   | SL_BT_HCI_FILTER_CONFIG_SERVICE_DATA_UUID_32_BIT_ENABLE_MASK \
   | SL_BT_HCI_FILTER_CONFIG_INCOMPLETE_UUID_32_BIT_ENABLE_MASK   \
   | SL_BT_HCI_FILTER_CONFIG_COMPLETE_UUID_32_BIT_ENABLE_MASK)

SL_PACK_START(1)
typedef struct {
  uint32_t config;
  int8_t rssi_threshold;
} SL_ATTRIBUTE_PACKED sl_btctrl_hci_event_filter_config_t;
SL_PACK_END()

SL_PACK_START(1)
typedef struct {
  uint8_t length;
  uint8_t data[0];
} SL_ATTRIBUTE_PACKED sl_btctrl_hci_event_uuid_filter_t;
SL_PACK_END()

/*******************************************************************************
 * Initialize HCI event filtering HCI command
 ******************************************************************************/
void sli_bt_hci_filter_init(void);

/*******************************************************************************
 * Configure HCI event filtering
 * @param filter is a pointer to sl_btctrl_hci_event_filter_t structure
 * @returns Returns SL_STATUS_OK if success
 ******************************************************************************/
sl_status_t sl_btctrl_hci_event_configure_filtering(void *filter);

/*******************************************************************************
 * Initialize HCI event filtering with RSSI and UUID filtering enabled
 * @returns Returns SL_STATUS_OK if success.
 ******************************************************************************/
sl_status_t sl_btctrl_rssi_uuid_filtering_init(void);

/** @} (end addtogroup hci_ext_adv_filter_rssi_uuid) */
#endif // SL_BT_HCI_EXT_ADV_FILTER_H
