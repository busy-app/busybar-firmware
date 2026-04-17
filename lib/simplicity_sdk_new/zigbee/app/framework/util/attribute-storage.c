/***************************************************************************//**
 * @file
 * @brief Contains the per-endpoint configuration of attribute tables.
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
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/common.h"
#include "stack/config/sl_zigbee_token_defines.h"
#ifndef SL_ZIGBEE_SCRIPTED_TEST
#include "zigbee_zcl_callback_dispatcher.h"
#include "zigbee_af_cluster_functions.h"
#else
extern void sli_zigbee_af_reset_attributes(uint8_t endpointId);
#undef ZCL_FIXED_ENDPOINT_COUNT
#define ZCL_FIXED_ENDPOINT_COUNT (10)
#endif

#if defined(GENERATED_MULTI_PROTOCOL_ATTRIBUTE_MAPPING) && defined(SL_CATALOG_MULTIPROTOCOL_ZIGBEE_MATTER_COMMON_PRESENT) && defined(GENERATED_MULTI_PROTOCOL_CLUSTER_MAPPING)
#include "sl-matter-attribute-storage.h"
#endif // defined(GENERATED_MULTI_PROTOCOL_ATTRIBUTE_MAPPING) && defined(SL_CATALOG_MULTIPROTOCOL_ZIGBEE_MATTER_COMMOON_PRESENT) && defined(GENERATED_MULTI_PROTOCOL_CLUSTER_MAPPING)
//------------------------------------------------------------------------------
// Globals
// This is not declared CONST in order to handle dynamic endpoint information
// retrieved from tokens.
sl_zigbee_af_defined_endpoint_t sli_zigbee_af_endpoints[MAX_ENDPOINT_COUNT];

#if (ZCL_ATTRIBUTE_MAX_SIZE == 0)
#define ACTUAL_ATTRIBUTE_SIZE 1
#else
#define ACTUAL_ATTRIBUTE_SIZE ZCL_ATTRIBUTE_MAX_SIZE
#endif

uint8_t sl_zigbee_attribute_data[ACTUAL_ATTRIBUTE_SIZE];

#if (!defined(ZCL_ATTRIBUTE_SINGLETONS_SIZE)) \
  || (ZCL_ATTRIBUTE_SINGLETONS_SIZE == 0)
#define ACTUAL_SINGLETONS_SIZE 1
#else
#define ACTUAL_SINGLETONS_SIZE ZCL_ATTRIBUTE_SINGLETONS_SIZE
#endif
uint8_t singletonAttributeData[ACTUAL_SINGLETONS_SIZE];

#if defined(SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT) && !defined(SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT)
uint8_t sl_zigbee_endpoint_count = 0;
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#if defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
extern uint8_t sl_zigbee_ncp_endpoint_count;
#ifdef sl_zigbee_endpoint_count
#undef sl_zigbee_endpoint_count
#endif // sl_zigbee_endpoint_count
#define sl_zigbee_endpoint_count sl_zigbee_ncp_endpoint_count
#else // !(defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT))
extern uint8_t sl_zigbee_endpoint_count;
#endif // defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

// If we have attributes that are more than 2 bytes, then
// we need this data block for the defaults
#if (ZCL_GENERATED_DEFAULTS_COUNT > 0)
const uint8_t generatedDefaults[]               = ZCL_GENERATED_DEFAULTS;
#endif // ZCL_GENERATED_DEFAULTS

#ifdef ZCL_GENERATED_MIN_MAX_DEFAULTS
// App Builder does not define ZCL_GENERATED_MIN_MAX_DEFAULT_COUNT, while
// zap does, but if the defined value is 0, it means ZCL_GENERATED_MIN_MAX_DEFAULTS
// is empty and we should not define minMaxDefaults
#if !defined(ZCL_GENERATED_MIN_MAX_DEFAULT_COUNT) || (ZCL_GENERATED_MIN_MAX_DEFAULT_COUNT > 0)
const sl_zigbee_af_attribute_min_max_value_t minMaxDefaults[]          = ZCL_GENERATED_MIN_MAX_DEFAULTS;
#endif // !ZCL_GENERATED_MIN_MAX_DEFAULTS || ZCL_GENERATED_MIN_MAX_DEFAULTS > 0
#endif // ZCL_GENERATED_MIN_MAX_DEFAULTS

#ifndef SL_ZIGBEE_SCRIPTED_TEST
const sli_cluster_function_structure_t generatedClusterFunctions[] = GENERATED_FUNCTION_STRUCTURES_ARRAY;
#else
//unused?
const sli_cluster_function_structure_t generatedClusterFunctions[1];
#endif

#ifdef SL_ZIGBEE_AF_SUPPORT_COMMAND_DISCOVERY
#if (SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT > 0)
const sl_zigbee_af_command_metadata_t generatedCommands[] = ZCL_GENERATED_COMMANDS;
#endif // SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT
#if (ZCL_GENERATED_COMMAND_MANUFACTURER_CODE_COUNT > 0)
const sl_zigbee_af_manufacturer_code_entry_t commandManufacturerCodes[] = ZCL_GENERATED_COMMAND_MANUFACTURER_CODES;
const uint16_t commandManufacturerCodeCount = ZCL_GENERATED_COMMAND_MANUFACTURER_CODE_COUNT;
#endif // ZCL_GENERATED_COMMAND_MANUFACTURER_CODE_COUNT
#endif // SL_ZIGBEE_AF_SUPPORT_COMMAND_DISCOVERY

const sl_zigbee_af_attribute_metadata_t generatedAttributes[] = ZCL_GENERATED_ATTRIBUTES;
// Attribute map between zigbee and matter.
#if defined(GENERATED_MULTI_PROTOCOL_ATTRIBUTE_MAPPING) && defined(SL_CATALOG_MULTIPROTOCOL_ZIGBEE_MATTER_COMMON_PRESENT) && defined(GENERATED_MULTI_PROTOCOL_CLUSTER_MAPPING)
const sl_zigbee_matter_af_multi_protocol_attribute_metadata_t multiProtocolAttributeMap[] = GENERATED_MULTI_PROTOCOL_ATTRIBUTE_MAPPING;
const sl_zigbee_matter_af_multi_protocol_cluster_metadata_t multiProtocolClusterMap[] = GENERATED_MULTI_PROTOCOL_CLUSTER_MAPPING;
const uint8_t mappedMpClusterCount = (sizeof(multiProtocolClusterMap) / sizeof(sl_zigbee_matter_af_multi_protocol_cluster_metadata_t));
#endif // defined(GENERATED_MULTI_PROTOCOL_ATTRIBUTE_MAPPING) && defined(SL_CATALOG_MULTIPROTOCOL_ZIGBEE_MATTER_COMMON_PRESENT) && defined(GENERATED_MULTI_PROTOCOL_CLUSTER_MAPPING)
const sl_zigbee_af_cluster_t generatedClusters[]          = ZCL_GENERATED_CLUSTERS;
const sl_zigbee_af_endpoint_type_t generatedEmberAfEndpointTypes[]   = ZCL_GENERATED_ENDPOINT_TYPES;

#if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT) || defined(SL_CATALOG_ZIGBEE_DYNAMIC_NODE_TYPE_PRESENT)
sli_zigbee_af_zigbee_pro_network sli_zigbee_af_zigbee_pro_networks[] =
{ { SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE, SLI_ZIGBEE_PRIMARY_NETWORK_SECURITY_TYPE } };
#else
#ifndef SL_ZIGBEE_SCRIPTED_TEST
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#if (SL_ZIGBEE_SUPPORTED_NETWORKS == 1)
const sli_zigbee_af_zigbee_pro_network sli_zigbee_af_zigbee_pro_networks[] =
{ { SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE, SLI_ZIGBEE_PRIMARY_NETWORK_SECURITY_TYPE } };
#elif (SL_ZIGBEE_SUPPORTED_NETWORKS == 2)
const sli_zigbee_af_zigbee_pro_network sli_zigbee_af_zigbee_pro_networks[] =
{ { SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE, SLI_ZIGBEE_PRIMARY_NETWORK_SECURITY_TYPE },
  { SLI_ZIGBEE_SECONDARY_NETWORK_DEVICE_TYPE, SLI_ZIGBEE_SECONDARY_NETWORK_SECURITY_TYPE } };
#else
  #error "wrong network count"
#endif
#endif // SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#else
const sli_zigbee_af_zigbee_pro_network sli_zigbee_af_zigbee_pro_networks[] = { SL_ZIGBEE_COORDINATOR, SL_ZIGBEE_AF_SECURITY_PROFILE_HA };
#endif
#endif
const sl_zigbee_af_manufacturer_code_entry_t clusterManufacturerCodes[] = ZCL_GENERATED_CLUSTER_MANUFACTURER_CODES;
const uint16_t clusterManufacturerCodeCount = ZCL_GENERATED_CLUSTER_MANUFACTURER_CODE_COUNT;
const sl_zigbee_af_manufacturer_code_entry_t attributeManufacturerCodes[] = ZCL_GENERATED_ATTRIBUTE_MANUFACTURER_CODES;
const uint16_t attributeManufacturerCodeCount = ZCL_GENERATED_ATTRIBUTE_MANUFACTURER_CODE_COUNT;

#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
// All generated clusters are enabled by default
static uint8_t clusterExtendedData[COUNTOF(generatedClusters)];
#define CLUSTER_MASK_DISABLED (0x01u)
#define CLUSTER_MASK_ENABLED  (0x00u)
#define sl_zigbee_af_cluster_is_enabled(clusterData) ((bool)((clusterData & CLUSTER_MASK_DISABLED) == 0))
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME

#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
#define endpointNumber(x)        fixedEndpoints[x]
#define endpointProfileId(x)     fixedProfileIds[x]
#define endpointDeviceId(x)      fixedDeviceIds[x]
#define endpointDeviceVersion(x) fixedDeviceVersions[x]
// Added 'Macro' to silence MISRA warning about conflict with synonymous vars.
#define endpointTypeMacro(x)     (sl_zigbee_af_endpoint_type_t*)&(generatedEmberAfEndpointTypes[fixedEmberAfEndpointTypes[x]])
#define endpointNetworkIndex(x)  fixedNetworks[x]
#else
#if !defined (__ATTRIBUTE_STORAGE_TEST__)
#define endpointNumber(index) (50 + index)
#define endpointTypeMacro(index) ((sl_zigbee_af_endpoint_type_t*)&(generatedEmberAfEndpointTypes[0]))
#define endpointNetworkIndex(index) (0)
#define endpointProfileId(index) (0xABBA)
#define endpointDeviceId(index) (0xBEEF)
#define endpointDeviceVersion(index) (0xBA)
#endif
#endif

//------------------------------------------------------------------------------
// Forward declarations

// Returns endpoint index within a given cluster
static uint8_t findClusterEndpointIndex(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, uint8_t mask, uint16_t manufacturerCode);

#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
// Returns cluster index in generatedClusters using given cluster
static uint8_t findClusterIdxInGeneratedList(sl_zigbee_af_cluster_t *cluster);
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME
//------------------------------------------------------------------------------

// Initial configuration
void sl_zigbee_af_endpoint_configure(void)
{
  uint8_t ep;

#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
  uint8_t fixedEndpoints[] = ZCL_FIXED_ENDPOINT_ARRAY;
  uint16_t fixedProfileIds[] = ZCL_FIXED_PROFILE_IDS;
  uint16_t fixedDeviceIds[] = ZCL_FIXED_DEVICE_IDS;
  uint8_t fixedDeviceVersions[] = ZCL_FIXED_DEVICE_VERSIONS;
  uint8_t fixedEmberAfEndpointTypes[] = ZCL_FIXED_ENDPOINT_TYPES;
  uint8_t fixedNetworks[] = ZCL_FIXED_NETWORKS;
#endif

  sl_zigbee_endpoint_count = MAX_ENDPOINT_COUNT;
  for ( ep = 0; ep < sl_zigbee_endpoint_count; ep++ ) {
    sli_zigbee_af_endpoints[ep].endpoint      = endpointNumber(ep);
    sli_zigbee_af_endpoints[ep].profileId     = endpointProfileId(ep);
    sli_zigbee_af_endpoints[ep].deviceId      = endpointDeviceId(ep);
    sli_zigbee_af_endpoints[ep].deviceVersion = endpointDeviceVersion(ep);
    sli_zigbee_af_endpoints[ep].endpointType  = endpointTypeMacro(ep);
    sli_zigbee_af_endpoints[ep].networkIndex  = endpointNetworkIndex(ep);
    sli_zigbee_af_endpoints[ep].bitmask = SL_ZIGBEE_AF_ENDPOINT_ENABLED;
  }
}

void sl_zigbee_af_set_endpoint_count(uint8_t dynamicEndpointCount)
{
  sl_zigbee_endpoint_count = ZCL_FIXED_ENDPOINT_COUNT + dynamicEndpointCount;
}

uint8_t sl_zigbee_af_fixed_endpoint_count(void)
{
  return ZCL_FIXED_ENDPOINT_COUNT;
}

uint8_t sl_zigbee_af_endpoint_count(void)
{
  return sl_zigbee_endpoint_count;
}

bool sl_zigbee_af_endpoint_index_is_enabled(uint8_t index)
{
  return (sli_zigbee_af_endpoints[index].bitmask & SL_ZIGBEE_AF_ENDPOINT_ENABLED);
}

// some data types (like strings) are sent OTA in human readable order
// (how they are read) instead of little endian as the data types are.
bool sl_zigbee_af_is_this_data_type_a_string_type(sl_zigbee_af_attribute_type_t dataType)
{
  return (dataType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE
          || dataType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool sl_zigbee_af_is_string_attribute_type(sl_zigbee_af_attribute_type_t attributeType)
{
  return (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE
          || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool sl_zigbee_af_is_long_string_attribute_type(sl_zigbee_af_attribute_type_t attributeType)
{
  return (attributeType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE
          || attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

// This function is used to call the per-cluster default response callback
void sl_zigbee_af_cluster_default_response_with_mfg_code_cb(uint8_t endpoint,
                                                            sl_zigbee_af_cluster_id_t clusterId,
                                                            uint8_t commandId,
                                                            sl_zigbee_af_status_t status,
                                                            uint8_t clientServerMask,
                                                            uint16_t manufacturerCode)
{
  sl_zigbee_af_cluster_t *cluster = sl_zigbee_af_find_cluster_with_mfg_code(endpoint,
                                                                            clusterId,
                                                                            clientServerMask,
                                                                            manufacturerCode);
  if (cluster != NULL) {
    sl_zigbee_af_generic_cluster_function_t f =
      sl_zigbee_af_find_cluster_function(cluster,
                                         CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION);
    if (f != NULL) {
      sl_zigbee_af_push_endpoint_network_index(endpoint);
      ((sl_zigbee_af_default_response_function_t)f)(endpoint, commandId, status);
      (void) sl_zigbee_af_pop_network_index();
    }
  }
}

// This function is used to call the per-cluster default response callback, and
// wraps the sl_zigbee_af_cluster_default_response_with_mfg_code_cb with a
// SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE.
void sl_zigbee_af_cluster_default_response_cb(uint8_t endpoint,
                                              sl_zigbee_af_cluster_id_t clusterId,
                                              uint8_t commandId,
                                              sl_zigbee_af_status_t status,
                                              uint8_t clientServerMask)
{
  sl_zigbee_af_cluster_default_response_with_mfg_code_cb(endpoint,
                                                         clusterId,
                                                         commandId,
                                                         status,
                                                         clientServerMask,
                                                         SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// This function is used to call the per-cluster message sent callback
void sl_zigbee_af_cluster_message_sent_with_mfg_code_cb(sl_zigbee_outgoing_message_type_t type,
                                                        uint16_t indexOrDestination,
                                                        sl_zigbee_aps_frame_t *apsFrame,
                                                        uint16_t msgLen,
                                                        uint8_t *message,
                                                        sl_status_t status,
                                                        uint16_t mfgCode)
{
  if (apsFrame != NULL && message != NULL && msgLen != 0) {
    sl_zigbee_af_cluster_t *cluster = sl_zigbee_af_find_cluster_with_mfg_code(apsFrame->sourceEndpoint,
                                                                              apsFrame->clusterId,
                                                                              (((message[0]
                                                                                 & ZCL_FRAME_CONTROL_DIRECTION_MASK)
                                                                                == ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)
                                                                               ? CLUSTER_MASK_SERVER
                                                                               : CLUSTER_MASK_CLIENT),
                                                                              mfgCode);
    if (cluster != NULL) {
      sl_zigbee_af_generic_cluster_function_t f =
        sl_zigbee_af_find_cluster_function(cluster,
                                           CLUSTER_MASK_MESSAGE_SENT_FUNCTION);
      if (f != NULL) {
        sl_zigbee_af_push_endpoint_network_index(apsFrame->sourceEndpoint);
        ((sl_zigbee_af_message_sent_function_t)f)(type,
                                                  indexOrDestination,
                                                  apsFrame,
                                                  msgLen,
                                                  message,
                                                  status);
        (void) sl_zigbee_af_pop_network_index();
      }
    }
  }
}

// This function is used to call the per-cluster message sent callback, and
// wraps the sl_zigbee_af_cluster_message_sent_with_mfg_code_cb with a
// SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE.
void sl_zigbee_af_cluster_message_sent_cb(sl_zigbee_outgoing_message_type_t type,
                                          uint16_t indexOrDestination,
                                          sl_zigbee_aps_frame_t *apsFrame,
                                          uint16_t msgLen,
                                          uint8_t *message,
                                          sl_status_t status)
{
  sl_zigbee_af_cluster_message_sent_with_mfg_code_cb(type,
                                                     indexOrDestination,
                                                     apsFrame,
                                                     msgLen,
                                                     message,
                                                     status,
                                                     SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// This function is used to call the per-cluster attribute changed callback
void sli_zigbee_af_cluster_attribute_changed_callback(uint8_t endpoint,
                                                      sl_zigbee_af_cluster_id_t clusterId,
                                                      sl_zigbee_af_attribute_id_t attributeId,
                                                      uint8_t clientServerMask,
                                                      uint16_t manufacturerCode)
{
  sl_zigbee_af_cluster_t *cluster = sl_zigbee_af_find_cluster_with_mfg_code(endpoint,
                                                                            clusterId,
                                                                            clientServerMask,
                                                                            manufacturerCode);
  if (cluster != NULL) {
    if (manufacturerCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) {
      sl_zigbee_af_generic_cluster_function_t f =
        sl_zigbee_af_find_cluster_function(cluster,
                                           CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION);
      if (f != NULL) {
        sl_zigbee_af_push_endpoint_network_index(endpoint);
        ((sl_zigbee_af_cluster_attribute_changed_callback_t)f)(endpoint, attributeId);
        (void) sl_zigbee_af_pop_network_index();
      }
    } else {
      sl_zigbee_af_generic_cluster_function_t f =
        sl_zigbee_af_find_cluster_function(cluster,
                                           CLUSTER_MASK_MANUFACTURER_SPECIFIC_ATTRIBUTE_CHANGED_FUNCTION);
      if (f != NULL) {
        sl_zigbee_af_push_endpoint_network_index(endpoint);
        ((sl_zigbee_af_manufacturer_specific_cluster_attribute_changed_callback_t)f)(endpoint,
                                                                                     attributeId,
                                                                                     manufacturerCode);
        (void) sl_zigbee_af_pop_network_index();
      }
    }
  }
}

// This function is used to call the per-cluster pre-attribute changed callback
sl_zigbee_af_status_t sli_zigbee_af_cluster_pre_attribute_changed_callback(uint8_t endpoint,
                                                                           sl_zigbee_af_cluster_id_t clusterId,
                                                                           sl_zigbee_af_attribute_id_t attributeId,
                                                                           uint8_t clientServerMask,
                                                                           uint16_t manufacturerCode,
                                                                           sl_zigbee_af_attribute_type_t attributeType,
                                                                           uint8_t size,
                                                                           uint8_t* value)
{
  sl_zigbee_af_cluster_t *cluster = sl_zigbee_af_find_cluster_with_mfg_code(endpoint,
                                                                            clusterId,
                                                                            clientServerMask,
                                                                            manufacturerCode);
  if (cluster == NULL) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  } else {
    sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    if (manufacturerCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) {
      sl_zigbee_af_generic_cluster_function_t f =
        sl_zigbee_af_find_cluster_function(cluster,
                                           CLUSTER_MASK_PRE_ATTRIBUTE_CHANGED_FUNCTION);
      if (f != NULL) {
        sl_zigbee_af_push_endpoint_network_index(endpoint);
        status = ((sl_zigbee_af_cluster_pre_attribute_changed_callback_t)f)(endpoint,
                                                                            attributeId,
                                                                            attributeType,
                                                                            size,
                                                                            value);
        (void) sl_zigbee_af_pop_network_index();
      }
    }
    return status;
  }
}

static void initializeEndpoint(sl_zigbee_af_defined_endpoint_t* definedEndpoint)
{
  uint8_t clusterIndex;
  sl_zigbee_af_endpoint_type_t* epType = definedEndpoint->endpointType;
  sl_zigbee_af_push_endpoint_network_index(definedEndpoint->endpoint);
  for ( clusterIndex = 0;
        clusterIndex < epType->clusterCount;
        clusterIndex++ ) {
    sl_zigbee_af_cluster_t *cluster = &(epType->cluster[clusterIndex]);
    sl_zigbee_af_generic_cluster_function_t f;
    sl_zigbee_af_cluster_init_cb(definedEndpoint->endpoint, cluster->clusterId);
    f = sl_zigbee_af_find_cluster_function(cluster, CLUSTER_MASK_INIT_FUNCTION);
    if ( f != NULL ) {
      ((sl_zigbee_af_init_function_t)f)(definedEndpoint->endpoint);
    }
  }
  (void) sl_zigbee_af_pop_network_index();
}

// Calls the init functions.
void sli_zigbee_af_call_inits(void)
{
  uint8_t index;
  for ( index = 0; index < sl_zigbee_af_endpoint_count(); index++ ) {
    if (sl_zigbee_af_endpoint_index_is_enabled(index)) {
      initializeEndpoint(&(sli_zigbee_af_endpoints[index]));
    }
  }
}

// Returns the pointer to metadata, or null if it is not found
sl_zigbee_af_attribute_metadata_t *sl_zigbee_af_locate_attribute_metadata(uint8_t endpoint,
                                                                          sl_zigbee_af_cluster_id_t clusterId,
                                                                          sl_zigbee_af_attribute_id_t attributeId,
                                                                          uint8_t mask,
                                                                          uint16_t manufacturerCode)
{
  sl_zigbee_af_attribute_metadata_t *metadata = NULL;
  sl_zigbee_af_attribute_search_record_t record;
  record.endpoint = endpoint;
  record.clusterId = clusterId;
  record.clusterMask = mask;
  record.attributeId = attributeId;
  record.manufacturerCode = manufacturerCode;
  sli_zigbee_af_read_attribute_from_storage(&record,
                                            &metadata,
                                            NULL, // buffer
                                            0); // buffer size
  return metadata;
}

static uint8_t* singletonAttributeLocation(sl_zigbee_af_attribute_metadata_t *am)
{
  sl_zigbee_af_attribute_metadata_t *m = (sl_zigbee_af_attribute_metadata_t *)&(generatedAttributes[0]);
  uint16_t index = 0;
  while ( m < am ) {
    if ((m->mask & ATTRIBUTE_MASK_SINGLETON) != 0U) {
      index += m->size;
    }
    m++;
  }
  return (uint8_t *)(singletonAttributeData + index);
}

// This function does mem copy, but smartly, which means that if the type is a
// string, it will copy as much as it can.
// If src == NULL, then this method will set memory to zeroes
static sl_zigbee_af_status_t typeSensitiveMemCopy(uint8_t* dest,
                                                  uint8_t* src,
                                                  sl_zigbee_af_attribute_metadata_t * am,
                                                  bool write,
                                                  uint16_t attributeReadDestBufferSize)
{
  if (dest == NULL) {
    // no destination buffer provided, return error
    return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  sl_zigbee_af_attribute_type_t attributeType = am->attributeType;
  uint16_t size = (attributeReadDestBufferSize == 0) ? am->size : attributeReadDestBufferSize;

  // String attribute: Copy[Long]String function will manage string size appropriately.
  // Non-string attribute: limit set/move to attribute size as specified in attribute metadata.
  if (sl_zigbee_af_is_string_attribute_type(attributeType)) {
    sl_zigbee_af_copy_string(dest, src, size - 1);
  } else if (sl_zigbee_af_is_long_string_attribute_type(attributeType)) {
    sl_zigbee_af_copy_long_string(dest, src, size - 2);
  } else {
    if (!write && attributeReadDestBufferSize != 0 && attributeReadDestBufferSize < am->size) {
      return SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
    if ( src == NULL ) {
      memset(dest, 0, am->size);
    } else {
      memmove(dest, src, am->size);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

// Returns the manufacturer code or ::SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE if none
// could be found.
static uint16_t getManufacturerCode(sl_zigbee_af_manufacturer_code_entry_t *codes,
                                    uint16_t codeTableSize,
                                    uint16_t tableIndex)
{
  uint16_t i;
  for (i = 0; i < codeTableSize; i++) {
    if (codes->index == tableIndex) {
      return codes->manufacturerCode;
    }
    codes++;
  }
  return SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE;
}

// This function basically wraps getManufacturerCode with the parameters
// associating an attributes metadata with its code.
uint16_t sl_zigbee_af_get_mfg_code(sl_zigbee_af_attribute_metadata_t *metadata)
{
  return getManufacturerCode((sl_zigbee_af_manufacturer_code_entry_t *) attributeManufacturerCodes,
                             attributeManufacturerCodeCount,
                             (metadata - generatedAttributes));
}

uint16_t sli_zigbee_af_get_manufacturer_code_for_attribute(sl_zigbee_af_cluster_t *cluster,
                                                           sl_zigbee_af_attribute_metadata_t *attMetaData)
{
  return (sl_zigbee_af_cluster_is_manufacturer_specific(cluster)
          ? sli_zigbee_af_get_manufacturer_code_for_cluster(cluster)
          : sl_zigbee_af_get_mfg_code(attMetaData));
}

uint16_t sli_zigbee_af_get_manufacturer_code_for_cluster(sl_zigbee_af_cluster_t *cluster)
{
  return getManufacturerCode((sl_zigbee_af_manufacturer_code_entry_t *)clusterManufacturerCodes,
                             clusterManufacturerCodeCount,
                             (cluster - generatedClusters));
}

/**
 * @brief Matches a cluster based on cluster id, direction and manufacturer code.
 *   This function assumes that the passed cluster's endpoint already
 *   matches the endpoint of the sl_zigbee_af_attribute_search_record_t.
 *
 * Cluster's match if:
 *   1. Cluster ids match AND
 *   2. Cluster directions match as defined by cluster->mask
 *        and attRecord->clusterMask AND
 *   3. If the clusters are mf specific, their mfg codes match.
 */
