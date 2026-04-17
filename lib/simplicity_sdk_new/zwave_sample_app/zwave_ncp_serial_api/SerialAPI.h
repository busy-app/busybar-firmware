/**
 * @file
 * @copyright 2019 Silicon Laboratories Inc.
 */
#ifndef _ZW_SERIALAPI_H_
#define _ZW_SERIALAPI_H_

/**
 * @addtogroup ZWaveAPI
 * @{
 * @addtogroup ZWSerialAPI Z-Wave Serial API
 * @{
 */

#define SOF 0x01  ///< Start Of Frame
#define ACK 0x06  ///< Acknowledge successful frame reception
#define NAK 0x15  ///< Not Acknowledge successful frame reception - please retransmit...
#define CAN 0x18  ///< Frame received (from host) was dropped - waiting for ACK

///@addtogroup FT Frame Types
/// @{
#define REQUEST                                         0x00
#define RESPONSE                                        0x01
///@}

///@addtogroup GET_INIT_DATA_FLAG
/// Flags used in FUNC_ID_SERIAL_API_GET_INIT_DATA functionality
/// @{
#define GET_INIT_DATA_FLAG_SLAVE_API                    0x01
#define GET_INIT_DATA_FLAG_SECONDARY_CTRL               0x04
#define GET_INIT_DATA_FLAG_IS_SUC                       0x08
///@}

/**
 * @addtogroup FUNC_ID
 * Function IDs
 * @{
 */
#define FUNC_ID_SERIAL_API_GET_INIT_DATA                0x02
#define FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION        0x03
#define FUNC_ID_APPLICATION_COMMAND_HANDLER             0x04
#define FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES          0x05

/* SERIAL API ver 4 added - START */
#define FUNC_ID_SERIAL_API_SET_TIMEOUTS                 0x06
#define FUNC_ID_SERIAL_API_GET_CAPABILITIES             0x07
#define FUNC_ID_SERIAL_API_SOFT_RESET                   0x08
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_GET_PROTOCOL_VERSION                 0x09

/* Function ID for startup message */
#define FUNC_ID_SERIAL_API_STARTED                      0x0A
#define FUNC_ID_SERIAL_API_SETUP                        0x0B

#define FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION_CMD_CLASSES  0x0C // This define is never used

#define FUNC_ID_ZW_SEND_DATA_EX                         0x0E
#define FUNC_ID_ZW_SEND_DATA_MULTI_EX                   0x0F

#define FUNC_ID_ZW_SET_RF_RECEIVE_MODE                  0x10
#define FUNC_ID_ZW_SET_SLEEP_MODE                       0x11  // This define is never used
#define FUNC_ID_ZW_SEND_NODE_INFORMATION                0x12
#define FUNC_ID_ZW_SEND_DATA                            0x13
#define FUNC_ID_ZW_SEND_DATA_MULTI                      0x14
#define FUNC_ID_ZW_GET_VERSION                          0x15

/* SERIAL API ver 4 added - START */
#define FUNC_ID_ZW_SEND_DATA_ABORT                      0x16
// FUNC_ID_ZW_RF_POWER_LEVEL_SET is no longer supported
#define FUNC_ID_ZW_RF_POWER_LEVEL_SET                   0x17  // This define is never used (mentioned in the comment above)
#define FUNC_ID_ZW_SEND_DATA_META                       0x18  // This define is never used
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_RESERVED_SD                          0x19  // This define is never used
#define FUNC_ID_ZW_RESERVED_SDM                         0x1A  // This define is never used
#define FUNC_ID_ZW_RESERVED_SRI                         0x1B  // This define is never used

#define FUNC_ID_ZW_SET_ROUTING_INFO                     0x1B  // This define is never used

#define FUNC_ID_ZW_GET_RANDOM                           0x1C
#define FUNC_ID_ZW_RANDOM                               0x1D  // This define is never used
/* Specified but not yet implemented */
#define FUNC_ID_SET_RF_POWERLEVEL_REDISCOVERY           0x1E  // This define is never used (mentioned in the comment above)

