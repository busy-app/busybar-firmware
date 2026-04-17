/***************************************************************************//**
 * @file
 * @brief RTT interface for DDP
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "SEGGER_RTT_Conf.h"
#include "sl_iostream_rtt.h"
#include "sl_ddp_common.h"
#include "sl_ddp_rtt.h"

// -----------------------------------------------------------------------------
// Definitions

#define SL_DDP_RTT_INPUT_BUF_SIZE   (BUFFER_SIZE_DOWN)
#define SL_DDP_RTT_OUTPUT_BUF_SIZE  (BUFFER_SIZE_UP)

// -----------------------------------------------------------------------------
// Private variables

// DPP RTT output buffer
static uint8_t rtt_output[SL_DDP_RTT_OUTPUT_BUF_SIZE];
// DPP RTT input buffer
static uint8_t rtt_input[SL_DDP_RTT_INPUT_BUF_SIZE];
// DPP RTT input buffer index
static uint16_t rtt_input_idx;

// -----------------------------------------------------------------------------
// Public functions

/******************************************************************************
 * Initialize DPP RTT interface.
 *****************************************************************************/
void sl_ddp_rtt_init(void)
{
  rtt_input_idx = 0u;
}

/******************************************************************************
 * Process DPP RTT interface.
 *****************************************************************************/
void sl_ddp_rtt_process_action(void)
{
  size_t rtt_input_len = 0;
  sl_status_t status;
  sl_ddp_rtt_req_t *req;
  sl_ddp_rtt_rsp_t *rsp;
  uint16_t output_len = 0;

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
                                                      sizeof(rtt_output) - sizeof(sl_ddp_rtt_rsp_t),
                                                      &output_len);
        // Update the RTT response length
        rsp->data_len = output_len;
        // Write the RTT response header
        sl_iostream_write(sl_iostream_rtt_handle,
                          rsp,
                          sizeof(*rsp));
        // Write the RTT response body if any
        if ((rsp->status == 0) && output_len) {
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
