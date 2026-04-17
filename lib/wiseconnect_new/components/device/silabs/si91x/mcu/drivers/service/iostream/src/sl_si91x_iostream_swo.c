/***************************************************************************/ /**
 * @file
 * @brief SI91x IO Stream SWO Component.
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
#include "sl_iostream.h"
#include "sl_si91x_iostream_swo.h"
#include "sl_status.h"
#include "sl_si91x_iostream_swo_config.h"
#include "sl_si91x_debug_swo.h"

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static sl_status_t swo_write(void *context, const void *buffer, size_t buffer_length);

/*******************************************************************************
 ******************************   VARIABLES   **********************************
 ******************************************************************************/

static sl_iostream_t sl_si91x_iostream_swo = { .write   = swo_write,
                                               .read    = NULL,
                                               .context = NULL }; // Public stream variable for SWO io component

sl_iostream_t *sl_si91x_iostream_swo_handle = &sl_si91x_iostream_swo;

sl_iostream_instance_info_t sl_si91x_iostream_instance_swo_info = {
  .handle    = &sl_si91x_iostream_swo,
  .name      = "swo",
  .type      = SL_SI91X_IOSTREAM_TYPE_SWO,
  .periph_id = 0,
  .init      = sl_si91x_iostream_swo_init,
};

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 * This function initializes iostream swo module by enabling itm
  *******************************************************************************/
sl_status_t sl_si91x_iostream_swo_init(void)
{
  sl_status_t ret = SL_STATUS_OK;
  ret             = sl_si91x_debug_swo_init();
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  // Configure SWO module
  ret = sl_si91x_debug_swo_enable_itm(0U);
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  // Set default IO Stream to SWO
  sl_iostream_set_system_default(&sl_si91x_iostream_swo);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This function de-initializes iostream swo module by disabling ITM
  *******************************************************************************/
sl_status_t sl_si91x_iostream_swo_deinit(void)
{

  // De-configure ITM
  // We cannot fully deconfigure the debug/trace system, since
  // it may be in use by others.
  return sl_si91x_debug_swo_disable_itm(0U);
}

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************/ /**
 * SWO stream component write function.
 ******************************************************************************/
static sl_status_t swo_write(void *context, const void *buffer, size_t buffer_length)
{
  sl_status_t ret = SL_STATUS_OK;
  uint8_t *buf_8  = (uint8_t *)buffer;
#if (SL_SI91X_IOSTREAM_SWO_WRITE_BYTE_PER_BYTE == 0)
  uint32_t *buf_32 = (uint32_t *)(buffer);
  uint16_t *buf_16 = 0;
#endif

  (void)context;

#if (SL_SI91X_IOSTREAM_SWO_WRITE_BYTE_PER_BYTE == 1)
  // Write buffer
  for (size_t i = 0; i < buffer_length; i++) {
    ret = sl_si91x_debug_swo_write_u8(0, buf_8[i]);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
  }
#else
  // Write the maximum number of words
  while (buffer_length >= sizeof(uint32_t)) {
    ret = sl_si91x_debug_swo_write_u32(0, *buf_32);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    buf_32++;
    buffer_length -= sizeof(uint32_t);
  }

  // Write a last half-word if remaining
  buf_16 = (uint16_t *)buf_32;
  if (buffer_length >= sizeof(uint16_t)) {
    ret = sl_si91x_debug_swo_write_u16(0, *buf_16);
    if (ret != SL_STATUS_OK) {
      return ret;
    }
    buf_16++;
    buffer_length -= sizeof(uint16_t);
  }

  // Write the last byte if remaining
  buf_8 = (uint8_t *)buf_16;
  if (buffer_length == sizeof(uint8_t)) {
    ret = sl_si91x_debug_swo_write_u8(0, *buf_8);
  }
#endif

  return ret;
}
