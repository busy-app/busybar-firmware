/***************************************************************************//**
 * @file
 * @brief GPD component callback header.
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

void sl_zigbee_gpd_af_plugin_nv_init_cb(void);
bool sl_zigbee_gpd_af_plugin_nv_save_and_load_cb(sl_zigbee_gpd_t_t * gpd,
                                                 uint8_t * nvmData,
                                                 uint8_t sizeOfNvmData,
                                                 EmebrGpdNvLoadStore_t loadStore);
void sl_zigbee_gpd_af_plugin_commissioning_channel_config_cb(uint8_t channel);
bool sl_zigbee_gpd_af_plugin_incoming_command_cb(uint8_t gpdCommand,
                                                 uint8_t length,
                                                 uint8_t * commandPayload);
void sl_zigbee_gpd_af_plugin_sleep_cb(void);
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_MAC_SEQ) && (SL_ZIGBEE_AF_PLUGIN_APPS_MAC_SEQ == SL_ZIGBEE_GPD_MAC_SEQ_RANDOM)
uint32_t sl_zigbee_gpd_af_plugin_get_random_cb(void);
#endif
void sl_zigbee_gpd_af_plugin_get_gpd_ieee_cb(uint8_t *eui,
                                             uint8_t *gpdEndpoint);
void sl_zigbee_gpd_af_plugin_nv_init_cb(void);
void sl_zigbee_gpd_af_plugin_commissioning_reply_cb(uint8_t length,
                                                    uint8_t * commReplyPayload);
