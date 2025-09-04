#ifndef SL_MATTER_CONFIG_H
#define SL_MATTER_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_MATTER_STACK_LOCK_TRACKING_MODE> Stack Lock Tracking Mode
// <SL_MATTER_STACK_LOCK_TRACKING_NONE=> None
// <SL_MATTER_STACK_LOCK_TRACKING_LOG=> Log
// <SL_MATTER_STACK_LOCK_TRACKING_FATAL=> Fatal
// <i> Default: SL_MATTER_STACK_LOCK_TRACKING_FATAL
#define SL_MATTER_STACK_LOCK_TRACKING_MODE SL_MATTER_STACK_LOCK_TRACKING_FATAL

// <o SL_MATTER_LOG_LEVEL> Log Level
// <SL_MATTER_LOG_NONE=> None
// <SL_MATTER_LOG_ERROR=> Error
// <SL_MATTER_LOG_PROGRESS=> Progress
// <SL_MATTER_LOG_DETAIL=> Detailed log (debug)
// <SL_MATTER_LOG_AUTOMATION=> Automation
// <i> Default: SL_MATTER_LOG_DETAIL
#ifndef SL_MATTER_LOG_LEVEL
#define SL_MATTER_LOG_LEVEL SL_MATTER_LOG_DETAIL
#endif

// <q SILABS_LOG_ENABLED> Enable Silabs specific log used in matter
// <i> Default: 1
#ifndef SILABS_LOG_ENABLED
#define SILABS_LOG_ENABLED 1
#endif

// <q HARD_FAULT_LOG_ENABLE> Enable hard fault logging
// <i> Default: 1
#ifndef HARD_FAULT_LOG_ENABLE
#define HARD_FAULT_LOG_ENABLE 1
#endif

// <o CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION> Device software version
// <i> Default: 1
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 1

// <s.128 CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING> Device software version string
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1"

// <o CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION> Device hardware version
// <i> Default: 1
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION 1

// <q SL_MATTER_CLI_ARG_PARSER> Enable CLI Argument Parser
// <i> Default: 1
#ifndef SL_MATTER_CLI_ARG_PARSER
#define SL_MATTER_CLI_ARG_PARSER 1
#endif

// <o CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES> Define the default number of ip addresses to discover
// <i> Default: 5
#define CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES 5

// <o KVS_MAX_ENTRIES> Maximum amount of KVS Entries
// <i> Default: 255
#define KVS_MAX_ENTRIES 255

// <q CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED> Synchronous Reports
// <i> Default: 0
#define CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED 1

// <o SL_MATTER_DEFERRED_ATTRIBUTE_STORE_DELAY_MS> Delay before the deferred attribute are stored in nvm
// <i> Default: 2000
#ifndef SL_MATTER_DEFERRED_ATTRIBUTE_STORE_DELAY_MS
#define SL_MATTER_DEFERRED_ATTRIBUTE_STORE_DELAY_MS 2000
#endif

// <<< end of configuration section >>>

#endif // SL_MATTER_CONFIG_H
