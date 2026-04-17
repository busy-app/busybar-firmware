/***************************************************************************//**
 * @file
 * @brief Commands for executing manufacturing related tests
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
#include "app/framework/util/af-main.h"
#include "stack/include/mfglib.h"
#include "app/framework/util/attribute-storage.h"
#include "manufacturing-library-cli-tokens.h"
#ifdef HAL_CONFIG
#include "hal-config.h"
#include "ember-hal-config.h"
#endif
#include "sl_zigbee_token.h"
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"
#include "sl_token_manager_api.h"

// -----------------------------------------------------------------------------
// Globals
extern sl_status_t mfglibStart(void (*mfglibRxCallback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi));

// The max packet size for 802.15.4 is 128, minus 1 byte for the length, and 2 bytes for the CRC.
#define MAX_BUFFER_SIZE 125

// the saved information for the first packet
static uint8_t savedPktLength = 0;
static int8_t savedRssi = 0;
static uint8_t savedLinkQuality = 0;
static uint8_t savedPkt[MAX_BUFFER_SIZE];

static uint16_t mfgCurrentPacketCounter = 0;

static bool inReceivedStream = false;

static bool mfgLibRunning = false;
static bool mfgToneTestRunning = false;
static bool mfgStreamTestRunning = false;

static uint16_t  mfgTotalPacketCounter = 0;

// Add 1 for the length byte which is at the start of the buffer.
ALIGNMENT(4) // efr32xg22 parts and later need 32-bit alignment
static uint8_t   sendBuff[MAX_BUFFER_SIZE + 1];

#define PLUGIN_NAME "Mfglib"

#define MIN_CLI_MESSAGE_SIZE 3
#define MAX_CLI_MESSAGE_SIZE 16

sl_zigbee_af_event_t sl_zigbee_af_manufacturing_library_cli_check_receive_complete_event;
#define checkReceiveCompleteEventControl (&sl_zigbee_af_manufacturing_library_cli_check_receive_complete_event)
void sl_zigbee_af_manufacturing_library_cli_check_receive_complete_event_handler(sl_zigbee_af_event_t * event);

static uint16_t savedPacketCount = 0;

#define CHECK_RECEIVE_COMPLETE_DELAY_QS 2

// -----------------------------------------------------------------------------
// Forward Declarations

// -----------------------------------------------------------------------------
// External APIs
// Function to determine whether the manufacturing library functionality is
// running.  This is used by the network manager and bulb ui plugins to
// determine if it is safe to kick off joining behavoir.
bool sl_zigbee_af_mfglib_running(void)
{
  return mfgLibRunning;
}

// Some joining behavoir kicks off before the device can receive a CLI command
// to start the manufacturing library.  Or in the case of devices that use
// UART for CLI access, they may be asleep.  In this case, we need to set a
// token that gives the manufacturing test a window of opportunity to enable
// the manufacturin library.  The idea is that fresh devices can more easily
// allow entry into the manufacturing test modes.  It is also intended to
// disable this token via CLI command at the end of manufacturing test so
// the end customer is not exposed to this functionality.
bool sl_zigbee_af_mfglib_enabled(void)
{
  uint8_t enabled;

#ifndef SL_ZIGBEE_TEST

  (void)sl_token_manager_get_data(COMMON_TOKEN_MFG_LIB_ENABLED, (void *)&enabled, sizeof(uint8_t));
#else
  return false;
#endif

  sl_zigbee_core_debug_print("MFG_LIB Enabled %02X\r\n", enabled);

  return enabled;
}

sl_status_t sl_zigbee_af_manufacturing_library_cli_token_init(void)
{
  uint8_t tokMfgLibEnabledDefault = TOKEN_MFG_LIB_ENABLED_DEFAULT;
  return sl_zigbee_initialize_basic_token(COMMON_TOKEN_MFG_LIB_ENABLED, &tokMfgLibEnabledDefault, sizeof(uint8_t));
}

// -----------------------------------------------------------------------------

void sli_zigbee_af_manufacturing_library_cli_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(checkReceiveCompleteEventControl,
                          sl_zigbee_af_manufacturing_library_cli_check_receive_complete_event_handler);

  assert(SL_STATUS_OK == sl_zigbee_af_manufacturing_library_cli_token_init());
}

// This is unfortunate but there is no callback indicating when sending is complete
// for all packets.  So we must create a timer that checks whether the packet count
// has increased within the last second.

void sl_zigbee_af_manufacturing_library_cli_check_receive_complete_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(checkReceiveCompleteEventControl);
  if (!inReceivedStream) {
    return;
  }

  if (savedPacketCount == mfgTotalPacketCounter) {
    inReceivedStream = false;
    sl_zigbee_af_core_println("%s Receive Complete %d packets",
                              PLUGIN_NAME,
                              mfgCurrentPacketCounter);
    sl_zigbee_af_core_println("First packet: lqi %d, rssi %d, len %d",
                              savedLinkQuality,
                              savedRssi,
                              savedPktLength);
    mfgCurrentPacketCounter = 0;
  } else {
    savedPacketCount = mfgTotalPacketCounter;
    sl_zigbee_af_event_set_delay_qs(checkReceiveCompleteEventControl,
                                    CHECK_RECEIVE_COMPLETE_DELAY_QS);
  }
}

static void fillBuffer(uint8_t* buff, uint8_t length, bool random)
{
  uint8_t i;
  // length byte does not include itself. If the user asks for 10
  // bytes of packet this means 1 byte length, 7 bytes, and 2 bytes CRC
  // this example will have a length byte of 9, but 10 bytes will show
  // up on the receive side
  buff[0] = length;

  for (i = 1; i < length; i += 2) {
    // Two buffer elements per iteration to use both random bytes.
    if (random) {
      uint16_t randomNumber = sl_zigbee_get_pseudo_random_number();
      buff[i] = (uint8_t)(randomNumber & 0xFF);
      buff[i + 1] = (uint8_t)((randomNumber >> 8)) & 0xFF;
    } else {
      // Test pattern is ascending integers starting from 1.
      buff[i] = i;
      buff[i + 1] = i + 1;
    }
  }
}

static void mfglibRxHandler(uint8_t *packet,
                            uint8_t linkQuality,
                            int8_t rssi)
{
  // This increments the total packets for the whole mfglib session
  // this starts when mfglibStart is called and stops when mfglibEnd
  // is called.
  mfgTotalPacketCounter++;

  mfgCurrentPacketCounter++;

  // If this is the first packet of a transmit group then save the information
  // of the current packet. Don't do this for every packet, just the first one.
  if (!inReceivedStream) {
    inReceivedStream = true;
    mfgCurrentPacketCounter = 1;
    savedRssi = rssi;
    savedLinkQuality = linkQuality;
    savedPktLength = *packet;
    memmove(savedPkt, (packet + 1), savedPktLength);
    sl_zigbee_af_event_set_active(checkReceiveCompleteEventControl);
  }
}

void sl_zigbee_af_mfglib_rx_statistics(uint16_t* packetsReceived,
                                       int8_t* savedRssiReturn,
                                       uint8_t* savedLqiReturn)
{
  *packetsReceived = mfgTotalPacketCounter;
  *savedRssiReturn = savedRssi;
  *savedLqiReturn = savedLinkQuality;
}

void sl_zigbee_af_mfglib_start(bool wantCallback)
{
  sl_status_t status = mfglibStart(wantCallback ? mfglibRxHandler : NULL);
  sl_zigbee_af_core_println("%s start, status 0x%02X",
                            PLUGIN_NAME,
                            status);
  if (status == SL_STATUS_OK) {
    mfgLibRunning = true;
    mfgTotalPacketCounter = 0;
  }
}

void sl_zigbee_af_mfglib_stop(void)
{
  sl_status_t status = mfglibEnd();
  sl_zigbee_af_core_println("%s end, status 0x%02X",
                            PLUGIN_NAME,
                            status);
  sl_zigbee_af_core_println("rx %d packets while in mfg mode", mfgTotalPacketCounter);
  if (status == SL_STATUS_OK) {
    mfgLibRunning = false;
  }
}

#include "app/util/serial/sl_zigbee_command_interpreter.h"

void sli_zigbee_af_mfglib_start_command(sl_cli_command_arg_t *arguments)
{
  bool wantCallback = sl_cli_get_argument_uint8(arguments, 0);

  sl_zigbee_af_mfglib_start(wantCallback);
}

void sli_zigbee_af_mfglib_stop_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_af_mfglib_stop();
}

void sli_zigbee_af_mfglib_tone_start_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status = mfglibStartTone();
  sl_zigbee_af_core_println("%s start tone 0x%02X", PLUGIN_NAME, status);
  if (status == SL_STATUS_OK) {
    mfgToneTestRunning = true;
  }
}

void sli_zigbee_af_mfglib_tone_stop_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status = mfglibStopTone();
  sl_zigbee_af_core_println("%s stop tone 0x%02X", PLUGIN_NAME, status);
  if (status == SL_STATUS_OK) {
    mfgToneTestRunning = false;
  }
}

void sli_zigbee_af_mfglib_stream_start_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status = mfglibStartStream();
  sl_zigbee_af_core_println("%s start stream 0x%02X", PLUGIN_NAME, status);
  if (status == SL_STATUS_OK) {
    mfgStreamTestRunning = true;
  }
}

void sli_zigbee_af_mfglib_stream_stop_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_status_t status = mfglibStopStream();
  sl_zigbee_af_core_println("%s stop stream 0x%02X", PLUGIN_NAME, status);
  if (status == SL_STATUS_OK) {
    mfgStreamTestRunning = false;
  }
}

void sli_zigbee_af_mfglib_send_command(sl_cli_command_arg_t *arguments)
{
  bool random = memcmp(sl_cli_get_command_string(arguments, 3 /*arguments->arg_ofs - 1 */),
                       "random", strlen("random")) == 0;
  uint16_t numPackets = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t length = sl_cli_get_argument_uint8(arguments, 1);

  if (length > MAX_BUFFER_SIZE) {
    sl_zigbee_af_core_println("Error: Length cannot be bigger than %d", MAX_BUFFER_SIZE);
    return;
  }

  if (numPackets == 0) {
    sl_zigbee_af_core_println("Error: Number of packets cannot be 0.");
    return;
  }

  fillBuffer(sendBuff, length, random);

  // The second parameter to the mfglibSendPacket() is the
  // number of "repeats", therefore we decrement numPackets by 1.
  numPackets--;
  sl_status_t status = mfglibSendPacket(sendBuff, numPackets);
  sl_zigbee_af_core_println("%s send packet, status 0x%02X", PLUGIN_NAME, status);
}