bool sli_zigbee_af_match_cluster(sl_zigbee_af_cluster_t *cluster,
                                 sl_zigbee_af_attribute_search_record_t *attRecord)
{
  return (cluster->clusterId == attRecord->clusterId
          && cluster->mask & attRecord->clusterMask
          && (!sl_zigbee_af_cluster_is_manufacturer_specific(cluster)
              || (sli_zigbee_af_get_manufacturer_code_for_cluster(cluster)
                  == attRecord->manufacturerCode)));
}

/**
 * @brief Matches an attribute based on attribute id and manufacturer code.
 *   This function assumes that the passed cluster already matches the
 *   clusterId, direction and mf specificity of the passed
 *   sl_zigbee_af_attribute_search_record_t.
 *
 * Note: If both the attribute and cluster are manufacturer specific,
 *   the cluster's mf code gets precedence.
 *
 * Attributes match if:
 *   1. Att ids match AND
 *      a. cluster IS mf specific OR
 *      b. both stored and saught attributes are NOT mf specific OR
 *      c. stored att IS mf specific AND mfg codes match.
 */
bool sli_zigbee_af_match_attribute(sl_zigbee_af_cluster_t *cluster,
                                   sl_zigbee_af_attribute_metadata_t *am,
                                   sl_zigbee_af_attribute_search_record_t *attRecord)
{
  return (am->attributeId == attRecord->attributeId
          && (sl_zigbee_af_cluster_is_manufacturer_specific(cluster)
              || (sli_zigbee_af_get_manufacturer_code_for_attribute(cluster, am)
                  == attRecord->manufacturerCode)));
}

