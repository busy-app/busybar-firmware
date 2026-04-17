/***************************************************************************//**
 * @file
 * @brief Silicon Labs Secure Engine Manager Device Data API.
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
#ifndef SLI_SE_MANAGER_DEVICE_DATA_H
#define SLI_SE_MANAGER_DEVICE_DATA_H

#include "em_device.h"
#include "sl_status.h"
#include "sl_se_manager_types.h"
#include <stdint.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_SILICON_LABS_32B_SERIES_3) && defined(SLI_MAILBOX_COMMAND_SUPPORTED)

/// DeviceData sections
typedef enum {
  SLI_SE_DEVICE_DATA_CC = 0x00,
  SLI_SE_DEVICE_DATA_DI0 = 0x10,
  SLI_SE_DEVICE_DATA_DI1 = 0x11,
  SLI_SE_DEVICE_DATA_DI2 = 0x12,
  SLI_SE_DEVICE_DATA_DI3 = 0x13,
  SLI_SE_DEVICE_DATA_DI4 = 0x14,
  SLI_SE_DEVICE_DATA_DI5 = 0x15,
  SLI_SE_DEVICE_DATA_DI6 = 0x16,
  SLI_SE_DEVICE_DATA_DI7 = 0x17,
  SLI_SE_DEVICE_DATA_DI8 = 0x18,
  SLI_SE_DEVICE_DATA_DI9 = 0x19,
  SLI_SE_DEVICE_DATA_DI10 = 0x1A,
  SLI_SE_DEVICE_DATA_DI11 = 0x1B,
  SLI_SE_DEVICE_DATA_DI12 = 0x1C,
  SLI_SE_DEVICE_DATA_DI13 = 0x1D,
  SLI_SE_DEVICE_DATA_DI14 = 0x1E,
  SLI_SE_DEVICE_DATA_DI15 = 0x1F,
  SLI_SE_DEVICE_DATA_WAFER_PROBE = 0x20,
} sli_se_device_data_t;

/***************************************************************************//**
 * @brief
 *   Read an entire device data section
 *
 * @details
 *   Reads out an entire device data section to the output buffer, returns an error if
 *   the output buffer is not large enough to hold the entire section
 *
 * @param[in] cmd_ctx
 *  Pointer to an SE command context object
 * @param[in] section
 *  The device data section to read from
 * @param[out] output_data
 *  Buffer holding the output data
 * @param[in] output_data_size
 *  Size of the output_data buffer in bytes. This has to be exactly the size of the
 *  section to be read. Use @ref sli_se_device_data_get_size to find the size of element.
 *
 * @return
 *   Status code, @ref sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_device_data_read(sl_se_command_context_t *cmd_ctx,
                                    sli_se_device_data_t section,
                                    void *output_data,
                                    size_t output_data_size);

/***************************************************************************//**
 * @brief
 *   Read a chunk of a certain length of a device data section
 *
 * @param[in] cmd_ctx
 *  Pointer to an SE command context object
 * @param[in] section
 *  The device data section to read from
 * @param[in] offset
 *  Offset to read from within the specified device data section, must be divisable
 *  by 4.
 * @param[in] chunk_size
 *  Requested size of the chunk to read from offset. Must be divisible by 4.
 * @param[out] output_data
 *  Buffer holding the output data, must be larger than or equal to
 *  @ref size.
 *
 * @return
 *   Status code, @ref sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_device_data_read_chunk(sl_se_command_context_t *cmd_ctx,
                                          sli_se_device_data_t section,
                                          uint32_t offset,
                                          size_t chunk_size,
                                          void *output_data);

/***************************************************************************//**
 * @brief
 *   Read one word (4 bytes) from an address offset of a device data section
 *
 * @param[in] cmd_ctx
 *  Pointer to an SE command context object
 * @param[in] section
 *  The device data section to read from
 * @param[in] offset
 *  Offset in bytes to read from within the specified device data section, must be divisable
 *  by 4.
 * @param[out] output_data
 *  Pointer to hold the output data
 *
 * @return
 *   Status code, @ref sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_device_data_read_word(sl_se_command_context_t *cmd_ctx,
                                         sli_se_device_data_t section,
                                         uint32_t offset,
                                         uint32_t *output_data);

/***************************************************************************//**
 * @brief
 *   Get the size of a device data section
 *
 * @param[in] cmd_ctx
 *  Pointer to an SE command context object
 * @param[in] section
 *  The device data section to read from
 * @param[out] size
 *  Size of the device data section
 *
 * @return
 *   Status code, @ref sl_status.h.
 ******************************************************************************/
sl_status_t sli_se_device_data_get_size(sl_se_command_context_t *cmd_ctx,
                                        sli_se_device_data_t section,
                                        uint32_t *size);

/***************************************************************************//**
 * @brief
 *   Read the value from an address in the CC section
 *
 * @param[in] cmd_ctx
 *  Pointer to an SE command context object
 * @param[in] cc_address
 *  CC address to read value from, must be word aligned and non-zero
 * @param[out] cc_value
 *  Value from CC table corresponding to @ref cc_address
 *
 * @return
 *   SL_STATUS_OK if value found
 *   SL_STATUS_NOT_INITIALIZED if \ref cc_address does not point to valid CC entry
 *   SL_STATUS_INVALID_PARAM if input param is invalid value
 ******************************************************************************/
sl_status_t sli_se_device_data_get_cc_value(sl_se_command_context_t *cmd_ctx,
                                            uint32_t cc_address,
                                            uint32_t *cc_value);

#endif // _SILICON_LABS_32B_SERIES_3

#ifdef __cplusplus
}
#endif

#endif // SLI_SE_MANAGER_DEVICE_DATA_H
