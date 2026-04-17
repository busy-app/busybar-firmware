/***************************************************************************//**
 * @file
 * @brief This file specifies GPIO pin information for sl_rail_test applications.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <string.h>

#if !defined(__ICCARM__)
// IAR doesn't have strings.h and puts those declarations in string.h
#include <strings.h>
#endif

#include "sl_core.h"

#include "response_print.h"

#include "sl_rail.h"
#include "app_common.h"
#include "app_trx.h"

#include "sl_hal_gpio.h"

#include "em_device.h"
#ifndef _GPIO_PORT_MASK
#warning "_GPIO_PORT_MASK not currently present in sl_gpio.h, using a workaround."
#define _GPIO_PORT_MASK(port) (                       \
    ((int)port) == 0  ? SL_HAL_GPIO_PORT_A_PIN_MASK   \
    : ((int)port) == 1  ? SL_HAL_GPIO_PORT_B_PIN_MASK \
    : ((int)port) == 2  ? SL_HAL_GPIO_PORT_C_PIN_MASK \
    : ((int)port) == 3  ? SL_HAL_GPIO_PORT_D_PIN_MASK \
    : ((int)port) == 4  ? SL_HAL_GPIO_PORT_E_PIN_MASK \
    : ((int)port) == 5  ? SL_HAL_GPIO_PORT_F_PIN_MASK \
    : ((int)port) == 6  ? SL_HAL_GPIO_PORT_G_PIN_MASK \
    : ((int)port) == 7  ? SL_HAL_GPIO_PORT_H_PIN_MASK \
    : ((int)port) == 8  ? SL_HAL_GPIO_PORT_I_PIN_MASK \
    : ((int)port) == 9  ? SL_HAL_GPIO_PORT_J_PIN_MASK \
    : ((int)port) == 10 ? SL_HAL_GPIO_PORT_K_PIN_MASK \
    : 0UL)
#endif

void setGpioOutPin(sl_cli_command_arg_t *args)
{
  //check number of arguments
  if (sl_cli_get_argument_count(args) < 3) {
    responsePrint(sl_cli_get_command_string(args, 0), "Error:Not enough inputs.");
    return;
  }

  sl_gpio_port_t port;
  uint16_t portMask;

  char portChar = *(sl_cli_get_argument_string(args, 0));
  int16_t diff = portChar - 'a';
  int16_t diffCap = portChar - 'A';
  int16_t diffNum = portChar - '0';

  //find port name from input
  if (diff >= 0 && diff <= SL_HAL_GPIO_PORT_MAX) {
    port = (sl_gpio_port_t)diff;
  } else if (diffCap >= 0 && diffCap <= SL_HAL_GPIO_PORT_MAX) {
    port = (sl_gpio_port_t)diffCap;
  } else if (diffNum >= 0 && diffNum <= SL_HAL_GPIO_PORT_MAX) {
    port = (sl_gpio_port_t)diffNum;
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "Error:%s is not a valid port.", sl_cli_get_argument_string(args, 0));
    return;
  }

  portMask = _GPIO_PORT_MASK(port);
  uint8_t pin = sl_cli_get_argument_uint8(args, 1);

  if (((portMask >> pin) & 0x1) == 0U) {
    responsePrint(sl_cli_get_command_string(args, 0),
                  "Error:The pin you are configuring is unavailable. \
                   Port %c has the following port mask: %x",
                  portChar, portMask);
    return;
  }
  //set the pin based on the input value
  uint8_t val = sl_cli_get_argument_uint8(args, 2);
  sl_gpio_set_pin_mode(&(sl_gpio_t){port, pin }, SL_GPIO_MODE_PUSH_PULL, val);
  responsePrint(sl_cli_get_command_string(args, 0), "Status:Success.");
}

void setAntDivPin(sl_cli_command_arg_t *args)
{
  sl_rail_antenna_config_t antennaConfig;

  antennaConfig.antenna_0_port = sl_cli_get_argument_uint8(args, 0);
  antennaConfig.antenna_0_pin  = sl_cli_get_argument_uint8(args, 1);
  antennaConfig.antenna_1_port = sl_cli_get_argument_uint8(args, 2);
  antennaConfig.antenna_1_pin  = sl_cli_get_argument_uint8(args, 3);

  if (sl_cli_get_argument_count(args) == 6) {
    antennaConfig.antenna_0_pin_enable = ((sl_cli_get_argument_uint8(args, 4)) ? true : false);
    antennaConfig.antenna_1_pin_enable = ((sl_cli_get_argument_uint8(args, 5)) ? true : false);
  } else if (sl_cli_get_argument_count(args) == 5) {
    antennaConfig.antenna_0_pin_enable = ((sl_cli_get_argument_uint8(args, 4)) ? true : false);
    antennaConfig.antenna_1_pin_enable = true;
  } else {
    antennaConfig.antenna_0_pin_enable = true;
    antennaConfig.antenna_1_pin_enable = true;
  }

  if (sl_rail_config_antenna(SL_RAIL_EFR32_HANDLE, &antennaConfig) != SL_RAIL_STATUS_NO_ERROR) {
    responsePrint(sl_cli_get_command_string(args, 0), "Status:Error.");
  }
  responsePrint(sl_cli_get_command_string(args, 0), "Status:Success.");
}

void getGpioInPin(sl_cli_command_arg_t *args)
{
  //check number of arguments
  if (sl_cli_get_argument_count(args) < 2) {
    responsePrint(sl_cli_get_command_string(args, 0), "Error:Not enough inputs.");
    return;
  }

  sl_gpio_port_t port;

  char portChar = *(sl_cli_get_argument_string(args, 0));
  int16_t diff = portChar - 'a';
  int16_t diffCap = portChar - 'A';
  int16_t diffNum = portChar - '0';

  //find port name from input
  if (diff >= 0 && diff <= SL_HAL_GPIO_PORT_MAX) {
    port = (sl_gpio_port_t)diff;
  } else if (diffCap >= 0 && diffCap <= SL_HAL_GPIO_PORT_MAX) {
    port = (sl_gpio_port_t)diffCap;
  } else if (diffNum >= 0 && diffNum <= SL_HAL_GPIO_PORT_MAX) {
    port = (sl_gpio_port_t)diffNum;
  } else {
    responsePrint(sl_cli_get_command_string(args, 0), "Error:%c is not a valid port.", portChar);
    return;
  }

  uint8_t pin = sl_cli_get_argument_uint8(args, 1);

  //read the pin level
  bool level;
  sl_status_t status = sl_gpio_get_pin_input(&(sl_gpio_t){port, pin }, &level);
  if (status != SL_STATUS_OK) {
    responsePrint(sl_cli_get_command_string(args, 0), "Error:Failed to read pin level.");
    return;
  }
  responsePrint(sl_cli_get_command_string(args, 0), "Port:%c,Pin:%u,Level:%u",
                portChar, pin, level);
}
