/***************************************************************************//**
 * @file
 * @brief This file implements informational commands for sl_rail_test applications.
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

#include <string.h>
#include "response_print.h"

#include "sl_rail.h"
#include "sl_rail_ieee802154.h"
#include "app_common.h"
#include "sl_rail_util_rssi.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#if (defined(SL_CATALOG_NVM3_DEFAULT_PRESENT) \
  && !defined(SL_TRUSTZONE_SECURE))
// RAIL_LIB-14627: nvm3_default.h is not added to the project by the nvm3_default
// component when trustzone_secure component is used.
#include "nvm3_default.h"
#endif

uint16_t getLikelyChannel(void)
{
  uint16_t chan = channel; // Default to last 'offline' setChannel setting
  if ((sl_rail_get_radio_state(railHandle)
       & (SL_RAIL_RF_STATE_RX | SL_RAIL_RF_STATE_TX)) != 0U) {
    (void) sl_rail_get_channel(railHandle, &chan); // capture online channel
    // Note: this may not reflect a channel-hopping channel
  }
  return chan;
}

static void printBitField(uint32_t bitField,
                          const char * const *fieldNames,
                          uint8_t numFields)
{
  uint8_t i;
  for (i = 0; i < numFields; ++i) {
    responsePrintContinue("%s:%u", fieldNames[i], bitField & 1);
    bitField >>= 1;
  }
}

static void printRailState(void)
{
  static const char * const railStates[] = {
    "RAIL_state_active",
    "RAIL_state_rx",
    "RAIL_state_tx"
  };
  sl_rail_radio_state_t rfState = sl_rail_get_radio_state(railHandle);
  responsePrintContinue("RfState:%s",
                        getRfStateName(rfState));
  printBitField(rfState,
                railStates,
                sizeof(railStates) / sizeof(railStates[0])
                );
}

const char *getStatusMessage(sl_rail_status_t status)
{
  switch (status) {
    case SL_RAIL_STATUS_NO_ERROR:
      return "Success";
    case SL_RAIL_STATUS_INVALID_PARAMETER:
      return "Invalid Parameter";
    case SL_RAIL_STATUS_INVALID_STATE:
      return "Invalid State";
    case SL_RAIL_STATUS_INVALID_CALL:
      return "Invalid Call";
    case SL_RAIL_STATUS_SUSPENDED:
      return "Suspended";
    case SL_RAIL_STATUS_SCHED_ERROR:
      return "Sched Error";
    default:
      return "Failure";
  }
}

void getStatus(sl_cli_command_arg_t *args)
{
  responsePrintStart(sl_cli_get_command_string(args, 0));
  responsePrintContinue("UserTxCount:%u,"
                        "AckTxCount:%u,"
                        "UserTxAborted:%u,"
                        "AckTxAborted:%u,"
                        "UserTxBlocked:%u,"
                        "AckTxBlocked:%u,"
                        "UserTxUnderflow:%u,"
                        "AckTxUnderflow:%u,"
                        "RxCount:%u,"
                        "RxCrcErrDrop:%u,"
                        "SyncDetect:%u,"
                        "SyncDetect0:%u,"
                        "SyncDetect1:%u,"
                        "NoRxBuffer:%u",
                        counters.userTx,
                        counters.ackTx,
                        counters.userTxAborted,
                        counters.ackTxAborted,
                        counters.userTxBlocked,
                        counters.ackTxBlocked,
                        counters.userTxUnderflow,
                        counters.ackTxUnderflow,
                        counters.receive,
                        counters.receiveCrcErrDrop,
                        counters.syncDetect,
                        counters.syncDetect0,
                        counters.syncDetect1,
                        counters.noRxBuffer
                        );
  responsePrintContinue("TxRemainErrs:%u,"
                        "RfSensed:%u,"
                        "ackTimeout:%u,"
                        "ackTxFpSet:%u,"
                        "ackTxFpFail:%u,"
                        "ackTxFpAddrFail:%u",
                        counters.userTxRemainingErrors,
                        counters.rfSensedEvent,
                        counters.ackTimeout,
                        counters.ackTxFpSet,
                        counters.ackTxFpFail,
                        counters.ackTxFpAddrFail
                        );
  printRailState();
  responsePrintContinue("Channel:%u,"
                        "AppMode:%s,"
                        "TimingLost:%u,"
                        "TimingDetect:%u,"
                        "FrameErrors:%u,"
                        "RxFifoFull:%u,"
                        "RxOverflow:%u,"
                        "AddrFilt:%u,"
                        "Aborted:%u,"
                        "RxBeams:%u,"
                        "DataRequests:%u,"
                        "RxTimeout:%u,"
                        "RxBlocked:%u",
                        getLikelyChannel(),
                        appModeNames(currentAppMode()),
                        counters.timingLost,
                        counters.timingDetect,
                        counters.frameError,
                        counters.rxFifoFull,
                        counters.rxOfEvent,
                        counters.addrFilterEvent,
                        counters.rxFail,
                        counters.rxBeams,
                        counters.dataRequests,
                        counters.rxTimeout,
                        counters.rxBlocked
                        );
  responsePrintContinue("Calibrations:%u,"
                        "TxChannelBusy:%u,"
                        "TxClear:%u,"
                        "TxCca:%u,"
                        "TxCcaAct:%u,"
                        "TxRetry:%u,"
                        "TxRetryCcaNow:%u,"
                        "UserTxStarted:%u,"
                        "PaProtect:%u",
                        counters.calibrations,
                        counters.txChannelBusy,
                        counters.lbtSuccess,
                        counters.lbtStartCca,
                        counters.lbtCcaActivated,
                        counters.lbtRetry,
                        counters.lbtNextCcaNow,
                        counters.userTxStarted,
                        counters.paProtect
                        );
  for (uint32_t subPhyId = 0; subPhyId < SUBPHYID_COUNT; subPhyId++) {
    responsePrintContinue("SubPhy%u:%u",
                          subPhyId,
                          counters.subPhyCount[subPhyId]);
  }
  // Avoid use of %ll long-long formats due to iffy printf library support
  responsePrintEnd("rxRawSourceBytes:0x%x%08x",
                   (uint32_t)(counters.rxRawSourceBytes >> 32),
                   (uint32_t)(counters.rxRawSourceBytes)
                   );
}

void fifoStatus(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  uint16_t spaceCount =  sl_rail_get_tx_fifo_space_available(railHandle);
  uint16_t byteCount = sl_rail_get_rx_fifo_bytes_available(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0),
                "TxSpaceCount:%u,"
                "RxByteCount:%u,"
                "TxFifoThreshold:%d,"
                "RxFifoThreshold:%d,"
                "TxFifoAlmostEmpty:%u,"
                "RxFifoAlmostFull:%u,"
                "RxFifoFull:%u,"
                "RxOverflow:%u,"
                "UserTxUnderflow:%u,"
                "AckTxUnderflow:%u",
                spaceCount,
                byteCount,
                sl_rail_get_tx_fifo_threshold(railHandle),
                sl_rail_get_rx_fifo_threshold(railHandle),
                counters.txFifoAlmostEmpty,
                counters.rxFifoAlmostFull,
                counters.rxFifoFull,
                counters.rxOfEvent,
                counters.userTxUnderflow,
                counters.ackTxUnderflow
                );
}

static void printRailVersion(sl_cli_command_arg_t *args, bool verbose)
{
  sl_rail_version_t rail_ver;
  sl_rail_get_version(&rail_ver);
  responsePrintStart(sl_cli_get_command_string(args, 0));
  responsePrintContinue("App:%u.%u.%u",
                        rail_ver.major, rail_ver.minor, rail_ver.rev);
  if (verbose) {
    responsePrintContinue("RAIL:%u.%u.%u.%u,"
                          "hash:0x%.8X,"
                          "flags:0x%.2X",
                          rail_ver.major, rail_ver.minor, rail_ver.rev, rail_ver.build,
                          rail_ver.hash,
                          rail_ver.flags);
  } else {
    responsePrintContinue("RAIL:%u.%u.%u",
                          rail_ver.major, rail_ver.minor, rail_ver.rev);
  }
  responsePrintEnd("Multiprotocol:%s,"
                   "Built:%s",
                   rail_ver.multiprotocol ? "True" : "False",
                   buildDateTime);
}

void getVersion(sl_cli_command_arg_t *args)
{
  printRailVersion(args, false);
}

void getVersionVerbose(sl_cli_command_arg_t *args)
{
  printRailVersion(args, true);
}

static const char * const ptiModes[] = {
  "Disabled", "3-wire-SPI", "2-wire-UART", "1-wire-UART", "??",
};

void getPti(sl_cli_command_arg_t *args)
{
  sl_rail_pti_config_t ptiConfig;
  CHECK_RAIL_HANDLE((args == NULL) ? "pti" : sl_cli_get_command_string(args, 0));
  // Get the current config and change the baud rate as requested
  sl_rail_get_pti_config(railHandle, &ptiConfig);
  if (ptiConfig.mode >= (sizeof(ptiModes) / sizeof(*ptiModes))) {
    ptiConfig.mode = (sizeof(ptiModes) / sizeof(*ptiModes)) - 1U;
  }
  responsePrint(((args == NULL) ? "pti" : sl_cli_get_command_string(args, 0)),
                "mode:%s,baud:%u,protocol:%u,radioConfig:0x%02x",
                ptiModes[ptiConfig.mode],
                ptiConfig.baud,
                sl_rail_get_pti_protocol(railHandle),
                sl_rail_ieee802154_get_phy_id(railHandle));
}

void setPtiProtocol(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
    return;
  }
  (void) sl_rail_set_pti_protocol(railHandle, (sl_rail_pti_protocol_t) sl_cli_get_argument_uint8(args, 0));
  // success (or not) will be reflected in getPti() output
  getPti(args);
}

void getRssi(sl_cli_command_arg_t *args)
{
  char bufRssi[10];
  uint32_t waitTimeout = SL_RAIL_GET_RSSI_NO_WAIT;

  if (sl_cli_get_argument_count(args) == 1) {
    waitTimeout = sl_cli_get_argument_uint8(args, 0);
    // For backwards compatibility, map the value 1 to a no timeout wait
    if (waitTimeout == 1) {
      waitTimeout = SL_RAIL_GET_RSSI_WAIT_WITHOUT_TIMEOUT;
    }
  }
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  int16_t rssi = sl_rail_get_rssi(railHandle, waitTimeout);

  // The lowest negative value is used to indicate an error reading the RSSI
  if (rssi == SL_RAIL_RSSI_INVALID) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x08, "Could not read RSSI. Ensure Rx is enabled");
    return;
  }

  sprintfFloat(bufRssi, sizeof(bufRssi), ((float) rssi / 4), 2);
  if (sl_rail_ieee802154_is_enabled(railHandle)) {
    uint8_t energyDetect = sl_rail_ieee802154_convert_rssi_to_ed(rssi / 4);
    responsePrint(sl_cli_get_command_string(args, 0), "rssi:%s,ed154:%u", bufRssi, energyDetect);
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "rssi:%s", bufRssi);
  }
}

void startAvgRssi(sl_cli_command_arg_t *args)
{
  uint32_t averageTimeUs = sl_cli_get_argument_uint32(args, 0);
  uint16_t avgChannel = channel;
  if (sl_cli_get_argument_count(args) == 2) {
    avgChannel = sl_cli_get_argument_uint16(args, 1);
  }
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));

#if defined(_SILICON_LABS_IP_PROJ_IS_LPWH74000) || (_SILICON_LABS_32B_SERIES_3_CONFIG == 353)
  if ((!inRadioState(SL_RAIL_RF_STATE_IDLE, NULL)) && (!inRadioState(SL_RAIL_RF_STATE_RX, NULL))) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x08, "Could not read RSSI. Ensure Radio is in Rx or in Idle State.");
    return;
  }
#else
  if (!inRadioState(SL_RAIL_RF_STATE_IDLE, NULL)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x08, "Could not read RSSI. Ensure RX is disabled.");
    return;
  }
#endif
  sl_rail_time_t startTime = sl_rail_get_time(railHandle);
  if (sl_rail_start_average_rssi(railHandle, avgChannel, averageTimeUs, NULL) != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x08, "Could not start RSSI averaging.");
    return;
  }
  responsePrint(sl_cli_get_command_string(args, 0), "Time:%d", startTime);
}

void getAvgRssi(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  int16_t rssi = sl_rail_get_average_rssi(railHandle);
  char bufRssi[10];
  if (rssi == SL_RAIL_RSSI_INVALID) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x08, "Invalid RSSI. Make sure startAvgRssi ran successfully.");
    return;
  }
  sprintfFloat(bufRssi, sizeof(bufRssi), ((float) rssi / 4), 2);
  responsePrint(sl_cli_get_command_string(args, 0), "rssi:%s", bufRssi);
}

void getRssiOffset(sl_cli_command_arg_t *args)
{
  // Protocol specific RSSI offset
  int8_t protocolRssiOffset = ((railHandle == NULL)
                               ? 0 : sl_rail_get_rssi_offset(railHandle));
  // Radio specific RSSI offset
  int8_t radioRssiOffset = sl_rail_get_rssi_offset(SL_RAIL_EFR32_HANDLE);
  // NVM RSSI offset
  int8_t nvmRssiOffset = sl_rail_util_read_nvm_rssi();
  char *nvmToken;
#if (defined(SL_CATALOG_NVM3_DEFAULT_PRESENT) \
  && !defined(SL_TRUSTZONE_SECURE))
  char nvmTokenContent[sizeof("invalid_0xffffffff\0")];
  uint32_t obj_type;
  size_t obj_size;
  sl_status_t status = nvm3_getObjectInfo(nvm3_defaultHandle,
                                          SL_RAIL_UTIL_RSSI_NVM_DATA_TAG,
                                          &obj_type, &obj_size);
  if (status != SL_STATUS_OK) {
    nvmToken = "nonexistent";
  } else if ((obj_type != NVM3_OBJECTTYPE_DATA)
             || (obj_size != sizeof(uint16_t))) {
    nvmToken = "invalid_size";
  } else {
    uint16_t readNvmRssiOffset;
    status = nvm3_readData(nvm3_defaultHandle,
                           SL_RAIL_UTIL_RSSI_NVM_DATA_TAG,
                           &readNvmRssiOffset,
                           sizeof(readNvmRssiOffset));
    if (status != SL_STATUS_OK) {
      nvmToken = "read_failed";
    } else {
      (void)snprintf(nvmTokenContent, sizeof(nvmTokenContent),
                     "%svalid_0x%04x",
                     (((~(readNvmRssiOffset) & 0xFFU) == ((readNvmRssiOffset >> 8U) & 0xFFU))
                      ? "" : "in"),
                     readNvmRssiOffset);
      nvmToken = nvmTokenContent;
    }
  }
#else
  nvmToken = "no_nvm";
#endif
  responsePrint(sl_cli_get_command_string(args, 0), "rssiOffset:%d,radioRssiOffset:%d,totalRssiOffset:%d,nvmRssiOffset:%d,nvmToken:%s", \
                protocolRssiOffset,
                radioRssiOffset,
                (protocolRssiOffset + radioRssiOffset),
                nvmRssiOffset, nvmToken);
}

void setRssiOffset(sl_cli_command_arg_t *args)
{
  int8_t rssiOffset = sl_cli_get_argument_int8(args, 0);
  char *which = "protocol"; // Assume protocol
  sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
  if (sl_cli_get_argument_count(args) > 1) {
    which = sl_cli_get_argument_string(args, 1);
  }
  switch (which[0]) {
    case 'r': // radio
      status = sl_rail_set_rssi_offset(SL_RAIL_EFR32_HANDLE, rssiOffset);
      break;
    case 'p': // protocol
      CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
      if (!inRadioState(SL_RAIL_RF_STATE_IDLE, sl_cli_get_command_string(args, 0))) {
        return;
      }
      status = sl_rail_set_rssi_offset(railHandle, rssiOffset);
      break;
    case 'n': // nvm
      if (rssiOffset == -128) { // erase the token
#if (defined(SL_CATALOG_NVM3_DEFAULT_PRESENT) \
        && !defined(SL_TRUSTZONE_SECURE))
        // No API for this, so use native NVM3 APIs
        status = (sl_rail_status_t)nvm3_deleteObject(nvm3_defaultHandle,
                                                     SL_RAIL_UTIL_RSSI_NVM_DATA_TAG);
        if ((sl_status_t)status != SL_STATUS_OK) {
          responsePrint(sl_cli_get_command_string(args, 0), "nvmTag:0x%x,deleteNvmStatus:0x%04X",
                        SL_RAIL_UTIL_RSSI_NVM_DATA_TAG, status);
          return;
        }
        // getRssiOffset below should show the token as nonexistent now
#else
        // getRssiOffset below should show the token as no_nvm
#endif
      } else {
        status = (sl_rail_status_t)sl_rail_util_write_nvm_rssi(rssiOffset);
      }
      break;
    default:
      responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
                         "Unrecognized location %s, expected 'protocol', 'radio', or 'nvm'",
                         which);
      return;
      break;
  }
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    responsePrintError(sl_cli_get_command_string(args, 0), status, "Error setting the rssiOffset");
  } else {
    getRssiOffset(args);
  }
}

void getRssiDetectThreshold(sl_cli_command_arg_t *args)
{
  int8_t rssiDetectThresholdDbm = sl_rail_get_rssi_detect_threshold(railHandle);
  responsePrint(sl_cli_get_command_string(args, 0), "rssiDetectThresholdDbm:%d,status:%s",
                rssiDetectThresholdDbm,
                rssiDetectThresholdDbm == SL_RAIL_RSSI_INVALID_DBM ? "Disabled" : "Enabled");
}

void setRssiDetectThreshold(sl_cli_command_arg_t *args)
{
  int8_t rssiDetectThresholdDbm = sl_cli_get_argument_int8(args, 0);
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (!sl_rail_supports_rssi_detect_threshold(railHandle)) {
    responsePrintError(sl_cli_get_command_string(args, 0), 0x0A, "RSSI threshold detection is not supported on this chip.");
  } else {
    if (SL_RAIL_STATUS_NO_ERROR != sl_rail_set_rssi_detect_threshold(railHandle, rssiDetectThresholdDbm)) {
      responsePrintError(sl_cli_get_command_string(args, 0), 0x0B, "Error setting the rssiDetectThreshold.");
    } else {
      getRssiDetectThreshold(args);
    }
  }
}

void sweepPower(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  int32_t lowPower = sl_cli_get_argument_int32(args, 0);
  int32_t hiPower = sl_cli_get_argument_int32(args, 1);
  int32_t period = sl_cli_get_argument_int32(args, 2);
  int32_t halfPeriodStepUs = period / 2;
  uint32_t expired = sl_rail_get_time(railHandle) + (uint32_t)5000000;
  while (expired > sl_rail_get_time(railHandle)) {
    sl_rail_stop_tx_stream(railHandle);
    sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, false);
    sli_rail_set_tx_power(railHandle, lowPower);
    sl_rail_start_tx_stream(railHandle, channel, SL_RAIL_STREAM_CARRIER_WAVE,
                            SL_RAIL_TX_OPTIONS_DEFAULT);
    usDelay(halfPeriodStepUs);
    sl_rail_stop_tx_stream(railHandle);
    sl_rail_idle(railHandle, SL_RAIL_IDLE_ABORT, false);
    sli_rail_set_tx_power(railHandle, hiPower);
    sl_rail_start_tx_stream(railHandle, channel, SL_RAIL_STREAM_CARRIER_WAVE,
                            SL_RAIL_TX_OPTIONS_DEFAULT);
    usDelay(halfPeriodStepUs);
  }
  sl_rail_stop_tx_stream(railHandle);
}

void isRssiRdy(sl_cli_command_arg_t *args)
{
  CHECK_RAIL_HANDLE(sl_cli_get_command_string(args, 0));
  if (sl_rail_is_average_rssi_ready(railHandle)) {
    responsePrint(sl_cli_get_command_string(args, 0), "isReady:True");
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "isReady:False");
  }
  return;
}

void resetCounters(sl_cli_command_arg_t *args)
{
  memset(&counters, 0, sizeof(counters));
  args->argc = sl_cli_get_command_count(args); /* only reference cmd str */
  getStatus(args);
}

