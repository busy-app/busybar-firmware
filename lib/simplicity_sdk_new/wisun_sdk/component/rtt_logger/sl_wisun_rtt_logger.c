/***************************************************************************//**
 * @file sl_wisun_rtt_logger.c
 * @brief Wi-SUN RTT Logger
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sl_assert.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_common.h"
#include "sl_status.h"
#include "sl_sleeptimer.h"
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#include "sl_wisun_rtt_logger.h"
#include "sl_wisun_rtt_logger_config.h"

#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// RTT Logger Thread stack size in words
#define SL_WISUN_RTT_LOGGER_STACK_SIZE_WORD             (256UL)

// RTT Logger Event flag all mask
#define SL_WISUN_RTT_LOGGER_EVT_ALL_MSK                 (0x00FFFFFFUL)

// RTT Logger Event flag start report mask
#define SL_WISUN_RTT_LOGGER_EVT_START_REPORT_MSK        (0x00000001UL << 0UL)

// RTT Logger Event flag stop report mask
#define SL_WISUN_RTT_LOGGER_EVT_STOP_REPORT_MSK         (0x00000001UL << 1UL)

// RTT Logger Event flag error mask
#define SL_WISUN_RTT_LOGGER_EVT_ERROR_MSK               (0x00000001UL << 31UL)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Task function
 * @details This function is the main task for the RTT Logger
 *
 * @param[in] args Arguments
 *****************************************************************************/
static void _rtt_logger_task_fnc(void *args);

/**************************************************************************//**
 * @brief RTT report timer callback
 * @details This function is the callback for the RTT report timer
 *
 * @param[in] handle The timer handle
 * @param[in] data The data
 *****************************************************************************/
static void _rtt_report_timer_cb(sl_sleeptimer_timer_handle_t *handle, void *data);

/**************************************************************************//**
 * @brief Read SEGGER RTT up buffer
 * @details This function reads the SEGGER RTT up buffer
 *
 * @param[in] buffer_index The buffer index
 * @param[out] p_data The data read
 * @param[in] buffer_size The size of the buffer
 *
 * @return The number of bytes read
 *****************************************************************************/
static uint32_t _read_segger_up_buffer(uint32_t buffer_index, void *p_data, uint32_t buffer_size);

/**************************************************************************//**
 * @brief Send RTT logs
 * @details This function sends RTT logs
 *
 * @param[in] rtt_log_buffer The RTT log buffer
 *****************************************************************************/
static void _send_rtt_logs(uint8_t * const rtt_log_buffer);
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// RTT Logger task ID
static osThreadId_t _rtt_logger_task = NULL;

// RTT Logger task attributes
static const osThreadAttr_t _rtt_logger_task_attr = {
  .name       = "RTT_logger_task",
  .attr_bits  = osThreadDetached,
  .cb_mem     = NULL,
  .cb_size    = 0UL,
  .stack_mem  = NULL,
  .stack_size = (SL_WISUN_RTT_LOGGER_STACK_SIZE_WORD * sizeof(void *)) & 0xFFFFFFF8U,
  .priority   = osPriorityNormal1,
  .tz_module  = 0UL,
  .reserved   = 0UL
};

// RTT Logger event flags
static osEventFlagsId_t _rtt_logger_evt = NULL;

// RTT Logger event flags attributes
static const osEventFlagsAttr_t _rtt_logger_evt_attr = {
  .name      = "RTTLoggerEvtFlags",
  .attr_bits = 0UL,
  .cb_mem    = NULL,
  .cb_size   = 0UL
};

// RTT report timer handle
static sl_sleeptimer_timer_handle_t _rtt_report_timer_handle = { 0 };
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/* RTT Logger init */
void sl_wisun_rtt_logger_init(void)
{
  // Create event flags
  _rtt_logger_evt = osEventFlagsNew(&_rtt_logger_evt_attr);
  EFM_ASSERT(_rtt_logger_evt != NULL);

  // Create task
  _rtt_logger_task = osThreadNew(_rtt_logger_task_fnc, NULL, &_rtt_logger_task_attr);
  EFM_ASSERT(_rtt_logger_task != NULL);
}

/* Start RTT report */
void sl_wisun_rtt_logger_start_report(void)
{
  sl_status_t ret = SL_STATUS_FAIL;

  // Start periodic timer
  ret = sl_sleeptimer_start_periodic_timer_ms(&_rtt_report_timer_handle,
                                              SL_WISUN_RTT_LOGGER_TIMER_REPORT_PERIOD_S * 1000UL,
                                              _rtt_report_timer_cb,
                                              NULL,
                                              0U,
                                              0U);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: RTT report timer start]\n");
  }
}

