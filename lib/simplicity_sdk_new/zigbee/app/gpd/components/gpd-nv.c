/***************************************************************************//**
 * @file
 * @brief Routines for non volatile memory interfaces used by the GPD.
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
#include "gpd-cb.h"

// The NVM array is arranged as follows
//-----------------------------------------------------------------------------
//  0 -- 3  |  4  ---------  19  |  20  |  21  |  22  |  23  |  24-31   |
//    FC    |        KEY         | SecL | SecT |  Ch  | State|  Reserved|
//-----------------------------------------------------------------------------
static uint8_t gpdNvData[SL_ZIGBEE_GPD_NV_DATA_SIZE];

void sl_zigbee_gpd_nv_init(void)
{
  sl_zigbee_gpd_af_plugin_nv_init_cb();
}
// Accessor functions
void sl_zigbee_gpd_save_gpd_frame_counter_to_shadow(uint32_t frameCounter)
{
  sl_zigbee_gpd_utility_copy_4_bytes(gpdNvData, frameCounter);
}

void sl_zigbee_gpd_copy_to_shadow(sl_zigbee_gpd_t_t * gpd)
{
  sl_zigbee_gpd_utility_copy_4_bytes(gpdNvData, gpd->securityFrameCounter);
  (void) memcpy(&gpdNvData[SL_ZIGBEE_GPD_NV_DATA_GPD_KEY_INDEX], gpd->securityKey, 16);
  gpdNvData[SL_ZIGBEE_GPD_NV_DATA_GPD_SEC_LEVEL_INDEX] = gpd->securityLevel;
  gpdNvData[SL_ZIGBEE_GPD_NV_DATA_GPD_SEC_KEY_TYPE_INDEX] = gpd->securityKeyType;
  gpdNvData[SL_ZIGBEE_GPD_NV_DATA_GPD_CHANNEL_INDEX] = gpd->channel;
  gpdNvData[SL_ZIGBEE_GPD_NV_DATA_GPD_STATE_INDEX] = gpd->gpdState;
}
void sl_zigbee_gpd_copy_from_shadow(sl_zigbee_gpd_t_t * gpd)
{
  gpd->securityFrameCounter = *((uint32_t *)gpdNvData);
  (void) memcpy(gpd->securityKey, (gpdNvData + SL_ZIGBEE_GPD_NV_DATA_GPD_KEY_INDEX), 16);
  gpd->securityLevel = *(gpdNvData + SL_ZIGBEE_GPD_NV_DATA_GPD_SEC_LEVEL_INDEX);
  gpd->securityKeyType = *(gpdNvData + SL_ZIGBEE_GPD_NV_DATA_GPD_SEC_KEY_TYPE_INDEX);
  gpd->channel = *(gpdNvData + SL_ZIGBEE_GPD_NV_DATA_GPD_CHANNEL_INDEX);
  gpd->gpdState = *(gpdNvData + SL_ZIGBEE_GPD_NV_DATA_GPD_STATE_INDEX);
}
void sl_zigbee_gpd_store_sec_data_to_nv(sl_zigbee_gpd_t_t * gpd)
{
  sl_zigbee_gpd_copy_to_shadow(gpd);
  sl_zigbee_gpd_af_plugin_nv_save_and_load_cb(gpd,
                                              gpdNvData,
                                              SL_ZIGBEE_GPD_NV_DATA_SIZE,
                                              EMEBER_GPD_AF_CALLBACK_STORE_GPD_TO_NVM);
}

void sl_zigbee_gpd_load_sec_data_from_nv(sl_zigbee_gpd_t_t * gpd)
{
  sl_zigbee_gpd_af_plugin_nv_save_and_load_cb(gpd,
                                              gpdNvData,
                                              SL_ZIGBEE_GPD_NV_DATA_SIZE,
                                              EMEBER_GPD_AF_CALLBACK_LOAD_GPD_FROM_NVM);
  sl_zigbee_gpd_copy_from_shadow(gpd);
}
