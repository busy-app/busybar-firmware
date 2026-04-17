/***************************************************************************//**
 * @file af-host.c
 * @brief Host-specific APIs and infrastructure code.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "micro.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#include "sl_status.h"
#include "stack/include/sl_zigbee_dhc.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/security/crypto-state.h"
#include "zigbee_host_callback_dispatcher.h"

// ZDO - ZigBee Device Object
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "app/util/zigbee-framework/zigbee-device-host.h"

#include "app/framework/plugin/zcl-framework-core/zcl-framework-core.h"

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
#include "app/framework/plugin/fragmentation/fragmentation.h"
#endif // SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT

#define MAX_CLUSTER (EZSP_MAX_FRAME_LENGTH) / 2 //currently == 94

extern char *sl_zigbee_ezsp_get_dhc_json_path(void);

// This is used to store the local EUI of the NCP when using
// fake certificates.
// Fake certificates are constructed by setting the data to all F's
// but using the device's real IEEE in the cert.  The Key establishment
// code requires access to the local IEEE to do this.
sl_802154_long_addr_t sli_802154mac_local_eui64;

// the stack version that the NCP is running
static uint16_t ncpStackVer;
static uint32_t ezsp_error_queue_full_counter = 0;

static uint16_t cachedConfigIdValues[SL_ZIGBEE_EZSP_CONFIG_ID_MAX + 1];
static bool cacheConfigIdValuesAllowed = false;
static bool ncpNeedsResetAndInit = false;
static uint8_t ezspSequenceNumber = 0;

typedef struct {
  sl_802154_short_addr_t nodeId;
  sl_802154_pan_id_t  panId;
  sl_zigbee_network_status_t networkState;
  uint8_t radioChannel;
} sli_zigbee_network_cache_t;

//------------------------------------------------------------------------------
// Forward declarations
void sli_zigbee_af_reset_and_init_ncp(void);
static uint8_t getNcpConfigItem(sl_zigbee_ezsp_config_id_t id);
static void createEndpoint(uint8_t endpointIndex);
static uint8_t ezspNextSequence(void);

//------------------------------------------------------------------------------
// Internal callbacks

void sli_zigbee_af_init_cb(void)
{
  //Initialize the hal
  halInit();

  sl_zigbee_af_app_println("Reset info: %d (%s)",
                           halGetResetInfo(),
                           halGetResetString());
  sl_zigbee_af_core_flush();

  // This will initialize the stack of networks maintained by the framework,
  // including setting the default network.
  sli_zigbee_af_initialize_network_index_stack();

  // We must initialize the endpoint information first so
  // that they are correctly added by sli_zigbee_af_reset_and_init_ncp()
  sl_zigbee_af_endpoint_configure();

  // initialize the network co-processor (NCP)
  sli_zigbee_af_reset_and_init_ncp();
}

//------------------------------------------------------------------------------
// Public APIs

uint8_t sl_zigbee_af_get_address_table_size(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_ADDRESS_TABLE_SIZE);
}

uint8_t sl_zigbee_af_get_binding_table_size(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_BINDING_TABLE_SIZE);
}

sl_status_t sl_zigbee_af_get_child_data(uint8_t index,
                                        sl_zigbee_child_data_t *childData)
{
  return sl_zigbee_ezsp_get_child_data(index, childData);
}

uint8_t sl_zigbee_af_get_child_table_size(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_MAX_END_DEVICE_CHILDREN);
}

void sl_zigbee_af_get_mfg_string(uint8_t* returnData)
{
  static uint8_t mfgString[MFG_STRING_MAX_LENGTH];
  static bool mfgStringRetrieved = false;

  if (mfgStringRetrieved == false) {
    sl_zigbee_ezsp_get_mfg_token(SL_ZIGBEE_EZSP_MFG_STRING, mfgString);
    mfgStringRetrieved = true;
  }
  // NOTE:  The MFG string is not NULL terminated.
  memmove(returnData, mfgString, MFG_STRING_MAX_LENGTH);
}

// Platform dependent interface to get various stack parameters.
void sl_zigbee_af_get_eui64(sl_802154_long_addr_t returnEui64)
{
  memcpy(returnEui64, sli_802154mac_local_eui64, EUI64_SIZE);
}

uint8_t sl_zigbee_af_get_key_table_size(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_KEY_TABLE_SIZE);
}

uint8_t sl_zigbee_af_get_neighbor_table_size(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_NEIGHBOR_TABLE_SIZE);
}

sl_status_t sl_zigbee_af_get_network_parameters(sl_zigbee_node_type_t* nodeType,
                                                sl_zigbee_network_parameters_t* parameters)
{
  return sl_zigbee_ezsp_get_network_parameters(nodeType, parameters);
}

sl_status_t sl_zigbee_af_get_node_type(sl_zigbee_node_type_t *nodeType)
{
  sl_zigbee_network_parameters_t parameters;
  return sl_zigbee_ezsp_get_network_parameters(nodeType, &parameters);
}

uint8_t sl_zigbee_af_get_open_network_duration_sec(void)
{
  uint8_t openTimeSec;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_NWK_OPEN_DURATION,
                           &valueLength,
                           &openTimeSec);
  return openTimeSec;
}

uint8_t sl_zigbee_af_get_route_table_size(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_ROUTE_TABLE_SIZE);
}

uint8_t sl_zigbee_af_get_security_level(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_SECURITY_LEVEL);
}

uint8_t sl_zigbee_af_get_sleepy_multicast_config(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_SEND_MULTICASTS_TO_SLEEPY_ADDRESS);
}

sl_status_t sl_zigbee_af_get_source_route_table_entry(uint8_t index,
                                                      sl_802154_short_addr_t *destination,
                                                      uint8_t *closerIndex)
{
  return sl_zigbee_ezsp_get_source_route_table_entry(index,
                                                     destination,
                                                     closerIndex);
}

uint8_t sl_zigbee_af_get_source_route_table_filled_size(void)
{
  return sl_zigbee_ezsp_get_source_route_table_filled_size();
}

uint8_t sl_zigbee_af_get_source_route_table_total_size(void)
{
  return sl_zigbee_ezsp_get_source_route_table_total_size();
}

uint8_t sl_zigbee_af_get_stack_profile(void)
{
  return getNcpConfigItem(SL_ZIGBEE_EZSP_CONFIG_STACK_PROFILE);
}

bool sl_zigbee_af_memory_byte_compare(const uint8_t* pointer,
                                      uint8_t count,
                                      uint8_t byteValue)
{
  uint8_t i;
  for (i = 0; i < count; i++, pointer++) {
    if (*pointer != byteValue) {
      return false;
    }
  }
  return true;
}

bool sl_zigbee_stack_is_up(void)
{
  sl_zigbee_network_status_t status = sl_zigbee_network_state();
  return (status == SL_ZIGBEE_JOINED_NETWORK
          || status == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT);
}

// this function sets an EZSP config value and
// prints out the results to the serial output
sl_status_t sl_zigbee_af_set_ezsp_config_value(sl_zigbee_ezsp_config_id_t configId,
                                               uint16_t value,
                                               const char * configIdName)
{
  uint16_t old_value;
  // Some configuration values cannot be written past a certain point
  // Check to see if the old config value is the same as the new so it doesnt
  // print a bunch of errors when the host is restarted without the NCP undergoing a reset

  // unused marker avoids compiler warnings if debug print is disabled
  UNUSED sl_status_t status = sl_zigbee_ezsp_get_configuration_value(configId, &old_value);
  UNUSED sl_zigbee_ezsp_status_t ezspStatus = sl_zigbee_ezsp_get_last_status();

  if (status == SL_STATUS_OK && old_value != value ) {
    status = sl_zigbee_ezsp_set_configuration_value(configId, value);
  }
  sl_zigbee_af_app_flush();
  sl_zigbee_af_app_print("Ezsp Config: set %s to 0x%04X:", configIdName, value);

  sl_zigbee_af_app_debug_exec(sli_zigbee_af_print_status("set", status));
  sl_zigbee_af_app_flush();

  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_flush();

  // If this fails, odds are the simulated NCP doesn't have enough
  // memory allocated to it.
  // SL_ZIGBEE_EZSP_ERROR_INVALID_ID status for unsupported configuration IDs on NCP such as
  // ZLL where not all NCPs need or support it.
  SL_ZIGBEE_TEST_ASSERT((ezspStatus == SL_ZIGBEE_EZSP_SUCCESS) || (ezspStatus == SL_ZIGBEE_EZSP_ERROR_INVALID_ID));
  return status;
}

void sl_zigbee_ezsp_set_stack_profile(uint8_t stackProfile)
{
  (void) sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_STACK_PROFILE, stackProfile, "stack profile");
}

void sl_zigbee_ezsp_set_security_level(uint8_t securityLevel)
{
  (void) sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_SECURITY_LEVEL, securityLevel, "security level");
}

// this function sets an EZSP policy and
// prints out the results to the serial output
sl_status_t sl_zigbee_af_set_ezsp_policy(sl_zigbee_ezsp_policy_id_t policyId,
                                         sl_zigbee_ezsp_decision_id_t decisionId,
                                         const char * policyName,
                                         const char * decisionName)
{
  UNUSED sl_status_t status = sl_zigbee_ezsp_set_policy(policyId,
                                                        decisionId);
  sl_zigbee_af_app_print("Ezsp Policy: set %s to \"%s\":",
                         policyName,
                         decisionName);
  sl_zigbee_af_app_debug_exec(sli_zigbee_af_print_status("set",
                                                         status));
  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_flush();
  return status;
}

// this function sets an EZSP value and
// prints out the results to the serial output
sl_status_t sl_zigbee_af_set_ezsp_value(sl_zigbee_ezsp_value_id_t valueId,
                                        uint8_t valueLength,
                                        uint8_t *value,
                                        const char * valueName)
{
  UNUSED sl_status_t status = sl_zigbee_ezsp_set_value(valueId, valueLength, value);

  sl_zigbee_af_app_print("Ezsp Value : set %s to ", valueName);

  // print the value based on the length of the value
  // for length 1/2/4 bytes, fetch int of that length and promote to 32 bits for printing
  switch (valueLength) {
    case 1:
      sl_zigbee_af_app_print("0x%08X:", (uint32_t)(*value));
      break;
    case 2:
      sl_zigbee_af_app_print("0x%08X:", (uint32_t)(*((uint16_t *)value)));
      break;
    case 4:
      sl_zigbee_af_app_print("0x%08X:", (uint32_t)(*((uint32_t *)value)));
      break;
    default:
      sl_zigbee_af_app_print("{val of len %02X}:", valueLength);
      break;
  }

  sl_zigbee_af_app_debug_exec(sli_zigbee_af_print_status("set", status));
  sl_zigbee_af_app_println("");
  sl_zigbee_af_app_flush();
  return status;
}

// This will cache all config items to make sure
// repeated calls do not go all the way to the NCP.
static uint8_t getNcpConfigItem(sl_zigbee_ezsp_config_id_t id)
{
  // In case we can't cache config items yet, we need a temp
  // variable to store the retrieved EZSP config ID.
  uint16_t temp = 0xFFFF;
  uint16_t *configItemPtr = &temp;
  bool cacheValid;

  SL_ZIGBEE_TEST_ASSERT(id <= SL_ZIGBEE_EZSP_CONFIG_ID_MAX);

  cacheValid = (cacheConfigIdValuesAllowed
                && id <= SL_ZIGBEE_EZSP_CONFIG_ID_MAX);

  if (cacheValid) {
    configItemPtr = &(cachedConfigIdValues[id]);
  }

  if (*configItemPtr == 0xFFFF
      && SL_ZIGBEE_EZSP_SUCCESS != sl_zigbee_ezsp_get_configuration_value(id, configItemPtr)) {
    // We return a 0 size (for tables) on error to prevent code from using the
    // invalid value of 0xFFFF.  This is particularly necessary for loops that
    // iterate over all indexes.
    return 0;
  }

  return (uint8_t)(*configItemPtr);
}

bool sl_zigbee_af_ncp_needs_reset(void)
{
  return ncpNeedsResetAndInit;
}

//------------------------------------------------------------------------------
// Internal APIs

// initialize the network co-processor (NCP)
void sli_zigbee_af_reset_and_init_ncp(void)
{
  uint8_t ep;
  sl_zigbee_ezsp_status_t ezspStatus;
  uint16_t seed0, seed1;

  sl_zigbee_af_pre_ncp_reset_cb();

  // sl_zigbee_ezsp_init resets the NCP by calling halNcpHardReset on a SPI host or
  // ashResetNcp on a UART host
  ezspStatus = sl_zigbee_ezsp_init();

  if (ezspStatus != SL_ZIGBEE_EZSP_SUCCESS) {
    sl_zigbee_af_core_println("ERROR: ezspForceReset 0x%02X", ezspStatus);
    sl_zigbee_af_core_flush();
    assert(false);
  }

  // send the version command before any other commands
  sli_zigbee_af_cli_version_command();

  // The random number generator on the host needs to be seeded with some
  // random data, which we can get from the NCP.
  sl_zigbee_ezsp_get_random_number(&seed0);
  sl_zigbee_ezsp_get_random_number(&seed1);
  halStackSeedRandom(((uint32_t)seed1 << 16) | (uint32_t)seed0);

  // set the address table size
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_ADDRESS_TABLE_SIZE,
                                     SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE,
                                     "address table size");

  // set the trust center address cache size
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_TRUST_CENTER_ADDRESS_CACHE_SIZE,
                                     SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE,
                                     "TC addr cache");

  // BUG 14222: If stack profile is 2 (ZigBee Pro), we need to enforce
  // the standard stack configuration values for that feature set.
  if ( SL_ZIGBEE_STACK_PROFILE == 2 ) {
    // MAC indirect timeout should be 7.68 secs
    sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT,
                                       7680,
                                       "MAC indirect TX timeout");

    // Max hops should be 2 * nwkMaxDepth, where nwkMaxDepth is 15
    sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_MAX_HOPS,
                                       30,
                                       "max hops");
  }

  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_TX_POWER_MODE,
                                     SL_ZIGBEE_AF_TX_POWER_MODE,
                                     "tx power mode");

  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_SUPPORTED_NETWORKS,
                                     SL_ZIGBEE_SUPPORTED_NETWORKS,
                                     "supported networks");
  sli_zigbee_af_network_security_init();

  uint8_t mode = SL_ZIGBEE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE;

  sl_zigbee_af_set_ezsp_value(SL_ZIGBEE_EZSP_VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE,
                              1, // value length
                              &mode,
                              "end device keep alive support mode");

  // allow other devices to modify the binding table
  sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_BINDING_MODIFICATION_POLICY,
                               SL_ZIGBEE_EZSP_CHECK_BINDING_MODIFICATIONS_ARE_VALID_ENDPOINT_CLUSTERS,
                               "binding modify",
                               "allow for valid endpoints & clusters only");

  // return message tag and message contents in sl_zigbee_message_sent_handler()
  sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_MESSAGE_CONTENTS_IN_CALLBACK_POLICY,
                               SL_ZIGBEE_EZSP_MESSAGE_TAG_AND_CONTENTS_IN_CALLBACK,
                               "message content in msgSent",
                               "return");

  {
    uint8_t value[2];
    value[0] = LOW_BYTE(SL_ZIGBEE_AF_INCOMING_BUFFER_LENGTH);
    value[1] = HIGH_BYTE(SL_ZIGBEE_AF_INCOMING_BUFFER_LENGTH);
    sl_zigbee_af_set_ezsp_value(SL_ZIGBEE_EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE,
                                2, // value length
                                value,
                                "maximum incoming transfer size");
    value[0] = LOW_BYTE(SL_ZIGBEE_AF_MAXIMUM_SEND_PAYLOAD_LENGTH);
    value[1] = HIGH_BYTE(SL_ZIGBEE_AF_MAXIMUM_SEND_PAYLOAD_LENGTH);
    sl_zigbee_af_set_ezsp_value(SL_ZIGBEE_EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE,
                                2, // value length
                                value,
                                "maximum outgoing transfer size");

    value[0] = LOW_BYTE(SL_ZIGBEE_TRANSIENT_DEVICE_DEFAULT_TIMEOUT_MS);
    value[1] = HIGH_BYTE(SL_ZIGBEE_TRANSIENT_DEVICE_DEFAULT_TIMEOUT_MS);
    sl_zigbee_af_set_ezsp_value(SL_ZIGBEE_EZSP_VALUE_TRANSIENT_DEVICE_TIMEOUT,
                                2,
                                value,
                                "default timeout for transient device table");
  }

  // Set the manufacturing code. This is defined by ZigBee document 053874r10
  // Ember's ID is 0x1002 and is the default, but this can be overridden in App Builder.
  sl_zigbee_set_manufacturer_code(SL_ZIGBEE_ZCL_MANUFACTURER_CODE);

  sli_zb_af_ncp_init();
  sl_zigbee_af_ncp_init_cb();

#ifdef SL_ZIGBEE_AF_MAX_TOTAL_CLUSTER_COUNT
#if SL_ZIGBEE_AF_MAX_TOTAL_CLUSTER_COUNT > MAX_CLUSTER
#error "ERROR: too many clusters are enabled on some of the endpoints"
#endif
#endif //#ifdef SL_ZIGBEE_AF_MAX_TOTAL_CLUSTER_COUNT

  // create endpoints
  for ( ep = 0; ep < sl_zigbee_af_endpoint_count(); ep++ ) {
    sl_zigbee_ezsp_endpoint_flags_t flags;
    uint8_t endpoint = sl_zigbee_af_endpoint_from_index(ep);;

    // check to see if sl_zigbee_ezsp_add_endpoint needs to be called
    // if sl_zigbee_ezsp_init is called without NCP reset, sl_zigbee_ezsp_add_endpoint is not necessary and will return an error
    if ( sl_zigbee_ezsp_get_endpoint_flags(endpoint, &flags) != SL_STATUS_OK ) {
      createEndpoint(ep);
    } else {
      sl_zigbee_af_app_println("Ezsp Endpoint %d previously added", endpoint);
    }
  }

  memset(cachedConfigIdValues, 0xFF, ((SL_ZIGBEE_EZSP_CONFIG_ID_MAX + 1) * sizeof(uint16_t)));
  cacheConfigIdValuesAllowed = true;

  // Set the localEui64 global
  sl_zigbee_ezsp_get_eui64(sli_802154mac_local_eui64);

  // Initialize messageSentCallbacks table
  sli_zigbee_af_initialize_message_sent_callback_array();

#ifdef SL_CATALOG_ZIGBEE_DHC_PRESENT
  {
    char *dhcPath = sl_zigbee_ezsp_get_dhc_json_path();
    if (dhcPath != NULL && dhcPath[0] != '\0') {
      char *dhcPathCopy = NULL;
      size_t dhcPathLen = strlen(dhcPath) + 1U;

      dhcPathCopy = malloc(dhcPathLen);
      if (dhcPathCopy != NULL) {
        memcpy(dhcPathCopy, dhcPath, dhcPathLen);
      } else {
        sl_zigbee_af_core_println("Unable to duplicate DHC path; using original buffer.");
        dhcPathCopy = dhcPath;
      }

      sl_status_t dhcStatus = sl_zigbee_dhc_init_from_json(dhcPathCopy, false, true);
      if (dhcStatus != SL_STATUS_OK) {
        sl_zigbee_af_core_println("DHC JSON load failed (0x%04X): %s",
                                  (uint16_t)dhcStatus,
                                  dhcPathCopy);
      }

      if (dhcPathCopy != dhcPath) {
        free(dhcPathCopy);
      }
    }
  }
#endif
}
#ifdef EZSP_CPC
extern bool in_ncp_reset(void);
#else
#define in_ncp_reset() false
#endif
void sli_zigbee_af_host_framework_tick(void)
{
  do {
    halResetWatchdog();   // Periodically reset the watchdog.

    // see if the NCP has anything waiting to send us
    sl_zigbee_ezsp_tick();

    while (sl_zigbee_ezsp_callback_pending()) {
      sl_zigbee_ezsp_callback();
    }

    // check if we have hit an EZSP Error and need to reset and init the NCP
    if (ncpNeedsResetAndInit || in_ncp_reset() ) {
      ncpNeedsResetAndInit = false;
      // re-initialize the NCP
      sl_zigbee_af_app_print("ezsp_error_queue_full_counter = 0x%0X\n", ezsp_error_queue_full_counter);
      ezsp_error_queue_full_counter = 0;
      sli_zigbee_af_reset_and_init_ncp();
      sli_zigbee_af_network_init(SL_ZIGBEE_INIT_LEVEL_DONE);
    }
    // Wait until ECC operations are done.  Don't allow any of the clusters
    // to send messages as the NCP is busy doing ECC
  } while (sli_zigbee_af_is_crypto_operation_in_progress());
}

void sli_zigbee_af_cli_version_command(void)
{
  // If there was an ezsp error prior to this function call that leads to bad information
  // being read from the NCP, the host asserts (ncp stack type is not mesh, ezsp version mismatch, etc)

  // This is especially important for info command where there is a ton of information that is read and
  // while the ezsp error handler fires, the main loop needs to run again before the cause of the error handler
  // is actually addressed. However, the info command will finish executing and the host will assert before
  // the error handler fires, causing the host to crash

  if (ncpNeedsResetAndInit) {
    return;
  }

  // Note that NCP == Network Co-Processor
  sl_zigbee_version_t versionStruct;

  // the EZSP protocol version that the NCP is using
  uint8_t ncpEzspProtocolVer;

  // the stackType that the NCP is running
  uint8_t ncpStackType;

  // the EZSP protocol version that the Host is running
  // we are the host so we set this value
  uint8_t hostEzspProtocolVer = EZSP_PROTOCOL_VERSION;

  // send the Host version number to the NCP. The NCP returns the EZSP
  // version that the NCP is running along with the stackType and stackVersion
  ncpEzspProtocolVer = sl_zigbee_ezsp_version(hostEzspProtocolVer,
                                              &ncpStackType,
                                              &ncpStackVer);

  // verify that the stack type is what is expected
  if (ncpStackType != EZSP_STACK_TYPE_MESH) {
    sl_zigbee_af_app_print("ERROR: stack type 0x%02X is not expected!",
                           ncpStackType);
    assert(false);
  }

  // verify that the NCP EZSP Protocol version is what is expected
  if (ncpEzspProtocolVer != EZSP_PROTOCOL_VERSION) {
    sl_zigbee_af_app_print("ERROR: NCP EZSP protocol version of 0x%02X does not match Host version 0x%02X\r\n",
                           ncpEzspProtocolVer,
                           hostEzspProtocolVer);
    assert(false);
  }

  sl_zigbee_af_app_print("ezsp ver 0x%02X stack type 0x%02X ",
                         ncpEzspProtocolVer, ncpStackType, ncpStackVer);

  if (SL_STATUS_OK != sl_zigbee_ezsp_get_version_struct(&versionStruct)) {
    // NCP has Old style version number
    sl_zigbee_af_app_println("stack ver [0x%04X]", ncpStackVer);
  } else {
    // NCP has new style version number
    sli_zigbee_af_parse_and_print_version(versionStruct);
  }
  sl_zigbee_af_app_flush();
}

uint16_t sli_zigbee_af_get_packet_buffer_free_space(void)
{
  uint16_t freeCount;
  uint8_t valueLength = 2;
  (void)sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_BUFFER_HEAP_FREE_SIZE,
                                 &valueLength,
                                 (uint8_t *) &freeCount);
  (void) valueLength;
  return freeCount;
}

uint16_t sli_zigbee_af_get_packet_buffer_total_space(void)
{
  uint16_t value;
  (void) sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_PACKET_BUFFER_HEAP_SIZE,
                                                &value);
  return value;
}

sl_status_t sli_zigbee_af_send(sl_zigbee_outgoing_message_type_t type,
                               uint16_t indexOrDestination,
                               sl_zigbee_aps_frame_t *apsFrame,
                               uint8_t messageLength,
                               uint8_t *message,
                               uint16_t *messageTag,
                               sl_802154_short_addr_t alias,
                               uint8_t sequence)
{
  sl_status_t status = SL_STATUS_OK;
  *messageTag = ezspNextSequence();
  uint8_t nwkRadius = ZA_MAX_HOPS;
  sl_802154_short_addr_t nwkAlias = SL_ZIGBEE_NULL_NODE_ID;

  switch (type) {
    case SL_ZIGBEE_OUTGOING_VIA_BINDING:
    case SL_ZIGBEE_OUTGOING_VIA_ADDRESS_TABLE:
    case SL_ZIGBEE_OUTGOING_DIRECT:
    {
      status = sl_zigbee_send_unicast(type,
                                      indexOrDestination,
                                      apsFrame,
                                      *messageTag,
                                      (uint8_t)messageLength,
                                      message,
                                      &apsFrame->sequence);

      break;
    }
    case SL_ZIGBEE_OUTGOING_MULTICAST:
    case SL_ZIGBEE_OUTGOING_MULTICAST_WITH_ALIAS:
    {
      if (type == SL_ZIGBEE_OUTGOING_MULTICAST_WITH_ALIAS
          || (apsFrame->sourceEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->destinationEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->options & SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER)) {
        nwkRadius = apsFrame->radius;
        nwkAlias = alias;
      }
      status = sl_zigbee_send_multicast(apsFrame,
                                        nwkRadius,      //radius
                                        0,      // broadcast Addr
                                        nwkAlias,
                                        sequence,
                                        *messageTag,
                                        messageLength,
                                        message,
                                        &apsFrame->sequence);
      break;
    }
    case SL_ZIGBEE_OUTGOING_BROADCAST:
    case SL_ZIGBEE_OUTGOING_BROADCAST_WITH_ALIAS:
    {
      if (type == SL_ZIGBEE_OUTGOING_BROADCAST_WITH_ALIAS
          || (apsFrame->sourceEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->destinationEndpoint == SL_ZIGBEE_GP_ENDPOINT
              && apsFrame->options & SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER)) {
        nwkRadius = apsFrame->radius;
        nwkAlias = alias;
      }
      status = sl_zigbee_send_broadcast(nwkAlias,
                                        indexOrDestination,
                                        sequence,
                                        apsFrame,
                                        nwkRadius, // radius
                                        *messageTag, // tag
                                        messageLength,
                                        message,
                                        &apsFrame->sequence);
      break;
    }
    default:
      status = SL_STATUS_INVALID_PARAMETER;
  }

  return status;
}

void sli_zigbee_af_print_ezsp_endpoint_flags(uint8_t endpoint)
{
  sl_zigbee_ezsp_endpoint_flags_t flags;
  sl_status_t status = sl_zigbee_ezsp_get_endpoint_flags(endpoint,
                                                         &flags);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_print("Error retrieving EZSP endpoint flags.");
  } else {
    sl_zigbee_af_core_print("- EZSP Endpoint flags: 0x%04X", flags);
  }
}

// WARNING:  This function executes in ISR context
void halNcpIsAwakeIsr(bool isAwake)
{
  if (isAwake) {
    sl_zigbee_af_ncp_is_awake_isr_cb();
  } else {
    // If we got indication that the NCP failed to wake up
    // there is not much that can be done.  We will reset the
    // host (which in turn will reset the NCP) and that will
    // hopefully bring things back in sync.
    assert(0);
  }
}

//------------------------------------------------------------------------------
// Static functions

// Creates the endpoint for 260 by calling sl_zigbee_ezsp_add_endpoint()
static void createEndpoint(uint8_t endpointIndex)
{
  uint16_t clusterList[MAX_CLUSTER];
  uint16_t *inClusterList;
  uint16_t *outClusterList;

  uint8_t endpoint = sl_zigbee_af_endpoint_from_index(endpointIndex);

  //to cover the code path that gets here after unexpected NCP reset
  bool initiallyDisabled = !sl_zigbee_af_endpoint_is_enabled(endpoint);
  if (initiallyDisabled) {
    sl_zigbee_af_endpoint_enable_disable(endpoint, true);
  }

  uint8_t inClusterCount;
  uint8_t outClusterCount;

  {
    sl_status_t status = sl_zigbee_af_push_endpoint_network_index(endpoint);
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_app_println("Error in creating endpoint %d: 0x%02X", endpoint, status);
      return;
    }
  }

  // Lay out clusters in the arrays.
  inClusterList = clusterList;
  inClusterCount = sl_zigbee_af_get_clusters_from_endpoint(endpoint, inClusterList, MAX_CLUSTER, true);

  outClusterList = clusterList + inClusterCount;
  outClusterCount = sl_zigbee_af_get_clusters_from_endpoint(endpoint, outClusterList, (MAX_CLUSTER - inClusterCount), false);

  // Call EZSP function with data.
  {
    sl_status_t status = sl_zigbee_ezsp_add_endpoint(endpoint,
                                                     sl_zigbee_af_profile_id_from_index(endpointIndex),
                                                     sl_zigbee_af_device_id_from_index(endpointIndex),
                                                     sl_zigbee_af_device_version_from_index(endpointIndex),
                                                     inClusterCount,
                                                     outClusterCount,
                                                     (uint16_t *)inClusterList,
                                                     (uint16_t *)outClusterList);
    if (status == SL_STATUS_OK) {
      //this is to sync the host and NCP, after an internal EZSP ERROR and NCP reset
      if (initiallyDisabled) {
        sl_zigbee_af_endpoint_enable_disable(endpoint, false);
      }
      sl_zigbee_af_app_println("Ezsp Endpoint %d added, profile 0x%04X, in clusters: %d, out clusters %d",
                               endpoint,
                               sl_zigbee_af_profile_id_from_index(endpointIndex),
                               inClusterCount,
                               outClusterCount);
    } else {
      sl_zigbee_af_app_println("Error in creating endpoint %d: 0x%02X", endpoint, status);
    }
  }

  (void) sl_zigbee_af_pop_network_index();
}

static uint8_t ezspNextSequence(void)
{
  return ((++ezspSequenceNumber) & SL_ZIGBEE_AF_MESSAGE_TAG_MASK);
}

//------------------------------------------------------------------------------
// EZSP Handlers

// This is called when an EZSP error is reported
void sl_zigbee_ezsp_error_handler(sl_zigbee_ezsp_status_t status)
{
  if ( status != SL_ZIGBEE_EZSP_ERROR_QUEUE_FULL ) {
    sl_zigbee_af_core_println("ERROR: sl_zigbee_ezsp_error_handler 0x%02X", status);
  } else {
    ezsp_error_queue_full_counter++;
  }
  sl_zigbee_af_core_flush();

  ncpNeedsResetAndInit |= sl_zigbee_af_zcl_framework_core_ezsp_error_cb(status);
}

//------------------------------------------------------------------------------
// Stack Callbacks for Dispatcher

void sli_zigbee_af_incoming_message_callback(sl_zigbee_incoming_message_type_t type,
                                             sl_zigbee_aps_frame_t *apsFrame,
                                             sl_zigbee_rx_packet_info_t *packetInfo,
                                             uint8_t messageLength,
                                             uint8_t *message)
{
  uint8_t sourceRouteOverhead;
  sl_802154_short_addr_t sender = packetInfo->sender_short_id;
  (void) sl_zigbee_af_push_callback_network_index();

  // The following code caches valid Source Route overheads sent pro actively
  // by the NCP and uses it once to calculate the overhead for a target, after
  // which it gets cleared.
  sourceRouteOverhead = getSourceRouteOverhead(messageLength);
  sl_zigbee_af_set_source_route_overhead_cb(sender, sourceRouteOverhead);

  sli_zigbee_af_incoming_message_handler(type,
                                         apsFrame,
                                         packetInfo,
                                         messageLength,
                                         message);

  // Invalidate the sourceRouteOverhead cached at the end of the current incomingMessageHandler
  sl_zigbee_af_set_source_route_overhead_cb(sender, SL_ZIGBEE_SOURCE_ROUTE_OVERHEAD_UNKNOWN);
  (void) sl_zigbee_af_pop_network_index();
}

extern void sli_zigbee_af_message_sent(sl_status_t status,
                                       sl_zigbee_outgoing_message_type_t type,
                                       uint16_t indexOrDestination,
                                       sl_zigbee_aps_frame_t *apsFrame,
                                       uint16_t messageTag,
                                       uint8_t messageLength,
                                       uint8_t *messageContents);

// Called when a message we sent is acked by the destination or when an
// ack fails to arrive after several retransmissions.
void sli_zigbee_af_message_sent_callback(sl_status_t status,
                                         sl_zigbee_outgoing_message_type_t type,
                                         uint16_t indexOrDestination,
                                         sl_zigbee_aps_frame_t *apsFrame,
                                         uint16_t messageTag,
                                         uint8_t messageLength,
                                         uint8_t *messageContents)
{
#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  // This call if returns true, ends up calling
  // sli_zigbee_af_fragmentation_message_sent_handler() when the last fragment was received.
  if (sli_zigbee_af_fragmentation_message_sent(apsFrame, status)) {
    return;
  }
#endif //SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT

  sli_zigbee_af_message_sent_handler(status,
                                     type,
                                     indexOrDestination,
                                     apsFrame,
                                     messageTag,
                                     messageLength,
                                     messageContents);

  // Generated dispatcher
  sli_zigbee_af_message_sent(status,
                             type,
                             indexOrDestination,
                             apsFrame,
                             messageTag,
                             messageLength,
                             messageContents);
}

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
void sli_zigbee_af_fragmentation_message_sent_handler(sl_status_t status,
                                                      sl_zigbee_outgoing_message_type_t type,
                                                      uint16_t indexOrDestination,
                                                      sl_zigbee_aps_frame_t *apsFrame,
                                                      uint16_t messageTag,
                                                      uint8_t *buffer,
                                                      uint16_t bufLen)
{
  // the fragmented message is no longer in process
  sl_zigbee_af_debug_println("%send.", "Fragmentation:");

  sli_zigbee_af_message_sent_handler(status,
                                     type,
                                     indexOrDestination,
                                     apsFrame,
                                     messageTag,
                                     bufLen,
                                     buffer);

  // Generated dispatcher
  sli_zigbee_af_message_sent(status,
                             type,
                             indexOrDestination,
                             apsFrame,
                             messageTag,
                             bufLen,
                             buffer);

  // EMZIGBEE-4437: setting back the buffers to the original in case someone set
  // that to something else.
  sl_zigbee_af_set_external_buffer(appResponseData,
                                   SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN,
                                   &appResponseLength,
                                   &sl_zigbee_af_response_aps_frame);
}
#endif // SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
