/***************************************************************************//**
 * @file
 * @brief Platform backend glue for the logging subsystem
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_log_proprietary_config.h"
#include "sl_log_platform_specific.h"
#include "sl_log_helper.h"
#include "sl_log.h"
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#include "em_device.h"

/*******************************************************************************
**************************   LOCAL FUNCTIONS   ********************************
*******************************************************************************/

/**
 * @brief Initialize the logging backend.
 *
 * This prototype is exported to the generic logging code via the
 * sl_log_api_backend structure below. The implementation brings up the
 * platform transport (for example, UART) and prepares it for log output.
 *
 * Synchronization: called from the logger initialization path (single-threaded)
 *
 * @return SL_STATUS_OK on success, an sl_status_t error code otherwise.
 */
sl_status_t sl_log_hal_backend_init(void)
{
  return sl_iostream_set_default(SL_LOG_IOSTREAM_HANDLE);
}

/**
 * @brief Write log events to the backend transport.
 *
 * This function sends up to event_count events from the circular event buffer
 * starting at read_index. It handles wrap-around by performing up to two
 * physical sends. The function forwards raw event bytes to the platform
 * transport driver (UART in proprietary backend).
 *
 * Synchronization: single-consumer (the logger flush path) calls this. It is
 * not safe for concurrent calls from multiple consumers.
 *
 * @param[in] buffer      Pointer to the circular buffer containing events
 * @param[in] read_index  Index (0..SL_LOG_NUMBER_OF_EVENTS-1) of first event
 * @param[in] event_count Number of events to send
 *
 * @return SL_STATUS_OK on success or an sl_status_t error code from the
 *         underlying transport send operation.
 */
sl_status_t sl_log_hal_backend_write(sl_log_event_t *buffer, uint32_t read_index, uint32_t event_count)
{
  sl_status_t status = SL_STATUS_OK;

  if (event_count == 0) {
    return SL_STATUS_OK;
  }

  if ((read_index >= SL_LOG_NUMBER_OF_EVENTS) || (buffer == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (event_count <= SL_LOG_NUMBER_OF_EVENTS - read_index) {
    /* contiguous block */
    status = sl_iostream_write(SL_LOG_IOSTREAM_HANDLE, &buffer[read_index], event_count * sizeof(sl_log_event_t));
    return status;
  } else {
    /* first chunk: from read_index to end */
    uint32_t first_chunk = SL_LOG_NUMBER_OF_EVENTS - read_index;
    status = sl_iostream_write(SL_LOG_IOSTREAM_HANDLE, &buffer[read_index], first_chunk * sizeof(sl_log_event_t));
    if (status != SL_STATUS_OK) {
      return status;
    }

    /* remaining events after wrap-around */
    uint32_t second_chunk = event_count - first_chunk; /* equals event_count + read_index - SL_LOG_NUMBER_OF_EVENTS */
    status = sl_iostream_write(SL_LOG_IOSTREAM_HANDLE, buffer, second_chunk * sizeof(sl_log_event_t));
    return status;
  }
}

/**
 * @brief Deinitialize the logging backend.
 *
 * Tear down any resources allocated by sl_log_hal_backend_init(). After this
 * call the backend is considered inactive until reinitialized.
 *
 * @return SL_STATUS_OK on success or an sl_status_t error code.
 */
sl_status_t sl_log_hal_backend_deinit(void)
{
  return SL_STATUS_OK;
}

/**
 * @brief   Logging backend API structure.
 */
sl_log_api_backend_t sl_log_api_backend = { .backend_init   = sl_log_hal_backend_init,
                                            .backend_write  = sl_log_hal_backend_write,
                                            .backend_deinit = sl_log_hal_backend_deinit };

/**
 * @brief Return pointer to the backend API structure.
 *
 * Provides the generic logging core with the platform-specific backend
 * implementation (init/write/deinit). This function returns a pointer to the
 * statically allocated `sl_log_api_backend` structure.
 *
 * @return Pointer to the populated sl_log_api_backend_t structure.
 */
sl_log_api_backend_t *sl_log_get_api_backend(void)
{
  return &sl_log_api_backend;
}
