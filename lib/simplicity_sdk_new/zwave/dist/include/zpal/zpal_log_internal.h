/**
 * @file
 * @brief Internal header file for the Z-Wave Platform Abstraction Layer logging.
 *
 * @details This file contains macros that enable seamless usage of the ZPAL_LOG
 * API. It provides a way to conditionally compile log messages based on the
 * log level and the component from which the log message originates.
 * In addition, it provides special logging macros intentded only for temporary
 * debugging purposes.
 *
 * @copyright 2025 Silicon Laboratories Inc.
 */

#ifndef ZPAL_LOG_INTERNAL_H_
#define ZPAL_LOG_INTERNAL_H_

#include "zw_log_config.h"
#include <assert.h>

// Logging is not available for unit tests.
#if defined(UNIT_TEST)
#define ZPAL_LOG_COMPILE_CONDITIONALLY(COMPONENT, LEVEL, PFORMAT, ...)
#else // defined(UNIT_TEST)

// Helper function to determine if a log level is enabled
#define ZPAL_LOG_FILTER_LEVEL(LEVEL) (                                            \
    ((LEVEL == ZPAL_LOG_LEVEL_DEBUG)      && (ZW_LOG_CHANNEL_DEBUG[0] != '\0'))   \
    || ((LEVEL == ZPAL_LOG_LEVEL_INFO)    && (ZW_LOG_CHANNEL_INFO[0] != '\0'))    \
    || ((LEVEL == ZPAL_LOG_LEVEL_WARNING) && (ZW_LOG_CHANNEL_WARNING[0] != '\0')) \
    || ((LEVEL == ZPAL_LOG_LEVEL_ERROR)   && (ZW_LOG_CHANNEL_ERROR[0] != '\0'))   \
    )

/**
 * The helper macros below are used to determine if logging of a specific
 * component is enabled.
 * They provide a definition for each component, even if they are not explicitly
 * disabled in the configuration. This prevents compile time errors that would
 * otherwise arise from referencing undefined macros.
 */

// Helper macro to determine if logging of component app is enabled
#if defined(ZW_LOG_ENABLE_APP) && (ZW_LOG_ENABLE_APP != 0)
#define ZW_LOG_APP_ENABLED 1
#else
#define ZW_LOG_APP_ENABLED 0
#endif

