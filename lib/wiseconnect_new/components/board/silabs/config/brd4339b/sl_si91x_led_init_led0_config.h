/***************************************************************************/ /**
 * @file sl_si91x_led_init_led0_config.h
 * @brief Led Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_LED_INIT_LED0_CONFIG_H
#define SL_SI91X_LED_INIT_LED0_CONFIG_H

// <<< sl:start pin_tool >>>
// <gpio> SL_SI91X_LED_0
// $[GPIO_SL_SI91X_LED_0]
#ifndef SL_SI91X_LED_0_PORT
#define SL_SI91X_LED_0_PORT ULP
#endif
#ifndef SL_SI91X_LED_0_PIN
#define SL_SI91X_LED_0_PIN 2
#endif
// [GPIO_SL_SI91X_LED_0]$
// <<< sl:end pin_tool >>>

#define SL_LED_LED0_PIN    SL_SI91X_LED_0_PIN
#define SL_LED_LED0_PORT   RTE_LED0_PORT
#define SL_LED_LED0_NUMBER RTE_LED0_NUMBER

#endif // SL_SI91X_LED_INIT_LED0_CONFIG_H
