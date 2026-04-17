/***************************************************************************//**
 * @file
 * @brief common code for apps.
 *
 * The common library is deprecated and will be removed in a future release.
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

#include PLATFORM_HEADER

#ifdef EZSP_HOST
// Includes needed for ember related functions for the EZSP host
  #include "stack/include/sl_zigbee_types.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/ezsp-utils.h"
  #include "app/util/ezsp/serial-interface.h"
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#else
// Includes needed for ember related functions for the EM250
  #include "stack/include/sl_zigbee.h"
#endif // EZSP_HOST

#include "hal/hal.h"
#include "serial/serial.h"
#ifdef SL_CATALOG_ZIGBEE_CLI_PRESENT
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#endif // SL_CATALOG_ZIGBEE_CLI_PRESENT
#include "app/util/common/common.h"
#include "event_queue/event-queue.h"

#include "stack/mac/multi-mac.h"  // for MAC_DUAL_PRESENT
#ifdef MAC_DUAL_PRESENT
#include "stack/core/sl_zigbee_multi_phy.h"
#endif

#ifdef MICRIUM

#ifdef EZSP_HOST
#error "EZSP_HOST and MICRIUM both defined"
#endif // EZSP_HOST

#include <os.h>

#endif // MICRIUM

//----------------------------------------------------------------
// Boilerplate

static sl_zigbee_network_parameters_t joinParameters;
static uint8_t joinNodeType;
static sl_802154_long_addr_t preconfiguredTrustCenterEui64;

#if defined(EZSP_HOST)
bool sl_zigbee_stack_is_up(void);
sl_status_t sl_zigbee_generate_random_key(sl_zigbee_key_data_t* result);
#endif

sli_zigbee_event_t blinkEvent;
sli_zigbee_event_queue_t event_queue;

#ifdef ZIGBEE_PRO_COMPLIANCE_ON_HOST
// If we run pro-compliance on host, it has a definition conflict
// with the serialPort defined in app/projects/zigbeed/serial_adapter.c
uint8_t sl_zigbee_serial_port = 1;
#define serialPort sl_zigbee_serial_port
#else
// Set Default baud on port 1 differently in sim vs hardware
// to get compliance app out for now.
uint8_t serialPort = 1;
#endif // ZIGBEE_PRO_COMPLIANCE_ON_HOST
#ifdef SL_ZIGBEE_TEST
SerialBaudRate serialBaudRate = BAUD_19200;
#else
SerialBaudRate serialBaudRate = BAUD_115200;
#endif

void configureSerial(uint8_t port, SerialBaudRate rate)
{
  serialPort = port;
  serialBaudRate = rate;
}

void initialize(void)
{
  initialize_sl_zigbee_stack_t();
  memset(joinParameters.extendedPanId, 0, 8);
  sli_zigbee_initialize_event_queue(&event_queue);
  blinkEvent.actions.queue = &event_queue;
  sli_zigbee_event_set_delay_ms(&blinkEvent, 500);       // every half second
}

void run(sl_zigbee_event_data_t* events,
         void (*heartbeat)(void))
{
#ifdef MICRIUM
  while (DEF_ON) {
#else
  while (true) {
#endif
    halResetWatchdog();
    sl_zigbee_tick();
    if (heartbeat != NULL) {
      (heartbeat)();
    }
    if (events != NULL) {
      runEvents(&event_queue);
    }
  }
}

//----------------------------------------------------------------
// Common commands

void helpCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sl_zigbee_core_debug_println("COMMAND [PARAMETERS] [- DESCRIPTION]\r\n"
                               "  where: b=string, s=int8_t, u=uint8_t, v=uint16_t, w=uint32_t, n=nested");
  (void) sli_legacy_serial_wait_send(serialPort);
  // EMZIGBEE-6745
}

void statusCommandZCP(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sl_zigbee_core_debug_print("%s ", applicationString);
  printLittleEndianEui64(serialPort, sl_zigbee_get_eui64());
  (void) sli_legacy_serial_wait_send(serialPort);
  sl_zigbee_core_debug_print(" (%02x)", sl_zigbee_get_node_id());
  if ( sl_zigbee_stack_is_up() ) {
    sl_zigbee_node_type_t nodeType;
    sl_zigbee_network_parameters_t networkParams;

    if (!getNetworkParameters(&nodeType, &networkParams)) {
      printErrorMessage("Failed to get Network parameters!");
      return;
    }

    sl_zigbee_core_debug_print(" Ember channel %d",
                               networkParams.radioChannel);
    sl_zigbee_core_debug_print(" power %d PAN ID %04X XPID ",
                               networkParams.radioTxPower,
                               networkParams.panId);
    printLittleEndianEui64(serialPort, networkParams.extendedPanId);
    sl_zigbee_core_debug_println("");
#ifdef MAC_DUAL_PRESENT
    uint8_t phyInterfaceCount = sl_zigbee_get_phy_interface_count();
    sl_zigbee_core_debug_println("Additional Interfaces:");
    if ((phyInterfaceCount > 1) && (255 != phyInterfaceCount)) {
      sl_zigbee_multi_phy_radio_parameters_t multiPhyRadioParams;
      uint8_t i;

      for (i = 1; i < phyInterfaceCount; ++i) {
        sl_status_t status = sl_zigbee_get_radio_parameters(i, &multiPhyRadioParams);
        if (status == SL_STATUS_OK ) {
          sl_zigbee_core_debug_println(" %d. Page %d Channel %d Power %d ", i,
                                       multiPhyRadioParams.radioPage,
                                       multiPhyRadioParams.radioChannel,
                                       multiPhyRadioParams.radioTxPower);
        } else {
          sl_zigbee_core_debug_println(" %d. Offline, status 0x%02X",
                                       i, status);
        }
      }
    } else {
      sl_zigbee_core_debug_println(" None");
    }
#endif
  } else {
    sl_zigbee_core_debug_println(" offline");
  }
}

void getRadioParameters(SL_CLI_COMMAND_ARG)
{
  uint8_t status;
  uint8_t phyIndex = 0;

  sl_zigbee_multi_phy_radio_parameters_t params;

  if (sl_cli_get_argument_count(arguments) > 0) {
    phyIndex = sl_cli_get_argument_uint8(arguments, 0);
  }

  status = sl_zigbee_get_radio_parameters(phyIndex, &params);

  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("Power %d Page %d Channel %d", params.radioTxPower,
                                 params.radioPage, params.radioChannel);
  } else {
    sl_zigbee_core_debug_println("failed to get radio parameters error 0x%0x", status);
  }
}

void stateCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  uint8_t childCount;
  uint8_t routerCount;
  sl_zigbee_node_type_t myNodeType;
  sl_802154_short_addr_t parentId;
  sl_802154_long_addr_t parentEui;
  sl_zigbee_network_parameters_t params;
  uint8_t stackProfile;
  sl_802154_short_addr_t myNodeId;

  if ( sl_zigbee_stack_is_up()
       && (SL_STATUS_OK
           == getOnlineNodeParameters(&childCount,
                                      &routerCount,
                                      &myNodeType,
                                      &parentId,
                                      // no '&' operand necessary here
                                      parentEui,
                                      &params)) ) {
    sl_zigbee_core_debug_print("NWK(%d) %s id=0x%04X parent=0x%04X ",
                               sl_zigbee_get_current_network(),
                               (myNodeType >= SL_ZIGBEE_END_DEVICE
                                ? "End device"
                                : "Router"),
                               sl_zigbee_get_node_id(),
                               parentId);
    printLittleEndianEui64(serialPort, parentEui);
    sl_zigbee_core_debug_println(" channel=%d pan=0x%04X end devices=%d routers=%d",
                                 sl_zigbee_get_radio_channel(),
                                 params.panId,
                                 childCount,
                                 routerCount);
    sl_zigbee_core_debug_println("nwkManager=0x%04X nwkUpdateId=%d channelMask=0x%08lX",
                                 params.nwkManagerId,
                                 params.nwkUpdateId,
                                 (unsigned long)params.channels);
  } else if ( SL_STATUS_OK == getOfflineNodeParameters(&myNodeId,
                                                       &myNodeType,
                                                       &stackProfile) ) {
    if (sl_zigbee_network_state() == SL_ZIGBEE_JOINING_NETWORK) {
      sl_zigbee_core_debug_print("Joining.   Data:");
      myNodeId = sl_zigbee_get_node_id();
    } else {
      sl_zigbee_core_debug_print("Not joined.  Tokens:");
    }
    sl_zigbee_core_debug_println(" id=0x%04X type=%d stackProfile=%d",
                                 myNodeId,
                                 myNodeType,
                                 stackProfile);
  } else {
    sl_zigbee_core_debug_println("Not joined.   Token fetch error.");
  }
}

void rebootCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  halReboot();
}

//----------------------------------------------------------------
// Utilities

// The initial < or > is meant to indicate the endian-ness of the EUI64
// '>' is big endian (most significant first)
// '<' is little endian (least significant first)

void printLittleEndianEui64(uint8_t port, sl_802154_long_addr_t eui64)
{
  UNUSED_VAR(port);
  sl_zigbee_core_debug_print("(<)%02X%02X%02X%02X%02X%02X%02X%02X",
                             eui64[0], eui64[1], eui64[2], eui64[3],
                             eui64[4], eui64[5], eui64[6], eui64[7]);
}

void printBigEndianEui64(uint8_t port, sl_802154_long_addr_t eui64)
{
  UNUSED_VAR(port);
  sl_zigbee_core_debug_print("(>)%02X%02X%02X%02X%02X%02X%02X%02X",
                             eui64[7], eui64[6], eui64[5], eui64[4],
                             eui64[3], eui64[2], eui64[1], eui64[0]);
}

void printHexByteArray(uint8_t port, uint8_t *byteArray, uint8_t length)
{
  UNUSED_VAR(port);
  uint8_t index = 0;

  while ((index + 4) <= length) {
    sl_zigbee_core_debug_print(" %02X %02X %02X %02X",
                               byteArray[index], byteArray[index + 1],
                               byteArray[index + 2], byteArray[index + 3]);
    index += 4;
  }

  while (index < length) {
    sl_zigbee_core_debug_print(" %0X", byteArray[index]);
    index += 1;
  }
}

uint8_t asciiHexToByteArray(uint8_t *bytesOut, uint8_t* asciiIn, uint8_t asciiCharLength)
{
  uint8_t destIndex = 0;
  uint8_t srcIndex = 0;

  // We need two characters of input for each byte of output.
  while (srcIndex < (asciiCharLength - 1)) {
    bytesOut[destIndex]  = ((((hexDigitValue(asciiIn[srcIndex])) & 0x0F) << 4)
                            + ((hexDigitValue(asciiIn[srcIndex + 1])) & 0x0F));
    destIndex += 1;
    srcIndex  += 2;
  }

  return destIndex;
}

uint8_t hexDigitValue(uint8_t digit)
{
  if ('0' <= digit && digit <= '9') {
    return digit - '0';
  } else if ('A' <= digit && digit <= 'F') {
    return digit - 'A' + 10;
  } else if ('a' <= digit && digit <= 'f') {
    return digit - 'a' + 10;
  } else {
    return 0;
  }
}

void createMulticastBinding(uint8_t index, uint8_t *multicastGroup, uint8_t endpoint)
{
  sl_zigbee_binding_table_entry_t entry;

  entry.type = SL_ZIGBEE_MULTICAST_BINDING;
  memmove(entry.identifier, multicastGroup, 8);
  entry.local = endpoint;

  assert(sl_zigbee_set_binding(index, &entry) == SL_STATUS_OK);
}

bool findEui64Binding(sl_802154_long_addr_t key, uint8_t *index)
{
  uint8_t i;
  uint8_t unused = 0xFF;

  for (i = 0; i < sl_zigbee_get_binding_table_size(); i++) {
    sl_zigbee_binding_table_entry_t binding;
    if (sl_zigbee_get_binding(i, &binding) == SL_STATUS_OK) {
      if (binding.type == SL_ZIGBEE_UNICAST_BINDING
          && memcmp(key, binding.identifier, 8) == 0) {
        *index = i;
        return true;
      } else if (binding.type == SL_ZIGBEE_UNUSED_BINDING
                 && unused == 0xFF) {
        unused = i;
      }
    }
  }

  *index = unused;
  return false;
}

void printCommandStatus(sl_status_t status,
                        const char * good,
                        const char * bad)
{
  if (status == SL_STATUS_OK) {
    if ( good != NULL ) {
      sl_zigbee_core_debug_println("%s", good);
    }
  } else {
    sl_zigbee_core_debug_println("%s, status:0x%02X",
                                 bad, status);
  }
}

void printCommandStatusWithPrefix(sl_status_t status,
                                  const char * prefix,
                                  const char * good,
                                  const char * bad)
{
  if (status == SL_STATUS_OK) {
    if ( good != NULL ) {
      sl_zigbee_core_debug_println("%s %s",
                                   prefix,
                                   good);
    }
  } else {
    sl_zigbee_core_debug_println("%s %s, status:0x%02X",
                                 prefix,
                                 bad,
                                 status);
  }
}

void printOperationStatus(sl_status_t status,
                          const char * operation)
{
  sl_zigbee_core_debug_print("%s", operation);
  printCommandStatus(status, "", " failed");
}

//----------------------------------------------------------------
// Common zigbee commands

uint16_t lastJoinTime;

void setSecurityCommand(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_initial_security_state_t securityState;
  sl_status_t status = SL_STATUS_OK;
  uint8_t securityLevel;

  uint32_t securityBitmask = sl_cli_get_argument_uint32(arguments, 0);

  securityState.bitmask = (uint16_t) (securityBitmask & 0xFFFF);

  securityLevel = (((securityState.bitmask & EM_SECURITY_INITIALIZED)
                    && (securityState.bitmask != 0xFFFF))
                   ? 0
                   : 5);

  status = (setSecurityLevel(securityLevel)
            ? SL_STATUS_OK
            : SL_STATUS_FAIL);

  if ( status == SL_STATUS_OK
       && securityLevel > 0
       && securityState.bitmask != 0xFFFF) {
    // If the bit is set saying that a key is being passed, and the key buffer
    // is NOT empty, use the passed key.

    // If the bit is set saying that a key is being passed, and the key buffer
    // is empty (""), generate a random key.

    // If the bit is NOT set saying that a key is being passed, but the key
    // buffer is NOT empty, set the appropriate bit for that key and use that
    // key.
    if (sl_zigbee_copy_hex_arg(arguments, 1, sl_zigbee_key_contents(&securityState.preconfiguredKey), SL_ZIGBEE_ENCRYPTION_KEY_SIZE, true)) {
      securityState.bitmask |= SL_ZIGBEE_HAVE_PRECONFIGURED_KEY;
    } else if ((securityState.bitmask & SL_ZIGBEE_HAVE_PRECONFIGURED_KEY)
               == SL_ZIGBEE_HAVE_PRECONFIGURED_KEY) {
      sl_zigbee_generate_random_key(&securityState.preconfiguredKey);
    }

    if (sl_zigbee_copy_hex_arg(arguments, 2, sl_zigbee_key_contents(&securityState.networkKey), SL_ZIGBEE_ENCRYPTION_KEY_SIZE, true)) {
      securityState.bitmask |= SL_ZIGBEE_HAVE_NETWORK_KEY;
    } else if ((securityState.bitmask & SL_ZIGBEE_HAVE_NETWORK_KEY)
               == SL_ZIGBEE_HAVE_NETWORK_KEY) {
      sl_zigbee_generate_random_key(&securityState.networkKey);
    }

    securityState.networkKeySequenceNumber = sl_cli_get_argument_uint8(arguments, 3);
    if (securityState.bitmask & SL_ZIGBEE_HAVE_TRUST_CENTER_EUI64) {
      memmove(securityState.preconfiguredTrustCenterEui64,
              preconfiguredTrustCenterEui64,
              EUI64_SIZE);
    }

    status = sl_zigbee_set_initial_security_state(&securityState);
    if (status == SL_STATUS_OK) {
      uint16_t extendedBitmask = (uint16_t) (securityBitmask >> 16);
      status = sl_zigbee_set_extended_security_bitmask(extendedBitmask);
    }
  }
  printOperationStatus(status,
                       "Security set");
}

void formNetworkCommand(SL_CLI_COMMAND_ARG)
{
  sl_status_t status;
  sl_zigbee_network_parameters_t parameters;
  uint8_t commandLength;

  parameters.radioChannel = sl_cli_get_argument_uint8(arguments, 0);
  parameters.panId = sl_cli_get_argument_uint16(arguments, 1);
  parameters.radioTxPower = sl_cli_get_argument_uint8(arguments, 2);
  sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &commandLength);
  if (commandLength == 4) {
    memset(parameters.extendedPanId, 0, 8);
  } else {
    sl_zigbee_copy_eui64_arg(arguments, 3, parameters.extendedPanId, false);
  }
  lastJoinTime = halCommonGetInt16uMillisecondTick();
  status = sl_zigbee_form_network(&parameters);
  printCommandStatus(status, "Formed", "Form failed");
}

#ifdef MAC_DUAL_PRESENT
void multiPhyStartCommand(SL_CLI_COMMAND_ARG)
{
  sl_status_t status;
  /** A power setting, in dBm.*/
  int8_t   radioTxPower;
  uint8_t radioPage;
  /** A radio channel. Be sure to specify a channel supported by the radio. */
  uint8_t   radioChannel;
  uint8_t optionsMask = 0;

  radioPage = sl_cli_get_argument_int8(arguments, 0);
  radioChannel = sl_cli_get_argument_uint8(arguments, 1);
  radioTxPower = sl_cli_get_argument_int8(arguments, 2);
  if (sl_cli_get_argument_count(arguments) > 3) {
    optionsMask = sl_cli_get_argument_uint8(arguments, 3);
  }
  status = sl_zigbee_multi_phy_start(PHY_INDEX_PRO2PLUS, radioPage,
                                     radioChannel, radioTxPower, optionsMask);
  printCommandStatus(status, "Started", "Start failed");
}

void multiPhyStopCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t phyIndex = PHY_INDEX_PRO2PLUS;

  if (sl_cli_get_argument_count(arguments) > 0) {
    phyIndex = sl_cli_get_argument_uint8(arguments, 0);
  }

  sl_status_t status = sl_zigbee_multi_phy_stop(phyIndex);
  printCommandStatus(status, "Stopped", "Start failed");
}
#endif

sl_status_t joinNetwork(void)
{
  lastJoinTime = halCommonGetInt16uMillisecondTick();
  return sl_zigbee_join_network(joinNodeType, &joinParameters);
}

void setExtPanIdCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sl_zigbee_copy_eui64_arg(arguments, 0, joinParameters.extendedPanId, false);
}

void setJoinMethod(SL_CLI_COMMAND_ARG)
{
  joinParameters.joinMethod = sl_cli_get_argument_uint8(arguments, 0);
}

void setCommissionParameters(SL_CLI_COMMAND_ARG)
{
  joinNodeType = (sl_cli_get_argument_uint8(arguments, 0) > 0
                  ? SL_ZIGBEE_COORDINATOR
                  : SL_ZIGBEE_ROUTER);
  joinParameters.nwkManagerId = sl_cli_get_argument_uint16(arguments, 1);
  joinParameters.nwkUpdateId  = sl_cli_get_argument_uint8(arguments, 2);
  joinParameters.channels     = sl_cli_get_argument_uint32(arguments, 3);
  joinParameters.channels    |= (sl_cli_get_argument_uint32(arguments, 4) << 16);
  sl_zigbee_copy_eui64_arg(arguments, 5, preconfiguredTrustCenterEui64, false);
}

