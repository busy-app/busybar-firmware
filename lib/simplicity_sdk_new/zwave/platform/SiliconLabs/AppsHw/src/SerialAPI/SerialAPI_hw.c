/**
 * @file
 * Platform abstraction for Serial API application
 *
 * @copyright 2022 Silicon Laboratories Inc.
 */
#include <SerialAPI_hw.h>
#include <app_hw.h>
#include <zw_region_config.h>
#include <utils.h>
#include <zpal_uart_config_ext.h>
#include <serial_api_config.h>
#include "serialapi_file.h"
#include "em_emu.h"
#include "em_cmu.h"

static const zpal_uart_config_ext_t zpal_uart_config_ext = {
#if defined(SERIAL_API_TX_PIN)
  .tx_pin = SERIAL_API_TX_PIN,
#endif /* defined(SERIAL_API_TX_PIN) */
#if defined(SERIAL_API_TX_PORT)
  .tx_port = SERIAL_API_TX_PORT,
#endif /* defined(SERIAL_API_TX_PORT) */
#if defined(SERIAL_API_TX_LOC)
  .tx_loc = SERIAL_API_TX_LOC,
#endif /* defined(SERIAL_API_TX_LOC) */
#if defined(SERIAL_API_RX_PIN)
  .rx_pin = SERIAL_API_RX_PIN,
#endif /* defined(SERIAL_API_RX_PIN) */
#if defined(SERIAL_API_RX_PORT)
  .rx_port = SERIAL_API_RX_PORT,
#endif /* defined(SERIAL_API_RX_PORT) */
#if defined(SERIAL_API_RX_LOC)
  .rx_loc = SERIAL_API_RX_LOC,
#endif /* defined(SERIAL_API_RX_LOC) */
};

void app_hw_init(void)
{
  uint8_t region;

  /* Unlatch EM4 GPIO pin states after wakeup (OK to call even if not EM4 wakeup) */
  /*If a device waked up from EM4 then all GPIO pins are isolated and we need to connected them to the GPIO registers.*/
  EMU_UnlatchPinRetention();

  /* Initialize the radio board (SAW filter setup, etc) */
  CMU_ClockEnable(cmuClock_GPIO, true);

  ReadApplicationRfRegion(&region);
}

const void * SerialAPI_get_uart_config_ext(void)
{
  return &zpal_uart_config_ext;
}
