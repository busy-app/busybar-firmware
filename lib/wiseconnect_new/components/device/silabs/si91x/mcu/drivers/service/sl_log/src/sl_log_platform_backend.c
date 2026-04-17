/*******************************************************************************
 * @file sl_log_platform_backend.c
 * @brief Platform backend glue for the logging subsystem (SI91x)
 *
 * This file implements the platform-specific logging backend adapter which
 * forwards log events from the ring buffer to the selected backend (for
 * example, the proprietary ULP UART). It exposes the backend API used by the
 * generic logging layer and provides helper callbacks used by the UART driver.
 *
 * The implementations are intended to be small and interrupt-safe where
 * possible; synchronization model is documented per-function.
 */

#include "sl_log_platform_specific.h"
#include "sl_log.h"
#include "sl_si91x_usart.h"

/*******************************************************************************
 *                               GLOBAL VARIABLES
 ******************************************************************************/
/**
 * @brief USART configuration structure.
 *
 * This structure holds the configuration parameters for the USART used by
 * the logging backend. It is populated during initialization and used to
 * configure the USART peripheral.
 */
sl_si91x_usart_control_config_t uart_config;
/**
 * @brief USART handle.
 *
 * This handle is used to reference the USART instance used by the logging
 * backend. It is obtained during USART initialization and passed to USART
 * driver functions for sending data.
 */
sl_usart_handle_t uart_handle;
/**
 * @brief ULP UART driver instance.
 *
 * This is the CMSIS driver instance for the ULP UART peripheral. It provides
 * the low-level driver functions used by the logging backend to send data
 * over the UART interface.
 */
extern ARM_DRIVER_USART Driver_ULP_UART;
/**
 * @brief Initialize the selected logging backend.
 *
 * This prototype is exported to the generic logging code via the
 * sl_log_api_backend structure below. The implementation brings up the
 * platform transport (for example, UART) and prepares it for log output.
 *
 * Synchronization: called from the logger initialization path (single-threaded)
 *
 * @return SL_STATUS_OK on success, an sl_status_t error code otherwise.
 */
sl_status_t sl_log_hal_backend_init(void);

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
sl_status_t sl_log_hal_backend_write(sl_log_event_t *buffer, uint32_t read_index, uint32_t event_count);

/**
 * @brief Deinitialize the selected logging backend.
 *
 * Tear down any resources allocated by sl_log_hal_backend_init(). After this
 * call the backend is considered inactive until reinitialized.
 *
 * @return SL_STATUS_OK on success or an sl_status_t error code.
 */
sl_status_t sl_log_hal_backend_deinit(void);
/**
 * @brief   Logging backend API structure.
 */
sl_log_api_backend_t sl_log_api_backend = { .backend_init   = sl_log_hal_backend_init,
                                            .backend_write  = sl_log_hal_backend_write,
                                            .backend_deinit = sl_log_hal_backend_deinit };
/**
 * @brief   Initialize the proprietary logging backend
 * 
 * @param interface  The backend interface to initialize
 * @return sl_status_t  Status of the operation.
 */
sl_status_t sl_log_hal_backend_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  status = sl_si91x_usart_init(ULPUART, &uart_handle);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Configure the UART configurations
  status = sl_si91x_usart_set_configuration(uart_handle, &uart_config);
  if (status != SL_STATUS_OK) {
    return status;
  }
  NVIC_DisableIRQ(ULPSS_UART_IRQn);
  return status;
}

/**
 * @brief Writes log events from the circular buffer to the backend transport.
 *
 * Sends up to @p event_count events starting at @p read_index. Handles
 * wrap-around by performing up to two physical send operations.
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
    return SL_STATUS_INVALID_PARAMETER;
  }

  /* Validate parameters */
  if ((read_index >= SL_LOG_NUMBER_OF_EVENTS) || (buffer == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (event_count <= SL_LOG_NUMBER_OF_EVENTS - read_index) {
    /* contiguous block */
    status = sli_si91x_usart_send_data_blocking(uart_handle, &buffer[read_index], event_count * sizeof(sl_log_event_t));
    return status;
  } else {
    /* first chunk: from read_index to end */
    uint32_t first_chunk = SL_LOG_NUMBER_OF_EVENTS - read_index;
    status = sli_si91x_usart_send_data_blocking(uart_handle, &buffer[read_index], first_chunk * sizeof(sl_log_event_t));
    if (status != SL_STATUS_OK) {
      return status;
    }
    /* remaining events after wrap-around */
    uint32_t second_chunk = event_count - first_chunk; /* equals event_count + read_index - SL_LOG_NUMBER_OF_EVENTS */
    status = sli_si91x_usart_send_data_blocking(uart_handle, buffer, second_chunk * sizeof(sl_log_event_t));
    return status;
  }
}

/**
 * @brief Deinitialize the logging backend implementation.
 *
 * This function tears down the backend transport (for example the UART used
 * by the proprietary backend). It is safe to call during shutdown or when
 * reconfiguring the logging backend.
 *
 * @return SL_STATUS_OK on success or an sl_status_t error code from the
 *         underlying transport deinitialization function.
 */
sl_status_t sl_log_hal_backend_deinit(void)
{
  sl_status_t status = SL_STATUS_OK;
  status             = sl_si91x_usart_deinit(uart_handle);
  return status;
}

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
