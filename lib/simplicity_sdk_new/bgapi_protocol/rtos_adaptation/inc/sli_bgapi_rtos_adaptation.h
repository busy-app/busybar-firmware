/***************************************************************************//**
 * @file
 * @brief Adaptation for running BGAPI devices in RTOS
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SLI_BGAPI_RTOS_ADAPTATION_H
#define SLI_BGAPI_RTOS_ADAPTATION_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond DOXYGEN_INCLUDE_INTERNAL */

/***************************************************************************//**
 * @addtogroup sli_bgapi_rtos_adaptation BGAPI RTOS adaptation
 * @{
 *
 * @brief BGAPI RTOS adaptation
 *
 * The BGAPI RTOS adaptation provides the functionality needed to run BGAPI
 * device implementations in an RTOS environment. This set of functionality is
 * generic to the BGAPI Protocol and is not specific to any particular wireless
 * stack that offers a BGAPI-based API.
 */

// -----------------------------------------------------------------------------
// Silicon Labs internal functions for BGAPI RTOS adaptation

/**
 * @brief Lock the BGAPI for exclusive access.
 *
 * This function is used internally by the BGAPI Protocol to lock the BGAPI and
 * obtain exclusive access when starting to handle a BGAPI command. If the
 * calling RTOS task is the task that will process the BGAPI command, the
 * function returns SL_STATUS_IS_OWNER and no lock is taken. For other tasks the
 * lock is taken and success is indicated by SL_STATUS_OK. The caller considers
 * both SL_STATUS_IS_OWNER and SL_STATUS_OK as successful and will call @ref
 * sli_bgapi_unlock to release the lock.
 *
 * @param[in] command_hdr The header of the BGAPI command to lock for
 *
 * @return SL_STATUS_OK if the lock has been obtained, otherwise an error code
 */
sl_status_t sli_bgapi_lock(uint32_t command_hdr);

/**
 * @brief Release the lock obtained by @ref sli_bgapi_lock
 *
 * @param[in] command_hdr The header of the BGAPI command that we locked for
 */
void sli_bgapi_unlock(uint32_t command_hdr);

/** @} end sli_bgapi_rtos_adaptation */

/** @endcond */ // DOXYGEN_INCLUDE_INTERNAL

#ifdef __cplusplus
}
#endif

#endif //SLI_BGAPI_RTOS_ADAPTATION_H
