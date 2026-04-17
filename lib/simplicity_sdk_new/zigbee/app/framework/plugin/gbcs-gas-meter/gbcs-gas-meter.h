/***************************************************************************//**
 * @file
 * @brief APIs and defines for the GBCS Gas Meter plugin.
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

#ifndef GBCS_GAS_METER_H_
#define GBCS_GAS_METER_H_

/**
 * @defgroup gbcs-gas-meter GBCS Gas Meter
 * @ingroup component
 * @brief API and Callbacks for the GBCS Gas Meter Component
 *
 * Silicon Labs implementation of Gas Smart Metering Equipment (GSME).
 *
 */

/**
 * @addtogroup gbcs-gas-meter
 * @{
 */

// Printing macros for plugin: GBCS Gas Meter
#define sl_zigbee_af_gbcs_gas_meter_print(...)    sl_zigbee_af_app_print(__VA_ARGS__)
#define sl_zigbee_af_gbcs_gas_meter_println(...)  sl_zigbee_af_app_println(__VA_ARGS__)
#define sl_zigbee_af_gbcs_gas_meter_debug_exec(x)  sl_zigbee_af_app_debug_exec(x)
#define sl_zigbee_af_gbcs_gas_meter_print_buffer(buffer, len, withSpace) sl_zigbee_af_app_print_buffer(buffer, len, withSpace)

/**
 * @name API
 * @{
 */

/** @brief Report attributes.
 *
 * If the mirror is ready to receive attribute reports and a report isn't already
 * in progress, this function kicks off the task of reporting
 * all attributes to the mirror.
 */
void sl_zigbee_af_gbcs_gas_meter_report_attributes(void);

/** @} */ // end of name API
/** @} */ // end of gbcs-gas-meter

#endif /* GBCS_GAS_METER_H_ */
