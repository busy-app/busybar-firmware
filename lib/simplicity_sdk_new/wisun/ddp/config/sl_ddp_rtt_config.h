/***************************************************************************//**
 * @file sl_ddp_rtt_config.h
 * @brief RTT interface configuration for DDP
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_DDP_RTT_CONFIG_H
#define SL_DDP_RTT_CONFIG_H

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/

// <h>DDP RTT Interface Configuration

// <o SL_DDP_RTT_INPUT_BUF_SIZE> Size of RTT input buffer.
// <i> Default: 256
// <i> Define the maximum size of RTT input (host->EFR3) buffer.
#define SL_DDP_RTT_INPUT_BUF_SIZE 256

// <o SL_DDP_RTT_OUTPUT_BUF_SIZE> Size of RTT output buffer.
// <i> Default: 256
// <i> Define the maximum size of RTT output (EFR3->host) buffer.
#define SL_DDP_RTT_OUTPUT_BUF_SIZE 256

#endif // SL_DDP_RTT_CONFIG_H

// </h>
// <<< end of configuration section >>>
