/***************************************************************************//**
 * @file
 * @brief implements sli_zigbee_stack_populate_counters() and keeps a tally
 * of the events reported by the stack.  The application must define
 * SL_ZIGBEE_APPLICATION_HAS_COUNTER_HANDLER in its configuration header
 * to use this file.
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

#include "sl_code_classification.h"

#include "stack/include/sl_zigbee.h"
#include "stack/internal/inc/internal-defs-patch.h"

#include "counters.h"

uint16_t sli_zigbee_counters[SL_ZIGBEE_COUNTER_TYPE_COUNT];
uint16_t sli_zigbee_counters_thresholds[SL_ZIGBEE_COUNTER_TYPE_COUNT];
#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
uint16_t sli_zigbee_multi_network_counters[SL_ZIGBEE_MAX_SUPPORTED_NETWORKS]
[MULTI_NETWORK_COUNTER_TYPE_COUNT];
static uint8_t getMultiNetworkCounterIndex(sl_zigbee_counter_type_t type);
static void multiNetworkCounterHandler(sl_zigbee_counter_type_t type, uint8_t data);
#endif // SL_ZIGBEE_MULTI_NETWORK_STRIPPED

uint16_t sli_zigbee_mac_counters[SL_ZIGBEE_MAX_NUM_PHYS][SL_ZIGBEE_MAX_MAC_ONLY_COUNTERS];

//Implement the stack callback by simply tallying up the counts.
// TODO: Once we fully port the pro-compliance app to UC, we can
// get rid of the following and just use the new name for this callback.
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
void sli_zigbee_stack_populate_counters(sl_zigbee_counter_type_t type, sl_zigbee_counter_info_t info)
{
  bool phyIndexRequiredCounter = sli_zigbee_stack_counter_requires_phy_index(type);

  bool destionationRequiredCounter = sli_zigbee_stack_counter_requires_destination_node_id(type);

  uint8_t data = info.data;

  uint8_t phyIndex = 0;

  sl_802154_short_addr_t destinationNodeId = SL_ZIGBEE_NULL_NODE_ID;

  // retreive possible counter info

  if (phyIndexRequiredCounter

      && destionationRequiredCounter) {
    phyIndex = ((sl_zigbee_extra_counters_info_t *) (info.otherFields))->phy_index;

    destinationNodeId = ((sl_zigbee_extra_counters_info_t *) (info.otherFields))->destinationNodeId;
  } else if (destionationRequiredCounter) {
    destinationNodeId = *((sl_802154_short_addr_t*)info.otherFields);
  } else if (phyIndexRequiredCounter) {
    phyIndex = *((uint8_t*)info.otherFields);
  }

  //update the counter statistics

  if (phyIndexRequiredCounter && (type < SL_ZIGBEE_MAX_MAC_ONLY_COUNTERS)) {
    if (phyIndex < SL_ZIGBEE_MAX_NUM_PHYS) {
      if (sli_zigbee_mac_counters[phyIndex][type] < 0xFFFF) {
        sli_zigbee_mac_counters[phyIndex][type]++;
      }
    } else if (phyIndex == SL_ZIGBEE_MAX_NUM_PHYS) {
      // phyIndex cannot be equal to SL_ZIGBEE_MAX_NUM_PHYS except for
      // SL_ZIGBEE_COUNTER_MAC_TX_BROADCAST counter as broadcast can be sent on all interfaces.
      uint8_t i;
      // update counter for all interfaces.
      for (i = PHY_INDEX_NATIVE; i < SL_ZIGBEE_MAX_NUM_PHYS; i++) {
        if (sli_zigbee_mac_counters[i][type] < 0xFFFF) {
          sli_zigbee_mac_counters[i][type]++;
        }
      }
    } else {
      // adding else part for MISRA
    }
  }

  //To ensure that we call the counter rollover handler exactly once.

  bool hasCounterExceededThreshold;

  if (sli_zigbee_counters[type] < sli_zigbee_counters_thresholds[type]) {
    hasCounterExceededThreshold = false;
  } else {
    hasCounterExceededThreshold = true;
  }

  if (sli_zigbee_counters[type] < 0xFFFF) {
    sli_zigbee_counters[type] += 1;
  }

  if (SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_SUCCESS == type

      || SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_FAILED == type) {
    if ((sli_zigbee_counters[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY] + data)

        < sli_zigbee_counters_thresholds[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY]) {
      sli_zigbee_counters[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY] += data;
    } else {
      sli_zigbee_counters[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY] =

        sli_zigbee_counters_thresholds[SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY];
    }
  } else if (SL_ZIGBEE_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED == type) {
    if ((sli_zigbee_counters[type] + data) < sli_zigbee_counters_thresholds[type]) {
      // (data - 1) as counter is already incremented once above.
      // Note: We log this counter only if data > zero so (data - 1) won't roll
      // over back to 255.
      sli_zigbee_counters[type] += (data - 1);
    } else {
      sli_zigbee_counters[type] = sli_zigbee_counters_thresholds[type];
    }
  }

  if (sli_zigbee_counters[type] >= sli_zigbee_counters_thresholds[type]) {
    if (!hasCounterExceededThreshold) {
      sl_zigbee_counter_rollover_handler(type);
    }
  }

#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
  multiNetworkCounterHandler(type, data);
#endif // SL_ZIGBEE_MULTI_NETWORK_STRIPPED

  if (destionationRequiredCounter
      && destinationNodeId == SL_ZIGBEE_NULL_NODE_ID) {
    //this might be an error
    return;
  }
}

sl_status_t sli_zigbee_stack_clear_counters(void)
{
  memset(sli_zigbee_counters, 0, sizeof(sli_zigbee_counters));
#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
  memset(sli_zigbee_multi_network_counters, 0, sizeof(sli_zigbee_multi_network_counters));
#endif // SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_stack_reset_counters_thresholds(void)
{
  memset(sli_zigbee_counters_thresholds, 0xFF, sizeof(sli_zigbee_counters_thresholds));
  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_stack_set_counter_threshold(sl_zigbee_counter_type_t type, uint16_t threshold)
{
  sli_zigbee_counters_thresholds[type] = threshold;
  return SL_STATUS_OK;
}

void sli_zigbee_stack_read_and_clear_counters(
  // Return: A list of all counter values ordered according to the
  // sl_zigbee_counter_type_t enumeration.
  uint16_t *counter_array,
  uint8_t len)
{
  for (uint8_t i = 0; i < len && i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++) {
    memmove(counter_array + i, sli_zigbee_counters + i, 2);
  }
  sli_zigbee_stack_clear_counters();
}

// Retrieves Ember counters. See the sl_zigbee_counter_type_t enumeration for
// the counter types.
void sli_zigbee_stack_read_counters(
  // Return: A list of all counter values ordered according to the
  // sl_zigbee_counter_type_t enumeration.
  uint16_t *counter_array,
  uint8_t len)
{
  for (int i = 0; i < len && i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++) {
    memmove(counter_array + i, sli_zigbee_counters + i, 2);
  }
}

/*******************************************************************************
 * Multi-network counters support
 *
 * Some of the counters are per-network. Some of them are implicitly not
 * per-network because of the limited multi-network support. i.e., a node can be
 * coordinator/router/end device on just one network. The per-network counters
 * are defined in a table. The per-network counters are stored in a separate
 * two-dimensional array. We keep writing the multi-network counters also in the
 * usual single-network counters array.
 ******************************************************************************/

