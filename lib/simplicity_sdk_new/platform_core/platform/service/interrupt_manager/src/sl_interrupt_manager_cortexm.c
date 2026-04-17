/***************************************************************************//**
 * @file
 * @brief Interrupt manager API to enable disable interrupts.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "em_device.h"
#include "sl_core.h"
#include "sl_assert.h"
#include "sli_interrupt_manager.h"

#if defined (SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"

#if defined(SL_CATALOG_INTERRUPT_MANAGER_HOOKS_PRESENT)
#include "sli_interrupt_manager_hooks.h"
#endif /* SL_CATALOG_INTERRUPT_MANAGER_HOOKS_PRESENT */

#if defined(SL_CATALOG_CODE_CLASSIFICATION_VALIDATOR_PRESENT)
#include "sli_code_classification_validator.h"
#endif /* SL_CATALOG_CODE_CLASSIFICATION_VALIDATOR_PRESENT */

#endif /* SL_COMPONENT_CATALOG_PRESENT */

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/
#define CORTEX_INTERRUPTS       (16)
#define TOTAL_INTERRUPTS        (CORTEX_INTERRUPTS + EXT_IRQ_COUNT)

#define LOWEST_NVIC_PRIORITY    ((1U << __NVIC_PRIO_BITS) - 1U)
#define SL_INTERRUPT_MANAGER_DEFAULT_PRIORITY 5U

// Use same alignement as IAR
#define VECTOR_TABLE_ALIGNMENT  (512)

// Interrupt vector placement is in RAM
#if defined(SL_CATALOG_INTERRUPT_MANAGER_VECTOR_TABLE_IN_RAM_PRESENT)
#define VECTOR_TABLE_IN_RAM (1)
#endif

#if defined(SL_CATALOG_INTERRUPT_MANAGER_HOOKS_PRESENT)
#define SL_INTERRUPT_MANAGER_ENABLE_HOOKS (1)
#endif

// Interrupt vector table need to be in a different section of RAM for Cortex-M55.
#if defined(__CM55_REV)
#if defined(__GNUC__)
#define VECTOR_TABLE_SECTION  __attribute__((section(".vector_table_ram")))
#else
#define VECTOR_TABLE_SECTION  _Pragma("location =\".vector_table_ram\"")
#endif
#else
#define VECTOR_TABLE_SECTION
#endif

#if defined(VECTOR_TABLE_IN_RAM)

#if defined(__GNUC__)
// Create a vector table in RAM aligned to 512.
static sl_interrupt_manager_irq_handler_t vector_table_ram[TOTAL_INTERRUPTS] __attribute__((aligned(VECTOR_TABLE_ALIGNMENT) )) VECTOR_TABLE_SECTION;
#elif defined(__ICCARM__)
#pragma data_alignment = VECTOR_TABLE_ALIGNMENT
static sl_interrupt_manager_irq_handler_t vector_table_ram[TOTAL_INTERRUPTS] VECTOR_TABLE_SECTION;
#endif /* defined(__GNUC__) */

#if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
// When interrupt manager hooks are enabled, the actual vector table (either in
// ram or in flash) will call an ISR wrapper. The actual ISRs will be registered
// and called from the wrapped_vector_table.
#if defined(__GNUC__)
static sl_interrupt_manager_irq_handler_t wrapped_vector_table[TOTAL_INTERRUPTS] __attribute__((aligned(VECTOR_TABLE_ALIGNMENT) )) VECTOR_TABLE_SECTION;
#elif defined(__ICCARM__)
#pragma data_alignment = VECTOR_TABLE_ALIGNMENT
static sl_interrupt_manager_irq_handler_t wrapped_vector_table[TOTAL_INTERRUPTS] VECTOR_TABLE_SECTION;
#endif /* defined(__GNUC__) */
#endif /* SL_INTERRUPT_MANAGER_ENABLE_HOOKS */

#endif /* VECTOR_TABLE_IN_RAM */

/*******************************************************************************
 *****************************   PROTOTYPES  ***********************************
 ******************************************************************************/