// Helper macro to determine if logging of component hw is enabled
#if defined(ZW_LOG_ENABLE_HW) && (ZW_LOG_ENABLE_HW != 0)
#define ZW_LOG_HW_ENABLED 1
#else
#define ZW_LOG_HW_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_association is enabled
#if defined(ZW_LOG_ENABLE_CC_ASSOCIATION) && (ZW_LOG_ENABLE_CC_ASSOCIATION != 0)
#define ZW_LOG_CC_ASSOCIATION_ENABLED 1
#else
#define ZW_LOG_CC_ASSOCIATION_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_battery is enabled
#if defined(ZW_LOG_ENABLE_CC_BATTERY) && (ZW_LOG_ENABLE_CC_BATTERY != 0)
#define ZW_LOG_CC_BATTERY_ENABLED 1
#else
#define ZW_LOG_CC_BATTERY_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_binary_switch is enabled
#if defined(ZW_LOG_ENABLE_CC_BINARY_SWITCH) && (ZW_LOG_ENABLE_CC_BINARY_SWITCH != 0)
#define ZW_LOG_CC_BINARY_SWITCH_ENABLED 1
#else
#define ZW_LOG_CC_BINARY_SWITCH_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_central_scene is enabled
#if defined(ZW_LOG_ENABLE_CC_CENTRAL_SCENE) && (ZW_LOG_ENABLE_CC_CENTRAL_SCENE != 0)
#define ZW_LOG_CC_CENTRAL_SCENE_ENABLED 1
#else
#define ZW_LOG_CC_CENTRAL_SCENE_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_color_switch is enabled
#if defined(ZW_LOG_ENABLE_CC_COLOR_SWITCH) && (ZW_LOG_ENABLE_CC_COLOR_SWITCH != 0)
#define ZW_LOG_CC_COLOR_SWITCH_ENABLED 1
#else
#define ZW_LOG_CC_COLOR_SWITCH_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_door_lock is enabled
#if defined(ZW_LOG_ENABLE_CC_DOOR_LOCK) && (ZW_LOG_ENABLE_CC_DOOR_LOCK != 0)
#define ZW_LOG_CC_DOOR_LOCK_ENABLED 1
#else
#define ZW_LOG_CC_DOOR_LOCK_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_firmware_update is enabled
#if defined(ZW_LOG_ENABLE_CC_FIRMWARE_UPDATE) && (ZW_LOG_ENABLE_CC_FIRMWARE_UPDATE != 0)
#define ZW_LOG_CC_FIRMWARE_UPDATE_ENABLED 1
#else
#define ZW_LOG_CC_FIRMWARE_UPDATE_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_indicator is enabled
#if defined(ZW_LOG_ENABLE_CC_INDICATOR) && (ZW_LOG_ENABLE_CC_INDICATOR != 0)
#define ZW_LOG_CC_INDICATOR_ENABLED 1
#else
#define ZW_LOG_CC_INDICATOR_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_multi_channel is enabled
#if defined(ZW_LOG_ENABLE_CC_MULTI_CHANNEL) && (ZW_LOG_ENABLE_CC_MULTI_CHANNEL != 0)
#define ZW_LOG_CC_MULTI_CHANNEL_ENABLED 1
#else
#define ZW_LOG_CC_MULTI_CHANNEL_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_multilevel_sensor is enabled
#if defined(ZW_LOG_ENABLE_CC_MULTILEVEL_SENSOR) && (ZW_LOG_ENABLE_CC_MULTILEVEL_SENSOR != 0)
#define ZW_LOG_CC_MULTILEVEL_SENSOR_ENABLED 1
#else
#define ZW_LOG_CC_MULTILEVEL_SENSOR_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_multilevel_switch is enabled
#if defined(ZW_LOG_ENABLE_CC_MULTILEVEL_SWITCH) && (ZW_LOG_ENABLE_CC_MULTILEVEL_SWITCH != 0)
#define ZW_LOG_CC_MULTILEVEL_SWITCH_ENABLED 1
#else
#define ZW_LOG_CC_MULTILEVEL_SWITCH_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_notification is enabled
#if defined(ZW_LOG_ENABLE_CC_NOTIFICATION) && (ZW_LOG_ENABLE_CC_NOTIFICATION != 0)
#define ZW_LOG_CC_NOTIFICATION_ENABLED 1
#else
#define ZW_LOG_CC_NOTIFICATION_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_user_code is enabled
#if defined(ZW_LOG_ENABLE_CC_USER_CODE) && (ZW_LOG_ENABLE_CC_USER_CODE != 0)
#define ZW_LOG_CC_USER_CODE_ENABLED 1
#else
#define ZW_LOG_CC_USER_CODE_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_user_credential is enabled
#if defined(ZW_LOG_ENABLE_CC_USER_CREDENTIAL) && (ZW_LOG_ENABLE_CC_USER_CREDENTIAL != 0)
#define ZW_LOG_CC_USER_CREDENTIAL_ENABLED 1
#else
#define ZW_LOG_CC_USER_CREDENTIAL_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_version is enabled
#if defined(ZW_LOG_ENABLE_CC_VERSION) && (ZW_LOG_ENABLE_CC_VERSION != 0)
#define ZW_LOG_CC_VERSION_ENABLED 1
#else
#define ZW_LOG_CC_VERSION_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_wake_up is enabled
#if defined(ZW_LOG_ENABLE_CC_WAKE_UP) && (ZW_LOG_ENABLE_CC_WAKE_UP != 0)
#define ZW_LOG_CC_WAKE_UP_ENABLED 1
#else
#define ZW_LOG_CC_WAKE_UP_ENABLED 0
#endif

