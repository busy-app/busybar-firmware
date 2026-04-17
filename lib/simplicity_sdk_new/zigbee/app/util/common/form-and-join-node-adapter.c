/***************************************************************************//**
 * @file
 * @brief this file adapts the form-and-join library to work
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

#include PLATFORM_HEADER     // Micro and compiler specific typedefs and macros

#include "stack/include/sl_zigbee.h"
#include "hal/hal.h" // for ATOMIC()
#include "form-and-join.h"
#include "form-and-join-adapter.h"
#include "app/framework/include/af.h"

#if defined(SL_ZIGBEE_SCRIPTED_TEST)
  #define HIDDEN
#else
  #define HIDDEN static
#endif

// We use this flat buffer for caching energy scan results,
// pan id candidates, and joinable beacons.
HIDDEN uint16_t dataCache[FORM_AND_JOIN_MAX_NETWORKS * (NETWORK_STORAGE_SIZE / 2)];

#if !defined(SL_ZIGBEE_AF_PLUGIN_FORM_AND_JOIN)
// We are NOT compiled in the context of an App Framework application
// (e.g. Sensor/sink) and the event is the normal name here.
HIDDEN sl_zigbee_event_control_t cleanupEvent;
#else
// Compiled in the context of an Afv2 application and AppBuilder will generate
// event names and functions.
extern sl_zigbee_af_event_t sl_zigbee_af_form_and_join_cleanup_event;
#define cleanupEvent (&sl_zigbee_af_form_and_join_cleanup_event)
#endif

#define CLEANUP_TIMEOUT_QS 120

uint8_t formAndJoinStackProfile(void)
{
  return sl_zigbee_stack_profile();
}

// We're relying on the fact that message buffers are a multiple of 16 bytes
// in size, so that sli_zigbee_network_info_t records do not cross buffer boundaries.
sli_zigbee_network_info_t *formAndJoinGetNetworkPointer(uint8_t index)
{
  return ((sli_zigbee_network_info_t *) dataCache) + index;
}

void formAndJoinSetCleanupTimeout(void)
{
  sl_zigbee_af_event_set_delay_qs(cleanupEvent, CLEANUP_TIMEOUT_QS);
}

bool formAndJoinAllocateBuffer(void)
{
  return true;
}
uint8_t*   getFormAndJoinBufferPointer(void)
{
  return (uint8_t *) dataCache;
}

sl_status_t formAndJoinSetBufferLength(uint8_t entryCount)
{
  return (sizeof(dataCache) < entryCount * sizeof(sli_zigbee_network_info_t)
          ? SL_STATUS_ALLOCATION_FAILED
          : SL_STATUS_OK);
}

void formAndJoinReleaseBuffer(void)
{
  sl_zigbee_af_event_set_inactive(cleanupEvent);
}
