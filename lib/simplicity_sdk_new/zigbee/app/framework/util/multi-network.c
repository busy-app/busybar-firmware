/***************************************************************************//**
 * @file
 * @brief
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
#include "sl_code_classification.h"

#if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT) || defined(SL_CATALOG_ZIGBEE_DYNAMIC_NODE_TYPE_PRESENT)
// This is now non-const, so that the node type may be changed at run-time.
sli_zigbee_af_zigbee_pro_network *sli_zigbee_af_current_zigbee_pro_network = NULL;
#else
const sli_zigbee_af_zigbee_pro_network *sli_zigbee_af_current_zigbee_pro_network = NULL;
#endif

//#define NETWORK_INDEX_DEBUG
#if defined(SL_ZIGBEE_TEST) || defined(NETWORK_INDEX_DEBUG)
  #define NETWORK_INDEX_ASSERT(x) assert(x)
#else
  #define NETWORK_INDEX_ASSERT(x)
#endif

#if SL_ZIGBEE_SUPPORTED_NETWORKS == 1
sl_status_t sli_zigbee_af_initialize_network_index_stack(void)
{
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == 0);
  NETWORK_INDEX_ASSERT(SL_ZIGBEE_AF_DEFAULT_NETWORK_INDEX == 0);
  sli_zigbee_af_current_zigbee_pro_network = &sli_zigbee_af_zigbee_pro_networks[0];
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex)
{
  NETWORK_INDEX_ASSERT(networkIndex == 0);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == 0);
  return (networkIndex == 0 ? SL_STATUS_OK : SL_STATUS_INVALID_STATE);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sl_zigbee_af_push_callback_network_index(void)
{
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == 0);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_callback_network() == 0);
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_af_push_endpoint_network_index(uint8_t endpoint)
{
  uint8_t networkIndex = sl_zigbee_af_network_index_from_endpoint(endpoint);
  NETWORK_INDEX_ASSERT(networkIndex != 0xFF);
  if (networkIndex == 0xFF) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  NETWORK_INDEX_ASSERT(networkIndex == 0);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == 0);
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_af_pop_network_index(void)
{
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == 0);
  return SL_STATUS_OK;
}

void sli_zigbee_af_assert_network_index_stack_is_empty(void)
{
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == 0);
}

uint8_t sl_zigbee_af_primary_endpoint_for_network_index(uint8_t networkIndex)
{
  NETWORK_INDEX_ASSERT(networkIndex == 0);
  return (networkIndex == 0 ? sl_zigbee_af_primary_endpoint() : 0xFF);
}

uint8_t sl_zigbee_af_primary_endpoint_for_current_network_index(void)
{
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == 0);
  return sl_zigbee_af_primary_endpoint();
}

uint8_t sl_zigbee_af_network_index_from_endpoint(uint8_t endpoint)
{
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  NETWORK_INDEX_ASSERT(index != 0xFF);
  if (index == 0xFF) {
    return 0xFF;
  }
  NETWORK_INDEX_ASSERT(sl_zigbee_af_network_index_from_endpoint_index(index) == 0);
  return 0;
}

#else // if SL_ZIGBEE_SUPPORTED_NETWORKS == 1

// We use two bits to describe a network index and sixteen bits to store our
// stack of network indices.  This limits us to a maximum of four networks
// indices and a maximum of eight in our stack.  We also remember one default
// network that we resort to when our stack is empty.
static uint16_t networkIndexStack = 0;
static uint8_t networkIndices = 0;
#define NETWORK_INDEX_BITS       2
#define NETWORK_INDEX_MAX        (1 << NETWORK_INDEX_BITS)
#define NETWORK_INDEX_MASK       (NETWORK_INDEX_MAX - 1)
#define NETWORK_INDEX_STACK_SIZE (sizeof(networkIndexStack) * 8 / NETWORK_INDEX_BITS)

static sl_status_t setCurrentNetwork(void)
{
  sl_status_t status;
  uint8_t networkIndex = (networkIndices == 0
                          ? SL_ZIGBEE_AF_DEFAULT_NETWORK_INDEX
                          : networkIndexStack & NETWORK_INDEX_MASK);
  status = sl_zigbee_set_current_network(networkIndex);
  NETWORK_INDEX_ASSERT(status == SL_STATUS_OK);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == networkIndex);
  if (status == SL_STATUS_OK) {
    //TODO RF4CE removal: can we get rid of the separate pro network??
    sli_zigbee_af_current_zigbee_pro_network = &sli_zigbee_af_zigbee_pro_networks[networkIndex];
  }
  return status;
}

sl_status_t sli_zigbee_af_initialize_network_index_stack(void)
{
  sl_status_t status;
  NETWORK_INDEX_ASSERT(networkIndices == 0);
  if (networkIndices != 0) {
    return SL_STATUS_INVALID_STATE;
  }
  status = setCurrentNetwork();
  NETWORK_INDEX_ASSERT(status == SL_STATUS_OK);
  NETWORK_INDEX_ASSERT(networkIndices == 0);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == SL_ZIGBEE_AF_DEFAULT_NETWORK_INDEX);
  return status;
}

sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex)
{
  sl_status_t status;
  NETWORK_INDEX_ASSERT(networkIndex < NETWORK_INDEX_MAX);
  if (NETWORK_INDEX_MAX <= networkIndex) {
    return SL_STATUS_INVALID_INDEX;
  }
  NETWORK_INDEX_ASSERT(networkIndices < NETWORK_INDEX_STACK_SIZE);
  if (NETWORK_INDEX_STACK_SIZE <= networkIndices) {
    return SL_STATUS_FULL;
  }
  networkIndexStack <<= NETWORK_INDEX_BITS;
  networkIndexStack |= networkIndex;
  networkIndices++;
  status = setCurrentNetwork();
  NETWORK_INDEX_ASSERT(status == SL_STATUS_OK);
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == networkIndex);
  return status;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
sl_status_t sl_zigbee_af_push_callback_network_index(void)
{
  sl_status_t status = sl_zigbee_af_push_network_index(sl_zigbee_get_callback_network());
  NETWORK_INDEX_ASSERT(status == SL_STATUS_OK);
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == sl_zigbee_get_callback_network());
  return status;
}

sl_status_t sl_zigbee_af_push_endpoint_network_index(uint8_t endpoint)
{
  sl_status_t status;
  uint8_t networkIndex = sl_zigbee_af_network_index_from_endpoint(endpoint);
  NETWORK_INDEX_ASSERT(networkIndex != 0xFF);
  if (networkIndex == 0xFF) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  status = sl_zigbee_af_push_network_index(networkIndex);
  NETWORK_INDEX_ASSERT(status == SL_STATUS_OK);
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == networkIndex);
  return status;
}

sl_status_t sl_zigbee_af_pop_network_index(void)
{
  sl_status_t status;
  NETWORK_INDEX_ASSERT(0 < networkIndices);
  if (networkIndices == 0) {
    return SL_STATUS_INVALID_STATE;
  }
  networkIndexStack >>= NETWORK_INDEX_BITS;
  networkIndices--;
  status = setCurrentNetwork();
  NETWORK_INDEX_ASSERT(status == SL_STATUS_OK);
  return status;
}

void sli_zigbee_af_assert_network_index_stack_is_empty(void)
{
  NETWORK_INDEX_ASSERT(networkIndices == 0);
  NETWORK_INDEX_ASSERT(sl_zigbee_get_current_network() == SL_ZIGBEE_AF_DEFAULT_NETWORK_INDEX);
}

uint8_t sl_zigbee_af_primary_endpoint_for_network_index(uint8_t networkIndex)
{
  uint8_t i;
  NETWORK_INDEX_ASSERT(networkIndex < NETWORK_INDEX_MAX);
  for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
    if (sl_zigbee_af_network_index_from_endpoint_index(i) == networkIndex) {
      return sl_zigbee_af_endpoint_from_index(i);
    }
  }
  return 0xFF;
}

uint8_t sl_zigbee_af_primary_endpoint_for_current_network_index(void)
{
  return sl_zigbee_af_primary_endpoint_for_network_index(sl_zigbee_get_current_network());
}

uint8_t sl_zigbee_af_network_index_from_endpoint(uint8_t endpoint)
{
  uint8_t index = sl_zigbee_af_index_from_endpoint(endpoint);
  NETWORK_INDEX_ASSERT(index != 0xFF);
  return (index == 0xFF ? 0xFF : sl_zigbee_af_network_index_from_endpoint_index(index));
}

#endif // SL_ZIGBEE_SUPPORTED_NETWORKS