void sli_zigbee_af_mfglib_send_message_command(sl_cli_command_arg_t *arguments)
{
  size_t length = 0;
  uint8_t *message = sl_cli_get_argument_hex(arguments, 0, &length);
  uint16_t numPackets = sl_cli_get_argument_uint16(arguments, 1);

  if (length < MIN_CLI_MESSAGE_SIZE) {
    sl_zigbee_af_core_println("Error: Minimum length is %d bytes.", MIN_CLI_MESSAGE_SIZE);
    return;
  }

  if (length > MAX_CLI_MESSAGE_SIZE) {
    sl_zigbee_af_core_println("Error: Maximum length is %d bytes.", MAX_CLI_MESSAGE_SIZE);
    return;
  }

  if (numPackets == 0) {
    sl_zigbee_af_core_println("Error: Number of packets cannot be 0.");
    return;
  }

  sendBuff[0] = length + 2; // message length plus 2-byte CRC
  memmove(sendBuff + 1, message, length);
  numPackets--;
  sl_status_t status = mfglibSendPacket(sendBuff, numPackets);
  sl_zigbee_af_core_println("%s send message, status 0x%02X", PLUGIN_NAME, status);
}

void sli_zigbee_af_mfglib_status_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t channel = mfglibGetChannel();
  int8_t power = mfglibGetPower();
  uint16_t powerMode = sl_802154_get_tx_power_mode();
  uint8_t options = mfglibGetOptions();
  sl_zigbee_af_core_println("Channel: %d", channel);
  sl_zigbee_af_core_println("Power: %d", power);
  sl_zigbee_af_core_println("Power Mode: 0x%04X", powerMode);
  sl_zigbee_af_core_println("Options: 0x%02X", options);
  sl_zigbee_af_core_println("%s running: %s", PLUGIN_NAME, (mfgLibRunning ? "yes" : "no"));
  sl_zigbee_af_core_println("%s tone test running: %s", PLUGIN_NAME, (mfgToneTestRunning ? "yes" : "no"));
  sl_zigbee_af_core_println("%s stream test running: %s", PLUGIN_NAME, (mfgStreamTestRunning ? "yes" : "no"));
  sl_zigbee_af_core_println("Total %s packets received: %d", PLUGIN_NAME, mfgTotalPacketCounter);
}

