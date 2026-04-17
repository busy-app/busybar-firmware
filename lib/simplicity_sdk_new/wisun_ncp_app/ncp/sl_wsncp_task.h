/***************************************************************************//**
 * @file sl_wsncp_task.h
 * @brief Wi-SUN NCP task function declarations
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WSNCP_TASK_H
#define SL_WSNCP_TASK_H

/***************************************************************************//**
 * Initialize the Wi-SUN NCP task and related resources.
 *
 * Sets up the NCP interface, creates RTOS objects, and starts the NCP task
 * responsible for processing requests and indications between the host and
 * the Wi-SUN stack. Call once during system initialization.
 ******************************************************************************/
void sl_wsncp_task_init(void);

#endif // SL_WSNCP_TASK_H