static void enable_interrupt(int32_t irqn);
static void disable_interrupt(int32_t irqn);
static void set_priority(int32_t irqn, uint32_t priority);
static uint32_t get_priority(int32_t irqn);

#if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
#if defined(SL_CATALOG_CODE_CLASSIFICATION_VALIDATOR_PRESENT)
CCV_SECTION
#else
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_INTERRUPT_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
#endif
static void sli_interrupt_manager_isr_wrapper(void);
#endif /* SL_INTERRUPT_MANAGER_ENABLE_HOOKS */

/*******************************************************************************
 *****************************   VARIABLES   ***********************************
 ******************************************************************************/

// Initialization flag.
static bool is_interrupt_manager_initialized = false;

#if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
static volatile uint32_t interrupt_nesting_counter = 0U;
#endif

/*******************************************************************************
 *****************************   FUNCTIONS   ***********************************
 ******************************************************************************/

#if defined(VECTOR_TABLE_IN_RAM)
#if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)

static void sli_interrupt_manager_isr_wrapper(void)
{
  // Grab the current active interrupt
  uint32_t irqn = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk);

  sli_interrupt_manager_irq_enter_hook();

  __disable_irq();
  interrupt_nesting_counter++;
  __enable_irq();

  // Call the interrupt function
  wrapped_vector_table[irqn]();

  __disable_irq();
  interrupt_nesting_counter--;
  __enable_irq();

  sli_interrupt_manager_irq_exit_hook();

#if defined(SL_CATALOG_POWER_MANAGER_ARM_SLEEP_ON_EXIT_PRESENT)
  if (SCB->SCR & SCB_SCR_SLEEPONEXIT_Msk && interrupt_nesting_counter == 0U) {
    __DSB();
    __ISB();
  }
#endif
}

#endif /* SL_INTERRUPT_MANAGER_ENABLE_HOOKS */

/***************************************************************************//**
 * @brief
 *   Set a new RAM based interrupt vector table.
 ******************************************************************************/
sl_interrupt_manager_irq_handler_t *sli_interrupt_manager_set_irq_table(sl_interrupt_manager_irq_handler_t *table,
                                                                        uint32_t handler_count)
{
  sl_interrupt_manager_irq_handler_t * current;

  // ASSERT if misaligned with respect to the VTOR register implementation.
  EFM_ASSERT(((uint32_t)table & ~SCB_VTOR_TBLOFF_Msk) == 0U);

  // ASSERT if misaligned with respect to the vector table size.
  // The vector table address must be aligned at its size rounded up to nearest 2^n.
  EFM_ASSERT(((uint32_t)table
              & ((1UL << (32UL - __CLZ((handler_count * 4UL) - 1UL))) - 1UL))
             == 0UL);

  // Disable all interrupts while updating the vector table
  sl_interrupt_manager_disable_interrupts();

  current = (sl_interrupt_manager_irq_handler_t*)SCB->VTOR;

  SCB->VTOR = (uint32_t)table;

  // Make sure all explicit memory access are complete before proceding.
  __DSB();

  sl_interrupt_manager_enable_interrupts();

  return current;
}

#endif /* VECTOR_TABLE_IN_RAM */

/***************************************************************************//**
 * @brief
 *   Initialize interrupt controller hardware and initialise vector table in RAM.
 *
 * @note
 *   The interrupt manager init function will perform the initialization only
 *   once even if it's called multiple times.
 ******************************************************************************/
void sl_interrupt_manager_init(void)
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();

  if (!is_interrupt_manager_initialized) {
    is_interrupt_manager_initialized = true;
    CORE_EXIT_ATOMIC();
  } else {
    CORE_EXIT_ATOMIC();
    return;
  }

  #if defined(VECTOR_TABLE_IN_RAM)

  sl_interrupt_manager_irq_handler_t* current;

  CORE_ENTER_CRITICAL();

#if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
  interrupt_nesting_counter = 0U;
