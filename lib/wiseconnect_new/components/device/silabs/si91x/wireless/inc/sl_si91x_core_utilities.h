/***************************************************************************/ /**
 * @file
 * @brief
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
#pragma once
#include "sli_wifi_types.h"
#include "sl_si91x_types.h"
#include "sl_wifi_device.h"
#include "sl_additional_status.h"
#ifndef __ZEPHYR__
#include "sli_cmsis_os2_ext_task_register.h"

/// External variable representing the index of the thread local array at which the firmware status will be stored.
extern sli_task_register_id_t sli_fw_status_storage_index;

/** \addtogroup SI91X_DRIVER_FUNCTIONS 
 * \ingroup SL_SI91X_API
 * @{ */
/***************************************************************************/ /**
 * @brief 
 *   Retrieves the saved thread-specific firmware status value.
 *  
 * @details
 *   This function fetches the firmware status value that is specific to the current thread.
 * 
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
static inline SL_DEPRECATED_API_WISECONNECT_4_0 sl_status_t sl_si91x_get_saved_firmware_status(void)
{
  sl_status_t status = SL_STATUS_FAIL;

  sli_osTaskRegisterGetValue(NULL, sli_fw_status_storage_index, &status);
  return status;
}
#endif
/** @} */

/******************************************************************************
 * A utility function to extract firmware status from RX packet.
 * The extracted firmware status can be given to sli_convert_and_save_firmware_status() to get sl_status equivalent.
 * @param packet packet that contains the frame status which needs to be extracted.
 * @return  frame status
 *****************************************************************************/
static inline uint16_t sli_get_si91x_frame_status(const sl_wifi_system_packet_t *packet)
{
  return (uint16_t)(packet->desc[12] + (packet->desc[13] << 8));
}

#ifndef __ZEPHYR__
/******************************************************************************
 * @brief
 * 	A utility function that store the firmware status code in thread specific storage.
 * @param[in] converted_firmware_status
 *	Firmware status code that needs to be saved.
 *****************************************************************************/
static inline void sli_save_si91x_firmware_status(sl_status_t converted_firmware_status)
{
  sli_osTaskRegisterSetValue(NULL, sli_fw_status_storage_index, converted_firmware_status);
}
#endif

/******************************************************************************
 * @brief
 *   A utility function that converts frame status sent by firmware to sl_status_t and stores in thread local storage of caller thread.
 * @param[in] si91x_firmware_status
 *   si91x_firmware_status that needs to be converted to sl_status_t.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 *****************************************************************************/
static inline sl_status_t sli_convert_and_save_firmware_status(uint16_t si91x_firmware_status)
{
  sl_status_t converted_firmware_status = (si91x_firmware_status == SL_STATUS_OK) ? SL_STATUS_OK
                                                                                  : (si91x_firmware_status | BIT(16));
#ifndef __ZEPHYR__
  sli_save_si91x_firmware_status(converted_firmware_status);
#endif
  return converted_firmware_status;
}

/******************************************************************************
 * @brief
 *   A utility function that converts si91x_status_t to sl_status_t
 * @param[in] si91x_status
 *   si91x_status that needs to be converted to sl_status_t.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 *****************************************************************************/
static inline sl_status_t sli_convert_si91x_status_to_sl_status(si91x_status_t si91x_status)
{
  switch (si91x_status) {
    case RSI_ERROR_NONE:
      return SL_STATUS_OK;
    case RSI_ERROR_TIMEOUT:
      return SL_STATUS_TIMEOUT;
    case RSI_ERROR_INVALID_PARAM:
      return SL_STATUS_INVALID_PARAMETER;
    case RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE:
      return SL_STATUS_INVALID_STATE;
    case RSI_ERROR_PKT_ALLOCATION_FAILURE:
      return SL_STATUS_ALLOCATION_FAILED;
    case RSI_ERROR_COMMAND_NOT_SUPPORTED:
      return SL_STATUS_NOT_SUPPORTED;
    case RSI_ERROR_INSUFFICIENT_BUFFER:
      return SL_STATUS_NO_MORE_RESOURCE;
    case RSI_ERROR_IN_OS_OPERATION:
      return SL_STATUS_OS_OPERATION_FAILURE;
    case RSI_ERROR_BOOTUP_OPTIONS_NOT_SAVED:
      return SL_STATUS_BOOTUP_OPTIONS_NOT_SAVED;
    case RSI_ERROR_BOOTLOADER_VERSION_NOT_MATCHING:
      return SL_STATUS_BOOTLOADER_VERSION_MISMATCH;
    case RSI_ERROR_WAITING_FOR_BOARD_READY:
      return SL_STATUS_WAITING_FOR_BOARD_READY;
    case RSI_ERROR_VALID_FIRMWARE_NOT_PRESENT:
      return SL_STATUS_VALID_FIRMWARE_NOT_PRESENT;
    case RSI_ERROR_INVALID_OPTION:
      return SL_STATUS_INVALID_OPTION;
    case RSI_ERROR_SPI_BUSY:
      return SL_STATUS_SPI_BUSY;
    case RSI_ERROR_CARD_READY_TIMEOUT:
      return SL_STATUS_CARD_READY_TIMEOUT;
    case RSI_ERROR_FW_LOAD_OR_UPGRADE_TIMEOUT:
      return SL_STATUS_FW_LOAD_OR_UPGRADE_TIMEOUT;
    default:
      return SL_STATUS_FAIL;
  }
}

/**
 * @brief Atomically append a given buffer to the end of a buffer queue.
 * 
 * This function appends a buffer to the end of a specified buffer queue in an atomic operation,
 * ensuring thread safety during the append operation.
 *
 * @param[in] queue Pointer to the destination buffer queue where the buffer will be appended.
 * @param[in] buffer Pointer to the buffer that is to be appended to the queue.
 */
void sli_si91x_append_to_buffer_queue(sli_wifi_buffer_queue_t *queue, sl_wifi_buffer_t *buffer);

/**
 * @brief
 *   Allocate a buffer to send a command
 * @param[out] host_buffer
 *   Destination buffer object
 * @param[out] buffer
 *   Start of the internal buffer data
 * @param[in] requested_buffer_size
 *   Requested buffer size
 * @param[in] wait_duration_ms
 *   Duration to wait for buffer to become available
 * @return sl_status_t Returns the status of the operation. A value of 0 (SL_STATUS_OK) indicates success.
 *                     Other values indicate failure. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 */
sl_status_t sli_si91x_allocate_command_buffer(sl_wifi_buffer_t **host_buffer,
                                              void **buffer,
                                              uint32_t requested_buffer_size,
                                              uint32_t wait_duration_ms);

/******************************************************************************
 * @brief
 *   Check if buffer queue is empty
 * @param[in] queue
 *   Requested buffer size
 * @return
 *   true if empty; false if not empty.
 *****************************************************************************/
static inline bool sli_si91x_buffer_queue_empty(const sli_wifi_buffer_queue_t *queue)
{
  return (queue->head == NULL);
}
