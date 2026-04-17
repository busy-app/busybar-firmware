/***************************************************************************/ /**
 * @file app.c
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app.h"
#include "wdt_manager.h"

/***************************************************************************/ /**
 * Initialize application.
 * 
 * This function initializes the WDT Manager example.
 * The WDT manager is automatically initialized and started by the
 * WDT Manager component to ensure system reliability.
 ******************************************************************************/
void app_init(void)
{
  wdt_manager_init();
}

/***************************************************************************/ /**
 * App ticking function.
 * 
 * This function processes the main application logic and should be called
 * continuously in the main loop. It handles LED toggling based on timer
 * events while the WDT manager runs in the background to ensure system
 * reliability.
 ******************************************************************************/
void app_process_action(void)
{
  wdt_manager_process_action();
}