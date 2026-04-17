/***************************************************************************//**
 * @file
 * @brief  EZSP ASH Host user interface functions
 *
 *  This includes command option parsing, trace and counter functions.
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
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include "stack/include/sl_zigbee_types.h"
#include "ash-protocol.h"
#include "ash-common.h"
#include "app/ezsp-host/ash/ash-host.h"
#include "app/ezsp-host/ezsp-host-priv.h"
#include "app/ezsp-host/ezsp-host-io.h"
#include "app/ezsp-host/ezsp-host-queues.h"
#include "app/ezsp-host/ezsp-host-ui.h"
#include "app/ezsp-host/ash/ash-host-ui.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif  // SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT
#include "ota-storage-linux-config.h"
#endif  // SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT

//------------------------------------------------------------------------------
// Preprocessor definitions

#define ERR_LEN               128   // max length error message

#define txControl (txBuffer[0])     // more descriptive aliases
#define rxControl (rxBuffer[0])

static const char options[] = "b:f:hv::i:j:n:o:p:r:s:t:x:d:";
extern bool checkSerialPort(const char* portString, bool silent);

extern int optind, opterr, optopt;
extern char *optarg;
#ifdef SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT
// To set the OTA file storage directory in runtime.
extern char defaultStorageDirectory[OTA_FILE_STORAGE_DIR_LENGTH];
#endif  // SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT

static char *dhcJsonPath = NULL;

bool sl_zigbee_ezsp_internal_process_command_options(int argc, char *argv[], char *errStr)
{
  int c;
  char port[ASH_PORT_LEN];
  char devport[ASH_PORT_LEN];
  uint32_t baud;
  uint8_t stops;
  uint8_t trace;
  uint8_t enable;
  uint8_t portnum;
  uint8_t cfg;
  int blksize;
  int optionCount = 0;

  if (!argv || !errStr) {
    snprintf(errStr, ERR_LEN, "Error: argv[] is %s and errStr is %s.\n",
             argv ? "valid" : "NULL",
             errStr ? "valid" : "NULL");
    return false;
  }

  while (true) {
    c = getopt(argc, argv, options);
    if (c == -1) {
      if (optind != argc ) {
        snprintf(errStr, ERR_LEN, "Invalid option %s.\n", argv[optind]);
      }
      break;
    }

    optionCount++;

    switch (c) {
      case 'b':
        if (!optarg || (sscanf(optarg, "%u", &baud) != 1)) {
          snprintf(errStr, ERR_LEN, "Invalid baud rate %s.\n",
                   optarg ? optarg : "NULL");
        } else {
          sli_ash_host_set_config(baudRate, baud);
        }
        break;
      case 'f':
        if (!optarg) {
          snprintf(errStr, ERR_LEN, "Invalid flow control choice NULL.\n");
          break;
        }
        switch (*optarg) {
          case 'r':
            sli_ash_host_set_config(rtsCts, true);
            break;
          case 'x':
            sli_ash_host_set_config(rtsCts, false);
            break;
          default:
            snprintf(errStr, ERR_LEN, "Invalid flow control choice %s.\n", optarg);
        }
        break;
      case 'h':
      case '?':
        snprintf(errStr, ERR_LEN, "\n");
        break;
      case 'i':
        if (!optarg || (sscanf(optarg, "%hhu", &enable) != 1) || (enable > 1)) {
          snprintf(errStr, ERR_LEN, "Invalid input buffer choice %s.\n",
                   optarg ? optarg : "NULL");
        } else {
          blksize = enable ? 256 : 1;
          sli_ash_host_set_config(inBlockLen, blksize);
        }
        break;
      case 'j':
        if (!optarg) {
          snprintf(errStr, ERR_LEN, "Invalid DHC configuration path NULL.\n");
          return false;
        } else if (access(optarg, R_OK) != 0) {
          snprintf(errStr, ERR_LEN, "Cannot access DHC configuration file %s.\n", optarg);
          return false;
        } else {
          if (dhcJsonPath != NULL) {
            free(dhcJsonPath);
            dhcJsonPath = NULL;
          }
          size_t len = strlen(optarg) + 1U;
          dhcJsonPath = malloc(len);
          if (dhcJsonPath == NULL) {
            snprintf(errStr, ERR_LEN, "Failed to allocate memory for DHC configuration path.\n");
            return false;
          } else {
            memcpy(dhcJsonPath, optarg, len);
          }
        }
        break;
      case 'n':
        if (optionCount != 1) {
          snprintf(errStr, ERR_LEN, "NCP option, if present, must be first.\n");
        } else if (!optarg || (sscanf(optarg, "%hhu", &cfg) != 1) || (cfg > 1)) {
          snprintf(errStr, ERR_LEN, "Invalid NCP config choice %s.\n",
                   optarg ? optarg : "NULL");
        } else {
          ashSelectHostConfig(cfg);
        }
        break;
      case 'o':
        if (!optarg || (sscanf(optarg, "%hhu", &enable) != 1) || (enable > 1)) {
          snprintf(errStr, ERR_LEN, "Invalid output buffer choice %s.\n",
                   optarg ? optarg : "NULL");
        } else {
          blksize = enable ? 256 : 1;
          sli_ash_host_set_config(outBlockLen, blksize);
        }
        break;
      case 'p':
        if (!optarg || (sscanf(optarg, "%39s", port) <= 0)) {
          snprintf(errStr, ERR_LEN, "Invalid serial port name %s.\n",
                   optarg ? optarg : "NULL");
        } else if (strlen(port) >= ASH_PORT_LEN - 1) {
          snprintf(errStr, ERR_LEN, "Serial port name %s too long.\n", port);
        } else {
          // Handle some common variations specifying a serial port
          if ( port[0] == '/' ) {
            strncpy(devport, port, ASH_PORT_LEN - 1);
  #ifdef __CYGWIN__
          } else if ( ((strncmp("COM", port, 3) == 0)
                       || (strncmp("com", port, 3) == 0) )
                      && (sscanf(port + 3, "%hhu", &portnum) == 1)
                      && (portnum > 0) ) {
            snprintf(devport, ASH_PORT_LEN, "/dev/ttyS%hhu", portnum - 1);
          } else if ( (sscanf(port, "%hhu", &portnum) == 1) && portnum ) {
            snprintf(devport, ASH_PORT_LEN, "/dev/ttyS%hhu", portnum - 1);
  #else
          } else if (sscanf(port, "%hhu", &portnum) == 1) {
            snprintf(devport, ASH_PORT_LEN, "/dev/ttyS%hhu", portnum);
  #endif
          } else {
            strncpy(devport, "/dev/", 6);
            devport[5] = '\0';
            strncat(devport, port, ASH_PORT_LEN - 1);
          }
          strncpy(ashHostConfig.serialPort, devport, ASH_PORT_LEN - 1);
          ashHostConfig.serialPort[ASH_PORT_LEN - 1] = '\0';
          if (!checkSerialPort(ashHostConfig.serialPort, false)) {
            return false;
          }
        }
        break;
      case 'r':
        if (!optarg) {
          snprintf(errStr, ERR_LEN, "Invalid reset method NULL.\n");
          break;
        }
        switch (*optarg) {
          case 'r':
            sli_ash_host_set_config(resetMethod, ASH_RESET_METHOD_RST);
            break;
          case 'd':
            sli_ash_host_set_config(resetMethod, ASH_RESET_METHOD_DTR);
            break;
          case 'c':
            sli_ash_host_set_config(resetMethod, ASH_RESET_METHOD_CUSTOM);
            break;
          default:
            snprintf(errStr, ERR_LEN, "Invalid reset method %s.\n", optarg);
        }
        break;
      case 's':
        if (!optarg || (sscanf(optarg, "%hhu", &stops) != 1) || (stops < 1)
            || (stops > 2)) {
          snprintf(errStr, ERR_LEN, "Invalid number of stop bits %s.\n",
                   optarg ? optarg : "NULL");
        } else {
          sli_ash_host_set_config(stopBits, stops);
        }
        break;
      case 't':
        if (!optarg || (sscanf(optarg, "%hhu", &trace) != 1)) {
          snprintf(errStr, ERR_LEN, "Invalid trace flag value %s.\n",
                   optarg ? optarg : "NULL");
        } else {
          sli_ash_host_set_config(traceFlags, trace);
        }
        break;
      case 'x':
        if (!optarg || (sscanf(optarg, "%hhu", &enable) != 1) || (enable > 1)) {
          snprintf(errStr, ERR_LEN, "Invalid randomization choice %s.\n",
                   optarg ? optarg : "NULL");
        } else {
          sli_ash_host_set_config(randomize, enable);
        }
        break;
      case 'd': {
  #ifdef SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT
        char otaStorageDir[128];
        int ret = 0;
        if (!optarg || (sscanf(optarg, "%127s", otaStorageDir) <= 0)) {
          ret = snprintf(errStr, ERR_LEN, "Invalid OTA files storage directory name %s.\n",
                         optarg ? optarg : "NULL");
        } else if (strlen(otaStorageDir) >= OTA_FILE_STORAGE_DIR_LENGTH) {
          ret = snprintf(errStr, ERR_LEN, "OTA files storage directory %s too long.\n",
                         otaStorageDir);
        } else {
          otaStorageDir[OTA_FILE_STORAGE_DIR_LENGTH - 1] = '\0';
          strncpy(defaultStorageDirectory, otaStorageDir, OTA_FILE_STORAGE_DIR_LENGTH);
        }
        if (ret < 0) {
          snprintf(errStr, ERR_LEN, "Error with OTA files storage directory.\n");
        }
  #endif  // SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT
      }
      break;
      default:
        assert(1);
        break;
    }   // end of switch (c)
  } //end while
  return true;
}

char *sl_zigbee_ezsp_get_dhc_json_path(void)
{
  return dhcJsonPath;
}

//------------------------------------------------------------------------------
// Counter functions

void ashPrintCounters(sli_ash_host_count_t *counters, bool clear)
{
  sli_ash_host_count_t a = *counters;

  if (clear) {
    ashClearCounters(counters);
  }
  printf("Host Counts        Received Transmitted\n");
  printf("Total bytes      %10d  %10d\n", a.rxBytes, a.txBytes);
  printf("DATA bytes       %10d  %10d\n", a.rxData, a.txData);
  printf("I/O blocks       %10d  %10d\n\n", a.rxBlocks, a.txBlocks);
  printf("Total frames     %10d  %10d\n", a.rxAllFrames, a.txAllFrames);
  printf("DATA frames      %10d  %10d\n", a.rxDataFrames, a.txDataFrames);
  printf("ACK frames       %10d  %10d\n", a.rxAckFrames, a.txAckFrames);
  printf("NAK frames       %10d  %10d\n", a.rxNakFrames, a.txNakFrames);
  printf("Retry frames     %10d  %10d\n", a.rxReDataFrames, a.txReDataFrames);
  printf("Cancelled        %10d  %10d\n", a.rxCancelled, a.txCancelled);
  printf("nRdy frames      %10d  %10d\n", a.rxN1Frames, a.txN1Frames);

  printf("\nHost Receive Errors\n");
  printf("CRC errors       %10d\n", a.rxCrcErrors);
  printf("Comm errors      %10d\n", a.rxCommErrors);
  printf("Length < minimum %10d\n", a.rxTooShort);
  printf("Length > maximum %10d\n", a.rxTooLong);
  printf("Bad controls     %10d\n", a.rxBadControl);
  printf("Bad lengths      %10d\n", a.rxBadLength);
  printf("Bad ACK numbers  %10d\n", a.rxBadAckNumber);
  printf("Out of buffers   %10d\n", a.rxNoBuffer);
  printf("Retry dupes      %10d\n", a.rxDuplicates);
  printf("Out of sequence  %10d\n", a.rxOutOfSequence);
  printf("ACK timeouts     %10d\n", a.rxAckTimeouts);
}

void ashCountFrame(bool sent)
{
  uint8_t control;

  if (sent) {
    control = readTxControl();
    BUMP_HOST_COUNTER(txAllFrames);
  } else {
    control = readRxControl();
    BUMP_HOST_COUNTER(rxAllFrames);
  }
  if ( (control & ASH_DFRAME_MASK) == ASH_CONTROL_DATA) {
    if (sent) {
      if (control & ASH_RFLAG_MASK) {
        BUMP_HOST_COUNTER(txReDataFrames);
      } else {
        BUMP_HOST_COUNTER(txDataFrames);
      }
    } else {
      if (control & ASH_RFLAG_MASK) {
        BUMP_HOST_COUNTER(rxReDataFrames);
      } else {
        BUMP_HOST_COUNTER(rxDataFrames);
      }
    }
  } else if ( (control & ASH_SHFRAME_MASK) == ASH_CONTROL_ACK) {
    if (sent) {
      BUMP_HOST_COUNTER(txAckFrames);
      if (control & ASH_NFLAG_MASK) {
        BUMP_HOST_COUNTER(txN1Frames);
      } else {
        BUMP_HOST_COUNTER(txN0Frames);
      }
    } else {
      BUMP_HOST_COUNTER(rxAckFrames);
      if (control & ASH_NFLAG_MASK) {
        BUMP_HOST_COUNTER(rxN1Frames);
      } else {
        BUMP_HOST_COUNTER(rxN0Frames);
      }
    }
  } else if ( (control & ASH_SHFRAME_MASK) == ASH_CONTROL_NAK) {
    if (sent) {
      BUMP_HOST_COUNTER(txNakFrames);
      if (control & ASH_NFLAG_MASK) {
        BUMP_HOST_COUNTER(txN1Frames);
      } else {
        BUMP_HOST_COUNTER(txN0Frames);
      }
    } else {
      BUMP_HOST_COUNTER(rxNakFrames);
      if (control & ASH_NFLAG_MASK) {
        BUMP_HOST_COUNTER(rxN1Frames);
      } else {
        BUMP_HOST_COUNTER(rxN0Frames);
      }
    }
  }
}

void ashClearCounters(sli_ash_host_count_t *counters)
{
  *counters = zeroAshCount;
}

//------------------------------------------------------------------------------
// Trace output functions

static void ashPrintFrame(uint8_t c)
{
  if ((c & ASH_DFRAME_MASK) == ASH_CONTROL_DATA) {
    if (c & ASH_RFLAG_MASK) {
      ezspDebugPrintf("DATA(%d,%d)", ASH_GET_FRMNUM(c), ASH_GET_ACKNUM(c));
    } else {
      ezspDebugPrintf("data(%d,%d)", ASH_GET_FRMNUM(c), ASH_GET_ACKNUM(c));
    }
  } else if ((c & ASH_SHFRAME_MASK) == ASH_CONTROL_ACK) {
    if (ASH_GET_NFLAG(c)) {
      ezspDebugPrintf("ack(%d)-  ", ASH_GET_ACKNUM(c));
    } else {
      ezspDebugPrintf("ack(%d)+  ", ASH_GET_ACKNUM(c));
    }
  } else if ((c & ASH_SHFRAME_MASK) == ASH_CONTROL_NAK) {
    if (ASH_GET_NFLAG(c)) {
      ezspDebugPrintf("NAK(%d)-  ", ASH_GET_ACKNUM(c));
    } else {
      ezspDebugPrintf("NAK(%d)+  ", ASH_GET_ACKNUM(c));
    }
  } else if (c == ASH_CONTROL_RST) {
    ezspDebugPrintf("RST      ");
  } else if (c == ASH_CONTROL_RSTACK) {
    ezspDebugPrintf("RSTACK   ");
  } else if (c == ASH_CONTROL_ERROR) {
    ezspDebugPrintf("ERROR    ");
  } else {
    ezspDebugPrintf("???? 0x%02X", c);
  }
}

static void ashPrintStatus(void)
{
  ezspDebugPrintf(" Ar=%d At=%d Ft=%d Fr=%d Frt=%d Ql=%d To=%d Tp=%d Bf=%d",
                  readAckRx(), readAckTx(), readFrmTx(), readFrmRx(), readFrmReTx(),
                  sl_zigbee_ezsp_queue_length(&reTxQueue),
                  readAshTimeouts(), ashGetAckPeriod(), sl_zigbee_ezsp_free_list_length(&rxFree) );
}

void ashTraceFrame(bool sent)
{
  uint8_t flags;

  ashCountFrame(sent);
  flags = sli_ash_host_get_config(traceFlags);
  if (flags & (TRACE_FRAMES_BASIC | TRACE_FRAMES_VERBOSE)) {
    sl_zigbee_ezsp_print_elapsed_time();
    if (sent) {
      ezspDebugPrintf("Tx ");
      ashPrintFrame(readTxControl());
      if (flags & TRACE_FRAMES_VERBOSE) {
        ashPrintStatus();
      }
      ezspDebugPrintf("   \r\n");
    } else {
      ezspDebugPrintf("   ");
      ashPrintFrame(readRxControl());
      if (flags & TRACE_FRAMES_VERBOSE) {
        ashPrintStatus();
      }
      ezspDebugPrintf(" Rx\r\n");
    }
  }
}

void ashTraceEventRecdFrame(const char *string)
{
  if (sli_ash_host_get_config(traceFlags) & TRACE_EVENTS) {
    sl_zigbee_ezsp_print_elapsed_time();
    ezspDebugPrintf("Rec'd frame: ");
    ezspDebugPrintf("%s", string);
    ezspDebugPrintf("\r\n");
  }
}

void ashTraceEventTime(const char *string)
{
  if (sli_ash_host_get_config(traceFlags) & TRACE_EVENTS) {
    sl_zigbee_ezsp_print_elapsed_time();
    ezspDebugPrintf("%s", string);
    ezspDebugPrintf("\r\n");
  }
}

void ashTraceDisconnected(uint8_t error)
{
  if (sli_ash_host_get_config(traceFlags) & TRACE_EVENTS) {
    sl_zigbee_ezsp_print_elapsed_time();
    ezspDebugPrintf("ASH disconnected: %s\r\n", ezspErrorString(error));
    ezspDebugPrintf("    NCP status: %s\r\n", ezspErrorString(ncpError));
    sl_zigbee_ezsp_debug_flush();
  }
}

void ashTraceArray(uint8_t *name, uint8_t len, uint8_t *data)
{
  if (sli_ash_host_get_config(traceFlags) & TRACE_EVENTS) {
    ezspDebugPrintf("%s ", name);
    while (len--) {
      ezspDebugPrintf(" %02X", *data++);
    }
    ezspDebugPrintf("\r\n");
  }
}
