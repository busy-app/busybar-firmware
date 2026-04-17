/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "green-power-token-wrapper.h"

/***************************************************************************
* Token stub functions
* Customer should define these in their own application code
***************************************************************************/
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_TOKEN_SYSTEM == 1)
WEAK(void sl_zigbee_gp_set_token_wrapper(uint32_t token, void *data, uint32_t length))
{
  (void)token;
  (void)data;
  (void)length;
}

WEAK(void sl_zigbee_gp_get_token_wrapper(uint32_t token, void *data, uint32_t length))
{
  (void)token;
  (void)data;
  (void)length;
}

#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_TOKEN_SYSTEM == 1)
