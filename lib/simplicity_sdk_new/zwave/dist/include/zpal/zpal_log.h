/**
 * @file
 * @brief Provides an API for logging messages from all Z-Wave components.
 *
 * @details Allows logging messages for different software components,
 * on different output channels.
 *
 * The macro zpal_log(component, level, pformat, ...) should be implemented
 * by the platform to provide the actual logging functionality.
 *
 * @copyright 2025 Silicon Laboratories Inc.
 */

#ifndef ZPAL_LOG_H_
#define ZPAL_LOG_H_

typedef enum zpal_log_level_t_ {
  ZPAL_LOG_LEVEL_DEBUG,
  ZPAL_LOG_LEVEL_INFO,
  ZPAL_LOG_LEVEL_WARNING,
  ZPAL_LOG_LEVEL_ERROR,
  ZPAL_LOG_LEVEL_END
} zpal_log_level_t;

typedef enum zpal_log_component_t_ {
  ZPAL_LOG_NONE,
  ZPAL_LOG_APP,
  ZPAL_LOG_HW,
  ZPAL_LOG_CC_ASSOCIATION,
  ZPAL_LOG_CC_BATTERY,
  ZPAL_LOG_CC_BINARY_SWITCH,
  ZPAL_LOG_CC_CENTRAL_SCENE,
  ZPAL_LOG_CC_COLOR_SWITCH,
  ZPAL_LOG_CC_DOOR_LOCK,
  ZPAL_LOG_CC_FIRMWARE_UPDATE,
  ZPAL_LOG_CC_INDICATOR,
  ZPAL_LOG_CC_MULTI_CHANNEL,
  ZPAL_LOG_CC_MULTILEVEL_SENSOR,
  ZPAL_LOG_CC_MULTILEVEL_SWITCH,
  ZPAL_LOG_CC_NOTIFICATION,
  ZPAL_LOG_CC_USER_CODE,
  ZPAL_LOG_CC_USER_CREDENTIAL,
  ZPAL_LOG_CC_VERSION,
  ZPAL_LOG_CC_WAKE_UP,
  ZPAL_LOG_CC_ZWAVE_PLUS_INFO,
  ZPAL_LOG_ZAF_ACTUATOR,
  ZPAL_LOG_ZAF_APP_TIMER,
  ZPAL_LOG_ZAF_CMD_PUBLISHER,
  ZPAL_LOG_ZAF_COMMON,
  ZPAL_LOG_ZAF_EVENT_DISTRIBUTOR,
  ZPAL_LOG_ZAF_NVM,
  ZPAL_LOG_ZAF_TRANSPORT,
  ZPAL_LOG_ZAF_TSE,
  ZPAL_LOG_SHUTDOWN_MANAGER,
  ZPAL_LOG_END
} zpal_log_component_t;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif // defined(__clang__)

/*
 * This function must never be called directly!
 * All log messages must be sent via the macro ZPAL_LOG.
 */
void zpal_log(
  zpal_log_component_t component,
  zpal_log_level_t level,
  const char * pFormat,
  ...);

// This include directive is deliberately not placed near the top of the file
#include "zpal_log_internal.h" // NOSONAR

/**
 * Logs a message from a module to the appropriate channel.
 *
 * @param[in] COMPONENT The software component supplying the log message
 * @param[in] LEVEL The log level
 * @param[in] PFORMAT The log message (format string)
 * @param[in] ... The arguments to the format string
 */
#define ZPAL_LOG(COMPONENT, LEVEL, PFORMAT, ...) \
  ZPAL_LOG_COMPILE_CONDITIONALLY(COMPONENT, LEVEL, PFORMAT, ##__VA_ARGS__)

/**
 * Logs a message from a module to the debug channel.
 *
 * @param[in] COMPONENT The software component supplying the log message
 * @param[in] PFORMAT The log message (format string)
 * @param[in] ... The arguments to the format string
 */
#define ZPAL_LOG_DEBUG(COMPONENT, PFORMAT, ...) \
  ZPAL_LOG(COMPONENT, ZPAL_LOG_LEVEL_DEBUG, PFORMAT, ##__VA_ARGS__)

/**
 * Logs a message from a module to the info channel.
 *
 * @param[in] COMPONENT The software component supplying the log message
 * @param[in] PFORMAT The log message (format string)
 * @param[in] ... The arguments to the format string
 */
#define ZPAL_LOG_INFO(COMPONENT, PFORMAT, ...) \
  ZPAL_LOG(COMPONENT, ZPAL_LOG_LEVEL_INFO, PFORMAT, ##__VA_ARGS__)

/**
 * Logs a message from a module to the warning channel.
 *
 * @param[in] COMPONENT The software component supplying the log message
 * @param[in] PFORMAT The log message (format string)
 * @param[in] ... The arguments to the format string
 */
#define ZPAL_LOG_WARNING(COMPONENT, PFORMAT, ...) \
  ZPAL_LOG(COMPONENT, ZPAL_LOG_LEVEL_WARNING, PFORMAT, ##__VA_ARGS__)

/**
 * Logs a message from a module to the error channel.
 *
 * @param[in] COMPONENT The software component supplying the log message
 * @param[in] PFORMAT The log message (format string)
 * @param[in] ... The arguments to the format string
 */
#define ZPAL_LOG_ERROR(COMPONENT, PFORMAT, ...) \
  ZPAL_LOG(COMPONENT, ZPAL_LOG_LEVEL_ERROR, PFORMAT, ##__VA_ARGS__)

#if defined(__clang__)
#pragma clang diagnostic pop // "-Wgnu-zero-variadic-macro-arguments"
#endif // defined(__clang__)
#endif /* ZPAL_LOG_H */