// Helper macro to determine if logging of component cc_zwave_plus_info is enabled
#if defined(ZW_LOG_ENABLE_CC_ZWAVE_PLUS_INFO) && (ZW_LOG_ENABLE_CC_ZWAVE_PLUS_INFO != 0)
#define ZW_LOG_CC_ZWAVE_PLUS_INFO_ENABLED 1
#else
#define ZW_LOG_CC_ZWAVE_PLUS_INFO_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_actuator is enabled
#if defined(ZW_LOG_ENABLE_ZAF_ACTUATOR) && (ZW_LOG_ENABLE_ZAF_ACTUATOR != 0)
#define ZW_LOG_ZAF_ACTUATOR_ENABLED 1
#else
#define ZW_LOG_ZAF_ACTUATOR_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_app_timer is enabled
#if defined(ZW_LOG_ENABLE_ZAF_APP_TIMER) && (ZW_LOG_ENABLE_ZAF_APP_TIMER != 0)
#define ZW_LOG_ZAF_APP_TIMER_ENABLED 1
#else
#define ZW_LOG_ZAF_APP_TIMER_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_cmd_publisher is enabled
#if defined(ZW_LOG_ENABLE_ZAF_CMD_PUBLISHER) && (ZW_LOG_ENABLE_ZAF_CMD_PUBLISHER != 0)
#define ZW_LOG_ZAF_CMD_PUBLISHER_ENABLED 1
#else
#define ZW_LOG_ZAF_CMD_PUBLISHER_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_common is enabled
#if defined(ZW_LOG_ENABLE_ZAF_COMMON) && (ZW_LOG_ENABLE_ZAF_COMMON != 0)
#define ZW_LOG_ZAF_COMMON_ENABLED 1
#else
#define ZW_LOG_ZAF_COMMON_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_event_distributor is enabled
#if defined(ZW_LOG_ENABLE_ZAF_EVENT_DISTRIBUTOR) && (ZW_LOG_ENABLE_ZAF_EVENT_DISTRIBUTOR != 0)
#define ZW_LOG_ZAF_EVENT_DISTRIBUTOR_ENABLED 1
#else
#define ZW_LOG_ZAF_EVENT_DISTRIBUTOR_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_nvm is enabled
#if defined(ZW_LOG_ENABLE_ZAF_NVM) && (ZW_LOG_ENABLE_ZAF_NVM != 0)
#define ZW_LOG_ZAF_NVM_ENABLED 1
#else
#define ZW_LOG_ZAF_NVM_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_transport is enabled
#if defined(ZW_LOG_ENABLE_ZAF_TRANSPORT) && (ZW_LOG_ENABLE_ZAF_TRANSPORT != 0)
#define ZW_LOG_ZAF_TRANSPORT_ENABLED 1
#else
#define ZW_LOG_ZAF_TRANSPORT_ENABLED 0
#endif

// Helper macro to determine if logging of component zaf_tse is enabled
#if defined(ZW_LOG_ENABLE_ZAF_TSE) && (ZW_LOG_ENABLE_ZAF_TSE != 0)
#define ZW_LOG_ZAF_TSE_ENABLED 1
#else
#define ZW_LOG_ZAF_TSE_ENABLED 0
#endif

// Helper macro to determine if logging of component zw_shutdown_manager is enabled
#if defined(ZW_LOG_ENABLE_SHUTDOWN_MANAGER) && (ZW_LOG_ENABLE_SHUTDOWN_MANAGER != 0)
#define ZW_LOG_SHUTDOWN_MANAGER_ENABLED 1
#else
#define ZW_LOG_SHUTDOWN_MANAGER_ENABLED 0
#endif

