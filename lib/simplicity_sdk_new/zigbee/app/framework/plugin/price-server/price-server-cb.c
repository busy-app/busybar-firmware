/***************************************************************************//**
 * @file
 * @brief Price Server Cluster plugin's callback WEAK definitions.
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
#include "price-server.h"

/** @brief New Active Price Matrix
 *
 * This callback is used to notify the application that a new PriceMatrix has
 * become active. The application is responsible for updating proper ZCL
 * attributes to reflect the necessary vendor specific behaviors. Argument,
 * priceCommonInfo, would contain common informations such as startTime and
 * etc. Argument, priceMatrix, would contain the PriceMatrix information
 * itself.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceMatrix   Ver.: always
 */
WEAK(void sl_zigbee_af_price_server_new_active_price_matrix_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                               const sl_zigbee_af_scheduled_price_matrix_t *priceMatrix))
{
  UNUSED_VAR(priceCommonInfo);
  UNUSED_VAR(priceMatrix);
}

/** @brief New Active Tariff Information
 *
 * This callback is used to notify the application that a new Tariff
 * Information has become active. The application is responsible for updating
 * proper ZCL attributes to reflect the necessary vendor specific behaviors.
 * Argument, priceCommonInfo, would contain common informations such as
 * startTime and etc. Argument, priceTariffInfo, would contain the PriceMatrix
 * information itself.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceTariffInfo   Ver.: always
 */
WEAK(void sl_zigbee_af_price_server_new_active_tariff_information_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                                     const sl_zigbee_af_scheduled_tariff_t *priceTariffInfo))
{
  UNUSED_VAR(priceCommonInfo);
  UNUSED_VAR(priceTariffInfo);
}

/** @brief New Active Block Thresholds Information
 *
 * This callback is used to notify the application that a new Block Thresholds
 * Information has become active. The application is responsible for updating
 * proper ZCL attributes to reflect the necessary vendor specific behaviors.
 * Argument, priceCommonInfo, would contain common informations such as
 * startTime and etc. Argument, priceTariffInfo, would contain the
 * BlockThresholds information itself.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceBlockThresholds   Ver.: always
 */
WEAK(void sl_zigbee_af_price_server_new_active_block_thresholds_information_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                                               const sl_zigbee_af_scheduled_block_thresholds_t *priceBlockThresholds))
{
  UNUSED_VAR(priceCommonInfo);
  UNUSED_VAR(priceBlockThresholds);
}

/** @brief New Active Block Period Information
 *
 * This callback is used to notify the application that a new Block Period
 * Information has become active. The application is responsible for updating
 * proper ZCL attributes to reflect the necessary vendor specific behaviors.
 * Argument, priceCommonInfo, would contain common informations such as
 * startTime and etc. Argument, priceBlockPeriod, would contain the BlockPeriod
 * information itself.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceBlockPeriods   Ver.: always
 */
WEAK(void sl_zigbee_af_price_server_new_active_block_period_information_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                                           const sl_zigbee_af_price_block_period_t *priceBlockPeriods))
{
  UNUSED_VAR(priceCommonInfo);
  UNUSED_VAR(priceBlockPeriods);
}
