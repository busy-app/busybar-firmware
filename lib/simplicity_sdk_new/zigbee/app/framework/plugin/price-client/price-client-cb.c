/***************************************************************************//**
 * @file
 * @brief
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

#include "app/framework/include/af.h"

/** @brief Price Started
 *
 * This function is called by the Price client plugin whenever a price starts.
 *
 * @param price The price that has started. Ver.: always
 */
WEAK(void sl_zigbee_af_price_client_price_started_cb(sl_zigbee_af_plugin_price_client_price_t *price))
{
  UNUSED_VAR(price);
}

/** @brief Price Expired
 *
 * This function is called by the Price client plugin whenever a price expires.
 *
 * @param price The price that has expired. Ver.: always
 */
WEAK(void sl_zigbee_af_price_client_price_expired_cb(sl_zigbee_af_plugin_price_client_price_t *price))
{
  UNUSED_VAR(price);
}

/** @brief Pending Cpp Event
 *
 * This function is called by the Price client plugin when a CPP event is
 * received with pending status.
 *
 * @param cppAuth The CPP Auth status. Ver.: always
 */
WEAK(uint8_t sl_zigbee_af_price_client_pending_cpp_event_cb(uint8_t cppAuth))
{
  extern uint8_t sl_zigbee_af_price_cluster_default_cpp_event_authorization;

  /* This callback should decide which CPP Auth status to send in response to the pending status.
     It may return SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_ACCEPTED or SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_REJECTED. */
  if ( cppAuth != SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_FORCED ) {
    return sl_zigbee_af_price_cluster_default_cpp_event_authorization;
  } else {
    return SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_ACCEPTED;
  }
}