/* Stop RTT report */
void sl_wisun_rtt_logger_stop_report(void)
{
  sl_status_t ret = SL_STATUS_FAIL;

  // Stop periodic timer
  ret = sl_sleeptimer_stop_timer(&_rtt_report_timer_handle);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: RTT report timer stop]\n");
    return;
  }

  (void) osEventFlagsSet(_rtt_logger_evt, SL_WISUN_RTT_LOGGER_EVT_STOP_REPORT_MSK);
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/* RTT Logger task function */
static void _rtt_logger_task_fnc(void *args)
{
  uint32_t flags = 0UL;
  static uint8_t rtt_log_buffer[BUFFER_SIZE_UP] = { 0U };

  (void) args;

  // Clear all event flags
  (void) osEventFlagsClear(_rtt_logger_evt, SL_WISUN_RTT_LOGGER_EVT_ALL_MSK);

  // RTT Logger thread loop
  SL_WISUN_RTT_LOGGER_THREAD_LOOP {
    flags = osEventFlagsWait(_rtt_logger_evt,
                             SL_WISUN_RTT_LOGGER_EVT_START_REPORT_MSK
                             | SL_WISUN_RTT_LOGGER_EVT_STOP_REPORT_MSK,
                             osFlagsWaitAny,
                             osWaitForever);

    if (flags & SL_WISUN_RTT_LOGGER_EVT_ERROR_MSK) {
      printf("[Failed: RTT Logger evt flags (0x%08lX)]\n", flags);
      continue;
    }

    if (flags & SL_WISUN_RTT_LOGGER_EVT_START_REPORT_MSK) {
      // Send RTT logs
      _send_rtt_logs(rtt_log_buffer);
    } else if (flags & SL_WISUN_RTT_LOGGER_EVT_STOP_REPORT_MSK) {
      printf("[RTT reporting is stopped]\n");
    }
  }
}

/* RTT report timer callback */
static void _rtt_report_timer_cb(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;

  (void) osEventFlagsSet(_rtt_logger_evt, SL_WISUN_RTT_LOGGER_EVT_START_REPORT_MSK);
}

/**
 * This function is an adaptation of SEGGER_RTT_ReadNoLock(). As Segger does not provide
 * any API to read from up buffers, a little work around was necessary.
 */
static uint32_t _read_segger_up_buffer(uint32_t buffer_index, void *p_data, uint32_t buffer_size)
{
  uint32_t num_bytes_rem = 0UL;
  uint32_t num_bytes_read = 0UL;
  uint32_t rd_off = 0UL;
  uint32_t wr_off = 0UL;
  uint8_t *p_buffer = NULL;
  SEGGER_RTT_BUFFER_UP * p_ring = NULL;

  p_ring = &_SEGGER_RTT.aUp[buffer_index];
  p_buffer = (uint8_t *)p_data;
  rd_off = p_ring->RdOff;
  wr_off = p_ring->WrOff;
  num_bytes_read = 0U;

  // Read from current read position to wrap-around of buffer, first
  if (rd_off > wr_off) {
    num_bytes_rem = p_ring->SizeOfBuffer - rd_off;
    num_bytes_rem = SL_MIN(num_bytes_rem, buffer_size);
    memcpy(p_buffer, p_ring->pBuffer + rd_off, num_bytes_rem);
    num_bytes_read += num_bytes_rem;
    p_buffer += num_bytes_rem;
    buffer_size -= num_bytes_rem;
    rd_off += num_bytes_rem;

    // Handle wrap-around of buffer
    if (rd_off == p_ring->SizeOfBuffer) {
      rd_off = 0U;
    }
  }

  // Read remaining items of buffer
  num_bytes_rem = wr_off - rd_off;
  num_bytes_rem = SL_MIN(num_bytes_rem, buffer_size);
  if (num_bytes_rem > 0U) {
    memcpy(p_buffer, p_ring->pBuffer + rd_off, num_bytes_rem);
    num_bytes_read += num_bytes_rem;
    p_buffer += num_bytes_rem;
    buffer_size -= num_bytes_rem;
    rd_off += num_bytes_rem;
  }
  if (num_bytes_read) {
    p_ring->RdOff = rd_off;
  }

  return num_bytes_read;
}

/* Send RTT logs */
static void _send_rtt_logs(uint8_t * const rtt_log_buffer)
{
  // Retrieve logs from SEGGER_RTT
  // SEGGER does not provide any API to read the content of its up buffers (target to host).
  // However, the _SEGGER_RTT structure is globally accessible from the SEGGER_RTT.h file.
  // It contains every buffer structure and metadata. The stack writes logs in the up buffer 0.
  // We need to lock RTT logs writing and read data inside the stack's RTT up buffer.
  uint16_t read_bytes = 0U;
  sl_iostream_t *iostream = NULL;

  if (rtt_log_buffer == NULL) {
    return;
  }

  SEGGER_RTT_LOCK();
  read_bytes = _read_segger_up_buffer(0U, rtt_log_buffer, BUFFER_SIZE_UP);
  SEGGER_RTT_UNLOCK();

  if (read_bytes > 0U) {
    iostream = sl_iostream_get_handle("spp");
    if (iostream != NULL) {
      (void) sl_iostream_printf(iostream, "%.*s", read_bytes, rtt_log_buffer);
    }
  }
}