#define FUNC_ID_MEMORY_GET_ID                           0x20
#define FUNC_ID_MEMORY_GET_BYTE                         0x21
#define FUNC_ID_MEMORY_PUT_BYTE                         0x22
#define FUNC_ID_MEMORY_GET_BUFFER                       0x23
#define FUNC_ID_MEMORY_PUT_BUFFER                       0x24
/* Unimplemented - START */
#define FUNC_ID_SERIAL_API_GET_APPL_HOST_MEMORY_OFFSET  0x25  // This define is never used (mentioned in the comment above)
#define FUNC_ID_DEBUG_OUTPUT                            0x26  // This define is never used (mentioned in the comment above)
/* Unimplemented - END */

#define FUNC_ID_AUTO_PROGRAMMING                        0x27

#define FUNC_ID_NVR_GET_VALUE                           0x28

#define FUNC_ID_NVM_GET_ID                              0x29
#define FUNC_ID_NVM_EXT_READ_LONG_BUFFER                0x2A
#define FUNC_ID_NVM_EXT_WRITE_LONG_BUFFER               0x2B  // This define is never used
#define FUNC_ID_NVM_EXT_READ_LONG_BYTE                  0x2C
#define FUNC_ID_NVM_EXT_WRITE_LONG_BYTE                 0x2D  // This define is never used
#define FUNC_ID_NVM_BACKUP_RESTORE                      0x2E

#define FUNC_ID_ZW_NVR_GET_APP_VALUE                    0x2F  // This define is never used

#define FUNC_ID_CLOCK_SET                               0x30  // This define is never used
#define FUNC_ID_CLOCK_GET                               0x31  // This define is never used
#define FUNC_ID_CLOCK_CMP                               0x32  // This define is never used
#define FUNC_ID_RTC_TIMER_CREATE                        0x33  // This define is never used
#define FUNC_ID_RTC_TIMER_READ                          0x34  // This define is never used
#define FUNC_ID_RTC_TIMER_DELETE                        0x35  // This define is never used
#define FUNC_ID_RTC_TIMER_CALL                          0x36  // This define is never used

#define FUNC_ID_CLEAR_TX_TIMERS                         0x37
#define FUNC_ID_GET_TX_TIMERS                           0x38

#define FUNC_ID_ZW_CLEAR_NETWORK_STATS                  0x39
#define FUNC_ID_ZW_GET_NETWORK_STATS                    0x3A
#define FUNC_ID_ZW_GET_BACKGROUND_RSSI                  0x3B
#define FUNC_ID_ZW_SET_LISTEN_BEFORE_TALK_THRESHOLD     0x3C
#define FUNC_ID_NVM_EXT_BACKUP_RESTORE                  0x3D
#define FUNC_ID_ZW_REMOVE_NODE_ID_FROM_NETWORK          0x3F

#define FUNC_ID_ZW_SET_LEARN_NODE_STATE                 0x40  // This define is never used
#define FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO               0x41
#define FUNC_ID_ZW_SET_DEFAULT                          0x42
#define FUNC_ID_ZW_NEW_CONTROLLER                       0x43  // This define is never used
// <Obsoleted>
#define FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE         0x44
#define FUNC_ID_ZW_REPLICATION_SEND_DATA                0x45
// </Obsoleted>
#define FUNC_ID_ZW_ASSIGN_RETURN_ROUTE                  0x46
#define FUNC_ID_ZW_DELETE_RETURN_ROUTE                  0x47
#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE         0x48
#define FUNC_ID_ZW_APPLICATION_UPDATE                   0x49

/** @deprecated Use ZW_APPLICATION_UPDATE */
#define FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE        0x49  // This define is never used (mentioned in the comment above)

