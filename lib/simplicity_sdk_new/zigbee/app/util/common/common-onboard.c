/***************************************************************************//**
 * @file
 * @brief common code for non-host (em250/em2420) apps.
 *
 * The common library is deprecated and will be removed in a future release.
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

#include "stack/include/sl_zigbee.h"
#include "hal/hal.h"
#if !defined(SL_ZIGBEE_TEST)
#include "sl_mbedtls.h"
#endif
#include "serial/serial.h"
#include "app/util/common/common.h"
#include "event_queue/event-queue.h"
#include "stack/core/sl_zigbee_multi_network.h"
#include "stack/config/sl_zigbee_token_defines.h"
#include "sl_token_manager_api.h"

//------------------------------------------------------------------------------
// External Declarations

//------------------------------------------------------------------------------

bool setSecurityLevel(uint8_t level)
{
  sl_zigbee_set_security_level(level);
  return true;
}

//------------------------------------------------------------------------------

bool getNetworkParameters(sl_zigbee_node_type_t* nodeType,
                          sl_zigbee_network_parameters_t* networkParams)
{
  return (SL_STATUS_OK == sl_zigbee_get_network_parameters(nodeType, networkParams));
}

//------------------------------------------------------------------------------

void initialize_sl_zigbee_stack_t(void)
{
  //Initialize the hal
  halInit();

  #if !defined(SL_ZIGBEE_TEST)
  // This function must be called by an application before using any mbedTLS
  // functions. This function will make sure that the platform hooks in mbedTLS
  // are configured to ensure correct runtime behavior.
  sl_mbedtls_init();
  #endif

  INTERRUPTS_ON();
#if defined(HAL_CONFIG)
  configureSerial(BSP_SERIAL_APP_PORT, HAL_SERIAL_APP_BAUD_RATE);
#endif
}

//------------------------------------------------------------------------------

sl_status_t getOnlineNodeParameters(uint8_t* childCountReturn,
                                    uint8_t* routerCountReturn,   // tree legacy
                                    sl_zigbee_node_type_t* nodeTypeReturn,
                                    sl_802154_short_addr_t* parentNodeIdReturn,
                                    sl_802154_long_addr_t parentEuiReturn,
                                    sl_zigbee_network_parameters_t* networkParamReturn)
{
  sl_status_t status = sl_zigbee_get_network_parameters(nodeTypeReturn, networkParamReturn);
  if ( status != SL_STATUS_OK ) {
    return status;
  }

  *childCountReturn = sli_zigbee_end_device_child_count;
  *routerCountReturn = 0;
  *parentNodeIdReturn = sl_zigbee_get_parent_node_id();
  memmove(parentEuiReturn, sl_zigbee_get_parent_eui64(), EUI64_SIZE);
  return SL_STATUS_OK;
}

//------------------------------------------------------------------------------

sl_status_t getOfflineNodeParameters(sl_802154_short_addr_t *myNodeIdReturn,
                                     sl_zigbee_node_type_t *myNodeTypeReturn,
                                     uint8_t* stackProfileReturn)
{
  tokTypeStackNodeData tok;
  (void)sl_token_manager_get_data(COMMON_TOKEN_STACK_NODE_DATA, (void *)&tok, sizeof(tokTypeStackNodeData));
  *myNodeIdReturn = tok.zigbeeNodeId;
  *myNodeTypeReturn = tok.nodeType;
  *stackProfileReturn = tok.stackProfile;
  return SL_STATUS_OK;
}

//------------------------------------------------------------------------------

void runEvents(sli_zigbee_event_queue_t* event_queue)
{
  sli_zigbee_run_event_queue(event_queue);
}

//------------------------------------------------------------------------------