/**
 * @brief Retrieves the endpoint type and storage offset for a given endpoint.
 *
 * This function determines the endpoint type and calculates the offset
 * for attributes associated with the specified endpoint start in sl_zigbee_attribute_data array.
 *
 * @param[in]  endpoint          Endpoint number.
 * @param[out] EpStorageOffset   Pointer to store the calculated storage offset.
 *
 * @return the endpoint type address if found; NULL otherwise.
 */
sl_zigbee_af_endpoint_type_t* getEndpointTypeAndStorageOffset(uint8_t endpoint, uint16_t* EpStorageOffset)
{
  if (EpStorageOffset == NULL) {
    // invalid parameters we can't continue.
    return NULL;
  }

  uint16_t offset = 0;
  // Iterate over sli_zigbee_af_endpoints until the matching enabled endpoint is found
  for (uint8_t epIndex = 0; epIndex < sl_zigbee_af_endpoint_count(); epIndex++) {
    if (sli_zigbee_af_endpoints[epIndex].endpoint != endpoint) {
      // Not our endpoint. Skip over all its attribute storage range
      offset += sli_zigbee_af_endpoints[epIndex].endpointType->endpointSize;
    } else if (sl_zigbee_af_endpoint_index_is_enabled(epIndex)) {
      *EpStorageOffset = offset;
      return sli_zigbee_af_endpoints[epIndex].endpointType;
    }
  }

  // endpoint not found or is disabled.
  return NULL;
}

/**
 * @brief Retrieves the cluster for a given endpoint matching the attribute record.
 *
 * This function locates the cluster associated with the specified endpoint and cluster ID.
 * It also calculates the storage offset of sl_zigbee_attribute_data array, for attributes within the cluster.
 *
 * @param[in]  endpoint          The endpoint number.
 * @param[in]  attRecord         Pointer to the attribute search record containing search criteria.
 * @param[out] clStorageOffset   Pointer to store the calculated storage offset.
 *
 * @return Pointer to the cluster if successfully retrieved; NULL otherwise.
 */
sl_zigbee_af_cluster_t* getClusterAndStorageOffset(sl_zigbee_af_endpoint_type_t* epType, sl_zigbee_af_attribute_search_record_t *attRecord, uint16_t* clStorageOffset)
{
  if (epType == NULL || attRecord == NULL || clStorageOffset == NULL) {
    // invalid parameters we can't continue.
    return NULL;
  }
  // Parse the endpoint's cluster list to find our matching cluster
  uint16_t offset = 0;
  for (uint8_t clusterIndex = 0;
       clusterIndex < epType->clusterCount;
       clusterIndex++) {
    sl_zigbee_af_cluster_t* cluster = &(epType->cluster[clusterIndex]);
    if (sli_zigbee_af_match_cluster(cluster, attRecord)) {
      // Found the maching cluster. Parse its attribute list for our attribute.
      *clStorageOffset = offset;
      return cluster;
    } else {
      // This isn't the cluster we looking for, Skip over its attribute storage range
      offset += cluster->clusterSize;
    }
  }
  //cluster not found
  return NULL;
}

