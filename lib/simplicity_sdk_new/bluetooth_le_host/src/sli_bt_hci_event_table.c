/***************************************************************************//**
 * @file
 * @brief Lookup table for mapping HCI events to the corresponding handlers
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <string.h>
#include "sl_status.h"
#include "sl_component_catalog.h"
#include "sli_bt_hci_event_table.h"

// Some features do not correspond directly to a particular component but are
// needed depending on a specific combination of components. Decide the derived
// feature selections here to simplify the feature inclusion rules below.

// CTE receiver is present if either AoA or AoD receiver is present
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_AOA_RECEIVER_PRESENT) \
  || defined(SL_CATALOG_BLUETOOTH_FEATURE_AOD_RECEIVER_PRESENT)
#define SLI_BT_CTE_RECEIVER_PRESENT
#endif

// -----------------------------------------------------------------------------
// Forward declaration of each HCI event handler function

extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_disconnection_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_encryption_change;
extern sli_bt_hci_event_handler_func_t sli_bt_resource_handle_hci_event_number_of_completed_packets;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_encryption_key_refresh_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_le_connection_update_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_le_read_remote_features_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_sm_handle_hci_event_le_ltk_request;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_le_data_length_change;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_le_phy_update_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_sync_handle_hci_event_le_periodic_advertising_sync_lost;
extern sli_bt_hci_event_handler_func_t sli_bt_gap_adv_handle_hci_event_le_advertising_set_terminated;
extern sli_bt_hci_event_handler_func_t sli_bt_advertiser_handle_hci_event_le_scan_request_received;
extern sli_bt_hci_event_handler_func_t sli_bt_cte_receiver_handle_hci_event_le_connectionless_iq_report;
extern sli_bt_hci_event_handler_func_t sli_bt_cte_receiver_handle_hci_event_le_connection_iq_report;
extern sli_bt_hci_event_handler_func_t sli_bt_cte_receiver_handle_hci_event_le_cte_request_failed;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_le_transmit_power_reporting;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_subrating_handle_hci_event_le_subrate_change;
extern sli_bt_hci_event_handler_func_t sli_bt_sync_scanner_handle_hci_event_le_periodic_advertising_sync_established_v2;
extern sli_bt_hci_event_handler_func_t sli_bt_sync_handle_hci_event_le_periodic_advertising_report_v2;
extern sli_bt_hci_event_handler_func_t sli_bt_past_receiver_handle_hci_event_le_periodic_advertising_sync_transfer_received_v2;
extern sli_bt_hci_event_handler_func_t sli_bt_pawr_advertiser_handle_hci_event_le_periodic_advertising_subevent_data_request;
extern sli_bt_hci_event_handler_func_t sli_bt_pawr_advertiser_handle_hci_event_le_periodic_advertising_response_report;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_handle_hci_event_le_enhanced_connection_complete_v2;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_sounding_handle_hci_event_le_cs_read_remote_supported_capabilities_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_sounding_handle_hci_event_le_cs_security_enable_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_sounding_handle_hci_event_le_cs_config_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_sounding_handle_hci_event_le_cs_procedure_enable_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_sounding_handle_hci_event_le_cs_subevent_result;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_sounding_handle_hci_event_le_cs_subevent_result_continue;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_sounding_test_handle_hci_event_le_cs_test_end_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_scanner_handle_hci_event_le_silabs_advertising_report;
extern sli_bt_hci_event_handler_func_t sli_bt_channel_classification_handle_hci_event_vs_silabs_channel_classification;
extern sli_bt_hci_event_handler_func_t sli_bt_past_receiver_handle_hci_event_vs_siliconlabs_periodic_advertising_sync_transfer_received;
extern sli_bt_hci_event_handler_func_t sli_bt_sm_handle_hci_event_le_silabs_sk_request;
extern sli_bt_hci_event_handler_func_t sli_bt_cte_receiver_handle_hci_event_le_silabs_iq_report;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_statistics_handle_hci_event_vs_siliconlabs_connection_statistics;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_analyzer_handle_hci_event_le_silabs_sniff_connection;
extern sli_bt_hci_event_handler_func_t sli_bt_connection_analyzer_handle_hci_event_le_silabs_sniff_complete;
extern sli_bt_hci_event_handler_func_t sli_bt_linklayer_handle_hci_event_vs_silabs_event_info_report;
extern sli_bt_hci_event_handler_func_t sli_bt_periodic_advertiser_handle_hci_event_le_silabs_periodic_advertising_tx;

// -----------------------------------------------------------------------------
// Definition of the HCI event lookup table used by the Bluetooth host stack

// Array of HCI event lookup keys
const sli_bt_hci_event_key_t sli_bt_hci_event_lookup_keys[] = {
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  0x0500,
  0x0800,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOURCE_REPORT_PRESENT)
  0x1300,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOURCE_REPORT_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  0x3000,
  0x3e03,
  0x3e04,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
  0x3e05,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  0x3e07,
  0x3e0c,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  0x3e10,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
  0x3e12,
  0x3e13,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
#if defined(SLI_BT_CTE_RECEIVER_PRESENT)
  0x3e15,
  0x3e16,
  0x3e17,
#endif // defined(SLI_BT_CTE_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  0x3e21,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT)
  0x3e23,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_SCANNER_PRESENT)
  0x3e24,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_SCANNER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  0x3e25,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
  0x3e26,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
  0x3e27,
  0x3e28,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  0x3e29,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT)
  0x3e2c,
  0x3e2e,
  0x3e2f,
  0x3e30,
  0x3e31,
  0x3e32,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
  0x3e33,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
  0xff0d,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CHANNEL_CLASSIFICATION_PRESENT)
  0xffeb,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CHANNEL_CLASSIFICATION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
  0xffec,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
  0xffed,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
#if defined(SLI_BT_CTE_RECEIVER_PRESENT)
  0xffee,
#endif // defined(SLI_BT_CTE_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_STATISTICS_PRESENT)
  0xffef,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_STATISTICS_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
  0xfff0,
  0xfff1,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_LINKLAYER_INTERFACE_PRESENT)
  0xfff2,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_LINKLAYER_INTERFACE_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  0xffff,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  0xffff // Unused stub entry to avoid an empty array
};

// Array of HCI event handler function pointers
sli_bt_hci_event_handler_func_t * const sli_bt_hci_event_lookup_data[] = {
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sli_bt_connection_handle_hci_event_disconnection_complete,
  sli_bt_connection_handle_hci_event_encryption_change,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOURCE_REPORT_PRESENT)
  sli_bt_resource_handle_hci_event_number_of_completed_packets,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOURCE_REPORT_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sli_bt_connection_handle_hci_event_encryption_key_refresh_complete,
  sli_bt_connection_handle_hci_event_le_connection_update_complete,
  sli_bt_connection_handle_hci_event_le_read_remote_features_complete,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
  sli_bt_sm_handle_hci_event_le_ltk_request,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sli_bt_connection_handle_hci_event_le_data_length_change,
  sli_bt_connection_handle_hci_event_le_phy_update_complete,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  sli_bt_sync_handle_hci_event_le_periodic_advertising_sync_lost,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
  sli_bt_gap_adv_handle_hci_event_le_advertising_set_terminated,
  sli_bt_advertiser_handle_hci_event_le_scan_request_received,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
#if defined(SLI_BT_CTE_RECEIVER_PRESENT)
  sli_bt_cte_receiver_handle_hci_event_le_connectionless_iq_report,
  sli_bt_cte_receiver_handle_hci_event_le_connection_iq_report,
  sli_bt_cte_receiver_handle_hci_event_le_cte_request_failed,
#endif // defined(SLI_BT_CTE_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sli_bt_connection_handle_hci_event_le_transmit_power_reporting,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT)
  sli_bt_connection_subrating_handle_hci_event_le_subrate_change,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_SCANNER_PRESENT)
  sli_bt_sync_scanner_handle_hci_event_le_periodic_advertising_sync_established_v2,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_SCANNER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  sli_bt_sync_handle_hci_event_le_periodic_advertising_report_v2,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
  sli_bt_past_receiver_handle_hci_event_le_periodic_advertising_sync_transfer_received_v2,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
  sli_bt_pawr_advertiser_handle_hci_event_le_periodic_advertising_subevent_data_request,
  sli_bt_pawr_advertiser_handle_hci_event_le_periodic_advertising_response_report,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sli_bt_connection_handle_hci_event_le_enhanced_connection_complete_v2,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT)
  sli_bt_channel_sounding_handle_hci_event_le_cs_read_remote_supported_capabilities_complete,
  sli_bt_channel_sounding_handle_hci_event_le_cs_security_enable_complete,
  sli_bt_channel_sounding_handle_hci_event_le_cs_config_complete,
  sli_bt_channel_sounding_handle_hci_event_le_cs_procedure_enable_complete,
  sli_bt_channel_sounding_handle_hci_event_le_cs_subevent_result,
  sli_bt_channel_sounding_handle_hci_event_le_cs_subevent_result_continue,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
  sli_bt_channel_sounding_test_handle_hci_event_le_cs_test_end_complete,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
  sli_bt_scanner_handle_hci_event_le_silabs_advertising_report,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CHANNEL_CLASSIFICATION_PRESENT)
  sli_bt_channel_classification_handle_hci_event_vs_silabs_channel_classification,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CHANNEL_CLASSIFICATION_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
  sli_bt_past_receiver_handle_hci_event_vs_siliconlabs_periodic_advertising_sync_transfer_received,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
  sli_bt_sm_handle_hci_event_le_silabs_sk_request,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
#if defined(SLI_BT_CTE_RECEIVER_PRESENT)
  sli_bt_cte_receiver_handle_hci_event_le_silabs_iq_report,
#endif // defined(SLI_BT_CTE_RECEIVER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_STATISTICS_PRESENT)
  sli_bt_connection_statistics_handle_hci_event_vs_siliconlabs_connection_statistics,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_STATISTICS_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
  sli_bt_connection_analyzer_handle_hci_event_le_silabs_sniff_connection,
  sli_bt_connection_analyzer_handle_hci_event_le_silabs_sniff_complete,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_LINKLAYER_INTERFACE_PRESENT)
  sli_bt_linklayer_handle_hci_event_vs_silabs_event_info_report,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_LINKLAYER_INTERFACE_PRESENT)
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  sli_bt_periodic_advertiser_handle_hci_event_le_silabs_periodic_advertising_tx,
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  NULL // Unused stub entry to avoid an empty array
};

// Number of used entries in the HCI event lookup table. The stub entry that we
// use to avoid an empty array is excluded from the number of entries.
const uint8_t sli_bt_hci_event_lookup_num_entries = sizeof(sli_bt_hci_event_lookup_keys) / sizeof(sli_bt_hci_event_lookup_keys[0]) - 1;