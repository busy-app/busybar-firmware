/***************************************************************************/ /**
 * @file sl_si91x_button_init_btn1_config.h
 * @brief Button Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_BUTTON_INIT_BTN1_CONFIG_H
#define SL_SI91X_BUTTON_INIT_BTN1_CONFIG_H

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
// <gpio> SL_SI91X_BUTTON_1
// $[GPIO_SL_SI91X_BUTTON_1]
#ifndef SL_SI91X_BUTTON_1_PORT
#define SL_SI91X_BUTTON_1_PORT HP
#endif
#ifndef SL_SI91X_BUTTON_1_PIN
#define SL_SI91X_BUTTON_1_PIN 49
#endif
// [GPIO_SL_SI91X_BUTTON_1]$
// <<< sl:end pin_tool >>>

#define SL_BUTTON_BTN1_PIN    SL_SI91X_BUTTON_1_PIN
#define SL_BUTTON_BTN1_PORT   SL_SI91X_BUTTON_1_PORT
#define SL_BUTTON_BTN1_NUMBER RTE_BUTTON1_NUMBER
#define SL_BUTTON_BTN1_PAD    RTE_BUTTON1_PAD

#endif // SL_SI91X_BUTTON_INIT_BTN1_CONFIG_H