#endif

  current = (sl_interrupt_manager_irq_handler_t*)SCB->VTOR;

  // copy ROM vector table to RAM table
  for (uint32_t i = 0; i < TOTAL_INTERRUPTS; i++) {
    #if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
    wrapped_vector_table[i] = current[i];
    if ( i >= CORTEX_INTERRUPTS ) {
      vector_table_ram[i] = sli_interrupt_manager_isr_wrapper;
    } else {
      vector_table_ram[i] = current[i];
    }
    #else
    vector_table_ram[i] = current[i];
    #endif
  }

  // Set RAM table as irq table.
  sli_interrupt_manager_set_irq_table(vector_table_ram, TOTAL_INTERRUPTS);

  CORE_EXIT_CRITICAL();

  #endif /* VECTOR_TABLE_IN_RAM */

  for (IRQn_Type i = SVCall_IRQn; i < EXT_IRQ_COUNT; i++) {
    sl_interrupt_manager_set_irq_priority(i, SL_INTERRUPT_MANAGER_DEFAULT_PRIORITY);
  }
}

/***************************************************************************//**
 * @brief
 *   Reset the cpu core.
 ******************************************************************************/
void sl_interrupt_manager_reset_system(void)
{
  CORE_RESET_SYSTEM();
}

/***************************************************************************//**
 * @brief
 *   Disable interrupts.
 ******************************************************************************/
void sl_interrupt_manager_disable_interrupts(void)
{
  __disable_irq();
}

/***************************************************************************//**
 * @brief
 *   Enable interrupts.
 ******************************************************************************/
void sl_interrupt_manager_enable_interrupts(void)
{
  __enable_irq();
}

/***************************************************************************//**
 * @brief
 *   Disable interrupt for an interrupt source.
 ******************************************************************************/
void sl_interrupt_manager_disable_irq(int32_t irqn)
{
  EFM_ASSERT((irqn >= 0) && (irqn <= EXT_IRQ_COUNT));

  disable_interrupt(irqn);
}

/***************************************************************************//**
 * @brief
 *   Enable interrupt for an interrupt source.
 ******************************************************************************/
void sl_interrupt_manager_enable_irq(int32_t irqn)
{
  EFM_ASSERT((irqn >= 0) && (irqn <= EXT_IRQ_COUNT));

  enable_interrupt(irqn);
}

/***************************************************************************//**
 * @brief
 *   Check if an interrupt is disabled.
 ******************************************************************************/
bool sl_interrupt_manager_is_irq_disabled(int32_t irqn)
{
  EFM_ASSERT((irqn >= 0) && (irqn < EXT_IRQ_COUNT));

  return (NVIC->ISER[irqn >> 5U] & (1UL << (irqn & 0x1FUL))) == 0UL;
}

/***************************************************************************//**
 * @brief
 *   Check if an interrupt is disabled or blocked.
 ******************************************************************************/
bool sl_interrupt_manager_is_irq_blocked(int32_t irqn)
{
  uint32_t irq_priority;
  uint32_t active_irq;

#if (__CORTEX_M >= 3)
  uint32_t basepri;

  EFM_ASSERT((irqn >= MemoryManagement_IRQn)
             && (irqn < (IRQn_Type)EXT_IRQ_COUNT));
#else
  EFM_ASSERT((irqn >= SVCall_IRQn) && ((IRQn_Type)irqn < EXT_IRQ_COUNT));
#endif

  if ((__get_PRIMASK() & 1U) != 0U) {
    return true;
  }

  if (sl_interrupt_manager_is_irq_disabled(irqn)) {
    return true;
  }

  irq_priority  = get_priority(irqn);

#if (__CORTEX_M >= 3)
  basepri = __get_BASEPRI();

  if ((basepri != 0U)
      && (irq_priority >= (basepri >> (8U - __NVIC_PRIO_BITS)))) {
    return true;
  }
#endif

  active_irq = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos;

  if ((active_irq != 0U)
      && (irq_priority >= get_priority(active_irq - 16U))) {
    return true;
  }

  return false;
}

/***************************************************************************//**
 * @brief
 *   Get Pending Interrupt.
 ******************************************************************************/
