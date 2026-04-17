/*******************************************************************************
* @file  app.h
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
#ifndef APP_H
#define APP_H
#include <stdint.h>
/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/
void app_init(void);

/***************************************************************************/ /**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void);
/******************************************************
 * *               Function Declarations
 * ******************************************************/
void application_start();
void sl_decode();

//! Length of token to be received
#define LENGTH_OF_TOKEN 500

//! Number of random needed to generate in Bytes
#define LENGTH_OF_RANDOM_DATA 32
#endif // APP_H