void joinNetworkCommand(SL_CLI_COMMAND_ARG)
{
  sl_status_t status;
  uint8_t commandLength;
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &commandLength);;
  joinParameters.radioChannel = sl_cli_get_argument_uint8(arguments, 0);
  joinParameters.panId = sl_cli_get_argument_uint16(arguments, 1);
  joinParameters.radioTxPower = sl_cli_get_argument_int8(arguments, 2);
  if (joinParameters.joinMethod != SL_ZIGBEE_USE_CONFIGURED_NWK_STATE) {
    joinNodeType = (commandLength == 4 ? SL_ZIGBEE_ROUTER
                    : command[5] == 'e' ? SL_ZIGBEE_END_DEVICE
                    : SL_ZIGBEE_SLEEPY_END_DEVICE);
  }
  status = joinNetwork();
  if ( !(status == SL_STATUS_OK
         && joinParameters.joinMethod == SL_ZIGBEE_USE_CONFIGURED_NWK_STATE)) {
    printOperationStatus(status,
                         "Joining");
  }
}

void networkInitCommandZCP(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_network_init_struct_t nwkInitStruct;
  sl_status_t status;

  lastJoinTime = halCommonGetInt16uMillisecondTick();
  nwkInitStruct.bitmask = sl_cli_get_argument_uint16(arguments, 0);
  status = sl_zigbee_network_init(&nwkInitStruct);
  printOperationStatus(status, "Re-initializing network");
}

