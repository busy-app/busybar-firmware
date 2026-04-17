/***************************************************************************//**
 * @file
 * @brief Implemented routines for interacting with the debt log.
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

#ifndef SILABS_PREPAYMENT_DEBT_LOG_H
#define SILABS_PREPAYMENT_DEBT_LOG_H

/**
 * @brief Initializes the debt log.
 *
 **/
void sl_zigbee_af_prepayment_server_init_debt_log(void);

enum {
  DEBT_TYPE_1  = 0x00,
  DEBT_TYPE_2  = 0x01,
  DEBT_TYPE_3  = 0x02,
  DEBT_TYPE_ALL = 0xFF
};

/**
 * @brief Updates the debt collection attributes for the specified debt index (DEBT_INDEX_1, 2, or 3).
 * @param endpoint The endpoint number of the prepayment server.
 * @param debtIndex The debt index whose collection attributes should be updated.
 *
 **/
void sl_zigbee_af_update_debt_on_collection_event(uint8_t endpoint, uint8_t debtType);

/**
 * @brief Prints the information in the debt log at the specified index.
 * @param index The index into the debt log whose contents should be printed.
 *
 **/
void sl_zigbee_af_prepayment_print_debt_log_index(uint8_t index);

/**
 * @brief Prints the debt attribute set specified by the index parameter.
 * @param endpoint The endpoint number of the prepayment server.
 * @param index Specifies which of the 3 debt attribute sets should be printed.
 *
 **/
void sl_zigbee_af_prepayment_print_debt_attributes(uint8_t endpoint, uint8_t index);

/**
 * @brief Requests sending a Publish Debt Log command.
 * @param nodeId The address of the destination node to whom the Publish Debt Log command should be sent.
 * @param srcEndpoint The endpoint of the sender that supports the prepayment server.
 * @param dstEndpoint The endpoint on the destination node that supports the prepayment client.
 * @param latestEndTime The latest collection time of debt repayment records to be returned in
 * the Publish Debt Log command.
 * @param numberOfDebts The maximum number of debt repayment records to send in the command.
 * @param debtType Identifies the type of debt record(s) to include in the command.
 *
 **/
void sl_zigbee_af_send_publish_debt_log(sl_802154_short_addr_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint,
                                        uint32_t latestEndTime, uint8_t numberOfDebts, uint8_t debtType);

/**
 * @brief Calculates the percentage of a top up amount that should be applied towards debt reduction.
 * @param endpoint The endpoint number of the prepayment server
 * @param topUpValue The top up value amount.
 * @return Returns the amount of the top up value that should be applied to debt reduction.
 **/
uint32_t sl_zigbee_af_prepayment_get_debt_recovery_top_up_percentage(uint8_t endpoint, uint32_t topUpValue);

#endif  // #ifndef _PREPAYMENT_DEBT_LOG_H_
