/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_zigbee.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "stack/include/sl_zigbee_types.h"
#include "stack/internal/inc/internal-callbacks-patch.h"

/** @brief
 *
 * Application framework equivalent of ::sl_zigbee_radio_needs_calibrating_handler
 */
void sl_zigbee_af_radio_needs_calibrating_cb(void)
{
  sl_mac_calibrate_current_channel();
}

#if defined(OPENTHREAD_FTD) || defined(OPENTHREAD_RADIO)

#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/ncp.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>

#include "openthread-system.h"

#ifndef OPENTHREAD_ENABLE_COVERAGE
#define OPENTHREAD_ENABLE_COVERAGE 0
#endif

/**
 * This function initializes the NCP app.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
extern void otAppNcpInit(otInstance *aInstance);

static otInstance* sInstance = NULL;

otInstance *otGetInstance(void)
{
  return sInstance;
}

void sl_ot_create_instance(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
  size_t   otInstanceBufferLength = 0;
  uint8_t *otInstanceBuffer       = NULL;

  // Call to query the buffer size
  (void)otInstanceInit(NULL, &otInstanceBufferLength);

  // Call to allocate the buffer
  otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
  assert(otInstanceBuffer);

  // Initialize OpenThread with the buffer
  sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
  sInstance = otInstanceInitSingle();
#endif
  assert(sInstance);
}

void sl_ot_ncp_init(void)
{
  otAppNcpInit(sInstance);
}
#define OT_NWK_UP (otThreadGetDeviceRole(sInstance) != OT_DEVICE_ROLE_DISABLED)
#else
#define OT_NWK_UP false
#endif //#if defined(OPENTHREAD_FTD) || defined(OPENTHREAD_RADIO)

void sl_rail_mux_invalid_rx_channel_detected_cb(uint8_t new_rx_channel, uint8_t old_rx_channel)
{
  sl_zigbee_node_type_t type = SL_ZIGBEE_UNKNOWN_DEVICE;
  sl_zigbee_get_network_parameters(&type, NULL);

  if (type != SL_ZIGBEE_UNKNOWN_DEVICE
      // || sli_zigbee_node_type != SL_ZIGBEE_UNKNOWN_DEVICE
      || OT_NWK_UP) {
    sl_zigbee_mux_invalid_rx_handler(new_rx_channel, old_rx_channel);
    //EFM_ASSERT(false);
  }
}
