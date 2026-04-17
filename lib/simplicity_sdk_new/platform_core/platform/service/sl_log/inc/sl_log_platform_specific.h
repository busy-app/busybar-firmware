#ifndef SL_LOG_PLATFORM_SPECIFIC_H
#define SL_LOG_PLATFORM_SPECIFIC_H

#include "sl_log.h"

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * @brief platform specific API function pointers
   *
   * Contains function pointers for all platform-specific logging operations.
   * These functions are provided by the platform-specific implementation
   * and handle platform core timing, power management, and backend operations.
   */
  typedef struct {
    /** @brief Initialize the platform core */
    sl_status_t (*platform_core_init)(void);
    /** @brief  Deinitialize the platform core */
    sl_status_t (*platform_core_deinit)(void);
    /** @brief Get the frequency of the timestamp timer */
    uint32_t (*get_timestamp_timer_frequency)(uint8_t core_id);
    /** @brief Prepare logging system for sleep mode */
    sl_status_t (*pre_sleep_process)(void * args);
    /** @brief Initialize logging system after wake-up */
    sl_status_t (*post_sleep_process)(void * args);
    /** @brief Get timestamp from captive core (1µs resolution counter) */
    uint32_t (*get_timestamp)(uint8_t core_id);
    /** @brief Set captive core-specific configuration parameters */
    sl_status_t (*set_configuration)(void *args, uint8_t core_id);
    /** @brief Get captive core-specific configuration parameters */
    sl_status_t (*get_configuration)(void *args, uint8_t core_id);
    /** @brief Synchronize timestamps between host and captive core */
    sl_status_t (*time_sync)(void * args, uint8_t core_id);

  } sl_log_api_core_t;

 /**
  * @brief  Backend interface API function pointers
  * 
  */
  typedef struct {

   sl_status_t (*backend_init)(void);
   /** @brief Deinitialize the backend interface */
   sl_status_t (*backend_deinit)(void);
   /** @brief Write log events to proprietary backend */
   sl_status_t (*backend_write)(sl_log_event_t *buffer, uint32_t read_index, uint32_t event_count);
  } sl_log_api_backend_t;

/**
 * @brief Backend status structure
 * 
 * Contains the status of the backend transfer.
 */
  typedef struct {
    uint8_t backend_transfer_done;
  }sl_log_backend_status_t;


/**
 * @brief Obtain the active logging platform core API instance.
 *
 * @return sl_log_api_core_t* Pointer to the logging core instance, or
 *         nullptr if unavailable or not initialized.
 *
 */
sl_log_api_core_t * sl_log_get_api_core(void);

/**
 * @brief Obtain the active logging backend interface API instance.
 *
 * @return sl_log_api_backend_t* Pointer to the logging backend instance, or
 *         nullptr on failure / not yet initialized.
 *
 */
sl_log_api_backend_t * sl_log_get_api_backend(void);



#ifdef __cplusplus
}
#endif

#endif // SL_LOG_PLATFORM_SPECIFIC_H
