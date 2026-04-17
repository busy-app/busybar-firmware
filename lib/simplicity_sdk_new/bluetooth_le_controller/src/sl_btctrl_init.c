/***************************************************************************//**
 * @file
 * @brief Bluetooth initialization and event processing
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <sl_common.h>
#include "sl_btctrl_linklayer.h"
#include "sl_btctrl_config.h"
#include "sl_btctrl_scheduler_priority_config.h"
#include "sl_bt_ll_config.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_CONTROLLER_ANCHOR_SELECTION_PRESENT)
#include "sl_btctrl_anchor_selection_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT)
#include "sl_bt_power_control_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#include "sl_bluetooth_connection_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
#include "sl_bluetooth_advertiser_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PHY_SUPPORT_CONFIG_PRESENT)
#include "sl_btctrl_phy_support_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT)
#include "sl_bt_resolving_list_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ACCEPT_LIST_PRESENT)
#include "sl_bt_accept_list_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT) || defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
#include "sl_bluetooth_cs_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_RCP_PRESENT)
#include "sl_btctrl_hci.h"
#endif // SL_CATALOG_BLUETOOTH_RCP_PRESENT

#if defined(SL_CATALOG_RAIL_UTIL_COEX_PRESENT)
#include "coexistence-ble.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_PRESENT)
#include "sl_bluetooth.h"
#include "sl_bt_host_adaptation_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
#include "sl_bt_periodic_advertiser_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
#include "sl_bt_pawr_advertiser_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
#include "sl_bluetooth_periodic_sync_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_SYNC_PRESENT)
#include "sl_bt_pawr_sync_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
#include "sl_bluetooth_connection_analyzer_config.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT)
#include "sl_btctrl_iso_conn_config.h"
#include "sl_btctrl_iso.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_RCP_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_HCI_UART_PRESENT) || defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
#include "sl_hci_uart.h"
#endif // SL_CATALOG_BLUETOOTH_HCI_UART_PRESENT || SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
#endif // SL_CATALOG_BLUETOOTH_RCP_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_HCI_TRANSPORT_PRESENT)
#include "sl_hci_common_transport.h"
#endif // SL_CATALOG_BLUETOOTH_HCI_TRANSPORT_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_HCI_UART_RTOS_PRESENT)
#include "sl_btctrl_hci_uart_rtos.h"
#endif // SL_CATALOG_BLUETOOTH_HCI_UART_RTOS_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_HCI_CPC_RTOS_PRESENT)
#include "sl_btctrl_hci_cpc_rtos.h"
#endif // SL_CATALOG_BLUETOOTH_HCI_CPC_RTOS_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_HCI_SLEEP_PRESENT)
#include "sl_btctrl_hci_sleep.h"
#endif // SL_CATALOG_BLUETOOTH_HCI_SLEEP_PRESENT

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include <sl_power_manager.h>
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

#if defined(SL_CATALOG_SLEEPTIMER_PRESENT)
#include <sl_sleeptimer.h>
#endif // SL_CATALOG_SLEEPTIMER_PRESENT

#ifndef SL_BT_CONFIG_MAX_CONNECTIONS
#define SL_BT_CONFIG_MAX_CONNECTIONS 0
#endif
#ifndef SL_BT_CONFIG_USER_ADVERTISERS
#define SL_BT_CONFIG_USER_ADVERTISERS 0
#endif
#ifndef SL_BT_CONFIG_ACCEPT_LIST_SIZE
#define SL_BT_CONFIG_ACCEPT_LIST_SIZE 0
#endif
#ifndef SL_BT_ACTIVATE_POWER_CONTROL
#define SL_BT_ACTIVATE_POWER_CONTROL 0
#endif
#ifndef SL_BT_CONFIG_RESOLVING_LIST_SIZE
#define SL_BT_CONFIG_RESOLVING_LIST_SIZE 0
#endif
#ifndef SL_BT_COMPONENT_ADVERTISERS
#define SL_BT_COMPONENT_ADVERTISERS 0
#endif
#ifndef SL_BT_COMPONENT_CONNECTIONS
#define SL_BT_COMPONENT_CONNECTIONS 0
#endif

static sl_btctrl_ll_priorities sli_btctrl_priority_table = SL_BTCTRL_SCHEDULER_PRIORITIES;

SL_WEAK void sl_btctrl_debug_init(struct sl_btctrl_config *config)
{
  (void) config;
}

sl_status_t sl_btctrl_init_internal(struct sl_btctrl_config *config)
{
  sl_status_t status = SL_STATUS_OK;

  if (sl_btctrl_is_initialized() == true) {
    return SL_STATUS_OK;
  }

#if defined(SL_CATALOG_BLUETOOTH_RCP_PRESENT)
  sl_btctrl_configure_le_buffer_size(SL_BT_CONTROLLER_LE_BUFFER_SIZE_MAX);
#endif // SL_CATALOG_BLUETOOTH_RCP_PRESENT

  status = sl_btctrl_init_mem(config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  return sl_btctrl_init_functional(config);
}

sl_status_t sl_btctrl_init_functional(struct sl_btctrl_config *config)
{
  sl_status_t status = SL_STATUS_OK;

  if (sl_btctrl_is_initialized() == true) {
    return SL_STATUS_OK;
  }

#if SL_BT_CONTROLLER_CONN_EVENT_LENGTH_EXTENSION == 1
  config->flags |= SL_BTCTRL_CONFIG_FLAG_ENABLE_CONN_EVENT_LENGTH_EXTENSION;
#endif // SL_BT_CONTROLLER_CONN_EVENT_LENGTH_EXTENSION

#if SL_BT_CONTROLLER_SCANNER_RECEPTION_EARLY_ABORT == 1
  sl_btctrl_init_radio_early_rx_abort();
  config->flags |= SL_BTCTRL_CONFIG_FLAG_SCANNER_RECEPTION_EARLY_ABORT;
#endif // SL_BT_CONTROLLER_SCANNER_RECEPTION_EARLY_ABORT

  config->conn_ce_length_min = SL_BT_CONTROLLER_CONN_EVENT_LENGTH_MIN;

#if !defined(SL_CATALOG_KERNEL_PRESENT)
  config->rtos_enabled = false;
// Ensure that the radio IRQs have a higher priority than the Link Layer IRQ priority
#if (SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY <= SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY)
#error Invalid configuration: SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY <= SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY
#endif // SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY <= SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY
#else // SL_CATALOG_KERNEL_PRESENT
  config->rtos_enabled = true;
#endif // !SL_CATALOG_KERNEL_PRESENT

// Beginning of TX Power and IRQ priority initialization section
#if defined(SL_CATALOG_BLUETOOTH_PRESENT) // Stack present

#if ((SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE == 1) && (SL_BT_CONFIG_MIN_TX_POWER == -30))
  config->tx_power_min = SL_BT_CONTROLLER_MIN_POWER_LEVEL;
#endif

#if ((SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE == 1) && (SL_BT_CONFIG_MAX_TX_POWER == 80))
  config->tx_power_max = SL_BT_CONTROLLER_MAX_POWER_LEVEL;
#endif

#if !defined(SL_CATALOG_KERNEL_PRESENT)
// Ensure that the radio IRQs have a higher priority than the Link Layer IRQ priority
#if (SL_BT_HOST_ADAPTATION_LINKLAYER_IRQ_PRIORITY <= SL_BT_HOST_ADAPTATION_RADIO_IRQ_PRIORITY)
#error Invalid configuration: SL_BT_HOST_ADAPTATION_LINKLAYER_IRQ_PRIORITY <= SL_BT_HOST_ADAPTATION_RADIO_IRQ_PRIORITY
#endif // SL_BT_HOST_ADAPTATION_LINKLAYER_IRQ_PRIORITY <= SL_BT_HOST_ADAPTATION_RADIO_IRQ_PRIORITY
#endif // !SL_CATALOG_KERNEL_PRESENT

  // Use the value from Host Adaptation if it is not set to the default of 5, as the customer has modified the config in this case
  // and thus intends to use this particular IRQ priority.
  if (SL_BT_HOST_ADAPTATION_LINKLAYER_IRQ_PRIORITY != 5) {
    config->linklayer_irq_priority = SL_BT_HOST_ADAPTATION_LINKLAYER_IRQ_PRIORITY;
  } else {
    config->linklayer_irq_priority = SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY;
  }

  // Use the value from Host Adaptation if it is not set to the default of 4, as the customer has modified the config in this case
  // and thus intends to use this particular IRQ priority.
  if (SL_BT_HOST_ADAPTATION_RADIO_IRQ_PRIORITY != 4) {
    config->radio_irq_priority = SL_BT_HOST_ADAPTATION_RADIO_IRQ_PRIORITY;
  } else {
    config->radio_irq_priority = SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY;
  }

#else // SL_CATALOG_BLUETOOTH_PRESENT

#if (SL_BT_CONTROLLER_MIN_POWER_LEVEL_OVERRIDE == 1)
  config->tx_power_min = SL_BT_CONTROLLER_MIN_POWER_LEVEL;
#else
  config->tx_power_min = SL_BT_USE_MIN_POWER_LEVEL_SUPPORTED_BY_RADIO;
#endif

#if (SL_BT_CONTROLLER_MAX_POWER_LEVEL_OVERRIDE == 1)
  config->tx_power_max = SL_BT_CONTROLLER_MAX_POWER_LEVEL;
#else
  config->tx_power_max = SL_BT_USE_MAX_POWER_LEVEL_SUPPORTED_BY_RADIO;
#endif

  config->linklayer_irq_priority = SL_BT_CONTROLLER_LINKLAYER_IRQ_PRIORITY;
  config->radio_irq_priority = SL_BT_CONTROLLER_RADIO_IRQ_PRIORITY;

#endif // SL_CATALOG_BLUETOOTH_PRESENT
// End of TX Power and IRQ priority initialization section

#if defined(SL_CATALOG_BLUETOOTH_RCP_PRESENT) && !defined(SL_CATALOG_KERNEL_PRESENT)
  sli_btctrl_events_init();
#endif // SL_CATALOG_BLUETOOTH_RCP_PRESENT && !SL_CATALOG_KERNEL_PRESENT

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  // Configure RAIL to use timersync in sleep handling for Bluetooth.
  config->flags |= SL_BTCTRL_CONFIG_FLAG_SYNCHRONIZE_TO_SLEEP_CLOCK;
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

#if defined(SL_CATALOG_SLEEPTIMER_PRESENT)
  config->sleepClockAccuracy = sl_sleeptimer_get_clock_accuracy();
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  if (!(config->flags & SL_BTCTRL_CONFIG_FLAG_USE_INACCURATE_SLEEP_CLOCK)) {
    if (config->sleepClockAccuracy > SL_BTCTRL_SLEEP_CLOCK_ACCURACY_LIMIT) {
      sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
    }
  }
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
#endif // SL_CATALOG_SLEEPTIMER_PRESENT

  status = sl_btctrl_init_ll(config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  if (config->priorities != NULL) {
    sl_btctrl_configure_scheduler_priorities(config->priorities);
  } else {
    sl_btctrl_configure_scheduler_priorities(&sli_btctrl_priority_table);
  }

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PHY_SUPPORT_CONFIG_PRESENT)
#if SL_BT_CONTROLLER_2M_PHY_SUPPORT == 0
  sl_btctrl_disable_2m_phy();
#endif
#if SL_BT_CONTROLLER_CODED_PHY_SUPPORT == 0
  sl_btctrl_disable_coded_phy();
#endif
#endif // SL_CATALOG_BLUETOOTH_FEATURE_PHY_SUPPORT_CONFIG_PRESENT

#if defined(SL_CATALOG_RAIL_UTIL_COEX_PRESENT)
  sl_bt_init_coex_hal();
#endif

#if (SL_RAIL_LIB_MULTIPROTOCOL_SUPPORT == 1)
  sl_btctrl_init_multiprotocol();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RADIO_WATCHDOG_PRESENT)
  sl_btctrl_enable_radio_watchdog();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
  sl_btctrl_init_adv();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTENDED_ADVERTISER_PRESENT)
  sl_btctrl_init_adv_ext();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
  sl_btctrl_init_scan();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTENDED_SCANNER_PRESENT)
  sl_btctrl_init_scan_ext();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EVEN_SCHEDULING_PRESENT)
  sl_btctrl_init_even_anchor_selection();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PAWR_SCHEDULING_PRESENT)
  sl_btctrl_init_empty_center_anchor_selection();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sl_btctrl_init_conn();
#if !defined(SL_CATALOG_BLUETOOTH_CONNECTION_PHY_UPDATE_INCOMPATIBLE_PRESENT)
  sl_btctrl_init_phy();
#endif
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_STATISTICS_PRESENT)
  sl_btctrl_init_conn_statistics();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT)
  sl_btctrl_init_subrate();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CHANNEL_CLASSIFICATION_PRESENT)
  sl_btctrl_init_channel_classification();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT)
  const sl_bt_ll_power_control_config_t power_control_config = {
    .activate_power_control = SL_BT_ACTIVATE_POWER_CONTROL,
    .golden_rssi_min_1m = SL_BT_GOLDEN_RSSI_MIN_1M,
    .golden_rssi_max_1m = SL_BT_GOLDEN_RSSI_MAX_1M,
    .golden_rssi_min_2m = SL_BT_GOLDEN_RSSI_MIN_2M,
    .golden_rssi_max_2m = SL_BT_GOLDEN_RSSI_MAX_2M,
    .golden_rssi_min_coded_s8 = SL_BT_GOLDEN_RSSI_MIN_CODED_S8,
    .golden_rssi_max_coded_s8 = SL_BT_GOLDEN_RSSI_MAX_CODED_S8,
    .golden_rssi_min_coded_s2 = SL_BT_GOLDEN_RSSI_MIN_CODED_S2,
    .golden_rssi_max_coded_s2 = SL_BT_GOLDEN_RSSI_MAX_CODED_S2
  };

  status = sl_btctrl_init_power_control((const uint8_t *) &power_control_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_USER_POWER_CONTROL_PRESENT)
  sl_bt_init_app_controlled_tx_power();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  sl_btctrl_init_periodic_adv();
  sl_btctrl_alloc_periodic_adv(SL_BT_CONFIG_MAX_PERIODIC_ADVERTISERS);
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
  struct sl_btctrl_pawr_advertiser_config pawr_config = {
    .max_pawr_sets = SL_BT_CONFIG_MAX_PAWR_ADVERTISERS,
    .max_advertised_data_length_hint = SL_BT_CONFIG_MAX_PAWR_ADVERTISED_DATA_LENGTH_HINT,
    .subevent_data_request_count = SL_BT_CONFIG_PAWR_PACKET_REQUEST_COUNT,
    .subevent_data_request_advance = SL_BT_CONFIG_PAWR_PACKET_REQUEST_ADVANCE,
  };
  status = sl_btctrl_pawr_advertiser_configure(&pawr_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  sl_btctrl_init_periodic_scan();
  status = sl_btctrl_alloc_periodic_scan(SL_BT_CONFIG_MAX_PERIODIC_ADVERTISING_SYNC);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_SYNC_PRESENT)
  struct sl_btctrl_pawr_synchronizer_config pawr_sync_config = {
    .max_pawr_sets = SL_BT_CONFIG_MAX_PAWR_SYNCHRONIZERS,
  };
  status = sl_btctrl_pawr_synchronizer_configure(&pawr_sync_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_AFH_PRESENT)
  status = sl_btctrl_init_afh(SL_BT_CONTROLLER_ADAPTIVITY_MODE);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif // SL_CATALOG_BLUETOOTH_FEATURE_AFH_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_HIGH_POWER_PRESENT)
  sl_btctrl_init_highpower();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_HIGH_POWER_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PHY_SUPPORT_CONFIG_PRESENT)
#if SL_BT_CONTROLLER_2M_PHY_SUPPORT == 0
  sl_btctrl_disable_2m_phy();
#endif
#if SL_BT_CONTROLLER_CODED_PHY_SUPPORT == 0
  sl_btctrl_disable_coded_phy();
#endif
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_AOA_RECEIVER_PRESENT) \
  || defined(SL_CATALOG_BLUETOOTH_FEATURE_AOD_RECEIVER_PRESENT)
  status = sl_btctrl_init_cte_receiver();
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_AOA_TRANSMITTER_PRESENT) \
  || defined(SL_CATALOG_BLUETOOTH_FEATURE_AOD_TRANSMITTER_PRESENT)
  status = sl_btctrl_init_cte_transmitter();
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PAST_PRESENT)
  sl_btctrl_init_past_local_sync_transfer();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PAST_PRESENT)
  sl_btctrl_init_past_remote_sync_transfer();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
  sl_btctrl_init_past_receiver();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
  status = sl_btctrl_init_sniff(SL_BT_CONFIG_MAX_CONNECTION_ANALYZERS);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT) || defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
  struct sl_btctrl_cs_config cs_config = { 0 };
  cs_config.configs_per_connection = SL_BT_CONFIG_MAX_CS_CONFIGS_PER_CONNECTION;
  cs_config.procedures = SL_BT_CONFIG_MAX_CS_PROCEDURES;
  cs_config.cs_sync_antennas_max = SL_BT_CONFIG_CS_SYNC_MAX_ANTENNAS;
  sl_btctrl_init_cs(&cs_config);
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT or SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT

  // In addition to the user advertisers and connections, the number of internal
  // advertisers and connections also needs to be accounted for (i.e. SL_BT_COMPONENT_ADVERTISERS
  // and SL_BT_COMPONENT_CONNECTIONS).
  sl_btctrl_init_basic(SL_BT_CONFIG_MAX_CONNECTIONS + SL_BT_COMPONENT_CONNECTIONS,
                       SL_BT_CONFIG_USER_ADVERTISERS + SL_BT_COMPONENT_ADVERTISERS,
                       SL_BT_CONFIG_ACCEPT_LIST_SIZE);

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT) || defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTENDED_ADVERTISER_PRESENT)
  struct sl_btctrl_adv_config adv_config = { 0 };

#if SL_BT_CONTROLLER_PRIMARY_EXT_PACKET_INCLUDE_ADDRESS == 1
  adv_config.flags |= SL_BTCTRL_CONFIG_FLAG_PRIMARY_EXT_PACKET_ADDRESS;
#endif

#if SL_BT_CONTROLLER_PRIMARY_EXT_PACKET_INCLUDE_TX_POWER == 1
  adv_config.flags |= SL_BTCTRL_CONFIG_FLAG_PRIMARY_EXT_PACKET_TX_POWER;
#endif
  sl_btctrl_config_adv(&adv_config);
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT)
  sl_btctrl_init_privacy();
  status = sl_btctrl_allocate_resolving_list_memory(SL_BT_CONFIG_RESOLVING_LIST_SIZE);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sl_btctrl_configure_completed_packets_reporting(SL_BT_CONTROLLER_COMPLETED_PACKETS_THRESHOLD,
                                                  SL_BT_CONTROLLER_COMPLETED_PACKETS_EVENTS_TIMEOUT);
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
  sl_btctrl_configure_max_queued_adv_reports(SL_BT_CONFIG_MAX_QUEUED_ADV_REPORTS);
#endif // SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT)
  struct sl_btctrl_iso_conn_config iso_conn_config = {
    .max_cises = SL_BT_CONFIG_MAX_CISES,
    .max_cigs = SL_BT_CONFIG_MAX_CIGS,
  };
  sl_btctrl_configure_iso_conn(&iso_conn_config);
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT

// Initialize HCI

#if defined(SL_CATALOG_BLUETOOTH_HCI_UART_RTOS_PRESENT)
  sl_btctrl_hci_uart_rtos_init();
#endif // SL_CATALOG_BLUETOOTH_HCI_UART_RTOS_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_HCI_CPC_RTOS_PRESENT)
  sl_btctrl_hci_cpc_rtos_init();
#endif // SL_CATALOG_BLUETOOTH_HCI_CPC_RTOS_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_RCP_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_HCI_UART_PRESENT) || defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
  sl_hci_uart_init();
#endif // SL_CATALOG_BLUETOOTH_HCI_UART_PRESENT || SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
#endif // SL_CATALOG_BLUETOOTH_RCP_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_HCI_TRANSPORT_PRESENT)
  hci_common_transport_init();
#endif // SL_CATALOG_BLUETOOTH_HCI_TRANSPORT_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_RCP_PRESENT)
  sl_bthci_init_upper();

#if !defined(SL_BLUETOOTH_NO_VS_HCI_COMMANDS)
  sl_bthci_init_vs();
#endif // SL_BLUETOOTH_NO_VS_HCI_COMMANDS

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EVENT_INFO_REPORTING_PRESENT)
  sl_btctrl_init_event_info_report();
#endif

  sl_btctrl_hci_parser_init_default();

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sl_btctrl_hci_parser_init_conn();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
  sl_btctrl_hci_parser_init_adv();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PHY_UPDATE_PRESENT)
  sl_btctrl_hci_parser_init_phy();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PHY_UPDATE_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PAST_PRESENT) \
  || defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PAST_PRESENT)      \
  || defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
  sl_btctrl_hci_parser_init_past();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PAST_PRESENT or
  // SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PAST_PRESENT or
  // SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT)
  sl_btctrl_hci_parser_init_privacy();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT) || defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
  sl_btctrl_hci_parser_init_cs();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT or SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT)
  sl_btctrl_hci_parser_init_subrate();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CHANNEL_CLASSIFICATION_PRESENT)
  sl_btctrl_hci_parser_init_channel_classification();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CHANNEL_CLASSIFICATION_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT)
  sl_btctrl_hci_parser_init_iso_cis();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT) || defined(SL_CATALOG_BLUETOOTH_FEATURE_BIS_PRESENT)
  sl_btctrl_hci_parser_init_iso_common();
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT or SL_CATALOG_BLUETOOTH_FEATURE_BIS_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_HCI_SLEEP_PRESENT)
  sl_btctrl_hci_sleep_init();
#endif // SL_CATALOG_BLUETOOTH_HCI_SLEEP_PRESENT
#endif // SL_CATALOG_BLUETOOTH_RCP_PRESENT

#if defined(SL_CATALOG_BLUETOOTH_CONTROLLER_ANCHOR_SELECTION_PRESENT)
#if (SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM == SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EVEN)
  sl_btctrl_init_even_anchor_selection();
#elif (SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM == SL_BTCTRL_ANCHOR_SELECTION_ALGORITHM_EMPTY_CENTER)
  sl_btctrl_init_empty_center_anchor_selection();
#endif
#endif // SL_CATALOG_BLUETOOTH_CONTROLLER_ANCHOR_SELECTION_PRESENT

#if SL_BT_CONTROLLER_USE_LEGACY_VENDOR_SPECIFIC_EVENT_CODE == 1
  sl_btctrl_init_hci_vs_legacy_event();
#endif // SL_BT_CONTROLLER_USE_LEGACY_VENDOR_SPECIFIC_EVENT_CODE
  sl_btctrl_debug_init(config);

  return status;
}

sl_status_t sl_btctrl_init(void)
{
  struct sl_btctrl_config config;

  sl_btctrl_init_config(&config);
  config.buffer_memory = SL_BT_CONTROLLER_BUFFER_MEMORY;

  return sl_btctrl_init_internal(&config);
}

void sl_btctrl_deinit(void)
{
  sl_btctrl_deinit_functional();
  sl_btctrl_deinit_mem();
#if defined(SL_CATALOG_SLEEPTIMER_PRESENT)
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  bool inaccurate_clock_enabled = sl_btctrl_get_config_flags(SL_BTCTRL_CONFIG_FLAG_USE_INACCURATE_SLEEP_CLOCK);
  if (!inaccurate_clock_enabled) {
    if (sl_sleeptimer_get_clock_accuracy() > SL_BTCTRL_SLEEP_CLOCK_ACCURACY_LIMIT) {
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
    }
  }
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
#endif // SL_CATALOG_SLEEPTIMER_PRESENT
}

void sl_btctrl_deinit_functional(void)
{
  if (sl_btctrl_is_initialized() == false) {
    return;
  }

  sl_btctrl_deinit_ll();

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
  sl_btctrl_deinit_sniff();
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT)
  sl_btctrl_allocate_resolving_list_memory(0);
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  sl_btctrl_alloc_periodic_scan(0);
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
  struct sl_btctrl_pawr_advertiser_config pawr_config = {
    .max_pawr_sets = 0,
    .max_advertised_data_length_hint = 0,
    .subevent_data_request_count = 0,
    .subevent_data_request_advance = 0,
  };
  (void) sl_btctrl_pawr_advertiser_configure(&pawr_config);
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_SYNC_PRESENT)
  struct sl_btctrl_pawr_synchronizer_config pawr_sync_config = {
    .max_pawr_sets = 0,
  };
  (void) sl_btctrl_pawr_synchronizer_configure(&pawr_sync_config);
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  (void) sl_btctrl_alloc_periodic_adv(0);
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT)
  struct sl_btctrl_iso_conn_config iso_conn_config = {
    .max_cises = 0,
    .max_cigs = 0,
  };
  (void) sl_btctrl_configure_iso_conn(&iso_conn_config);
#endif
}
