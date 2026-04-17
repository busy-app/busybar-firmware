/***************************************************************************//**
 * @file sl_rail_mux.c
 * @brief RAIL Multiplexer implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_COEX_SUPPORTED
#define SLI_COEX_SUPPORTED 1
#endif // SLI_COEX_SUPPORTED

#ifndef SLI_ANTDIV_SUPPORTED
#define SLI_ANTDIV_SUPPORTED 1
#endif // SLI_ANTDIV_SUPPORTED

#define SLI_DMP_TUNING_SUPPORTED SLI_COEX_SUPPORTED // DMP_TUNING requires COEX

#include "sl_code_classification.h"

#include "sl_rail.h"
#include "sl_rail_ieee802154.h"

#include "sl_rail_util_ieee802154/sl_rail_util_ieee802154_phy_select.h"
#ifdef SL_RAIL_UTIL_PA_CONFIG_HEADER
#include SL_RAIL_UTIL_PA_CONFIG_HEADER
#endif
#include "sl_common.h" // for SL_WEAK
#include "sl_rail_util_compatible_pa.h"

#include "sl_rail_mux.h"
#include "mac-flat-header.h"
#include "buffer_manager/buffer-management.h"
#include "buffer_manager/buffer-queue.h"
#include "sl_assert.h" // for EFM_ASSERT
#include "mfglib_modes.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) || defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)
#include "sl_rail_util_ieee802154_fast_channel_switching.h"
#include "sl_rail_util_ieee802154_fast_channel_switching_config.h"

// This file supports 2 instances of stacks (ZB, OT). Similar configuration is expected on RAIL side
// when concurrent Rx feature is enabled
#if SUPPORTED_PROTOCOL_COUNT != SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS
#error "SL RAIL MUX: SUPPORTED_PROTOCOL_COUNT is different from SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS"
#endif //SUPPORTED_PROTOCOL_COUNT != SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_NUM_CHANNELS

static sl_rail_ieee802154_rx_channel_switching_cfg_t channel_switching_cfg;
static SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT_TYPE channel_switching_buf[SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES / SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_ALIGNMENT];

// Fast channel switching or concurrent listening requires the following:
// 1. Load fast channel switching PHY (similar to antenna diversity PHY) - done in the component init
// 2. Setup config buffers and enable corresponding rx option - done in this file
// This handy function checks the OT and ZB configurations to determine if we need to turn on the rx option
// When this feature is enabled, the ZB and OT networks are allowed to be on different channels
// Without this feature, the multiplexer will ONLY work if the two stacks use the same channel and this needs to be manually enforced
static bool sli_is_multi_channel_enabled(void)
{
  uint16_t firstChannel = INVALID_CHANNEL;
  for (uint8_t i = 0U; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (channel_switching_cfg.channels[i] != INVALID_CHANNEL) {
      if (firstChannel == INVALID_CHANNEL) {
        firstChannel = channel_switching_cfg.channels[i];
      } else if (firstChannel != channel_switching_cfg.channels[i]) {
        return true;
      }
    }
  }
  return false;
}

static inline void SET_CHANNEL_SWITCHING_CFG_CH(uint8_t context_index, uint16_t channel)
{
  channel_switching_cfg.channels[context_index] = channel;
}

// This macro is called prior to sl_rail_start_rx to enable or disable Rx option and setup ch switching configuration
// Some of these settings only take effect when radio is idle
static inline void CONFIGURE_RX_CHANNEL_SWITCHING(sl_rail_handle_t mux_rail_handle, sl_rail_ieee802154_rx_channel_switching_cfg_t channel_switching_cfg)
{
  sl_rail_idle(mux_rail_handle, SL_RAIL_IDLE, true);
  sl_rail_status_t status = sl_rail_util_ieee802154_config_radio(mux_rail_handle);
  assert(status == SL_RAIL_STATUS_NO_ERROR);
  //this checks if stacks are actually on 2 different channels regardless of fcs being enabled
  if (sli_is_multi_channel_enabled()) {
    status = sl_rail_ieee802154_config_rx_channel_switching(mux_rail_handle, &channel_switching_cfg);
    assert(status == SL_RAIL_STATUS_NO_ERROR);
    status = sl_rail_config_rx_options(mux_rail_handle, SL_RAIL_RX_OPTION_CHANNEL_SWITCHING, SL_RAIL_RX_OPTION_CHANNEL_SWITCHING);
    assert(status == SL_RAIL_STATUS_NO_ERROR);
  } else {
    sl_rail_config_rx_options(mux_rail_handle, SL_RAIL_RX_OPTION_CHANNEL_SWITCHING, SL_RAIL_RX_OPTIONS_NONE);
  }
}
#define CHANNEL_CHECK   channel_switching_cfg.channels[i]
#else // STUBS FOR OPERATING WITHOUT FAST CHANNEL SWITCHING FEATURE
#define CHANNEL_CHECK   rx_channel
#define CONFIGURE_RX_CHANNEL_SWITCHING(mux_rail_handle, channel_switching_cfg)
#define SET_CHANNEL_SWITCHING_CFG_CH(context_index, channel)
#define sli_is_multi_channel_enabled() false
#endif //#if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) || defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)

//------------------------------------------------------------------------------
// Forward declarations
extern uint8_t sli_legacy_mfglib_mode;

static void fn_set_tx_power_for_context(uint8_t context_index);
static void fn_set_tx_power_for_context(uint8_t context_index);
static void fn_set_global_flag(uint16_t flag, bool val);
static bool fn_get_global_flag(uint16_t flag);
static uint8_t fn_get_context_index(sl_rail_handle_t handle);
static void fn_set_context_flag_by_index(uint8_t index, uint16_t flag, bool val);
static bool fn_get_context_flag_by_index(uint8_t index, uint16_t flag);
static void fn_init_802154_address_config(sl_rail_ieee802154_addr_config_t *addr_config);
static void fn_update_802154_address_filtering_table(void);
static sl_rail_status_t fn_start_pending_tx(void);
static bool fn_operation_in_progress(uint16_t operation_flags);
static uint8_t fn_get_active_tx_context_index(void);

HIDDEN void fn_mux_rail_init_callback(sl_rail_handle_t railHandle);
HIDDEN void fn_mux_rail_events_callback(sl_rail_handle_t railHandle, sl_rail_events_t events);
HIDDEN void fn_timer_callback(struct sl_rail_multi_timer *tmr,
                              sl_rail_time_t expectedTimeOfEvent,
                              void *cbArg);

#define tx_in_progress()                                          \
  fn_operation_in_progress(RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS \
                           | RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK)

// ( no lock is active || lock acquired by current context_index )
#define check_lock_permissions(context_index)                      \
  ((!fn_operation_in_progress(RAIL_MUX_PROTOCOL_FLAGS_LOCK_ACTIVE) \
    || fn_get_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_LOCK_ACTIVE)) )
//------------------------------------------------------------------------------
// Globals

sl_rail_handle_t emPhyRailHandle = NULL;
#define mux_rail_handle emPhyRailHandle

static volatile uint16_t internal_flags = 0;
static sl_rail_tx_power_t current_tx_power = SL_RAIL_TX_POWER_MIN;
static uint8_t pending_power_context = INVALID_CONTEXT_INDEX;
static uint16_t rx_channel = INVALID_CHANNEL;
static sl_rail_ieee802154_addr_config_t rail_addresses_802154;

HIDDEN sl_rail_mux_context_t protocol_context[SUPPORTED_PROTOCOL_COUNT];

HIDDEN sl_rail_config_t mux_rail_config = {
  .events_callback = fn_mux_rail_events_callback,
};
#ifdef SL_CATALOG_HIGH_DATARATE_PHY_PRESENT
extern uint16_t sli_mac_max_rx_flat_pkt_size;
extern uint32_t *sli_phy_rx_fifo;
#endif
HIDDEN const sl_rail_ieee802154_config_t ieee_802154_config = {
  NULL, // addresses
  {
    // ack_config
    true, // ack_config.enable
    672,  // ack_config.ack_timeout
    {
      // ack_config.rx_transitions.
      SL_RAIL_RF_STATE_RX,   // ack_config.rx_transitions.success
      SL_RAIL_RF_STATE_RX,   // ack_config.rx_transitions.error
    },
    {
      // ack_config.tx_transitions
      SL_RAIL_RF_STATE_RX,   // ack_config.tx_transitions.success
      SL_RAIL_RF_STATE_RX,   // ack_config.tx_transitions.error
    },
  },
  {
    // timings
    100,      // timings.idle_to_rx
    192 - 10, // timings.tx_to_rx
    100,      // timings.idle_to_tx
    256,      // timings.rx_to_tx - This is set to 256 to accommodate enhanced
              // ACKs (part of OT >= 1.2), regular value would 192.
    0,        // timings.rxsearch_timeout
    0,        // timings.tx_to_rxsearch_timeout
    0,        // timings.tx_to_tx
  },
  SL_RAIL_IEEE802154_ACCEPT_STANDARD_FRAMES, // frames_mask
  false,                                  // promiscuous_mode
  false,                                  // is_pan_coordinator
  false,                                  // default_frame_pending_in_outgoing_acks
};

SL_WEAK void sl_rail_mux_invalid_rx_channel_detected_cb(int new_rx_channel, int old_rx_channel)
{
  (void)new_rx_channel;
  (void)old_rx_channel;
  //if(network up)
  EFM_ASSERT(false);
}

//------------------------------------------------------------------------------
// Internal APIs

void sli_rail_mux_local_init(void)
{
  uint8_t i;

  // We use the RAIL config pointer to indicate whether an entry is already in
  // use or not.
  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    protocol_context[i].rail_config = NULL;
    protocol_context[i].events = 0;
    protocol_context[i].flags = 0;
    protocol_context[i].fifo_tx_info.tx_init_length = 0;
    protocol_context[i].fifo_tx_info.tx_size = 0;

    protocol_context[i].channel = INVALID_CHANNEL;
    protocol_context[i].tx_repeat_config.iterations = 0;
    protocol_context[i].tx_power = SL_RAIL_TX_POWER_MIN;
    // 802.15.4 specific fields
    fn_init_802154_address_config(&protocol_context[i].addr_802154);
    protocol_context[i].is_pan_coordinator_802154 = false;
    // Initialize to address broadcast and PAN broadcast
    protocol_context[i].addr_filter_mask_802154 = RAIL_MUX_FILTERING_MASK_BROADCAST_ENABLED;
  }

  internal_flags = 0;
  current_tx_power = SL_RAIL_TX_POWER_MIN;
  pending_power_context = INVALID_CONTEXT_INDEX;
  rx_channel = INVALID_CHANNEL;
  fn_init_802154_address_config(&rail_addresses_802154);

#if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) || defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)

  channel_switching_cfg.buffer_bytes = SL_RAIL_IEEE802154_RX_CHANNEL_SWITCHING_BUF_BYTES;
  channel_switching_cfg.p_buffer      = channel_switching_buf;
  for (uint8_t i = 0U; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    channel_switching_cfg.channels[i] = INVALID_CHANNEL;
  }
#endif //#if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) || defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)

  // TODO: for now we assume all protocols to be 802.15.4 and use the 2.4 OQPSK
  // standard PHY. In order to support SubGHz PHY, we will need to modify the
  // rail_config accordingly.
}

//------------------------------------------------------------------------------
// Public APIs

sl_rail_status_t sl_rail_mux_Init(sl_rail_handle_t *p_rail_handle,
                                  sl_rail_config_t *p_rail_config,
                                  sl_rail_init_complete_callback_t init_complete_callback)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  uint8_t i;

  RAIL_MUX_ENTER_CRITICAL();

  // Find an unused protocol context entry. Return a NULL handler if an entry
  // is not available. If we find an entry containing the same railCfg, it means
  // that the protocol called sl_rail_init() multiple times, in this case we just
  // return the RAIL handle.
  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (protocol_context[i].rail_config == p_rail_config) {
      *p_rail_handle = (sl_rail_handle_t )&(protocol_context[i]);
      return SL_RAIL_STATUS_NO_ERROR;
    }

    if (protocol_context[i].rail_config == NULL) {
      break;
    }
  }

  if (i >= SUPPORTED_PROTOCOL_COUNT) {
    return SL_RAIL_STATUS_INVALID_CALL;
  }

  protocol_context[i].rail_config = p_rail_config;
  protocol_context[i].init_callback = init_complete_callback;
  fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_INIT_CB_PENDING, true);

  // TODO: confirm with the RAIL team that the init callback *always* gets
  // called before sl_rail_init() returns. If that is the case, this logic can be
  // much simpler.

  // First call to sl_rail_mux_Init(), we need to call sl_rail_init().
  if (!fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_STARTED)) {
    fn_set_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_STARTED, true);
    mux_rail_config.rx_packet_queue_entries = p_rail_config->rx_packet_queue_entries;
    mux_rail_config.p_rx_packet_queue = p_rail_config->p_rx_packet_queue;
  #ifdef SL_CATALOG_HIGH_DATARATE_PHY_PRESENT
    if (sli_phy_rx_fifo) {
      mux_rail_config.rx_fifo_bytes = sli_mac_max_rx_flat_pkt_size << 1;
      mux_rail_config.p_rx_fifo_buffer = sli_phy_rx_fifo;
      p_rail_config->rx_fifo_bytes = 0;
      p_rail_config->p_rx_fifo_buffer = NULL;
    } else {
      mux_rail_config.rx_fifo_bytes = p_rail_config->rx_fifo_bytes;
      mux_rail_config.p_rx_fifo_buffer = p_rail_config->p_rx_fifo_buffer;
    }
  #else
    mux_rail_config.rx_fifo_bytes = p_rail_config->rx_fifo_bytes;
    mux_rail_config.p_rx_fifo_buffer = p_rail_config->p_rx_fifo_buffer;
  #endif
    mux_rail_config.tx_fifo_init_bytes = p_rail_config->tx_fifo_init_bytes;
    mux_rail_config.p_tx_fifo_buffer = p_rail_config->p_tx_fifo_buffer;
    mux_rail_config.tx_fifo_bytes = p_rail_config->tx_fifo_bytes;
    if ( (p_rail_config->p_tx_fifo_buffer != NULL) && (p_rail_config->tx_fifo_bytes > 0)) {
      fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SETUP_TX_FIFO, true);
      protocol_context[i].fifo_tx_info.data_ptr = (uint8_t *)p_rail_config->p_tx_fifo_buffer;
      protocol_context[i].fifo_tx_info.tx_init_length = p_rail_config->tx_fifo_init_bytes;;
      protocol_context[i].fifo_tx_info.tx_size = p_rail_config->tx_fifo_bytes;
    }

    mux_rail_handle = SL_RAIL_EFR32_HANDLE;
    sl_rail_status_t status = sl_rail_init(&mux_rail_handle, &mux_rail_config, fn_mux_rail_init_callback);
    EFM_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
  } else {
    // to save RAM
    p_rail_config->rx_fifo_bytes = 0;
    p_rail_config->p_rx_fifo_buffer = NULL;
    if (fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_COMPLETED)) {
      // RAIL Init already completed, we can just call the protocol
      // init_complete callback here.

      fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_INIT_CB_PENDING, false);
      if (init_complete_callback != NULL) {
        init_complete_callback(&(protocol_context[i]));
      }
    }
  }

  // If the RAIL initialization was started by another protocol, but not
  // completed yet. We will call the protocol init_completed callback when
  // RAIL calls it.

  RAIL_MUX_EXIT_CRITICAL();

  // Enable use of RAIL multi-timer.
  sl_rail_config_multi_timer(mux_rail_handle, true);

  // The RAIL handler we return to the protocol layers is a pointer to our own
  // internal protocol context structure.
  *p_rail_handle = (sl_rail_handle_t )(&(protocol_context[i]));
  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_IEEE802154_Init(sl_rail_handle_t railHandle,
                                             const sl_rail_ieee802154_config_t *config)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  (void)railHandle;
  (void)config;

  RAIL_MUX_ENTER_CRITICAL();

  if (!fn_get_global_flag(RAIL_MUX_FLAGS_IEEE802154_INIT_COMPLETED)) {
    sl_rail_status_t status = sl_rail_ieee802154_init(mux_rail_handle, &ieee_802154_config);
    EFM_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
    fn_set_global_flag(RAIL_MUX_FLAGS_IEEE802154_INIT_COMPLETED, true);
  }

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_status_t sli_rail_mux_lock_radio(sl_rail_handle_t railHandle)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);
  sl_status_t ret_val = SL_STATUS_OK;

  RAIL_MUX_ENTER_CRITICAL();
  // Is lock currently active, but for a different protocol
  if ( !check_lock_permissions(context_index) || (tx_in_progress() && (fn_get_active_tx_context_index() != context_index) )) {
    ret_val = SL_STATUS_FAIL;
  } else {
    // LOCK CAN BE GRANTED
    // Some cleanup to be performed before granting the lock

    for (uint8_t i = 0; (i < SUPPORTED_PROTOCOL_COUNT) && (i != context_index); i++) {
      // 1. Cancel any previously scheduled RX operations on the other protocol
      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_RX_SCHEDULED)) {
        // Call to RAIL Idle will cancel any previously scheduled Rx on the other protocol
        sl_rail_idle(mux_rail_handle, SL_RAIL_IDLE_ABORT, true);

        //restore rx state since we were previously in rx
        if ( rx_channel != INVALID_CHANNEL) {
          CONFIGURE_RX_CHANNEL_SWITCHING(mux_rail_handle, channel_switching_cfg);
          sl_rail_start_rx(mux_rail_handle, rx_channel, NULL);
        }
        fn_mux_rail_events_callback(mux_rail_handle, SL_RAIL_EVENT_RX_SCHEDULED_RX_END);
      }

      // 2. Check if there are any TX scheduled events on the other protocol
      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED)) {
        // Stop all pending transmit operations
        sl_rail_stop_tx(mux_rail_handle, SL_RAIL_STOP_MODE_PENDING_SHIFT);

        // First call tx started, this will unblock statemachine and clear the flag
        fn_mux_rail_events_callback(mux_rail_handle, SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED);

        // Now post TX Blocked to inform the application that the tx did not go through
        fn_mux_rail_events_callback(mux_rail_handle, SL_RAIL_EVENT_TX_BLOCKED);
      }
    }
    fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_LOCK_ACTIVE, true);
  }
  RAIL_MUX_EXIT_CRITICAL();
  return ret_val;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sli_rail_mux_unlock_radio(sl_rail_handle_t railHandle)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);
  sl_status_t ret_val = SL_STATUS_OK;

  RAIL_MUX_ENTER_CRITICAL();
  // Is lock currently active, but for a different protocol
  if ( !check_lock_permissions(context_index) ) {
    ret_val = SL_STATUS_FAIL;
  } else {
    fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_LOCK_ACTIVE, false);
  }
  RAIL_MUX_EXIT_CRITICAL();

  return ret_val;
}

sl_rail_status_t sl_rail_mux_ConfigEvents(sl_rail_handle_t railHandle,
                                          sl_rail_events_t mask,
                                          sl_rail_events_t events)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  sl_rail_events_t all_protocols_events = SL_RAIL_EVENTS_NONE;
  sl_rail_events_t updated_protocol_events;
  sl_rail_status_t status;
  uint8_t i;

  uint8_t context_index = fn_get_context_index(railHandle);
  if (context_index >= SUPPORTED_PROTOCOL_COUNT) {
    EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);
    // It will never return from here as long as assert is present.
    // This is to take care of gcc-12 warning -Werror=array-bounds promoted to error.
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }
  updated_protocol_events = (protocol_context[context_index].events & ~mask);
  updated_protocol_events |= (mask & events);

  RAIL_MUX_ENTER_CRITICAL();

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (i == context_index) {
      // Use the updated events for the protocol that is requesting a change.
      all_protocols_events |= updated_protocol_events;
    } else {
      all_protocols_events |= protocol_context[i].events;
    }
  }

  status = sl_rail_config_events(mux_rail_handle,
                                 SL_RAIL_EVENTS_ALL,
                                 all_protocols_events);

  // Update protocol events only if the actual sl_rail_config_events() call was
  // successful.
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    protocol_context[context_index].events = updated_protocol_events;
  }

  RAIL_MUX_EXIT_CRITICAL();

  return status;
}

sl_rail_status_t sl_rail_mux_SetPtiProtocol(sl_rail_handle_t railHandle,
                                            sl_rail_pti_protocol_t protocol)
{
  (void)railHandle;
  (void)protocol;

  if (!fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_SET_PTI_DONE)) {
    fn_set_global_flag(RAIL_MUX_FLAGS_RAIL_SET_PTI_DONE, true);
    // TODO: for now we simply set it to 802154, which means that Network
    // Analyzer as is, would only decode the MAC portion of the Zigbee/OT
    // frames. We might want to refine this, either by using a custom protocol
    // and do some work on the network analyzer side of things, or at least
    // change the PTI protocol at runtime so that least for outgoing packets
    // with get proper decoding.
    sl_rail_set_pti_protocol(mux_rail_handle, SL_RAIL_PTI_PROTOCOL_802154);
  }

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetShortAddress(sl_rail_handle_t railHandle,
                                                        uint16_t shortAddr,
                                                        uint8_t index)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  if (index >= SL_RAIL_IEEE802154_MAX_ADDRESSES) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }

  RAIL_MUX_ENTER_CRITICAL();

  protocol_context[context_index].addr_802154.short_addr[index] = shortAddr;

  fn_update_802154_address_filtering_table();

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetLongAddress(sl_rail_handle_t railHandle,
                                                       const uint8_t *longAddr,
                                                       uint8_t index)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  if (index >= SL_RAIL_IEEE802154_MAX_ADDRESSES) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }

  RAIL_MUX_ENTER_CRITICAL();

  memcpy(protocol_context[context_index].addr_802154.long_addr[index], longAddr, 8);

  fn_update_802154_address_filtering_table();

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetPanId(sl_rail_handle_t railHandle,
                                                 uint16_t panId,
                                                 uint8_t index)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  if (index >= SL_RAIL_IEEE802154_MAX_ADDRESSES) {
    return SL_RAIL_STATUS_INVALID_PARAMETER;
  }

  RAIL_MUX_ENTER_CRITICAL();

  protocol_context[context_index].addr_802154.pan_id[index] = panId;

  fn_update_802154_address_filtering_table();

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetAddresses(sl_rail_handle_t railHandle,
                                                     const sl_rail_ieee802154_addr_config_t *addresses)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  RAIL_MUX_ENTER_CRITICAL();

  memcpy(&protocol_context[context_index].addr_802154,
         addresses,
         sizeof(sl_rail_ieee802154_addr_config_t));

  fn_update_802154_address_filtering_table();

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetPanCoordinator(sl_rail_handle_t railHandle,
                                                          bool isPanCoordinator)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  uint8_t i;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  RAIL_MUX_ENTER_CRITICAL();

  protocol_context[context_index].is_pan_coordinator_802154 = isPanCoordinator;

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (protocol_context[i].is_pan_coordinator_802154) {
      break;
    }
  }
  // We push down to RAIL a 'true' coordinator flag if at least one protocol is
  // configured as 802.15.4 coordinator.
  sl_rail_ieee802154_set_pan_coordinator(mux_rail_handle,
                                         (i < SUPPORTED_PROTOCOL_COUNT));

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_StartRx(sl_rail_handle_t railHandle,
                                     uint16_t channel,
                                     const sl_rail_scheduler_info_t *schedulerInfo)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);
  sl_rail_status_t ret_status = SL_RAIL_STATUS_NO_ERROR;

  RAIL_MUX_ENTER_CRITICAL();

  // Store channel in the protocol context
  // This is done regardless of the lock because a lock will
  // prevent start rx operation and channel change from happening
  // and rx events will use the channel to determine whether to
  // pass the event up or not
  protocol_context[context_index].channel = channel;
  SET_CHANNEL_SWITCHING_CFG_CH(context_index, channel);

  // Check to ensure lock is not active before acting on startRx
  if ( check_lock_permissions(context_index) && !tx_in_progress()) {
    #if !defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) && !defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)
    if (!fn_get_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_LOCK_ACTIVE)) {
      if (rx_channel != INVALID_CHANNEL && rx_channel != channel) {
        // this protocol context rx channel is different from the other protocol context rx channel
        sl_rail_mux_invalid_rx_channel_detected_cb(channel, rx_channel);
      }
    }
    #endif

    rx_channel = channel;
    fn_update_802154_address_filtering_table();
    CONFIGURE_RX_CHANNEL_SWITCHING(mux_rail_handle, channel_switching_cfg);
    // TODO: for now we pass down the schedulerInfo "as is".
    ret_status = sl_rail_start_rx(mux_rail_handle, channel, schedulerInfo);
  }
  RAIL_MUX_EXIT_CRITICAL();

  return ret_status;
}

sl_rail_status_t sl_rail_mux_Idle(sl_rail_handle_t railHandle,
                                  sl_rail_idle_mode_t mode,
                                  bool wait)
{
  uint8_t i;
 
  if (tx_in_progress()) {
    return SL_RAIL_STATUS_INVALID_CALL;
  }

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  protocol_context[context_index].channel = INVALID_CHANNEL;
  SET_CHANNEL_SWITCHING_CFG_CH(context_index, INVALID_CHANNEL);

  // If another protocol is RXing, we stay in RX, otherwise we idle the radio.
  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (check_lock_permissions(i) && protocol_context[i].channel != INVALID_CHANNEL) {
      CONFIGURE_RX_CHANNEL_SWITCHING(mux_rail_handle, channel_switching_cfg);
      rx_channel = protocol_context[i].channel;
      sl_rail_start_rx(mux_rail_handle, protocol_context[i].channel, NULL);
      return SL_RAIL_STATUS_NO_ERROR;
    }
  }

  rx_channel = INVALID_CHANNEL;
  return sl_rail_idle(mux_rail_handle, mode, wait);
}

sl_rail_status_t sl_rail_mux_CopyRxPacket(sl_rail_handle_t railHandle,
                                          uint8_t *pDest,
                                          const sl_rail_rx_packet_info_t *pPacketInfo)
{
  (void)railHandle;
  return sl_rail_copy_rx_packet(mux_rail_handle, pDest, pPacketInfo);
}

sl_rail_status_t sl_rail_mux_ConfigMultiTimer(sl_rail_handle_t rail_handle,
                                              bool enable)
{
  (void) rail_handle;
  return (sl_rail_config_multi_timer(mux_rail_handle, enable));
}

sl_rail_status_t sl_rail_mux_SetMultiTimer(sl_rail_handle_t rail_handle,
                                           sl_rail_multi_timer_t *p_tmr,
                                           sl_rail_time_t expiration_time,
                                           sl_rail_time_mode_t expiration_mode,
                                           sl_rail_multi_timer_callback_t expiration_callback,
                                           void *cb_arg)
{
  (void)rail_handle;
  return (sl_rail_set_multi_timer(mux_rail_handle,
                                  p_tmr,
                                  expiration_time,
                                  expiration_mode,
                                  expiration_callback,
                                  cb_arg));
}

sl_rail_status_t sl_rail_mux_CancelMultiTimer(sl_rail_handle_t rail_handle,
                                              sl_rail_multi_timer_t *p_tmr)
{
  (void)rail_handle;
  return (sl_rail_cancel_multi_timer(mux_rail_handle, p_tmr));
}

bool sl_rail_mux_IsMultiTimerRunning(sl_rail_handle_t rail_handle,
                                     sl_rail_multi_timer_t *p_tmr)
{
  (void)rail_handle;
  return (sl_rail_is_multi_timer_running(mux_rail_handle, p_tmr));
}

sl_rail_time_t sl_rail_mux_GetTime(sl_rail_handle_t rail_handle)
{
  (void)rail_handle;
  return (sl_rail_get_time(mux_rail_handle));
}
sl_rail_status_t sl_rail_mux_PrepareChannel(sl_rail_handle_t rail_handle,
                                            uint16_t channel)
{
  (void)rail_handle;
  return sl_rail_prepare_channel(mux_rail_handle, channel);
}

sl_rail_status_t sl_rail_mux_ConfigRxOptions(sl_rail_handle_t railHandle,
                                             sl_rail_rx_options_t mask,
                                             sl_rail_rx_options_t options)
{
  (void)railHandle;

  // TODO: Maintain separate options per protocol? Complain if there is some
  // conflict among the protocols' options?
  // For now we can skip this since both MAC layers configure the same RX
  // options.

  return sl_rail_config_rx_options(mux_rail_handle, mask, options);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_SetTaskPriority(sl_rail_handle_t railHandle,
                                             uint8_t priority,
                                             sl_rail_task_type_t taskType)
{
  (void)railHandle;

  return sl_rail_set_task_priority(mux_rail_handle, priority, taskType);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadio(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_config_2p4_ghz_radio(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_ieee802154_config_2_mbps_rx_channel(sl_rail_handle_t railHandle,
                                                                 uint16_t channel)
{
  (void)railHandle;

  return sl_rail_ieee802154_config_2_mbps_rx_channel(mux_rail_handle, channel);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadio1MbpsFec(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_config_2p4_ghz_radio_1_mbps_fec(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadio2Mbps(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_config_2p4_ghz_radio_2_mbps(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioFcs1MbpsFec(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_config_2p4_ghz_radio_fcs_1_mbps_fec(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioFcs2Mbps(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_config_2p4_ghz_radio_fcs_2_mbps(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDiv(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_2p4_ghz_radio_ant_div(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_ieee802154_config_2p4_ghz_radio_fast_channel_switching(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  #if defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) || defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)
  return sl_rail_ieee802154_config_2p4_ghz_radio_fast_channel_switching(mux_rail_handle);
  #else
  return SL_STATUS_NOT_SUPPORTED;
  #endif
}

sl_rail_status_t sl_rail_mux_ieee802154_config_2p4_ghz_radio_rx_duty_cycling(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  #if defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_PRESENT) || defined(SL_CATALOG_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_PRESENT)
  return sl_rail_ieee802154_config_2p4_ghz_radio_rx_duty_cycling(mux_rail_handle);
  #else
  return SL_STATUS_NOT_SUPPORTED;
  #endif
}

sl_rail_status_t sl_rail_mux_ConfigChannels(sl_rail_handle_t railHandle,
                                            const sl_rail_channel_config_t *config,
                                            sl_rail_radio_config_changed_callback_t cb)
{
  (void)railHandle;

  // TODO: this currently only called from Zigbee in simulation.

  return sl_rail_config_channels(mux_rail_handle, config, cb);
}

sl_rail_status_t sl_rail_mux_ConvertLqi(sl_rail_handle_t railHandle,
                                        sl_rail_convert_lqi_callback_t cb)
{
  (void)railHandle;

  // The callback does not pass up a sl_rail_handle_t reference, so we can just
  // pass the callback from the upper layer down to RAIL.

  return sl_rail_convert_lqi(mux_rail_handle, cb);
}

sl_rail_status_t sl_rail_mux_Calibrate(sl_rail_handle_t railHandle,
                                       sl_rail_cal_values_t *calValues,
                                       sl_rail_cal_mask_t calForce)
{
  (void)railHandle;

  return sl_rail_calibrate(mux_rail_handle, calValues, calForce);
}

sl_rail_status_t sl_rail_mux_IEEE802154_CalibrateIr2p4Ghz(sl_rail_handle_t railHandle,
                                                          uint32_t *imageRejection)
{
  (void)railHandle;

  return sl_rail_ieee802154_calibrate_ir_2p4_ghz(mux_rail_handle, imageRejection);
}

uint16_t sl_rail_mux_GetRadioEntropy(sl_rail_handle_t railHandle,
                                     uint8_t *buffer,
                                     uint16_t bytes)
{
  (void)railHandle;

  return sl_rail_get_radio_entropy(mux_rail_handle, buffer, bytes);
}

sl_rail_status_t sl_rail_mux_IsValidChannel(sl_rail_handle_t railHandle, uint16_t channel)
{
  (void)railHandle;

  return sl_rail_is_valid_channel(mux_rail_handle, channel);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
int16_t sl_rail_mux_GetRssi(sl_rail_handle_t railHandle, sl_rail_time_t  waitTimeout)
{
  (void)railHandle;

  return sl_rail_get_rssi(mux_rail_handle, waitTimeout);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
uint32_t sl_rail_mux_GetSymbolRate(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_get_symbol_rate(mux_rail_handle);
}

uint32_t sl_rail_mux_GetBitRate(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_get_bit_rate(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_SetFreqOffset(sl_rail_handle_t railHandle,
                                           sl_rail_frequency_offset_t freqOffset)
{
  (void)railHandle;

  return sl_rail_set_freq_offset(mux_rail_handle, freqOffset);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_SetTimer(sl_rail_handle_t railHandle,
                                      sl_rail_time_t time,
                                      sl_rail_time_mode_t mode,
                                      sl_rail_timer_callback_t cb)
{
  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  protocol_context[context_index].timer_callback = cb;
  return sl_rail_set_multi_timer(mux_rail_handle,
                                 &protocol_context[context_index].timer,
                                 time,
                                 mode,
                                 (cb == NULL) ? NULL : fn_timer_callback,
                                 railHandle);
}

sl_rail_status_t sl_rail_mux_CancelTimer(sl_rail_handle_t railHandle)
{
  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  return (sl_rail_cancel_multi_timer(mux_rail_handle, &protocol_context[context_index].timer));
}

sl_rail_cal_mask_t sl_rail_mux_GetPendingCal(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_get_pending_cal(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_CalibrateTemp(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_calibrate_temp(mux_rail_handle);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_rx_packet_handle_t sl_rail_mux_GetRxPacketInfo(sl_rail_handle_t railHandle,
                                                       sl_rail_rx_packet_handle_t packetHandle,
                                                       sl_rail_rx_packet_info_t *pPacketInfo)
{
  (void)railHandle;

  return sl_rail_get_rx_packet_info(mux_rail_handle, packetHandle, pPacketInfo);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_GetRxPacketDetailsAlt(sl_rail_handle_t railHandle,
                                                   sl_rail_rx_packet_handle_t packetHandle,
                                                   sl_rail_rx_packet_details_t *pPacketDetails)
{
  (void)railHandle;

  return sl_rail_get_rx_packet_details(mux_rail_handle,
                                       packetHandle,
                                       pPacketDetails);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
uint16_t sl_rail_mux_PeekRxPacket(sl_rail_handle_t railHandle,
                                  sl_rail_rx_packet_handle_t packetHandle,
                                  uint8_t *pDst,
                                  uint16_t len,
                                  uint16_t offset)
{
  (void)railHandle;

  return sl_rail_peek_rx_packet(mux_rail_handle,
                                packetHandle,
                                pDst,
                                len,
                                offset);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_IEEE802154_GetAddress(sl_rail_handle_t railHandle,
                                                   sl_rail_ieee802154_address_t *pAddress)
{
  (void)railHandle;

  return sl_rail_ieee802154_get_address(mux_rail_handle, pAddress);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_GetRxIncomingPacketInfo(sl_rail_handle_t railHandle,
                                                     sl_rail_rx_packet_info_t *pPacketInfo)
{
  (void)railHandle;

  return sl_rail_get_rx_incoming_packet_info(mux_rail_handle, pPacketInfo);
}

int8_t sl_rail_mux_GetRssiOffset(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_get_rssi_offset(mux_rail_handle);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_IEEE802154_SetFramePending(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_toggle_frame_pending(mux_rail_handle);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)

sl_rail_status_t sl_rail_mux_GetSchedulerStatus(sl_rail_handle_t railHandle,
                                                sl_rail_scheduler_status_t *p_scheduler_status,
                                                sl_rail_status_t *p_rail_status)

{
  (void)railHandle;

  return sl_rail_get_scheduler_status(mux_rail_handle, p_scheduler_status, p_rail_status);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
bool sl_rail_mux_IsRxAutoAckPaused(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_is_rx_auto_ack_paused(mux_rail_handle);
}

sl_rail_status_t sli_rail_mux_GetTxPowerConfig(sl_rail_handle_t railHandle,
                                               sl_rail_tx_power_config_t *config)
{
  (void)railHandle;

  return sli_rail_get_tx_power_config(mux_rail_handle, config);
}

sl_rail_tx_power_t sl_rail_mux_GetTxPowerDbm(sl_rail_handle_t railHandle)
{
  uint8_t context_index = fn_get_context_index(railHandle);
  if (context_index < SUPPORTED_PROTOCOL_COUNT) {
    // Return the protocol-specific power setting, not the aggregated RAIL power
    return protocol_context[context_index].tx_power;
  }
  // Fallback to RAIL power if context not found
  return sl_rail_get_tx_power_dbm(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_GetChannel(sl_rail_handle_t railHandle, uint16_t *channel)
{
  (void)railHandle;

  return sl_rail_get_channel(mux_rail_handle, channel);
}

sl_rail_tx_power_t sli_rail_mux_GetTxPower(sl_rail_handle_t railHandle)
{
  (void) railHandle;
  return sli_rail_get_tx_power(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_util_pa_post_init(sl_rail_handle_t railHandle,
                                               sl_rail_tx_pa_mode_t pa_mode)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  uint16_t status = SL_RAIL_STATUS_NO_ERROR;
  (void)railHandle;

  RAIL_MUX_ENTER_CRITICAL();

  if (!fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_CONFIG_TX_POWER_DONE)) {
    status = sl_rail_util_pa_post_init(mux_rail_handle, pa_mode);
    fn_set_global_flag(RAIL_MUX_FLAGS_RAIL_CONFIG_TX_POWER_DONE, true);
  }

  RAIL_MUX_EXIT_CRITICAL();

  return status;
}

sl_rail_tx_pa_mode_t sl_rail_mux_get_pa_mode(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_get_pa_mode(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_ConfigTxPower(sl_rail_handle_t railHandle,
                                           const sl_rail_tx_power_config_t *config)
{
  (void)config;
  return sl_rail_mux_util_pa_post_init(railHandle, SL_RAIL_TX_PA_MODE_2P4_GHZ);
}

sl_rail_status_t sl_rail_mux_SetTxPowerDbm(sl_rail_handle_t railHandle,
                                           sl_rail_tx_power_t power)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  uint8_t active_tx_index;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  RAIL_MUX_ENTER_CRITICAL();
  protocol_context[context_index].tx_power = power;

  // Determine if we should update hardware immediately
  active_tx_index = fn_get_active_tx_context_index();
  if (active_tx_index == INVALID_CONTEXT_INDEX) {
    // No TX in progress - only update hardware if this is initial power configuration
    if (current_tx_power == SL_RAIL_TX_POWER_MIN) {
      RAIL_SetTxPowerDbm(mux_rail_handle, power);
      current_tx_power = power;
    }
    pending_power_context = INVALID_CONTEXT_INDEX;
  } else if (active_tx_index == context_index) {
    // This context is currently transmitting, update its power immediately
    RAIL_SetTxPowerDbm(mux_rail_handle, power);
    current_tx_power = power;
    pending_power_context = INVALID_CONTEXT_INDEX;
  } else {
    // Another context is transmitting, defer hardware update
    pending_power_context = context_index;
  }

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_YieldRadio(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_yield_radio(mux_rail_handle);
}

/*
 * This function is used to get the current radio state.
 * If the radio is not in RX/Idle state, it will return the current state.
 * If the radio is in RX/Idle state, it will check:
 * - If the given protocol thinks it is in RX state, it will return RX state.
 * - If the given protocol thinks it is in Idle state, it will return Idle state.
 *
 * The complicated logic here is added to handle the case where:
 * The radio is in RX state
 * but the given protocol's last call to the RAIL.... APIs was consequently to sl_rail_idle(), and mux's
 * protocol context for that protocol adheres to the idle call and vise versa.
 */
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_radio_state_t sl_rail_mux_GetRadioState(sl_rail_handle_t railHandle)
{
  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);
  sl_rail_radio_state_t radio_state = sl_rail_get_radio_state(mux_rail_handle);
  return ((radio_state & SL_RAIL_RF_STATE_IDLE) != SL_RAIL_RF_STATE_IDLE && (radio_state & SL_RAIL_RF_STATE_RX) != SL_RAIL_RF_STATE_RX) ? radio_state
         : (protocol_context[context_index].channel == INVALID_CHANNEL ? SL_RAIL_RF_STATE_IDLE : SL_RAIL_RF_STATE_RX);
}
sl_rail_status_t sl_rail_mux_SetTxFifo(sl_rail_handle_t rail_handle,
                                       sl_rail_fifo_buffer_align_t *p_addr,
                                       uint16_t size_bytes,
                                       uint16_t init_bytes,
                                       uint16_t start_offset_bytes)
{
  (void)start_offset_bytes;
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(rail_handle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  RAIL_MUX_ENTER_CRITICAL();

  fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_SETUP_TX_FIFO, true);
  protocol_context[context_index].fifo_tx_info.data_ptr = (uint8_t *)p_addr;
  protocol_context[context_index].fifo_tx_info.tx_init_length = init_bytes;
  protocol_context[context_index].fifo_tx_info.tx_size = size_bytes;

  RAIL_MUX_EXIT_CRITICAL();

  return SL_RAIL_STATUS_NO_ERROR;
}

