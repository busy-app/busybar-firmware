/***************************************************************************//**
 * @file
 * @brief Gateway specific behavior for a host application.
 *   In this case we assume our application is running on
 *   a PC with Unix library support, connected to an NCP via serial uart.
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
#include "app/ezsp-host/ezsp-host-common.h"
#include "app/ezsp-host/ezsp-host-io.h"
#include "app/ezsp-host/ezsp-host-ui.h"

#include "app/framework/util/af-event.h"
#include "app/util/ezsp/serial-interface.h"
#include "app/util/serial/linux-serial.h"
#include "gateway-config.h"

#include "app/framework/plugin-host/file-descriptor-dispatch/file-descriptor-dispatch.h"

#include <sys/time.h>   // for select()
#include <sys/types.h>  // ""
#include <unistd.h>     // ""
#include <errno.h>      // ""
#include <stdio.h>

//------------------------------------------------------------------------------
// Globals

// If the application wishes to limit how long the select() call will yield
// for, they can do it by specifying a max timeout.  This may be necessary
// if the main() loop expects to be serviced at some regular interval.
// Ideally the application code can use an event, but it is easier to
// tune it this way.  0xFFFFFFFFUL = no read timeout, thus allowing the
// select() call to yield forever if there are no events scheduled.
#define MAX_READ_TIMEOUT_MS  SL_ZIGBEE_AF_PLUGIN_GATEWAY_MAX_WAIT_FOR_EVENT_TIMEOUT_MS
#define MAX_FDS              SL_ZIGBEE_AF_PLUGIN_GATEWAY_MAX_FDS
#define INVALID_FD           -1

static const char* debugLabel = "gateway-debug";
static const bool debugOn = false;

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------
// Forward Declarations
int sl_zigbee_af_gateway_select_file_descriptors_cb(int*list, int maxSize);

static void debugPrint(const char* formatString, ...);
static void ezspFileDescriptorReadyCallback(void* data, sl_zigbee_af_file_descriptor_operation_t operation);
static void ezspSerialPortCallback(EzspSerialPortEvent event, int fileDescriptor);

//------------------------------------------------------------------------------
// Functions

bool sl_zigbee_af_main_start_cb(int* returnCode,
                                int argc,
                                char** argv)
{
  debugPrint("gatewaitInit()");

  // This will process EZSP command-line options
  if (!sl_zigbee_ezsp_process_command_options(argc, argv)) {
    *returnCode = SL_STATUS_FAIL;
    return true;
  }

  return false;
}

void sl_zigbee_af_gateway_tick_cb(void)
{
  // If the CLI process is waiting for the 'go-ahead' to prompt the user
  // and read input, we need to tell it to do that before going to sleep
  // (potentially indefinitely) via select().
#ifndef EZSP_HOST
  sli_legacy_serial_send_ready_to_read(APP_SERIAL);
#endif

  uint32_t msToNextEvent = sl_zigbee_af_ms_to_next_event(0xFFFFFFFFUL);
  msToNextEvent = (msToNextEvent > MAX_READ_TIMEOUT_MS
                   ? MAX_READ_TIMEOUT_MS
                   : msToNextEvent);

  sl_zigbee_af_file_descriptor_dispatch_wait_for_events(msToNextEvent);
}

void sl_zigbee_af_gateway_init_cb(uint8_t init_level)
{
  UNUSED_VAR(init_level);

  int fdList[MAX_FDS];
  int count = 0;
  int i;

  sl_zigbee_af_file_descriptor_dispatch_struct_t dispatchStruct = {
    NULL, // callback
    NULL, // data passed to callback
    SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_READ,
    -1,
  };
  dispatchStruct.fileDescriptor = sli_legacy_serial_get_input_fd(0);
  if (dispatchStruct.fileDescriptor != -1
      && SL_STATUS_OK != sl_zigbee_af_file_descriptor_dispatch_add(&dispatchStruct)) {
    sl_zigbee_af_core_println("Error: Gateway Plugin failed to register serial Port 0 FD");
  }
  dispatchStruct.fileDescriptor = sli_legacy_serial_get_input_fd(1);
#if defined(ZA_CLI_FULL)
  if (dispatchStruct.fileDescriptor != -1
      && SL_STATUS_OK != sl_zigbee_af_file_descriptor_dispatch_add(&dispatchStruct)) {
    sl_zigbee_af_core_println("Error: Gateway Plugin failed to register serial Port 1 FD");
  }
#endif

#ifdef EZSP_SPI
  // For SPI, we need the nHOST_INT line as well
  sl_zigbee_af_file_descriptor_dispatch_struct_t spiDispatchStruct = {
    NULL, // callback
    NULL, // data passed to callback
    SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_EXCEPT,
    -1,
  };
  spiDispatchStruct.fileDescriptor = serialGetSpiFd();
  if (spiDispatchStruct.fileDescriptor != -1
      && SL_STATUS_OK != sl_zigbee_af_file_descriptor_dispatch_add(&spiDispatchStruct)) {
    sl_zigbee_af_core_println("Error: Gateway Plugin failed to register SPI FD");
  }
#endif // EZSP_SPI

  sl_zigbee_ezsp_serial_port_register_cb(ezspSerialPortCallback);
  if (sl_zigbee_ezsp_serial_get_fd() != NULL_FILE_DESCRIPTOR) {
    ezspSerialPortCallback(EZSP_SERIAL_PORT_OPENED, sl_zigbee_ezsp_serial_get_fd());
  }

  memset(fdList, 0xFF, sizeof(int) * MAX_FDS);
  count = sl_zigbee_af_gateway_select_file_descriptors_cb(fdList, MAX_FDS);
  for (i = 0; i < count; i++) {
    dispatchStruct.fileDescriptor = fdList[i];
    if (SL_STATUS_OK != sl_zigbee_af_file_descriptor_dispatch_add(&dispatchStruct)) {
      sl_zigbee_af_core_println("Error: Gateway plugin failed to add FD %d for watching.", fdList[i]);
    }
  }
}

static void debugPrint(const char* formatString, ...)
{
  if (debugOn) {
    va_list ap;
    fprintf(stderr, "[%s] ", debugLabel);
    va_start(ap, formatString);
    vfprintf(stderr, formatString, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    fflush(stderr);
  }
}

// EMZIGBEE-2126 case where host/NCP USB disconnect causes host CPU spike.
// Withdraw EZSP file descriptor from select polling after 10 consecutive
// "ready" polls with no data actually available.
#define MAX_CONSECUTIVE_TIMES_NO_EZSP_DATA (10)
static uint8_t consecutiveTimesEzspNoData = 0;
static void ezspFileDescriptorReadyCallback(void* data, sl_zigbee_af_file_descriptor_operation_t operation)
{
  if (operation == SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_READ) {
    uint16_t count = 0;
    if (sl_zigbee_ezsp_serial_read_available(&count) == SL_ZIGBEE_EZSP_SUCCESS && count > 0) {
      consecutiveTimesEzspNoData = 0; // reset
    } else {
      ++consecutiveTimesEzspNoData;
    }
    if (consecutiveTimesEzspNoData >= MAX_CONSECUTIVE_TIMES_NO_EZSP_DATA) {
      // Assume EZSP connection is broken, withdraw file descriptor (data)
      sl_zigbee_af_file_descriptor_dispatch_remove((intptr_t) data);
    }
  }
}

static void ezspSerialPortCallback(EzspSerialPortEvent event, int fileDescriptor)
{
  assert(fileDescriptor != NULL_FILE_DESCRIPTOR);

  if (event == EZSP_SERIAL_PORT_CLOSED) {
    debugPrint("EZSP serial port closed.  FD=%d", fileDescriptor);
    sl_zigbee_af_file_descriptor_dispatch_remove(fileDescriptor);
  } else if (event == EZSP_SERIAL_PORT_OPENED) {
    sl_zigbee_af_file_descriptor_dispatch_struct_t dispatchStruct = {
      ezspFileDescriptorReadyCallback,   // callback
      (void*)(intptr_t)fileDescriptor,   // data passed to callback
      SL_ZIGBEE_AF_FILE_DESCRIPTOR_OPERATION_READ,
      fileDescriptor,
    };

    debugPrint("Registered EZSP FD %d", fileDescriptor);
    if (SL_STATUS_OK != sl_zigbee_af_file_descriptor_dispatch_add(&dispatchStruct)) {
      sl_zigbee_af_core_println("Error: Gateway Plugin failed to register EZSP FD %d", fileDescriptor);
    }
  }
}
