/***************************************************************************/
/**
 * @file ulp_dma_example.c
 * @brief Ulp DMA Example
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licenser of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
/*==================================================================================
 * @brief : This file contains example application for memory to memory transfer
 of DMA
 * using sl_si91x DMA driver.
 * This example uses both simple transfer and generic transfer API for
 transferring.
 * User can transfer different sizes of data by changing DMA transfer size in
 bytes
 * in UC
 * This example transfers data from src0 to dst0 buffer
 ===================================================================================*/
#include "ulp_dma_example.h"
#include "rsi_debug.h"
#include "sl_common.h"
#include "sl_si91x_dma.h"
#include "sl_si91x_dma_config.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_wireless_shutdown.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define MS_DELAY_COUNTER        4600 // Delay count
#define FIVE_SECOND_DELAY       5000 // giving the 5 second delay in between state changes
#define ULP_DMA_SIMPLE_TRANSFER 1    //Enable/Disable simple transfer
#define ULP_DMA_INSTANCE        1    //ULP_DMA instance
#define ULP_DMA_CHANNEL         12   //ULP_DMA channel number 12
#define ULP_DMA_TRANSFER_SIZE   2048 //DMA transfer size

/*******************************************************************************
 **********************  Local variables   ***************************
 ******************************************************************************/
volatile uint32_t transfer_done = 0;          // Transfer done flag
uint32_t src0[ULP_DMA_TRANSFER_SIZE];         // source buffer
uint32_t dst0[ULP_DMA_TRANSFER_SIZE] = { 0 }; // destination buffer
sl_status_t status                   = SL_STATUS_OK;
static void delay(uint32_t idelay);
static void configuring_ps2_power_state(void);
/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
// Enum for different transmission scenarios
typedef enum {
  ULP_DMA_PROCESS_ACTION,         // dma event triggering and operational transforming
  ULP_POWER_STATE_TRANSITION,     // dma power state transitioning
  ULP_DMA_TRANSMISSION_COMPLETED, // dma transmission completed mode
} ulp_dma_enum_t;

static ulp_dma_enum_t ulp_dma_current_mode  = ULP_DMA_PROCESS_ACTION;
static sl_power_state_t current_power_state = SL_SI91X_POWER_MANAGER_PS4;
uint32_t channel                            = ULP_DMA_CHANNEL;
sl_dma_callback_t callbacks;
sl_dma_init_t dma_init = { ULP_DMA_INSTANCE };
/*******************************************************************************
 * Transfer callback function.
 ******************************************************************************/
void transfer_complete_callback_dmadrv(uint32_t channel, void *data);
/*******************************************************************************
 **********************  Local Function definitions ***************************
 ******************************************************************************/

/*******************************************************************************
 * Transfer callback function.
 ******************************************************************************/