sl_rail_status_t sl_rail_mux_StartCcaCsmaTx(sl_rail_handle_t railHandle,
                                            uint16_t channel,
                                            sl_rail_tx_options_t options,
                                            const sl_rail_csma_config_t *csmaConfig,
                                            const sl_rail_scheduler_info_t *schedulerInfo)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  uint16_t ret_status = SL_RAIL_STATUS_INVALID_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  // Reject operation if other protocol holds lock
  RAIL_MUX_ENTER_CRITICAL();
  if ( check_lock_permissions(context_index) ) {
    if (!fn_get_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING)) {
      fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING, true);
      protocol_context[context_index].csma_tx_info.tx_type = SINGLE_TX_CCA_CSMA_REQUEST;
      protocol_context[context_index].csma_tx_info.channel = channel;
      protocol_context[context_index].csma_tx_info.options = options;

      fn_set_context_flag_by_index(context_index,
                                   RAIL_MUX_PROTOCOL_FLAGS_CSMA_CONFIG_VALID,
                                   (csmaConfig != NULL));
      if (csmaConfig != NULL) {
        protocol_context[context_index].csma_tx_info.csmaConfig = *csmaConfig;
      }

      fn_set_context_flag_by_index(context_index,
                                   RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID,
                                   (schedulerInfo != NULL));
      if (schedulerInfo != NULL) {
        protocol_context[context_index].csma_tx_info.schedulerInfo = *schedulerInfo;
      }
    }
    ret_status = fn_start_pending_tx();
  }
  RAIL_MUX_EXIT_CRITICAL();
  return ret_status;
}

