/***************************************************************************//**
 * @brief Handles RAIL assertion failures and outputs the error code via
 *        UART if available.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_component_catalog.h"
#include "sl_rail.h"
#include "sl-connect-assert.h"
#ifdef SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
#include "sl_iostream.h"
#endif

void sl_railcb_assert_failed(sl_rail_handle_t rail_handle,
                             sl_rail_assert_error_codes_t error_code,
                             int line)
{
  (void)rail_handle;
  (void)error_code;
  (void)line;
#if defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
  sl_iostream_printf(SL_IOSTREAM_STDOUT, "\r\n[ASSERT:librail.a:%ld at %d]\r\n", error_code, line);
#endif // SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT
  CONNECT_STACK_ASSERT(false);
}
