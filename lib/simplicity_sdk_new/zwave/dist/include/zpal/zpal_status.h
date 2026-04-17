/**
 * @file
 * Defines status codes returned by zpal functions.
 *
 * @copyright 2021 Silicon Laboratories Inc.
 */

#ifndef ZPAL_STATUS_H_
#define ZPAL_STATUS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup zpal
 * @brief
 * Z-Wave Platform Abstraction Layer.
 * @{
 * @addtogroup zpal-status
 * @brief
 * Defines status codes returned by zpal functions.
 *
 * @{
 */

typedef enum {
  ZPAL_STATUS_OK = 0,               ///< No error.
  ZPAL_STATUS_FAIL,                 ///< Generic error.
  ZPAL_STATUS_BUFFER_FULL,          ///< There is no more room in the buffer.
  ZPAL_STATUS_BUSY,                 ///< Process is busy.
  ZPAL_STATUS_INVALID_ARGUMENT      ///< At least one argument is invalid.
} zpal_status_t;

/**
 * @} //zpal-status
 * @} //zpal
 */

#ifdef __cplusplus
}
#endif

#endif /* ZPAL_STATUS_H_ */