void sli_zigbee_af_mfglib_set_channel_command(sl_cli_command_arg_t *arguments)
{
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = mfglibSetChannel(channel);
  sl_zigbee_af_core_println("%s set channel, status 0x%02X", PLUGIN_NAME, status);
}

void sli_zigbee_af_mfglib_set_power_and_mode_command(sl_cli_command_arg_t *arguments)
{
  int8_t power = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t mode = sl_cli_get_argument_uint16(arguments, 1);
  sl_status_t status = mfglibSetPower(mode, power);
  sl_zigbee_af_core_println("%s set power and mode, status 0x%02X", PLUGIN_NAME, status);
}

void sli_zigbee_af_mfglib_sleep_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  // In UC, Zigbee sits on top of the Legacy HAL component instead of the old
  // Ember HAL (previously located in platform/base). Whereas the Ember HAL
  // would allow applications to essentially force sleep, the Power Manager
  // component does not. We could try requesting it here (by calling
  // sl_power_manager_sleep), but there's no guarantee it would succeed (in
  // fact, it will likely fail under many circumstances). Furthermore, even
  // whether it succeeded requires additional code and increases complexity,
  // leaving aside how to handle the case where it fails. The best course of
  // action is to deprecate this command. I am leaving in place with an error
  // message for now to notify anyone who used it in the past.
  sl_zigbee_af_core_println("Error: %s no longer supports forced sleep", PLUGIN_NAME);
}

