/***************************************************************************/ /**
 * @file sl_si91x_rgb_led_init_inst_config.h
 * @brief RGB Led Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_RGB_LED_INIT_INST_CONFIG_H
#define SL_SI91X_RGB_LED_INIT_INST_CONFIG_H

// <<< sl:start pin_tool >>>
// <gpio> SL_SI91X_LED_LED0_RED
// $[GPIO_SL_SI91X_LED_LED0_RED]
#ifndef SL_SI91X_LED_LED0_RED_PORT
#define SL_SI91X_LED_LED0_RED_PORT HP
#endif
#ifndef SL_SI91X_LED_LED0_RED_PIN
#define SL_SI91X_LED_LED0_RED_PIN 50
#endif
// [GPIO_SL_SI91X_LED_LED0_RED]$

// <gpio> SL_SI91X_LED_LED0_GREEN
// $[GPIO_SL_SI91X_LED_LED0_GREEN]
#ifndef SL_SI91X_LED_LED0_GREEN_PORT
#define SL_SI91X_LED_LED0_GREEN_PORT HP
#endif
#ifndef SL_SI91X_LED_LED0_GREEN_PIN
#define SL_SI91X_LED_LED0_GREEN_PIN 51
#endif
// [GPIO_SL_SI91X_LED_LED0_GREEN]$

// <gpio> SL_SI91X_LED_LED0_BLUE
// $[GPIO_SL_SI91X_LED_LED0_BLUE]
#ifndef SL_SI91X_LED_LED0_BLUE_PORT
#define SL_SI91X_LED_LED0_BLUE_PORT HP
#endif
#ifndef SL_SI91X_LED_LED0_BLUE_PIN
#define SL_SI91X_LED_LED0_BLUE_PIN 15
#endif
// [GPIO_SL_SI91X_LED_LED0_BLUE]$
// <<< sl:end pin_tool >>>

#define SL_LED_LED0_RED_PIN    (SL_SI91X_LED_LED0_RED_PIN % 16)
#define SL_LED_LED0_RED_PORT   RTE_LED0_LEDR_PORT
#define SL_LED_LED0_RED_NUMBER RTE_LED0_LEDR_NUMBER
#define SL_LED_LED0_RED_PAD    RTE_LED0_LEDR_PAD

#define SL_LED_LED0_GREEN_PIN    (SL_SI91X_LED_LED0_GREEN_PIN % 16)
#define SL_LED_LED0_GREEN_PORT   RTE_LED0_LEDG_PORT
#define SL_LED_LED0_GREEN_NUMBER RTE_LED0_LEDG_NUMBER
#define SL_LED_LED0_GREEN_PAD    RTE_LED0_LEDG_PAD

#define SL_LED_LED0_BLUE_PIN    SL_SI91X_LED_LED0_BLUE_PIN
#define SL_LED_LED0_BLUE_PORT   RTE_LED0_LEDB_PORT
#define SL_LED_LED0_BLUE_NUMBER RTE_LED0_LEDB_NUMBER
#define SL_LED_LED0_BLUE_PAD    RTE_LED0_LEDB_PAD

#endif // SL_SI91X_RGB_LED_INIT_INST_CONFIG_H