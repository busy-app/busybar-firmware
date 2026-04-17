/***************************************************************************/ /**
 * @file sl_si91x_button_config.h
 * @brief Button Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_BUTTON_PIN_CONFIG_H
#define SL_SI91X_BUTTON_PIN_CONFIG_H

#include "em_device.h"
#include "sl_si91x_button_instances.h"

#define SL_SI91x_BUTTON_COUNT (2)

#ifndef SL_SI91X_BUTTON_0_PORT
#define SL_BUTTON_BTN0_PIN    RTE_BUTTON0_PIN
#define SL_BUTTON_BTN0_PORT   RTE_BUTTON0_PORT
#define SL_BUTTON_BTN0_NUMBER RTE_BUTTON0_NUMBER
#endif

#ifndef SL_SI91X_BUTTON_1_PORT
#define SL_BUTTON_BTN1_PIN    RTE_BUTTON1_PIN
#define SL_BUTTON_BTN1_PORT   RTE_BUTTON1_PORT
#define SL_BUTTON_BTN1_NUMBER RTE_BUTTON1_NUMBER
#define SL_BUTTON_BTN1_PAD    RTE_BUTTON1_PAD
#endif

#endif // SL_SI91X_BUTTON_PIN_CONFIG_H