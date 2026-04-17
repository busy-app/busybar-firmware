/*******************************************************************************
* @file  sl_si91x_stack_size_config.h
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef __SL_SI91X_STACK_SIZE_CONFIG_H_
#define __SL_SI91X_STACK_SIZE_CONFIG_H_

// <<< Use Configuration Wizard in Context Menu >>>
// <h> Memory configuration

// <o SL_STACK_SIZE> Stack size for the application.
// <i> Default: 12288
// <i> The stack size configured here will be used by the stack that the
// <i> application uses when coming out of a reset.
#ifndef SL_STACK_SIZE
#define SL_STACK_SIZE 12288
#endif

// </h>
// <<< end of configuration section >>>

#endif // __SL_SI91X_STACK_SIZE_CONFIG_H_