/**
 * @brief Retrieves the attribute metadata and calculates the storage offset for a given attribute.
 *
 * This function searches for the attribute metadata within the specified cluster based on the
 * provided attribute search record. It also calculates the storage offset of sl_zigbee_attribute_data
 * array for the attribute in the cluster's attribute storage.
 *
 * @param[in]  cluster           Pointer to the cluster containing the attributes.
 * @param[in]  attRecord         Pointer to the attribute search record containing search criteria.
 * @param[out] attStorageOffset  Pointer to store the calculated storage offset for the attribute.
 *
 * @return Pointer to the located attribute metadata if found; NULL otherwise.
 */
sl_zigbee_af_attribute_metadata_t* getAttributeMetaDataAndStorageOffset(sl_zigbee_af_cluster_t* cluster, sl_zigbee_af_attribute_search_record_t *attRecord, uint16_t* attStorageOffset)
{
  if (cluster == NULL ||  attRecord == NULL || attStorageOffset == NULL) {
    // invalid parameters we can't continue.
    return NULL;
  }

  uint16_t offset = 0;
  for (uint16_t attrIndex = 0;
       attrIndex < cluster->attributeCount;
       attrIndex++) {
    sl_zigbee_af_attribute_metadata_t* am = &(cluster->attributes[attrIndex]);
    if (sli_zigbee_af_match_attribute(cluster,
                                      am,
                                      attRecord)) {
      *attStorageOffset = offset;
      return am;
    } else if (!sl_zigbee_af_attribute_is_external(am) && !sl_zigbee_af_attribute_is_singleton(am)) {
      // Not the attribute we are looking for
      // Increase the attStorageOffset for non-singleton and non-external attributes.
      offset += sl_zigbee_af_attribute_size(am);
    }
  }
  //attribute not found
  return NULL;
}

/**
 * @brief Retrieves the cluster, attribute metadata, and storage location for a given attribute search record.
 *
 * This function searches for the cluster and attribute metadata based on the provided
 * attribute search record. It also determines the storage location of the attribute,
 * considering whether it is a singleton, external, or regular attribute.
 *
 * @param[in]  attRecord                 Pointer to the attribute search record containing search criteria.
 * @param[out] cluster                   Pointer to store the located cluster.
 * @param[out] attributeMetadata         Pointer to store the located attribute metadata.
 * @param[out] attributeStorageLocation  Pointer to store the attribute's storage location.
 *
 * @return True if the cluster and attribute metadata are found; false otherwise.
 */
bool sli_retrieve_cluster_attribute_metadata_and_storage_location(
  sl_zigbee_af_attribute_search_record_t *attRecord,
  sl_zigbee_af_cluster_t **cluster,
  sl_zigbee_af_attribute_metadata_t **attributeMetadata,
  uint8_t **attributeStorageLocation)
{
  if (attRecord == NULL || cluster == NULL || attributeMetadata == NULL || attributeStorageLocation == NULL) {
    // invalid parameters we can't continue.
    return false;
  }

  // totalStorageOffset is used to track the offset in sl_zigbee_attribute_data where the non-single, non external attribute is stored.
  uint16_t totalStorageOffset = 0;
  uint16_t storageOffset = 0;
  sl_zigbee_af_endpoint_type_t* endpointType = getEndpointTypeAndStorageOffset(attRecord->endpoint, &storageOffset);
  if (endpointType == NULL) {
    // No matching endpoint found
    return false;
  }

  totalStorageOffset += storageOffset;
  sl_zigbee_af_cluster_t* cl = getClusterAndStorageOffset(endpointType, attRecord, &storageOffset);
  if (cl == NULL) {
    // No matching cluster found
    return false;
  }

  totalStorageOffset += storageOffset;
  sl_zigbee_af_attribute_metadata_t* am = getAttributeMetaDataAndStorageOffset(cl, attRecord, &storageOffset);
  if (am) {
    *cluster = cl;
    *attributeMetadata =  am;
    totalStorageOffset += storageOffset;

    // Determine the attribute storage location based on the attribute mask
    if (sl_zigbee_af_attribute_is_singleton(am)) {
      *attributeStorageLocation = singletonAttributeLocation(am);
    } else if (!sl_zigbee_af_attribute_is_external(am)) {
      *attributeStorageLocation = sl_zigbee_attribute_data + totalStorageOffset;
    } else {
      *attributeStorageLocation = NULL;       // External attributes use nvm storage.
    }
    // search complete.
    return true;
  }

  return false; // Attribute not found
}

