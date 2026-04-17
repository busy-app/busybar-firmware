
/***************************************************************************/ /**
* @file sl_log_helper_si91x.h
* @brief SL Log Helper Macros and Utilities for si91x
* @version 1.0.0
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

#ifndef SL_LOG_HELPER_SI91X_H
#define SL_LOG_HELPER_SI91X_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_LOGGER_COMPONENT_PRESENT
#include "sl_log_helper.h"
#else

#define SL_LOG_EMPTY_STRING_ADDRESS 0xFFFFFFFE

/** @addtogroup sl_log_helper SL Log Helper Functions
 * @brief Helper macros and utilities for the Silicon Labs logging system
 *
 * This module provides the core helper macros and utilities that enable
 * efficient logging with compile-time optimization, argument validation,
 * and backend abstraction.
 *
 * @{
 */

/**
 * @defgroup sl_log_disabled_macros Disabled Logging Macros
 * @brief Macros that disable logging when compile-time level is set to NONE
 *
 * When SL_LOG_CONFIG_LEVEL_COMPILE_TIME is set to SL_LOG_CONFIG_LEVEL_NONE,
 * all logging macros are replaced with void operations to completely eliminate
 * logging overhead at compile time.
 *
 * @{
 */

/**
 * @defgroup sl_log_memory_sections Memory Section Definitions
 * @brief Linker section attributes for log string placement
 * @{
 */

/**
 * @defgroup sl_log_compiler_support Compiler-Specific Support
 * @brief Compiler-specific definitions for string section placement
 * @{
 */
#if defined(__GNUC__)
/**
 * @brief Linker section attribute for log format strings
 *
 * This attribute places log format strings in a dedicated memory section
 * for optimized memory layout and potential compression or removal.
 */
#define SL_COMPACT_STRINGS_SECTION __attribute__((section(".log_fmt")))

#elif defined(__ICCARM__)
/**
 * @brief Compiler-specific string section placement for IAR
 *
 * IAR-specific pragma for placing log format strings in a dedicated section.
 */
#define SL_COMPACT_STRINGS_SECTION _Pragma("location=\"log_fmt\"")

#else
/**
 * @brief Default definition (no special section)
 *
 * For compilers that don't support custom section placement.
 */
#define SL_COMPACT_STRINGS_SECTION
#endif

/** @} (end addtogroup sl_log_memory_sections) */

/** @} (end addtogroup sl_log_compiler_support) */

#define SL_PRINT_ARG0_DBG(EVENT, EVENT_TYPE) \
  ({                                         \
    (void)sizeof(EVENT);                     \
    (void)sizeof(EVENT_TYPE);                \
  })
#define SL_PRINT_ARG0_ERR(EVENT, EVENT_TYPE) \
  ({                                         \
    (void)sizeof(EVENT);                     \
    (void)sizeof(EVENT_TYPE);                \
  })
#define SL_PRINT_ARG0_WRN(EVENT, EVENT_TYPE) \
  ({                                         \
    (void)sizeof(EVENT);                     \
    (void)sizeof(EVENT_TYPE);                \
  })
#define SL_PRINT_ARG0_INFO(EVENT, EVENT_TYPE) \
  ({                                          \
    (void)sizeof(EVENT);                      \
    (void)sizeof(EVENT_TYPE);                 \
  })

#define SL_PRINT_ARG1_DBG(EVENT, EVENT_TYPE, ARG1) \
  ({                                               \
    (void)sizeof(EVENT);                           \
    (void)sizeof(EVENT_TYPE);                      \
    (void)sizeof(ARG1);                            \
  })
#define SL_PRINT_ARG1_ERR(EVENT, EVENT_TYPE, ARG1) \
  ({                                               \
    (void)sizeof(EVENT);                           \
    (void)sizeof(EVENT_TYPE);                      \
    (void)sizeof(ARG1);                            \
  })
#define SL_PRINT_ARG1_WRN(EVENT, EVENT_TYPE, ARG1) \
  ({                                               \
    (void)sizeof(EVENT);                           \
    (void)sizeof(EVENT_TYPE);                      \
    (void)sizeof(ARG1);                            \
  })
#define SL_PRINT_ARG1_INFO(EVENT, EVENT_TYPE, ARG1) \
  ({                                                \
    (void)sizeof(EVENT);                            \
    (void)sizeof(EVENT_TYPE);                       \
    (void)sizeof(ARG1);                             \
  })

