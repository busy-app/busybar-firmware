/***************************************************************************//**
 * @file sl_ddp_rtt.h
 * @brief RTT interface for DDP
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

#ifndef SL_DDP_RTT_H
#define SL_DDP_RTT_H

#include <stdint.h>
#include "sl_common.h"
#include "sl_ddp_types.h"

/**************************************************************************//**
 * @addtogroup SL_DDP_RTT_API DDP RTT API
 *
 * DDP RTT API provides a host interface for interacting with the dynamic
 * provisioning application using Serial Wire Debug (SWD) protocol.
 *
 * I/O handling utilises SEGGER's Real Time Transfer (RTT) technology,
 * providing a control block for state information and a ring buffer for
 * input and output each.
 *
 * The host writes a sl_ddp_rtt_req_t request using the interface, receiving
 * a sl_ddp_rtt_rsp_t response once the request has been handled. Multiple
 * requests can be written, up to the maximum size of the input buffer.
 *
 * @{
 *****************************************************************************/

/// DDP RTT command request
SL_PACK_START(1)
typedef struct {
  /// DDP Command ID
  uint16_t id;
  /// Length of command specific input structure in bytes
  uint16_t data_len;
  /// Command specific input structure
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_ddp_rtt_req_t;
SL_PACK_END()

/// DDP RTT command response
SL_PACK_START(1)
typedef struct {
  /// Status of the corresponding command request
  uint32_t status;
  /// Length of command specific output structure in bytes
  uint16_t data_len;
  /// Command specific output structure
  uint8_t data[];
} SL_ATTRIBUTE_PACKED sl_ddp_rtt_rsp_t;
SL_PACK_END()

/***************************************************************************//**
 * Initialize DPP RTT interface.
 *
 * This function is used to initialize the DPP RTT interface and it must be
 * called before any other function in this interface. Typically this happens
 * automatically with System Init.
 ******************************************************************************/
void sl_ddp_rtt_init(void);

/***************************************************************************//**
 * Process DPP RTT interface.
 *
 * This function is used to process any incoming and outgoing DPP RTT data.
 * In order to continue processing data, it must be called periodically. This
 * is typically called by System Init.
 ******************************************************************************/
void sl_ddp_rtt_process_action(void);

/** @} (end SL_DDP_RTT_API) */

#endif  // SL_DDP_RTT_H