sl_rail_status_t sl_rail_mux_StartTx(sl_rail_handle_t railHandle,
                                     uint16_t channel,
                                     sl_rail_tx_options_t options,
                                     const sl_rail_scheduler_info_t *schedulerInfo)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  sl_rail_status_t ret_status = SL_RAIL_STATUS_INVALID_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  RAIL_MUX_ENTER_CRITICAL();
  if ( check_lock_permissions(context_index) ) {
    if (!fn_get_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING)) {
      fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING, true);
      protocol_context[context_index].csma_tx_info.tx_type = SINGLE_TX_REQUEST;
      protocol_context[context_index].csma_tx_info.channel = channel;
      protocol_context[context_index].csma_tx_info.options = options;

      fn_set_context_flag_by_index(context_index,
                                   RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID,
                                   (schedulerInfo != NULL));
      if (schedulerInfo != NULL) {
        protocol_context[context_index].csma_tx_info.schedulerInfo = *schedulerInfo;
      }
    }
    ret_status = fn_start_pending_tx();
  }
  RAIL_MUX_EXIT_CRITICAL();

  return ret_status;
}

sl_rail_status_t sl_rail_mux_StopTxStream(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_stop_tx_stream(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetPromiscuousMode(sl_rail_handle_t railHandle,
                                                           bool enable)
{
  (void)railHandle;

  return sl_rail_ieee802154_set_promiscuous_mode(mux_rail_handle, enable);
}

sl_rail_status_t sl_rail_mux_PauseRxAutoAck(sl_rail_handle_t railHandle,
                                            bool pause)
{
  (void)railHandle;

  return sl_rail_pause_rx_auto_ack(mux_rail_handle, pause);
}

sl_rail_status_t sl_rail_mux_SetCcaThreshold(sl_rail_handle_t railHandle,
                                             int8_t ccaThresholdDbm)
{
  (void)railHandle;

  return sl_rail_set_cca_threshold(mux_rail_handle, ccaThresholdDbm);
}

bool sl_rail_mux_IEEE802154_IsEnabled(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_is_enabled(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_SetRxTransitions(sl_rail_handle_t railHandle,
                                              const sl_rail_state_transitions_t *transitions)
{
  (void)railHandle;

  return sl_rail_set_rx_transitions(mux_rail_handle, transitions);
}

sl_rail_status_t sl_rail_mux_ConfigCal(sl_rail_handle_t railHandle,
                                       sl_rail_cal_mask_t calEnable)
{
  (void)railHandle;

  return sl_rail_config_cal(mux_rail_handle, calEnable);
}

uint16_t sl_rail_mux_ReadRxFifo(sl_rail_handle_t railHandle,
                                uint8_t *dataPtr,
                                uint16_t readLength)
{
  (void)railHandle;

  return sl_rail_read_rx_fifo(mux_rail_handle, dataPtr, readLength);
}

uint16_t sl_rail_mux_GetRxFifoBytesAvailable(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_get_rx_fifo_bytes_available(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_StartScheduledCcaCsmaTx(sl_rail_handle_t railHandle,
                                                     uint16_t channel,
                                                     sl_rail_tx_options_t options,
                                                     const sl_rail_scheduled_tx_config_t *scheduleTxConfig,
                                                     const sl_rail_csma_config_t *csmaConfig,
                                                     const sl_rail_scheduler_info_t *schedulerInfo)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  sl_rail_status_t ret_status = SL_RAIL_STATUS_INVALID_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  // ToDo: we might need to consider protocol priorities here

  RAIL_MUX_ENTER_CRITICAL();
  if ( check_lock_permissions(context_index) ) {
    // Even if there was a pending tx here, we are gonna re-write it, but we have
    // to make sure tx FIFO is also written.
    fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULED_TX_PENDING, true);
    protocol_context[context_index].csma_tx_info.tx_type = SCHEDULED_TX_CCA_CSMA_REQUEST;
    protocol_context[context_index].csma_tx_info.channel = channel;
    protocol_context[context_index].csma_tx_info.options = options;

    fn_set_context_flag_by_index(context_index,
                                 RAIL_MUX_PROTOCOL_FLAGS_SCHEDULE_TX_CONFIG_VALID,
                                 (scheduleTxConfig != NULL));
    if (scheduleTxConfig != NULL) {
      protocol_context[context_index].csma_tx_info.scheduler_config = *scheduleTxConfig;
    }

    fn_set_context_flag_by_index(context_index,
                                 RAIL_MUX_PROTOCOL_FLAGS_CSMA_CONFIG_VALID,
                                 (csmaConfig != NULL));
    if (csmaConfig != NULL) {
      protocol_context[context_index].csma_tx_info.csmaConfig = *csmaConfig;
    }

    fn_set_context_flag_by_index(context_index,
                                 RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID,
                                 (schedulerInfo != NULL));
    if (schedulerInfo != NULL) {
      protocol_context[context_index].csma_tx_info.schedulerInfo = *schedulerInfo;
    }

    ret_status = fn_start_pending_tx();
  }
  RAIL_MUX_EXIT_CRITICAL();
  return ret_status;
}

sl_rail_status_t sl_rail_mux_StartScheduledTx(sl_rail_handle_t railHandle,
                                              uint16_t channel,
                                              sl_rail_tx_options_t options,
                                              const sl_rail_scheduled_tx_config_t *config,
                                              const sl_rail_scheduler_info_t *schedulerInfo)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  sl_rail_status_t ret_status = SL_RAIL_STATUS_INVALID_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  // ToDo: we might need to consider protocol priorities here

  RAIL_MUX_ENTER_CRITICAL();
  if ( check_lock_permissions(context_index) ) {
    // Even if there was a pending tx here, we are gonna re-write it, but we have
    // to make sure tx FIFO is also written.
    fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULED_TX_PENDING, true);
    protocol_context[context_index].csma_tx_info.tx_type = SCHEDULED_TX_REQUEST;
    protocol_context[context_index].csma_tx_info.channel = channel;
    protocol_context[context_index].csma_tx_info.options = options;

    fn_set_context_flag_by_index(context_index,
                                 RAIL_MUX_PROTOCOL_FLAGS_SCHEDULE_TX_CONFIG_VALID,
                                 (config != NULL));
    if (config != NULL) {
      protocol_context[context_index].csma_tx_info.scheduler_config = *config;
    }

    fn_set_context_flag_by_index(context_index,
                                 RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID,
                                 (schedulerInfo != NULL));
    if (schedulerInfo != NULL) {
      protocol_context[context_index].csma_tx_info.schedulerInfo = *schedulerInfo;
    }
    ret_status = fn_start_pending_tx();
  }
  RAIL_MUX_EXIT_CRITICAL();

  return ret_status;
}

sl_rail_status_t sl_rail_mux_ConfigSleep(sl_rail_handle_t rail_handle,
                                         const sl_rail_timer_sync_config_t *p_timer_sync_config)
{
  (void)rail_handle;
  return sl_rail_config_sleep(mux_rail_handle, p_timer_sync_config);
}

sl_rail_status_t sl_rail_mux_StartAverageRssi(sl_rail_handle_t railHandle,
                                              uint16_t channel,
                                              sl_rail_time_t averagingTimeUs,
                                              const sl_rail_scheduler_info_t *schedulerInfo)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);
  sl_rail_status_t ret_status = SL_RAIL_STATUS_INVALID_STATE;

  RAIL_MUX_ENTER_CRITICAL();
  bool is_ok_to_proceed = check_lock_permissions(context_index);
  RAIL_MUX_EXIT_CRITICAL();

  if ( is_ok_to_proceed ) {
    ret_status = sl_rail_start_average_rssi(mux_rail_handle,
                                            channel,
                                            averagingTimeUs,
                                            schedulerInfo);
  }
  return ret_status;
}

