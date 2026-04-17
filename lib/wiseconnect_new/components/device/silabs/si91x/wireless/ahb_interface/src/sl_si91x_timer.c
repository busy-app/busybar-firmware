/*******************************************************************************
* @file  rsi_timer.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

/*
 Include files
  */
#include "sli_siwx917_timer.h"
#include "sl_device.h"

/** @addtogroup DRIVER11
* @{
*/
/*==============================================*/
/**
 * @fn          void rsi_timer_expiry_interrupt_handler(void)
 * @brief       Called for every millisecond and increments the timer counter.
 * @param[in]   void
 * @return      void
 *
 */

/*==============================================*/
/**
 * @fn           uint32_t rsi_timer_read_counter()
 * @brief        Return the timer counter value.
 * @param[in]    void
 * @return       Timer counter value in milliseconds
 *
 */

uint32_t sli_si91x_timer_read_counter(void)
{
  extern uint32_t SystemCoreClock;
#define CYCLES_PER_MILLISECOND (SystemCoreClock / 1000)
  return DWT->CYCCNT / CYCLES_PER_MILLISECOND;
}

/*==============================================*/
/**
 * @fn            void rsi_init_timer(sli_si91x_timer_t *rsi_timer, uint32_t duration)
 * @brief         Initialize the timer instance with the expiry time.
 * @param[in]     rsi_timer - timer instance
 * @param[in]     duration  - duration in milli seconds
 * @return        void
 *
 */

void sli_si91x_timer_init(sli_si91x_timer_t *rsi_timer, uint32_t duration)
{
  rsi_timer->start_time = sli_si91x_timer_read_counter();
  rsi_timer->timeout    = duration;
}

/*==============================================*/
/**
 * @fn           int32_t rsi_timer_expired(sli_si91x_timer_t *timer)
 * @brief        Check whether the timer instance is expired or not.
 * @param[in]    rsi_timer - timer instance
 * @return       1 - if timer is expired \n
 *               0 - if timer is not expired
 *
 */

int32_t sli_si91x_timer_expired(const sli_si91x_timer_t *timer)
{
  if ((sli_si91x_timer_read_counter() - (timer->start_time)) > (timer->timeout))
    return 1;
  else
    return 0;
}

/*==============================================*/
/**
 * @fn           uint32_t rsi_timer_left(sli_si91x_timer_t *timer)
 * @brief        Get the remaining time for timer expiry.
 * @param[in]    rsi_timer - timer instance
 * @return       Positive value - time left to expire \n
 *               0              - if timer is expired
 *
 */

uint32_t sli_si91x_timer_left(const sli_si91x_timer_t *timer)
{
  int32_t left = (timer->timeout) - (sli_si91x_timer_read_counter() - (timer->start_time));
  return (left < 0) ? 0 : left;
}
/** @} */
