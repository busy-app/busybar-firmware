/***************************************************************************//**
 * @file sli_zigbee_dynamic_commissioning.h
 * @brief implements hooks for performing device interview during dynamic
 * commissioning (INTERNALS)
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_ZIGBEE_DYNAMIC_COMMISSIONING_H
#define SLI_ZIGBEE_DYNAMIC_COMMISSIONING_H

/**
 * @brief API to refresh the timers during device interview to keep temporary entries active
 * @param[in]  device_short   short address of the node
 * @param[in]  device_long  EUI64 of the noded
 */
void sli_zigbee_dynamic_commissioning_refresh_timers(sl_802154_short_addr_t device_short,
                                                     sl_802154_long_addr_t device_long);
/**
 * @brief API to update the status of device interview
 * @param[in] sl_zigbee_address_info structure to hold the short and long address
 */
void sli_zigbee_dynamic_commissioning_device_interview_ready(sl_zigbee_address_info *ids);

#endif // SLI_ZIGBEE_DYNAMIC_COMMISSIONING_H