// Function to program a custom EUI64 into the chip.
// Example:
// plugin mfglib programEui { 01 02 03 04 05 06 07 08 }
// Note:  this command is OTP.  It only works once.  To re-run, you
// must erase the chip.
void sli_zigbee_af_mfglib_program_eui_command(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t eui64;

  sl_zigbee_copy_eui64_arg(arguments, 0, eui64, true);

  // potentially verify first few bytes for customer OUI

#ifndef SL_ZIGBEE_TEST
  // OK, we verified the customer OUI.  Let's program it here.
  (void)sl_token_manager_set_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_CUSTOM_EUI_64),
                                  (void *)&eui64,
                                  sizeof(sl_802154_long_addr_t));
#endif
}

void sli_zigbee_af_mfglib_enable_mfglib(sl_cli_command_arg_t *arguments)
{
#ifndef SL_ZIGBEE_TEST
  uint8_t enabled = sl_cli_get_argument_uint8(arguments, 0);

  (void)sl_token_manager_set_data(COMMON_TOKEN_MFG_LIB_ENABLED, (void *)&enabled, sizeof(uint8_t));
#else
  UNUSED_VAR(arguments);
#endif
}

void sli_zigbee_af_mfglib_set_options(sl_cli_command_arg_t *arguments)
{
  uint8_t options = sl_cli_get_argument_uint8(arguments, 0);
  sl_status_t status = mfglibSetOptions(options);
  sl_zigbee_af_core_println("%s set options, status 0x%02X", PLUGIN_NAME, status);
}

void sli_zigbee_af_mfglib_set_ctune_command(sl_cli_command_arg_t *arguments)
{
  uint16_t ctune = sl_cli_get_argument_uint16(arguments, 0);
  sl_status_t status = mfglibSetCtune(ctune);
  sl_zigbee_af_core_println("%s set ctune, status 0x%02X", PLUGIN_NAME, status);
}

void sli_zigbee_af_mfglib_get_ctune_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint16_t ctune = mfglibGetCtune();
  sl_zigbee_af_core_println("%s get ctune value: %d", PLUGIN_NAME, ctune);
}
