/***************************************************************************/ /**
 * @file sl_si91x_button_init_INSTANCE_config.h
 * @brief Button Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_BUTTON_INIT_INSTANCE_CONFIG_H
#define SL_SI91X_BUTTON_INIT_INSTANCE_CONFIG_H

#if USER_CONFIGURATION_ENABLE
// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_BUTTON_CONFIG_INSTANCE_INTR> Button Interrupt Configuration
// <HIGH_LEVEL_INTERRUPT => High level interrupt
// <LOW_LEVEL_INTERRUPT => Low level interrupt
// <HIGH_LEVEL_AND_LOW_LEVEL_INTERRUPT => High level and low level interrupt
// <RISE_EDGE_INTERRUPT => Rise edge interrupt
// <FALL_EDGE_INTERRUPT => Fall edge interrupt
// <RISE_EDGE_AND_FALL_EDGE_INTERRUPT => Rise edge and fall edge interrupt
// <i> Default: RISE_EDGE_AND_FALL_EDGE_INTERRUPT
#define SL_BUTTON_CONFIG_INSTANCE_INTR RISE_EDGE_AND_FALL_EDGE_INTERRUPT

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>
// <gpio> SL_SI91X_BUTTON_INSTANCE
// $[GPIO_SL_SI91X_BUTTON_INSTANCE]
#ifndef SL_SI91X_BUTTON_INSTANCE_PORT
#define SL_SI91X_BUTTON_INSTANCE_PORT UULP_VBAT
#endif
#ifndef SL_SI91X_BUTTON_INSTANCE_PIN
#define SL_SI91X_BUTTON_INSTANCE_PIN 2
#endif
// [GPIO_SL_SI91X_BUTTON_INSTANCE]$
// <<< sl:end pin_tool >>>

#define SL_BUTTON_INSTANCE_PIN    SL_SI91X_BUTTON_INSTANCE_PIN
#define SL_BUTTON_INSTANCE_PORT   SL_SI91X_BUTTON_INSTANCE_PORT
#define SL_BUTTON_INSTANCE_NUMBER RTE_INSTANCE_NUMBER

#else

#warning \
  "Button instance not defined for this OPN/SoC project. To configure board macros, either install the [ENABLE USER CONFIGURATION] component or define USER_CONFIGURATION_ENABLE MACRO as 1, then define the required macros in this header file according to your board connections."

#endif // USER_CONFIGURATION_ENABLE
#endif // SL_SI91X_BUTTON_INIT_INSTANCE_CONFIG_H
