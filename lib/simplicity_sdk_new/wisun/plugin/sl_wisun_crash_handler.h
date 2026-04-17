/***************************************************************************//**
 * @file sl_wisun_crash_handler.h
 * @brief Crash handler for Wi-SUN applications
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WISUN_CRASH_HANDLER_H
#define SL_WISUN_CRASH_HANDLER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Maximum length of a file name field
#define SL_WISUN_CRASH_FILENAME_SIZE 128
/// Maximum length of a task name field
#define SL_WISUN_CRASH_TASKNAME_SIZE 64

/// Enumeration for crash types
typedef enum {
  /// No crash has occurred
  SL_WISUN_CRASH_TYPE_NONE = 0,
  /// Crash is an assert
  SL_WISUN_CRASH_TYPE_ASSERT,
  /// Crash is a RAIL assert
  SL_WISUN_CRASH_TYPE_RAIL_ASSERT,
  /// Crash is a stack overflow failure
  SL_WISUN_CRASH_TYPE_STACK_OVERFLOW,
  /// Crash is a stack protector failure
  SL_WISUN_CRASH_TYPE_STACK_PROTECTOR,
  /// Crash is a fault
  SL_WISUN_CRASH_TYPE_FAULT,
  /// Crash is a C-RUN error
  SL_WISUN_CRASH_TYPE_CRUN_ERROR,
  /// Crash is a program termination
  SL_WISUN_CRASH_TYPE_EXIT,
  /// Maximum enum value
  SL_WISUN_CRASH_TYPE_MAX
} sl_wisun_crash_type;

/// Data structure for a fault
typedef struct {
  /// R0
  uint32_t r0;
  /// R1
  uint32_t r1;
  /// R2
  uint32_t r2;
  /// R3
  uint32_t r3;
  /// R12
  uint32_t r12;
  /// Link Register (R14)
  uint32_t lr;
  /// Exception Return Address
  uint32_t return_address;
  /// Program Status Register
  uint32_t xpsr;
  /// Configurable Fault Status Register
  uint32_t cfsr;
  /// HardFault Status Register
  uint32_t hfsr;
  /// MemManage Fault Address Register
  uint32_t mmfar;
  /// BusFault Address Register
  uint32_t bfar;
  /// Auxiliary Fault Status Register
  uint32_t afsr;
} sl_wisun_crash_fault_t;

/// Data structure for an assert
typedef struct {
  /// File name where assert occurred
  char file[SL_WISUN_CRASH_FILENAME_SIZE];
  /// Line number where assert occurred
  uint16_t line;
} sl_wisun_crash_assert_t;

/// Data structure for a RAIL assert
typedef struct {
  /// RAIL error
  uint32_t error_code;
} sl_wisun_crash_rail_assert_t;

/// Data structure for a stack overflow failure
typedef struct {
  /// Current RTOS task name
  char task[SL_WISUN_CRASH_TASKNAME_SIZE];
} sl_wisun_crash_stack_overflow_t;

/// Data structure for a stack protector failure
typedef struct {
  /// Link Register
  uint32_t lr;
} sl_wisun_crash_stack_protector_t;

/// Data structure for a C-RUN error
typedef struct {
  /// C-RUN error
  uint32_t error_code;
} sl_wisun_crash_crun_error_t;

/// Data structure for a program termination
typedef struct {
  /// Program termination status
  int status;
} sl_wisun_crash_exit_t;


/// Data structure for crash data
typedef struct {
  /// Type of the crash
  uint32_t type;
  union {
    /// Crash data for an assert
    sl_wisun_crash_assert_t assert;
    /// Crash data for a RAIL assert
    sl_wisun_crash_rail_assert_t rail_assert;
    /// Crash data for a fault
    sl_wisun_crash_fault_t fault;
    /// Crash data for a stack overflow failure
    sl_wisun_crash_stack_overflow_t stack_overflow;
    /// Crash data for a stack protector failure
    sl_wisun_crash_stack_protector_t stack_protector;
    /// Crash data for a C-RUN error
    sl_wisun_crash_crun_error_t crun_error;
    /// Crash data for a program termination
    sl_wisun_crash_exit_t exit;
  } u;
} sl_wisun_crash_t;

/**************************************************************************//**
 * Initialize Wi-SUN crash handler.
 *
 * This fuction is used to initialize the crash handler. It must be called
 * before attempting to access crash data.
 *****************************************************************************/
void sl_wisun_crash_handler_init();

/**************************************************************************//**
 * Clear Wi-SUN crash handler.
 *
 * This function is used to clear crash data. The application should call the
 * function after having read and acted on crash data.
 *****************************************************************************/
void sl_wisun_crash_handler_clear();

/**************************************************************************//**
 * Read Wi-SUN crash data.
 *
 * @return NULL if no crash data available, a pointer to the data otherwise.
 *****************************************************************************/
const sl_wisun_crash_t *sl_wisun_crash_handler_read();

#ifdef __cplusplus
}
#endif

#endif // SL_WISUN_CRASH_HANDLER_H
