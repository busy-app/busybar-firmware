/***************************************************************************//**
 * @brief System adaptation layer used internally by Bluetooth AppLoader
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_APPLOADER_LIB_ADAPTATION_H
#define SLI_APPLOADER_LIB_ADAPTATION_H

#include <sl_rail.h>

void sli_apploader_set_interrupt_priorities();
sl_rail_tx_power_config_t *sli_apploader_get_tx_power_config();

#endif // SLI_APPLOADER_LIB_ADAPTATION_H
