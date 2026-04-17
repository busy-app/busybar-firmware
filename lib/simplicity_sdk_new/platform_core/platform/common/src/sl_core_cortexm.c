/***************************************************************************//**
 * @file
 * @brief Core API implemented for CortexM
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_core.h"
#include "sl_core_config.h"
#include "sl_common.h"
#include "em_device.h"

/**************************************************************************//**
 * @addtogroup sl_core
 * @{
 *****************************************************************************/

/*******************************************************************************
 **************************   STRUCTS   ****************************************
 ******************************************************************************/
/// A Cycle Counter Instance.
typedef struct {
  uint32_t start;    /*!< Cycle counter at start of recording. */
  uint32_t cycles;   /*!< Cycles elapsed in last recording. */
  uint32_t max;      /*!< Max recorded cycles since last reset or init. */
} dwt_cycle_counter_handle_t;

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
// cycle counter to record atomic sections
dwt_cycle_counter_handle_t atomic_cycle_counter   = { 0 };
// cycle counter to record critical sections
dwt_cycle_counter_handle_t critical_cycle_counter = { 0 };
#endif

/** @endcond */

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_CORE, SL_CODE_CLASS_TIME_CRITICAL)
static void cycle_counter_start(dwt_cycle_counter_handle_t *handle);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_CORE, SL_CODE_CLASS_TIME_CRITICAL)
static void cycle_counter_stop(dwt_cycle_counter_handle_t *handle);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Disable interrupts.
 ******************************************************************************/
SL_WEAK void CORE_CriticalDisableIrq(void)
{
  __disable_irq();
}

/***************************************************************************//**
 * @brief
 *   Enable interrupts.
 * @note
 *   __ISB() makes sure pending interrupts are executed before returning.
 *   This can be a problem if the first instruction after changing the BASEPRI
 *   or PRIMASK assumes that the pending interrupts have already been processed.
 ******************************************************************************/
SL_WEAK void CORE_CriticalEnableIrq(void)
{
  __enable_irq();
  __ISB();
}

/***************************************************************************//**
 * @brief
 *   Enter a CRITICAL section.
 ******************************************************************************/
SL_WEAK CORE_irqState_t CORE_EnterCritical(void)
{
  CORE_irqState_t irqState = __get_PRIMASK();
  __disable_irq();
#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  if (irqState == 0U) {
    cycle_counter_start(&critical_cycle_counter);
  }
#endif
  return irqState;
}

/***************************************************************************//**
 * @brief
 *   Exit a CRITICAL section.
 * @note
 *   __ISB() makes sure pending interrupts are executed before returning.
 *   This can be a problem if the first instruction after changing the BASEPRI
 *   or PRIMASK assumes that the pending interrupts have already been processed.
 ******************************************************************************/
SL_WEAK void CORE_ExitCritical(CORE_irqState_t irqState)
{
  if (irqState == 0U) {
#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
    cycle_counter_stop(&critical_cycle_counter);
#endif
    __enable_irq();
    __ISB();
  }
}

/***************************************************************************//**
 * @brief
 *   Brief interrupt enable/disable sequence to allow handling of
 *   pending interrupts.
 ******************************************************************************/
SL_WEAK void CORE_YieldCritical(void)
{
  if ((__get_PRIMASK() & 1U) != 0U) {
    __enable_irq();
    __ISB();
    __disable_irq();
  }
}

/***************************************************************************//**
 * @brief
 *   Disable interrupts.
 ******************************************************************************/
SL_WEAK void CORE_AtomicDisableIrq(void)
{
#ifndef __CM0PLUS_REV
  __set_BASEPRI(CORE_ATOMIC_BASE_PRIORITY_LEVEL << (8UL - __NVIC_PRIO_BITS));
#else
  __disable_irq();
#endif
}

/***************************************************************************//**
 * @brief
 *   Enable interrupts.
 * @note
 *   __ISB() makes sure pending interrupts are executed before returning.
 *   This can be a problem if the first instruction after changing the BASEPRI
 *   or PRIMASK assumes that the pending interrupts have already been processed.
 ******************************************************************************/
SL_WEAK void CORE_AtomicEnableIrq(void)
{
#ifndef __CM0PLUS_REV
  __set_BASEPRI(0);
#else
  __enable_irq();
#endif
  __ISB();
}

/***************************************************************************//**
 * @brief
 *   Enter an ATOMIC section.
 ******************************************************************************/
SL_WEAK CORE_irqState_t CORE_EnterAtomic(void)
{
#ifndef __CM0PLUS_REV
  CORE_irqState_t irqState = __get_BASEPRI();
  __set_BASEPRI(CORE_ATOMIC_BASE_PRIORITY_LEVEL << (8U - __NVIC_PRIO_BITS));
#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  if ((irqState & (CORE_ATOMIC_BASE_PRIORITY_LEVEL << (8U - __NVIC_PRIO_BITS)))
      != (CORE_ATOMIC_BASE_PRIORITY_LEVEL << (8U - __NVIC_PRIO_BITS))) {
    cycle_counter_start(&atomic_cycle_counter);
  }
#endif
  return irqState;
#else
  CORE_irqState_t irqState = __get_PRIMASK();
  __disable_irq();
#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  if (irqState == 0U) {
    cycle_counter_start(&critical_cycle_counter);
  }
