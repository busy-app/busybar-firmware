/***************************************************************************//**
 * # License
 * <b> Copyright 2024 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef ZW_CLI_SLEEPING_CONFIG_H_
#define ZW_CLI_SLEEPING_CONFIG_H_

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Wakeup Configuration with CLI

// <o ZW_CLI_SLEEPING_WAKEUP_TIME_AFTER_RESET> Wakeup time after reset <f.d>
// <i> The device will stay awake for this amount of time after reset. (Units: Seconds)
// <d> 10
#define ZW_CLI_SLEEPING_WAKEUP_TIME_AFTER_RESET 10

// <e ZW_CLI_SLEEPING_WAKEUP_EM1> Add EM1 lock during wakeup time<f.d>
// <i> If enabled, the component will add an EM1 lock during wakeup time and when sleeping is disabled through CLI command.
// <d> 0
#define ZW_CLI_SLEEPING_WAKEUP_EM1 0

// </h>

// <<< end of configuration section >>>

#endif /* ZW_CLI_SLEEPING_CONFIG_H_ */
