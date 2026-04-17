/***************************************************************************//**
 * @file: multirail-demo.c
 * @brief Routines to utilise a second instance of RAIL.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "stack/include/sl_zigbee_types.h"
#include "stack/include/stack-info.h"

#include "sl_rail_ieee802154.h"
#include "sl_rail_util_compatible_pa.h"

#include "multirail-demo.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

// Local callback prototype, used in railCfgLocal
static void multirailCbGeneric(sl_rail_handle_t handle, sl_rail_events_t events);

// Plugin local data
static sl_rail_handle_t rail2Handle = NULL;
static bool rfReady = false;

// Default RAIL configuration, used if the caller has not provided their own.
// It must be module global (i.e. 'static', not 'auto') and not const.
static sl_rail_config_t railCfgLocal = {
  .events_callback = &multirailCbGeneric,
};

// The default FIFO, used unless the caller provides their own
// This buffer is required to be RAIL_FIFO_ALIGNMENT aligned,
// which is 4 bytes for Series 2
ALIGNMENT(sizeof(sl_rail_fifo_buffer_align_t))
static uint8_t railTxFifoLocal[128];

// Local functions
static void multirailCbGeneric(sl_rail_handle_t handle, sl_rail_events_t events)
{
  // Alert the user...
  sl_zigbee_af_multirail_demo_rail_event_cb(handle, events);
  // ...and yield.
  if (events & (SL_RAIL_EVENT_TX_PACKET_SENT
                | SL_RAIL_EVENT_TX_ABORTED
                | SL_RAIL_EVENT_TX_UNDERFLOW
                | SL_RAIL_EVENT_SCHEDULER_STATUS)) {
    sl_rail_yield_radio(handle);
  }
}

// Called from sl_rail_init to indicate a new RAIL instance is ready
static void multirailCbRfReady(sl_rail_handle_t handle)
{
  UNUSED_VAR(handle);
  rfReady = true;
}

/** Return the second RAIL handle. Null if not initialized.
 */
sl_rail_handle_t sl_zigbee_af_multirail_demo_get_handle(void)
{
  return rail2Handle;
}

/** Send a packet using the second RAIL instance.
 *
 * buff .... Pointer to the data (make sue it is properly formatted)
 * size .... Size of the data
 * channel . Channel (11-26, or 0 to use the current Zigbee channel)
 * scheduledTxConfig (optional)
 * schedulerInfo (optional) .. Scheduler configuration (NULL to use defaults)
 *
 * Returns a RAIL status value.
 */
sl_rail_status_t sl_zigbee_af_multirail_demo_send(const uint8_t *buff,
                                                  uint32_t size,
                                                  uint8_t channel,
                                                  sl_rail_scheduled_tx_config_t *scheduledTxConfig,
                                                  sl_rail_scheduler_info_t *schedulerInfo)
{
  sl_rail_scheduled_tx_config_t scheduledTxConfigLocal;
  sl_rail_scheduler_info_t schedulerInfoLocal;

  // Sanity check
  if (!rail2Handle) {
    return SL_RAIL_STATUS_INVALID_STATE;
  }

  if (!scheduledTxConfig) {
    // Use the default schedule to send it as soon as possible.
    scheduledTxConfigLocal.when = 0;
    scheduledTxConfigLocal.mode = SL_RAIL_TIME_DELAY;
    scheduledTxConfig = &scheduledTxConfigLocal;
  }

  if (!schedulerInfo) {
    // This assumes the Tx time is around 10 ms but should be tweaked based
    // on the specific PHY configuration.
    schedulerInfoLocal.priority = 100;
    schedulerInfoLocal.slip_time = 50000;
    schedulerInfoLocal.transaction_time = 10000;
    schedulerInfo = &schedulerInfoLocal;
  }

  if (channel == 0) {
    channel = sl_zigbee_get_radio_channel();
  }

  // Load the transmit buffer with something to send.
  // If success, transmit this packet at the specified time.
  if (sl_rail_write_tx_fifo(rail2Handle, buff, size, true) != size) {
    return SL_RAIL_STATUS_INVALID_CALL;
  } else {
    return sl_rail_start_scheduled_tx(rail2Handle,
                                      channel,
                                      SL_RAIL_TX_OPTIONS_DEFAULT,
                                      scheduledTxConfig,
                                      schedulerInfo);
  }
}

