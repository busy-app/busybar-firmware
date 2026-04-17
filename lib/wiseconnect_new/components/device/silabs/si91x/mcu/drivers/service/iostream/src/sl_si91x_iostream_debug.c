/***************************************************************************/ /**
 * @file
 * @brief SI91x IO Stream debug Component.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sli_si91x_iostream_swo_itm_8.h"
#include "sl_iostream.h"
#include "sl_si91x_iostream_debug.h"
#include "sl_status.h"
#include "sl_assert.h"
#include "sl_si91x_debug_swo.h"

/*******************************************************************************
 ******************************  LOCAL VARIABLES   **********************************
 ******************************************************************************/

static sl_si91x_iostream_swo_itm_8_msg_type_t debug_type = SI91X_DEBUG_PRINTF; //sets the debug type as debug printf
static uint8_t seq_number                                = 0;
static sl_iostream_t sl_si91x_iostream_debug;
sl_iostream_t *sl_si91x_iostream_debug_handle = &sl_si91x_iostream_debug;

sl_iostream_instance_info_t sl_si91x_iostream_instance_debug_info = {
  .handle    = &sl_si91x_iostream_debug,
  .name      = "debug",
  .type      = SL_SI91X_IOSTREAM_TYPE_DEBUG_OUTPUT,
  .periph_id = 0,
  .init      = sl_si91x_iostream_debug_init,
};

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t debug_write(void *context, const void *buffer, size_t buffer_length);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize debug stream component.
 ******************************************************************************/
sl_status_t sl_si91x_iostream_debug_init(void)
{
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_debug_swo_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Configure SWO stimulus 8
  sli_si91x_iostream_swo_itm_8_init();

  sl_si91x_iostream_debug.write = debug_write;
  sl_si91x_iostream_debug.read  = NULL;

  // Set default IO Stream to debug
  status = sl_iostream_set_system_default(&sl_si91x_iostream_debug);
  return status;
}

/***************************************************************************/ /**
 * Set debug stream type
 ******************************************************************************/
void sl_si91x_iostream_set_debug_type(sl_si91x_iostream_swo_itm_8_msg_type_t type)
{
  debug_type = type;
}

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Internal stream write implementation
 ******************************************************************************/
static sl_status_t debug_write(void *context, const void *buffer, size_t buffer_length)
{
  sl_status_t status = SL_STATUS_OK;
  sl_si91x_iostream_swo_itm_8_msg_type_t type;
  type   = debug_type;
  status = sli_si91x_iostream_swo_itm_8_write(context, buffer, buffer_length, type, seq_number);
  if (status == SL_STATUS_OK) {
    seq_number++; // Wrap arround/overflow on the usigned int is expected to reset the sequence number
  }

  return status;
}
