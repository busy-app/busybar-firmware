/**
 * @file sl_log_backend_segger.c
 * @brief SEGGER SystemView backend implementation for Silicon Labs logging system
 *
 * This file implements the SEGGER SystemView backend for the Silicon Labs logging
 * framework. It provides integration with SEGGER SystemView for real-time
 * event visualization and debugging. The implementation handles event recording
 * with various argument counts and manages SystemView initialization and
 * configuration.
 *
 * @author Silicon Labs
 * @date October 2025
 * @version 1.0
 */

#include "SEGGER_SYSVIEW.h"
#include "sl_log_platform_specific.h"
#include "sl_log_common_config.h"


/**
 * @brief Global timestamp variable for SystemView events
 *
 * This variable stores the timestamp of the last recorded event.
 * It is updated each time an event is recorded to SystemView.
 * The timestamp is used for event correlation and timing analysis.
 * @todo Temporary variable used to pass logged event timestamps to SystemView
 *      It will be deprecated after the SystemView API integration is implemented.
 */
uint32_t timestamp_global;

sl_status_t sl_log_systemview_write(sl_log_event_t *buffer, uint32_t read_index, uint32_t event_count);
sl_status_t sl_log_systemview_init(void);
sl_status_t sl_log_systemview_deinit(void);
sl_status_t sl_log_systemview_record_event(sl_log_event_t *event);

sl_log_api_backend_t sl_log_api_backend={
  .backend_init = sl_log_systemview_init,
  .backend_write = sl_log_systemview_write,
  .backend_deinit = sl_log_systemview_deinit
};
/**
 * @brief Initialize the SystemView logging backend
 *
 * Initializes the SEGGER SystemView component for event recording.
 * This function must be called before any events can be recorded
 * to the SystemView backend. It configures SystemView and starts
 * the recording session.
 *
 * @note This function should be called once during system initialization.
 *       Multiple calls to this function may cause undefined behavior.
 *
 * @see sl_log_systemview_deinit()
 * @see sl_log_systemview_config()
 */
sl_status_t sl_log_systemview_init(void)
{
  SEGGER_SYSVIEW_Conf();
  SEGGER_SYSVIEW_Start();
  return SL_STATUS_OK;
}

/**
 * @brief Deinitialize the SystemView logging backend
 *
 * Performs cleanup and deinitialization of the SystemView component.
 * This function should be called when the SystemView backend is no
 * longer needed or during system shutdown.
 *
 * @note After calling this function, no events should be recorded
 *       until sl_log_systemview_init() is called again.
 *
 * @see sl_log_systemview_init()
 */
sl_status_t sl_log_systemview_deinit(void)
{
  SEGGER_SYSVIEW_Stop();
  return SL_STATUS_OK;
}

/**
 * @brief Writes (records) a sequence of log events to the SystemView backend.
 *
 * Iterates over a ring buffer of log events starting at the provided read_index and
 * attempts to record up to event_count events via sl_log_systemview_record_event().
 * The read_index wraps to 0 when it reaches SL_LOG_NUMBER_OF_EVENTS (ring buffer behavior).
 * Processing stops early if any call to sl_log_systemview_record_event() returns a status
 * different from SL_STATUS_OK.
 *
 * @param[in] buffer
 *   Pointer to the ring buffer containing log events. The buffer is expected to have
 *   at least SL_LOG_NUMBER_OF_EVENTS elements.
 *
 * @param[in] read_index
 *   Starting index within the ring buffer (0 <= read_index <= SL_LOG_NUMBER_OF_EVENTS).
 *   If equal to SL_LOG_NUMBER_OF_EVENTS, it is immediately wrapped to 0.
 *
 * @param[in] event_count
 *   Number of events to attempt to record. Iteration stops earlier if an error occurs.
 *
 * @return sl_status_t
 *   - SL_STATUS_OK if all requested events were successfully recorded.
 *   - The first non-OK status returned by sl_log_systemview_record_event() if a failure occurs.
 *
 */
sl_status_t sl_log_systemview_write(sl_log_event_t *buffer, uint32_t read_index, uint32_t event_count)
{
  sl_status_t status = SL_STATUS_OK;
  if(event_count == 0){
    return SL_STATUS_OK;
  }
  if(buffer == NULL){
    return SL_STATUS_INVALID_PARAMETER;
  }
  if(read_index >= SL_LOG_NUMBER_OF_EVENTS || event_count > SL_LOG_NUMBER_OF_EVENTS){
    return SL_STATUS_INVALID_PARAMETER;
  }
  for (uint32_t i = 0; i < event_count; i++) {
    if (read_index == SL_LOG_NUMBER_OF_EVENTS) {
      read_index = 0;
    }
    status = sl_log_systemview_record_event(&buffer[read_index]);
    if(status!=SL_STATUS_OK){
      break;
    }
    read_index++;
  }
  return status;
}

/**
 * @brief Record a log event to SystemView
 *
 * Records a log event to the SystemView backend with appropriate formatting
 * based on the number of arguments. This function handles events with 0-3
 * arguments and updates the global timestamp for event correlation.
 *
 * The function dispatches to different SystemView recording functions based
 * on the argument count:
 * - 0 arguments: SEGGER_SYSVIEW_RecordVoid()
 * - 1 argument: SEGGER_SYSVIEW_RecordU32()
 * - 2 arguments: SEGGER_SYSVIEW_RecordU32x2()
 * - 3 arguments: SEGGER_SYSVIEW_RecordU32x3()
 *
 * @param[in] event Pointer to the log event structure containing:
 *                  - timestamp: Event timestamp in system timer units
 *                  - event_id: Unique event identifier
 *                  - args: Array of up to 3 32-bit arguments
 *                  - arg_count: Number of valid arguments (0-3)
 *                  - Additional metadata (core_id, flags, version)
 */
sl_status_t sl_log_systemview_record_event(sl_log_event_t *event)
{
  sl_status_t status = SL_STATUS_OK;
  if (event == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if(event->arg_count > SL_LOG_CONFIG_ARG){
    return SL_STATUS_INVALID_PARAMETER;
  }
  timestamp_global = event->timestamp;
  switch (event->arg_count) {
    case 0:
      SEGGER_SYSVIEW_RecordVoid(event->event_id);
      break;
    case 1:
      SEGGER_SYSVIEW_RecordU32(event->event_id, event->args[0]);
      break;
    case 2:
      SEGGER_SYSVIEW_RecordU32x2(event->event_id, event->args[0], event->args[1]);
      break;
    case 3:
      SEGGER_SYSVIEW_RecordU32x3(event->event_id, event->args[0], event->args[1], event->args[2]);
      break;
    default:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
  }
  return status;
}

/**
 * @brief Retrieve the SEGGER logging backend API instance.
 *
 * This function returns a pointer to the statically allocated (or globally
 * defined) SEGGER logging backend API structure. It allows the logging
 * framework to access the concrete backend implementation (function pointers etc.).
 *
 *
 * @return sl_log_api_backend_t* Pointer to the SEGGER log backend API struct.
 */
sl_log_api_backend_t * sl_log_get_api_backend(void){
  return &sl_log_api_backend;
}
