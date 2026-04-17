/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/include/sl_zigbee_random_api.h"
#include "hal/hal.h"
#include "ash-protocol.h"
#include "app/ezsp-host/ezsp-host-io.h"

#include "serial/serial.h"

// Disabling the Random Byte corruption to circumvent EMHAL-1626 which causes wwah-silabs.bsh to fail
#ifndef NO_RD_WR_BAD_RANDOM
  #define WR_BAD_RANDOM        400  // corrupt write data with 1/N probablility
  #define RD_BAD_RANDOM        400  // corrupt read data with 1/N probablility
#endif

sl_zigbee_ezsp_status_t sl_zigbee_ezsp_serial_init(void)
{
  sli_legacy_serial_init(1, BAUD_115200, PARITY_NONE, 1);
  return SL_ZIGBEE_EZSP_SUCCESS;
}

void sl_zigbee_ezsp_serial_write_byte(uint8_t byte)
{
#ifdef WR_BAD_RANDOM
  uint16_t n = sl_zigbee_get_pseudo_random_number();
  if (n < 0xFFFF / WR_BAD_RANDOM) {
    if (n & 8) {
      byte = ASH_SUB;
    } else {
      byte ^= (1 << (n & 7));
    }
  }
#endif
  (void)sli_legacy_serial_write_byte(1, byte);
}

sl_zigbee_ezsp_status_t sl_zigbee_ezsp_serial_write_available(void)
{
  if (sli_legacy_serial_write_available(1) > 0) {
    return SL_ZIGBEE_EZSP_SUCCESS;
  } else {
    return SL_ZIGBEE_EZSP_NO_TX_SPACE;
  }
}

sl_zigbee_ezsp_status_t sl_zigbee_ezsp_serial_read_byte(uint8_t *byte)
{
  uint8_t ch;
  if (sli_legacy_serial_read_byte(1, &ch) == SL_STATUS_OK) {
    *byte = ch;
#ifdef RD_BAD_RANDOM
    uint16_t n = sl_zigbee_get_pseudo_random_number();
    if (n < 0xFFFF / RD_BAD_RANDOM) {
      if (n & 8) {
        *byte = ASH_SUB;
      } else {
        *byte ^= (1 << (n & 7));
      }
      if ( (*byte == ASH_XON) || (*byte == ASH_XOFF) ) {
        (*byte)++;
      }
    }
#endif
    return SL_ZIGBEE_EZSP_SUCCESS;
  } else {
    return SL_ZIGBEE_EZSP_NO_RX_DATA;
  }
}

sl_zigbee_ezsp_status_t sl_zigbee_ezsp_serial_read_available(uint16_t *count)
{
  *count = sli_legacy_serial_read_available(1);

  if (*count) {
    return SL_ZIGBEE_EZSP_SUCCESS;
  } else {
    return SL_ZIGBEE_EZSP_NO_RX_DATA;
  }
}

void sl_zigbee_ezsp_serial_read_flush(void)
{
  uint8_t byte;
  while (sl_zigbee_ezsp_serial_read_byte(&byte) == SL_ZIGBEE_EZSP_SUCCESS) {
  }
}

void sl_zigbee_ezsp_serial_write_flush(void)
{
}

void sl_zigbee_ezsp_debug_flush(void)
{
}

void sl_zigbee_ezsp_reset_dtr(void)
{
}

void sl_zigbee_ezsp_reset_custom(void)
{
}

void sl_zigbee_ezsp_serial_close(void)
{
}

bool sl_zigbee_ezsp_serial_output_is_idle(void)
{
  return true;
}

sl_zigbee_ezsp_status_t sl_zigbee_ezsp_setup_serial_port(int* serialPortFdReturn,
                                                         char* errorStringLocation,
                                                         int maxErrorLength,
                                                         bool bootloaderMode)
{
  return SL_ZIGBEE_EZSP_ERROR_INVALID_CALL;
}
