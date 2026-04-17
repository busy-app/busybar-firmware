/***************************************************************************//**
 * @file sl_ddp_rtt.c
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

#include <stdlib.h>
#include <string.h>
#include "sl_iostream_rtt.h"
#include "sl_ddp_common.h"
#include "sl_ddp_rtt_config.h"
#include "sl_ddp_rtt.h"

// DPP RTT output buffer
static uint8_t rtt_output[SL_DDP_RTT_OUTPUT_BUF_SIZE];
// DPP RTT input buffer
static uint8_t rtt_input[SL_DDP_RTT_INPUT_BUF_SIZE];
// DPP RTT input buffer index
static uint16_t rtt_input_idx;

/**************************************************************************//**
 * Initialize DPP RTT interface.
 *****************************************************************************/
void sl_ddp_rtt_init(void)
{
  // No initialization actions required
}

/**************************************************************************//**
 * Process DPP RTT interface.
 *****************************************************************************/
void sl_ddp_rtt_process_action(void)
{
  size_t rtt_input_len = 0;
  sl_status_t status;
  sl_ddp_rtt_req_t *req;
  sl_ddp_rtt_rsp_t *rsp;
  uint16_t output_len;

  status = sl_iostream_read(sl_iostream_rtt_handle,
                            &rtt_input[rtt_input_idx],
                            SL_DDP_RTT_INPUT_BUF_SIZE - rtt_input_idx,
                            &rtt_input_len);
  if (status == SL_STATUS_OK) {
    // Process the received input data, if any
    rtt_input_idx += rtt_input_len;
    if (rtt_input_idx >= sizeof(sl_ddp_rtt_req_t)) {
      // There is enough data for the RTT request header
      req = (sl_ddp_rtt_req_t *)rtt_input;
      rtt_input_len = sizeof(*req) + req->data_len;
      if (rtt_input_idx >= rtt_input_len) {
        // There is enough data for the RTT request body
        rsp = (sl_ddp_rtt_rsp_t *)rtt_output;
        // Handle the RTT request
        rsp->status = (uint32_t)sl_ddp_handle_command(req->id,
                                                      req->data,
                                                      req->data_len,
                                                      rsp->data,
                                                      sizeof(rtt_output),
                                                      &output_len);
        // Update the RTT response length
        rsp->data_len = output_len;
        // Write the RTT response header
        sl_iostream_write(sl_iostream_rtt_handle,
                          rsp,
                          sizeof(*rsp));
        // Write the RTT response body if any
        if (output_len) {
          sl_iostream_write(sl_iostream_rtt_handle,
                            rsp->data,
                            output_len);
        }
        // Move the remaining input data to the start of the buffer if any
        memmove(rtt_input, &rtt_input[rtt_input_len], rtt_input_idx - rtt_input_len);
        // Amount of remaining input data
        rtt_input_idx -= rtt_input_len;
      }
    }
  }
}
