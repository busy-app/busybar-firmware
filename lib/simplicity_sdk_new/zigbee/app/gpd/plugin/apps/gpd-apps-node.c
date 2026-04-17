/***************************************************************************//**
 * @file
 * @brief Application main entry and initialisations.
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
#include "gpd-components-common.h"

// GPD Instance
static sl_zigbee_gpd_t_t gpdNode = { 0 };

sl_zigbee_gpd_t_t * sl_zigbee_gpd_get_gpd(void)
{
  return &gpdNode;
}

void sl_zigbee_gpd_load_gpd_default_configuration(sl_zigbee_gpd_t_t * gpd)
{
  gpd->addr.appId = SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID;
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID) && (SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID == SL_ZIGBEE_GPD_APP_ID_SRC_ID)
  gpd->addr.id.srcId = SL_ZIGBEE_AF_PLUGIN_APPS_SRC_ID;
#elif defined(SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID) && (SL_ZIGBEE_AF_PLUGIN_APPS_APPLICATION_ID == SL_ZIGBEE_GPD_APP_ID_IEEE_ID)
  sl_zigbee_gpd_af_plugin_get_gpd_ieee_cb(gpd->addr.id.ieee,
                                          &gpd->addr.gpdEndpoint);
#else
#error "Unsupported GPD Application Id"
#endif

  uint8_t channelSet[] = SL_ZIGBEE_AF_PLUGIN_APPS_APP_CHANNEL_SET;
  // Initialise the first member of the channel set as default channel
  gpd->channel = channelSet[0];
  gpd->minRxWindow = SL_ZIGBEE_AF_PLUGIN_APPS_RX_WINDOW;
  //gpd->radioState = GPD_RADIO_STATE_IDLE;
  gpd->rxAfterTx = SL_ZIGBEE_AF_PLUGIN_APPS_RX_CAPABILITY;
  gpd->rxOffset = SL_ZIGBEE_AF_PLUGIN_APPS_RX_OFFSET;
  gpd->skipCca = true;

  uint8_t defaultKey[SL_ZIGBEE_AF_PLUGIN_APPS_KEY_LENGTH] = SL_ZIGBEE_AF_PLUGIN_APPS_KEY;
  (void) memcpy(gpd->securityKey, defaultKey, SL_ZIGBEE_AF_PLUGIN_APPS_KEY_LENGTH);
  gpd->securityKeyType = SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_KEY_TYPE;
  gpd->securityLevel = SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_LEVEL;

  gpd->gpdState = SL_ZIGBEE_GPD_APP_STATE_NOT_COMMISSIONED;
  //gpd->securityFrameCounter = 0; // Default FC , no need to initialise
}

// Initialises the Gpd device data structure with configured default values.
// Upon power up, the members that are saved to the non volatile memory to be
// retrieved and loaded.
sl_zigbee_gpd_t_t * sl_zigbee_gpd_init(void)
{
  sl_zigbee_gpd_load_gpd_default_configuration(&gpdNode);
  sl_zigbee_gpd_load_sec_data_from_nv(&gpdNode);
  sl_zigbee_gpd_security_init(&(gpdNode.addr),
                              gpdNode.securityKey,
                              gpdNode.securityFrameCounter);
  return &gpdNode;
}

void sl_zigbee_gpd_set_channel(uint8_t channel)
{
  gpdNode.channel = channel;
}

void sl_zigbee_gpd_set_state(uint8_t gpdState)
{
  gpdNode.gpdState = gpdState;
}

sl_zigbee_gpd_addr_t_t * sl_zigbee_gpd_get_addr(void)
{
  return &(gpdNode.addr);
}
