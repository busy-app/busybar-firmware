/***************************************************************************//**
 * @file
 * @brief Matrix routines for the Price Server plugin.
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

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "price-server.h"
#include "price-server-tick.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

void sl_zigbee_af_price_clear_tariff_table(uint8_t endpoint)
{
  (void) endpoint;
}

void sl_zigbee_af_price_clear_price_matrix_table(uint8_t endpoint)
{
  (void) endpoint;
}

void sl_zigbee_af_price_clear_block_thresholds_table(uint8_t endpoint)
{
  (void) endpoint;
}

bool sl_zigbee_af_price_set_tariff_table_entry(uint8_t endpoint,
                                               uint8_t index,
                                               sl_zigbee_af_price_common_info_t *info,
                                               const sl_zigbee_af_scheduled_tariff_t *tariff)
{
  (void) endpoint;
  (void) index;
  (void) *info;
  (void) *tariff;
  return false;
}

bool sl_zigbee_af_price_get_tariff_table_entry(uint8_t endpoint,
                                               uint8_t index,
                                               sl_zigbee_af_price_common_info_t *info,
                                               sl_zigbee_af_scheduled_tariff_t *tariff)
{
  (void) endpoint;
  (void) index;
  (void) *info;
  (void) *tariff;
  return false;
}

bool sl_zigbee_af_price_add_tariff_table_entry(uint8_t endpoint,
                                               sl_zigbee_af_price_common_info_t *info,
                                               const sl_zigbee_af_scheduled_tariff_t *curTariff)
{
  (void) endpoint;
  (void) *info;
  (void) *curTariff;
  return false;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_tariff_information_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;
  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_price_matrix_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;
  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_block_thresholds_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  (void)cmd;
  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}

bool sl_zigbee_af_price_get_price_matrix(uint8_t endpoint,
                                         uint8_t index,
                                         sl_zigbee_af_price_common_info_t *info,
                                         sl_zigbee_af_scheduled_price_matrix_t *pm)
{
  (void) endpoint;
  (void) index;
  (void) *info;
  (void) *pm;
  return false;
}

bool sl_zigbee_af_price_get_block_thresholds_table_entry(uint8_t endpoint,
                                                         uint8_t index,
                                                         sl_zigbee_af_scheduled_block_thresholds_t *bt)
{
  (void) endpoint;
  (void) index;
  (void) *bt;
  return false;
}

bool sl_zigbee_af_price_get_tariff_by_issuer_tariff_id(uint8_t endpoint,
                                                       uint32_t issuerTariffId,
                                                       sl_zigbee_af_price_common_info_t *info,
                                                       sl_zigbee_af_scheduled_tariff_t *tariff)
{
  (void) endpoint;
  (void) issuerTariffId;
  (void) *info;
  (void) *tariff;
  return false;
}

bool sl_zigbee_af_price_get_price_matrix_by_issuer_tariff_id(uint8_t endpoint,
                                                             uint32_t issuerTariffId,
                                                             sl_zigbee_af_price_common_info_t *info,
                                                             sl_zigbee_af_scheduled_price_matrix_t *pm)
{
  (void) endpoint;
  (void) issuerTariffId;
  (void) *info;
  (void) *pm;
  return false;
}

bool sl_zigbee_af_price_get_block_thresholds_by_issuer_tariff_id(uint8_t endpoint,
                                                                 uint32_t issuerTariffId,
                                                                 sl_zigbee_af_price_common_info_t *info,
                                                                 sl_zigbee_af_scheduled_block_thresholds_t *bt)
{
  (void) endpoint;
  (void) issuerTariffId;
  (void) *info;
  (void) *bt;
  return false;
}

bool sl_zigbee_af_price_set_price_matrix(uint8_t endpoint,
                                         uint8_t index,
                                         sl_zigbee_af_price_common_info_t *info,
                                         const sl_zigbee_af_scheduled_price_matrix_t *pm)
{
  (void) endpoint;
  (void) index;
  (void) *info;
  (void) *pm;
  return false;
}

bool sl_zigbee_af_price_set_block_thresholds_table_entry(uint8_t endpoint,
                                                         uint8_t index,
                                                         const sl_zigbee_af_price_common_info_t *info,
                                                         const sl_zigbee_af_scheduled_block_thresholds_t *bt)
{
  (void) endpoint;
  (void) index;
  (void) *info;
  (void) *bt;
  return false;
}

void sl_zigbee_af_price_print_tariff(const sl_zigbee_af_price_common_info_t *info,
                                     const sl_zigbee_af_scheduled_tariff_t *tariff)
{
  (void) *info;
  (void) *tariff;
}
void sl_zigbee_af_price_print_price_matrix(uint8_t endpoint,
                                           const sl_zigbee_af_price_common_info_t *info,
                                           const sl_zigbee_af_scheduled_price_matrix_t *pm)
{
  (void) endpoint;
  (void) *info;
  (void) *pm;
}
void sl_zigbee_af_price_print_block_thresholds(uint8_t endpoint,
                                               const sl_zigbee_af_price_common_info_t *info,
                                               const sl_zigbee_af_scheduled_block_thresholds_t *bt)
{
  (void) endpoint;
  (void) *info;
  (void) *bt;
}
void sl_zigbee_af_price_print_tariff_table(uint8_t endpoint)
{
  (void) endpoint;
}
void sl_zigbee_af_price_print_price_matrix_table(uint8_t endpoint)
{
  (void) endpoint;
}
void sl_zigbee_af_price_print_block_thresholds_table(uint8_t endpoint)
{
  (void) endpoint;
}
bool sl_zigbee_af_price_add_block_thresholds_table_entry(uint8_t endpoint,
                                                         uint32_t providerId,
                                                         uint32_t issuerEventId,
                                                         uint32_t startTime,
                                                         uint32_t issuerTariffId,
                                                         uint8_t commandIndex,
                                                         uint8_t numberOfCommands,
                                                         uint8_t subpayloadControl,
                                                         uint8_t* payload)
{
  (void) endpoint;
  (void) providerId;
  (void) issuerEventId;
  (void) startTime;
  (void) issuerTariffId;
  (void) commandIndex;
  (void) numberOfCommands;
  (void) subpayloadControl;
  (void) *payload;
  return false;
}

bool sl_zigbee_af_price_add_price_matrix_raw(uint8_t endpoint,
                                             uint32_t providerId,
                                             uint32_t issuerEventId,
                                             uint32_t startTime,
                                             uint32_t issuerTariffId,
                                             uint8_t commandIndex,
                                             uint8_t numberOfCommands,
                                             uint8_t subPayloadControl,
                                             uint8_t* payload)
{
  (void) endpoint;
  (void) providerId;
  (void) issuerEventId;
  (void) startTime;
  (void) issuerTariffId;
  (void) commandIndex;
  (void) numberOfCommands;
  (void) subPayloadControl;
  (void) *payload;
  return false;
}