#define FUNC_ID_ZW_ADD_NODE_TO_NETWORK                  0x4A
#define FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK             0x4B
/* Specified but not yet implemented */
#define FUNC_ID_ZW_ADD_CTRL_AND_ASSIGN_PRIMARY_CTRL     0x4C  // This define is never used (mentioned in the comment above)

#define FUNC_ID_ZW_CONTROLLER_CHANGE                    0x4D

#define FUNC_ID_ZW_RESERVED_FN                          0x4E  // This define is never used

/* Devkit 6.6x */
#define FUNC_ID_ZW_ASSIGN_PRIORITY_RETURN_ROUTE         0x4F

/* Slave only */
#define FUNC_ID_ZW_SET_LEARN_MODE                       0x50
/* Slave only end */

#define FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE              0x51
#define FUNC_ID_ZW_ENABLE_SUC                           0x52  // This define is never used
#define FUNC_ID_ZW_REQUEST_NETWORK_UPDATE               0x53
#define FUNC_ID_ZW_SET_SUC_NODE_ID                      0x54
#define FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE              0x55
#define FUNC_ID_ZW_GET_SUC_NODE_ID                      0x56
#define FUNC_ID_ZW_SEND_SUC_ID                          0x57

/* Devkit 6.6x */
#define FUNC_ID_ZW_ASSIGN_PRIORITY_SUC_RETURN_ROUTE     0x58

/* Obsolete - Not available in devkit 6.6x+ */
#define FUNC_ID_ZW_REDISCOVERY_NEEDED                   0x59  // This define is never used (mentioned in the comment above)

#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE_OPTION  0x5A  // This define is never used

/* Slave only */
#define FUNC_ID_ZW_SUPPORT9600_ONLY                     0x5B  // This define is never used
/* Slave only end */

/* Specified but not yet implemented */
#define FUNC_ID_ZW_REQUEST_NEW_ROUTE_DESTINATIONS       0x5C  // This define is never used (mentioned in the comment above)
#define FUNC_ID_ZW_IS_NODE_WITHIN_DIRECT_RANGE          0x5D  // This define is never used (mentioned in the comment above)
/* end */

#define FUNC_ID_ZW_EXPLORE_REQUEST_INCLUSION            0x5E
#define FUNC_ID_ZW_EXPLORE_REQUEST_EXCLUSION            0x5F

#define FUNC_ID_ZW_REQUEST_NODE_INFO                    0x60
#define FUNC_ID_ZW_REMOVE_FAILED_NODE_ID                0x61
#define FUNC_ID_ZW_IS_FAILED_NODE_ID                    0x62
#define FUNC_ID_ZW_REPLACE_FAILED_NODE                  0x63

/* In 6.0x the function id was wrong so we need to support this wrong function id as well in the future */
#define FUNC_ID_ZW_SET_ROUTING_MAX_6_00                 0x65
/* */

#define FUNC_ID_ZW_IS_PRIMARY_CTRL                      0x66  // This define is never used

#define FUNC_ID_ZW_AES_ECB                              0x67  // This define is never used

#define FUNC_ID_ZW_REQUEST_NODETYPE_NEIGHBOR_UPDATE     0x68

#define FUNC_ID_ZW_TRANSFER_PROTOCOL_CC                 0x69

#define FUNC_ID_ZW_ENABLE_NODE_NLS                      0x6A
#define FUNC_ID_ZW_GET_NODE_NLS_STATE                   0x6B

#define FUNC_ID_ZW_REQUEST_PROTOCOL_CC_ENCRYPTION       0x6C

#define FUNC_ID_TIMER_START                             0x70  // This define is never used
#define FUNC_ID_TIMER_RESTART                           0x71  // This define is never used
#define FUNC_ID_TIMER_CANCEL                            0x72  // This define is never used
#define FUNC_ID_TIMER_CALL                              0x73  // This define is never used

/* Firmware Update API */
#define FUNC_ID_ZW_FIRMWARE_UPDATE_NVM                  0x78  // This define is never used

