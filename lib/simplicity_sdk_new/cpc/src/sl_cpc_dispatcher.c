/***************************************************************************/ /**
 * @file
 * @brief CPC API implementation.
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

#include "sl_atomic.h"
#include "sli_cpc.h"
#include "sli_cpc_assert.h"
#include "sli_cpc_instance.h"

/***************************************************************************//**
 * Initialize the dispatcher handle.
 ******************************************************************************/
void sli_cpc_dispatcher_init_handle(sl_cpc_dispatcher_handle_t *handle,
                                    sli_cpc_instance_t *inst)
{
  SLI_CPC_ASSERT(handle != NULL);

  handle->submitted = false;
  handle->fnct = NULL;
  handle->data = NULL;
  handle->instance = inst;
}

/***************************************************************************//**
 * Push function in dispatch queue along with the data to be passed when
 * dispatched.
 ******************************************************************************/
sl_status_t sli_cpc_dispatcher_push(sl_cpc_dispatcher_handle_t *handle,
                                    sl_cpc_dispatcher_fnct_t fnct,
                                    void *data)
{
  MCU_DECLARE_IRQ_STATE;
  MCU_ENTER_ATOMIC();

  if (handle->submitted) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_BUSY; // Already dispatched
  }

  handle->fnct = fnct;
  handle->data = data;
  sl_slist_push_back(&handle->instance->process_queue, &handle->node);

  SLI_CPC_ASSERT(handle->instance->event_counter < 255);
  ++handle->instance->event_counter;

  handle->submitted = true;

  MCU_EXIT_ATOMIC();

  sli_cpc_signal_event(handle->instance, SL_CPC_SIGNAL_SYSTEM);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Remove function from dispatch queue.
 ******************************************************************************/
void sli_cpc_dispatcher_cancel(sl_cpc_dispatcher_handle_t *handle)
{
  MCU_DECLARE_IRQ_STATE;

  MCU_ENTER_ATOMIC();
  if (handle->submitted) {
    sl_slist_remove(&handle->instance->process_queue, &handle->node);

    SLI_CPC_ASSERT(handle->instance->event_counter > 0);
    --handle->instance->event_counter;

    handle->submitted = false;
  }
  MCU_EXIT_ATOMIC();
}

/***************************************************************************//**
 * Process the dispatch queue.
 ******************************************************************************/
void sli_cpc_dispatcher_process(sli_cpc_instance_t *inst)
{
  sl_slist_node_t *node;
  sl_cpc_dispatcher_handle_t *handle;
  uint8_t event_count_processed;
  uint8_t event_count_to_process;

  MCU_DECLARE_IRQ_STATE;

  MCU_ATOMIC_LOAD(event_count_processed, inst->event_counter);
  event_count_to_process = event_count_processed;

  do {
    MCU_ATOMIC_SECTION(node = sl_slist_pop(&inst->process_queue); );
    if (node != NULL) {
      handle = SL_SLIST_ENTRY(node, sl_cpc_dispatcher_handle_t, node);
      SLI_CPC_ASSERT(handle->instance == inst);

      handle->submitted = false;
      handle->fnct(handle->data);
    }
  } while (node != NULL && --event_count_to_process);

  // Update the global event_counter
  MCU_ENTER_ATOMIC();
  if (inst->event_counter >= event_count_processed) {
    inst->event_counter -= event_count_processed;
  } else {
    // If we are here, an event was cancelled when processing the queue
    inst->event_counter = 0;
  }
  MCU_EXIT_ATOMIC();
}