// Read a given attribute's data from storage (external, singleton or attribute storage)
// When reading non-string attributes, this function returns an error when destination
// buffer isn't large enough to accommodate the attribute type.  For strings, the
// function will copy at most readLength bytes.  This means the resulting string
// may be truncated.  The length byte(s) in the resulting string will reflect
// any truncation.  If readLength is zero, we are working with backwards-
// compatibility wrapper functions and we just cross our fingers and hope for
// the best.
//
sl_zigbee_af_status_t sli_zigbee_af_read_attribute_from_storage(sl_zigbee_af_attribute_search_record_t *attRecord,
                                                                sl_zigbee_af_attribute_metadata_t **metadata,
                                                                uint8_t *buffer,
                                                                uint16_t readLength)
{
  sl_zigbee_af_cluster_t *cluster;
  sl_zigbee_af_attribute_metadata_t *am;
  uint8_t* storageLocation;
  if (sli_retrieve_cluster_attribute_metadata_and_storage_location(attRecord, &cluster, &am, &storageLocation)) {
    // If passed metadata location is not null, populate
    if (metadata != NULL) {
      *metadata = am;
    }

    // If received buffer is null, we don't pursue the attribute data read
    // Caller was probably just interested in the metadata.
    if (buffer == NULL) {
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }

    uint16_t mfgCode = sli_zigbee_af_get_manufacturer_code_for_attribute(cluster, am);
    if (!sl_zigbee_af_attribute_read_access_cb(attRecord->endpoint,
                                               attRecord->clusterId,
                                               mfgCode,
                                               am->attributeId)) {
      return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    }

    if (sl_zigbee_af_attribute_is_external(am)) {
      return sl_zigbee_af_external_attribute_read_cb(attRecord->endpoint,
                                                     attRecord->clusterId,
                                                     am,
                                                     mfgCode,
                                                     buffer,
                                                     sl_zigbee_af_attribute_size(am));
    } else {
      SL_ZIGBEE_TEST_ASSERT(storageLocation != NULL);
      return typeSensitiveMemCopy(buffer, storageLocation, am, false /* write */, readLength);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE; // Sorry, attribute was not found.
}

// Write a given attribute's data to it's storage (external, singleton or attribute storage)
// For non-string attributes, this function assumes the source buffer is the same size as the attribute
// type.  For strings, the function will copy as many bytes as will fit in the
// attribute.  This means the resulting string may be truncated.  The length
// byte(s) in the resulting string will reflect any truncated.
sl_zigbee_af_status_t sli_zigbee_af_write_attribute_to_storage(sl_zigbee_af_attribute_search_record_t *attRecord,
                                                               uint8_t *buffer,
                                                               bool syncMultiProtocol)
{
  (void)syncMultiProtocol; // Could be unused
  sl_zigbee_af_cluster_t *cluster;
  sl_zigbee_af_attribute_metadata_t *am;
  uint8_t* storageLocation;

  if (sli_retrieve_cluster_attribute_metadata_and_storage_location(attRecord, &cluster, &am, &storageLocation)) {
    uint16_t mfgCode = sli_zigbee_af_get_manufacturer_code_for_attribute(cluster, am);
    if (!sl_zigbee_af_attribute_write_access_cb(attRecord->endpoint,
                                                attRecord->clusterId,
                                                mfgCode,
                                                am->attributeId)) {
      return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    }

#if defined(GENERATED_MULTI_PROTOCOL_ATTRIBUTE_MAPPING) && defined(SL_CATALOG_MULTIPROTOCOL_ZIGBEE_MATTER_COMMON_PRESENT) && defined(GENERATED_MULTI_PROTOCOL_CLUSTER_MAPPING)
    if (syncMultiProtocol) {
      bool isAttributeSynced = false;
      uint8_t i = 0;
      while (!isAttributeSynced && i < mappedMpClusterCount) {
        if (multiProtocolClusterMap[i].zigbeeClusterId == attRecord->clusterId
            && multiProtocolClusterMap[i].zigbeeMfgClusterId == mfgCode) {
          const sl_zigbee_matter_af_multi_protocol_attribute_metadata_t *attributeMpMap = multiProtocolClusterMap[i].zigbeeMatterAttributeMap;

          for (uint8_t j = 0; j < multiProtocolClusterMap[i].clusterMappedAttributeCount; j++) {
            if (attributeMpMap[j].zigbeeAttributeId == attRecord->attributeId
                && attributeMpMap[j].zigbeeMfgAttributeId == mfgCode) {
              sli_matter_af_write_attribute(attRecord->endpoint,
                                            (((uint32_t)multiProtocolClusterMap[i].matterMfgClusterId << 16) | (uint32_t)multiProtocolClusterMap[i].matterClusterId),
                                            (((uint32_t)attributeMpMap[j].matterMfgAttributeId << 16) | (uint32_t)attributeMpMap[j].matterAttributeId),
                                            buffer,
                                            attributeMpMap[j].matterAttributeType);
              isAttributeSynced = true;
              break;
            }
          }
        }
        i++;
      }
    }
#endif //defined(GENERATED_MULTI_PROTOCOL_ATTRIBUTE_MAPPING) && defined(SL_CATALOG_MULTIPROTOCOL_ZIGBEE_MATTER_COMMON_PRESENT) && defined(GENERATED_MULTI_PROTOCOL_CLUSTER_MAPPING)

    if (sl_zigbee_af_attribute_is_external(am)) {
      return sl_zigbee_af_external_attribute_write_cb(attRecord->endpoint,
                                                      attRecord->clusterId,
                                                      am,
                                                      mfgCode,
                                                      buffer);
    } else {
      SL_ZIGBEE_TEST_ASSERT(storageLocation != NULL);
      return typeSensitiveMemCopy(storageLocation, buffer, am, true, 0);
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE; // Sorry, attribute was not found.
}

// Check if a cluster is implemented or not. If yes, the cluster is returned.
// If the cluster is not manufacturerSpecific [ClusterId < FC00] then
// manufacturerCode argument is ignored otherwise checked.
//
// mask = 0 -> find either client or server
// mask = CLUSTER_MASK_CLIENT -> find client
// mask = CLUSTER_MASK_SERVER -> find server
static sl_zigbee_af_cluster_t *findClusterInTypeWithMfgCode(sl_zigbee_af_endpoint_type_t *endpointType,
                                                            sl_zigbee_af_cluster_id_t clusterId,
                                                            sl_zigbee_af_cluster_mask_t mask,
                                                            uint16_t manufacturerCode,
                                                            bool ignoreDisabledClusters)
{
  uint8_t i;
#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
  uint8_t clusterIdx;
  uint8_t clusterData;
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME

  for (i = 0; i < endpointType->clusterCount; i++) {
    sl_zigbee_af_cluster_t *cluster = &(endpointType->cluster[i]);
    #if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
    #if !defined(SL_ZIGBEE_SCRIPTED_TEST)
    clusterIdx = findClusterIdxInGeneratedList(cluster);
    if (0xFF == clusterIdx) {
      // Invalid cluster
      continue;
    }

    clusterData = clusterExtendedData[clusterIdx];
    if (ignoreDisabledClusters == false || sl_zigbee_af_cluster_is_enabled(clusterData)) {
    #endif //SL_ZIGBEE_SCRIPTED_TEST
    #else //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME
    (void)ignoreDisabledClusters;
    #endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME
    if (cluster->clusterId == clusterId
        && (mask == 0
            || (mask == CLUSTER_MASK_CLIENT && sl_zigbee_af_cluster_is_client(cluster))
            || (mask == CLUSTER_MASK_SERVER && sl_zigbee_af_cluster_is_server(cluster)))
        && (!sl_zigbee_af_cluster_is_manufacturer_specific(cluster)
            || (sli_zigbee_af_get_manufacturer_code_for_cluster(cluster)
                == manufacturerCode)
            // For compatibility with older stack api, we ignore manufacturer code here
            // if the manufacturerCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
            || manufacturerCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)) {
      return cluster;
    }
#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
  }
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME
  }
  return NULL;
}

sl_zigbee_af_cluster_t *sl_zigbee_af_find_cluster_in_type_with_mfg_code(sl_zigbee_af_endpoint_type_t *endpointType,
                                                                        sl_zigbee_af_cluster_id_t clusterId,
                                                                        sl_zigbee_af_cluster_mask_t mask,
                                                                        uint16_t manufacturerCode)
{
  return findClusterInTypeWithMfgCode(endpointType,
                                      clusterId,
                                      mask,
                                      manufacturerCode,
                                      true // Skip disabled cluster
                                      );
}

// This functions wraps sl_zigbee_af_find_cluster_in_type_with_mfg_code with
// a manufacturerCode of SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE.
sl_zigbee_af_cluster_t *sl_zigbee_af_find_cluster_in_type(sl_zigbee_af_endpoint_type_t *endpointType,
                                                          sl_zigbee_af_cluster_id_t clusterId,
                                                          sl_zigbee_af_cluster_mask_t mask)
{
  return sl_zigbee_af_find_cluster_in_type_with_mfg_code(endpointType,
                                                         clusterId,
                                                         mask,
                                                         SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// This code is used during unit tests for clusters that do not involve manufacturer code.
// Should this code be used in other locations, manufacturerCode should be added.
uint8_t sl_zigbee_af_cluster_index(uint8_t endpoint,
                                   sl_zigbee_af_cluster_id_t clusterId,
                                   sl_zigbee_af_cluster_mask_t mask)
{
  uint8_t ep;
  uint8_t index = 0xFF;
  for ( ep = 0; ep < sl_zigbee_af_endpoint_count(); ep++ ) {
    sl_zigbee_af_endpoint_type_t *endpointType = sli_zigbee_af_endpoints[ep].endpointType;
    if ( sl_zigbee_af_find_cluster_in_type_with_mfg_code(endpointType, clusterId, mask, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) != NULL ) {
      index++;
      if ( sli_zigbee_af_endpoints[ep].endpoint == endpoint ) {
        return index;
      }
    }
  }
  return 0xFF;
}

// Returns true uf endpoint contains passed cluster
bool sl_zigbee_af_contains_cluster_with_mfg_code(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, uint16_t manufacturerCode)
{
  return (sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, 0, manufacturerCode) != NULL);
}

// Returns true if endpoint contains passed cluster as a server
bool sl_zigbee_af_contains_server_with_mfg_code(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, uint16_t manufacturerCode)
{
  return (sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, CLUSTER_MASK_SERVER, manufacturerCode) != NULL);
}

// Returns true if endpoint contains passed cluster as a client
bool sl_zigbee_af_contains_client_with_mfg_code(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, uint16_t manufacturerCode)
{
  return (sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, CLUSTER_MASK_CLIENT, manufacturerCode) != NULL);
}

// Wraps sl_zigbee_af_contains_cluster_with_mfg_code with SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
// This will find the first cluster that has the clusterId given, regardless of mfgCode.
bool sl_zigbee_af_contains_cluster(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  return (sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, 0, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) != NULL);
}

// Wraps sl_zigbee_af_contains_server_with_mfg_code with SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
// This will find the first server that has the clusterId given, regardless of mfgCode.
bool sl_zigbee_af_contains_server(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  return (sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, CLUSTER_MASK_SERVER, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) != NULL);
}

// Wraps sl_zigbee_af_contains_client_with_mfg_code with SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
// This will find the first client that has the clusterId given, regardless of mfgCode.
bool sl_zigbee_af_contains_client(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  return (sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, CLUSTER_MASK_CLIENT, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) != NULL);
}

// Finds the cluster that matches endpoint, clusterId, direction, and manufacturerCode.
sl_zigbee_af_cluster_t *sl_zigbee_af_find_cluster_with_mfg_code(uint8_t endpoint,
                                                                sl_zigbee_af_cluster_id_t clusterId,
                                                                sl_zigbee_af_cluster_mask_t mask,
                                                                uint16_t manufacturerCode)
{
  uint8_t ep = sl_zigbee_af_index_from_endpoint(endpoint);
  if (ep == 0xFF
#if ZCL_FIXED_ENDPOINT_COUNT > 0
      || (ep >= ZCL_FIXED_ENDPOINT_COUNT)
#endif
      ) {
    return NULL;
  } else {
    return sl_zigbee_af_find_cluster_in_type_with_mfg_code(sli_zigbee_af_endpoints[ep].endpointType, clusterId, mask, manufacturerCode);
  }
}

