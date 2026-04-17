/***************************************************************************//**
 * @brief Initialization APIs for Bluetooth stack
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BT_STACK_INIT_H
#define SL_BT_STACK_INIT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Bluetooth stack.
 *
 * This single-stage init is used when the application uses `sl_system` for
 * initialization.
 *
 * @return SL_STATUS_OK if the initialization was successful; Other error code
 *   indicates a failure.
 */
sl_status_t sl_bt_stack_init();

/**
 * @brief Make permanent memory allocations for the Bluetooth stack.
 *
 * This function is part of a two-stage initialization used when the application
 * uses `sl_main` for initialization. Do not call this function directly from
 * the application. The Bluetooth host stack components contribute the necessary
 * calls to the right steps of `sl_main` init.
 */
void sli_bt_stack_permanent_allocation(void);

/**
 * @brief Perform functional initialization of the Bluetooth stack.
 *
 * This function is part of a two-stage initialization used when the application
 * uses `sl_main` for initialization. Do not call this function directly from
 * the application. The Bluetooth host stack components contribute the necessary
 * calls to the right steps of `sl_main` init.
 */
void sli_bt_stack_functional_init(void);

#ifdef __cplusplus
}
#endif

#endif