void rejoinCommand(SL_CLI_COMMAND_ARG)
{
  bool secure = (bool)sl_cli_get_argument_uint32(arguments, 0);
  uint32_t channelMask = sl_cli_get_argument_uint32(arguments, 1);
  printOperationStatus(sl_zigbee_find_and_rejoin_network(secure, channelMask,
                                                         SL_ZIGBEE_REJOIN_DUE_TO_APP_EVENT_1,
                                                         SL_ZIGBEE_UNKNOWN_DEVICE),
                       "Rejoining");
}

void leaveNetworkCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sl_status_t status = sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
  printCommandStatus(status, "Left", "Leave failed");
}

//The function above is widely used and rather cumbersome to change.
void leaveWithOptionsNetworkCommand(SL_CLI_COMMAND_ARG)
{
  #ifndef EZSP_HOST
  uint8_t options = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = sl_zigbee_leave_network(options);
  printCommandStatus(status, "Left", "Leave failed");
  #endif
}

void addressRequestCommand(SL_CLI_COMMAND_ARG)
{
  bool reportKids = (bool)sl_cli_get_argument_uint32(arguments, 1);
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, NULL);
  if (command[0] == 'n') {
    sl_802154_long_addr_t targetEui64;
    sl_zigbee_copy_eui64_arg(arguments, 0, targetEui64, false);
    sl_zigbee_network_address_request(targetEui64, reportKids, 0);
  } else {
    sl_802154_short_addr_t target = sl_cli_get_argument_uint16(arguments, 0);
    sl_zigbee_ieee_address_request(target, reportKids, 0, 0);
  }
}

void permitJoiningCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t duration = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_core_debug_println("permitJoining(%d) -> 0x%02X\n",
                               duration,
                               sl_zigbee_permit_joining(duration));
}

void setNetworkCommand(SL_CLI_COMMAND_ARG)
{
  UNUSED uint8_t nwkIndex = sl_cli_get_argument_uint8(arguments, 0);
  (void) sl_zigbee_set_current_network(nwkIndex);
}

void getCurrentNwkIndexCommand(SL_CLI_COMMAND_ARG)
{
  #ifdef SL_CATALOG_CLI_PRESENT
  (void)arguments;
  #endif // SL_CATALOG_CLI_PRESENT
  sl_zigbee_core_debug_println("Current network index %d",
                               sl_zigbee_get_current_network());
}

// Routine to save some const/flash space
void printCarriageReturn(void)
{
  sl_zigbee_core_debug_print("\r\n");
}

void printErrorMessage(const char * message)
{
  sl_zigbee_core_debug_println("Error: %s", message);
}

void printErrorMessage2(const char * message, const char * message2)
{
  sl_zigbee_core_debug_println("Error: %s %s", message, message2);
}