/* Installer API */
#define FUNC_ID_GET_NEIGHBOR_TABLE_LINE                 0x80
/** @deprecated Use FUNC_ID_GET_NEIGHBOR_TABLE_LINE */
#define FUNC_ID_GET_ROUTING_TABLE_LINE                  FUNC_ID_GET_NEIGHBOR_TABLE_LINE
/** @deprecated */

#define FUNC_ID_GET_TX_COUNTER                          0x81
#define FUNC_ID_RESET_TX_COUNTER                        0x82
#define FUNC_ID_STORE_NODEINFO                          0x83  // This define is never used
#define FUNC_ID_STORE_HOMEID                            0x84  // This define is never used
/* Installer API only end */

#define FUNC_ID_LOCK_ROUTE_RESPONSE                     0x90

// ZW_ROUTING_DEMO is never used ...
#ifdef ZW_ROUTING_DEMO
/* Max hops in route */
#define TRANSMIT_ROUTED_ATTEMPT                         0x08  // This define is never used
#define FUNC_ID_ZW_SEND_DATA_ROUTE_DEMO                 0x91  // This define is never used
#endif

/* ZW_SetPriorityRoute/ZW_GetPriorityRoute replaces ZW_SetLastWorkingRoute/ZW_GetLastWorkingRoute */
#define FUNC_ID_ZW_GET_PRIORITY_ROUTE                   0x92
#define FUNC_ID_ZW_SET_PRIORITY_ROUTE                   0x93
/* NOTE: Obsoleted - BEGIN */
#define FUNC_ID_ZW_GET_LAST_WORKING_ROUTE               0x92  // This define is never used
#define FUNC_ID_ZW_SET_LAST_WORKING_ROUTE               0x93  // This define is never used
/* NOTE: Obsoleted - END */

#define FUNC_ID_SERIAL_API_TEST                         0x95  // This define is never used

#define FUNC_ID_SERIAL_API_EXT                          0x98

/* DevKit 6.7x introduced - slave_enhanced_232 and slave_routing only */
#define FUNC_ID_ZW_SECURITY_SETUP                       0x9C  // This define is never used outside of comments ...
#define FUNC_ID_APPLICATION_SECURITY_EVENT              0x9D  // This define is never used

/* ZW_CONTROLLER_BRIDGE only START */
#define FUNC_ID_SERIAL_API_APPL_SLAVE_NODE_INFORMATION  0xA0

/** @deprecated OBSOLETE: In DevKit 4.5x/6.0x Controller Bridge applications, this is obsoleted
 * by the FUNC_ID_APPLICATION_COMMAND_HANDLER_BRIDGE
 */
#define FUNC_ID_APPLICATION_SLAVE_COMMAND_HANDLER       0xA1  // This define is never used
#define FUNC_ID_ZW_SEND_SLAVE_NODE_INFORMATION          0xA2
#define FUNC_ID_ZW_SEND_SLAVE_DATA                      0xA3  // This define is never used
#define FUNC_ID_ZW_SET_SLAVE_LEARN_MODE                 0xA4
#define FUNC_ID_ZW_GET_VIRTUAL_NODES                    0xA5
#define FUNC_ID_ZW_IS_VIRTUAL_NODE                      0xA6
#define FUNC_ID_ZW_RESERVED_SSD                         0xA7  // This define is never used
/* DevKit 4.5x/6.0x added - obsoletes FUNC_ID_APPLICATION_SLAVE_COMMAND_HANDLER and */
/* FUNC_ID_APPLICATION_COMMAND_HANDLER for the Controller Bridge applications as */
/* this handles both cases - only for 4.5x/6.0x based Controller Bridge applications */
#define FUNC_ID_APPLICATION_COMMAND_HANDLER_BRIDGE      0xA8
/* DevKit 4.5x/6.0x added - Adds sourceNodeID to the parameter list */
#define FUNC_ID_ZW_SEND_DATA_BRIDGE                     0xA9
/* Obsolete */
#define FUNC_ID_ZW_SEND_DATA_META_BRIDGE                0xAA  // This define is never used
#define FUNC_ID_ZW_SEND_DATA_MULTI_BRIDGE               0xAB
/* ZW_CONTROLLER_BRIDGE only END */