uint16_t sl_rail_mux_WriteTxFifo(sl_rail_handle_t railHandle,
                                 const uint8_t *dataPtr,
                                 uint16_t writeLength,
                                 bool reset)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  uint8_t active_tx_context_index = fn_get_active_tx_context_index();
  uint16_t ret_len = 0;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  bool setupTxFifoFlag = fn_get_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_SETUP_TX_FIFO);
  EFM_ASSERT(setupTxFifoFlag);

  RAIL_MUX_ENTER_CRITICAL();
  // ToDo:
  // verify This call does not ever result to a call to sl_rail_write_tx_fifo
  // veify we do not need to worry about details such as bytewarps etc?
  if (active_tx_context_index != context_index) {
    if (!reset) {
      memcpy(protocol_context[context_index].fifo_tx_info.data_ptr + protocol_context[context_index].fifo_tx_info.tx_init_length, dataPtr, writeLength);
      protocol_context[context_index].fifo_tx_info.tx_init_length +=  writeLength;
    } else {
      memcpy(protocol_context[context_index].fifo_tx_info.data_ptr, dataPtr, writeLength);
      protocol_context[context_index].fifo_tx_info.tx_init_length = writeLength;
    }
    ret_len = writeLength;
  } else {
    ret_len = sl_rail_write_tx_fifo(mux_rail_handle,
                                    dataPtr,
                                    writeLength,
                                    reset);

    if (!reset) {
      protocol_context[context_index].fifo_tx_info.tx_init_length += ret_len;
    } else {
      protocol_context[context_index].fifo_tx_info.tx_init_length = ret_len;
    }
  }
  RAIL_MUX_EXIT_CRITICAL();

  return ret_len;
}