#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)

extern uint8_t sli_zigbee_supported_networks;

static const sl_zigbee_counter_type_t multiNetworkCounterTable[] = {
  SL_ZIGBEE_COUNTER_MAC_RX_BROADCAST,
  SL_ZIGBEE_COUNTER_MAC_TX_BROADCAST,
  SL_ZIGBEE_COUNTER_MAC_RX_UNICAST,
  SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_SUCCESS,
  SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_RETRY,
  SL_ZIGBEE_COUNTER_MAC_TX_UNICAST_FAILED,
  SL_ZIGBEE_COUNTER_APS_DATA_RX_BROADCAST,
  SL_ZIGBEE_COUNTER_APS_DATA_TX_BROADCAST,
  SL_ZIGBEE_COUNTER_APS_DATA_RX_UNICAST,
  SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_SUCCESS,
  SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY,
  SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_FAILED,
  SL_ZIGBEE_COUNTER_NWK_FRAME_COUNTER_FAILURE,
  SL_ZIGBEE_COUNTER_APS_FRAME_COUNTER_FAILURE,
  SL_ZIGBEE_COUNTER_APS_LINK_KEY_NOT_AUTHORIZED,
  SL_ZIGBEE_COUNTER_NWK_DECRYPTION_FAILURE,
  SL_ZIGBEE_COUNTER_APS_DECRYPTION_FAILURE,
};

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
static uint8_t getMultiNetworkCounterIndex(sl_zigbee_counter_type_t type)
{
  uint8_t i;
  for (i = 0; i < MULTI_NETWORK_COUNTER_TYPE_COUNT; i++) {
    if (multiNetworkCounterTable[i] == type) {
      return i;
    }
  }
  return 0xFF;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
static void multiNetworkCounterHandler(sl_zigbee_counter_type_t type, uint8_t data)
{
  uint8_t counterIndex = getMultiNetworkCounterIndex(type);

  // This function is always called in a callback context sli_zigbee_stack_populate_counters().
  // Not a multi-network counter, nothing to do.
  if (counterIndex >= MULTI_NETWORK_COUNTER_TYPE_COUNT) {
    return;
  }

  uint8_t nwkIndex = sli_zigbee_stack_get_callback_network();
  assert(nwkIndex < sli_zigbee_supported_networks);

  if (sli_zigbee_multi_network_counters[nwkIndex][counterIndex] < 0xFFFF) {
    sli_zigbee_multi_network_counters[nwkIndex][counterIndex] += 1;
  }

  // TODO: we should avoid wrapping around here
  if (SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_SUCCESS == type
      || SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_FAILED == type) {
    sli_zigbee_multi_network_counters[nwkIndex][SL_ZIGBEE_COUNTER_APS_DATA_TX_UNICAST_RETRY]
      += data;
  }
}

#endif // SL_ZIGBEE_MULTI_NETWORK_STRIPPED
