/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "green-power-print-wrapper.h"

/***************************************************************************
* Print stub functions
* Customer should define these in their own application code
***************************************************************************/

#ifndef SL_ZIGBEE_TEST
WEAK(void sl_zigbee_af_core_print_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_print_big_endian_eui64_wrapper(uint8_t * eui, ...))
{
  (void)eui;
}

WEAK(void sl_zigbee_af_print_buffer_wrapper(uint16_t area, const uint8_t *buffer, uint16_t bufferLen, bool withSpace))
{
  (void)area;
  (void)buffer;
  (void)bufferLen;
  (void)withSpace;
}

WEAK(void sl_zigbee_af_app_println_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_app_print_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_print_wrapper(uint16_t area, const char * formatString, ...))
{
  (void)area;
  (void)formatString;
}

WEAK(void sl_zigbee_af_core_println_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_debug_print_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_debug_println_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_green_power_cluster_print_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_green_power_cluster_println_wrapper(const char * formatString, ...))
{
  (void)formatString;
}

WEAK(void sl_zigbee_af_green_power_cluster_print_buffer_wrapper(const uint8_t *buffer, uint16_t bufferLen, bool withSpace))
{
  (void)buffer;
  (void)bufferLen;
  (void)withSpace;
}

WEAK(void sl_zigbee_af_green_power_cluster_print_string_wrapper(const uint8_t *buffer))
{
  (void)buffer;
}

WEAK(void sl_zigbee_af_println_wrapper(uint16_t area, const char * formatString, ...))
{
  (void)area;
  (void)formatString;
}
#endif // !SL_ZIGBEE_TEST