sl_rail_status_t sl_rail_mux_IEEE802154_WriteEnhAck(sl_rail_handle_t railHandle,
                                                    const uint8_t *ackData,
                                                    uint16_t ackDataLen)
{
  (void)railHandle;

  return sl_rail_ieee802154_write_enh_ack(mux_rail_handle, ackData, ackDataLen);
}

sl_rail_status_t sl_rail_mux_ReleaseRxPacket(sl_rail_handle_t railHandle,
                                             sl_rail_rx_packet_handle_t packetHandle)
{
  (void)railHandle;

  return sl_rail_release_rx_packet(mux_rail_handle, packetHandle);
}

sl_rail_rx_packet_handle_t sl_rail_mux_HoldRxPacket(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_hold_rx_packet(mux_rail_handle);
}

int16_t sl_rail_mux_GetAverageRssi(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_get_average_rssi(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_GetRxTimeSyncWordEndAlt(sl_rail_handle_t railHandle,
                                                     sl_rail_rx_packet_details_t *pPacketDetails)
{
  (void)railHandle;

  return sl_rail_get_rx_time_sync_word_end(mux_rail_handle, pPacketDetails);
}

sl_rail_ieee802154_phy_t sl_rail_mux_IEEE802154_GetPtiRadioConfig(sl_rail_handle_t railHandle)
{
  (void)railHandle;

  return sl_rail_ieee802154_get_phy_id(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_EnableEarlyFramePending(sl_rail_handle_t railHandle,
                                                                bool enable)
{
  (void)railHandle;

  return sl_rail_ieee802154_enable_early_frame_pending(mux_rail_handle, enable);
}

uint16_t sl_rail_mux_SetTxFifoThreshold(sl_rail_handle_t railHandle, uint16_t txThreshold)
{
  (void)railHandle;

  return sl_rail_set_tx_fifo_threshold(mux_rail_handle, txThreshold);
}

sl_rail_status_t sl_rail_mux_SetNextTxRepeat(sl_rail_handle_t railHandle,
                                             const sl_rail_tx_repeat_config_t *repeatConfig)
{
  sl_rail_status_t status = SL_RAIL_STATUS_INVALID_CALL;

  if (sl_rail_supports_tx_to_tx(mux_rail_handle)) {
    uint8_t context_index = fn_get_context_index(railHandle);
    EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

    status = SL_RAIL_STATUS_NO_ERROR;
    protocol_context[context_index].tx_repeat_config = *repeatConfig;
    fn_set_context_flag_by_index(context_index,
                                 RAIL_MUX_PROTOCOL_FLAGS_CONFIG_REPEATED_TX,
                                 true);
  }

  return status;
}

sl_rail_status_t sl_rail_mux_RAIL_ScheduleRx(sl_rail_handle_t railHandle,
                                             uint16_t channel,
                                             const sl_rail_scheduled_rx_config_t *cfg,
                                             const sl_rail_scheduler_info_t *schedulerInfo)
{
  #if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) || defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)
  // UID 1327639: Schedule Rx and Concurrent listening when used together
  // will cause undefined behavior. Therefore it was decided to assert upon detecting
  // this condition
  assert(0);
  #endif //SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT

  RAIL_MUX_DECLARE_IRQ_STATE;
  sl_rail_status_t ret_status = SL_RAIL_STATUS_INVALID_CALL;

  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  RAIL_MUX_ENTER_CRITICAL();

  // If there are any other scheduled RX
  if (!fn_operation_in_progress(RAIL_MUX_PROTOCOL_FLAGS_RX_SCHEDULED)) {
    fn_set_context_flag_by_index(context_index, RAIL_MUX_PROTOCOL_FLAGS_RX_SCHEDULED, true);

    protocol_context[context_index].channel = channel;
    SET_CHANNEL_SWITCHING_CFG_CH(context_index, channel);

    if ( check_lock_permissions(context_index) ) {
      rx_channel = channel;
      fn_update_802154_address_filtering_table();
      ret_status = sl_rail_start_scheduled_rx(mux_rail_handle, channel, cfg, schedulerInfo);
    } else {
      ret_status = SL_RAIL_STATUS_INVALID_STATE;
    }
  }

  RAIL_MUX_EXIT_CRITICAL();
  return ret_status;
}

void sl_rail_mux_set_coex_counter_handler(sl_rail_handle_t railHandle,
                                          COEX_CounterHandler_t counter_handler)
{
  uint8_t context_index = fn_get_context_index(railHandle);

  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  protocol_context[context_index].coex_counter_handler = counter_handler;
}

void sl_rail_util_coex_counter_on_event(sl_rail_util_coex_event_t event)
{
  uint8_t i;

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    // we don't have a way to know which protocol is currently supposed to receive the coex event
    if (protocol_context[i].coex_counter_handler != NULL) {
      (*protocol_context[i].coex_counter_handler)(event);
    }
  }
}

//------------------------------------------------------------------------------
// Static functions

static void fn_set_global_flag(uint16_t flag, bool val)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  RAIL_MUX_ENTER_CRITICAL();
  if (val) {
    internal_flags |= flag;
  } else {
    internal_flags &= ~flag;
  }
  RAIL_MUX_EXIT_CRITICAL();
}

static bool fn_get_global_flag(uint16_t flag)
{
  return ((internal_flags & flag) != 0);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static uint8_t fn_get_context_index(sl_rail_handle_t handle)
{
  uint8_t i;

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (handle == &protocol_context[i]) {
      return i;
    }
  }

  return INVALID_CONTEXT_INDEX;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static bool fn_get_context_flag_by_index(uint8_t index, uint16_t flag)
{
  return ((protocol_context[index].flags & flag) != 0);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static void fn_set_context_flag_by_index(uint8_t index, uint16_t flag, bool val)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  RAIL_MUX_ENTER_CRITICAL();
  if (val) {
    protocol_context[index].flags |= flag;
  } else {
    protocol_context[index].flags &= ~flag;
  }
  RAIL_MUX_EXIT_CRITICAL();
}

static void fn_init_802154_address_config(sl_rail_ieee802154_addr_config_t *addr_config)
{
  uint8_t i;

  for (i = 0; i < SL_RAIL_IEEE802154_MAX_ADDRESSES; i++) {
    addr_config->pan_id[i] = 0xFFFF;
    addr_config->short_addr[i] = 0xFFFF;
    memset(addr_config->long_addr[i], 0x00, 8);
  }
}

// It updates the RAIL 802.15.4 addressing filtering table, according to the
// current RX channel.
static void fn_update_802154_address_filtering_table(void)
{
  sl_rail_ieee802154_addr_config_t temp_addr_config;
  uint8_t temp_addr_filter_mask[SUPPORTED_PROTOCOL_COUNT];
  bool addr_filter_mask_changed = false;
  uint8_t i, j;
  uint8_t entry_index = 0;
  const uint8_t zeroed_long_addr[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  // Nothing to do, the node is not RXing.
  if (rx_channel == INVALID_CHANNEL) {
    return;
  }

  fn_init_802154_address_config(&temp_addr_config);
  // Set the broadcast address bit and the broadcast PAN bit for each protocol.
  memset(temp_addr_filter_mask, RAIL_MUX_FILTERING_MASK_BROADCAST_ENABLED, SUPPORTED_PROTOCOL_COUNT);

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    for (j = 0; j < SL_RAIL_IEEE802154_MAX_ADDRESSES; j++) {
      // If either PAN ID or short Address is a value other than 0xFFFF or
      // the long ID is a value other than 0x00s, we have some non-default
      // address filtering that needs to be pushed down to RAIL.
      if (protocol_context[i].channel == CHANNEL_CHECK
          && (protocol_context[i].addr_802154.pan_id[j] != 0xFFFF
              || protocol_context[i].addr_802154.short_addr[j] != 0xFFFF
              || memcmp(protocol_context[i].addr_802154.long_addr[j],
                        zeroed_long_addr,
                        8) != 0)) {
        temp_addr_config.pan_id[entry_index] = protocol_context[i].addr_802154.pan_id[j];
        temp_addr_config.short_addr[entry_index] = protocol_context[i].addr_802154.short_addr[j];
        memcpy(temp_addr_config.long_addr[entry_index],
               protocol_context[i].addr_802154.long_addr[j],
               8);
        // Set the panID bit and address bit corresponding to the rail addressing entry.
        temp_addr_filter_mask[i] |= ((1 << (entry_index + 1)) | (1 << (entry_index + 5)));
        entry_index++;

        // We maxed out the RAIL address filtering entries.
        // TODO: for now we just go in arbitrary order, we might revisit this
        // later on and use some sort of protocol priority to decide which
        // address filtering entry should go first.
        if (entry_index == SL_RAIL_IEEE802154_MAX_ADDRESSES) {
          goto done;
        }
      }
    }
  }

  done:
  // Determine if the filtering mask changed for any of the protocols
  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (temp_addr_filter_mask[i] != protocol_context[i].addr_filter_mask_802154) {
      addr_filter_mask_changed = true;
      break;
    }
  }
  // We only update the address table if it changed or the filter mask changed.
  if (addr_filter_mask_changed
      || memcmp(&rail_addresses_802154,
                &temp_addr_config,
                sizeof(sl_rail_ieee802154_addr_config_t)) != 0) {
    for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
      protocol_context[i].addr_filter_mask_802154 = temp_addr_filter_mask[i];
    }
    memcpy(&rail_addresses_802154, &temp_addr_config, sizeof(sl_rail_ieee802154_addr_config_t));
    sl_rail_ieee802154_set_addresses(mux_rail_handle, &rail_addresses_802154);
  }
}

HIDDEN void fn_mux_rail_init_callback(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  uint8_t i;

  bool initStartedFlag = fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_STARTED);
  EFM_ASSERT(initStartedFlag);
  bool initCompletedFlag = fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_COMPLETED);
  EFM_ASSERT(!initCompletedFlag);

  fn_set_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_COMPLETED, true);

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_INIT_CB_PENDING)) {
      if (protocol_context[i].init_callback != NULL) {
        protocol_context[i].init_callback(&protocol_context[i]);
      }
    }
  }
}

