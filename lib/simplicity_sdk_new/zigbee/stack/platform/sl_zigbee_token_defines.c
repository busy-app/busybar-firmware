/***************************************************************************//**
 * @brief ZigBee token definition code.
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

#include "sl_zigbee_token.h"
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"
#include "sl_zigbee_types.h"
#include "multi-pan-token-config.h"
#include "stack-info.h"
#include "stack/include/binding-table.h"

#if !defined(EZSP_HOST) && (defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST)) && !defined(SL_ZIGBEE_AF_API_TOKEN) && !defined(ZIGBEE_PRO_COMPLIANCE_ON_HOST)
#include "sl_zigbee_green_power_config.h"
#endif // !defined(EZSP_HOST) && (defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST)) && !SL_ZIGBEE_AF_API_TOKEN

extern sl_status_t sl_zigbee_initialize_app_tokens(void);

sl_status_t halStackInitTokens(void)
{
  sl_status_t status = SL_STATUS_OK;
  status = sl_zigbee_initialize_app_tokens();
  assert(status == SL_STATUS_OK);

  tokTypeStackNvdataVersion tokTypeStackNvdataVersionDefault = TOKEN_STACK_NVDATA_VERSION_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_NVDATA_VERSION, &tokTypeStackNvdataVersionDefault, sizeof(tokTypeStackNvdataVersion));
  assert(status == SL_STATUS_OK);
  tokTypeStackNonceCounter tokTypeStackAPSFrameCounterDefault = TOKEN_STACK_APS_FRAME_COUNTER_DEFAULT;
  status = sl_zigbee_initialize_counter_token(COMMON_TOKEN_STACK_APS_FRAME_COUNTER, &tokTypeStackAPSFrameCounterDefault, sizeof(tokTypeStackNonceCounter));
  assert(status == SL_STATUS_OK);
  tokTypeStackKeys tokTypeStackAlternateKeysDefault = TOKEN_STACK_ALTERNATE_KEY_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_ALTERNATE_KEY, &tokTypeStackAlternateKeysDefault, sizeof(tokTypeStackKeys));
  assert(status == SL_STATUS_OK);
  tokTypeStackBootCounter tokTypeStackBootCounterDefault = TOKEN_STACK_BOOT_COUNTER_DEFAULT;
  status = sl_zigbee_initialize_counter_token(COMMON_TOKEN_STACK_BOOT_COUNTER, &tokTypeStackBootCounterDefault, sizeof(tokTypeStackBootCounter));
  assert(status == SL_STATUS_OK);
  tokTypeStackNonceCounter tokTypeStackNonceCounterDefault = TOKEN_STACK_NONCE_COUNTER_DEFAULT;
  status = sl_zigbee_initialize_counter_token(COMMON_TOKEN_STACK_NONCE_COUNTER, &tokTypeStackNonceCounterDefault, sizeof(tokTypeStackNonceCounter));
  assert(status == SL_STATUS_OK);
  tokTypeStackAnalysisReboot tokTypeStackAnalysisRebootDefault = TOKEN_STACK_ANALYSIS_REBOOT_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_ANALYSIS_REBOOT, &tokTypeStackAnalysisRebootDefault, sizeof(tokTypeStackAnalysisReboot));
  assert(status == SL_STATUS_OK);
  tokTypeStackKeys tokTypeStackKeysDefault = TOKEN_STACK_KEYS_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_KEYS, &tokTypeStackKeysDefault, sizeof(tokTypeStackKeys));
  assert(status == SL_STATUS_OK);
  tokTypeStackNodeData tokTypeStackNodeDataDefault = TOKEN_STACK_NODE_DATA_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_NODE_DATA, &tokTypeStackNodeDataDefault, sizeof(tokTypeStackNodeData));
  assert(status == SL_STATUS_OK);
  tokTypeStackTrustCenter tokTypeStackTrustCenterDefault = TOKEN_STACK_TRUST_CENTER_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_TRUST_CENTER, &tokTypeStackTrustCenterDefault, sizeof(tokTypeStackTrustCenter));
  assert(status == SL_STATUS_OK);
  tokTypeStackNetworkManagement tokTypeStackNetworkManagementDefault = TOKEN_STACK_NETWORK_MANAGEMENT_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_NETWORK_MANAGEMENT, &tokTypeStackNetworkManagementDefault, sizeof(tokTypeStackNetworkManagement));
  assert(status == SL_STATUS_OK);
  tokTypeStackParentInfo tokTypeStackParentInfoDefault = TOKEN_STACK_PARENT_INFO_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_PARENT_INFO, &tokTypeStackParentInfoDefault, sizeof(tokTypeStackParentInfo));
  assert(status == SL_STATUS_OK);
  tokTypeStackParentAdditionalInfo tokTypeStackParentAdditionalInfoDefault = TOKEN_STACK_PARENT_ADDITIONAL_INFO_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_PARENT_ADDITIONAL_INFO, &tokTypeStackParentAdditionalInfoDefault, sizeof(tokTypeStackParentAdditionalInfo));
  assert(status == SL_STATUS_OK);
  tokTypeStackMultiPhyNwkInfo tokTypeStackMultiPhyNwkInfoDefault = TOKEN_STACK_MULTI_PHY_NWK_INFO_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_MULTI_PHY_NWK_INFO, &tokTypeStackMultiPhyNwkInfoDefault, sizeof(tokTypeStackMultiPhyNwkInfo));
  assert(status == SL_STATUS_OK);
  tokTypeRSSI tokTypeRSSIDefault = TOKEN_STACK_MIN_RECEIVED_RSSI_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_MIN_RECEIVED_RSSI, &tokTypeRSSIDefault, sizeof(tokTypeRSSI));
  assert(status == SL_STATUS_OK);
  tokTypeStackRestoredEui64 tokTypeStackRestoredEui64Default = TOKEN_STACK_RESTORED_EUI64_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_RESTORED_EUI64, &tokTypeStackRestoredEui64Default, sizeof(tokTypeStackRestoredEui64));
  assert(status == SL_STATUS_OK);
#if defined (SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT) && defined (SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
  tokStackContext tokTypeStackContextDefault = TOKEN_STACK_CONTEXT_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_CONTEXT, &tokTypeStackContextDefault, sizeof(tokTypeStackContextDefault));
  assert(status == SL_STATUS_OK);
#endif // defined (SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT) && defined (SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT)
  tokStackTrustCenterAdditionalInfo tokStackTrustCenterAdditionalInfoDefault = TOKEN_STACK_TRUST_CENTER_ADDITIONAL_INFO_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_TRUST_CENTER_ADDITIONAL_INFO, &tokStackTrustCenterAdditionalInfoDefault, sizeof(tokStackTrustCenterAdditionalInfoDefault));
  assert(status == SL_STATUS_OK);
  // Multi-network stack tokens
#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
  uint8_t extra_networks_number = sl_zigbee_get_supported_networks() - 1;
  tokTypeStackKeys tokTypeMNStackKeysDefault = TOKEN_MULTI_NETWORK_STACK_KEYS_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_NETWORK_STACK_KEYS, &tokTypeMNStackKeysDefault, sizeof(tokTypeStackKeys), extra_networks_number);
  assert(status == SL_STATUS_OK);
  tokTypeStackNodeData tokTypeMNStackNodeDataDefault = TOKEN_MULTI_NETWORK_STACK_NODE_DATA_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_NETWORK_STACK_NODE_DATA, &tokTypeMNStackNodeDataDefault, sizeof(tokTypeStackNodeData), extra_networks_number);
  assert(status == SL_STATUS_OK);
  tokTypeStackKeys tokTypeMNAlternateStackKeysDefault = TOKEN_MULTI_NETWORK_STACK_ALTERNATE_KEY_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_NETWORK_STACK_ALTERNATE_KEY, &tokTypeMNAlternateStackKeysDefault, sizeof(tokTypeStackKeys), extra_networks_number);
  assert(status == SL_STATUS_OK);
  tokTypeStackTrustCenter tokTypeMNStackTrustCenterDefault = TOKEN_MULTI_NETWORK_STACK_TRUST_CENTER_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_NETWORK_STACK_TRUST_CENTER, &tokTypeMNStackTrustCenterDefault, sizeof(tokTypeStackTrustCenter), extra_networks_number);
  assert(status == SL_STATUS_OK);
  tokTypeStackNetworkManagement tokTypeMNStackNetworkManagementDefault = TOKEN_MULTI_NETWORK_STACK_NETWORK_MANAGEMENT_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_NETWORK_STACK_NETWORK_MANAGEMENT, &tokTypeMNStackNetworkManagementDefault, sizeof(tokTypeStackNetworkManagement), extra_networks_number);
  assert(status == SL_STATUS_OK);
  tokTypeStackParentInfo tokTypeMNStackParentInfoDefault = TOKEN_MULTI_NETWORK_STACK_PARENT_INFO_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_NETWORK_STACK_PARENT_INFO, &tokTypeMNStackParentInfoDefault, sizeof(tokTypeStackParentInfo), extra_networks_number);
  assert(status == SL_STATUS_OK);
#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
  // MULTI_NETWORK_STACK_NONCE_COUNTER was always defined, even if MN was not present
  tokTypeStackNonceCounter tokTypeMNStackNonceCounterDefault = TOKEN_MULTI_NETWORK_STACK_NONCE_COUNTER_DEFAULT;
  status = sl_zigbee_initialize_counter_token(COMMON_TOKEN_MULTI_NETWORK_STACK_NONCE_COUNTER, &tokTypeMNStackNonceCounterDefault, sizeof(tokTypeStackNonceCounter));
  assert(status == SL_STATUS_OK);
#endif  // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  tokTypeStackParentAdditionalInfo tokTypeMNStackParentAdditionalInfoDefault = TOKEN_MULTI_NETWORK_STACK_PARENT_ADDITIONAL_INFO_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_NETWORK_STACK_PARENT_ADDITIONAL_INFO, &tokTypeMNStackParentAdditionalInfoDefault, sizeof(tokTypeStackParentAdditionalInfo), extra_networks_number);
  assert(status == SL_STATUS_OK);
#endif  // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED

#if defined(EZSP_HOST)
  uint8_t binding_table_size = SL_ZIGBEE_BINDING_TABLE_SIZE;
#else
  uint8_t binding_table_size = sl_zigbee_get_binding_table_size();
#endif  // EZSP_HOST
  // App tokens
  tokTypeStackBindingTable tokTypeStackBindingTableDefault = TOKEN_STACK_BINDING_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_BINDING_TABLE, &tokTypeStackBindingTableDefault, sizeof(tokTypeStackBindingTable), binding_table_size);
  assert(status == SL_STATUS_OK);
  tokTypeStackChildTable tokTypeStackChildTableDefault = TOKEN_STACK_CHILD_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_CHILD_TABLE, &tokTypeStackChildTableDefault, sizeof(tokTypeStackChildTable), sl_zigbee_get_child_table_size());
  assert(status == SL_STATUS_OK);
#if !defined(EZSP_HOST)
  uint8_t original_key_table_max_size = sl_zigbee_get_key_table_size() < 0x80 ? sl_zigbee_get_key_table_size() : 0x7f;
  uint8_t extended_key_table_max_size = sl_zigbee_get_key_table_size() < 0x80 ? 0 : sl_zigbee_get_key_table_size() - 0x7f;
  tokTypeStackKeyTable tokTypeStackKeyTableDefault = TOKEN_STACK_KEY_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_KEY_TABLE, &tokTypeStackKeyTableDefault, sizeof(tokTypeStackKeyTable), original_key_table_max_size);
  assert(status == SL_STATUS_OK);
  tokTypeStackKeyTable tokTypeStackKeyTableExtendedDefault = TOKEN_STACK_KEY_TABLE_EXTENDED_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_KEY_TABLE_EXTENDED, &tokTypeStackKeyTableExtendedDefault, sizeof(tokTypeStackKeyTable), extended_key_table_max_size);
  assert(status == SL_STATUS_OK);
  tokStackKeyTableAdditionalInfo tokStackKeyTableAdditionalInfoDefault = TOKEN_STACK_KEY_TABLE_ADDITIONAL_INFO_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_KEY_TABLE_ADDITIONAL_INFO, &tokStackKeyTableAdditionalInfoDefault, sizeof(tokStackKeyTableAdditionalInfoDefault), sl_zigbee_get_key_table_size());
  assert(status == SL_STATUS_OK);
#endif // !defined(EZSP_HOST
  tokTypeStackCertificateTable tokTypeStackCertificateTableDefault = TOKEN_STACK_CERTIFICATE_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_CERTIFICATE_TABLE, &tokTypeStackCertificateTableDefault, sizeof(tokTypeStackCertificateTable), sl_zigbee_get_certificate_table_size());
  assert(status == SL_STATUS_OK);
  tokTypeStackAdditionalChildData tokTypeStackAdditionalChildDataDefault = TOKEN_STACK_ADDITIONAL_CHILD_DATA_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_ADDITIONAL_CHILD_DATA, &tokTypeStackAdditionalChildDataDefault, sizeof(tokTypeStackAdditionalChildData), sl_zigbee_get_child_table_size());
  assert(status == SL_STATUS_OK);

  // ZLL tokens
  tokTypeStackZllData tokTypeStackZllDataDefault = TOKEN_STACK_ZLL_DATA_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_ZLL_DATA, &tokTypeStackZllDataDefault, sizeof(tokTypeStackZllData));
  assert(status == SL_STATUS_OK);
  tokTypeStackZllSecurity tokTypeStackZllSecurityDefault = TOKEN_STACK_ZLL_SECURITY_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_ZLL_SECURITY, &tokTypeStackZllSecurityDefault, sizeof(tokTypeStackZllSecurity));
  assert(status == SL_STATUS_OK);

#if !defined(EZSP_HOST) && (defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST))
  tokTypeStackGpData tokTypeStackGpDataDefault = TOKEN_STACK_GP_DATA_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_STACK_GP_DATA, &tokTypeStackGpDataDefault, sizeof(tokTypeStackGpData));
  assert(status == SL_STATUS_OK);
  tokTypeStackGpProxyTableEntry tokTypeStackGpProxyTableEntryDefault = TOKEN_STACK_GP_PROXY_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_GP_PROXY_TABLE, &tokTypeStackGpProxyTableEntryDefault, sizeof(tokTypeStackGpProxyTableEntry), SL_ZIGBEE_GP_PROXY_TABLE_SIZE);
  assert(status == SL_STATUS_OK);
  tokTypeGPDIncomingFC tokTypeGPDIncomingFCDefault = TOKEN_STACK_GP_INCOMING_FC_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_GP_INCOMING_FC, &tokTypeGPDIncomingFCDefault, sizeof(tokTypeGPDIncomingFC), SL_ZIGBEE_GP_INCOMING_FC_TOKEN_TABLE_SIZE);
  assert(status == SL_STATUS_OK);
  tokTypeStackGpSinkTableEntry tokTypeStackGpSinkTableEntryDefault = TOKEN_STACK_GP_SINK_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_GP_SINK_TABLE, &tokTypeStackGpSinkTableEntryDefault, sizeof(tokTypeStackGpSinkTableEntry), SL_ZIGBEE_GP_SINK_TABLE_SIZE);
  assert(status == SL_STATUS_OK);
  tokTypeGPDIncomingFCInSink tokTypeGPDIncomingFCInSinkDefault = TOKEN_STACK_GP_INCOMING_FC_IN_SINK_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_STACK_GP_INCOMING_FC_IN_SINK, &tokTypeGPDIncomingFCInSinkDefault, sizeof(tokTypeGPDIncomingFCInSink), SL_ZIGBEE_GP_SINK_TABLE_SIZE);
  assert(status == SL_STATUS_OK);
#endif // !defined(EZSP_HOST) && (defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST))

  // Multi-pan tokens
  // We only reserve token space for multi PAN child table
  // when multiple ZC and ZR devices devices are present.
  uint8_t multi_pan_child_table_token_size = sl_zigbee_get_zc_and_zr_count() >= 1 ? sl_zigbee_get_child_table_size() : 0;
  tokTypeStackChildTable tokTypeMPStackChildTableDefault = TOKEN_MULTI_PAN_STACK_CHILD_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_PAN_STACK_CHILD_TABLE, &tokTypeMPStackChildTableDefault, sizeof(tokTypeStackChildTable), multi_pan_child_table_token_size);
  assert(status == SL_STATUS_OK);
  tokTypeStackAdditionalChildData tokTypeMPStackAdditionalChildDataDefault = TOKEN_MULTI_PAN_STACK_ADDITIONAL_CHILD_DATA_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_MULTI_PAN_STACK_ADDITIONAL_CHILD_DATA, &tokTypeMPStackAdditionalChildDataDefault, sizeof(tokTypeStackAdditionalChildData), multi_pan_child_table_token_size);
  assert(status == SL_STATUS_OK);
  return status;
}
