/*******************************************************************************
* @file  sl_net_ethernet.c
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

#include "sl_net.h"
#include "sl_ethernet.h"
//#include "sl_thread.h"
#include "cmsis_os2.h"
#include "sl_net_ethernet_types.h"
#include "sl_constants.h"

#define ETHERNET_DRIVER_THREAD_STACK_SIZE 1024 // No idea how big this needs to be
#define DEFAULT_ETHERNET_TX_BUFFER_COUNT  30
#define DEFAULT_ETHERNET_RX_BUFFER_COUNT  10

static void sli_si91x_ethernet_driver_thread_main(uint32_t arg);

sl_status_t sli_net_ethernet_init(const sl_ethernet_config_t *configuration,
                                  sl_ethernet_context_t *workspace,
                                  void *event_handler);
sl_status_t sli_net_ethernet_deinit(sl_ethernet_context_t *workspace);
sl_status_t sli_net_ethernet_up(sl_net_profile_id_t profile_id);
sl_status_t sli_net_ethernet_down(void);

sl_net_ethernet_api_t ethernet_interface = {
  .init   = sli_net_ethernet_init,
  .deinit = sli_net_ethernet_deinit,
  .up     = sli_net_ethernet_up,
  .down   = sli_net_ethernet_down,
};

static sl_status_t sli_si91x_sli_ethernet_init_context(sl_ethernet_context_t *workspace,
                                                       uint32_t tx_buffer_count,
                                                       uint32_t rx_buffer_count)
{
  sl_status_t status;

  // Allocate tx_buffers, rx_buffers, and the thread stack
  status = SL_STATUS_NO_MORE_RESOURCE; // In case any of the following fails, this is the error we want to return
  workspace->tx_buffer_config =
    malloc(sizeof(sl_ethernet_tx_buffer_configuration_t) + tx_buffer_count * sizeof(sl_ethernet_tx_descriptor_t));
  SL_VERIFY_POINTER_OR_EXIT(workspace->tx_buffer_config);
  memset(workspace->tx_buffer_config,
         0,
         sizeof(sl_ethernet_tx_buffer_configuration_t) + tx_buffer_count * sizeof(sl_ethernet_tx_descriptor_t));
  workspace->rx_buffer_config =
    malloc(sizeof(sl_ethernet_rx_buffer_configuration_t) + rx_buffer_count * sizeof(sl_ethernet_rx_descriptor_t));
  SL_VERIFY_POINTER_OR_EXIT(workspace->rx_buffer_config);
  memset(workspace->rx_buffer_config,
         0,
         sizeof(sl_ethernet_rx_buffer_configuration_t) + rx_buffer_count * sizeof(sl_ethernet_rx_descriptor_t));
  workspace->thread_stack = malloc(ETHERNET_DRIVER_THREAD_STACK_SIZE);
  SL_VERIFY_POINTER_OR_EXIT(workspace->thread_stack);
  memset(workspace->thread_stack, 0, ETHERNET_DRIVER_THREAD_STACK_SIZE);
  // Set the buffer counts
  workspace->tx_buffer_config->buffer_count = tx_buffer_count;
  workspace->rx_buffer_config->buffer_count = rx_buffer_count;

  const osEventFlagsAttr_t event_attributes = { .name = "", .attr_bits = 0, .cb_mem = 0, .cb_size = 0 };
  workspace->events                         = osEventFlagsNew(&event_attributes);

exit:
  SL_CLEANUP_MALLOC(workspace->tx_buffer_config);
  SL_CLEANUP_MALLOC(workspace->rx_buffer_config);
  SL_CLEANUP_MALLOC(workspace->thread_stack);
}

sl_status_t sli_net_ethernet_init(const sl_ethernet_config_t *configuration,
                                  sl_ethernet_context_t *workspace,
                                  void *event_handler)
{
  sl_status_t status;

  status = sli_ethernet_init_context(workspace, DEFAULT_ETHERNET_TX_BUFFER_COUNT, DEFAULT_ETHERNET_RX_BUFFER_COUNT);
  SL_VERIFY_SUCCESS_OR_RETURN(status);

  status = sl_ethernet_init(workspace, configuration, configuration->pin_configuration);
  SL_VERIFY_SUCCESS_OR_EXIT(status);

  status = sl_ethernet_set_buffer_configuration(workspace, workspace->rx_buffer_config, workspace->tx_buffer_config);
  SL_VERIFY_SUCCESS_OR_EXIT(status);

  const osThreadAttr_t thread_attributes = {
    .name       = "ethernet_driver_thread",
    .attr_bits  = 0,
    .cb_mem     = 0,
    .cb_size    = 0,
    .stack_mem  = workspace->thread_stack,
    .stack_size = ETHERNET_DRIVER_THREAD_STACK_SIZE,
    .priority   = 0,
    .tz_module  = 0,
    .reserved   = 0,
  };
  osThreadNew((osThreadFunc_t)ethernet_driver_thread_main, workspace, &thread_attributes);
  //  sl_thread_init( &ethernet_driver_thread, "ethernet_driver_thread", ethernet_driver_thread_main, workspace, workspace->thread_stack, ETHERNET_DRIVER_THREAD_STACK_SIZE );

  status = sl_ethernet_start_link(workspace, 5000);
  SL_VERIFY_SUCCESS_OR_EXIT(status);

exit:
  sli_net_ethernet_deinit(workspace);
}

sl_status_t sli_net_ethernet_deinit(sl_ethernet_context_t *workspace)
{
  sl_ethernet_deinit();
  //  SL_CLEANUP_MALLOC(tx_buffers);
  //  SL_CLEANUP_MALLOC(rx_buffers);
  //  SL_CLEANUP_MALLOC(stack);
}

sl_status_t sli_net_ethernet_up(sl_net_profile_id_t profile_id)
{
}

sl_status_t sli_net_ethernet_down(void)
{
}

static void sli_si91x_ethernet_driver_thread_main(uint32_t arg)
{
  sl_ethernet_context_t *workspace = (sl_ethernet_context_t *)arg;

  while (1) {
    uint32_t set_events =
      osEventFlagsWait(workspace->events, SL_ETHERNET_TX_COMPLETE_EVENT | SL_ETHERNET_RX_EVENT, 0, SL_WAIT_FOREVER);
    //        sl_event_wait_for_events( &workspace->events, SL_ETHERNET_TX_COMPLETE_EVENT | SL_ETHERNET_RX_EVENT, SL_WAIT_FOREVER, &set_events );
    if (set_events & SL_ETHERNET_RX_EVENT) {
      //            // Identify the packet
      //            int buffer_index = 0;
      //            for ( ; buffer_index < workspace->rx_buffer_config->buffer_count; ++buffer_index )
      //            {
      //                sl_ethernet_rx_descriptor_t* descriptor = &workspace->rx_buffer_config->buffer[buffer_index];
      //                if (descriptor->word1.used != 0 && descriptor->address == (uint32_t)workspace->rx_buffers_config->data)
      //                {
      //                    break;
      //                }
      //            }
      //
      //            // Pass packet to the network stack
      //            sl_network_process_rx_buffer(SL_ETHERNET_NETWORK_INTERFACE, workspace->rx_buffers_config);
      //
      //            workspace->rx_buffers_config = workspace->rx_buffers_config->next;
    }

    if (set_events & SL_ETHERNET_TX_COMPLETE_EVENT) {
      //            // Clean up descriptor
      //            for ( int i = 0; i < workspace->tx_buffer_config->buffer_count && workspace->tx_buffers_start != NULL; ++i )
      //            {
      //                sl_buffer_t* buffer = workspace->tx_buffers_start;
      //                sl_ethernet_tx_descriptor_t* descriptor = &workspace->tx_buffer_config->buffer[i];
      //
      //                if ( descriptor->word1.used != 0 && descriptor->address == (uint32_t)buffer->data )
      //                {
      //                    descriptor->address = 0;
      //                    descriptor->word1.value = SL_ETHERNET_TXD_USED;
      //                    workspace->tx_buffers_start = sl_memory_free_buffer( buffer, SL_MEMORY_FREE_SINGLE_BUFFER );
      //                }
      //            }
    }
  }
}
