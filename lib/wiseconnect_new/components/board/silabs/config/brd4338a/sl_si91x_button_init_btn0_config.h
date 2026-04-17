/***************************************************************************/ /**
 * @file sl_si91x_button_init_btn0_config.h
 * @brief Button Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_BUTTON_INIT_BTN0_CONFIG_H
#define SL_SI91X_BUTTON_INIT_BTN0_CONFIG_H

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
// <gpio> SL_SI91X_BUTTON_0
// $[GPIO_SL_SI91X_BUTTON_0]
#ifndef SL_SI91X_BUTTON_0_PORT
#define SL_SI91X_BUTTON_0_PORT UULP_VBAT
#endif
#ifndef SL_SI91X_BUTTON_0_PIN
#define SL_SI91X_BUTTON_0_PIN 2
#endif
// [GPIO_SL_SI91X_BUTTON_0]$
// <<< sl:end pin_tool >>>

#define SL_BUTTON_BTN0_PIN    SL_SI91X_BUTTON_0_PIN
#define SL_BUTTON_BTN0_PORT   SL_SI91X_BUTTON_0_PORT
#define SL_BUTTON_BTN0_NUMBER RTE_BUTTON0_NUMBER

#endif // SL_SI91X_BUTTON_INIT_BTN0_CONFIG_H
