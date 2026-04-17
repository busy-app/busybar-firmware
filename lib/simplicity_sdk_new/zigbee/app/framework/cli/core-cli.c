/***************************************************************************//**
 * @file
 * @brief Core CLI commands used by all applications regardless of profile.
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/af-main.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "library.h"
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
#include "sl_rail_ieee802154.h"
#endif
#ifdef SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE
  #include "stack/include/cbke-crypto-engine.h"  // sl_zigbee_get_certificate()
#endif

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "app/framework/common/zigbee_rtos_adaptation.h"
#endif //#ifdef SL_CATALOG_KERNEL_PRESENT

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
#include "test-harness-z3-config.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
#include "app/test/pro-compliance.h"
#endif

extern uint8_t sli_zigbee_af_cli_network_index;

#if defined SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT && !defined EMBER_TEST
extern void sli_get_ztt_version_number_string(char *versionString);
#endif

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
void  sli_get_pti_radio_config(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_core_debug_println("Current Config: %0x", sl_rail_ieee802154_get_phy_id(sl_zigbee_get_rail_handle()));
}

#endif

void sli_zigbee_cli_config_cca_mode_command(sl_cli_command_arg_t *arguments)
{
  uint8_t ccaMode = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = sl_zigbee_set_radio_ieee802154_cca_mode(ccaMode);
  sl_zigbee_core_debug_println("Set CCA mode to %d: %0x",
                               ccaMode,
                               status);
}

void sli_cli_pre_cmd_hook(sl_cli_command_arg_t* arguments)
{
  (void)arguments;
  sl_zigbee_af_push_network_index(sli_zigbee_af_cli_network_index);
#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_acquire_lock();
#endif
}

void sli_cli_post_cmd_hook(sl_cli_command_arg_t* arguments)
{
  (void)arguments;

  sl_zigbee_af_pop_network_index();

#if defined(EMBER_QA)
  sl_zigbee_core_debug_println("CLI Finished");
#endif

#ifdef SL_CATALOG_KERNEL_PRESENT
  // After every CLI command we wake up the ZigBee task so that it can be
  // processed in a timely fashion.
  sl_zigbee_af_release_lock();
  sl_zigbee_wakeup_common_task();
#endif // SL_CATALOG_KERNEL_PRESENT
}

#if !defined SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT || defined SL_ZIGBEE_TEST
static void printMfgString(void)
{
  uint8_t mfgString[MFG_STRING_MAX_LENGTH + 1] = { 0 };
  sl_zigbee_af_format_mfg_string(mfgString);

  // Note: We use '%s' here because this is a RAM string. Normally,
  // most strings are literals or constants in flash and use '%s'.
  sl_zigbee_core_debug_println("MFG String: %s", mfgString);
}
#endif

static void printPacketBufferStats(void)
{
  sl_zigbee_core_debug_println("Buffer bytes: %d / %d",
                               sli_zigbee_af_get_packet_buffer_free_space(),
                               sli_zigbee_af_get_packet_buffer_total_space());
}

static bool printSmartEnergySecurityInfo(void)
{
#ifdef SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE
  bool securityGood = true;
  sl_zigbee_core_debug_print("SE Security Info [");
  {
    // For SE security, print the state of ECC, CBKE, and the programmed Cert.
    sl_zigbee_certificate_data_t cert;
    sl_status_t status = sl_zigbee_get_certificate(&cert);

    // Check the status of the ECC library.
    if ((sl_zigbee_get_library_status(SL_ZIGBEE_ECC_LIBRARY_ID)
         & SL_ZIGBEE_LIBRARY_PRESENT_MASK) != 0U) {
      sl_zigbee_core_debug_print("Real163k1ECC ");
    } else {
      sl_zigbee_core_debug_print("No163k1ECC ");
      securityGood = false;
    }

    // Status of SL_STATUS_NOT_AVAILABLE means the CBKE is not present
    // in the image and that no information is known about the certificate.
    if (status == SL_STATUS_NOT_AVAILABLE) {
      sl_zigbee_core_debug_print("No163k1Cbke UnknownCert ");
      securityGood = false;
    } else {
      sl_zigbee_core_debug_print("Real163k1Cbke ");

      if (status == SL_STATUS_OK) {
        sl_zigbee_core_debug_print("GoodCert");
      } else {
        sl_zigbee_core_debug_print("BadCert");
        securityGood = false;
      }
    }
    sl_zigbee_core_debug_println("]");
  }
  sl_zigbee_af_app_flush();
  return securityGood;
#else
  return false;
#endif
}

static bool printSmartEnergySecurityInfo283k1(void)
{
#ifdef SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE
  bool securityGood = true;
  sl_zigbee_core_debug_print("SE Security 283k1 Info [");
  {
    // For SE security, print the state of ECC, CBKE, and the programmed Cert.
    sl_zigbee_certificate_283k1_data_t cert;
    // This is a temporary step until the EZSP frames are added
    // to the EZSP code.
    sl_status_t status = SL_STATUS_NOT_AVAILABLE;
    status = sl_zigbee_get_certificate_283k1(&cert);
    // Check the status of the ECC library.
    if ((sl_zigbee_get_library_status(SL_ZIGBEE_ECC_LIBRARY_283K1_ID)
         & SL_ZIGBEE_LIBRARY_PRESENT_MASK) != 0U) {
      sl_zigbee_core_debug_print("Real283k1ECC ");
    } else {
      sl_zigbee_core_debug_print("No283k1ECC ");
      securityGood = false;
    }

    // Status of SL_STATUS_NOT_AVAILABLE means the CBKE is not present
    // in the image and that no information is known about the certificate.
    if (status == SL_STATUS_NOT_AVAILABLE) {
      sl_zigbee_core_debug_print("No283k1Cbke UnknownCert");
      securityGood = false;
    } else {
      sl_zigbee_core_debug_print("Real283k1Cbke ");

      if (status == SL_STATUS_OK) {
        sl_zigbee_core_debug_print("GoodCert");
      } else {
        sl_zigbee_core_debug_print("BadCert");
        securityGood = false;
      }
    }
    sl_zigbee_core_debug_println("]");
  }
  sl_zigbee_af_app_flush();
  return securityGood;
#else
  return false;
#endif
}
#if defined(SL_ZIGBEE_TEST) && !defined(EZSP_HOST)
extern void blacklist_mask(uint32_t channel_mask);
void blackListCommand(sl_cli_command_arg_t *arguments)
{
  uint32_t channel_mask = sl_cli_get_argument_uint32(arguments, 0);
  blacklist_mask(channel_mask);
}
#endif

#if defined SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT && !defined SL_ZIGBEE_TEST
// ZTT Firmware application version - displayed on LCD (for WSTK) and for "info" command.

void sli_get_ztt_version_number_string(char *versionString)
{
  memcpy(versionString, ZTT_FIRMWARE_APPLICATION_VERSION, strlen(ZTT_FIRMWARE_APPLICATION_VERSION));
}
#endif

void sli_zigbee_af_cli_info_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_zigbee_node_type_t nodeTypeResult = 0xFF;
  sl_802154_long_addr_t myEui64;
  sl_zigbee_network_parameters_t networkParams;
  uint8_t numPhyInterfaces;
#if defined SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT && !defined SL_ZIGBEE_TEST
  char versionString[10] = { 0 };
  sli_get_ztt_version_number_string(versionString);
  sl_zigbee_app_debug_println("Silicon Labs ZTT Firmware Application v%s", versionString);
#else
  printMfgString();
#endif
  sl_zigbee_core_debug_println("AppBuilder MFG Code: 0x%04X", SL_ZIGBEE_ZCL_MANUFACTURER_CODE);
  sl_zigbee_af_get_eui64(myEui64);
  sl_zigbee_af_get_network_parameters(&nodeTypeResult, &networkParams);
  sl_zigbee_core_debug_print("node [");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(myEui64));
  sl_zigbee_af_app_flush();
  sl_zigbee_core_debug_println("] chan [%d] pwr [%d]",
                               networkParams.radioChannel,
                               networkParams.radioTxPower);

  numPhyInterfaces = sl_zigbee_get_phy_interface_count();

  if ((numPhyInterfaces > 1) && (255 != numPhyInterfaces)) {
    sl_zigbee_multi_phy_radio_parameters_t multiPhyRadioParams;
    uint8_t i;
    sl_status_t status;

    sl_zigbee_core_debug_println("Additional interfaces");
    for (i = 1; i < numPhyInterfaces; ++i) {
      sl_zigbee_core_debug_print("  %d: ", i);
      status = sl_zigbee_get_radio_parameters(i, &multiPhyRadioParams);
      switch (status) {
        case SL_STATUS_OK:
          sl_zigbee_core_debug_println("page [%d] chan [%d] pwr [%d]",
                                       multiPhyRadioParams.radioPage,
                                       multiPhyRadioParams.radioChannel,
                                       multiPhyRadioParams.radioTxPower);
          break;
        case SL_STATUS_NETWORK_DOWN:
        case SL_STATUS_NOT_JOINED:
          sl_zigbee_core_debug_println("not active");
          break;
        default:
          sl_zigbee_core_debug_println("error 0x%08X", status);
          break;
      }
    }
  }

  sl_zigbee_core_debug_print("panID [0x%04X] nodeID [0x%04X] ",
                             networkParams.panId,
                             sl_zigbee_af_get_node_id());
  sl_zigbee_af_app_flush();
  sl_zigbee_core_debug_print("xpan [0x");
  sl_zigbee_af_app_debug_exec(sl_zigbee_af_print_big_endian_eui64(networkParams.extendedPanId));
  sl_zigbee_core_debug_println("]");
  sl_zigbee_af_app_flush();

  #ifndef EZSP_HOST
  sl_zigbee_core_debug_println("parentID [0x%04X] parentRssi [%d]", sl_zigbee_get_parent_id(), sl_zigbee_get_avg_parent_rssi());
  sl_zigbee_af_app_flush();
  #endif // EZSP_HOST

#if !defined SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  sli_zigbee_af_cli_version_command();
#endif

  sl_zigbee_core_debug_print("nodeType [");
  if (nodeTypeResult != 0xFF) {
    sl_zigbee_core_debug_print("0x%02X", nodeTypeResult);
  } else {
    sl_zigbee_core_debug_print("unknown");
  }
  sl_zigbee_core_debug_println("]");
  sl_zigbee_af_app_flush();

  sl_zigbee_core_debug_println("Security level [%02X]", sl_zigbee_af_get_security_level());

  printSmartEnergySecurityInfo();
  printSmartEnergySecurityInfo283k1();

  sl_zigbee_core_debug_print("network state [%02X] ", sl_zigbee_af_network_state());
  printPacketBufferStats();

  // EMZIGBEE-5125: apps with lots of endpoints will wdog while printing
  halResetWatchdog();
  // Print the endpoint information.
  {
    uint8_t i, j;
    sl_zigbee_core_debug_println("Ep cnt: %d", sl_zigbee_af_endpoint_count());
    // Loop for each endpoint.
    for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
      sl_zigbee_af_endpoint_type_t *et = sli_zigbee_af_endpoints[i].endpointType;
      sl_zigbee_core_debug_print("ep %d [endpoint %s, device %s] ",
                                 sl_zigbee_af_endpoint_from_index(i),
                                 (sl_zigbee_af_endpoint_index_is_enabled(i)
                                  ? "enabled"
                                  : "disabled"),
                                 (sl_zigbee_af_is_device_enabled(sl_zigbee_af_endpoint_from_index(i))
                                  ? "enabled"
                                  : "disabled"));
      sl_zigbee_core_debug_println("nwk [%d] profile [0x%04X] devId [0x%04X] ver [0x%02X]",
                                   sl_zigbee_af_network_index_from_endpoint_index(i),
                                   sl_zigbee_af_profile_id_from_index(i),
                                   sl_zigbee_af_device_id_from_index(i),
                                   sl_zigbee_af_device_version_from_index(i));
      // Loop for the clusters within the endpoint.
      for (j = 0; j < et->clusterCount; j++) {
        sl_zigbee_af_cluster_t *zc = &(et->cluster[j]);
        sl_zigbee_core_debug_print("    %s cluster: 0x%04X ",
                                   (sl_zigbee_af_cluster_is_client(zc)
                                    ? "out(client)"
                                    : "in (server)"),
                                   zc->clusterId);
        sl_zigbee_af_app_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(zc->clusterId, sli_zigbee_af_get_manufacturer_code_for_cluster(zc)));
        sl_zigbee_core_debug_println("");
        sl_zigbee_af_app_flush();
      }
      sl_zigbee_af_app_flush();
      // EMZIGBEE-5125
      halResetWatchdog();
    }
    // EMZIGBEE-5125
    halResetWatchdog();
  }

  {
    const char * names[] = {
      SL_ZIGBEE_AF_GENERATED_NETWORK_STRINGS
    };
    uint8_t i;
    sl_zigbee_core_debug_println("Nwk cnt: %d", SL_ZIGBEE_SUPPORTED_NETWORKS);
    for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
      sl_zigbee_af_push_network_index(i);
      sl_zigbee_core_debug_println("nwk %d [%s]", i, names[i]);
      if (sli_zigbee_af_pro_is_current_network()) {
        sl_zigbee_core_debug_println("  nodeType [0x%02X]",
                                     sli_zigbee_af_current_zigbee_pro_network->nodeType);
        sl_zigbee_core_debug_println("  securityProfile [0x%02X]",
                                     sli_zigbee_af_current_zigbee_pro_network->securityProfile);
      }
      sl_zigbee_af_pop_network_index();
    }
  }
#ifdef SL_CATALOG_KERNEL_PRESENT
#ifdef SL_CATALOG_MICRIUMOS_KERNEL_PRESENT
  sl_zigbee_core_debug_println("RTOS [Micrium]");
#elif defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
  sl_zigbee_core_debug_println("RTOS [FreeRTOS]");
#endif // MICRIUMOS_KERNEL || FREERTOS_KERNEL
#else // SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_core_debug_println("RTOS [No]");
#endif // SL_CATALOG_KERNEL_PRESENT
}

void sli_zigbee_cli_version_command(sl_cli_command_arg_t *arguments)
{
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isZCPContext() == true) {
    versionCommandZCP(arguments);
    return;
  }
#endif
  (void)arguments;
  // Print the Version
  sli_zigbee_af_cli_version_command();
}

void resetCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  halReboot();
}

void sli_zigbee_cli_factory_reset_command(sl_cli_command_arg_t *arguments)
{
  // Right now this just wipes key table, binding table, and network state (which also wipes network keys)
  // Ideally this should become something that invalidates all persistent storage, including any PSA keys,
  // and reboots
  // This is only used for testing right now
  (void)arguments;
  (void)sl_zigbee_clear_key_table();
  (void)sl_zigbee_clear_binding_table();
  (void)sl_zigbee_write_node_data(true); // clear network token

  halReboot();
}

void endpointPrint(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint8_t i;
  for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
    sl_zigbee_core_debug_print("EP %d: %s ",
                               sli_zigbee_af_endpoints[i].endpoint,
                               (sl_zigbee_af_endpoint_index_is_enabled(i)
                                ? "Enabled"
                                : "Disabled"));
    sli_zigbee_af_print_ezsp_endpoint_flags(sli_zigbee_af_endpoints[i].endpoint);
    sl_zigbee_core_debug_println("");
  }
}

void enableDisableEndpoint(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = (uint8_t)sl_cli_get_argument_uint32(arguments, 0);
  bool enable = (sl_cli_get_command_string(arguments, 1)[0] == 'e'
                 ? true
                 : false);
  if (!sl_zigbee_af_endpoint_enable_disable(endpoint,
                                            enable)) {
    sl_zigbee_core_debug_println("Error:  Unknown endpoint.");
  }
}

#if SL_ZIGBEE_EVENT_DEBUG_ENABLED
#ifndef EVENT_QUEUE_LIST_END
#define EVENT_QUEUE_LIST_END ((sl_zigbee_af_event_t *) 1)
#endif
#endif // SL_ZIGBEE_EVENT_DEBUG_ENABLED

void printEvents(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#if SL_ZIGBEE_EVENT_DEBUG_ENABLED

  sl_zigbee_af_event_t *finger = sli_zigbee_af_app_event_queue.events;

  while (finger != EVENT_QUEUE_LIST_END) {
    sl_zigbee_core_debug_print("Event %s : ", (finger->actions.name == NULL
                                               ? "?"
                                               : finger->actions.name));
    // Get NWK Index
    if (sli_zigbee_af_event_is_network_event(finger)) {
      sl_zigbee_core_debug_print("NWK %d : ", sli_zigbee_af_event_get_network_index(finger));
    }

    // Get Endpoint
    if (sli_zigbee_af_event_is_endpoint_event(finger)) {
      sl_zigbee_core_debug_print("EP %d : ", sli_zigbee_af_event_get_endpoint(finger));
    }

    // Get Remaining Time
    sl_zigbee_core_debug_println("%d ms", sl_zigbee_af_event_get_remaining_ms(finger));
    finger = finger->next;
  }
#else
  sl_zigbee_core_debug_print("Enable event debug info in Core CLI component");
#endif // SL_ZIGBEE_EVENT_DEBUG_ENABLED
}
