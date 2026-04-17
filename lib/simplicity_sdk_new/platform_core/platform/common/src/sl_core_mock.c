/***************************************************************************//**
 * @file
 * @brief Mocked Core API for Host applications.
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

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Disable interrupts.
 ******************************************************************************/
void CORE_CriticalDisableIrq(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Enable interrupts.
 ******************************************************************************/
void CORE_CriticalEnableIrq(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Enter a CRITICAL section.
 ******************************************************************************/
CORE_irqState_t CORE_EnterCritical(void)
{
  return 0U;
}

/***************************************************************************//**
 * @brief
 *   Exit a CRITICAL section.
 ******************************************************************************/
void CORE_ExitCritical(CORE_irqState_t irqState)
{
  (void)irqState;
  return;
}

/***************************************************************************//**
 * @brief
 *   Brief interrupt enable/disable sequence to allow handling of
 *   pending interrupts.
 ******************************************************************************/
void CORE_YieldCritical(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Disable interrupts.
 ******************************************************************************/
void CORE_AtomicDisableIrq(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Enable interrupts.
 ******************************************************************************/
void CORE_AtomicEnableIrq(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Enter an ATOMIC section.
 ******************************************************************************/
CORE_irqState_t CORE_EnterAtomic(void)
{
  return (CORE_irqState_t)0U;
}

/***************************************************************************//**
 * @brief
 *   Exit an ATOMIC section.
 ******************************************************************************/
void CORE_ExitAtomic(CORE_irqState_t irqState)
{
  (void)irqState;
  return;
}

/***************************************************************************//**
 * @brief
 *   Brief interrupt enable/disable sequence to allow handling of
 *   pending interrupts.
 ******************************************************************************/
void CORE_YieldAtomic(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Check whether the current CPU operation mode is handler mode.
 ******************************************************************************/
bool CORE_InIrqContext(void)
{
  return true;
}

/***************************************************************************//**
 * @brief
 *   Check if interrupts are disabled.
 ******************************************************************************/
bool CORE_IrqIsDisabled(void)
{
  return false;
}

/***************************************************************************//**
 * @brief
 *   Returns the max time spent in critical section.
 ******************************************************************************/
uint32_t CORE_get_max_time_critical_section(void)
{
  return 0U;
}

/***************************************************************************//**
 * @brief
 *   Returns the max time spent in atomic section.
 ******************************************************************************/
uint32_t CORE_get_max_time_atomic_section(void)
{
  return 0U;
}

/***************************************************************************//**
 * @brief
 *   Clears the max time spent in atomic section.
 ******************************************************************************/
void CORE_clear_max_time_critical_section(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Clears the max time spent in atomic section.
 ******************************************************************************/
void CORE_clear_max_time_atomic_section(void)
{
  return;
}

/***************************************************************************//**
 * @brief
 *   Reset chip routine.
 ******************************************************************************/
void CORE_ResetSystem(void)
{
  return;
}
