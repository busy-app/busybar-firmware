/***************************************************************************/ /**
 * @file
 * @brief CPC system endpoint secondary part
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_CPC_SYSTEM_SECONDARY_H
#define SLI_CPC_SYSTEM_SECONDARY_H

#include "sl_common.h"
#include "sl_compiler.h"
#include "sli_cpc_system_common.h"
#include "sli_cpc_timer.h"

/***************************************************************************//**
 * Each command buffer must be big enough to hold the metadata around the command
 * and the command itself (command_id, seq, length, and payload).  Considering
 * PAYLOAD_MAX_LENGTH is already accounted for in sizeof(system_cmd_context),
 * it must be substracted to avoid allocating extra space.
 ******************************************************************************/
#define SLI_CPC_SYSTEM_COMMAND_BUFFER_SIZE \
  (sizeof(sli_cpc_system_cmd_context_t)    \
   - PAYLOAD_LENGTH_MAX                    \
   + SLI_CPC_SYSTEM_COMMAND_PAYLOAD_LENGTH_MAX)

typedef struct sli_cpc_system_endpoint {
  sl_cpc_endpoint_handle_t handle;

  sli_cpc_system_primary_version_t primary_version;

  bool stop_flag;
  bool reset_sequence_done;

  uint32_t enter_irq_end_ms;
  sli_cpc_timer_handle_t enter_irq_timer;

  bool process_uframes_flag;

  uint32_t fc_validation_value;
} sli_cpc_system_endpoint_t;

/***************************************************************************//**
 * System endpoint command context. It contains both the command to send and
 * some context around it that can be used when the command completes.
 ******************************************************************************/
typedef struct {
  sli_cpc_system_endpoint_t         *ep;
  void                              *on_complete_arg;
  uint8_t                           ep_id;

  // due to its definition, sli_cpc_system_cmd_t alignment is two bytes,
  // but some casts in the system endpoint expects it to be word-aligned.
  __ALIGNED(4) sli_cpc_system_cmd_t command;
} sli_cpc_system_cmd_context_t;

SL_NORETURN void cpc_system_reset(sli_cpc_system_reboot_mode_t reboot_mode);

sl_cpc_system_status_t cpc_get_reset_reason(void);

void cpc_get_bootloader_info(sli_cpc_system_bootloader_info_t *infos);

/***************************************************************************//**
 * Invoked when a frame with the Poll/Final flag is set.
 *
 * @return Status code.
 ******************************************************************************/
sl_status_t sli_cpc_system_secondary_on_poll(sli_cpc_system_endpoint_t *system_ep,
                                             sli_cpc_system_ep_frame_type_t frame_type,
                                             void *poll_data,
                                             uint32_t poll_data_length, // Rx buffer is freed once this on_poll function return
                                             void **reply_data,
                                             uint32_t *reply_data_length,
                                             sli_cpc_system_cmd_context_t **context);

#endif
