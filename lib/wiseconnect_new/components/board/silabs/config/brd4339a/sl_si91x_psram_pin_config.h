/*******************************************************************************
 * @file  sl_si91x_psram_pin_config.h
 * @brief
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
#ifndef __SL_SI91X_PSRAM_PIN_CONFIG_H_
#define __SL_SI91X_PSRAM_PIN_CONFIG_H_

#include "sl_si91x_psram_handle.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h> PSRAM GPIO Pinset Selection

// <o PSRAM_GPIO_PIN_SET_SEL> Select the Pin Set
//   <PSRAM_GPIO_PIN_SET_52_TO_57=> Pin Set 52 to 57 (External PSRAM pins)
//   <PSRAM_GPIO_PIN_SET_0_TO_5=> Pin Set 0 to 5 (Stacked PSRAM pins)
//   <PSRAM_GPIO_PIN_SET_46_TO_51_CS_0=> Pin Set 46 to 51 with CS 0
//   <PSRAM_GPIO_PIN_SET_46_TO_51_CS_1=> Pin Set 46 to 51 with CS 1
//   <PSRAM_GPIO_PIN_SET_46_TO_57_CS_0=> Pin Set 46 to 57 with CS 0
//   <PSRAM_GPIO_PIN_SET_46_TO_57_CS_1=> Pin Set 46 to 57 with CS 1
// <d>  PSRAM_GPIO_PIN_SET_52_TO_57
#define PSRAM_GPIO_PIN_SET_SEL PSRAM_GPIO_PIN_SET_0_TO_5

// </h> end PSRAM GPIO Pinset Selection

// <<< end of configuration section >>>

#endif //__SL_SI91X_PSRAM_PIN_CONFIG_H_