#endif
  return irqState;
#endif
}

/***************************************************************************//**
 * @brief
 *   Exit an ATOMIC section.
 * @note
 *   __ISB() makes sure pending interrupts are executed before returning.
 *   This can be a problem if the first instruction after changing the BASEPRI
 *   or PRIMASK assumes that the pending interrupts have already been processed.
 ******************************************************************************/
SL_WEAK void CORE_ExitAtomic(CORE_irqState_t irqState)
{
#ifndef __CM0PLUS_REV
#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  if ((irqState & (CORE_ATOMIC_BASE_PRIORITY_LEVEL << (8U - __NVIC_PRIO_BITS)))
      != (CORE_ATOMIC_BASE_PRIORITY_LEVEL << (8U - __NVIC_PRIO_BITS))) {
    cycle_counter_stop(&atomic_cycle_counter);
  }
#endif
  __set_BASEPRI(irqState);
  __ISB();
#else
  if (irqState == 0U) {
#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
    cycle_counter_stop(&critical_cycle_counter);
#endif
    __enable_irq();
    __ISB();
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Brief interrupt enable/disable sequence to allow handling of
 *   pending interrupts.
 ******************************************************************************/
SL_WEAK void CORE_YieldAtomic(void)
{
#ifndef __CM0PLUS_REV
  CORE_irqState_t basepri = __get_BASEPRI();
  if (basepri >= (CORE_ATOMIC_BASE_PRIORITY_LEVEL << (8U - __NVIC_PRIO_BITS))) {
    __set_BASEPRI(0);
    __ISB();
    __set_BASEPRI(basepri);
  }
#else
  if ((__get_PRIMASK() & 1U) != 0U) {
    __enable_irq();
    __ISB();
    __disable_irq();
  }
#endif
}

/***************************************************************************//**
 * @brief
 *   Check whether the current CPU operation mode is handler mode.
 ******************************************************************************/
SL_WEAK bool CORE_InIrqContext(void)
{
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U;
}

/***************************************************************************//**
 * @brief
 *   Check if interrupts are disabled.
 ******************************************************************************/
SL_WEAK bool CORE_IrqIsDisabled(void)
{
#ifndef __CM0PLUS_REV
  return ((__get_PRIMASK() & 1U) == 1U)
         || (__get_BASEPRI() >= (CORE_ATOMIC_BASE_PRIORITY_LEVEL
                                 << (8U - __NVIC_PRIO_BITS)));
#else
  return (__get_PRIMASK() & 1U == 1U);
#endif
}

#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
/***************************************************************************//**
 * @brief
 *   Start a recording.
 *
 * @param[in] handle
 *   Pointer to initialized counter handle.
 *
 * @note SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING must be enabled.
 ******************************************************************************/
static void cycle_counter_start(dwt_cycle_counter_handle_t *handle)
{
  handle->start = DWT->CYCCNT;
}
#endif //(SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)

#if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
/***************************************************************************//**
 * @brief
 *   Stop a recording.
 *
 * @param[in] handle
 *   Pointer to initialized counter handle.
 *
 * @note SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING must be enabled.
 ******************************************************************************/
static void cycle_counter_stop(dwt_cycle_counter_handle_t *handle)
{
  handle->cycles = DWT->CYCCNT - handle->start;

  if (handle->cycles > handle->max) {
    handle->max = handle->cycles;
  }
}
#endif //(SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)

/***************************************************************************//**
 * @brief
 *   Returns the max time spent in critical section.
 ******************************************************************************/
uint32_t CORE_get_max_time_critical_section(void)
{
  #if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  return critical_cycle_counter.max;
  #else
  return 0U;
  #endif //(SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
}

/***************************************************************************//**
 * @brief
 *   Returns the max time spent in atomic section.
 ******************************************************************************/
uint32_t CORE_get_max_time_atomic_section(void)
{
  #if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  return atomic_cycle_counter.max;
  #else
  return 0U;
  #endif //(SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
}

/***************************************************************************//**
 * @brief
 *   Clears the max time spent in atomic section.
 ******************************************************************************/
void CORE_clear_max_time_critical_section(void)
{
  #if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  critical_cycle_counter.max = 0;
  #endif //(SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
}

/***************************************************************************//**
 * @brief
 *   Clears the max time spent in atomic section.
 ******************************************************************************/
void CORE_clear_max_time_atomic_section(void)
{
  #if (SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
  atomic_cycle_counter.max = 0;
  #endif //(SL_CORE_DEBUG_INTERRUPTS_MASKED_TIMING == 1)
}

/***************************************************************************//**
 * @brief
 *   Reset chip routine.
 ******************************************************************************/
void CORE_ResetSystem(void)
{
  // Ensure all outstanding memory accesses including buffered writes are
  // completed before reset
  __DSB();

#ifndef __CM0PLUS_REV
  // Keep priority group unchanged
  SCB->AIRCR  = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos)
                | (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk)
                | SCB_AIRCR_SYSRESETREQ_Msk;
#else
  SCB->AIRCR  = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos)
                | SCB_AIRCR_SYSRESETREQ_Msk;
#endif

  // Ensure completion of memory access
  __DSB();

  // Wait until reset
  for (;; ) {
    __NOP();
  }
}

/** @} (end addtogroup sl_core) */