HIDDEN void fn_timer_callback(struct sl_rail_multi_timer *tmr,
                              sl_rail_time_t expectedTimeOfEvent,
                              void *cbArg)
{
  (void)tmr;
  (void)expectedTimeOfEvent;

  sl_rail_handle_t railHandle = (sl_rail_handle_t)cbArg;
  uint8_t context_index = fn_get_context_index(railHandle);

  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);

  if (protocol_context[context_index].timer_callback != NULL) {
    protocol_context[context_index].timer_callback(railHandle);
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static bool fn_operation_in_progress(uint16_t operation_flags)
{
  uint8_t i;

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    if (fn_get_context_flag_by_index(i, operation_flags)) {
      return true;
    }
  }

  return false;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static uint8_t fn_get_active_tx_context_index(void)
{
  uint8_t active_tx_context_index = INVALID_CONTEXT_INDEX;
  uint8_t i;

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    // Protocol context not in use
    if (protocol_context[i].rail_config == NULL) {
      continue;
    }

    if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_ACTIVE_TX_FLAGS)) {
      // Enforce that only one protocol could be in "active TX" state
      EFM_ASSERT(active_tx_context_index == INVALID_CONTEXT_INDEX);
      active_tx_context_index = i;
    }
  }

  return active_tx_context_index;
}