#define ZPAL_LOG_COMPILE_CONDITIONALLY(COMPONENT, LEVEL, PFORMAT, ...)                                 \
  do {                                                                                                 \
    if (ZPAL_LOG_FILTER_LEVEL(LEVEL)                                                                   \
        && (                                                                                           \
          ((COMPONENT == ZPAL_LOG_APP) && (ZW_LOG_APP_ENABLED))                                        \
          || ((COMPONENT == ZPAL_LOG_HW) && (ZW_LOG_HW_ENABLED))                                       \
          || ((COMPONENT == ZPAL_LOG_CC_ASSOCIATION) && (ZW_LOG_CC_ASSOCIATION_ENABLED))               \
          || ((COMPONENT == ZPAL_LOG_CC_BATTERY) && (ZW_LOG_CC_BATTERY_ENABLED))                       \
          || ((COMPONENT == ZPAL_LOG_CC_BINARY_SWITCH) && (ZW_LOG_CC_BINARY_SWITCH_ENABLED))           \
          || ((COMPONENT == ZPAL_LOG_CC_CENTRAL_SCENE) && (ZW_LOG_CC_CENTRAL_SCENE_ENABLED))           \
          || ((COMPONENT == ZPAL_LOG_CC_COLOR_SWITCH) && (ZW_LOG_CC_COLOR_SWITCH_ENABLED))             \
          || ((COMPONENT == ZPAL_LOG_CC_DOOR_LOCK) && (ZW_LOG_CC_DOOR_LOCK_ENABLED))                   \
          || ((COMPONENT == ZPAL_LOG_CC_FIRMWARE_UPDATE) && (ZW_LOG_CC_FIRMWARE_UPDATE_ENABLED))       \
          || ((COMPONENT == ZPAL_LOG_CC_INDICATOR) && (ZW_LOG_CC_INDICATOR_ENABLED))                   \
          || ((COMPONENT == ZPAL_LOG_CC_MULTI_CHANNEL) && (ZW_LOG_CC_MULTI_CHANNEL_ENABLED))           \
          || ((COMPONENT == ZPAL_LOG_CC_MULTILEVEL_SENSOR) && (ZW_LOG_CC_MULTILEVEL_SENSOR_ENABLED))   \
          || ((COMPONENT == ZPAL_LOG_CC_MULTILEVEL_SWITCH) && (ZW_LOG_CC_MULTILEVEL_SWITCH_ENABLED))   \
          || ((COMPONENT == ZPAL_LOG_CC_NOTIFICATION) && (ZW_LOG_CC_NOTIFICATION_ENABLED))             \
          || ((COMPONENT == ZPAL_LOG_CC_USER_CODE) && (ZW_LOG_CC_USER_CODE_ENABLED))                   \
          || ((COMPONENT == ZPAL_LOG_CC_USER_CREDENTIAL) && (ZW_LOG_CC_USER_CREDENTIAL_ENABLED))       \
          || ((COMPONENT == ZPAL_LOG_CC_VERSION) && (ZW_LOG_CC_VERSION_ENABLED))                       \
          || ((COMPONENT == ZPAL_LOG_CC_WAKE_UP) && (ZW_LOG_CC_WAKE_UP_ENABLED))                       \
          || ((COMPONENT == ZPAL_LOG_CC_ZWAVE_PLUS_INFO) && (ZW_LOG_CC_ZWAVE_PLUS_INFO_ENABLED))       \
          || ((COMPONENT == ZPAL_LOG_ZAF_ACTUATOR) && (ZW_LOG_ZAF_ACTUATOR_ENABLED))                   \
          || ((COMPONENT == ZPAL_LOG_ZAF_APP_TIMER) && (ZW_LOG_ZAF_APP_TIMER_ENABLED))                 \
          || ((COMPONENT == ZPAL_LOG_ZAF_CMD_PUBLISHER) && (ZW_LOG_ZAF_CMD_PUBLISHER_ENABLED))         \
          || ((COMPONENT == ZPAL_LOG_ZAF_COMMON) && (ZW_LOG_ZAF_COMMON_ENABLED))                       \
          || ((COMPONENT == ZPAL_LOG_ZAF_EVENT_DISTRIBUTOR) && (ZW_LOG_ZAF_EVENT_DISTRIBUTOR_ENABLED)) \
          || ((COMPONENT == ZPAL_LOG_ZAF_NVM) && (ZW_LOG_ZAF_NVM_ENABLED))                             \
          || ((COMPONENT == ZPAL_LOG_ZAF_TRANSPORT) && (ZW_LOG_ZAF_TRANSPORT_ENABLED))                 \
          || ((COMPONENT == ZPAL_LOG_ZAF_TSE) && (ZW_LOG_ZAF_TSE_ENABLED))                             \
          || ((COMPONENT == ZPAL_LOG_SHUTDOWN_MANAGER) && (ZW_LOG_SHUTDOWN_MANAGER_ENABLED))           \
          )) {                                                                                         \
      zpal_log(COMPONENT, LEVEL, PFORMAT, ##__VA_ARGS__);                                              \
    }                                                                                                  \
  } while (0)

#endif // !defined(UNIT_TEST)

#if !defined(NDEBUG)
/**
 * Logs a temporary error message, intended only for ad-hoc debugging.
 *
 * @param[in] PFORMAT The log message (format string)
 * @param[in] ... The arguments to the format string
 */
#define ZPAL_LOG_PRINT(PFORMAT, ...)                                       \
  do {                                                                     \
    zpal_log(ZPAL_LOG_NONE, ZPAL_LOG_LEVEL_DEBUG, PFORMAT, ##__VA_ARGS__); \
  } while (0)
#else // !defined(NDEBUG)
#define ZPAL_LOG_PRINT(PFORMAT, ...) \
  _Static_assert(false, "STATIC_ASSERT_FAILED_Ad_hoc_log_message_was_left_in_production_code");
#endif // !defined(NDEBUG)

#endif // ZPAL_LOG_INTERNAL_H_