bool sl_interrupt_manager_is_irq_pending(int32_t irqn)
{
  if (irqn >= 0) {
    return (NVIC->ISPR[((uint32_t)irqn) >> 5UL] & (1UL << (((uint32_t)irqn) & 0x1FUL))) != 0UL;
  } else {
    return false;
  }
}

/***************************************************************************//**
 * @brief
 *   Set irq status to pending.
 ******************************************************************************/
void sl_interrupt_manager_set_irq_pending(int32_t irqn)
{
  if (irqn >= 0) {
    NVIC->ISPR[((uint32_t)irqn) >> 5UL] = (uint32_t)(1UL << (((uint32_t)irqn) & 0x1FUL));
  }
}

/***************************************************************************//**
 * @brief
 *   Clear Pending Interrupt.
 ******************************************************************************/
void sl_interrupt_manager_clear_irq_pending(int32_t irqn)
{
  if (irqn >= 0) {
    NVIC->ICPR[((uint32_t)irqn) >> 5UL] = (uint32_t)(1UL << (((uint32_t)irqn) & 0x1FUL));
  }
}

/***************************************************************************//**
 * @brief
 *   Set the interrupt handler of an interrupt source.
 ******************************************************************************/
sl_status_t sl_interrupt_manager_set_irq_handler(int32_t irqn,
                                                 sl_interrupt_manager_irq_handler_t handler)
{
#if defined(VECTOR_TABLE_IN_RAM)

  uint32_t interrupt_status;
  sl_interrupt_manager_irq_handler_t *table;

  CORE_DECLARE_IRQ_STATE;

  if ((irqn < 0) || (irqn >= EXT_IRQ_COUNT)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_ENTER_CRITICAL();

  interrupt_status = (uint32_t)(((NVIC->ISER[((uint32_t)irqn) >> 5UL] & (1UL << (((uint32_t)irqn) & 0x1FUL))) != 0UL) ? 1UL : 0UL);

  // Disable irqn interrupt while updating the handler's address
  sl_interrupt_manager_disable_irq(irqn);

  #if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
  table = wrapped_vector_table;
  #else
  table = (sl_interrupt_manager_irq_handler_t*)SCB->VTOR;
  #endif /* SL_INTERRUPT_MANAGER_ENABLE_HOOKS */

  // Make sure the VTOR is initialized and points to a table in RAM.
  if (!is_interrupt_manager_initialized) {
    CORE_EXIT_CRITICAL();
    return SL_STATUS_NOT_INITIALIZED;
  }

  table[irqn + 16] = handler;

  // Make sure all explicit memory access are complete before proceding.
  __DSB();
  __ISB();

  CORE_EXIT_CRITICAL();

  if (interrupt_status == 1) {
    sl_interrupt_manager_enable_irq(irqn);
  }

  return SL_STATUS_OK;
#else
  (void) irqn;
  (void) handler;
  return SL_STATUS_INVALID_CONFIGURATION;
#endif /* VECTOR_TABLE_IN_RAM */
}

/***************************************************************************//**
 * @brief
 *   Get the interrupt preemption priority of an interrupt source.
 ******************************************************************************/
uint32_t sl_interrupt_manager_get_irq_priority(int32_t irqn)
{
  uint32_t irq_priority = 0;

  EFM_ASSERT((irqn >= -CORTEX_INTERRUPTS) && (irqn <= EXT_IRQ_COUNT));

  irq_priority = get_priority(irqn);

  return irq_priority;
}

/***************************************************************************//**
 * @brief
 *   Set the interrupt preemption priority of an interrupt source.
 ******************************************************************************/
void sl_interrupt_manager_set_irq_priority(int32_t irqn, uint32_t priority)
{
  EFM_ASSERT((irqn >= -CORTEX_INTERRUPTS) && (irqn <= EXT_IRQ_COUNT));
  EFM_ASSERT(priority <= LOWEST_NVIC_PRIORITY);

  set_priority(irqn, priority);
}

/***************************************************************************//**
 * @brief
 *   Increase the interrupt preemption priority of an interrupt source
 *   relative to the default priority.
 ******************************************************************************/
void sl_interrupt_manager_increase_irq_priority_from_default(int32_t irqn, uint32_t diff)
{
  uint32_t prio = sl_interrupt_manager_get_default_priority();
  sl_interrupt_manager_set_irq_priority(irqn, prio - diff);
}

/***************************************************************************//**
 * @brief
 *   Decrease the interrupt preemption priority of an interrupt source.
 *   relative to the default priority.
 ******************************************************************************/
void sl_interrupt_manager_decrease_irq_priority_from_default(int32_t irqn, uint32_t diff)
{
  uint32_t prio = sl_interrupt_manager_get_default_priority();
  sl_interrupt_manager_set_irq_priority(irqn, prio + diff);
}

/***************************************************************************//**
 * @brief
 *   Get the default interrupt preemption priority value.
 ******************************************************************************/
uint32_t sl_interrupt_manager_get_default_priority(void)
{
  return SL_INTERRUPT_MANAGER_DEFAULT_PRIORITY;
}

/***************************************************************************//**
 * @brief
 *   Get the highest interrupt preemption priority value.
 ******************************************************************************/
uint32_t sl_interrupt_manager_get_highest_priority(void)
{
  return 0;
}

/***************************************************************************//**
 * @brief
 *   Get the lowest interrupt preemption priority value.
 ******************************************************************************/
uint32_t sl_interrupt_manager_get_lowest_priority(void)
{
  return LOWEST_NVIC_PRIORITY;
}

/***************************************************************************//**
 * @brief
 *   Get the interrupt active status.
 ******************************************************************************/
uint32_t sl_interrupt_manager_get_active_irq(int32_t irqn)
{
  if (irqn >= 0) {
    return((uint32_t)(((NVIC->IABR[((uint32_t)irqn) >> 5UL] & (1UL << (((uint32_t)irqn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));
  } else {
    return 0U;
  }
}

/***************************************************************************//**
 * @brief
 *   Enable an interrupts on the current core
 ******************************************************************************/
static void enable_interrupt(int32_t irqn)
{
  if (irqn >= 0) {
    __COMPILER_BARRIER();
    NVIC->ISER[((uint32_t)irqn) >> 5UL] = (uint32_t)(1UL << (((uint32_t)irqn) & 0x1FUL));
    __COMPILER_BARRIER();
  }
}

/***************************************************************************//**
 * @brief
 *   Disable an interrupts on the current core
 ******************************************************************************/
static void disable_interrupt(int32_t irqn)
{
  if (irqn >= 0) {
    NVIC->ICER[((uint32_t)irqn) >> 5UL] = (uint32_t)(1UL << (((uint32_t)irqn) & 0x1FUL));
    __DSB();
    __ISB();
  }
}

/***************************************************************************//**
 * @brief
 *   Set the priority of an interrupt.
 ******************************************************************************/
static void set_priority(int32_t irqn, uint32_t priority)
{
  __NVIC_SetPriority((IRQn_Type)irqn, priority);
}

/***************************************************************************//**
 * @brief
 *   Get the priority of an interrupt.
 ******************************************************************************/
static uint32_t get_priority(int32_t irqn)
{
  return __NVIC_GetPriority((IRQn_Type)irqn);
}

/***************************************************************************//**
 * @brief
 *   Gets the interrupt nesting count.
 ******************************************************************************/
uint32_t sli_interrupt_manager_get_nested_interrupt_counter(void)
{
  uint32_t counter = 0U;
#if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  counter = interrupt_nesting_counter;

  CORE_EXIT_CRITICAL();
#endif
  return counter;
}

/***************************************************************************//**
 * @brief
 *   Get the current table of ISRs.
 ******************************************************************************/
sl_interrupt_manager_irq_handler_t* sl_interrupt_manager_get_isr_table(void)
{
#if defined(SL_INTERRUPT_MANAGER_ENABLE_HOOKS)
  return wrapped_vector_table;
#else
  return (sl_interrupt_manager_irq_handler_t*)SCB->VTOR;
#endif /* SL_INTERRUPT_MANAGER_ENABLE_HOOKS */
}