sl_rail_status_t sl_rail_mux_SetStateTiming(sl_rail_handle_t railHandle,
                                            sl_rail_state_timing_t *timings)
{
  (void) railHandle;
  return sl_rail_set_state_timing(mux_rail_handle, timings);
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetRxToEnhAckTx(sl_rail_handle_t railHandle,
                                                        sl_rail_transition_time_t *pRxToEnhAckTx)
{
  (void) railHandle;
  return sl_rail_ieee802154_set_rx_to_enh_ack_tx(mux_rail_handle, pRxToEnhAckTx);
}
#ifdef HIGH_DATARATE_PHY
#define not_high_datarate_packet() (packet_details.channel <= 26)
#define high_datarate_packet() (packet_details.channel > 26)
static uint8_t high_datarate_phy_index = 0xFF;
void sl_rail_mux_set_high_datarate_phy_index(sl_rail_handle_t railHandle)
{
  high_datarate_phy_index = fn_get_context_index(railHandle);
}
#else // !HIGH_DATARATE_PHY
#define not_high_datarate_packet() true
#define high_datarate_packet() false
#endif //HIGH_DATARATE_PHY

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
HIDDEN void fn_mux_rail_events_callback(sl_rail_handle_t railHandle, sl_rail_events_t events)
{
  (void)railHandle;
  sl_rail_rx_packet_info_t rx_info, data_req_Info;
  sl_rail_rx_packet_handle_t rx_packet_handle;
  sl_rail_rx_packet_details_t packet_details;
  bool start_pending_tx = false;
  bool is_beacon = false;
  uint8_t active_tx_protocol_index = fn_get_active_tx_context_index();
  uint8_t i;

  if (events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
    rx_packet_handle = sl_rail_get_rx_packet_info(mux_rail_handle,
                                                  SL_RAIL_RX_PACKET_HANDLE_NEWEST,
                                                  &rx_info);
    sl_rail_status_t getRxPacketDetailsAltStatus = sl_rail_get_rx_packet_details(mux_rail_handle,
                                                                                 rx_packet_handle,
                                                                                 &packet_details);
    EFM_ASSERT(getRxPacketDetailsAltStatus == SL_RAIL_STATUS_NO_ERROR);
    //need to read phy header (1 byte) + framecontrol (2 bytes)
    if (not_high_datarate_packet()) {
      uint8_t macHdr[SL_802154_HEADER_SEQUENCE_NUMBER_OFFSET];
      uint8_t *rxPacket = macHdr;
      uint16_t sizeofRxPacket = sl_rail_peek_rx_packet(mux_rail_handle,
                                                       rx_packet_handle,
                                                       macHdr,
                                                       sizeof(macHdr),
                                                       0);
      EFM_ASSERT(sizeofRxPacket == sizeof(macHdr));
      is_beacon = sli_mac_flat_frame_type(rxPacket, true) == SL_802154_HEADER_FC_FRAME_TYPE_BEACON;
    }
  }

  if (events & SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND) {
    sl_rail_get_rx_incoming_packet_info(railHandle, &data_req_Info);
  }

  // Bubble up only events that should be raised to each protocol.
  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    sl_rail_events_t enabled_events = (protocol_context[i].events & events);

    // This protocol context is not in use
    if (protocol_context[i].rail_config == NULL) {
      continue;
    }

    // This code executes when we switch out of 15.4 in a DMP scenario
    // This is the correct place to handle errors that would result in incomplete transmits and receives
    // that would otherwise get the statemachine stuck; Note that some of these operations were previously
    // being done upon receiving a configunscheduled event. While this might work, it is a bit overzealous
    if ( events & SL_RAIL_EVENT_SCHEDULER_STATUS ) {
      sl_rail_scheduler_status_t scheduler_status;
      sl_rail_status_t rail_status;
      sl_rail_get_scheduler_status(railHandle, &scheduler_status, &rail_status);
      if ( scheduler_status != SL_RAIL_SCHEDULER_STATUS_NO_ERROR) {
        // If we were waiting for an ACK: we will never get the ACK we were waiting for
        // Clear flags to match lower mac statemachine and allow things to go back to idle
        // and retry upon resuming 15.4
        if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK)) {
          fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK, false);
        }

        // If transmit was in progress, clear flags to match lower mac statemachine
        // and allow things to go back to idle; retries if any, will happen when things
        // resume
        if ( fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS)) {
          fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS, false);
        }

        // If tx was scheduled, DMP interruption will no longer allow it. clear flags
        if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED)) {
          fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED, false);
        }
      }
    }
    if (enabled_events & SL_RAIL_EVENTS_TX_COMPLETION) {
      if (i != active_tx_protocol_index) {
        enabled_events &= ~SL_RAIL_EVENTS_TX_COMPLETION;
      } else {
        // Note: In case of repeated transmissions, we do not want to mess with
        // buffers as this may cause corruption with the next packet. Wait until all
        // packets are sent
        if ( !sl_rail_get_tx_packets_remaining(mux_rail_handle) ) {
          uint16_t unused_tail = protocol_context[i].fifo_tx_info.tx_size - protocol_context[i].fifo_tx_info.tx_init_length;
          uint16_t unused = sl_rail_get_tx_fifo_space_available(mux_rail_handle);
          if (unused > unused_tail) {
            uint16_t unused_head = unused - unused_tail;
            memmove(protocol_context[i].fifo_tx_info.data_ptr,
                    protocol_context[i].fifo_tx_info.data_ptr + unused_head,
                    protocol_context[i].fifo_tx_info.tx_size - unused);
          }

          protocol_context[i].fifo_tx_info.tx_init_length =
            protocol_context[i].fifo_tx_info.tx_size - unused;

          if (!sl_rail_get_tx_packets_remaining(mux_rail_handle)) {
            fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS, false);
            fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED, false);
          }
          if (!(enabled_events & SL_RAIL_EVENT_TX_PACKET_SENT)) {
            // Any event other than a successful packet sent would unset the wait
            // for ack flag.
            fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK, false);
          }

          if (!tx_in_progress()) {
            start_pending_tx = true;
          }
        }
      }
    }

    // We got a SCHEDULED_TX_STARTED event: we transition the protocol that
    // originally invoked the StartScheduledTX() API to "tx in progress".
    if (events & SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED) {
      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED)) {
        fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS, true);
        fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED, false);
      } else {
        enabled_events &= ~SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED;
      }
    }

    if (events & SL_RAIL_EVENT_RX_SCHEDULED_RX_END) {
      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_RX_SCHEDULED)) {
        fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_RX_SCHEDULED, false);
      } else {
        enabled_events &= ~SL_RAIL_EVENT_RX_SCHEDULED_RX_END;
      }
    }

    if ((enabled_events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) && (sli_legacy_mfglib_mode == MFGLIB_OFF)) {
      // If we are not in mfglib mode and
      // The protocol is currently on a different channel or idling or the
      // packet did not satisfy any of the protocol filtering: we mask out the
      // SL_RAIL_EVENT_RX_PACKET_RECEIVED event.
      if ( not_high_datarate_packet() ) {
        if ( (CHANNEL_CHECK != protocol_context[i].channel && (!packet_details.is_ack || (CHANNEL_CHECK != protocol_context[i].csma_tx_info.channel)))
             // MAC acks and beacons do not contain any addressing information
             // Do not check for filterMask on Rx packets that are acks or beacons
             || (!packet_details.is_ack && !is_beacon && (rx_info.filter_mask & protocol_context[i].addr_filter_mask_802154) == 0)
             || (packet_details.is_ack && (i != active_tx_protocol_index)) ) {
          enabled_events &= ~SL_RAIL_EVENT_RX_PACKET_RECEIVED;
        } else {
          if (packet_details.is_ack) {
            fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK, false);
            start_pending_tx = true;
          }
        }
      }
      #ifdef HIGH_DATARATE_PHY
      else if (high_datarate_phy_index != 0xFF) {
        protocol_context[high_datarate_phy_index].rail_config->events_callback(&protocol_context[high_datarate_phy_index],
                                                                               enabled_events);
        enabled_events &= ~SL_RAIL_EVENT_RX_PACKET_RECEIVED;
        break; // Call the packet received callback for high_datarate_phy packet only once
      }
      #endif //HIGH_DATARATE_PHY
    }

    // Deal with ACK timeout after possible RX completion in case RAIL
    // notifies us of the ACK and the timeout simultaneously -- we want
    // the ACK to win over the timeout.
    if (enabled_events & (SL_RAIL_EVENT_RX_ACK_TIMEOUT)) {
      if ( i != active_tx_protocol_index) {
        enabled_events &= ~(SL_RAIL_EVENT_RX_ACK_TIMEOUT);
      } else if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK)) {
        fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK, false);
        start_pending_tx = true;
      }
    }

    if (enabled_events & SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND) { // ToDo: do we also need to check the packet type?
      if (CHANNEL_CHECK != protocol_context[i].channel
          || (data_req_Info.filter_mask & protocol_context[i].addr_filter_mask_802154) == 0) {
        enabled_events &= ~SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND;
      } else {
      }
    }

    if ((enabled_events) != 0) {
      protocol_context[i].rail_config->events_callback(&protocol_context[i],
                                                       enabled_events);
    }
  }

  // After all protocol callbacks, if TX completed and there's a pending power change, apply it
  if ((events & SL_RAIL_EVENTS_TX_COMPLETION) && !tx_in_progress() && pending_power_context != INVALID_CONTEXT_INDEX) {
    RAIL_SetTxPowerDbm(mux_rail_handle, protocol_context[pending_power_context].tx_power);
    current_tx_power = protocol_context[pending_power_context].tx_power;
    pending_power_context = INVALID_CONTEXT_INDEX;
  }

  if (start_pending_tx) {
    fn_start_pending_tx();
  }
}

// TODO: We do not prioritize the next protocol within this function if we need
// any kind of priority taking place we need to add that in.
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static sl_rail_status_t fn_start_pending_tx(void)
{
  uint8_t i;

  // We are currently TXing, nothing to do.
  if (tx_in_progress()) {
    return SL_RAIL_STATUS_NO_ERROR;
  }

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    // Pending scheduled TX
    if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULED_TX_PENDING)) {
      sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;
      if ( !check_lock_permissions(i)) {
        // Post a tx blocked event to notify mac state machines
        fn_mux_rail_events_callback(mux_rail_handle, SL_RAIL_EVENT_TX_BLOCKED);
        continue;
      }
      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_CONFIG_REPEATED_TX)) {
        sl_rail_set_next_tx_repeat(mux_rail_handle, &protocol_context[i].tx_repeat_config);
        fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_CONFIG_REPEATED_TX, false);
      }

      // Set TX power for this specific protocol context before setting up TX FIFO
      fn_set_tx_power_for_context(i);

      //TODO: we might need to check if there is already scheduled TX, and it is by somebody with higher priority?
      // a new scheduled TX can overwrite the existing one with no priority considerataions
      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SETUP_TX_FIFO)) {
        sl_rail_set_tx_fifo(mux_rail_handle, (sl_rail_fifo_buffer_align_t*)protocol_context[i].fifo_tx_info.data_ptr, protocol_context[i].fifo_tx_info.tx_size,
                            protocol_context[i].fifo_tx_info.tx_init_length,
                            0);
      }
      // Set TX power for this specific protocol context before starting TX
      fn_set_tx_power_for_context(i);

      fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_SCHEDULED, true);

      if (protocol_context[i].csma_tx_info.tx_type == SCHEDULED_TX_CCA_CSMA_REQUEST) {
        status = sl_rail_start_scheduled_cca_csma_tx(mux_rail_handle,
                                                     protocol_context[i].csma_tx_info.channel,
                                                     protocol_context[i].csma_tx_info.options,
                                                     (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULE_TX_CONFIG_VALID)
                                                      ? &protocol_context[i].csma_tx_info.scheduler_config : NULL),
                                                     (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_CSMA_CONFIG_VALID)
                                                      ? &protocol_context[i].csma_tx_info.csmaConfig : NULL),
                                                     (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID)
                                                      ? &protocol_context[i].csma_tx_info.schedulerInfo : NULL));
      } else if (protocol_context[i].csma_tx_info.tx_type == SCHEDULED_TX_REQUEST) {
        status = sl_rail_start_scheduled_tx(mux_rail_handle,
                                            protocol_context[i].csma_tx_info.channel,
                                            protocol_context[i].csma_tx_info.options,
                                            (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULE_TX_CONFIG_VALID)
                                             ? &protocol_context[i].csma_tx_info.scheduler_config : NULL),
                                            (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID)
                                             ? &protocol_context[i].csma_tx_info.schedulerInfo : NULL));
      } else {
        // TX type corrupted?
        EFM_ASSERT(0);
      }

      fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULED_TX_PENDING, false);

      // This would result to assert in lower-mac code
      EFM_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

      return SL_RAIL_STATUS_NO_ERROR;
    }

    // Pending normal TX
    if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING)) {
      sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

      if ( !check_lock_permissions(i)) {
        // Post a tx blocked event to notify mac state machines
        fn_mux_rail_events_callback(mux_rail_handle, SL_RAIL_EVENT_TX_BLOCKED);
        continue;
      }
      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_CONFIG_REPEATED_TX)) {
        sl_rail_set_next_tx_repeat(mux_rail_handle, &protocol_context[i].tx_repeat_config);
        fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_CONFIG_REPEATED_TX, false);
      }

      // Set TX power for this specific protocol context before setting up TX FIFO
      fn_set_tx_power_for_context(i);

      if (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SETUP_TX_FIFO)) {
        // We have to keep calling the setTxFifo() before each call to
        // sl_rail_write_tx_fifo() since  protocol B might have reset/recreated its
        // own txFifo buffer in between each call to writeTxFifo by protocol A
        sl_rail_set_tx_fifo(mux_rail_handle, (sl_rail_fifo_buffer_align_t*)protocol_context[i].fifo_tx_info.data_ptr,
                            protocol_context[i].fifo_tx_info.tx_size,
                            protocol_context[i].fifo_tx_info.tx_init_length,
                            0);
      }
      // Set TX power for this specific protocol context before starting TX
      fn_set_tx_power_for_context(i);

      fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS, true);

      if (protocol_context[i].csma_tx_info.options & SL_RAIL_TX_OPTION_WAIT_FOR_ACK) {
        fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK, true);
      }

      if (protocol_context[i].csma_tx_info.tx_type == SINGLE_TX_CCA_CSMA_REQUEST) {
        status = sl_rail_start_cca_csma_tx(mux_rail_handle,
                                           protocol_context[i].csma_tx_info.channel,
                                           protocol_context[i].csma_tx_info.options,
                                           (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_CSMA_CONFIG_VALID)
                                            ? &protocol_context[i].csma_tx_info.csmaConfig : NULL),
                                           (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID)
                                            ? &protocol_context[i].csma_tx_info.schedulerInfo : NULL));
      } else if (protocol_context[i].csma_tx_info.tx_type == SINGLE_TX_REQUEST) {
        status = sl_rail_start_tx(mux_rail_handle,
                                  protocol_context[i].csma_tx_info.channel,
                                  protocol_context[i].csma_tx_info.options,
                                  (fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SCHEDULER_INFO_VALID)
                                   ? &protocol_context[i].csma_tx_info.schedulerInfo  : NULL));
      } else {
        // TX type corrupted?
        EFM_ASSERT(0);
      }

      fn_set_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING, false);

      // This would result to assert in lower-mac code
      EFM_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

      return SL_RAIL_STATUS_NO_ERROR;
    }
  }

  return SL_RAIL_STATUS_NO_ERROR;
}