#define FUNC_ID_PWR_SETSTOPMODE                         0xB0  // This define is never used (ZW102 only)
#define FUNC_ID_PWR_CLK_PD                              0xB1  // This define is never used (ZW102 only)
#define FUNC_ID_PWR_CLK_PUP                             0xB2  // This define is never used (ZW102 only)
#define FUNC_ID_PWR_SELECT_CLK                          0xB3  // This define is never used (ZW102 only)
#define FUNC_ID_ZW_SET_WUT_TIMEOUT                      0xB4  // This define is never used
#define FUNC_ID_ZW_IS_WUT_KICKED                        0xB5  // This define is never used (ZW102 only)

#define FUNC_ID_ZW_WATCHDOG_ENABLE                      0xB6  // This define is never used
#define FUNC_ID_ZW_WATCHDOG_DISABLE                     0xB7  // This define is never used
#define FUNC_ID_ZW_WATCHDOG_KICK                        0xB8  // This define is never used
/** @deprecated Use FUNC_ID_ZW_INT_EXT_LEVEL_SET */
#define FUNC_ID_ZW_SET_EXT_INT_LEVEL                    0xB9  // This define is never used (ZW102 only)
#define FUNC_ID_ZW_INT_EXT_LEVEL_SET                    0xB9  // This define is never used

// FUNC_ID_ZW_RF_POWER_LEVEL_GET is no longer supported
#define FUNC_ID_ZW_RF_POWER_LEVEL_GET                   0xBA  // This define is never used (mentioned in the comment above)
#define FUNC_ID_ZW_GET_NEIGHBOR_COUNT                   0xBB  // This define is never used
#define FUNC_ID_ZW_ARE_NODES_NEIGHBOURS                 0xBC  // This define is never used

#define FUNC_ID_ZW_TYPE_LIBRARY                         0xBD
#define FUNC_ID_ZW_SEND_TEST_FRAME                      0xBE
#define FUNC_ID_ZW_GET_PROTOCOL_STATUS                  0xBF

#define FUNC_ID_ZW_SEND_PROTOCOL_DATA                   0xAC

#define FUNC_ID_ZW_SET_PROMISCUOUS_MODE                 0xD0  // This define is never used
/* SERIAL API ver 5 added - START */
#define FUNC_ID_PROMISCUOUS_APPLICATION_COMMAND_HANDLER 0xD1
/* SERIAL API ver 5 added - END */

#define FUNC_ID_ZW_WATCHDOG_START                       0xD2
#define FUNC_ID_ZW_WATCHDOG_STOP                        0xD3

#define FUNC_ID_ZW_SET_ROUTING_MAX                      0xD4
/* Unimplemented - START */
/* Obsoleted */
#define FUNC_ID_ZW_GET_ROUTING_MAX                      0xD5  // This define is never used

#define FUNC_ID_POWER_MANAGEMENT_STAY_AWAKE             0xD7  // Power Management Stay Awake Command
#define FUNC_ID_POWER_MANAGEMENT_CANCEL                 0xD8  // Power Management Cancel Command
/* Unimplemented - END */

/* Allocated for setting the maximum number of 128sec ticks inbetween SmartStart inclusion requests. */
#define FUNC_ID_ZW_NETWORK_MANAGEMENT_SET_MAX_INCLUSION_REQUEST_INTERVALS 0xD6

#define FUNC_ID_ZW_INITIATE_SHUTDOWN                    0xD9

#define FUNC_ID_SERIAL_API_GET_LR_NODES                 0xDA

