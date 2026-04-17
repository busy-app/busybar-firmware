/***************************************************************************//**
 * @file
 * @brief Routines for interacting with the prepayment-server.
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
#include "app/framework/util/af-main.h"
#include "prepayment-client.h"

/******
   TEMPORARY FIX -
    According to Bob, we believe these types should be auto-generated and put into enums.h
    Currently that is not the case.  For now, define these here.
******/

typedef uint16_t sli_zigbee_payment_control_configuration_t;
typedef uint32_t sli_zigbee_prepay_snapshot_payload_cause_t;
typedef uint8_t  sli_zigbee_prepay_snapshot_payload_type_t;
typedef uint8_t  sli_zigbee_friendly_credit_t;

void sli_zigbee_af_prepayment_client_cli_change_payment_mode(SL_CLI_COMMAND_ARG);

void sli_zigbee_af_prepayment_client_cli_change_payment_mode(SL_CLI_COMMAND_ARG)
{
  sl_802154_short_addr_t nodeId;
  uint8_t srcEndpoint, dstEndpoint;
  uint32_t providerId, issuerEventId;
  uint32_t implementationDateTime;
  sli_zigbee_payment_control_configuration_t proposedPaymentControlConfiguration;
  uint32_t cutOffValue;

  nodeId = sl_cli_get_argument_uint16(arguments, 0);
  srcEndpoint = sl_cli_get_argument_uint8(arguments, 1);
  dstEndpoint = sl_cli_get_argument_uint8(arguments, 2);
  providerId = sl_cli_get_argument_uint32(arguments, 3);
  issuerEventId = sl_cli_get_argument_uint32(arguments, 4);

  implementationDateTime = sl_cli_get_argument_uint32(arguments, 5);
  proposedPaymentControlConfiguration = sl_cli_get_argument_uint16(arguments, 6);
  cutOffValue = sl_cli_get_argument_uint32(arguments, 7);

  //sl_zigbee_af_app_println("RX Publish Prepay Snapshot Cmd, varLen=%d", i );
  sl_zigbee_af_app_println("Change Payment Mode, srcEp=%d, dstEp=%d, addr=%04X", srcEndpoint, dstEndpoint, nodeId);
  sl_zigbee_af_prepayment_client_change_payment_mode(nodeId, srcEndpoint, dstEndpoint, providerId, issuerEventId,
                                                     implementationDateTime, proposedPaymentControlConfiguration,
                                                     cutOffValue);
}