void transfer_complete_callback_dmadrv(uint32_t channel, void *data)
{

  (void)channel;
  (void)data;
  transfer_done = 1;
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 * DMA example initialization function
 * Configures DMA for basic memory to memory transfer
 ******************************************************************************/
void dma_example_init(void)
{
  do {
    // Initialize UDMA peripheral
    status = sl_si91x_dma_init(&dma_init);
    if (status) {
      // UDMA initialization fail
      DEBUGOUT("\r\nFailed to Initialize UDMA\r\n");
      break;
    } else {
      // UDMA initialization success
      DEBUGOUT("\r\nUDMA Initialization Success\r\n");
    }
    // Allocate channel for transfer
    status = sl_si91x_dma_allocate_channel(ULP_DMA_INSTANCE, &channel, 0);
    if (status) {
      // Channel allocation failed
      DEBUGOUT("\r\nChannel not allocated \r\n");
      break;
    } else {
      // Channel successfully allocated
      DEBUGOUT("\r\nChannel Allocated successfully\r\n");
    }
    callbacks.transfer_complete_cb = transfer_complete_callback_dmadrv;

    // Register transfer complete callback
    status = sl_si91x_dma_register_callbacks(ULP_DMA_INSTANCE, channel, &callbacks);
    if (status) {
      // Callback registration success
      DEBUGOUT("\r\nCallbacks not registered\r\n");
      break;
    } else {
      // Callback not registered
      DEBUGOUT("\r\nCallbacks registered\r\n");
    }

    // Filled data in source buffer
    for (int i = 0; i < ULP_DMA_TRANSFER_SIZE; i++) {
      src0[i] = (uint32_t)(i + 1);
    }
#if defined(ULP_DMA_SIMPLE_TRANSFER) \
  && (ULP_DMA_SIMPLE_TRANSFER == 1) // Perform DMA transfer using simple dma transfer API
    status = sl_si91x_dma_simple_transfer(ULP_DMA_INSTANCE, channel, src0, dst0, ULP_DMA_TRANSFER_SIZE);
#else
    sl_dma_xfer_t dma_transfer_t = { 0 };
    // Update source and destination address
    dma_transfer_t.src_addr  = src0;
    dma_transfer_t.dest_addr = dst0;
    // Update Source and destination data increment
    dma_transfer_t.src_inc = SL_TRANSFER_SRC_INC_32;
    dma_transfer_t.dst_inc = SL_TRANSFER_DST_INC_32;
    // Update the single data transfer size
    dma_transfer_t.xfer_size = SL_TRANSFER_SIZE_32;
    // Update total DMA transfer size
    dma_transfer_t.transfer_count = ULP_DMA_TRANSFER_SIZE;
    // Update DMA transfer type and mode
    dma_transfer_t.dma_mode      = SL_DMA_BASIC_MODE;
    dma_transfer_t.transfer_type = SL_DMA_MEMORY_TO_MEMORY;
    // Perform DMA transfer using generic dma transfer API
    status = sl_si91x_dma_transfer(ULP_DMA_INSTANCE, channel, &dma_transfer_t);
#endif
    if (status) {
      // Transfer start failed
      DEBUGOUT("\r\nTransfer start fail\r\n");
    } else {
      // Transfer started successfully
      DEBUGOUT("\r\n Xfer start\r\n");
    }
  } while (0);
}

/*******************************************************************************
 * Function will run continuously and will wait for trigger
 ******************************************************************************/
void dma_example_process_action(void)
{
  /*************************************************************************************************
   * This section manages power state transitions within the system, optimizing
   *power consumption while maintaining essential functionality. It transitions
   *the system from a higher power state (PS4) to a lower one (PS2) during
   *specific operations to conserve power. This involves adjusting clock
   * references and shutting down unnecessary power supplies. After completing
   *the operation, the code transitions back to the higher power state (PS4) to
   *ensure adequate resources for subsequent tasks. This approach balances power
   *efficiency with operational requirements across various system functions.
   ***************************************************************************************************/
  switch (ulp_dma_current_mode) {
    case ULP_DMA_PROCESS_ACTION:
      if (transfer_done) {
        transfer_done = 0;
        // DMA transfer done
        DEBUGOUT("\r\nTransfer completed successfully\r\n");
        // de allocating the dma channel
        sl_si91x_dma_deallocate_channel(ULP_DMA_INSTANCE, channel);
        // current mode being updated with power state transition to change the
        // power state mode
        ulp_dma_current_mode = ULP_POWER_STATE_TRANSITION;
      }
      break;
    case ULP_POWER_STATE_TRANSITION:
      if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
        DEBUGOUT("Switching dma from PS4 -> PS2 state \n");
        // Control power management by adjusting clock references and shutting down
        // the power supply
        // This function is for demonstration purpose only. For more details, refer to the README file.
        sl_si91x_wireless_shutdown();
        // switching the power state PS4 to PS2 mode.
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          break;
        }
        /* Calling the trim_eFuse API within the power manager modifies the clock frequency. 
        If debug is not reinitialized afterward, it may result in corrupted or 
        missing output data in the console.*/
        DEBUGINIT();
        // Configuring the ps2 power state by configuring
        // the ram retention and removing the unused peripherals
        configuring_ps2_power_state();
        // Adding 5 seconds delay for verifying the current consumption
        delay(FIVE_SECOND_DELAY);
        // intializing the dma
        dma_example_init();
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_dma_current_mode = ULP_DMA_PROCESS_ACTION;
        // current power state is updated to PS2
        current_power_state = SL_SI91X_POWER_MANAGER_PS2;
        break;
      } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
        DEBUGOUT("Switching the dma from PS2 -> PS4 state\n");
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          break;
        }
        /* Due to calling trim_efuse API om power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // Adding 5 seconds delay for verifying the current consumption
        delay(FIVE_SECOND_DELAY);
        // intializing the dma
        dma_example_init();
        // current power state is updated to last enum after the power state cycle
        // is completed
        current_power_state = LAST_ENUM_POWER_STATE;
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_dma_current_mode = ULP_DMA_PROCESS_ACTION;
        break;
      } else {
        //  de initializing the dma
        if (sl_si91x_dma_deinit(ULP_DMA_INSTANCE)) {
          DEBUGOUT("\r\nFailed to Uninitialize UDMA\r\n");
        } else {
          DEBUGOUT("\r\nUDMA de-initialization Success\r\n");
        }
        ulp_dma_current_mode = ULP_DMA_TRANSMISSION_COMPLETED;
      }
      break;
    case ULP_DMA_TRANSMISSION_COMPLETED:
      break;
    default:
      break;
  }
}
/*******************************************************************************
 * Delay function for 1ms
 ******************************************************************************/
static void delay(uint32_t idelay)
{
  for (uint32_t x = 0; x < MS_DELAY_COUNTER * idelay; x++) {
    __NOP();
  }
}
/*******************************************************************************
 * powering off the peripherals not in use,
 * Configuring power manager ram-retention
 ******************************************************************************/
static void configuring_ps2_power_state(void)
{
  sl_power_ram_retention_config_t config;
  sl_power_peripheral_t peri;
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3;
  // Clear the peripheral configuration
  peri.m4ss_peripheral = 0;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX;
  // Ored value for npss peripheral.
  peri.npss_peripheral = SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3;
  do {
    // Peripherals passed in this API are powered off.
    status = sl_si91x_power_manager_remove_peripheral_requirement(&peri);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("sl_si91x_power_manager_remove_peripheral_requirement failed, "
               "Error Code: 0x%lX",
               status);
      break;
    }
    // RAM retention modes are configured and passed into this API.
    status = sl_si91x_power_manager_configure_ram_retention(&config);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("sl_si91x_power_manager_configure_ram_retention failed, Error "
               "Code: 0x%lX",
               status);
      break;
    }
  } while (false);
}