/** Initialize a new RAIL handle.
 *
 * railCfg ........ RAIL init options (NULL to use the current Zigbee channel config)
 * txPowerConfig .. RAIL power options (NULL to use defaults)
 * paAutoMode ..... Whether an automatic PA mode is enabled
 * defaultTxPower . Default TX power in deci-dBm
 * txFifo ......... Pointer to a 32 bit aligned TX FIFO buffer (NULL to use the default)
 * txFifoSize ..... Size of the TX buffer
 * panId .......... PAN ID (use 0xFFFF if not needed)
 * ieeeAddr ....... Long IEEE address (use NULL if not needed)
 *
 * Returns a new RAIL handle, NULL if not successfully initialized.
 */
sl_rail_handle_t sl_zigbee_af_multirail_demo_init(sl_rail_config_t *railCfg,
                                                  sl_rail_tx_power_config_t *txPowerConfig,
                                                  bool paAutoMode,
                                                  sl_rail_tx_power_t defaultTxPower,
                                                  uint8_t *txFifo,
                                                  uint16_t txFifoSize,
                                                  uint16_t panId,
                                                  const uint8_t *ieeeAddr)
{
  UNUSED_VAR(paAutoMode);
  UNUSED_VAR(txPowerConfig);

  // Sanity check
  if (rail2Handle) {
    return rail2Handle;
  }

// In case it's a Dynamic Multi-Protocol Application (DMP),
// additional RAM buffer(s) for other RAIL instances needs to be allocated.
#if defined SL_CATALOG_BLUETOOTH_PRESENT && defined SL_CATALOG_OPENTHREAD_STACK_PRESENT
  sl_rail_add_state_buffer_3(SL_RAIL_EFR32_HANDLE);
  sl_rail_add_state_buffer_4(SL_RAIL_EFR32_HANDLE);
#elif defined SL_CATALOG_BLUETOOTH_PRESENT || defined SL_CATALOG_OPENTHREAD_STACK_PRESENT
  sl_rail_add_state_buffer_3(SL_RAIL_EFR32_HANDLE);
#endif

  sl_rail_handle_t handle = SL_RAIL_EFR32_HANDLE;
  sl_rail_status_t status = sl_rail_init(&handle, railCfg ? railCfg : &railCfgLocal,
                                         multirailCbRfReady);
  if (status != SL_RAIL_STATUS_NO_ERROR) {
    return NULL;
  }

  // Initialise the PA now that the HFXO is up and the timing is correct
  if (sl_rail_util_pa_post_init(handle, SL_RAIL_TX_PA_MODE_2P4_GHZ) != SL_RAIL_STATUS_NO_ERROR) {
    // Error: The PA could not be initialised due to an improper configuration.
    // Please ensure your configuration is valid for the selected part.
    return NULL;
  }

  // Set TX FIFO, and verify that the size is correct
  if (!txFifo) {
    txFifo = railTxFifoLocal;
    txFifoSize = sizeof railTxFifoLocal;
  }
  if (sl_rail_set_tx_fifo(handle, (sl_rail_fifo_buffer_align_t*)txFifo, txFifoSize, 0, 0) != SL_RAIL_STATUS_NO_ERROR) {
    return NULL;
  }

  // Initialise Radio Calibrations
  if (sl_rail_config_cal(handle, SL_RAIL_CAL_ALL) != SL_RAIL_STATUS_NO_ERROR) {
    return NULL;
  }

  // Configure all RAIL events with appended info
  sl_rail_events_t events = SL_RAIL_EVENT_TX_PACKET_SENT
                            | SL_RAIL_EVENT_TX_ABORTED
                            | SL_RAIL_EVENT_TX_UNDERFLOW
                            | SL_RAIL_EVENT_SCHEDULER_STATUS
                            | SL_RAIL_EVENT_RX_PACKET_RECEIVED
                            | SL_RAIL_EVENT_RX_FRAME_ERROR // invalid CRC
                            | SL_RAIL_EVENT_RX_ADDRESS_FILTERED;
  if (sl_rail_config_events(handle, SL_RAIL_EVENTS_ALL, events) != SL_RAIL_STATUS_NO_ERROR) {
    return NULL;
  }

  sl_rail_state_transitions_t transitions = {
    .success = SL_RAIL_RF_STATE_IDLE,
    .error = SL_RAIL_RF_STATE_IDLE
  };
  if (sl_rail_set_rx_transitions(handle, &transitions) != SL_RAIL_STATUS_NO_ERROR) {
    return NULL;
  }

  // Initialise IEEE
  if (!sl_rail_ieee802154_is_enabled(handle)) {
    //This initialises the radio for 2.4GHz operation.
    if (sl_rail_ieee802154_config_2p4_ghz_radio(handle) != SL_RAIL_STATUS_NO_ERROR) {
      return NULL;
    }

    sl_rail_state_timing_t timings = {
      .idle_to_tx = 100,
      .idle_to_rx = 100,
      .rx_to_tx = 192,
      // Make txToRx slightly lower than desired to make sure we get to
      // RX in time
      .tx_to_rx = 192 - 10,
      .rxsearch_timeout = 0,
      .tx_to_rxsearch_timeout = 0
    };

    sl_rail_ieee802154_config_t rail802154_init = {
      .p_addresses = NULL,
      .ack_config = {
        .enable = false,
        .ack_timeout_us = 0,
        .rx_transitions = {
          .success = SL_RAIL_RF_STATE_IDLE,
          .error = SL_RAIL_RF_STATE_IDLE // ignored
        },
        .tx_transitions = {
          .success = SL_RAIL_RF_STATE_IDLE,
          .error = SL_RAIL_RF_STATE_IDLE // ignored
        }
      },
      .timings = timings,
      .frames_mask = SL_RAIL_IEEE802154_ACCEPT_STANDARD_FRAMES
                     | SL_RAIL_IEEE802154_ACCEPT_ACK_FRAMES
                     | SL_RAIL_IEEE802154_ACCEPT_DATA_FRAMES
                     | SL_RAIL_IEEE802154_ACCEPT_COMMAND_FRAMES
                     | SL_RAIL_IEEE802154_ACCEPT_BEACON_FRAMES,
      .promiscuous_mode = true,
      .is_pan_coordinator = false
    };

    if (sl_rail_ieee802154_init(handle, &rail802154_init) != SL_RAIL_STATUS_NO_ERROR) {
      return NULL;
    }

    // Set the PAN ID
    if (sl_rail_ieee802154_set_pan_id(handle, panId, 0x00) != SL_RAIL_STATUS_NO_ERROR) {
      return NULL;
    }

    // Set Long address, if the address is provided.
    if (ieeeAddr
        && sl_rail_ieee802154_set_long_address(handle, ieeeAddr, 0x00) != SL_RAIL_STATUS_NO_ERROR) {
      return NULL;
    }
  }

  // Wait until the intialisation is complete (flag set by a sl_rail_init callback)
  while (!rfReady) ;

  // Set default TX power in deci-dBm
  if (sl_rail_set_tx_power_dbm(handle, defaultTxPower) != SL_RAIL_STATUS_NO_ERROR) {
    return NULL;
  }

  // Cache and return the new RAIL handle
  rail2Handle = handle;
  return handle;
}

WEAK(void sl_zigbee_multirail_gp_tx_queue_init(void))
{
}
