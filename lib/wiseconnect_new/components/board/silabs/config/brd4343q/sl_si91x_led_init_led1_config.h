/***************************************************************************/ /**
 * @file sl_si91x_led_init_led1_config.h
 * @brief Led Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_LED_INIT_LED1_CONFIG_H
#define SL_SI91X_LED_INIT_LED1_CONFIG_H

// <<< sl:start pin_tool >>>
// <gpio> SL_SI91X_LED_1
// $[GPIO_SL_SI91X_LED_1]
#ifndef SL_SI91X_LED_1_PORT
#define SL_SI91X_LED_1_PORT HP
#endif
#ifndef SL_SI91X_LED_1_PIN
#define SL_SI91X_LED_1_PIN 10
#endif
// [GPIO_SL_SI91X_LED_1]$
// <<< sl:end pin_tool >>>

#define SL_LED_LED1_PIN    SL_SI91X_LED_1_PIN
#define SL_LED_LED1_PORT   RTE_LED1_PORT
#define SL_LED_LED1_NUMBER RTE_LED1_NUMBER

#endif // SL_SI91X_LED_INIT_LED1_CONFIG_H
