/***************************************************************************//**
 * @file
 * @brief Convenience stubs for endpoint APIs.
 *
 *
 * \b Note: Application developers should \b not modify any portion
 * of this file. Doing so may lead to mysterious bugs. Allocations should be
 * adjusted only with macros in a custom CONFIGURATION_HEADER.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "hal.h"
#include "stack/include/sl_zigbee.h"
#include "stack/include/sl_zigbee_types_internal.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#if !defined(SL_ZIGBEE_APPLICATION_HAS_GET_ENDPOINT) && !defined(SL_ZIGBEE_AF_NCP) && !defined(__NCP_CONFIG__)
uint8_t sli_zigbee_stack_get_endpoint(uint8_t index)
{
  return sl_zigbee_endpoints[index].endpoint;
}

bool sli_zigbee_stack_get_endpoint_description(uint8_t endpoint,
                                               sl_zigbee_endpoint_description_t *result)
{
  const sl_zigbee_endpoint_t *endpoints = sl_zigbee_endpoints;
  for (uint8_t i = 0; i < sl_zigbee_endpoint_count; i++, endpoints++) {
    if (endpoints->endpoint == endpoint) {
      sl_zigbee_endpoint_description_t const * d = endpoints->description;
      result->profileId                   = d->profileId;
      result->deviceId                    = d->deviceId;
      result->deviceVersion               = d->deviceVersion;
      result->inputClusterCount           = d->inputClusterCount;
      result->outputClusterCount          = d->outputClusterCount;
      return true;
    }
  }
  return false;
}

uint16_t sli_zigbee_stack_get_endpoint_cluster(uint8_t endpoint,
                                               sl_zigbee_cluster_list_id_t listId,
                                               uint8_t listIndex)
{
  const sl_zigbee_endpoint_t *endpoints = sl_zigbee_endpoints;
  for (uint8_t i = 0; i < sl_zigbee_endpoint_count; i++, endpoints++) {
    if (endpoints->endpoint == endpoint) {
      switch (listId) {
        case SL_ZIGBEE_INPUT_CLUSTER_LIST:
          return endpoints->inputClusterList[listIndex];
        case SL_ZIGBEE_OUTPUT_CLUSTER_LIST:
          return endpoints->outputClusterList[listIndex];
        default:
          break;
      }
    }
  }
  return 0;
}

uint8_t sli_zigbee_stack_get_endpoint_count(void)
{
  return sl_zigbee_endpoint_count;
}

#endif // defined SL_ZIGBEE_APPLICATION_HAS_GET_ENDPOINT
