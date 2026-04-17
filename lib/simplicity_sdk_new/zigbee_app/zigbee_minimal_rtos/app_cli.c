/***************************************************************************//**
 * @brief Zigbee - simple stack test application - CLI.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include PLATFORM_HEADER
#include "sl_zigbee.h"
#include "hal.h"

#include "sl_cli.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

#include "app.h"
#include "zigbee_app_framework_event.h"
#include "stack/include/zigbee-security-manager.h"

static sl_zigbee_af_event_t app_cli_event;
static void app_cli_event_handler(const sl_zigbee_af_event_t *event);

void app_cli_init(uint8_t init_level)
{
  (void) init_level;
  sl_zigbee_af_event_init(&app_cli_event, app_cli_event_handler);
}

void form_network_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  sl_zigbee_network_parameters_t parameters;
  memset( (void*)&parameters, 0, sizeof (parameters));
  parameters.radioChannel = sl_cli_get_argument_uint8(arguments, 0);
  parameters.panId = sl_cli_get_argument_uint16(arguments, 1);
  parameters.radioTxPower = sl_cli_get_argument_int8(arguments, 2);

  status = sl_zigbee_form_network(&parameters);

  sl_zigbee_app_debug_println("Form status=0x%08X", status);
}

void join_network_command(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  sl_zigbee_network_parameters_t parameters;
  memset( (void*)&parameters, 0, sizeof (parameters));
  parameters.joinMethod = SL_ZIGBEE_USE_MAC_ASSOCIATION;
  parameters.radioChannel = sl_cli_get_argument_uint8(arguments, 0);
  parameters.panId = sl_cli_get_argument_uint16(arguments, 1);
  parameters.radioTxPower = sl_cli_get_argument_int8(arguments, 2);

  status = sl_zigbee_join_network(SL_ZIGBEE_ROUTER, &parameters);

  sl_zigbee_app_debug_println("Join status=0x%08X", status);
}

void permit_join_command(sl_cli_command_arg_t *arguments)
{
  uint8_t duration = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = sl_zigbee_permit_joining(duration);

  sl_zigbee_app_debug_println("Permit join set 0x%08X", status);
}

void set_security_command(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_initial_security_state_t securityState;
  sl_status_t status = SL_STATUS_OK;
  uint8_t securityLevel;
  size_t paramLength;
  uint8_t *paramPtr;

  uint32_t securityBitmask = sl_cli_get_argument_uint32(arguments, 0);

  securityState.bitmask = (uint16_t)(securityBitmask & 0xFFFF);

  securityLevel = (((securityState.bitmask & EM_SECURITY_INITIALIZED)
                    && (securityState.bitmask != 0xFFFF))
                   ? 0
                   : 5);

  sl_zigbee_set_security_level(securityLevel);

  if (securityLevel > 0
      && securityState.bitmask != 0xFFFF) {
    // If the bit is set saying that a key is being passed, and the key buffer
    // is NOT empty, use the passed key.

    // If the bit is set saying that a key is being passed, and the key buffer
    // is empty (""), generate a random key.

    // If the bit is NOT set saying that a key is being passed, but the key
    // buffer is NOT empty, set the appropriate bit for that key and use that
    // key.

    paramPtr = sl_cli_get_argument_hex(arguments, 1, &paramLength);
    if (paramLength > 0) {
      if (paramLength != SL_ZIGBEE_ENCRYPTION_KEY_SIZE) {
        sl_zigbee_app_debug_println("wrong param");
        return;
      }
      memcpy(&securityState.preconfiguredKey, paramPtr, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
      securityState.bitmask |= SL_ZIGBEE_HAVE_PRECONFIGURED_KEY;
    } else if ((securityState.bitmask & SL_ZIGBEE_HAVE_PRECONFIGURED_KEY)
               == SL_ZIGBEE_HAVE_PRECONFIGURED_KEY) {
      sl_zigbee_generate_random_key(&securityState.preconfiguredKey);
    }

    paramPtr = sl_cli_get_argument_hex(arguments, 2, &paramLength);
    if (paramLength > 0) {
      if (paramLength != SL_ZIGBEE_ENCRYPTION_KEY_SIZE) {
        sl_zigbee_app_debug_println("wrong param");
        return;
      }
      memcpy(&securityState.networkKey, paramPtr, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
      securityState.bitmask |= SL_ZIGBEE_HAVE_NETWORK_KEY;
    } else if ((securityState.bitmask & SL_ZIGBEE_HAVE_NETWORK_KEY)
               == SL_ZIGBEE_HAVE_NETWORK_KEY) {
      sl_zigbee_generate_random_key(&securityState.networkKey);
    }

    securityState.networkKeySequenceNumber = sl_cli_get_argument_uint8(arguments, 3);
    if (securityState.bitmask & SL_ZIGBEE_HAVE_TRUST_CENTER_EUI64) {
      sl_zigbee_app_debug_println("not supported");
      return;
    }

    status = sl_zigbee_set_initial_security_state(&securityState);
    if (status == SL_STATUS_OK) {
      uint16_t extendedBitmask = (uint16_t) (securityBitmask >> 16);
      status = sl_zigbee_set_extended_security_bitmask(extendedBitmask);
    }
  }

  sl_zigbee_app_debug_println("Security set 0x%08X", status);
}

extern uint8_t joinDecision;
void set_trust_center_policy_command(sl_cli_command_arg_t *arguments)
{
  joinDecision = sl_cli_get_argument_uint8(arguments, 0);

  sl_zigbee_app_debug_println("Trust Center policy set 0x%02X", joinDecision);
}

static inline void print_key_data(const uint8_t *keyData)
{
  for (uint8_t i = 0; i < SL_ZIGBEE_ENCRYPTION_KEY_SIZE; i++) {
    sl_zigbee_app_debug_print("%02X", *keyData);
    keyData++;
  }
  sl_zigbee_app_debug_println("");
}

static inline void print_eui64(const uint8_t *eui64)
{
  sl_zigbee_app_debug_println("node [(>)%02X%02X%02X%02X%02X%02X%02X%02X]",
                              eui64[7],
                              eui64[6],
                              eui64[5],
                              eui64[4],
                              eui64[3],
                              eui64[2],
                              eui64[1],
                              eui64[0]);
}

void info_command(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_802154_long_addr_t myEui64;
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t networkParams;

  memmove(myEui64, sl_zigbee_get_eui64(), EUI64_SIZE);
  sl_zigbee_get_network_parameters(&nodeType, &networkParams);

  print_eui64(myEui64);
  sl_zigbee_app_debug_println("chan [%d] pwr [%d]",
                              networkParams.radioChannel,
                              networkParams.radioTxPower);
  sl_zigbee_app_debug_println("panID [0x%04X] nodeID [0x%04X]",
                              networkParams.panId,
                              sl_zigbee_get_node_id());
  sl_zigbee_app_debug_println("parentID [0x%04X] parentRssi [%d]",
                              sl_zigbee_get_parent_id(),
                              sl_zigbee_get_avg_parent_rssi());
  sl_zigbee_app_debug_println("nodeType [%d], securityLevel [%d], networkState [%02X]",
                              nodeType,
                              sl_zigbee_security_level(),
                              sl_zigbee_network_state());
}

void keys_print_command(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_network_key_info_t network_key_info;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;

  sl_zigbee_sec_man_init_context(&context);

  // Print the network key
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;
  (void) sl_zigbee_sec_man_export_key(&context, &plaintext_key);
  (void)sl_zigbee_sec_man_get_network_key_info(&network_key_info);

  sl_zigbee_app_debug_println("NWK Key out FC: %04x", network_key_info.network_key_frame_counter);
  sl_zigbee_app_debug_println("NWK Key seq num: 0x%02X", network_key_info.network_key_sequence_number);
  sl_zigbee_app_debug_print("NWK Key: ");
  print_key_data(plaintext_key.key);

  // Print the TC link key
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
  (void)sl_zigbee_sec_man_export_key(&context, &plaintext_key);
  (void)sl_zigbee_sec_man_get_aps_key_info(&context, &key_data);

  sl_zigbee_app_debug_println("TC Link Key out FC: %04x", key_data.outgoing_frame_counter);
  sl_zigbee_app_debug_print("TC Link Key: ");
  print_key_data(plaintext_key.key);
}
static sl_802154_short_addr_t destination;
void msg_buffer_command(sl_cli_command_arg_t *arguments)
{
  destination = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_af_event_set_active(&app_cli_event);
}

static void app_cli_event_handler(const sl_zigbee_af_event_t *event)
{
  (void)event;
  sl_status_t status;
  uint8_t data[1] = { 0x10 }; // 0x10 is also used by pro-compliance msg buffer test
  sl_zigbee_aps_frame_t apsFrame;

  apsFrame.sourceEndpoint = 1;
  apsFrame.destinationEndpoint = 1;
  apsFrame.clusterId = CLUSTER_BUFFER_TEST_REQUEST;
  apsFrame.profileId = TEST_PROFILE_ID;
  apsFrame.options = 0x0100; // default unicastOptions

  status = sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                  destination,
                                  &apsFrame,
                                  0x00, // tag
                                  1,
                                  data,
                                  NULL);

  sl_zigbee_app_debug_println("Msg status=0x%08X", status);
}

void set_profile_command(sl_cli_command_arg_t *arguments)
{
  uint8_t stackProfile = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_set_stack_profile(stackProfile);
}

void hal_reboot_command(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  halReboot();
}