// Set TX power for a specific protocol context
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static void fn_set_tx_power_for_context(uint8_t context_index)
{
  if (protocol_context[context_index].tx_power != current_tx_power) {
    current_tx_power = protocol_context[context_index].tx_power;
    RAIL_SetTxPowerDbm(mux_rail_handle, current_tx_power);
    // Clear pending power if we just applied it
    if (pending_power_context == context_index) {
      pending_power_context = INVALID_CONTEXT_INDEX;
    }
  }
}

#ifdef SL_ZIGBEE_TEST
sl_rail_util_ieee802154_stack_status_t sl_rail_mux_ieee802154_on_event(
  sl_rail_handle_t railHandle,
  sl_rail_util_ieee802154_stack_event_t stack_event,
  uint32_t supplement)
{
  UNUSED_VAR(railHandle);
  UNUSED_VAR(stack_event);
  UNUSED_VAR(supplement);

  return SL_RAIL_UTIL_IEEE802154_STACK_STATUS_SUCCESS;
}
#else //!SL_ZIGBEE_TEST
static bool check_event_filter(uint8_t context_index,
                               uint16_t flag,
                               bool enable)
{
  fn_set_context_flag_by_index(context_index, flag, enable);

  if (!enable) {
    for (uint8_t i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
      if (fn_get_context_flag_by_index(i, flag)) {
        // filter stop event since the radio operation
        // is in progress on another stack
        return false;
      }
    }
  }
  return true;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
static bool filter_stack_event(uint8_t context_index,
                               sl_rail_util_ieee802154_stack_event_t stack_event)
{
  switch (stack_event) {
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_PENDED_PHY:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_PENDED_MAC:
      return check_event_filter(context_index,
                                RAIL_MUX_PROTOCOL_FLAGS_STACK_EVENT_TX_ACTIVE,
                                true);
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_WAITING:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_RECEIVED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_TIMEDOUT:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_BLOCKED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ABORTED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ENDED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_IDLED:
      return check_event_filter(context_index,
                                RAIL_MUX_PROTOCOL_FLAGS_STACK_EVENT_TX_ACTIVE,
                                false);
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_LISTEN:
      return check_event_filter(context_index,
                                RAIL_MUX_PROTOCOL_FLAGS_STACK_EVENT_RADIO_ACTIVE,
                                true);
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_IDLED:
      return check_event_filter(context_index,
                                RAIL_MUX_PROTOCOL_FLAGS_STACK_EVENT_RADIO_ACTIVE,
                                false);
    default:
      return true;
  }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_util_ieee802154_stack_status_t sl_rail_mux_ieee802154_on_event(
  sl_rail_handle_t railHandle,
  sl_rail_util_ieee802154_stack_event_t stack_event,
  uint32_t supplement)
{
  RAIL_MUX_DECLARE_IRQ_STATE;

  sl_rail_status_t status = SL_RAIL_UTIL_IEEE802154_STACK_STATUS_SUCCESS;
  uint8_t context_index = fn_get_context_index(railHandle);

  RAIL_MUX_ENTER_CRITICAL();
  if (filter_stack_event(context_index, stack_event)) {
    status = sl_rail_util_ieee802154_on_event(stack_event, supplement);
  }
  RAIL_MUX_EXIT_CRITICAL();

  return status;
}
#endif

#ifdef PRINT_DEBUG_ON
uint16_t fn_get_tx_init(uint8_t index)
{
  return protocol_context[index].fifo_tx_info.tx_init_length;
}
void fn_print_flags(void)
{
  uint8_t i;

  fprintf(stderr, "\nGlobal Flags: %x %x %x \n", fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_STARTED),
          fn_get_global_flag(RAIL_MUX_FLAGS_RAIL_INIT_COMPLETED),
          fn_get_global_flag(RAIL_MUX_FLAGS_IEEE802154_INIT_COMPLETED));

  for (i = 0; i < SUPPORTED_PROTOCOL_COUNT; i++) {
    fprintf(stderr, "\nContext %d Flags: %x %x %x %x %x\n", i, fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_INIT_CB_PENDING),
            fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_SETUP_TX_FIFO),
            fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_START_TX_PENDING),
            fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_TX_IN_PROGRESS),
            fn_get_context_flag_by_index(i, RAIL_MUX_PROTOCOL_FLAGS_WAIT_FOR_ACK) );
  }
}
#endif

void sl_rail_mux_update_active_radio_config(void)
{
  sl_rail_idle(mux_rail_handle, SL_RAIL_IDLE_ABORT, true);
  sl_rail_util_ieee802154_config_radio(mux_rail_handle);
  if (rx_channel != INVALID_CHANNEL) {
    CONFIGURE_RX_CHANNEL_SWITCHING(mux_rail_handle, channel_switching_cfg);
    sl_rail_start_rx(mux_rail_handle, rx_channel, NULL);
  }
}

sl_rail_status_t sl_rail_mux_IEEE802154_EnableDataFramePending(sl_rail_handle_t railHandle,
                                                               bool enable)
{
  (void)railHandle;
  return sl_rail_ieee802154_enable_data_frame_pending(mux_rail_handle, enable);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
uint16_t sl_rail_mux_GetTxPacketsRemaining(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return(sl_rail_get_tx_packets_remaining(mux_rail_handle));
}

sl_rail_status_t sl_rail_mux_ResetFifo(sl_rail_handle_t railHandle, bool txFifo, bool rxFifo)
{
  (void)railHandle;
  return sl_rail_reset_fifo(mux_rail_handle, txFifo, rxFifo);
}

sl_rail_status_t sl_rail_mux_ConfigRxDutyCycle(sl_rail_handle_t railHandle,
                                               const sl_rail_rx_duty_cycle_config_t *config)
{
  (void)railHandle;
  return sl_rail_config_rx_duty_cycle(mux_rail_handle, config);
}

sl_rail_status_t sl_rail_mux_EnableRxDutyCycle(sl_rail_handle_t railHandle,
                                               bool enable)
{
  (void)railHandle;
  return sl_rail_enable_rx_duty_cycle(mux_rail_handle, enable);
}

uint16_t sl_rail_mux_GetTxFifoSpaceAvailable(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_get_tx_fifo_space_available(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_AcceptFrames(sl_rail_handle_t railHandle,
                                                     uint8_t framesMask)
{
  (void)railHandle;
  return sl_rail_ieee802154_accept_frames(mux_rail_handle, framesMask);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivCoex(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_coex(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivCoexFem(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_coex_fem(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioAntDivFem(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_2p4_ghz_radio_ant_div_fem(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioCoex(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_2p4_ghz_radio_coex(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioCoexFem(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_2p4_ghz_radio_coex_fem(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_Config2p4GHzRadioFem(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_2p4_ghz_radio_fem(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_IEEE802154_ConfigCcaMode(sl_rail_handle_t railHandle,
                                                      sl_rail_ieee802154_cca_mode_t ccaMode)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_cca_mode(mux_rail_handle, ccaMode);
}

sl_rail_status_t sl_rail_mux_IEEE802154_ConfigEOptions(sl_rail_handle_t railHandle,
                                                       sl_rail_ieee802154_e_options_t mask,
                                                       sl_rail_ieee802154_e_options_t options)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_e_options(mux_rail_handle, mask, options);
}

sl_rail_status_t sl_rail_mux_IEEE802154_ConfigGOptions(sl_rail_handle_t railHandle,
                                                       sl_rail_ieee802154_g_options_t mask,
                                                       sl_rail_ieee802154_g_options_t options)
{
  (void)railHandle;
  return sl_rail_ieee802154_config_g_options(mux_rail_handle, mask, options);
}

sl_rail_status_t sl_rail_mux_IEEE802154_SetPtiRadioConfig(sl_rail_handle_t railHandle,
                                                          sl_rail_ieee802154_phy_t ptiRadioConfigId)
{
  (void)railHandle;
  return sl_rail_ieee802154_set_phy_id(mux_rail_handle, ptiRadioConfigId);
}

sl_rail_status_t sl_rail_mux_SetRssiOffset(sl_rail_handle_t railHandle, int8_t rssiOffset)
{
  (void)railHandle;
  return sl_rail_set_rssi_offset(mux_rail_handle, rssiOffset);
}

sl_rail_status_t sl_rail_mux_StartTxStreamAlt(sl_rail_handle_t railHandle,
                                              uint16_t channel,
                                              sl_rail_stream_mode_t mode,
                                              sl_rail_tx_options_t options)
{
  RAIL_MUX_DECLARE_IRQ_STATE;
  uint8_t context_index = fn_get_context_index(railHandle);
  EFM_ASSERT(context_index < SUPPORTED_PROTOCOL_COUNT);
  sl_rail_status_t ret_status = SL_RAIL_STATUS_INVALID_STATE;

  RAIL_MUX_ENTER_CRITICAL();
  bool is_ok_to_proceed = check_lock_permissions(context_index);

  if ( is_ok_to_proceed ) {
    // Set TX power for this specific protocol context before starting TX stream
    // Must be done inside critical section to avoid race conditions
    fn_set_tx_power_for_context(context_index);
  }

  RAIL_MUX_EXIT_CRITICAL();

  if ( is_ok_to_proceed ) {
    ret_status = sl_rail_start_tx_stream(mux_rail_handle, channel, mode, options);
  }

  return ret_status;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_RAIL_MUX_15_4, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_status_t sl_rail_mux_GetRxTimeFrameEnd(sl_rail_handle_t railHandle,
                                               sl_rail_rx_packet_details_t *pPacketdetails)
{
  (void)railHandle;
  return sl_rail_get_rx_time_frame_end(mux_rail_handle,
                                       pPacketdetails);
}

bool sl_rail_mux_IsNextCcaNow(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_is_next_cca_now(mux_rail_handle);
}

sl_rail_status_t sl_rail_mux_util_ieee802154_config_radio(sl_rail_handle_t railHandle)
{
  (void)railHandle;
  return sl_rail_util_ieee802154_config_radio(mux_rail_handle);
}

#if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_PRESENT) || defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_PRESENT)
sl_rail_ieee802154_phy_features_t duty_cycling_phy_features = SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_DUTY_CYCLING;
#else
sl_rail_ieee802154_phy_features_t duty_cycling_phy_features = SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ;
#endif
sl_rail_ieee802154_phy_features_t sl_rail_util_ieee802154_get_fast_channel_switching_phy_features(void)
{
  return sli_is_multi_channel_enabled() ? SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ_RX_CH_SWITCHING : duty_cycling_phy_features;
}
sl_rail_ieee802154_phy_features_t sl_rail_util_ieee802154_get_rx_duty_cycling_phy_features(void)
{
  // if fast channel switching is enabled, we don't want to load duty cycling phy
  #if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT) || defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT)
  return SL_RAIL_IEEE802154_PHY_FEATURE_2P4_GHZ;
  #else
  return duty_cycling_phy_features;
  #endif
}