#define FUNC_ID_GET_LR_CHANNEL                          0xDB
#define FUNC_ID_SET_LR_CHANNEL                          0xDC

#define FUNC_ID_ZW_SET_LR_VIRTUAL_IDS                   0xDD

#define FUNC_ID_ZW_GET_NLS_NODES                        0xC0

#define FUNC_ID_GET_DCDC_CONFIG                         0xDE
#define FUNC_ID_SET_DCDC_CONFIG                         0xDF

/* Allocated for NUNIT test */
#define FUNC_ID_ZW_NUNIT_CMD                            0xE0 // This define is never used (DEPRECATED)
#define FUNC_ID_ZW_NUNIT_INIT                           0xE1 // This define is never used (DEPRECATED)
#define FUNC_ID_ZW_NUNIT_LIST                           0xE2 // This define is never used (DEPRECATED)
#define FUNC_ID_ZW_NUNIT_RUN                            0xE3 // This define is never used (DEPRECATED)
#define FUNC_ID_ZW_NUNIT_END                            0xE4 // This define is never used (DEPRECATED)

#define FUNC_ID_RADIO_DEBUG_GET_PROTOCOL_LIST           0xE6  // Radio Debug Get Protocol List Command
#define FUNC_ID_RADIO_DEBUG_ENABLE                      0xE7  // Radio Debug Enable Command
#define FUNC_ID_RADIO_DEBUG_STATUS                      0xE8  // Radio Debug Status Command

/* Specified but not yet implemented */
#define FUNC_ID_SEND_NOP                                0xE9  // This define is never used (mentioned in the comment above)

/* Allocated for Power Management */
#define FUNC_ID_SERIAL_API_POWER_MANAGEMENT             0xEE  // This define is never used
#define FUNC_ID_SERIAL_API_READY                        0xEF  // This define is never used

/* Allocated for proprietary serial API commands */
#define FUNC_ID_PROPRIETARY_0                           0xF0
#define FUNC_ID_PROPRIETARY_1                           0xF1
#define FUNC_ID_PROPRIETARY_2                           0xF2
#define FUNC_ID_PROPRIETARY_3                           0xF3
#define FUNC_ID_PROPRIETARY_4                           0xF4
#define FUNC_ID_PROPRIETARY_5                           0xF5
#define FUNC_ID_PROPRIETARY_6                           0xF6
#define FUNC_ID_PROPRIETARY_7                           0xF7
#define FUNC_ID_PROPRIETARY_8                           0xF8
#define FUNC_ID_PROPRIETARY_9                           0xF9
#define FUNC_ID_PROPRIETARY_A                           0xFA
#define FUNC_ID_PROPRIETARY_B                           0xFB
#define FUNC_ID_PROPRIETARY_C                           0xFC
#define FUNC_ID_PROPRIETARY_D                           0xFD
#define FUNC_ID_PROPRIETARY_E                           0xFE

/* Illegal function ID */
#define FUNC_ID_UNKNOWN                                 0xFF
///@}

/**size of the address field of NVM_BACKUP_RESTORE or NVM_EXT_BACKUP_RESTORE frames.
   Item value is the size in byte of the address field.
 */
typedef enum {
  NVM_BACKUP_RESTORE_ADDR_SIZE = 2,     ///< 2 bytes for the legacy NVM backup & restore command
  NVM_EXT_BACKUP_RESTORE_ADDR_SIZE = 4, ///< 4 bytes for the extended NVM backup & restore command
} nvm_backup_restore_addr_size_t;

/**
 * @brief Power Lock Type enumeration
 */
typedef enum {
  POWER_LOCK_TYPE_RADIO = 0,  /**< Radio power lock */
  POWER_LOCK_TYPE_IO = 1,     /**< IO power lock (requires Shutdown Manager) */
} power_lock_type_t;

///@}
///@}

#endif /*_ZW_SERIALAPI_H_*/