#define SL_PRINT_ARG2_DBG(EVENT, EVENT_TYPE, ARG1, ARG2) \
  ({                                                     \
    (void)sizeof(EVENT);                                 \
    (void)sizeof(EVENT_TYPE);                            \
    (void)sizeof(ARG1);                                  \
    (void)sizeof(ARG2);                                  \
  })
#define SL_PRINT_ARG2_ERR(EVENT, EVENT_TYPE, ARG1, ARG2) \
  ({                                                     \
    (void)sizeof(EVENT);                                 \
    (void)sizeof(EVENT_TYPE);                            \
    (void)sizeof(ARG1);                                  \
    (void)sizeof(ARG2);                                  \
  })
#define SL_PRINT_ARG2_WRN(EVENT, EVENT_TYPE, ARG1, ARG2) \
  ({                                                     \
    (void)sizeof(EVENT);                                 \
    (void)sizeof(EVENT_TYPE);                            \
    (void)sizeof(ARG1);                                  \
    (void)sizeof(ARG2);                                  \
  })
#define SL_PRINT_ARG2_INFO(EVENT, EVENT_TYPE, ARG1, ARG2) \
  ({                                                      \
    (void)sizeof(EVENT);                                  \
    (void)sizeof(EVENT_TYPE);                             \
    (void)sizeof(ARG1);                                   \
    (void)sizeof(ARG2);                                   \
  })

#define SL_PRINT_ARG3_DBG(EVENT, EVENT_TYPE, ARG1, ARG2, ARG3) \
  ({                                                           \
    (void)sizeof(EVENT);                                       \
    (void)sizeof(EVENT_TYPE);                                  \
    (void)sizeof(ARG1);                                        \
    (void)sizeof(ARG2);                                        \
    (void)sizeof(ARG3);                                        \
  })
#define SL_PRINT_ARG3_ERR(EVENT, EVENT_TYPE, ARG1, ARG2, ARG3) \
  ({                                                           \
    (void)sizeof(EVENT);                                       \
    (void)sizeof(EVENT_TYPE);                                  \
    (void)sizeof(ARG1);                                        \
    (void)sizeof(ARG2);                                        \
    (void)sizeof(ARG3);                                        \
  })
#define SL_PRINT_ARG3_WRN(EVENT, EVENT_TYPE, ARG1, ARG2, ARG3) \
  ({                                                           \
    (void)sizeof(EVENT);                                       \
    (void)sizeof(EVENT_TYPE);                                  \
    (void)sizeof(ARG1);                                        \
    (void)sizeof(ARG2);                                        \
    (void)sizeof(ARG3);                                        \
  })
#define SL_PRINT_ARG3_INFO(EVENT, EVENT_TYPE, ARG1, ARG2, ARG3) \
  ({                                                            \
    (void)sizeof(EVENT);                                        \
    (void)sizeof(EVENT_TYPE);                                   \
    (void)sizeof(ARG1);                                         \
    (void)sizeof(ARG2);                                         \
    (void)sizeof(ARG3);                                         \
  })

/** @} (end addtogroup sl_log_disabled_macros) */

/**
 * @defgroup sl_log_level_checks Log Level Check Macros
 * @brief Macros for checking if a log level should be processed
 *
 * These macros provide efficient level checking to determine if a log
 * message should be processed based on the current log level configuration.
 *
 * @{
 */

/** @brief Check if debug level logging is enabled for the given level */
#define SL_LOG_LVL_DEBUG_CHECK(level)           (level <= SL_LOG_CONFIG_LEVEL_DEBUG)

/** @brief Check if info level logging is enabled for the given level */
#define SL_LOG_LVL_INFO_CHECK(level)            (level <= SL_LOG_CONFIG_LEVEL_INFO)

/** @brief Check if error level logging is enabled for the given level */
#define SL_LOG_LVL_ERROR_CHECK(level)           (level <= SL_LOG_CONFIG_LEVEL_ERROR)

/** @brief Check if warning level logging is enabled for the given level */
#define SL_LOG_LVL_WARN_CHECK(level)            (level <= SL_LOG_CONFIG_LEVEL_WARN)

/** @} (end addtogroup sl_log_level_checks) */

/**
 * @defgroup sl_log_macro_helpers Macro Processing Helpers
 * @brief Helper macros for token concatenation and argument processing
 *
 * These macros provide the foundation for the variadic logging system,
 * enabling automatic selection of appropriate logging functions based
 * on argument count and generating unique identifiers.
 *
 * @{
 */

