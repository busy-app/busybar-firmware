/***************************************************************************/ /**
 * @file rgb_led.h
 * @brief RGB LED examples functions
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

#ifndef RGB_LED_H
#define RGB_LED_H

/***************************************************************************/ /**
 * Initialize example
 ******************************************************************************/
void rgb_led_init(void);

/***************************************************************************/ /**
 * RGB LED ticking function
 ******************************************************************************/
void rgb_led_process_action(void);

#endif // RGB_LED_H
