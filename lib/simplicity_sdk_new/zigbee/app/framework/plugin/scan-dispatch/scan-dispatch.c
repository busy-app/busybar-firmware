/***************************************************************************//**
 * @file
 * @brief Dispatching 802.15.4 scan results to interested parties.
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
#ifdef SL_CATALOG_CLI_PRESENT
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#endif // SL_CATALOG_CLI_PRESENT
#include "scan-dispatch.h"

// -----------------------------------------------------------------------------
// Global variables

static sl_zigbee_af_event_t scanNetworkEvents[SL_ZIGBEE_SUPPORTED_NETWORKS];
static void scanNetworkEventHandler(sl_zigbee_af_event_t * event);

static bool cliInitiatedScan = false;

// -----------------------------------------------------------------------------
// Internal implementation elements

static uint8_t head = 0;
static uint8_t tail = 0;
static uint8_t count = 0;
static sl_zigbee_af_plugin_scan_dispatch_scan_data_t handlerQueue[SL_ZIGBEE_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE];
#define handlerQueueIsEmpty() (!count)
#define handlerQueueIsFull()  (count == SL_ZIGBEE_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE)

// We save 12 bytes of text by having this as a function.
static uint8_t handlerQueueNextIndex(uint8_t i)
{
  return (++i % SL_ZIGBEE_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE);
}

static void maybeCallNextHandler(uint8_t statusOrRssi,
                                 uint8_t channelOrLqi,
                                 sl_zigbee_network_scan_type_t scanType,
                                 bool isComplete,
                                 bool isFailure,
                                 sl_zigbee_zigbee_network_t *network)
{
  if (!handlerQueueIsEmpty()) {
    sl_zigbee_af_plugin_scan_dispatch_scan_results_t results = {
      .status = statusOrRssi,
      .rssi = statusOrRssi,
      .channel = channelOrLqi,
      .lqi = channelOrLqi,
      .network = network,
      .mask = (scanType
               | (isComplete
                  ? EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_COMPLETE
                  : 0)
               | (isFailure
                  ? EM_AF_PLUGIN_SCAN_DISPATCH_SCAN_RESULTS_MASK_FAILURE
                  : 0)),
    };
    handlerQueue[head].handler(&results);
  }
}

// -----------------------------------------------------------------------------
// Public API

sl_status_t sl_zigbee_af_scan_dispatch_schedule_scan(sl_zigbee_af_plugin_scan_dispatch_scan_data_t *data)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (!handlerQueueIsFull()) {
    handlerQueue[tail] = *data;
    tail = handlerQueueNextIndex(tail);
    count++;
    status = SL_STATUS_OK;
    sl_zigbee_af_event_set_active(scanNetworkEvents);
  }

  return status;
}

void sl_zigbee_af_scan_dispatch_clear(void)
{
  sl_zigbee_stop_scan();
  head = tail = count = 0;
}

// -----------------------------------------------------------------------------
// Internal and public callbacks

void sli_zigbee_af_scan_dispatch_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_network_event_init(scanNetworkEvents,
                                  scanNetworkEventHandler);
}

static void scanNetworkEventHandler(sl_zigbee_af_event_t * event)
{
  (void)event;
  sl_status_t status;

  sl_zigbee_af_event_set_inactive(scanNetworkEvents);

  // If there is a handler in the queue, start a scan for it.
  // If we are already scanning, we should try again.
  // If there is an error, we abort the scan and tell the consumer.
  if (!handlerQueueIsEmpty()) {
    status = sl_zigbee_start_scan(handlerQueue[head].scanType,
                                  handlerQueue[head].channelMask,
                                  handlerQueue[head].duration);

    if (status != SL_STATUS_MAC_SCANNING) {
      if (status != SL_STATUS_OK) {
        maybeCallNextHandler(status,
                             0,     // channel. 0 indicating that the actual scan was not started
                             handlerQueue[head].scanType,
                             true,     // complete?
                             true,     // failure?
                             NULL);     // network
        head = handlerQueueNextIndex(head);
        count--;
        sl_zigbee_af_event_set_active(scanNetworkEvents);
      }
    }
  }
}

void sli_zigbee_af_scan_dispatch_energy_scan_result_callback(uint8_t channel, int8_t rssi)
{
  maybeCallNextHandler((uint8_t)rssi,
                       channel,
                       SL_ZIGBEE_ENERGY_SCAN,
                       false,     // complete?
                       false,     // failure?
                       NULL);     // network
  if (cliInitiatedScan) {
    sl_zigbee_af_core_println("Energy scan result: channel %d, RSSI %d dBm",
                              channel, rssi);
  }
}

void sli_zigbee_af_scan_dispatch_network_found_callback(sl_zigbee_zigbee_network_t *networkFound,
                                                        uint8_t lqi,
                                                        int8_t rssi)
{
  maybeCallNextHandler((uint8_t)rssi,
                       lqi,
                       SL_ZIGBEE_ACTIVE_SCAN,
                       false,     // complete?
                       false,     // failure?
                       networkFound);
}

void sli_zigbee_af_scan_dispatch_scan_complete_callback(uint8_t channel, sl_status_t status)
{
  if (!handlerQueueIsEmpty()) {
      maybeCallNextHandler(status,
                          channel,
                          handlerQueue[head].scanType,
                          true,     // complete?
                          false,     // failure?
                          NULL);     // network
  }

  // The scan is done when the status is set to SL_STATUS_OK.
  // See documentation for the sl_zigbee_scan_complete_handler callback.
  // For Active scan we call the EMBER_ACTIVE_SCAN_XX anyways,
  // beacuse of the change in network-steering code which
  // continues to schedule a scan on the next channel after a failure

  if (cliInitiatedScan) {
    if (status == SL_STATUS_OK) {
      sl_zigbee_af_core_println("Energy scan complete");
    }
    cliInitiatedScan = false;  // Reset CLI flag
  }
  
  if ((status == SL_STATUS_OK) && !handlerQueueIsEmpty()) {
    head = handlerQueueNextIndex(head);
    count--;
    sl_zigbee_af_event_set_active(scanNetworkEvents);
  }
}

#ifdef SL_CATALOG_CLI_PRESENT
void sli_zigbee_af_scan_dispatch_energy_scan_cli_command(sl_cli_command_arg_t *arguments)
{
  uint32_t channelMask = sl_cli_get_argument_uint32(arguments, 0);
  uint8_t scanDuration = sl_cli_get_argument_uint8(arguments, 1);
  sl_status_t status = sl_zigbee_start_scan(SL_ZIGBEE_ENERGY_SCAN,
                                            channelMask,
                                            scanDuration);
  if (status == SL_STATUS_OK) {
    cliInitiatedScan = true;
    sl_zigbee_af_core_println("Energy scan started on channel mask 0x%08X, duration %d",
                              channelMask,
                              scanDuration);
  } else {
    sl_zigbee_af_core_println("Energy scan failed to start: 0x%08X", status);
  }
}
#endif // SL_CATALOG_CLI_PRESENT
