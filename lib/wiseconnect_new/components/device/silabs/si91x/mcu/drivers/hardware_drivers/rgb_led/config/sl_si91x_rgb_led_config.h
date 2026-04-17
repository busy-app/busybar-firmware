/***************************************************************************/ /**
 * @file sl_si91x_rgb_led_config.h
 * @brief Led Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************/

#ifndef SL_SI91X_RGB_LED_CONFIG_H
#define SL_SI91X_RGB_LED_CONFIG_H

#ifdef SLI_SI917
#include "RTE_Device_917.h"
#endif

#define SL_SI91X_RGB_LED_COUNT 1 // Define the number of RGB LEDs

#ifndef SL_SI91X_LED_LED0_RED_PORT
#define SL_LED_LED0_RED_PIN    RTE_LED0_LEDR_PIN
#define SL_LED_LED0_RED_PORT   RTE_LED0_LEDR_PORT
#define SL_LED_LED0_RED_NUMBER RTE_LED0_LEDR_NUMBER
#define SL_LED_LED0_RED_PAD    RTE_LED0_LEDR_PAD
#endif

#ifndef SL_SI91X_LED_LED0_GREEN_PORT
#define SL_LED_LED0_GREEN_PIN    RTE_LED0_LEDG_PIN
#define SL_LED_LED0_GREEN_PORT   RTE_LED0_LEDG_PORT
#define SL_LED_LED0_GREEN_NUMBER RTE_LED0_LEDG_NUMBER
#define SL_LED_LED0_GREEN_PAD    RTE_LED0_LEDG_PAD
#endif

#ifndef SL_SI91X_LED_LED0_BLUE_PORT
#define SL_LED_LED0_BLUE_PIN    RTE_LED0_LEDB_PIN
#define SL_LED_LED0_BLUE_PORT   RTE_LED0_LEDB_PORT
#define SL_LED_LED0_BLUE_NUMBER RTE_LED0_LEDB_NUMBER
#define SL_LED_LED0_BLUE_PAD    RTE_LED0_LEDB_PAD
#endif

// To support exisiting RGB LED instances.
#ifndef SL_SI91X_LED_RED_PORT
#define SL_LED_RED_PIN    RTE_LED0_LEDR_PIN
#define SL_LED_RED_PORT   RTE_LED0_LEDR_PORT
#define SL_LED_RED_NUMBER RTE_LED0_LEDR_NUMBER
#define SL_LED_RED_PAD    RTE_LED0_LEDR_PAD
#endif

#ifndef SL_SI91X_LED_GREEN_PORT
#define SL_LED_GREEN_PIN    RTE_LED0_LEDG_PIN
#define SL_LED_GREEN_PORT   RTE_LED0_LEDG_PORT
#define SL_LED_GREEN_NUMBER RTE_LED0_LEDG_NUMBER
#define SL_LED_GREEN_PAD    RTE_LED0_LEDG_PAD
#endif

#ifndef SL_SI91X_LED_BLUE_PORT
#define SL_LED_BLUE_PIN    RTE_LED0_LEDB_PIN
#define SL_LED_BLUE_PORT   RTE_LED0_LEDB_PORT
#define SL_LED_BLUE_NUMBER RTE_LED0_LEDB_NUMBER
#define SL_LED_BLUE_PAD    RTE_LED0_LEDB_PAD
#endif

#endif // SL_SI91X_LED_CONFIG_H
