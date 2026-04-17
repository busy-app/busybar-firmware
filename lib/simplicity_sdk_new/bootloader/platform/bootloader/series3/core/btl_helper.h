/***************************************************************************//**
 * @file
 * @brief Core helper functions for Silicon Labs bootloader.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef BTL_HELPER_H
#define BTL_HELPER_H

#include "em_device.h"

// Linker Symbols
#if defined (__GNUC__)
extern uint32_t __StackTop;
extern uint32_t __StackLimit;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
#else
extern uint32_t CSTACK$$Base, CSTACK$$Limit;
extern uint32_t STACKSEAL$$Limit, HEAP$$Base;
#endif

static inline void jump_to_application_routine(uint32_t startOfAppSpace)
{
  // Load SP and PC of application
  __ASM volatile(
    "mov r0, %[startOfAppSpace]       \n" // Load address of SP into R0
    "ldr r1, [r0]                     \n" // Load SP into R1
    "msr msp, r1                      \n" // Set MSP
    "msr psp, r1                      \n" // Set PSP
    "ldr r0, [r0, #4]                 \n" // Load PC into R0
    "mov pc, r0                       \n" // Set PC
    : // No outputs
    :[startOfAppSpace] "r" (startOfAppSpace)  // Inputs
    : "r0", "r1" // Clobbered registers
    );
}

#endif // BTL_HELPER_H
