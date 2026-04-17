/*******************************************************************************
* @file  timer.c
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

#include "timer_platform.h"
#include "cmsis_os2.h"

bool has_timer_expired(const Timer *timer)
{
  long int elapsed_time = (timer->end_time - osKernelGetTickCount());
  return (elapsed_time < 0);
}

void countdown_ms(Timer *timer, uint32_t timeout_ms)
{
  timer->timeout  = timeout_ms;
  timer->end_time = osKernelGetTickCount() + timeout_ms;
}

void countdown_sec(Timer *timer, uint32_t timeout_sec)
{
  timer->timeout  = (timeout_sec * 1000);
  timer->end_time = osKernelGetTickCount() + (timeout_sec * 1000);
}

uint32_t left_ms(const Timer *timer)
{
  long int remaining_time = timer->end_time - osKernelGetTickCount();
  return (remaining_time < 0) ? 0 : remaining_time;
}

void init_timer(Timer *timer)
{
  timer->end_time = 0;
}

void delay(unsigned milliseconds)
{
  osDelay(milliseconds);
}