// This function wraps sl_zigbee_af_find_cluster_with_mfg_code with SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
// and will ignore the manufacturerCode when trying to find clusters.
// This will return the first cluster in the cluster table that matches the parameters given.
sl_zigbee_af_cluster_t *sl_zigbee_af_find_cluster(uint8_t endpoint,
                                                  sl_zigbee_af_cluster_id_t clusterId,
                                                  sl_zigbee_af_cluster_mask_t mask)
{
  return sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, mask, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// Returns cluster within the endpoint; Does not ignore disabled endpoints
sl_zigbee_af_cluster_t *sl_zigbee_af_find_cluster_including_disabled_endpoints_with_mfg_code(uint8_t endpoint,
                                                                                             sl_zigbee_af_cluster_id_t clusterId,
                                                                                             sl_zigbee_af_cluster_mask_t mask,
                                                                                             uint16_t manufacturerCode)
{
#if (MAX_ENDPOINT_COUNT > 0)
  uint8_t ep = sl_zigbee_af_index_from_endpoint_including_disabled_endpoints(endpoint);
#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
  if (ep == 0xFF) {
    return NULL;
  }
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME
  if (ep < MAX_ENDPOINT_COUNT) {
    return sl_zigbee_af_find_cluster_in_type_with_mfg_code(sli_zigbee_af_endpoints[ep].endpointType, clusterId, mask, manufacturerCode);
  }
#else // MAX_ENDPOINT_COUNT == 0
  UNUSED_VAR(endpoint);
  UNUSED_VAR(clusterId);
  UNUSED_VAR(mask);
  UNUSED_VAR(manufacturerCode);
#endif // (MAX_ENDPOINT_COUNT > 0)
  return NULL;
}

// Returns cluster within the endpoint; Does not ignore disabled endpoints
// This will ignore manufacturerCode.
sl_zigbee_af_cluster_t *sl_zigbee_af_find_cluster_including_disabled_endpoints(uint8_t endpoint,
                                                                               sl_zigbee_af_cluster_id_t clusterId,
                                                                               sl_zigbee_af_cluster_mask_t mask)
{
  return sl_zigbee_af_find_cluster_including_disabled_endpoints_with_mfg_code(endpoint, clusterId, mask, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// Server wrapper for findClusterEndpointIndex.
static uint8_t sl_zigbee_af_find_cluster_server_endpoint_index_with_mfg_code(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, uint16_t manufacturerCode)
{
  return findClusterEndpointIndex(endpoint, clusterId, CLUSTER_MASK_SERVER, manufacturerCode);
}

// Client wrapper for findClusterEndpointIndex.
static uint8_t sl_zigbee_af_find_cluster_client_endpoint_index_with_mfg_code(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, uint16_t manufacturerCode)
{
  return findClusterEndpointIndex(endpoint, clusterId, CLUSTER_MASK_CLIENT, manufacturerCode);
}

// Server wrapper for findClusterEndpointIndex
// This will ignore manufacturerCode, and return the index for the first server that matches on clusterId
uint8_t sl_zigbee_af_find_cluster_server_endpoint_index(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  return sl_zigbee_af_find_cluster_server_endpoint_index_with_mfg_code(endpoint, clusterId, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// Client wrapper for findClusterEndpointIndex
// This will ignore manufacturerCode, and return the index for the first client that matches on clusterId
uint8_t sl_zigbee_af_find_cluster_client_endpoint_index(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId)
{
  return sl_zigbee_af_find_cluster_client_endpoint_index_with_mfg_code(endpoint, clusterId, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
}

// Returns the endpoint index within a given cluster
static uint8_t findClusterEndpointIndex(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId, uint8_t mask, uint16_t manufacturerCode)
{
  uint8_t i, epi = 0;

  if (sl_zigbee_af_find_cluster_with_mfg_code(endpoint, clusterId, mask, manufacturerCode) == NULL) {
    return 0xFF;
  }

  for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
    if (sli_zigbee_af_endpoints[i].endpoint == endpoint) {
      break;
    }
    epi += (sl_zigbee_af_find_cluster_including_disabled_endpoints_with_mfg_code(sli_zigbee_af_endpoints[i].endpoint, clusterId, mask, manufacturerCode) != NULL) ? 1 : 0;
  }

  return epi;
}

#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
// Calculate cluster index in generatedClusters[].
// Returns 0xff if not found
static uint8_t findClusterIdxInGeneratedList(sl_zigbee_af_cluster_t *cluster)
{
  if (cluster != NULL) {
    uint8_t clusterIdx;
    if (((uint32_t)cluster >= (uint32_t)generatedClusters)
        && ((uint32_t)cluster < (uint32_t)(generatedClusters) + sizeof(generatedClusters))) {
      clusterIdx = ((uint32_t)cluster - (uint32_t)generatedClusters) / sizeof(generatedClusters[0]);
      return clusterIdx;
    }
  }
  return 0xFF;
}

// Returns the cluster index in generatedClusters[] within a given {endpoint, cluster, mask, manuCode}
// Returns 0xff if not found
static uint8_t findClusterIdxInGeneratedListByEndpoint(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId,
                                                       uint8_t mask, uint16_t manufacturerCode)
{
  uint8_t i;
  uint8_t epi = sl_zigbee_af_index_from_endpoint(endpoint);
  bool clusterFound;
  if ( epi != 0xFF ) {
    sl_zigbee_af_endpoint_type_t *endpointType = sli_zigbee_af_endpoints[epi].endpointType;
    uint8_t clusterCount = endpointType->clusterCount;
    sl_zigbee_af_cluster_t *cluster = NULL;
    clusterFound = false;

    for (i = 0; i < clusterCount; i++) {
      cluster = &(endpointType->cluster[i]);
      // add mask (CLUSTER_MASK_DISABLED) for enabled cluster only
      if (cluster->clusterId == clusterId
          && (mask == 0
              || (mask == CLUSTER_MASK_CLIENT && sl_zigbee_af_cluster_is_client(cluster))
              || (mask == CLUSTER_MASK_SERVER && sl_zigbee_af_cluster_is_server(cluster)))
          && (!sl_zigbee_af_cluster_is_manufacturer_specific(cluster)
              || (sli_zigbee_af_get_manufacturer_code_for_cluster(cluster)
                  == manufacturerCode)
              // For compatibility with older stack api, we ignore manufacturer code here
              // if the manufacturerCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
              || manufacturerCode == SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE)) {
        clusterFound = true;
        break;
      }
    }

    //calculate idx in generatedClusters[].
    if (clusterFound) {
      return findClusterIdxInGeneratedList(cluster);
    }
  }

  return 0xFF;
}
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME

static uint8_t findIndexFromEndpoint(uint8_t endpoint, bool ignoreDisabledEndpoints)
{
  uint8_t epi;
  for (epi = 0; epi < sl_zigbee_af_endpoint_count(); epi++) {
    if (sli_zigbee_af_endpoints[epi].endpoint == endpoint
        && (!ignoreDisabledEndpoints
            || sli_zigbee_af_endpoints[epi].bitmask & SL_ZIGBEE_AF_ENDPOINT_ENABLED)) {
      return epi;
    }
  }
  return 0xFF;
}

bool sl_zigbee_af_endpoint_is_enabled(uint8_t endpoint)
{
  uint8_t index = findIndexFromEndpoint(endpoint,
                                        false);  // ignore disabled endpoints?

  SL_ZIGBEE_TEST_ASSERT(0xFF != index);

  if (0xFF == index) {
    return false;
  }

  return sl_zigbee_af_endpoint_index_is_enabled(index);
}

bool sl_zigbee_af_endpoint_enable_disable(uint8_t endpoint, bool enable)
{
  uint8_t index = findIndexFromEndpoint(endpoint,
                                        false);  // ignore disabled endpoints?
  bool currentlyEnabled;

  if (0xFF == index) {
    return false;
  }

  currentlyEnabled = sli_zigbee_af_endpoints[index].bitmask & SL_ZIGBEE_AF_ENDPOINT_ENABLED;

  if (enable) {
    sli_zigbee_af_endpoints[index].bitmask |= SL_ZIGBEE_AF_ENDPOINT_ENABLED;
  } else {
    sli_zigbee_af_endpoints[index].bitmask &= SL_ZIGBEE_AF_ENDPOINT_DISABLED;
  }

#if defined(EZSP_HOST)
  sl_zigbee_ezsp_set_endpoint_flags(endpoint,
                                    (enable
                                     ? SL_ZIGBEE_EZSP_ENDPOINT_ENABLED
                                     : SL_ZIGBEE_EZSP_ENDPOINT_DISABLED));
#endif

  if (currentlyEnabled != enable) {
    if (enable) {
      initializeEndpoint(&(sli_zigbee_af_endpoints[index]));
    } else {
      uint8_t i;
      for (i = 0; i < sli_zigbee_af_endpoints[index].endpointType->clusterCount; i++) {
        sl_zigbee_af_cluster_t* cluster = &((sli_zigbee_af_endpoints[index].endpointType->cluster)[i]);

        sl_zigbee_zcl_deactivate_cluster_tick(endpoint,
                                              cluster->clusterId,
                                              (cluster->mask & CLUSTER_MASK_CLIENT
                                               ? SL_ZIGBEE_AF_CLIENT_CLUSTER_TICK
                                               : SL_ZIGBEE_AF_SERVER_CLUSTER_TICK));
      }
    }
  }

  return true;
}

#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
bool sl_zigbee_af_cluster_enable_disable(uint8_t endpoint,
                                         sl_zigbee_af_cluster_id_t clusterId,
                                         sl_zigbee_af_cluster_mask_t mask,
                                         bool enable)
{
  uint8_t clusterData;
  sl_zigbee_af_cluster_t *cluster = NULL;
  bool currentlyEnabled;
  uint8_t clusterIdx;

  clusterIdx = findClusterIdxInGeneratedListByEndpoint(endpoint, clusterId,
                                                       mask, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);

  if (0xFF == clusterIdx) {
    return false;
  }

  cluster = (sl_zigbee_af_cluster_t*)&generatedClusters[clusterIdx];

  clusterData = clusterExtendedData[clusterIdx];
  currentlyEnabled = sl_zigbee_af_cluster_is_enabled(clusterData);

  if (currentlyEnabled != enable) {
    if (enable) {
      clusterData &= ~CLUSTER_MASK_DISABLED;
      sl_zigbee_af_cluster_init_cb(endpoint, cluster->clusterId);
      sl_zigbee_af_generic_cluster_function_t f = sl_zigbee_af_find_cluster_function(cluster, CLUSTER_MASK_INIT_FUNCTION);
      if (f != NULL) {
        ((sl_zigbee_af_init_function_t)f)(endpoint);
      }
    } else {
      clusterData |= CLUSTER_MASK_DISABLED;
      sl_zigbee_zcl_deactivate_cluster_tick(endpoint,
                                            cluster->clusterId,
                                            (cluster->mask & CLUSTER_MASK_CLIENT
                                             ? SL_ZIGBEE_AF_CLIENT_CLUSTER_TICK
                                             : SL_ZIGBEE_AF_SERVER_CLUSTER_TICK));
    }
    clusterExtendedData[clusterIdx] = clusterData;
  }

  return true;
}

bool sl_zigbee_af_is_cluster_enabled(uint8_t endpoint,
                                     sl_zigbee_af_cluster_id_t clusterId,
                                     sl_zigbee_af_cluster_mask_t mask)
{
  uint8_t clusterData;
  uint8_t clusterIdx = findClusterIdxInGeneratedListByEndpoint(endpoint, clusterId,
                                                               mask, SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (0xFF == clusterIdx) {
    return false;
  }
  clusterData = clusterExtendedData[clusterIdx];

  return sl_zigbee_af_cluster_is_enabled(clusterData);
}
#endif // SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME

// Returns the index of a given endpoint.  Does not consider disabled endpoints.
uint8_t sl_zigbee_af_index_from_endpoint(uint8_t endpoint)
{
  return findIndexFromEndpoint(endpoint,
                               true);    // ignore disabled endpoints?
}

// Returns the index of a given endpoint.  Considers disabled endpoints.
uint8_t sl_zigbee_af_index_from_endpoint_including_disabled_endpoints(uint8_t endpoint)
{
  return findIndexFromEndpoint(endpoint,
                               false);    // ignore disabled endpoints?
}

uint8_t sl_zigbee_af_endpoint_from_index(uint8_t index)
{
  return sli_zigbee_af_endpoints[index].endpoint;
}

// If server == true, returns the number of server clusters,
// otherwise number of client clusters on this endpoint
uint8_t sl_zigbee_af_cluster_count(uint8_t endpoint, bool server)
{
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  uint8_t i, c = 0;
  sl_zigbee_af_defined_endpoint_t *de;
  sl_zigbee_af_cluster_t *cluster;
#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
  uint8_t clusterIdx;
  uint8_t clusterData;
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME
  if ( index == 0xFF
#if MAX_ENDPOINT_COUNT > 0
       || (index >= MAX_ENDPOINT_COUNT)
#endif
       ) {
    return 0;
  }
  de = &(sli_zigbee_af_endpoints[index]);
  if ( de->endpointType == NULL) {
    return 0;
  }
  for ( i = 0; i < de->endpointType->clusterCount; i++ ) {
    cluster = &(de->endpointType->cluster[i]);
#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
    /* Don't count
     * if cluster is disabled
     */
    clusterIdx = findClusterIdxInGeneratedList(cluster);
    if (0xFF == clusterIdx) {
      continue;
    }
    clusterData = clusterExtendedData[clusterIdx];
    if (false == sl_zigbee_af_cluster_is_enabled(clusterData)) {
      continue;
    }
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME
    if ( server && sl_zigbee_af_cluster_is_server(cluster) ) {
      c++;
    }
    if ( (!server) && sl_zigbee_af_cluster_is_client(cluster) ) {
      c++;
    }
  }
  return c;
}

uint8_t sl_zigbee_af_get_cluster_count_for_endpoint(uint8_t endpoint)
{
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  if ( index == 0xFF
#if MAX_ENDPOINT_COUNT > 0
       || (index >= MAX_ENDPOINT_COUNT)
#endif
       ) {
    return 0;
  }
  return sli_zigbee_af_endpoints[index].endpointType->clusterCount;
}

// Note the difference in implementation from sl_zigbee_af_get_nth_cluster().
// sl_zigbee_af_get_cluster_by_index() retrieves the cluster by index regardless of server/client
// and those indexes may be DIFFERENT than the indexes returned from
// sl_zigbee_af_get_nth_cluster().  In other words:
//
//  - Use sl_zigbee_af_get_clusters_from_endpoint()  with sl_zigbee_af_get_nth_cluster()
//  - Use sl_zigbee_af_get_cluster_count_for_endpoint() with sl_zigbee_af_get_cluster_by_index()
//
// Don't mix them.
sl_zigbee_af_cluster_t* sl_zigbee_af_get_cluster_by_index(uint8_t endpoint, uint8_t clusterIndex)
{
  uint8_t endpointIndex = sl_zigbee_af_index_from_endpoint(endpoint);
  sl_zigbee_af_defined_endpoint_t* definedEndpoint;

  if (endpointIndex == 0xFF
#if MAX_ENDPOINT_COUNT > 0
      || (endpointIndex >= MAX_ENDPOINT_COUNT)
#endif
      ) {
    return NULL;
  }
  definedEndpoint = &(sli_zigbee_af_endpoints[endpointIndex]);

  if (clusterIndex >= definedEndpoint->endpointType->clusterCount) {
    return NULL;
  }
  return &(definedEndpoint->endpointType->cluster[clusterIndex]);
}

sl_zigbee_af_profile_id_t sl_zigbee_af_get_profile_id_for_endpoint(uint8_t endpoint)
{
  uint8_t endpointIndex = sl_zigbee_af_index_from_endpoint(endpoint);
  if (endpointIndex == 0xFF
#if MAX_ENDPOINT_COUNT > 0
      || (endpointIndex >= MAX_ENDPOINT_COUNT)
#endif
      ) {
    return SL_ZIGBEE_AF_INVALID_PROFILE_ID;
  }
  return sli_zigbee_af_endpoints[endpointIndex].profileId;
}

uint16_t sl_zigbee_af_get_device_id_for_endpoint(uint8_t endpoint)
{
  uint8_t endpointIndex = sl_zigbee_af_index_from_endpoint(endpoint);
  if (endpointIndex == 0xFF
#if MAX_ENDPOINT_COUNT > 0
      || (endpointIndex >= MAX_ENDPOINT_COUNT)
#endif
      ) {
    return SL_ZIGBEE_AF_INVALID_PROFILE_ID;
  }
  return sli_zigbee_af_endpoints[endpointIndex].deviceId;
}

// Returns the cluster of Nth server or client cluster,
// depending on server toggle.
sl_zigbee_af_cluster_t *sl_zigbee_af_get_nth_cluster(uint8_t endpoint, uint8_t n, bool server)
{
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  sl_zigbee_af_defined_endpoint_t *de;
  uint8_t i, c = 0;
  sl_zigbee_af_cluster_t *cluster;
#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
  uint8_t clusterIdx;
  uint8_t clusterData;
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME

  if ( index == 0xFF
#if MAX_ENDPOINT_COUNT > 0
       || (index >= MAX_ENDPOINT_COUNT)
#endif
       ) {
    return NULL;
  }
  de = &(sli_zigbee_af_endpoints[index]);

  for ( i = 0; i < de->endpointType->clusterCount; i++ ) {
    cluster = &(de->endpointType->cluster[i]);
#if (SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME == 1)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
    /* Don't count
     * if cluster is disabled
     */
    clusterIdx = findClusterIdxInGeneratedList(cluster);
    if (0xFF == clusterIdx) {
      continue;
    }
    clusterData = clusterExtendedData[clusterIdx];
    if (false == sl_zigbee_af_cluster_is_enabled(clusterData)) {
      continue;
    }
#endif //SL_ZIGBEE_SCRIPTED_TEST
#endif //SL_ZIGBEE_AF_PLUGIN_ZCL_CLUSTER_ENABLE_DISABLE_RUN_TIME

    if ( (server && sl_zigbee_af_cluster_is_server(cluster) )
         || ( (!server) &&  sl_zigbee_af_cluster_is_client(cluster) ) ) {
      if ( c == n ) {
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
        // Apply cluster suppression for match descriptor response.
        if ((server && sl_zigbee_af_get_suppress_cluster(cluster->clusterId, false))
            || (!server && sl_zigbee_af_get_suppress_cluster(cluster->clusterId, true))) {
          sl_zigbee_af_debug_println("sl_zigbee_af_get_nth_cluster - skipping: server = %d, cluster = %04X", server, cluster->clusterId);
          return NULL;
        }
#endif
        return cluster;
      }
      c++;
    }
  }
  return NULL;
}

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t sl_zigbee_af_get_clusters_from_endpoint(uint8_t endpoint, sl_zigbee_af_cluster_id_t *clusterList, uint8_t listLen, bool server)
{
  uint8_t clusterCount = sl_zigbee_af_cluster_count(endpoint, server);
  uint8_t i;
  sl_zigbee_af_cluster_t *cluster;
  if (clusterCount > listLen) {
    clusterCount = listLen;
  }
  for (i = 0; i < clusterCount; i++) {
    cluster = sl_zigbee_af_get_nth_cluster(endpoint, i, server);
    clusterList[i] = (cluster == NULL ? 0xFFFF : cluster->clusterId);
  }
  return clusterCount;
}

void sl_zigbee_af_initialize_attributes(uint8_t endpoint)
{
  sli_zigbee_af_load_attribute_defaults(endpoint, false);
}

void sl_zigbee_af_reset_attributes(uint8_t endpoint)
{
  sli_zigbee_af_load_attribute_defaults(endpoint, true);
  sli_zigbee_af_reset_attributes(endpoint);
}

void sli_zigbee_af_load_attribute_defaults(uint8_t endpoint, bool writeTokens)
{
  uint8_t ep, clusterI, curNetwork = sl_zigbee_get_current_network();
  uint16_t attr;
  uint8_t *ptr;
  uint8_t epCount = sl_zigbee_af_endpoint_count();

  for ( ep = 0; ep < epCount; ep++ ) {
    sl_zigbee_af_defined_endpoint_t *de;
    if (endpoint != SL_ZIGBEE_BROADCAST_ENDPOINT) {
      ep = sl_zigbee_af_index_from_endpoint(endpoint);
      if (ep == 0xFF
#if ZCL_FIXED_ENDPOINT_COUNT > 0
          || (ep >= ZCL_FIXED_ENDPOINT_COUNT)
#endif
          ) {
        return;
      }
    }
    de = &(sli_zigbee_af_endpoints[ep]);

    // Ensure that the endpoint is on the current network
    if (endpoint == SL_ZIGBEE_BROADCAST_ENDPOINT
        && de->networkIndex != curNetwork) {
      continue;
    }
    for ( clusterI = 0; clusterI < de->endpointType->clusterCount; clusterI++) {
      sl_zigbee_af_cluster_t *cluster = &(de->endpointType->cluster[clusterI]);

      // when the attributeCount is high, the loop takes too long to run and a
      // watchdog kicks in causing a reset. As a workaround, we'll
      // conditionally manually reset the watchdog. 300 sounds like a good
      // magic number for now.
      if (cluster->attributeCount > 300) {
        halResetWatchdog();
      }
      for ( attr = 0; attr < cluster->attributeCount; attr++) {
        sl_zigbee_af_attribute_metadata_t *am = &(cluster->attributes[attr]);
        if (!(am->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE)) {
          sl_zigbee_af_attribute_search_record_t record;
          record.endpoint = de->endpoint;
          record.clusterId = cluster->clusterId;
          record.clusterMask = (sl_zigbee_af_attribute_is_client(am)
                                ? CLUSTER_MASK_CLIENT
                                : CLUSTER_MASK_SERVER);
          record.attributeId = am->attributeId;
          record.manufacturerCode = sli_zigbee_af_get_manufacturer_code_for_attribute(cluster,
                                                                                      am);
          if ((am->mask & ATTRIBUTE_MASK_MIN_MAX) != 0U) {
            if ( sl_zigbee_af_attribute_size(am) <= 2 ) {
              ptr = (uint8_t*)&(am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue);
            } else {
              ptr = (uint8_t*)am->defaultValue.ptrToMinMaxValue->defaultValue.ptrToDefaultValue;
            }
          } else {
            if ( sl_zigbee_af_attribute_size(am) <= 2 ) {
              ptr = (uint8_t*)&(am->defaultValue.defaultValue);
            } else {
              ptr = (uint8_t*)am->defaultValue.ptrToDefaultValue;
            }
          }
          // At this point, ptr either points to a default value, or is NULL, in which case
          // it should be treated as if it is pointing to an array of all zeroes.

#if (BIGENDIAN_CPU)
          // The default value for one- and two-byte attributes is stored in an
          // uint16_t.  On big-endian platforms, a pointer to the default value of
          // a one-byte attribute will point to the wrong byte.  So, for those
          // cases, nudge the pointer forward so it points to the correct byte.
          if (sl_zigbee_af_attribute_size(am) == 1 && ptr != NULL) {
            ptr++;
          }
#endif //BIGENDIAN_CPU
          sli_zigbee_af_write_attribute_to_storage(&record,
                                                   ptr,
                                                   false); // don't sync matter on load default
          if (writeTokens) {
            sli_zigbee_af_save_attribute_to_token(ptr, de->endpoint, record.clusterId, am);
          }
        }
      }
    }
    if (endpoint != SL_ZIGBEE_BROADCAST_ENDPOINT) {
      break;
    }
  }

  if (!writeTokens) {
    sli_zigbee_af_load_attributes_from_tokens(endpoint);
  }
}

void sli_zigbee_af_load_attributes_from_tokens(uint8_t endpoint)
{
  (void)endpoint;
  // On EZSP host we currently do not support this. We need to come up with some
  // callbacks.
#ifndef EZSP_HOST
#ifdef GENERATED_TOKEN_LOADER
  GENERATED_TOKEN_LOADER(endpoint);
#endif // GENERATED_TOKEN_LOADER
#endif // EZSP_HOST
}

// 'data' argument may be null, since we changed the ptrToDefaultValue
// to be null instead of pointing to all zeroes.
// This function has to be able to deal with that.
void sli_zigbee_af_save_attribute_to_token(uint8_t *data,
                                           uint8_t endpoint,
                                           sl_zigbee_af_cluster_id_t clusterId,
                                           sl_zigbee_af_attribute_metadata_t *metadata)
{
  (void)data;
  (void)endpoint;
  (void)clusterId;
  // Get out of here if this attribute doesn't have a token.
  if ( !sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return;
  }

// On EZSP host we currently do not support this. We need to come up with some
// callbacks.
#ifndef EZSP_HOST
#ifdef GENERATED_TOKEN_SAVER
  GENERATED_TOKEN_SAVER;
#endif // GENERATED_TOKEN_SAVER
#endif // EZSP_HOST
}

// This function returns the function pointer from the cluster functions array,
// The functionMask must be a single flag out of the following ("OR"-ing is not allowed):
// CLUSTER_MASK_INIT_FUNCTION, CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION, CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION,
// CLUSTER_MASK_MESSAGE_SENT_FUNCTION, CLUSTER_MASK_MANUFACTURER_SPECIFIC_ATTRIBUTE_CHANGED_FUNCTION,
// CLUSTER_MASK_PRE_ATTRIBUTE_CHANGED_FUNCTION
sl_zigbee_af_generic_cluster_function_t
sl_zigbee_af_find_cluster_function(sl_zigbee_af_cluster_t *cluster,
                                   sl_zigbee_af_cluster_mask_t functionMask)
{
  #ifndef SL_ZIGBEE_SCRIPTED_TEST
  const sli_cluster_function_structure_t *funcStruct = &generatedClusterFunctions[0];

  for (int i = 0; i < (int)(sizeof(generatedClusterFunctions) / sizeof(sli_cluster_function_structure_t)); i++) {
    if ((funcStruct->clusterId == cluster->clusterId)
        && (funcStruct->functionMask & cluster->mask & (CLUSTER_MASK_SERVER | CLUSTER_MASK_CLIENT))
        && (functionMask & funcStruct->functionMask)) {
      return funcStruct->functionPtr;
    }
    funcStruct++;
  }

  return NULL;
  #else
  //For unit tests where generatedClusterFunctions is not populated
  sl_zigbee_af_cluster_mask_t mask = 0x01;
  uint8_t functionIndex = 0;
  if ((cluster->mask & functionMask) == 0) {
    return NULL;
  }

  while (mask < functionMask) {
    if ((cluster->mask & mask) != 0) {
      functionIndex++;
    }
    mask <<= 1;
  }
  return cluster->functions[functionIndex];
  #endif
}

#ifdef SL_ZIGBEE_AF_SUPPORT_COMMAND_DISCOVERY

#if (SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT > 0)
static UNUSED uint16_t get_manufacturer_code_for_command(sl_zigbee_af_command_metadata_t *command)
{
#if (ZCL_GENERATED_COMMAND_MANUFACTURER_CODE_COUNT > 0)
  return getManufacturerCode((sl_zigbee_af_manufacturer_code_entry_t *)commandManufacturerCodes,
                             commandManufacturerCodeCount,
                             (command - generatedCommands));
#else // !ZCL_GENERATED_COMMAND_MANUFACTURER_CODE_COUNT
  (void)command;
  return SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE;
#endif // ZCL_GENERATED_COMMAND_MANUFACTURER_CODE_COUNT
}
#endif

// Resolve the manufacturing code for a command when the Discover Command
// request specifies the wildcard 0xFFFF for the mfg code. Choose the mfg code
// for the first mfg-specific command at or following the startId. Iterate
// through all commands to choose the smallest mfg code if the command is
// overloaded by more than one mfg-specific implementation of that command ID.
// If no valid mfg code is resolved, return 0xFFFF.
uint16_t sli_zigbee_af_resolve_mfg_code_for_discover_command(bool outgoing,
                                                             sl_zigbee_af_cluster_command_t *cmd,
                                                             uint16_t clusterId,
                                                             uint8_t startId)
{
  uint16_t commandMfgCode = 0xFFFF;
#if (SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT > 0)
  bool foundFirst = false;
  uint8_t foundCmdId = 0;
  uint8_t cmdDirMask = 0;

  // determine the appropriate mask to match the request
  // discover commands generated, client is asking server what commands do you generate?
  if (outgoing && (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER)) {
    cmdDirMask = COMMAND_MASK_OUTGOING_SERVER;
    // discover commands generated server is asking client what commands do you generate?
  } else if (outgoing && (cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT)) {
    cmdDirMask = COMMAND_MASK_OUTGOING_CLIENT;
    // discover commands received client is asking server what commands do you receive?
  } else if (!outgoing && (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER)) {
    cmdDirMask = COMMAND_MASK_INCOMING_SERVER;
    // discover commands received server is asking client what commands do you receive?
  } else {
    cmdDirMask = COMMAND_MASK_INCOMING_CLIENT;
  }
  for ( uint16_t i = 0; i < SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT; i++ ) {
    if ( generatedCommands[i].clusterId != clusterId ) {
      continue;
    }

    if ((generatedCommands[i].mask & cmdDirMask) == 0 ) {
      continue;
    }

    // Only start from the passed command id
    if (generatedCommands[i].commandId < startId) {
      continue;
    }

    // After having previously found a first mfg-spec candidate,
    // ignore an attr that has a higher attrId.
    if (foundFirst && generatedCommands[i].commandId > foundCmdId) {
      continue;
    }

    // Get command's mfg-code. Update the search state if:
    // this is the first qualifying cmd found;
    // else, this qualifying cmd has a lower cmdId than prior cmd found;
    // else, this cmdId equals prior, prefer this cmd's lower mfg-code.
    if ( generatedCommands[i].mask & COMMAND_MASK_MANUFACTURER_SPECIFIC ) {
      uint16_t candidateMfgCode
        = get_manufacturer_code_for_command( (sl_zigbee_af_command_metadata_t*) &(generatedCommands[i]));
      if (!foundFirst
          || generatedCommands[i].commandId < foundCmdId
          || candidateMfgCode < commandMfgCode) {
        foundFirst = true;
        foundCmdId = generatedCommands[i].commandId;
        commandMfgCode = candidateMfgCode;
      }
    }
  }
#else
  UNUSED_VAR(outgoing);
  UNUSED_VAR(cmd);
  UNUSED_VAR(clusterId);
  UNUSED_VAR(startId);
#endif // SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT
  return commandMfgCode;
}

/**
 * This function populates command IDs into a given buffer.
 *
 * It returns true if commands are complete, meaning there are NO MORE
 * commands that would be returned after the last command.
 * It returns false, if there were more commands, but were not populated
 * because of maxIdCount limitation.
 */
bool sl_zigbee_af_extract_command_ids(bool outgoing,
                                      sl_zigbee_af_cluster_command_t *cmd,
                                      uint16_t clusterId,
                                      uint8_t *buffer,
                                      uint16_t bufferLength,
                                      uint16_t *bufferIndex,
                                      uint8_t startId,
                                      uint8_t maxIdCount)
{
  bool returnValue = true;
#if (SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT > 0)
  uint16_t i, count = 0;
  uint8_t cmdDirMask = 0;
  // determine the appropriate mask to match the request
  // discover commands generated, client is asking server what commands do you generate?
  if (outgoing && (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER)) {
    cmdDirMask = COMMAND_MASK_OUTGOING_SERVER;
    // discover commands generated server is asking client what commands do you generate?
  } else if (outgoing && (cmd->direction == ZCL_DIRECTION_SERVER_TO_CLIENT)) {
    cmdDirMask = COMMAND_MASK_OUTGOING_CLIENT;
    // discover commands received client is asking server what commands do you receive?
  } else if (!outgoing && (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER)) {
    cmdDirMask = COMMAND_MASK_INCOMING_SERVER;
    // discover commands received server is asking client what commands do you receive?
  } else {
    cmdDirMask = COMMAND_MASK_INCOMING_CLIENT;
  }
  for ( i = 0; i < SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT; i++ ) {
    if ( generatedCommands[i].clusterId != clusterId ) {
      continue;
    }

    if ((generatedCommands[i].mask & cmdDirMask) == 0 ) {
      continue;
    }

    // Only start from the passed command id
    if (generatedCommands[i].commandId < startId) {
      continue;
    }

    // According to spec: if cmd->mfgSpecific is set, then we ONLY return the
    // mfg specific commands. If it's not, then we ONLY return non-mfg specific.
    if ( generatedCommands[i].mask & COMMAND_MASK_MANUFACTURER_SPECIFIC ) {
      // Command is Mfg specific
      if ( !cmd->mfgSpecific ) {
        continue;                        // ignore if asking for not mfg specific
      }
      if ( cmd->mfgCode != get_manufacturer_code_for_command( (sl_zigbee_af_command_metadata_t*) &(generatedCommands[i]))) {
        continue;                        // Ignore if mfg code doesn't match the commands
      }
    } else {
      // Command is not mfg specific.
      if ( cmd->mfgSpecific ) {
        continue;                       // Ignore if asking for mfg specific
      }
    }

    // The one we are about to put in, is beyond the maxIdCount,
    // so instead of populating it in, we set the return flag to
    // false and get out of here.
    if ( maxIdCount == count || count >= bufferLength ) {
      returnValue = false;
      break;
    }
    buffer[count] = generatedCommands[i].commandId;
    (*bufferIndex)++;
    count++;
  }
#else
  UNUSED_VAR(outgoing);
  UNUSED_VAR(cmd);
  UNUSED_VAR(clusterId);
  UNUSED_VAR(buffer);
  UNUSED_VAR(bufferLength);
  UNUSED_VAR(bufferIndex);
  UNUSED_VAR(startId);
  UNUSED_VAR(maxIdCount);
#endif // SL_ZIGBEE_ZCL_GENERATED_COMMAND_COUNT
  return returnValue;
}
#else
// We just need an empty stub if we don't support it
bool sl_zigbee_af_extract_command_ids(bool outgoing,
                                      sl_zigbee_af_cluster_command_t *cmd,
                                      uint16_t clusterId,
                                      uint8_t *buffer,
                                      uint16_t bufferLength,
                                      uint16_t *bufferIndex,
                                      uint8_t startId,
                                      uint8_t maxIdCount)
{
  UNUSED_VAR(outgoing);
  UNUSED_VAR(cmd);
  UNUSED_VAR(clusterId);
  UNUSED_VAR(buffer);
  UNUSED_VAR(bufferLength);
  UNUSED_VAR(bufferIndex);
  UNUSED_VAR(startId);
  UNUSED_VAR(maxIdCount);
  return true;
}
#endif