void getTime(sl_cli_command_arg_t *args)
{
  responsePrint(sl_cli_get_command_string(args, 0), "Time:%u",
                sl_rail_get_time(railHandle));
}

void setTime(sl_cli_command_arg_t *args)
{
  uint32_t timeUs = sl_cli_get_argument_uint32(args, 0);
  if (SL_RAIL_STATUS_NO_ERROR == sl_rail_set_time(railHandle, timeUs)) {
    responsePrint(sl_cli_get_command_string(args, 0), "Status:Success,CurrentTime:%u",
                  sl_rail_get_time(railHandle));
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "Status:Error,CurrentTime:%u",
                  sl_rail_get_time(railHandle));
  }
}

static const uint32_t compileTimePowerModes
  = (0U
    #ifdef  SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP
     | (1UL << SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP
     | (1UL << SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP
     | (1UL << SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP
     | (1UL << SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST
     | (1UL << SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP
     | (1UL << SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP
     | (1UL << SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP
     | (1UL << SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP
     | (1UL << SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST
     | (1UL << SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE
     | (1UL << SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE
     | (1UL << SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE)
    #endif
    #ifdef  SL_RAIL_TX_POWER_MODE_NONE
     | (1UL << SL_RAIL_TX_POWER_MODE_NONE)
    #endif
     );

void printChipFeatures(sl_cli_command_arg_t *args)
{
  sli_rail_tx_power_level_t maxPowerLevel;
  responsePrintHeader(sl_cli_get_command_string(args, 0), "Feature:%s,CompileTime:%s,RunTime:%s");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_DUAL_BAND",
                     SL_RAIL_SUPPORTS_DUAL_BAND ? "Yes" : "No",
                     sl_rail_supports_dual_band(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_2P4_GHZ_BAND",
                     SL_RAIL_SUPPORTS_2P4_GHZ_BAND ? "Yes" : "No",
                     sl_rail_supports_2p4_ghz_band(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_SUB_GHZ_BAND",
                     SL_RAIL_SUPPORTS_SUB_GHZ_BAND ? "Yes" : "No",
                     sl_rail_supports_sub_ghz_band(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_OFDM_PA",
                     SL_RAIL_SUPPORTS_OFDM_PA ? "Yes" : "No",
                     sl_rail_supports_ofdm_pa(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_ALTERNATE_TX_POWER",
                     SL_RAIL_SUPPORTS_ALTERNATE_TX_POWER ? "Yes" : "No",
                     sl_rail_supports_alternate_tx_power(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY",
                     SL_RAIL_SUPPORTS_ANTENNA_DIVERSITY ? "Yes" : "No",
                     sl_rail_supports_antenna_diversity(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_CHANNEL_HOPPING",
                     SL_RAIL_SUPPORTS_CHANNEL_HOPPING ? "Yes" : "No",
                     sl_rail_supports_channel_hopping(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_DUAL_SYNC_WORDS",
                     SL_RAIL_SUPPORTS_DUAL_SYNC_WORDS ? "Yes" : "No",
                     sl_rail_supports_dual_sync_words(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR",
                     SL_RAIL_SUPPORTS_EXTERNAL_THERMISTOR ? "Yes" : "No",
                     sl_rail_supports_external_thermistor(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_HFXO_COMPENSATION",
                     SL_RAIL_SUPPORTS_HFXO_COMPENSATION ? "Yes" : "No",
                     sl_rail_supports_hfxo_compensation(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_AUXADC",
                     SL_RAIL_SUPPORTS_AUXADC ? "Yes" : "No",
                     sl_rail_supports_aux_adc(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_PRECISION_LFRCO",
                     SL_RAIL_SUPPORTS_PRECISION_LFRCO ? "Yes" : "No",
                     sl_rail_supports_precision_lfrco(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_RADIO_ENTROPY",
                     SL_RAIL_SUPPORTS_RADIO_ENTROPY ? "Yes" : "No",
                     sl_rail_supports_radio_entropy(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION",
                     SL_RAIL_SUPPORTS_RF_SENSE_ENERGY_DETECTION ? "Yes" : "No",
                     sl_rail_supports_rf_sense_energy_detection(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK",
                     SL_RAIL_SUPPORTS_RF_SENSE_SELECTIVE_OOK ? "Yes" : "No",
                     sl_rail_supports_rf_sense_selective_ook(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_TX_REPEAT_START_TO_START",
                     SL_RAIL_SUPPORTS_TX_REPEAT_START_TO_START ? "Yes" : "No",
                     sl_rail_supports_tx_repeat_start_to_start(railHandle) ? "Yes" : "No");
  uint32_t compileTimeMask = compileTimePowerModes;
  for (sl_rail_tx_power_mode_t mode = 0U; mode < SL_RAIL_TX_POWER_MODE_NONE; mode++) {
    sl_rail_tx_power_mode_t altMode = mode;
    if (sli_rail_supports_tx_power_mode(railHandle, &altMode, &maxPowerLevel, NULL)
        && (maxPowerLevel != SLI_RAIL_TX_POWER_LEVEL_INVALID)) {
      responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%u",
                         paStrings[mode],
                         (compileTimeMask & 1U) ? "Yes" : "N/A",
                         maxPowerLevel);
    } else {
      responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:No",
                         paStrings[mode],
                         (compileTimeMask & 1U) ? "Yes" : "N/A");
    }
    compileTimeMask >>= 1;
  }
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_PROTOCOL_BLE",
                     SL_RAIL_SUPPORTS_PROTOCOL_BLE ? "Yes" : "No",
                     sl_rail_supports_protocol_ble(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_1_MBPS",
                     SL_RAIL_BLE_SUPPORTS_1_MBPS ? "Yes" : "No",
                     sl_rail_ble_supports_1_mbps(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_2_MBPS",
                     SL_RAIL_BLE_SUPPORTS_2_MBPS ? "Yes" : "No",
                     sl_rail_ble_supports_2_mbps(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_ANTENNA_SWITCHING",
                     SL_RAIL_BLE_SUPPORTS_ANTENNA_SWITCHING ? "Yes" : "No",
                     sl_rail_ble_supports_antenna_switching(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_CODED_PHY",
                     SL_RAIL_BLE_SUPPORTS_CODED_PHY ? "Yes" : "No",
                     sl_rail_ble_supports_coded_phy(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_CTE",
                     SL_RAIL_BLE_SUPPORTS_CTE ? "Yes" : "No",
                     sl_rail_ble_supports_cte(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_QUUPPA",
                     SL_RAIL_BLE_SUPPORTS_QUUPPA ? "Yes" : "No",
                     sl_rail_ble_supports_quuppa(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_IQ_SAMPLING",
                     SL_RAIL_BLE_SUPPORTS_IQ_SAMPLING ? "Yes" : "No",
                     sl_rail_ble_supports_iq_sampling(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_BLE_SUPPORTS_PHY_SWITCH_TO_RX",
                     SL_RAIL_BLE_SUPPORTS_PHY_SWITCH_TO_RX ? "Yes" : "No",
                     sl_rail_ble_supports_phy_switch_to_rx(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154",
                     SL_RAIL_SUPPORTS_PROTOCOL_IEEE802154 ? "Yes" : "No",
                     sl_rail_supports_protocol_ieee802154(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY",
                     SL_RAIL_IEEE802154_SUPPORTS_COEX_PHY ? "Yes" : "No",
                     sl_rail_ieee802154_supports_coex_phy(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_E_SUBSET_GB868",
                     SL_RAIL_IEEE802154_SUPPORTS_E_SUBSET_GB868 ? "Yes" : "No",
                     sl_rail_ieee802154_supports_e_subset_gb868(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_E_ENHANCED_ACK",
                     SL_RAIL_IEEE802154_SUPPORTS_E_ENHANCED_ACK ? "Yes" : "No",
                     sl_rail_ieee802154_supports_e_enhanced_ack(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868",
                     SL_RAIL_IEEE802154_SUPPORTS_G_SUBSET_GB868 ? "Yes" : "No",
                     sl_rail_ieee802154_supports_g_subset_gb868(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_G_DYN_FEC",
                     SL_RAIL_IEEE802154_SUPPORTS_G_DYN_FEC ? "Yes" : "No",
                     sl_rail_ieee802154_supports_g_dyn_fec(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH",
                     SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH ? "Yes" : "No",
                     sl_rail_ieee802154_supports_g_mode_switch(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_RX",
                     SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_RX ? "Yes" : "No",
                     sl_rail_ieee802154_supports_g_unwhitened_rx(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_TX",
                     SL_RAIL_IEEE802154_SUPPORTS_G_UNWHITENED_TX ? "Yes" : "No",
                     sl_rail_ieee802154_supports_g_unwhitened_tx(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_G_4_BYTE_CRC",
                     SL_RAIL_IEEE802154_SUPPORTS_G_4_BYTE_CRC ? "Yes" : "No",
                     sl_rail_ieee802154_supports_g_4_byte_crc(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_CANCEL_FRAME_PENDING_LOOKUP",
                     SL_RAIL_IEEE802154_SUPPORTS_CANCEL_FRAME_PENDING_LOOKUP ? "Yes" : "No",
                     sl_rail_ieee802154_supports_cancel_frame_pending_lookup(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_EARLY_FRAME_PENDING_LOOKUP",
                     SL_RAIL_IEEE802154_SUPPORTS_EARLY_FRAME_PENDING_LOOKUP ? "Yes" : "No",
                     sl_rail_ieee802154_supports_early_frame_pending_lookup(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_E_MULTIPURPOSE_FRAMES",
                     SL_RAIL_IEEE802154_SUPPORTS_E_MULTIPURPOSE_FRAMES ? "Yes" : "No",
                     sl_rail_ieee802154_supports_e_multipurpose_frames(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_DUAL_PA_CONFIG",
                     SL_RAIL_IEEE802154_SUPPORTS_DUAL_PA_CONFIG ? "Yes" : "No",
                     sl_rail_ieee802154_supports_dual_pa_config(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING",
                     SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING ? "Yes" : "No",
                     sl_rail_ieee802154_supports_rx_channel_switching(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY",
                     SL_RAIL_IEEE802154_SUPPORTS_2_MBPS_PHY ? "Yes" : "No",
                     sl_rail_ieee802154_supports_2_mbps_phy(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE",
                     SL_RAIL_SUPPORTS_PROTOCOL_ZWAVE ? "Yes" : "No",
                     sl_rail_supports_protocol_zwave(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_ZWAVE_SUPPORTS_REGION_PTI",
                     SL_RAIL_ZWAVE_SUPPORTS_REGION_PTI ? "Yes" : "No",
                     sl_rail_zwave_supports_region_pti(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_ZWAVE_SUPPORTS_CONC_PHY",
                     SL_RAIL_ZWAVE_SUPPORTS_CONC_PHY ? "Yes" : "No",
                     sl_rail_zwave_supports_conc_phy(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_SQ_PHY",
                     SL_RAIL_SUPPORTS_SQ_PHY ? "Yes" : "No",
                     sl_rail_supports_sq_phy(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_RX_RAW_DATA",
                     SL_RAIL_SUPPORTS_RX_RAW_DATA ? "Yes" : "No",
                     sl_rail_supports_rx_raw_data(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_DIRECT_MODE",
                     SL_RAIL_SUPPORTS_DIRECT_MODE ? "Yes" : "No",
                     sl_rail_supports_direct_mode(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_ADDR_FILTER_ADDRESS_BIT_MASK",
                     SL_RAIL_SUPPORTS_ADDR_FILTER_ADDRESS_BIT_MASK ? "Yes" : "No",
                     sl_rail_supports_addr_filter_address_bit_mask(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_MFM",
                     SL_RAIL_SUPPORTS_MFM ? "Yes" : "No",
                     sl_rail_supports_mfm(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_VDET",
                     SL_RAIL_SUPPORTS_VDET ? "Yes" : "No",
                     sl_rail_supports_vdet(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND",
                     SL_RAIL_IEEE802154_SUPPORTS_2P4_GHZ_BAND ? "Yes" : "No",
                     sl_rail_ieee802154_supports_2p4_ghz_band(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_RSSI_DETECT_THRESHOLD",
                     SL_RAIL_SUPPORTS_RSSI_DETECT_THRESHOLD ? "Yes" : "No",
                     sl_rail_supports_rssi_detect_threshold(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_THERMAL_PROTECTION",
                     SL_RAIL_SUPPORTS_THERMAL_PROTECTION ? "Yes" : "No",
                     sl_rail_supports_thermal_protection(railHandle) ? "Yes" : "No");
  responsePrintMulti("Feature:%s,CompileTime:%s,RunTime:%s",
                     "SL_RAIL_SUPPORTS_RX_DUTY_CYCLING",
                     SL_RAIL_SUPPORTS_RX_DUTY_CYCLING ? "Yes" : "No",
                     sl_rail_supports_rx_duty_cycling(railHandle) ? "Yes" : "No");
}

void cliSeparatorHack(sl_cli_command_arg_t *args)
{
  (void) args;
}