/** @brief Token concatenation helper */
#define CONCAT(a, b)                            a##b

/** @brief Expanded token concatenation helper */
#define EXPAND_CONCAT(a, b)                     CONCAT(a, b)

/** @brief Generate unique name based on line number */
#define UNIQUE_NAME(base)                       EXPAND_CONCAT(base, EXPAND_CONCAT(__LINE__, 0))

/** @brief Implementation macro for counting variadic arguments */
#define COUNT_ARGS_IMPL(_0, _1, _2, _3, N, ...) N

/** @brief Count the number of variadic arguments (up to 3) */
#define COUNT_ARGS(fmt, ...)                    COUNT_ARGS_IMPL(_, ##__VA_ARGS__, 3, 2, 1, 0, 0)

/** @brief Helper for choosing logging macro based on argument count */
#define LOG_MACRO_CHOOSER2(count)               EXPAND_CONCAT(SL_PRINT_ARG, count##_)

/** @brief Macro dispatcher for selecting appropriate logging function */
#define LOG_MACRO_CHOOSER1(count)               LOG_MACRO_CHOOSER2(count)

/** @} (end addtogroup sl_log_macro_helpers) */

/**
 * @defgroup sl_log_common_macros Common Logging Macros
 * @brief Core macros for printf-style and event-based logging
 *
 * These macros provide the foundation for both printf-style string logging
 * and numeric event-based logging with compile-time argument validation.
 *
 * @{
 */

// Common API Functions

/**
 * @defgroup sl_log_common_macros Common Logging Macros
 * @brief Core macros for printf-style and event-based logging
 *
 * These macros provide the foundation for both printf-style string logging
 * and numeric event-based logging with compile-time argument validation.
 *
 * @{
 */

/**
 * @brief Common printf-style logging macro
 *
 * This API discards the string provided by the user and sends an empty string address magic word.
 *
 * @param level Log level suffix (INFO, DBG, ERR, WRN)
 * @param fmt Format string
 * @param ... Variable arguments (up to 3)
 */
#define sl_printf_common(level, fmt, ...)                                     \
  _Static_assert(COUNT_ARGS(fmt, ##__VA_ARGS__) <= 3, "Too many arguments!"); \
  EXPAND_CONCAT(LOG_MACRO_CHOOSER1(COUNT_ARGS(fmt, ##__VA_ARGS__)), level)    \
  (SL_LOG_EMPTY_STRING_ADDRESS, 0, ##__VA_ARGS__)

/**
* @brief Common event-based logging macro
*
* Logs a numeric event ID with optional arguments. Includes compile-time
* validation to ensure no more than 3 arguments are provided.
*
* @param level Log level suffix (INFO, DBG, ERR, WRN)
* @param event_id Numeric event identifier
* @param ... Variable arguments (up to 3)
*/
#define sl_event_common(level, event_id, ...)                                      \
  _Static_assert(COUNT_ARGS(event_id, ##__VA_ARGS__) <= 3, "Too many arguments!"); \
  EXPAND_CONCAT(LOG_MACRO_CHOOSER1(COUNT_ARGS(event_id, ##__VA_ARGS__)), level)    \
  (event_id, 1, ##__VA_ARGS__)

/* Compile-time–controlled PRINTF-style macros */

#define SL_PRINT_STRING_INFO(fmt, ...) sl_printf_common(INFO, fmt, ##__VA_ARGS__)

#define SL_PRINT_STRING_DEBUG(fmt, ...) sl_printf_common(DBG, fmt, ##__VA_ARGS__)

#define SL_PRINT_STRING_WARN(fmt, ...) sl_printf_common(WRN, fmt, ##__VA_ARGS__)

#define SL_PRINT_STRING_ERROR(fmt, ...) sl_printf_common(ERR, fmt, ##__VA_ARGS__)

/* Compile-time–controlled EVENT-style macros */

#define SL_PRINT_EVENT_INFO(event_id, ...) sl_event_common(INFO, event_id, ##__VA_ARGS__)

#define SL_PRINT_EVENT_DEBUG(event_id, ...) sl_event_common(DBG, event_id, ##__VA_ARGS__)

#define SL_PRINT_EVENT_WARN(event_id, ...) sl_event_common(WRN, event_id, ##__VA_ARGS__)

#define SL_PRINT_EVENT_ERROR(event_id, ...) sl_event_common(ERR, event_id, ##__VA_ARGS__)

#endif

#ifdef __cplusplus
}
#endif

#endif // SL_LOG_HELPER_SI91X_H
