/***************************************************************************/ /**
 * @file hspi_secondary_example.h
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef HSPI_SECONDARY_EXAMPLE_H
#define HSPI_SECONDARY_EXAMPLE_H

#include "sl_si91x_hspi_secondary_drv_config.h"
#include "sl_si91x_peripheral_hspi_secondary.h"
#include "sl_si91x_hspi_secondary.h"
#include "rsi_debug.h"

/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/
void hspi_secondary_example_init(void);

/***************************************************************************/ /**
 * The state machine code for send and receive is implemented here.
 * This function is called in while loop.
 ******************************************************************************/
void hspi_secondary_example_process_action(void);

#endif // HSPI_SECONDARY_EXAMPLE_H
