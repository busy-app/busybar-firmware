/***************************************************************************//**
 * @file
 * @brief This file contains the type definitions for RAIL structures, enums,
 *   and other types.
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

#ifndef SL_RAIL_TYPES_H
#define SL_RAIL_TYPES_H

// Include standard type headers to help define structures
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/// The RAIL library does not use enumerations because the ARM EABI leaves their
/// size ambiguous, which causes problems if the application is built
/// with different flags than the library. Instead, uint8_t typedefs
/// are used in compiled code for all enumerations. For documentation purposes, this is
/// converted to an actual enumeration since it's much easier to read in Doxygen.
#define SLI_RAIL_ENUM(name) enum name
/// This macro is a more generic version of the \ref SLI_RAIL_ENUM() macro that
/// allows the size of the type to be overridden instead of forcing the use of
/// a uint8_t. See \ref SLI_RAIL_ENUM() for more information.
#define SLI_RAIL_ENUM_GENERIC(name, type) enum name
#else//!DOXYGEN_SHOULD_SKIP_THIS
/// Define used for the RAIL library, which sets each enumeration to a uint8_t
/// typedef and creates a named enumeration structure for the enumeration values.
/// For debugging, use the following define to turn this back into a proper enumeration
/// #define SLI_RAIL_ENUM(name) typedef enum name##_enum name; enum name##_enum
#define SLI_RAIL_ENUM(name) typedef uint8_t name; enum name##_enum
#define SLI_RAIL_ENUM_GENERIC(name, type) typedef type name; enum name##_enum
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @addtogroup RAIL_API
 * @{
 */

/******************************************************************************
 * General Structures
 *****************************************************************************/
/**
 * @addtogroup General
 * @{
 */

/**
 * @struct sl_rail_version_t
 * @brief Contains RAIL Library Version Information.
 *   It is filled in by \ref sl_rail_get_version().
 */
typedef struct sl_rail_version {
  /**
   * Git hash.
   */
  uint32_t hash;
  /**
   * Major number.
   */
  uint8_t major;
  /**
   * Minor number.
   */
  uint8_t minor;
  /**
   * Revision number.
   */
  uint8_t rev;
  /**
   * Build number.
   */
  uint8_t build;
  /**
   * Build flags.
   */
  uint8_t flags;
  /**
   * Boolean to indicate whether this is a multiprotocol library or not.
   */
  bool multiprotocol;
} sl_rail_version_t;

/**
 * @typedef sl_rail_handle_t
 * @brief A radio-generic handle (e.g., \ref SL_RAIL_EFR32_HANDLE),
 *   or a real RAIL instance handle as returned from \ref sl_rail_init().
 *
 * Generic-radio handles must be used for certain RAIL APIs that are called
 * prior to RAIL initialization. However, once RAIL has been initialized,
 * the real handle returned by \ref sl_rail_init() should be used instead.
 */
typedef void *sl_rail_handle_t;

/**
 * A generic-radio RAIL handle for the primary EFR32 radio.
 * Using NULL as a RAIL handle is not recommended.
 * As a result, this value that can't be de-referenced is used.
 *
 * Generic radio handles must be used for RAIL APIs that are called
 * prior to RAIL initialization, and passed into \ref sl_rail_init().
 */
#define SL_RAIL_EFR32_HANDLE ((sl_rail_handle_t)0xFFFFFFFFUL)

/**
 * @typedef sl_rail_status_t
 * @brief A status returned by many RAIL API calls indicating their success or
 *   failure. It is a subset of sl_status_t.
 */
typedef sl_status_t sl_rail_status_t;

/**
 * RAIL function reports no error.
 */
#define SL_RAIL_STATUS_NO_ERROR SL_STATUS_OK // 0x0000

/**
 * Call to RAIL function threw an error because of an invalid parameter.
 */
#define SL_RAIL_STATUS_INVALID_PARAMETER SL_STATUS_INVALID_PARAMETER // 0x0021

/**
 * Call to RAIL function threw an error because it was called during
 * an invalid radio state.
 */
#define SL_RAIL_STATUS_INVALID_STATE SL_STATUS_INVALID_STATE // 0x0002

/**
 * RAIL function is called in an invalid order.
 */
#define SL_RAIL_STATUS_INVALID_CALL SL_STATUS_NOT_AVAILABLE // 0x000E

/**
 * RAIL function did not finish in the allotted time.
 */
#define SL_RAIL_STATUS_SUSPENDED SL_STATUS_IN_PROGRESS // 0x0005

/**
 * RAIL function could not be scheduled by the Radio scheduler.
 * Only issued when using a Multiprotocol application.
 */
#define SL_RAIL_STATUS_SCHED_ERROR SL_STATUS_ABORT // 0x0006

/**
 * RAIL function could not allocate needed resource(s).
 */
#define SL_RAIL_STATUS_NO_MORE_RESOURCE SL_STATUS_NO_MORE_RESOURCE // 0x001A

/**
 * A pointer to an initialization complete callback function.
 *
 * @param[in] rail_handle The initialized RAIL instance handle.
 */
typedef void (*sl_rail_init_complete_callback_t)(sl_rail_handle_t rail_handle);

/**
 * A value to signal that RAIL should not use DMA.
 */
#define SL_RAIL_DMA_INVALID (0xFFU)

/**
 * @typedef sl_rail_opaque_value_t
 * @brief An opaque type not examined by RAIL; can be used for application-
 *  specific purposes.
 */
typedef uint32_t sl_rail_opaque_value_t[1];

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * A linked list structure for RAIL state buffers which \ref sl_rail_init()
 * utilizes for managing internal RAIL state.
 */
typedef struct sl_rail_state_buffer_entry {
  /**
   * A pointer to the next stae buffer entry in the linked list.
   */
  struct sl_rail_state_buffer_entry *p_next;
  /**
   * Size of the state buffer in bytes.
   */
  uint32_t buffer_bytes;
  /**
   * A pointer to the state buffer in RAM.
   */
  uint64_t *p_buffer;
} sl_rail_state_buffer_entry_t;

#endif//DOXYGEN_UNDOCUMENTED

/** @} */ // end of group General

/******************************************************************************
 * System Timing Structures
 *****************************************************************************/
/**
 * @addtogroup System_Timing
 * @{
 */

/**
 * @typedef sl_rail_time_t
 * @brief Time in microseconds
 */
typedef uint32_t sl_rail_time_t;

/**
 * A pointer to the callback called when the RAIL timer expires.
 *
 * @param[in] rail_handle The RAIL handle that was used in the
 *   \ref sl_rail_set_timer() call.
 */
typedef void (*sl_rail_timer_callback_t)(sl_rail_handle_t rail_handle);

/**
 * @enum sl_rail_time_mode_t
 * @brief Specify a time offset in RAIL APIs.
 *
 * Different APIs use the same constants and may provide more specifics about
 * how they're used but the general use for each is described below.
 */
SLI_RAIL_ENUM(sl_rail_time_mode_t) {
  /**
   * The time specified is an exact time in the RAIL timebase. The given
   * event should happen at exactly that time. If this time is already in the
   * past, an error is returned. Because the RAIL timebase wraps at 32
   * bits, there is no real 'past'. Instead, any event greater than
   * 3/4 of the way into the future is considered to be in the past.
   */
  SL_RAIL_TIME_ABSOLUTE = 0u,
  /**
   * The time specified is relative to the current time. The event should occur
   * that many ticks in the future. Delays are only guaranteed at least as long
   * as the value specified. An overhead may occur between the time when the
   * API is called and when the delay starts. As a result, using this for
   * operations that must happen at an exact given time is not recommended.
   * For that, you must use \ref SL_RAIL_TIME_ABSOLUTE delays.
   *
   * Note that, if you specify a delay 0, that event is triggered as soon as
   * possible. This is different than specifying an absolute time of now which
   * would return an error unless it was possible.
   */
  SL_RAIL_TIME_DELAY = 1u,
  /**
   * The specified time is invalid and should be ignored. For some APIs this
   * can also indicate that any previously stored delay should be invalidated
   * and disabled.
   */
  SL_RAIL_TIME_DISABLED = 2u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_TIME_ABSOLUTE ((sl_rail_time_mode_t) SL_RAIL_TIME_ABSOLUTE)
#define SL_RAIL_TIME_DELAY    ((sl_rail_time_mode_t) SL_RAIL_TIME_DELAY)
#define SL_RAIL_TIME_DISABLED ((sl_rail_time_mode_t) SL_RAIL_TIME_DISABLED)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_multi_timer
 * Forward structure declaration of \ref sl_rail_multi_timer_t.
 */
struct sl_rail_multi_timer;

/**
 * @typedef sl_rail_multi_timer_callback_t
 * @brief Callback fired when timer expires.
 *
 * @param[in] p_tmr A pointer to an expired timer.
 * @param[in] expected_time_of_event An absolute time event fired.
 * @param[in] cb_arg A user-supplied callback argument.
 *   Since this callback doesn't include a parameter for the \ref
 *   sl_rail_handle_t passed into \ref sl_rail_set_multi_timer() it might
 *   be handy to also pass that RAIL handle as the cb_arg when calling
 *   \ref sl_rail_set_multi_timer().
 */
typedef void (*sl_rail_multi_timer_callback_t)(struct sl_rail_multi_timer *p_tmr,
                                               sl_rail_time_t expected_time_of_event,
                                               void *cb_arg);

/**
 * @struct sl_rail_multi_timer_t
 * @brief RAIL timer state structure.
 *
 * This structure is filled out and maintained internally only.
 * The user/application should not alter any elements of this structure.
 */
typedef struct sl_rail_multi_timer {
  /**
   * Absolute time before the next event.
   */
  sl_rail_time_t abs_offset;
  /**
   * Relative, periodic time between events; 0 = timer is oneshot.
   */
  sl_rail_time_t rel_periodic;
  /**
   * A user callback.
   */
  sl_rail_multi_timer_callback_t callback;
  /**
   * A user callback argument.
   */
  void *cb_arg;
  /**
   * A pointer to the next soft timer structure.
   */
  struct sl_rail_multi_timer *next;
  /**
   * A priority of the callback; 0 = highest priority; 255 = lowest.
   */
  uint8_t priority;
  /**
   * Indicates the timer is currently running.
   */
  bool is_running;
  /**
   * Indicates the callback needs to run.
   */
  bool do_callback;
} sl_rail_multi_timer_t;

/**
 * @enum sl_rail_packet_time_position_t
 * @brief The available packet time stamp position choices.
 */
SLI_RAIL_ENUM(sl_rail_packet_time_position_t) {
  /**
   * Indicate that a time stamp is not to be or was not provided.
   * It is useful if the application doesn't care about packet time stamps
   * and doesn't want RAIL to spend time calculating one.
   */
  SL_RAIL_PACKET_TIME_INVALID = 0u,
  /**
   * Request the choice most expedient for RAIL to calculate,
   * which may depend on the radio and/or its configuration.
   * The actual choice would always be reflected in the \ref
   * sl_rail_packet_time_stamp_t::time_position field of the \ref
   * sl_rail_rx_packet_details_t::time_received or \ref
   * sl_rail_tx_packet_details_t::time_sent
   * returned and would never be one of the _USED_TOTAL values.
   */
  SL_RAIL_PACKET_TIME_DEFAULT = 1u,
  /**
   * Request the time stamp corresponding to the first preamble bit
   * sent or received.
   * Indicate that time stamp did not require using total_packet_bytes.
   */
  SL_RAIL_PACKET_TIME_AT_PREAMBLE_START = 2u,
  /**
   * Request the time stamp corresponding to the first preamble bit
   * sent or received.
   * Indicate that time stamp did require using total_packet_bytes.
   */
  SL_RAIL_PACKET_TIME_AT_PREAMBLE_START_USED_TOTAL = 3u,
  /**
   * Request the time stamp corresponding to right after its last
   * SYNC word bit has been sent or received.
   * Indicate that time stamp did not require using total_packet_bytes.
   */
  SL_RAIL_PACKET_TIME_AT_SYNC_END = 4u,
  /**
   * Request the time stamp corresponding to right after its last
   * SYNC word bit has been sent or received.
   * Indicate that time stamp did require using total_packet_bytes.
   */
  SL_RAIL_PACKET_TIME_AT_SYNC_END_USED_TOTAL = 5u,
  /**
   * Request the time stamp corresponding to right after its last
   * bit has been sent or received.
   * Indicate that time stamp did not require using total_packet_bytes.
   */
  SL_RAIL_PACKET_TIME_AT_PACKET_END = 6u,
  /**
   * Request the time stamp corresponding to right after its last
   * bit has been sent or received.
   * Indicate that time stamp did require using total_packet_bytes.
   */
  SL_RAIL_PACKET_TIME_AT_PACKET_END_USED_TOTAL = 7u,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_PACKET_TIME_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_PACKET_TIME_INVALID                      ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_INVALID)
#define SL_RAIL_PACKET_TIME_DEFAULT                      ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_DEFAULT)
#define SL_RAIL_PACKET_TIME_AT_PREAMBLE_START            ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_AT_PREAMBLE_START)
#define SL_RAIL_PACKET_TIME_AT_PREAMBLE_START_USED_TOTAL ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_AT_PREAMBLE_START_USED_TOTAL)
#define SL_RAIL_PACKET_TIME_AT_SYNC_END                  ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_AT_SYNC_END)
#define SL_RAIL_PACKET_TIME_AT_SYNC_END_USED_TOTAL       ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_AT_SYNC_END_USED_TOTAL)
#define SL_RAIL_PACKET_TIME_AT_PACKET_END                ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_AT_PACKET_END)
#define SL_RAIL_PACKET_TIME_AT_PACKET_END_USED_TOTAL     ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_AT_PACKET_END_USED_TOTAL)
#define SL_RAIL_PACKET_TIME_COUNT                        ((sl_rail_packet_time_position_t) SL_RAIL_PACKET_TIME_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_packet_time_stamp_t
 * @brief Information for calculating and representing a packet time stamp.
 */
typedef struct sl_rail_packet_time_stamp {
  /**
   * Time stamp of the packet in the RAIL timebase.
   */
  sl_rail_time_t packet_time;
  /**
   * A value specifying the total length in bytes of the packet
   * used when calculating the packetTime requested by the time_position
   * field. This should account for all bytes sent over the air after
   * the Preamble and Sync word(s) including CRC bytes.
   */
  uint16_t total_packet_bytes;
  /**
   * A \ref sl_rail_packet_time_position_t value specifying the packet position
   * to return in the packet_time field.
   * If this is \ref SL_RAIL_PACKET_TIME_DEFAULT, this field will be
   * updated with the actual position corresponding to the packet_time
   * value filled in by a call using this structure.
   */
  sl_rail_packet_time_position_t time_position;
  /**
   * In RX for EFR32xG25 only:
   * A value specifying the on-air duration of the data packet,
   * starting with the first bit of the PHR (i.e., end of sync word);
   * preamble and sync word duration are hence excluded.
   *
   * In Tx for all platforms:
   * A value specifying the on-air duration of the data packet,
   * starting at the preamble (i.e. includes preamble, sync word, PHR,
   * payload and FCS). This value can be used to compute duty cycles.
   * @note This field is currently valid only for normal transmits but
   *   not Auto-Ack transmits which set the field to zero.
   */
  sl_rail_time_t packet_duration_us;
} sl_rail_packet_time_stamp_t;

/** @} */ // end of group System_Timing

/******************************************************************************
 * Sleep Structures
 *****************************************************************************/
/**
 * @addtogroup Sleep
 * @{
 */

/**
 * @enum sl_rail_sleep_config_t
 * @brief The configuration
 */
SLI_RAIL_ENUM(sl_rail_sleep_config_t) {
  /**
   * Disable timer sync before and after sleep.
   */
  SL_RAIL_SLEEP_CONFIG_TIMERSYNC_DISABLED = 0u,
  /**
   * Enable timer sync before and after sleep.
   */
  SL_RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED = 1u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_SLEEP_CONFIG_TIMERSYNC_DISABLED ((sl_rail_sleep_config_t) SL_RAIL_SLEEP_CONFIG_TIMERSYNC_DISABLED)
#define SL_RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED  ((sl_rail_sleep_config_t) SL_RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Platform-agnostic value to use default PRS channel when configuring sleep.
 */
#define SL_RAIL_TIMER_SYNC_PRS_CHANNEL_DEFAULT  (255U)

/**
 * Platform-agnostic vlaue to use default RTCC channel when configuring sleep.
 */
#define SL_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT (255U)

/**
 * @struct sl_rail_timer_sync_config_t
 * @brief Channel values used to perform timer sync before and after sleep.
 *
 * The default value of this structure is provided in the
 * \ref SL_RAIL_TIMER_SYNC_DEFAULT macro.
 */
typedef struct sl_rail_timer_sync_config {
  /**
   * PRS Channel used for timer sync operations.
   * Use \ref SL_RAIL_TIMER_SYNC_PRS_CHANNEL_DEFAULT or another suitable one.
   */
  uint8_t prs_channel;
  /**
   * RTCC Channel used for timer sync operations. Only applies to
   * platforms where the RTCC used for timer sync has multiple channels.
   * Use \ref SL_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT or another suitable one.
   */
  uint8_t rtcc_channel;
  /**
   * Whether to sync the timer before and after sleeping.
   */
  sl_rail_sleep_config_t sleep;
} sl_rail_timer_sync_config_t;

/**
 * Default timer synchronization configuration.
 */
#define SL_RAIL_TIMER_SYNC_DEFAULT {                         \
    .prs_channel  = SL_RAIL_TIMER_SYNC_PRS_CHANNEL_DEFAULT,  \
    .rtcc_channel = SL_RAIL_TIMER_SYNC_RTCC_CHANNEL_DEFAULT, \
    .sleep        = SL_RAIL_SLEEP_CONFIG_TIMERSYNC_ENABLED,  \
}

/** @} */ // end of group Sleep

/******************************************************************************
 * Multiprotocol Structures
 *****************************************************************************/
/**
 * @addtogroup Multiprotocol
 * @{
 */

/**
 * @struct sl_rail_scheduler_info_t
 * @brief A structure to hold information used by the scheduler.
 *
 * For multiprotocol versions of RAIL, this can be used to control how a receive
 * or transmit operation is run. It's not necessary in single-protocol applications.
 */
typedef struct sl_rail_scheduler_info {
  /**
   * The scheduler priority to use for this operation. This priority is used to
   * preempt a long running lower-priority task to ensure higher-priority
   * operations complete in time. A lower numerical value represents a higher
   * logical priority meaning 0 is the highest priority and 255 is the lowest.
   */
  uint8_t priority;
  /**
   * The amount of time in microseconds that this operation can slip by into the future
   * and still be run. This time is relative to the start time which may be
   * the current time for relative transmits. If the scheduler can't start the
   * operation by this time, it will be considered a failure.
   */
  sl_rail_time_t slip_time;
  /**
   * The transaction time in microseconds for this operation. Since transaction times may
   * not be known exactly, use a minimum or an expected
   * guess for this time. The scheduler will use the value entered here to look
   * for overlaps between low-priority and high-priority tasks and attempt to
   * find a schedule where all tasks get to run.
   */
  sl_rail_time_t transaction_time;
} sl_rail_scheduler_info_t;

/**
 * Radio Scheduler Status mask within \ref sl_rail_scheduler_status_t values.
 */
#define SL_RAIL_SCHEDULER_STATUS_MASK       0x0FU
/**
 * Radio Scheduler Status shift within \ref sl_rail_scheduler_status_t values.
 */
#define SL_RAIL_SCHEDULER_STATUS_SHIFT      0

/**
 * Radio Scheduler Task mask within \ref sl_rail_scheduler_status_t values.
 */
#define SL_RAIL_SCHEDULER_TASK_MASK         0xF0U
/**
 * Radio Scheduler Task shift within \ref sl_rail_scheduler_status_t values.
 */
#define SL_RAIL_SCHEDULER_TASK_SHIFT        4

/**
 * @enum sl_rail_scheduler_status_t
 * @brief Multiprotocol scheduler status returned by \ref sl_rail_get_scheduler_status().
 *
 * \ref Multiprotocol scheduler status is a combination of the upper 4 bits which
 * constitute the type of scheduler task and the lower 4 bits which constitute
 * the type of scheduler error.
 */
SLI_RAIL_ENUM(sl_rail_scheduler_status_t) {
  /**
   * Multiprotocol scheduler reports no error.
   */
  SL_RAIL_SCHEDULER_STATUS_NO_ERROR = (0U << SL_RAIL_SCHEDULER_STATUS_SHIFT),
  /**
   * The scheduler is disabled or the requested scheduler operation is
   * unsupported.
   */
  SL_RAIL_SCHEDULER_STATUS_UNSUPPORTED = (1U << SL_RAIL_SCHEDULER_STATUS_SHIFT),
  /**
   * The scheduled task was started but was interrupted by a higher-priority
   * event before it could be completed.
   */
  SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED = (2U << SL_RAIL_SCHEDULER_STATUS_SHIFT),
  /**
   * Scheduled task could not be scheduled given its priority and the other
   * tasks running on the system.
   */
  SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL = (3U << SL_RAIL_SCHEDULER_STATUS_SHIFT),
  /**
   * Calling the RAIL API associated with the Radio scheduler task returned
   * an error code. See \ref sl_rail_get_scheduler_status()
   * for more information about \ref sl_rail_status_t status.
   */
  SL_RAIL_SCHEDULER_STATUS_TASK_FAIL = (4U << SL_RAIL_SCHEDULER_STATUS_SHIFT),
  /**
   * An internal error occurred in scheduler data structures, which should
   * not happen and indicates a problem.
   */
  SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR = (5U << SL_RAIL_SCHEDULER_STATUS_SHIFT),

  /**
   * Radio scheduler has no task.
   */
  SL_RAIL_SCHEDULER_TASK_EMPTY = (0U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_scheduled_rx().
   */
  SL_RAIL_SCHEDULER_TASK_SCHEDULED_RX = (1U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_scheduled_tx().
   */
  SL_RAIL_SCHEDULER_TASK_SCHEDULED_TX = (2U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_tx().
   */
  SL_RAIL_SCHEDULER_TASK_SINGLE_TX = (3U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_cca_csma_tx().
   */
  SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_CSMA_TX = (4U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_cca_lbt_tx().
   */
  SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_LBT_TX = (5U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_scheduled_cca_csma_tx().
   */
  SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_CSMA_TX = (6U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_scheduled_cca_lbt_tx().
   */
  SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_LBT_TX = (7U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_tx_stream().
   */
  SL_RAIL_SCHEDULER_TASK_TX_STREAM = (8U << SL_RAIL_SCHEDULER_TASK_SHIFT),
  /**
   * Radio scheduler calls \ref sl_rail_start_average_rssi().
   */
  SL_RAIL_SCHEDULER_TASK_AVERAGE_RSSI = (9U << SL_RAIL_SCHEDULER_TASK_SHIFT),

  /**
   * \ref sl_rail_start_scheduled_tx() returned error status.
   */
  SL_RAIL_SCHEDULER_STATUS_SCHEDULED_TX_FAIL = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_TX
                                                | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * \ref sl_rail_start_tx() returned error status.
   */
  SL_RAIL_SCHEDULER_STATUS_SINGLE_TX_FAIL = (SL_RAIL_SCHEDULER_TASK_SINGLE_TX
                                             | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * \ref sl_rail_start_cca_csma_tx() returned error status.
   */
  SL_RAIL_SCHEDULER_STATUS_CCA_CSMA_TX_FAIL = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_CSMA_TX
                                               | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * \ref sl_rail_start_cca_lbt_tx() returned error status.
   */
  SL_RAIL_SCHEDULER_STATUS_CCA_LBT_TX_FAIL = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_LBT_TX
                                              | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * \ref sl_rail_start_scheduled_rx() returned error status.
   */
  SL_RAIL_SCHEDULER_STATUS_SCHEDULED_RX_FAIL = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_RX
                                                | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * \ref sl_rail_start_tx_stream() returned error status.
   */
  SL_RAIL_SCHEDULER_STATUS_TX_STREAM_FAIL = (SL_RAIL_SCHEDULER_TASK_TX_STREAM
                                             | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * \ref sl_rail_start_average_rssi() returned error status.
   */
  SL_RAIL_SCHEDULER_STATUS_AVERAGE_RSSI_FAIL = (SL_RAIL_SCHEDULER_TASK_AVERAGE_RSSI
                                                | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),

  /**
   * Multiprotocol scheduled receive function internal error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_RX_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_RX
                                                   | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * Multiprotocol scheduled receive scheduling error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_RX_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_RX
                                                     | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_scheduled_rx() operation interrupted
   */
  SL_RAIL_SCHEDULER_SCHEDULED_RX_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_RX
                                                | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol scheduled TX internal error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_TX_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_TX
                                                   | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * Multiprotocol scheduled TX scheduling error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_TX_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_TX
                                                     | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_scheduled_tx() operation interrupted.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_TX_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_TX
                                                | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol instantaneous TX internal error.
   */
  SL_RAIL_SCHEDULER_SINGLE_TX_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_SINGLE_TX
                                                | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * Multiprotocol instantaneous TX scheduling error.
   */
  SL_RAIL_SCHEDULER_SINGLE_TX_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_SINGLE_TX
                                                  | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_tx() operation interrupted.
   */
  SL_RAIL_SCHEDULER_SINGLE_TX_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_SINGLE_TX
                                             | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol single CSMA transmit function internal error.
   */
  SL_RAIL_SCHEDULER_SINGLE_CCA_CSMA_TX_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_CSMA_TX
                                                         | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * Multiprotocol single CSMA transmit scheduling error.
   */
  SL_RAIL_SCHEDULER_SINGLE_CCA_CSMA_TX_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_CSMA_TX
                                                           | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_cca_csma_tx() operation interrupted.
   */
  SL_RAIL_SCHEDULER_SINGLE_CCA_CSMA_TX_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_CSMA_TX
                                                      | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol single LBT transmit function internal error.
   */
  SL_RAIL_SCHEDULER_SINGLE_CCA_LBT_TX_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_LBT_TX
                                                        | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * Multiprotocol single LBT transmit scheduling error.
   */
  SL_RAIL_SCHEDULER_SINGLE_CCA_LBT_TX_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_LBT_TX
                                                          | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_cca_lbt_tx() operation interrupted.
   */
  SL_RAIL_SCHEDULER_SINGLE_CCA_LBT_TX_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_LBT_TX
                                                     | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol scheduled CSMA transmit function internal error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_CSMA_TX
                                                            | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * \ref sl_rail_start_scheduled_cca_csma_tx() returned error status.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_FAIL = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_CSMA_TX
                                                  | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * Multiprotocol scheduled CSMA transmit scheduling error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_CSMA_TX
                                                              | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_scheduled_cca_csma_tx() operation interrupted.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_CSMA_TX
                                                         | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol scheduled LBT transmit function internal error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_LBT_TX
                                                           | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * \ref sl_rail_start_scheduled_cca_lbt_tx() returned error status.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_FAIL = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_LBT_TX
                                                 | SL_RAIL_SCHEDULER_STATUS_TASK_FAIL),
  /**
   * Multiprotocol scheduled LBT transmit scheduling error.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_LBT_TX
                                                             | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_scheduled_cca_lbt_tx() operation interrupted.
   */
  SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_LBT_TX
                                                        | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol stream transmit function internal error.
   */
  SL_RAIL_SCHEDULER_TX_STREAM_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_TX_STREAM
                                                | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * Multiprotocol stream transmit scheduling error.
   */
  SL_RAIL_SCHEDULER_TX_STREAM_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_TX_STREAM
                                                  | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_tx_stream() operation interrupted.
   */
  SL_RAIL_SCHEDULER_TX_STREAM_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_TX_STREAM
                                             | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),

  /**
   * Multiprotocol RSSI averaging function internal error.
   */
  SL_RAIL_SCHEDULER_AVERAGE_RSSI_INTERNAL_ERROR = (SL_RAIL_SCHEDULER_TASK_AVERAGE_RSSI
                                                   | SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR),
  /**
   * Multiprotocol RSSI average scheduling error.
   */
  SL_RAIL_SCHEDULER_AVERAGE_RSSI_SCHEDULING_ERROR = (SL_RAIL_SCHEDULER_TASK_AVERAGE_RSSI
                                                     | SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL),
  /**
   * \ref sl_rail_start_average_rssi() operation interrupted.
   */
  SL_RAIL_SCHEDULER_AVERAGE_RSSI_INTERRUPTED = (SL_RAIL_SCHEDULER_TASK_AVERAGE_RSSI
                                                | SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED),
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_SCHEDULER_STATUS_NO_ERROR                        ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_NO_ERROR)
#define SL_RAIL_SCHEDULER_STATUS_UNSUPPORTED                     ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_UNSUPPORTED)
#define SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_EVENT_INTERRUPTED)
#define SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL                   ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_SCHEDULE_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_SCHEDULED_TX_FAIL               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_SCHEDULED_TX_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_SINGLE_TX_FAIL                  ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_SINGLE_TX_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_CCA_CSMA_TX_FAIL                ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_CCA_CSMA_TX_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_CCA_LBT_TX_FAIL                 ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_CCA_LBT_TX_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_SCHEDULED_RX_FAIL               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_SCHEDULED_RX_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_TX_STREAM_FAIL                  ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_TX_STREAM_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_AVERAGE_RSSI_FAIL               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_AVERAGE_RSSI_FAIL)
#define SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR                  ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_STATUS_INTERNAL_ERROR)

#define SL_RAIL_SCHEDULER_TASK_EMPTY                             ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_EMPTY)
#define SL_RAIL_SCHEDULER_TASK_SCHEDULED_RX                      ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_SCHEDULED_RX)
#define SL_RAIL_SCHEDULER_TASK_SCHEDULED_TX                      ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_SCHEDULED_TX)
#define SL_RAIL_SCHEDULER_TASK_SINGLE_TX                         ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_SINGLE_TX)
#define SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_CSMA_TX                ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_CSMA_TX)
#define SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_LBT_TX                 ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_SINGLE_CCA_LBT_TX)
#define SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_CSMA_TX             ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_CSMA_TX)
#define SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_LBT_TX              ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_SCHEDULED_CCA_LBT_TX)
#define SL_RAIL_SCHEDULER_TASK_TX_STREAM                         ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_TX_STREAM)
#define SL_RAIL_SCHEDULER_TASK_AVERAGE_RSSI                      ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TASK_AVERAGE_RSSI)

#define SL_RAIL_SCHEDULER_SCHEDULED_RX_INTERNAL_ERROR            ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_RX_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_RX_SCHEDULING_ERROR          ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_RX_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_RX_INTERRUPTED               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_RX_INTERRUPTED)
#define SL_RAIL_SCHEDULER_SCHEDULED_TX_INTERNAL_ERROR            ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_TX_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_TX_SCHEDULING_ERROR          ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_TX_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_TX_INTERRUPTED               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_TX_INTERRUPTED)
#define SL_RAIL_SCHEDULER_SINGLE_TX_INTERNAL_ERROR               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SINGLE_TX_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_SINGLE_TX_SCHEDULING_ERROR             ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SINGLE_TX_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_SINGLE_TX_INTERRUPTED                  ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SINGLE_TX_INTERRUPTED)
#define SL_RAIL_SCHEDULER_CCA_CSMA_TX_INTERNAL_ERROR             ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_CCA_CSMA_TX_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_CCA_CSMA_TX_SCHEDULING_ERROR           ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_CCA_CSMA_TX_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_CCA_CSMA_TX_INTERRUPTED                ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_CCA_CSMA_TX_INTERRUPTED)
#define SL_RAIL_SCHEDULER_CCA_LBT_TX_INTERNAL_ERROR              ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_CCA_LBT_TX_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_CCA_LBT_TX_SCHEDULING_ERROR            ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_CCA_LBT_TX_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_CCA_LBT_TX_INTERRUPTED                 ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_CCA_LBT_TX_INTERRUPTED)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_INTERNAL_ERROR   ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_FAIL             ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_FAIL)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_SCHEDULING_ERROR ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_INTERRUPTED      ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_CSMA_TX_INTERRUPTED)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_INTERNAL_ERROR    ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_FAIL              ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_FAIL)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_SCHEDULING_ERROR  ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_INTERRUPTED       ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_SCHEDULED_CCA_LBT_TX_INTERRUPTED)
#define SL_RAIL_SCHEDULER_TX_STREAM_INTERNAL_ERROR               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TX_STREAM_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_TX_STREAM_SCHEDULING_ERROR             ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TX_STREAM_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_TX_STREAM_INTERRUPTED                  ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_TX_STREAM_INTERRUPTED)
#define SL_RAIL_SCHEDULER_AVERAGE_RSSI_INTERNAL_ERROR            ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_AVERAGE_RSSI_INTERNAL_ERROR)
#define SL_RAIL_SCHEDULER_AVERAGE_RSSI_SCHEDULING_ERROR          ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_AVERAGE_RSSI_SCHEDULING_ERROR)
#define SL_RAIL_SCHEDULER_AVERAGE_RSSI_INTERRUPTED               ((sl_rail_scheduler_status_t) SL_RAIL_SCHEDULER_AVERAGE_RSSI_INTERRUPTED)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_task_type_t
 * @brief Multiprotocol radio operation task types, used with
 *   \ref sl_rail_set_task_priority().
 */
SLI_RAIL_ENUM(sl_rail_task_type_t) {
  /**
   * Indicate a task started using \ref sl_rail_start_rx().
   */
  SL_RAIL_TASK_TYPE_START_RX = 0u,
  /**
   * Indicate a task started functions other than \ref sl_rail_start_rx().
   */
  SL_RAIL_TASK_TYPE_OTHER = 1u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_TASK_TYPE_START_RX ((sl_rail_task_type_t) SL_RAIL_TASK_TYPE_START_RX)
#define SL_RAIL_TASK_TYPE_OTHER    ((sl_rail_task_type_t) SL_RAIL_TASK_TYPE_OTHER)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // end of group Multiprotocol

/******************************************************************************
 * Event Structures
 *****************************************************************************/
/**
 * @addtogroup Events
 * @{
 */

/**
 * @enum sl_rail_events_t
 * @brief RAIL events passed to the event callback. More than one event may be
 *   indicated due to interrupt latency.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_events_t, uint64_t) {
  // RX Event Bit Shifts

  /**
   * Shift position of \ref SL_RAIL_EVENT_RSSI_AVERAGE_DONE bit.
   */
  SL_RAIL_EVENT_RSSI_AVERAGE_DONE_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_ACK_TIMEOUT bit.
   */
  SL_RAIL_EVENT_RX_ACK_TIMEOUT_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL bit.
   */
  SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED bit.
   */
  SL_RAIL_EVENT_RX_PACKET_RECEIVED_SHIFT = 3,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_PREAMBLE_LOST bit.
   */
  SL_RAIL_EVENT_RX_PREAMBLE_LOST_SHIFT = 4,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_PREAMBLE_DETECT bit.
   */
  SL_RAIL_EVENT_RX_PREAMBLE_DETECT_SHIFT = 5,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_SYNC_0_DETECT bit.
   */
  SL_RAIL_EVENT_RX_SYNC_0_DETECT_SHIFT = 6,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_SYNC_1_DETECT bit.
   */
  SL_RAIL_EVENT_RX_SYNC_1_DETECT_SHIFT = 7,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_FRAME_ERROR bit.
   */
  SL_RAIL_EVENT_RX_FRAME_ERROR_SHIFT = 8,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_FIFO_FULL bit.
   */
  SL_RAIL_EVENT_RX_FIFO_FULL_SHIFT = 9,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW bit.
   */
  SL_RAIL_EVENT_RX_FIFO_OVERFLOW_SHIFT = 10,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_ADDRESS_FILTERED bit.
   */
  SL_RAIL_EVENT_RX_ADDRESS_FILTERED_SHIFT = 11,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_TIMEOUT bit.
   */
  SL_RAIL_EVENT_RX_TIMEOUT_SHIFT = 12,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED bit.
   */
  SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED_SHIFT = 13,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_END bit.
   */
  SL_RAIL_EVENT_RX_SCHEDULED_RX_END_SHIFT = 14,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED bit.
   */
  SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED_SHIFT = 15,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_PACKET_ABORTED bit.
   */
  SL_RAIL_EVENT_RX_PACKET_ABORTED_SHIFT = 16,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_FILTER_PASSED bit.
   */
  SL_RAIL_EVENT_RX_FILTER_PASSED_SHIFT = 17,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_TIMING_LOST bit.
   */
  SL_RAIL_EVENT_RX_TIMING_LOST_SHIFT = 18,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_TIMING_DETECT bit.
   */
  SL_RAIL_EVENT_RX_TIMING_DETECT_SHIFT = 19,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE bit.
   */
  SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE_SHIFT = 20,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END bit.
   */
  SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END_SHIFT = SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE_SHIFT,
  /**
   * Shift position of \ref SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND bit.
   */
  SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND_SHIFT = 21,
  /**
   * Shift position of \ref SL_RAIL_EVENT_ZWAVE_LR_ACK_REQUEST_COMMAND_SHIFT bit.
   */
  SL_RAIL_EVENT_ZWAVE_LR_ACK_REQUEST_COMMAND_SHIFT = SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND_SHIFT,
  /**
   * Shift position of \ref SL_RAIL_EVENT_MFM_TX_BUFFER_DONE bit.
   */
  SL_RAIL_EVENT_MFM_TX_BUFFER_DONE_SHIFT = SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND_SHIFT,

// TX Event Bit Shifts

  /**
   * Shift position of \ref SL_RAIL_EVENT_ZWAVE_BEAM bit.
   */
  SL_RAIL_EVENT_ZWAVE_BEAM_SHIFT = 22,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY bit.
   */
  SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY_SHIFT = 23,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_PACKET_SENT bit.
   */
  SL_RAIL_EVENT_TX_PACKET_SENT_SHIFT = 24,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TXACK_PACKET_SENT bit.
   */
  SL_RAIL_EVENT_TXACK_PACKET_SENT_SHIFT = 25,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_ABORTED bit.
   */
  SL_RAIL_EVENT_TX_ABORTED_SHIFT = 26,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TXACK_ABORTED bit.
   */
  SL_RAIL_EVENT_TXACK_ABORTED_SHIFT = 27,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_BLOCKED bit.
   */
  SL_RAIL_EVENT_TX_BLOCKED_SHIFT = 28,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TXACK_BLOCKED bit.
   */
  SL_RAIL_EVENT_TXACK_BLOCKED_SHIFT = 29,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_UNDERFLOW bit.
   */
  SL_RAIL_EVENT_TX_UNDERFLOW_SHIFT = 30,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TXACK_UNDERFLOW bit.
   */
  SL_RAIL_EVENT_TXACK_UNDERFLOW_SHIFT = 31,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_CHANNEL_CLEAR bit.
   */
  SL_RAIL_EVENT_TX_CHANNEL_CLEAR_SHIFT = 32,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_CHANNEL_BUSY bit.
   */
  SL_RAIL_EVENT_TX_CHANNEL_BUSY_SHIFT = 33,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_CCA_RETRY bit.
   */
  SL_RAIL_EVENT_TX_CCA_RETRY_SHIFT = 34,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_START_CCA bit.
   */
  SL_RAIL_EVENT_TX_START_CCA_SHIFT = 35,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_STARTED bit.
   */
  SL_RAIL_EVENT_TX_STARTED_SHIFT = 36,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED bit.
   */
  SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED_SHIFT = SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED_SHIFT,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED bit.
   */
  SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED_SHIFT = 37,

  // Scheduler Event Bit Shifts

  /**
   * Shift position of \ref SL_RAIL_EVENT_CONFIG_UNSCHEDULED bit.
   */
  SL_RAIL_EVENT_CONFIG_UNSCHEDULED_SHIFT = 38,
  /**
   * Shift position of \ref SL_RAIL_EVENT_CONFIG_SCHEDULED bit.
   */
  SL_RAIL_EVENT_CONFIG_SCHEDULED_SHIFT = 39,
  /**
   * Shift position of \ref SL_RAIL_EVENT_SCHEDULER_STATUS bit.
   */
  SL_RAIL_EVENT_SCHEDULER_STATUS_SHIFT = 40,

  // Other Event Bit Shifts

  /**
   * Shift position of \ref SL_RAIL_EVENT_CAL_NEEDED bit.
   */
  SL_RAIL_EVENT_CAL_NEEDED_SHIFT = 41,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RF_SENSED bit.
   */
  SL_RAIL_EVENT_RF_SENSED_SHIFT = 42,
  /**
   * Shift position of \ref SL_RAIL_EVENT_PA_PROTECTION bit.
   */
  SL_RAIL_EVENT_PA_PROTECTION_SHIFT = 43,
  /**
   * Shift position of \ref SL_RAIL_EVENT_SIGNAL_DETECTED bit.
   */
  SL_RAIL_EVENT_SIGNAL_DETECTED_SHIFT = 44,
  /**
   * Shift position of \ref SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START bit.
   */
  SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START_SHIFT = 45,
  /**
   * Shift position of \ref SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END bit.
   */
  SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END_SHIFT = 46,
  /**
   * Shift position of \ref SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD bit.
   */
  SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD_SHIFT = 47,
  /**
   * Shift position of \ref SL_RAIL_EVENT_THERMISTOR_DONE bit.
   */
  SL_RAIL_EVENT_THERMISTOR_DONE_SHIFT = 48,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RESERVED_49 bit.
   */
  SL_RAIL_EVENT_RESERVED_49_SHIFT = 49,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TEMPERATURE_TOO_HOT bit.
   */
  SL_RAIL_EVENT_TEMPERATURE_TOO_HOT_SHIFT = 50,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TEMPERATURE_COOL_DOWN bit.
   */
  SL_RAIL_EVENT_TEMPERATURE_COOL_DOWN_SHIFT = 51,
  /**
   * Shift position of \ref SL_RAIL_EVENT_USER_MBOX bit.
   */
  SL_RAIL_EVENT_USER_MBOX_SHIFT = 52,
  /**
   * Shift position of \ref SL_RAIL_EVENT_TX_CCA_ACTIVATED bit.
   */
  SL_RAIL_EVENT_TX_CCA_ACTIVATED_SHIFT = 53,
  /**
   * Shift position of \ref SL_RAIL_EVENT_RX_BLOCKED bit.
   */
  SL_RAIL_EVENT_RX_BLOCKED_SHIFT = 54,
};

// sl_rail_events_t bitmasks

/**
 * A value representing no events.
 */
#define SL_RAIL_EVENTS_NONE 0ULL

/**
 * Occurs when the hardware-averaged RSSI is done in response to
 * \ref sl_rail_start_average_rssi() to indicate that the hardware has completed
 * averaging.
 *
 * Call \ref sl_rail_get_average_rssi() to get the result.
 */
#define SL_RAIL_EVENT_RSSI_AVERAGE_DONE (1ULL << SL_RAIL_EVENT_RSSI_AVERAGE_DONE_SHIFT)

/**
 * Occurs when the Ack timeout expires while waiting to receive the
 * sync word of an expected Ack. If the timeout occurs within packet
 * reception, this event won't be signaled until after packet
 * completion has determined the packet wasn't the expected Ack.
 * See \ref sl_rail_rx_packet_details_t::is_ack for the definition of an
 * expected Ack.
 *
 * This event only occurs after calling \ref sl_rail_config_auto_ack() and after
 * transmitting a packet with \ref SL_RAIL_TX_OPTION_WAIT_FOR_ACK set.
 */
#define SL_RAIL_EVENT_RX_ACK_TIMEOUT (1ULL << SL_RAIL_EVENT_RX_ACK_TIMEOUT_SHIFT)

/**
 * Keeps occurring as long as the number of bytes in the receive FIFO
 * exceeds the configured threshold value.
 *
 * Call \ref sl_rail_get_rx_fifo_bytes_available() to get the number of
 * bytes available. When using this event, the threshold should be set via
 * \ref sl_rail_set_rx_fifo_threshold().
 *
 * How to avoid sticking in the event handler (even in idle state):
 * 1. Disable the event (via the config events API or the
 *    \ref SL_RAIL_FIFO_THRESHOLD_DISABLED parameter)
 * 2. Increase FIFO threshold
 * 3. Read the FIFO (that's not an option in
 *    \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE) in the event handler
 */
#define SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL (1ULL << SL_RAIL_EVENT_RX_FIFO_ALMOST_FULL_SHIFT)

/**
 * Occurs whenever a packet is received with \ref SL_RAIL_RX_PACKET_READY_SUCCESS
 * or \ref SL_RAIL_RX_PACKET_READY_CRC_ERROR.
 *
 * Call \ref sl_rail_get_rx_packet_info() to get
 * basic information about the packet along with a handle to this packet for
 * subsequent use with \ref sl_rail_peek_rx_packet(), \ref sl_rail_get_rx_packet_details(),
 * \ref sl_rail_hold_rx_packet(), and \ref sl_rail_release_rx_packet() as needed.
 */
#define SL_RAIL_EVENT_RX_PACKET_RECEIVED (1ULL << SL_RAIL_EVENT_RX_PACKET_RECEIVED_SHIFT)

/**
 * Occurs when the radio has lost a preamble.
 *
 * This event can occur multiple
 * times while searching for a packet and is generally used for diagnostic
 * purposes. It can only occur after a
 * \ref SL_RAIL_EVENT_RX_PREAMBLE_DETECT event has already occurred.
 *
 * @note See warning for \ref SL_RAIL_EVENT_RX_PREAMBLE_DETECT.
 */
#define SL_RAIL_EVENT_RX_PREAMBLE_LOST (1ULL << SL_RAIL_EVENT_RX_PREAMBLE_LOST_SHIFT)

/**
 * Occurs when the radio has detected a preamble.
 *
 * This event can occur multiple
 * times while searching for a packet and is generally used for diagnostic
 * purposes. It can only occur after a \ref SL_RAIL_EVENT_RX_TIMING_DETECT
 * event has already occurred.
 *
 * @warning This event, along with \ref SL_RAIL_EVENT_RX_PREAMBLE_LOST,
 *   may not work on some demodulators. Some demodulators usurped the signals
 *   on which these events are based for another purpose. These demodulators
 *   in particular are available on the EFR32xG23, EFR32xG25, and the EFR32xG28
 *   platforms. Enabling these events on these platforms may cause the
 *   events to fire infinitely and possibly freeze the application.
 */
#define SL_RAIL_EVENT_RX_PREAMBLE_DETECT (1ULL << SL_RAIL_EVENT_RX_PREAMBLE_DETECT_SHIFT)

/**
 * Occurs when the primary sync word is detected.
 *
 * After this event occurs, one of
 * the events in the \ref SL_RAIL_EVENTS_RX_COMPLETION mask will occur.
 */
#define SL_RAIL_EVENT_RX_SYNC_0_DETECT (1ULL << SL_RAIL_EVENT_RX_SYNC_0_DETECT_SHIFT)

/**
 * Occurs when the alternate sync word is detected.
 *
 * After this event occurs, one of
 * the events in the \ref SL_RAIL_EVENTS_RX_COMPLETION mask will occur.
 */
#define SL_RAIL_EVENT_RX_SYNC_1_DETECT (1ULL << SL_RAIL_EVENT_RX_SYNC_1_DETECT_SHIFT)

/**
 * Occurs when a receive is aborted with \ref SL_RAIL_RX_PACKET_ABORT_CRC_ERROR
 * which only happens after any filtering has passed.
 *
 * For EFR32 parts, this event includes CRC errors, block decoding errors,
 * and illegal frame length -- when detected after filtering. (When such
 * errors are detected during filtering, they're signaled as \ref
 * SL_RAIL_EVENT_RX_PACKET_ABORTED instead.)
 *
 * If \ref SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS is set, this event will not
 * occur for CRC errors, but could still occur for the other errors.
 */
#define SL_RAIL_EVENT_RX_FRAME_ERROR (1ULL << SL_RAIL_EVENT_RX_FRAME_ERROR_SHIFT)

/**
 * When using \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA this event
 * occurs coincident to a receive packet completion event in which the
 * receive FIFO or the supplemental receive Packet Queue (see \ref
 * Data_Management) are full and further packet reception is jeopardized.
 *
 * It signals that an overflow is imminent (and may already have occurred)
 * telling the application it should release the oldest packet(s) as soon
 * as possible. This event may may be posted multiple times with subsequent
 * receive completion events if the FIFO(s) remain full, and should also
 * occur coincident with \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW.
 *
 * When not using \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA this event
 * is not tied to packet completion and will occur coincident with
 * \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW when the receive FIFO has filled and
 * overflowed. The application should consume receive FIFO data via
 * \ref sl_rail_read_rx_fifo() as soon as possible to minimize lost raw data.
 */
#define SL_RAIL_EVENT_RX_FIFO_FULL (1ULL << SL_RAIL_EVENT_RX_FIFO_FULL_SHIFT)

/**
 * When using \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA this event
 * occurs when a receive is aborted with \ref SL_RAIL_RX_PACKET_ABORT_OVERFLOW
 * due to overflowing the receive FIFO or the supplemental receive Packet
 * Queue (see \ref Data_Management).
 *
 * The radio suspends receiving packets until this event is posted and
 * the receive FIFO(s) have been fully processed (drained and released
 * or reset). It is not guaranteed that a \ref SL_RAIL_EVENT_RX_FIFO_FULL
 * will precede this event, but both events should be coincident.
 *
 * When not using \ref sl_rail_rx_data_source_t::SL_RAIL_RX_DATA_SOURCE_PACKET_DATA this event
 * is not tied to packet completion and will occur coincident with
 * \ref SL_RAIL_EVENT_RX_FIFO_FULL when the receive FIFO has filled and
 * overflowed. The application should consume receive FIFO data via
 * \ref sl_rail_read_rx_fifo() as soon as possible to minimize lost raw data.
 */
#define SL_RAIL_EVENT_RX_FIFO_OVERFLOW (1ULL << SL_RAIL_EVENT_RX_FIFO_OVERFLOW_SHIFT)

/**
 * Occurs when a receive is aborted with \ref SL_RAIL_RX_PACKET_ABORT_FILTERED
 * because its address does not match the filtering settings.
 *
 * This event can only occur after calling \ref sl_rail_enable_address_filter().
 */
#define SL_RAIL_EVENT_RX_ADDRESS_FILTERED (1ULL << SL_RAIL_EVENT_RX_ADDRESS_FILTERED_SHIFT)

/**
 * Occurs when an RX event times out.
 *
 * This event can only occur if the
 * sl_rail_state_timing_t::rxsearch_timeout passed to \ref sl_rail_set_state_timing() is
 * not zero.
 */
#define SL_RAIL_EVENT_RX_TIMEOUT (1ULL << SL_RAIL_EVENT_RX_TIMEOUT_SHIFT)

/**
 * Occurs when a scheduled RX begins turning on the receiver.
 * This event has the same numerical value as \ref SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED
 * because one cannot schedule both RX and TX simultaneously.
 */
#define SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED (1ULL << SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED_SHIFT)

/**
 * Occurs when the receiver has been blocked due to radio arbitration
 *
 * This event will only occur if the receiver is blocked during packet search and no data has
 * been placed into the FIFO.  After this event the previous receive operation has been cancelled
 * and must be restarted.
 */
#define SL_RAIL_EVENT_RX_BLOCKED (1ULL << SL_RAIL_EVENT_RX_BLOCKED_SHIFT)

/**
 * Occurs when the scheduled RX window ends.
 *
 * This event only occurs in response
 * to a scheduled receive timeout after calling \ref sl_rail_start_scheduled_rx().
 * It will occur unless receive is explicitly terminated by a call to
 * \ref sl_rail_idle() or a scheduler preemption. If
 * sl_rail_scheduled_rx_config_t::rx_transition_end_schedule was passed as true,
 * any of the \ref SL_RAIL_EVENTS_RX_COMPLETION events occurring will also
 * trigger this event, since the scheduled receive ends with the
 * transition at the end of the packet.
 *
 * @note This event differs from RAIL 2.x behavior: In RAIL 2.x when
 *   \ref RAIL_ScheduleRxConfig_t::rxTransitionEndSchedule was passed as true,
 *   this event would be suppressed when any of the \ref RAIL_EVENTS_RX_COMPLETION
 *   events occurred and are enabled. In RAIL 3 the
 *   \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_END is never suppressed.
 */
#define SL_RAIL_EVENT_RX_SCHEDULED_RX_END (1ULL << SL_RAIL_EVENT_RX_SCHEDULED_RX_END_SHIFT)

/**
 * Occurs when start of a scheduled receive is missed.
 *
 * This can occur if the radio is put to sleep and not woken up with enough time
 * to configure the scheduled receive event.
 */
#define SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED (1ULL << SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED_SHIFT)

/**
 * Occurs when a receive is aborted during filtering with
 * \ref SL_RAIL_RX_PACKET_ABORT_FORMAT or after filtering with
 * \ref SL_RAIL_RX_PACKET_ABORT_ABORTED for reasons other than address
 * filtering mismatch (which triggers \ref SL_RAIL_EVENT_RX_ADDRESS_FILTERED
 * instead).
 *
 * For EFR32 parts, this event includes CRC errors, block decoding errors,
 * illegal frame length, and other RAIL built-in protocol-specific packet
 * content errors -- when detected during filtering. (When such errors
 * are detected after filtering, they're signaled as \ref
 * SL_RAIL_EVENT_RX_FRAME_ERROR instead.) It also includes application or
 * multiprotocol scheduler aborting a receive after filtering has passed.
 */
#define SL_RAIL_EVENT_RX_PACKET_ABORTED (1ULL << SL_RAIL_EVENT_RX_PACKET_ABORTED_SHIFT)

/**
 * Occurs when the packet has passed any configured address and frame
 * filtering options.
 *
 * This event will only occur between the start of the
 * packet, indicated by \ref SL_RAIL_EVENT_RX_SYNC_0_DETECT or
 * \ref SL_RAIL_EVENT_RX_SYNC_1_DETECT and one of the events in the
 * \ref SL_RAIL_EVENTS_RX_COMPLETION mask. It will always occur before or
 * concurrently with \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED. If IEEE 802.15.4 frame
 * and address filtering are enabled, this event will occur immediately after
 * destination address filtering.
 */
#define SL_RAIL_EVENT_RX_FILTER_PASSED (1ULL << SL_RAIL_EVENT_RX_FILTER_PASSED_SHIFT)

/**
 * Occurs when the modem timing is lost.
 *
 * This event can occur multiple times
 * while searching for a packet and is generally used for diagnostic purposes.
 * It can only occur after a \ref SL_RAIL_EVENT_RX_TIMING_DETECT event has
 * already occurred.
 *
 * @note See warning for \ref SL_RAIL_EVENT_RX_TIMING_DETECT.
 */
#define SL_RAIL_EVENT_RX_TIMING_LOST (1ULL << SL_RAIL_EVENT_RX_TIMING_LOST_SHIFT)

/**
 * Occurs when the modem timing is detected.
 *
 * This event can occur multiple times
 * while searching for a packet and is generally used for diagnostic purposes.
 *
 * @warning This event, along with \ref SL_RAIL_EVENT_RX_TIMING_LOST,
 *   may not work on some demodulators. Some demodulators usurped the signals
 *   on which these events are based for another purpose. These demodulators
 *   in particular are available on the EFR32xG23, EFR32xG25, and the EFR32xG28
 *   platforms. Enabling these events on these platforms may cause the
 *   events to fire infinitely and possibly freeze the application.
 */
#define SL_RAIL_EVENT_RX_TIMING_DETECT (1ULL << SL_RAIL_EVENT_RX_TIMING_DETECT_SHIFT)

/**
 * Occurs when RX Channel Hopping is enabled and channel hopping finishes
 * receiving on the last channel in its sequence.
 *
 * The intent behind this event
 * is to allow the user to keep the radio on for as short a time as possible.
 * That is, once the channel sequence is complete, the application will receive
 * this event and can trigger a sleep/idle until it is necessary to cycle
 * through the channels again. If this event is left on indefinitely and not
 * handled it will likely be a fairly noisy event, as it continues to fire
 * each time the hopping algorithm cycles through the channel sequence.
 */
#define SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE (1ULL << SL_RAIL_EVENT_RX_CHANNEL_HOPPING_COMPLETE_SHIFT)

/**
 * Occurs during RX duty cycle mode when the radio finishes its time in
 * receive mode.
 *
 * The application can then trigger a sleep/idle until it
 * needs to listen again.
 */
#define SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END (1ULL << SL_RAIL_EVENT_RX_DUTY_CYCLE_RX_END_SHIFT)

/**
 * Indicate a Data Request is received when using IEEE 802.15.4
 * functionality.
 *
 * It occurs when the command byte of an incoming Ack-requesting MAC Control
 * frame is for a data request. This callback is called before
 * the packet is fully received to allow the node to have more time to decide
 * whether to indicate a frame is pending in the outgoing Ack. This event only
 * occurs if the RAIL IEEE 802.15.4 functionality is enabled, but will never
 * occur if promiscuous mode is enabled via \ref
 * sl_rail_ieee802154_set_promiscuous_mode().
 *
 * Call \ref sl_rail_ieee802154_get_address() to get the source address of the packet.
 */
#define SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND (1ULL << SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND_SHIFT)

/**
 * Indicate a Z-Wave Beam Request relevant to the node was received.
 *
 * This event only occurs if the RAIL Z-Wave functionality is enabled
 * and its \ref SL_RAIL_ZWAVE_OPTION_DETECT_BEAM_FRAMES is enabled.
 * This event is used in lieu of \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED,
 * which is reserved for Z-Wave packets other than Beams.
 *
 * Call \ref sl_rail_zwave_get_rx_beam_details() to get the Node Id to which the Beam was
 * targeted, which would be either the broadcast id 0xFF or the node's own
 * single-cast id, as well as other beam packet details.
 *
 * @note All Z-Wave Beam requests are generally discarded, triggering
 *   \ref SL_RAIL_EVENT_RX_PACKET_ABORTED.
 */
#define SL_RAIL_EVENT_ZWAVE_BEAM (1ULL << SL_RAIL_EVENT_ZWAVE_BEAM_SHIFT)

/**
 * Indicate a MFM buffer has completely transmitted.
 *
 * This event only occurs if the RAIL MFM functionality is enabled
 * and a MFM buffer has completely transmitted.
 *
 * Following this event, the application can update the MFM buffer
 * that has transmitted to be used for the next transmission.
 */
#define SL_RAIL_EVENT_MFM_TX_BUFFER_DONE (1ULL << SL_RAIL_EVENT_MFM_TX_BUFFER_DONE_SHIFT)

/**
 * Indicate a request for populating Z-Wave LR Ack packet.
 * This event only occurs if the RAIL Z-Wave functionality is enabled.
 *
 * Following this event, the application must call \ref sl_rail_zwave_set_lr_ack_data()
 * to populate noise floor, TX power and receive RSSI fields of the Z-Wave
 * Long Range Ack packet.
 */
#define SL_RAIL_EVENT_ZWAVE_LR_ACK_REQUEST_COMMAND (1ULL << SL_RAIL_EVENT_ZWAVE_LR_ACK_REQUEST_COMMAND_SHIFT)

/**
 * The mask representing all events that determine the end of a received
 * packet.
 *
 * After a \ref SL_RAIL_EVENT_RX_SYNC_0_DETECT or a
 * \ref SL_RAIL_EVENT_RX_SYNC_1_DETECT,
 * exactly one of the following events will occur. When one of these events
 * occurs, a state transition will take place based on the parameter passed to
 * \ref sl_rail_set_rx_transitions(). The \ref sl_rail_state_transitions_t::success transition
 * will be followed only if the \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED event occurs.
 * Any of the other events will trigger the \ref sl_rail_state_transitions_t::error
 * transition.
 */
#define SL_RAIL_EVENTS_RX_COMPLETION (SL_RAIL_EVENT_RX_PACKET_RECEIVED    \
                                      | SL_RAIL_EVENT_RX_PACKET_ABORTED   \
                                      | SL_RAIL_EVENT_RX_FRAME_ERROR      \
                                      | SL_RAIL_EVENT_RX_FIFO_OVERFLOW    \
                                      | SL_RAIL_EVENT_RX_ADDRESS_FILTERED \
                                      | SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED)

// TX Event Bitmasks

/**
 * Occurs when the number of bytes in the transmit FIFO falls below the
 * configured threshold value.
 *
 * This event does not occur on initialization or after resetting the transmit
 * FIFO with \ref sl_rail_reset_fifo().
 *
 * Call \ref sl_rail_get_tx_fifo_space_available() to get the
 * number of bytes available in the transmit FIFO at the time of the callback
 * dispatch. When using this event, the threshold should be set via \ref
 * sl_rail_set_tx_fifo_threshold().
 */
#define SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY (1ULL << SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY_SHIFT)

/**
 * Occurs after a packet has been transmitted.
 *
 * Call \ref sl_rail_get_tx_packet_details()
 * to get information about the packet that was transmitted.
 * @note \ref sl_rail_get_tx_packet_details() is only valid to call during the time frame
 *   of the \ref sl_rail_config_t::events_callback().
 */
#define SL_RAIL_EVENT_TX_PACKET_SENT (1ULL << SL_RAIL_EVENT_TX_PACKET_SENT_SHIFT)

/**
 * Occurs after an Ack packet has been transmitted.
 *
 * Call \ref sl_rail_get_tx_packet_details()
 * to get information about the packet that was transmitted. This event can only occur
 * after calling \ref sl_rail_config_auto_ack().
 *
 * @note \ref sl_rail_get_tx_packet_details() is only valid to call during the time frame
 *   of the \ref sl_rail_config_t::events_callback().
 */
#define SL_RAIL_EVENT_TXACK_PACKET_SENT (1ULL << SL_RAIL_EVENT_TXACK_PACKET_SENT_SHIFT)

/**
 * Occurs when a transmit is aborted by the user.
 *
 * This can happen due to calling \ref sl_rail_idle() or due to a scheduler
 * preemption.
 *
 * @note The Transmit FIFO is left in an indeterminate state and should be
 *   reset prior to reuse for sending a new packet. Contrast this
 *   with \ref SL_RAIL_EVENT_TX_BLOCKED.
 */
#define SL_RAIL_EVENT_TX_ABORTED (1ULL << SL_RAIL_EVENT_TX_ABORTED_SHIFT)

/**
 * Occurs when an Ack transmit is aborted by the user.
 *
 * This event can only
 * occur after calling \ref sl_rail_config_auto_ack(), which can happen due to calling
 * \ref sl_rail_idle() or due to a scheduler preemption.
 */
#define SL_RAIL_EVENT_TXACK_ABORTED (1ULL << SL_RAIL_EVENT_TXACK_ABORTED_SHIFT)

/**
 * Occurs when a transmit is blocked from occurring because
 * \ref sl_rail_enable_tx_hold_off() was called.
 *
 * @note Since the transmit never started, the Transmit FIFO remains intact
 *   after this event -- no packet data was consumed from it. Contrast this
 *   with \ref SL_RAIL_EVENT_TX_ABORTED.
 */
#define SL_RAIL_EVENT_TX_BLOCKED (1ULL << SL_RAIL_EVENT_TX_BLOCKED_SHIFT)

/**
 * Occurs when an Ack transmit is blocked from occurring because
 * \ref sl_rail_enable_tx_hold_off() was called.
 *
 * This event can only occur after calling \ref sl_rail_config_auto_ack().
 */
#define SL_RAIL_EVENT_TXACK_BLOCKED (1ULL << SL_RAIL_EVENT_TXACK_BLOCKED_SHIFT)

/**
 * Occurs when the transmit buffer underflows.
 *
 * This can happen due to the
 * transmitted packet specifying an unintended length based on the current
 * radio configuration or due to \ref sl_rail_write_tx_fifo() calls not keeping up with
 * the transmit rate if the entire packet isn't loaded at once.
 *
 * @note The Transmit FIFO is left in an indeterminate state and should be
 *   reset prior to reuse for sending a new packet. Contrast this
 *   with \ref SL_RAIL_EVENT_TX_BLOCKED.
 */
#define SL_RAIL_EVENT_TX_UNDERFLOW (1ULL << SL_RAIL_EVENT_TX_UNDERFLOW_SHIFT)

/**
 * Occurs when the Ack transmit buffer underflows.
 *
 * This can happen due to the
 * transmitted packet specifying an unintended length based on the current
 * radio configuration or due to \ref sl_rail_write_auto_ack_fifo() not being called at
 * all before an Ack transmit.
 *
 * This event can only occur after calling \ref sl_rail_config_auto_ack().
 */
#define SL_RAIL_EVENT_TXACK_UNDERFLOW (1ULL << SL_RAIL_EVENT_TXACK_UNDERFLOW_SHIFT)

/**
 * Occurs when Carrier Sense Multiple Access (CSMA) or Listen Before Talk (LBT)
 * succeeds.
 *
 * This event can only happen after calling \ref sl_rail_start_cca_csma_tx() or
 * \ref sl_rail_start_cca_lbt_tx() or their scheduled equivalent.
 */
#define SL_RAIL_EVENT_TX_CHANNEL_CLEAR (1ULL << SL_RAIL_EVENT_TX_CHANNEL_CLEAR_SHIFT)

/**
 * Occurs when Carrier Sense Multiple Access (CSMA) or Listen Before Talk (LBT)
 * fails.
 *
 * This event can only happen after calling \ref sl_rail_start_cca_csma_tx() or
 * \ref sl_rail_start_cca_lbt_tx() or their scheduled equivalent.
 *
 * @note Since the transmit never started, the Transmit FIFO remains intact
 *   after this event -- no packet data was consumed from it.
 */
#define SL_RAIL_EVENT_TX_CHANNEL_BUSY (1ULL << SL_RAIL_EVENT_TX_CHANNEL_BUSY_SHIFT)

/**
 * Occurs during CSMA or LBT when an individual Clear Channel Assessment (CCA)
 * check fails, but there are more tries needed before the overall operation
 * completes.
 *
 * This event can occur multiple times based on the configuration
 * of the ongoing CSMA or LBT transmission. It can only happen after
 * calling \ref sl_rail_start_cca_csma_tx() or \ref sl_rail_start_cca_lbt_tx()
 * or their scheduled equivalent.
 */
#define SL_RAIL_EVENT_TX_CCA_RETRY (1ULL << SL_RAIL_EVENT_TX_CCA_RETRY_SHIFT)

/**
 * Occurs when the receiver is enabled to perform a Clear Channel Assessment
 * (CCA) check.
 *
 * This event generally precedes the actual CCA check activation by roughly
 * the \ref sl_rail_state_timing_t::idle_to_rx time (subject to
 * \ref SL_RAIL_MINIMUM_TRANSITION_US). It can
 * occur multiple times based on the configuration of the ongoing CSMA or LBT
 * transmission. It can only happen after calling \ref sl_rail_start_cca_csma_tx()
 * or \ref sl_rail_start_cca_lbt_tx() or their scheduled equivalent.
 */
#define SL_RAIL_EVENT_TX_START_CCA (1ULL << SL_RAIL_EVENT_TX_START_CCA_SHIFT)

/**
 * Occurs when the a Clear Channel Assessment (CCA) check is activated.
 *
 * This event generally follows \ref SL_RAIL_EVENT_TX_START_CCA by roughly
 * the \ref sl_rail_state_timing_t::idle_to_rx time (subject to
 * \ref SL_RAIL_MINIMUM_TRANSITION_US). It can
 * occur multiple times based on the configuration of the ongoing CSMA or LBT
 * transmission. It can only happen after calling \ref sl_rail_start_cca_csma_tx()
 * or \ref sl_rail_start_cca_lbt_tx() or their scheduled equivalent.
 *
 * @note This event requires that \ref SL_RAIL_EVENT_TX_START_CCA also be
 *   enabled. It may not occur if the current CCA's backoff is 0.
 */
#define SL_RAIL_EVENT_TX_CCA_ACTIVATED (1ULL << SL_RAIL_EVENT_TX_CCA_ACTIVATED_SHIFT)

/**
 * Occurs when the radio starts transmitting a normal packet on the air.
 *
 * A start-of-transmit time stamp is captured for this event. It can be
 * retrieved by calling \ref sl_rail_get_tx_time_preamble_start() passing \ref
 * SL_RAIL_TX_STARTED_BYTES for its total_packet_bytes parameter.
 *
 * @note This event does not apply to Ack transmits. Currently there
 *   is no equivalent event or time stamp captured for the start of an
 *   Ack transmit.
 */
#define SL_RAIL_EVENT_TX_STARTED (1ULL << SL_RAIL_EVENT_TX_STARTED_SHIFT)

/**
 * A value to pass as \ref sl_rail_get_tx_time_preamble_start() total_packet_bytes
 * parameter to retrieve the \ref SL_RAIL_EVENT_TX_STARTED time stamp.
 */
#define SL_RAIL_TX_STARTED_BYTES 0U

/**
 * Occurs when a scheduled TX begins turning on the transmitter.
 * This event has the same numerical value as \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED
 * because one cannot schedule both RX and TX simultaneously.
 */
#define SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED (1ULL << SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED_SHIFT)

/**
 * Occurs when the start of a scheduled transmit is missed
 *
 * This can occur if the radio is put to sleep and not woken up with enough time
 * to configure the scheduled transmit event.
 *
 * @note Since the transmit never started, the Transmit FIFO remains intact
 *   after this event -- no packet data was consumed from it.
 */
#define SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED (1ULL << SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED_SHIFT)

/**
 * A mask representing all events that determine the end of a transmitted
 * packet. After a \ref SL_RAIL_STATUS_NO_ERROR return value
 * from one of the transmit functions, exactly one of the following
 * events will occur. When one of these events occurs, a state transition
 * takes place based on the parameter passed to \ref sl_rail_set_tx_transitions().
 * The sl_rail_state_transitions_t::success transition will be followed only
 * if the \ref SL_RAIL_EVENT_TX_PACKET_SENT event occurs. Any of the other
 * events will trigger the \ref sl_rail_state_transitions_t::error transition.
 */
#define SL_RAIL_EVENTS_TX_COMPLETION (SL_RAIL_EVENT_TX_PACKET_SENT    \
                                      | SL_RAIL_EVENT_TX_ABORTED      \
                                      | SL_RAIL_EVENT_TX_BLOCKED      \
                                      | SL_RAIL_EVENT_TX_UNDERFLOW    \
                                      | SL_RAIL_EVENT_TX_CHANNEL_BUSY \
                                      | SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED)

/**
 * A mask representing all events that determine the end of a transmitted
 * Ack packet. After an Ack-requesting receive, exactly one of the
 * following events will occur. When one of these events occurs, a state
 * transition takes place based on the sl_rail_auto_ack_config_t::rx_transitions
 * passed to \ref sl_rail_config_auto_ack(). The receive transitions are used because the
 * transmitted Ack packet is considered a part of the Ack-requesting received
 * packet. The sl_rail_state_transitions_t::success transition will be followed
 * only if the \ref SL_RAIL_EVENT_TXACK_PACKET_SENT event occurs. Any of the other
 * events will trigger the sl_rail_state_transitions_t::error transition.
 */
#define SL_RAIL_EVENTS_TXACK_COMPLETION (SL_RAIL_EVENT_TXACK_PACKET_SENT \
                                         | SL_RAIL_EVENT_TXACK_ABORTED   \
                                         | SL_RAIL_EVENT_TXACK_BLOCKED   \
                                         | SL_RAIL_EVENT_TXACK_UNDERFLOW)

// Scheduler Event Bitmasks

/**
 * Occurs when the scheduler switches away from this configuration.
 *
 * This event will occur in dynamic multiprotocol scenarios each
 * time a protocol is shutting down. When it does occur, it will be
 * the only event passed to \ref sl_rail_config_t::events_callback(). Therefore,
 * to optimize protocol switch time, this event should be handled
 * among the first in that callback, and then the application can return
 * immediately.
 *
 * @note: To minimize protocol switch time, Silicon Labs recommends this event
 *   event being turned off unless it is used.
 */
#define SL_RAIL_EVENT_CONFIG_UNSCHEDULED (1ULL << SL_RAIL_EVENT_CONFIG_UNSCHEDULED_SHIFT)

/**
 * Occurs when the scheduler switches to this configuration.
 *
 * This event will occur in dynamic multiprotocol scenarios each time
 * a protocol is starting up. When it does occur, it will
 * be the only event passed to \ref sl_rail_config_t::events_callback(). Therefore, in
 * order to optimize protocol switch time, this event should be handled among
 * the first in that callback, and then the application can return immediately.
 *
 * @note: To minimize protocol switch time, Silicon Labs recommends this event
 *   event being turned off unless it is used.
 */
#define SL_RAIL_EVENT_CONFIG_SCHEDULED (1ULL << SL_RAIL_EVENT_CONFIG_SCHEDULED_SHIFT)

/**
 * Occurs when the scheduler has a status to report.
 *
 * The exact status can be found with \ref sl_rail_get_scheduler_status().
 * See \ref sl_rail_scheduler_status_t for more details. When this event
 * does occur, it will be the only event passed to \ref sl_rail_config_t::events_callback().
 * Therefore, to optimize protocol switch time, this event should
 * be handled among the first in that callback, and then the application
 * can return immediately.
 *
 * @note \ref sl_rail_get_scheduler_status() is only valid to call during the time frame
 *   of the \ref sl_rail_config_t::events_callback().
 *
 * @note: To minimize protocol switch time, Silicon Labs recommends this event
 *   event being turned off unless it is used.
 */
#define SL_RAIL_EVENT_SCHEDULER_STATUS (1ULL << SL_RAIL_EVENT_SCHEDULER_STATUS_SHIFT)

// Other Event Bitmasks

/**
 * Occurs when the application needs to run a calibration, as
 * determined by the RAIL library.
 *
 * The application determines the opportune time to call \ref sl_rail_calibrate().
 */
#define SL_RAIL_EVENT_CAL_NEEDED (1ULL << SL_RAIL_EVENT_CAL_NEEDED_SHIFT)

/**
 * Occurs when RF energy is sensed from the radio. This event can be used as
 * an alternative to the callback passed as \ref sl_rail_rf_sense_callback_t()
 * or the application polling with \ref sl_rail_is_rf_sensed().
 *
 * @note This event will not occur when waking up from EM4. Prefer
 *   \ref sl_rail_is_rf_sensed() when waking from EM4.
 */
#define SL_RAIL_EVENT_RF_SENSED (1ULL << SL_RAIL_EVENT_RF_SENSED_SHIFT)

/**
 * Occurs when PA protection circuit kicks in.
 */
#define SL_RAIL_EVENT_PA_PROTECTION (1ULL << SL_RAIL_EVENT_PA_PROTECTION_SHIFT)

/**
 * Occurs after enabling the signal detection using \ref sl_rail_ble_enable_signal_detection()
 * or \ref sl_rail_ieee802154_enable_signal_detection() when a signal is detected.
 * This is only used on platforms that support signal identifier, where
 * \ref SL_RAIL_BLE_SUPPORTS_SIGNAL_IDENTIFIER or
 * \ref SL_RAIL_IEEE802154_SUPPORTS_SIGNAL_IDENTIFIER is true.
 */
#define SL_RAIL_EVENT_SIGNAL_DETECTED (1ULL << SL_RAIL_EVENT_SIGNAL_DETECTED_SHIFT)

/**
 * Occurs when a Wi-SUN mode switch packet has been received, after switching to the new PHY.
 *
 * It doesn't occur when a mode switch packet is transmitted.
 *
 * IEEE 802.15.4 option \ref SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH must be enabled for this event to occur.
 *
 * Only available on platforms where \ref SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH is true.
 */
#define SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START (1ULL << SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_START_SHIFT)

/**
 * Occurs when switching back to the original base PHY in effect prior to the Wi-SUN mode switch reception.
 *
 * This typically occurs if no packet is seen within some timeframe after the mode switch packet was received
 * or if the first packet received in the new PHY is aborted, filtered, or fails CRC.
 *
 * IEEE 802.15.4 option \ref SL_RAIL_IEEE802154_G_OPTION_WI_SUN_MODE_SWITCH must be enabled for this event to occur.
 *
 * Only available on platforms where \ref SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH is true.
 */
#define SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END (1ULL << SL_RAIL_EVENT_IEEE802154_MODE_SWITCH_END_SHIFT)

/**
 * Occurs when the sampled RSSI is above the threshold set by
 * \ref sl_rail_set_rssi_detect_threshold().
 */
#define SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD (1ULL << SL_RAIL_EVENT_DETECT_RSSI_THRESHOLD_SHIFT)

/**
 * Occurs when the thermistor has finished its measurement in response to
 * \ref sl_rail_start_thermistor_measurement().
 */
#define SL_RAIL_EVENT_THERMISTOR_DONE (1ULL << SL_RAIL_EVENT_THERMISTOR_DONE_SHIFT)

/**
 * Reserved for possible future use.
 */
#define SL_RAIL_EVENT_RESERVED_49 (1ULL << SL_RAIL_EVENT_RESERVED_49_SHIFT)

/**
 * Occurs when die internal temperature exceeds the temperature threshold subtracted
 * by the cool down parameter from \ref sl_rail_chip_temp_config_t.
 * Transmits are blocked until temperature has cooled enough, indicated by
 * \ref SL_RAIL_EVENT_TEMPERATURE_COOL_DOWN.
 *
 * Only occurs on platforms where \ref SL_RAIL_SUPPORTS_THERMAL_PROTECTION is true.
 */
#define SL_RAIL_EVENT_TEMPERATURE_TOO_HOT (1ULL << SL_RAIL_EVENT_TEMPERATURE_TOO_HOT_SHIFT)

/**
 * Occurs when die internal temperature falls below the temperature threshold subtracted
 * by the cool down parameter from \ref sl_rail_chip_temp_config_t.
 * Transmits are no longer blocked by temperature limitation, indicated by
 * \ref SL_RAIL_EVENT_TEMPERATURE_TOO_HOT.
 *
 * Only occurs on platforms where \ref SL_RAIL_SUPPORTS_THERMAL_PROTECTION is true.
 */
#define SL_RAIL_EVENT_TEMPERATURE_COOL_DOWN (1ULL << SL_RAIL_EVENT_TEMPERATURE_COOL_DOWN_SHIFT)

/**
 * Occurs when the user received a mailbox message.
 */
#define SL_RAIL_EVENT_USER_MBOX (1ULL << SL_RAIL_EVENT_USER_MBOX_SHIFT)

/**
 * A value representing all possible events
 */
#define SL_RAIL_EVENTS_ALL 0xFFFFFFFFFFFFFFFFULL

/** @} */ // end of group Events

/******************************************************************************
 * Data Management Structures
 *****************************************************************************/
/**
 * @addtogroup Data_Management
 * @{
 */

/**
 * Type indicating the needed alignment for RX and TX FIFOs.
 */
typedef uint32_t sl_rail_fifo_buffer_align_t;

///
/// Convenience macro for declaring RAIL FIFO buffers of desired size.
///
/// @param[in] buffer_name The name of the (possibly-static) global
///   for the buffer.
/// @param[in] bytes The size of the buffer in bytes. Refer to chip-specific
///   documentation such as \ref efr32_main for appropriate size constraints.
///
/// For example:
/// @code{.c}
/// #define MY_RX_FIFO_BUFFER_BYTES 512
/// union my_rx_fifo_buffer_u {
///   SL_RAIL_DECLARE_FIFO_BUFFER(aligned, MY_RX_FIFO_BUFFER_BYTES);
///   uint8_t bytes[MY_RX_FIFO_BUFFER_BYTES]; // For byte accessing
/// } my_rx_fifo_buffer;
///
/// sl_rail_config_t my_rail_config = {
///   .rx_fifo_bytes = sizeof(my_rx_fifo_buffer),
///   .rx_fifo_buffer = my_rx_fifo_buffer.aligned,
///   ...
/// };
/// @endcode
///
#define SL_RAIL_DECLARE_FIFO_BUFFER(buffer_name, bytes) \
  sl_rail_fifo_buffer_align_t buffer_name[(bytes) / sizeof(sl_rail_fifo_buffer_align_t)]

/**
 * @def SL_RAIL_MINIMUM_FIFO_BYTES
 * @brief The minimum size in bytes of a receive / transmit / Auto-Ack FIFO.
 */
#define SL_RAIL_MINIMUM_FIFO_BYTES (64U)

/**
 * @def SL_RAIL_MAXIMUM_FIFO_BYTES
 * @brief The maximum size in bytes of a receive / transmit / Auto-Ack FIFO.
 */
#define SL_RAIL_MAXIMUM_FIFO_BYTES (4096U)

/**
 * Type used for Packet Queue entries.
 *
 * RAIL utilizes a receive Packet Queue for packet reception metadata that
 * supplements the receive FIFO, helping \ref sl_rail_get_rx_packet_info() and
 * \ref sl_rail_get_rx_packet_details() provide their information in the order
 * that packets were received.
 */
typedef uint64_t sl_rail_packet_queue_entry_t;

/**
 * @enum sl_rail_tx_data_source_t
 * @brief Transmit data sources supported by RAIL.
 */
SLI_RAIL_ENUM(sl_rail_tx_data_source_t) {
  /**
   * Uses the frame hardware to packetize data.
   */
  SL_RAIL_TX_DATA_SOURCE_PACKET_DATA = 0u,
  /**
   * Uses the multi-level frequency modulation data.
   * @note This is only supported on devices where \ref SL_RAIL_SUPPORTS_MFM
   *   or \ref sl_rail_supports_mfm() are true.
   * @note This feature cannot be used with built-in protocols (802.15.4, BLE,
   *   Z-Wave).
   */
  SL_RAIL_TX_DATA_SOURCE_MFM_DATA = 1u,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_TX_DATA_SOURCE_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_TX_DATA_SOURCE_PACKET_DATA ((sl_rail_tx_data_source_t) SL_RAIL_TX_DATA_SOURCE_PACKET_DATA)
#define SL_RAIL_TX_DATA_SOURCE_MFM_DATA    ((sl_rail_tx_data_source_t) SL_RAIL_TX_DATA_SOURCE_MFM_DATA)
#define SL_RAIL_TX_DATA_SOURCE_COUNT       ((sl_rail_tx_data_source_t) SL_RAIL_TX_DATA_SOURCE_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_rx_data_source_t
 * @brief Receive data sources supported by RAIL.
 *
 * @note Data sources other than \ref SL_RAIL_RX_DATA_SOURCE_PACKET_DATA require use of
 *   \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE.
 */
SLI_RAIL_ENUM(sl_rail_rx_data_source_t) {
  /**
   * Uses the frame hardware to packetize data.
   */
  SL_RAIL_RX_DATA_SOURCE_PACKET_DATA = 0u,
  /**
   * Gets 8-bit data output from the demodulator.
   */
  SL_RAIL_RX_DATA_SOURCE_DEMOD_DATA = 1u,
  /**
   * Gets lower 16 bits of I/Q data provided to the demodulator.
   */
  SL_RAIL_RX_DATA_SOURCE_IQDATA_FILTLSB = 2u,
  /**
   * Gets highest 16 bits of I/Q data provided to the demodulator.
   */
  SL_RAIL_RX_DATA_SOURCE_IQDATA_FILTMSB = 3u,
  /**
   * Gets RX direct mode data output from the demodulator.
   * BCRDEMOD captures bcr_dmod_rawd at the sample rate
   * (faster than the bit rate by the OSR), specifically
   * the demod_samp_rate trigger.
   * Only supported if \ref SL_RAIL_SUPPORTS_RX_DIRECT_MODE_DATA_TO_FIFO is true.
   */
  SL_RAIL_RX_DATA_SOURCE_DIRECT_MODE_DATA = 4u,
  /**
   * Gets synchronous RX direct mode data output from the demodulator.
   * BCRDEMOD_SYNCHRONOUS captures bcr_dmod_rxd_ext at the bit
   * rate (bcr_dmod_bitclk_ext trigger).
   * Only supported if \ref SL_RAIL_SUPPORTS_RX_DIRECT_MODE_DATA_TO_FIFO is true.
   * Only efr32xg23, efr32xg25, or efr32xg28 have this mode.
   */
  SL_RAIL_RX_DATA_SOURCE_DIRECT_SYNCHRONOUS_MODE_DATA = 5u,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_RX_DATA_SOURCE_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_RX_DATA_SOURCE_PACKET_DATA      ((sl_rail_rx_data_source_t) SL_RAIL_RX_DATA_SOURCE_PACKET_DATA)
#define SL_RAIL_RX_DATA_SOURCE_DEMOD_DATA       ((sl_rail_rx_data_source_t) SL_RAIL_RX_DATA_SOURCE_DEMOD_DATA)
#define SL_RAIL_RX_DATA_SOURCE_IQDATA_FILTLSB   ((sl_rail_rx_data_source_t) SL_RAIL_RX_DATA_SOURCE_IQDATA_FILTLSB)
#define SL_RAIL_RX_DATA_SOURCE_IQDATA_FILTMSB   ((sl_rail_rx_data_source_t) SL_RAIL_RX_DATA_SOURCE_IQDATA_FILTMSB)
#define SL_RAIL_RX_DATA_SOURCE_DIRECT_MODE_DATA ((sl_rail_rx_data_source_t) SL_RAIL_RX_DATA_SOURCE_DIRECT_MODE_DATA)
#define SL_RAIL_RX_DATA_SOURCE_DIRECT_SYNCHRONOUS_MODE_DATA ((sl_rail_rx_data_source_t) SL_RAIL_RX_DATA_SOURCE_DIRECT_SYNCHRONOUS_MODE_DATA)
#define SL_RAIL_RX_DATA_SOURCE_COUNT            ((sl_rail_rx_data_source_t) SL_RAIL_RX_DATA_SOURCE_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_data_method_t
 * @brief Methods for the application to provide and retrieve data from RAIL.
 *
 * For Transmit the distinction between \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE
 * and \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE has become more cosmetic than
 * functional, as the \ref sl_rail_write_tx_fifo() and \ref sl_rail_set_tx_fifo_threshold() APIs
 * and related \ref SL_RAIL_EVENT_TX_FIFO_ALMOST_EMPTY event can be used in
 * either mode. For Receive the distinction is functionally important because
 * in \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE rollback occurs automatically for
 * unsuccessfully-received packets (\ref sl_rail_rx_packet_status_t ABORT statuses),
 * flushing their data. In \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE rollback is
 * prevented, leaving the data from unsuccessfully-received packets in the
 * receive FIFO for the application to deal with. This allows for packets
 * larger than the receive FIFO size where automatic rollback would corrupt
 * the receive FIFO.
 */
SLI_RAIL_ENUM(sl_rail_data_method_t) {
  /**
   * Packet-based data method.
   */
  SL_RAIL_DATA_METHOD_PACKET_MODE = 0,
  /**
   * FIFO-based data method.
   */
  SL_RAIL_DATA_METHOD_FIFO_MODE = 1,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_DATA_METHOD_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_DATA_METHOD_PACKET_MODE ((sl_rail_data_method_t) SL_RAIL_DATA_METHOD_PACKET_MODE)
#define SL_RAIL_DATA_METHOD_FIFO_MODE   ((sl_rail_data_method_t) SL_RAIL_DATA_METHOD_FIFO_MODE)
#define SL_RAIL_DATA_METHOD_COUNT       ((sl_rail_data_method_t) SL_RAIL_DATA_METHOD_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @def SL_RAIL_FIFO_THRESHOLD_DISABLED
 * @brief A FIFO threshold value that disables the threshold.
 */
#define SL_RAIL_FIFO_THRESHOLD_DISABLED 0xFFFFU

/**
 * @struct sl_rail_tx_data_config_t
 * @brief RAIL TX data configuration structure
 *
 * Select the transmit data sources and the method the application
 * uses to provide data to RAIL.
 */
typedef struct {
  /**
   * Source of TX Data.
   */
  sl_rail_tx_data_source_t tx_source;
  /**
   * Method of providing transmit data.
   */
  sl_rail_data_method_t tx_method;
} sl_rail_tx_data_config_t;

/**
 * @struct sl_rail_rx_data_config_t
 * @brief RAIL RX data configuration structure
 *
 * Select the receive data sources and the method the application
 * uses to retrieve data from RAIL.
 */
typedef struct {
  /**
   * Source of RX Data.
   */
  sl_rail_rx_data_source_t rx_source;
  /**
   * Method of retrieving receive data.
   */
  sl_rail_data_method_t rx_method;
} sl_rail_rx_data_config_t;

/** @} */ // end of group Data Management

/******************************************************************************
 * General Structures (part 2)
 *****************************************************************************/
/**
 * @addtogroup General
 * @{
 */

/**
 * A pointer to the callback called when some RAIL event(s) occur.
 *
 * @param[in] rail_handle The RAIL handle active when the event occurred.
 * @param[in] events The RAIL event(s) that have occurred.
 */
typedef void (*sl_rail_events_callback_t)(sl_rail_handle_t rail_handle, sl_rail_events_t events);

/**
 * @struct sl_rail_config_t
 * @brief RAIL configuration structure.
 */
typedef struct sl_rail_config {
  /**
   * A pointer to a function, which is called whenever a RAIL event occurs.
   * See the \ref sl_rail_events_t documentation for the list of RAIL events.
   */
  sl_rail_events_callback_t events_callback;
  /**
   * A pointer for application convenience that is opaque to RAIL.
   */
  void *p_opaque_handle_0;
  /**
   * Another pointer for application convenience that is opaque to RAIL.
   */
  void *p_opaque_handle_1;
  /**
   * A value for application convenience that is opaque to RAIL.
   */
  sl_rail_opaque_value_t opaque_value;
  /**
   * The number of entries in the rx_packet_queue.
   *
   * May be set to 0 if application wants to defer specifying this at
   * \ref sl_rail_init() time to a later \ref sl_rail_set_rx_packet_queue() call.
   * This queue is used by RAIL for storing received packet metadata in the
   * order received. Its size indicates how many packets can be held by RAIL
   * for the application to process before \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW
   * might occur.
   *
   * \ref sl_rail_init() may update this to a smaller size based on chip constraints.
   */
  uint16_t rx_packet_queue_entries;
  /**
   * The number of bytes in the rx_fifo_buffer.
   *
   * May be set to 0 if application wants to defer specifying this at
   * \ref sl_rail_init() time to a later \ref sl_rail_set_rx_fifo() call.
   *
   * \ref sl_rail_init() may update this to a smaller size based on chip constraints.
   */
  uint16_t rx_fifo_bytes;
  /**
   * The number of bytes in the tx_fifo_buffer. Must be no more than
   * \ref SL_RAIL_MAXIMUM_FIFO_BYTES.
   *
   * May be set to 0 if application wants to defer specifying this at
   * \ref sl_rail_init() time to a later \ref sl_rail_set_tx_fifo() call.
   */
  uint16_t tx_fifo_bytes;
  /**
   * The number of data bytes already placed in the tx_fifo_buffer.
   *
   * Set to 0 if the tx_fifo_buffer is initially empty, otherwise set it
   * to the length of the pre-populated data already placed in the buffer.
   * If this is larger than the FIFO's actual size, the FIFO will be filled
   * only up to its actual size.
   */
  uint16_t tx_fifo_init_bytes;
  /**
   * Pointer to the receive Packet Queue to use.
   *
   * May be set to NULL if application wants to defer specifying this at
   * \ref sl_rail_init() time to a later \ref sl_rail_set_rx_packet_queue() call.
   *
   * The receive Packet Queue must be allocated in global read-write memory
   * with at least the number of entries indicated by rx_packet_queue_entries and
   * with the appropriate alignment dictated by this type. It must persist
   * for the duration of RAIL usage or until \ref sl_rail_set_rx_packet_queue()
   * is called to change it.
   *
   * Enabled receive packet completion events add an entry to this queue
   * which is removed when the sl_rail_config_t::events_callback() returns or
   * when \ref sl_rail_release_rx_packet() is called after \ref sl_rail_hold_rx_packet().
   *
   * Earlier versions of RAIL used a built-in fixed-size queue of 16 entries.
   */
  sl_rail_packet_queue_entry_t *p_rx_packet_queue;
  /**
   * Pointer to the receive FIFO buffer to use.
   *
   * May be set to NULL if application wants to defer specifying this at
   * \ref sl_rail_init() time to a later \ref sl_rail_set_rx_fifo() call.
   *
   * The receive FIFO buffer must be allocated in global read-write memory of
   * at least the size indicated by rx_fifo_bytes and with the appropriate
   * alignment dictated by this type. It must persist for the duration of RAIL
   * usage or until \ref sl_rail_set_rx_fifo() is called to change it.
   *
   * Earlier versions of RAIL provided a default built-in receive FIFO of
   * 512 bytes.
   */
  sl_rail_fifo_buffer_align_t *p_rx_fifo_buffer;
  /**
   * Pointer to the transmit FIFO buffer to use.
   *
   * May be set to NULL if application wants to defer specifying this at
   * \ref sl_rail_init() time to a later \ref sl_rail_set_tx_fifo() call.
   *
   * The transmit FIFO buffer must be allocated in global read-write memory of
   * at least the size indicated by tx_fifo_bytes and with the appropriate
   * alignment dictated by this type. It must persist for the duration of RAIL
   * usage or until \ref sl_rail_set_tx_fifo() is called to change it.
   */
  sl_rail_fifo_buffer_align_t *p_tx_fifo_buffer;
} sl_rail_config_t;

/** @} */ // end of group General

/******************************************************************************
 * PA Power Amplifier Structures
 *****************************************************************************/
/**
 * @addtogroup PA Power Amplifier (PA)
 * @ingroup Transmit
 * @{
 */

/**
 * The transmit power in deci-dBm units (e.g., 4.5 dBm -> 45 deci-dBm).
 * On EFR32 they can range from \ref SL_RAIL_TX_POWER_MIN to
 * \ref SL_RAIL_TX_POWER_MAX.
 */
typedef int16_t sl_rail_tx_power_t;

/**
 * The maximum valid value for a \ref sl_rail_tx_power_t.
 */
#define SL_RAIL_TX_POWER_MAX ((sl_rail_tx_power_t)0x7FFF)
/**
 * The minimum valid value for a \ref sl_rail_tx_power_t.
 */
#define SL_RAIL_TX_POWER_MIN ((sl_rail_tx_power_t)0x8000)

/**
 * The maximum power in deci-dBm the curve supports.
 */
#define SL_RAIL_TX_POWER_CURVE_DEFAULT_MAX ((sl_rail_tx_power_t)200)
/**
 * The increment step in deci-dBm for calculating power level.
 */
#define SL_RAIL_TX_POWER_CURVE_DEFAULT_INCREMENT ((sl_rail_tx_power_t)40)

/**
 * mV are used for all TX power voltage values.
 * TX power voltages take and return voltages multiplied by this factor.
 */
#define SL_RAIL_TX_POWER_VOLTAGE_SCALING_FACTOR 1000

/**
 * deci-dBm are used for all TX power dBm values.
 * All dBm inputs to TX power functions take dBm power times this factor.
 */
#define SL_RAIL_TX_POWER_DBM_SCALING_FACTOR 10

#ifndef DOXYGEN_UNDOCUMENTED

/**
 * Raw power levels used directly by \ref sli_rail_get_tx_power() and \ref sli_rail_set_tx_power() where a higher
 * numerical value corresponds to a higher output power. These are referred to
 * as 'raw (values/units)'. On EFR32, they can range from one of \ref
 * SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MIN, \ref SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MIN, or
 * \ref SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_HP_MIN to one of \ref
 * SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_LP_MAX, \ref SLI_RAIL_TX_POWER_LEVEL_2P4_GHZ_HP_MAX, and \ref
 * SLI_RAIL_TX_POWER_LEVEL_SUB_GHZ_HP_MAX, respectively, depending on the selected \ref
 * sl_rail_tx_power_mode_t.
 */
typedef uint8_t sli_rail_tx_power_level_t;

/**
 * Invalid \ref sli_rail_tx_power_level_t value returned when an error occurs
 * with \ref sli_rail_get_tx_power().
 */
#define SLI_RAIL_TX_POWER_LEVEL_INVALID (255U)

/**
 * Sentinel value that can be passed to \ref sli_rail_set_tx_power() to set
 * the highest power level available on the current PA, regardless
 * of which one is selected.
 */
#define SLI_RAIL_TX_POWER_LEVEL_MAX (254U)

#endif//DOXYGEN_UNDOCUMENTED

/**
 * PA power setting type used directly by the \ref PA_Power_Conversions
 * component which is decoded to the actual hardware register value(s).
 */
typedef uint32_t sl_rail_pa_power_setting_t;

/**
 * An unsupported power setting used with the \ref PA_Power_Conversions component.
 * when the device does not support the dBm to power setting mapping table.
 */
#define SL_RAIL_TX_PA_POWER_SETTING_UNSUPPORTED     (0U)

/**
 * @struct sl_rail_tx_power_setting_entry_t
 *
 * @brief A structure containing power-setting information for a deci-dBm power.
 */
typedef struct sl_rail_tx_power_setting_entry {
  /**
   * PowerSetting information corresponds to curr_pa_power_ddbm.
   */
  sl_rail_pa_power_setting_t pa_power_setting;
  /**
   * Current power (in deci-dBm).
   */
  sl_rail_tx_power_t curr_pa_power_ddbm;
} sl_rail_tx_power_setting_entry_t;

/**
 * @enum sl_rail_tx_pa_mode_t
 * @brief Suitable type for the supported PA modes.
 *
 * The PA modes correspond to the different on-chip PAs that are
 * physically available. For more information about the power and performance
 * characteristics of a given amplifier, see the data sheet.
 *
 * @note Some platforms support only a subset of these PAs.
 */
SLI_RAIL_ENUM(sl_rail_tx_pa_mode_t) {
  /**
   * PA for 2.4 GHz.
   */
  SL_RAIL_TX_PA_MODE_2P4_GHZ = 0U,
  /**
   * PA for Sub-GHz.
   */
  SL_RAIL_TX_PA_MODE_SUB_GHZ = 1U,
  /**
   * PA for Sub-GHz OFDM.
   */
  SL_RAIL_TX_PA_MODE_SUB_GHZ_OFDM = 2U,
  /**
   * An invalid Power Amplifier (PA) mode in the RAIL TX configuration.
   *
   * This constant is used to indicate an invalid or uninitialized state for the
   * TX Power Amplifier mode. It can be used as a placeholder or error value
   * when a valid PA mode is not set or recognized.
   *
   * @note Ensure that this value is not used in actual transmission configurations.
   */
  SL_RAIL_TX_PA_MODE_INVALID = 3U,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
// Refer to rail_chip_specific.h for per-platform defines of supported ones.
#define SL_RAIL_TX_PA_MODE_INVALID      ((sl_rail_tx_pa_mode_t)(SL_RAIL_TX_PA_MODE_INVALID))
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @enum sl_rail_tx_power_mode_t
 * @brief An enumeration of the power modes.
 *
 * The power modes correspond to the different on-chip PAs that
 * are available. For more information about the power and performance
 * characteristics of a given amplifier, see the data sheet.
 */
SLI_RAIL_ENUM(sl_rail_tx_power_mode_t) {
  /**
   *  High-power 2.4 GHz amplifier
   *  EFR32xG21: up to 20 dBm, raw values: 1-180
   *  EFR32xG22: up to  6 dBm, raw values: 1-128
   *  EFR32xG24: up to 20 dBm, raw values: 0-180, or
   *             up to 10 dBm, raw values: 0-90
   *  EFR32xG26: same as EFR32xG24
   *  EFR32xG27: up to  6 dBm, raw values: 1-128
   *  EFR32xG28: up to 10 dBm, raw values: 0-240
   *  Not supported on other platforms.
   */
  SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP = 0U,
  /**
   *  Mid-power 2.4 GHz amplifier
   *  EFR32xG21: up to 10 dBm, raw values: 1-90
   *  Not supported on other platforms.
   */
  SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP = 1U,
  /**
   *  Low-power 2.4 GHz amplifier
   *  EFR32xG21: up to 0 dBm, raw values: 1-64
   *  EFR32xG22: up to 0 dBm, raw values: 1-16
   *  EFR32xG24: up to 0 dBm, raw values: 1-16
   *  EFR32xG26: same as EFR32xG24
   *  EFR32xG27: up to 0 dBm, raw values: 1-16
   *  Not supported on other platforms.
   */
  SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP = 2U,
  /**
   *  Low-Low-power 2.4 GHz amplifier
   *  Not currently supported on any EFR32 platform.
   */
  SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP = 3U,
  /**
   *  Select the highest 2.4 GHz power PA available on the current chip.
   */
  SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST = 4U,
  /**
   *  PA for all Sub-GHz dBm values in range, using \ref
   *  sl_rail_pa_power_setting_t table.
   *  Only supported on platforms with \ref
   *  SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE (e.g., EFR32xG25).
   */
  SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE = 5U,
  /**
   *  High-power Sub-GHz amplifier (Class D mode)
   *  Supported on EFR32xG23 and EFR32xG28.
   *  Not supported other Sub-GHz-incapable platforms or those with \ref
   *  SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE.
   */
  SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP = 6U,
  /**
   *  Mid-power Sub-GHz amplifier
   *  Supported only on EFR32xG23 and EFR32xG28.
   *  Not supported other Sub-GHz-incapable platforms or those with \ref
   *  SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE.
   */
  SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP = 7U,
  /**
   *  Low-power Sub-GHz amplifier
   *  Supported only on EFR32xG23 and EFR32xG28.
   *  Not supported other Sub-GHz-incapable platforms or those with \ref
   *  SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE.
   */
  SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP = 8U,
  /**
   *  Low-Low-power Sub-GHz amplifier
   *  Supported only on EFR32xG23 and EFR32xG28.
   *  Not supported other Sub-GHz-incapable platforms or those with \ref
   *  SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE.
   */
  SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP = 9U,
  /**
   *  Select the highest Sub-GHz power PA available on the current chip.
   */
  SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST = 10U,
  /**
   *  PA for all OFDM Sub-GHz dBm values in range, using \ref
   *  sl_rail_pa_power_setting_t table.
   *  Supported only on platforms with both \ref
   *  SL_RAIL_SUPPORTS_DBM_POWERSETTING_MAPPING_TABLE and \ref
   *  SL_RAIL_SUPPORTS_OFDM_PA (e.g., EFR32xG25).
   */
  SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE = 11U,
  /**
   * Invalid amplifier Selection. Must be last.
   */
  SL_RAIL_TX_POWER_MODE_NONE
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
// Refer to rail_chip_specific.h for per-platform defines of supported ones.
#define SL_RAIL_TX_POWER_MODE_NONE ((sl_rail_tx_power_mode_t) SL_RAIL_TX_POWER_MODE_NONE)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @def SL_RAIL_TX_POWER_MODE_NAMES
 * @brief The names of the TX power modes.
 *
 * A list of the names for the TX power modes on EFR32 parts. This
 * macro is useful for test applications and debugging output.
 */
#define SL_RAIL_TX_POWER_MODE_NAMES {                   \
    "SL_RAIL_TX_POWER_MODE_2P4_GHZ_HP",                 \
    "SL_RAIL_TX_POWER_MODE_2P4_GHZ_MP",                 \
    "SL_RAIL_TX_POWER_MODE_2P4_GHZ_LP",                 \
    "SL_RAIL_TX_POWER_MODE_2P4_GHZ_LLP",                \
    "SL_RAIL_TX_POWER_MODE_2P4_GHZ_HIGHEST",            \
    "SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE", \
    "SL_RAIL_TX_POWER_MODE_SUB_GHZ_HP",                 \
    "SL_RAIL_TX_POWER_MODE_SUB_GHZ_MP",                 \
    "SL_RAIL_TX_POWER_MODE_SUB_GHZ_LP",                 \
    "SL_RAIL_TX_POWER_MODE_SUB_GHZ_LLP",                \
    "SL_RAIL_TX_POWER_MODE_SUB_GHZ_HIGHEST",            \
    "SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE", \
    "SL_RAIL_TX_POWER_MODE_NONE"                        \
}

/**
 * @struct sl_rail_tx_power_config_t
 *
 * @brief A structure containing values used to initialize the power amplifiers.
 */
typedef struct sli_rail_tx_power_config {
  /**
   * TX power mode.
   */
  sl_rail_tx_power_mode_t mode;
  /**
   * Power amplifier supply voltage in mV, generally:
   * DCDC supply ~ 1800 mV (1.8 V)
   * Battery supply ~ 3300 mV (3.3 V)
   */
  uint16_t voltage_mv;
  /**
   * The amount of time to spend ramping for TX in microseconds.
   */
  uint16_t ramp_time_us;
} sl_rail_tx_power_config_t;

/**
 * Convenience macro for any OFDM mapping table mode.
 */
#define SL_RAIL_POWER_MODE_IS_DBM_POWERSETTING_MAPPING_TABLE_OFDM(x) \
  ((x) == SL_RAIL_TX_POWER_MODE_OFDM_PA_POWERSETTING_TABLE)
/**
 * Convenience macro for any Sub-GHz mapping table mode.
 */
#define SL_RAIL_POWER_MODE_IS_DBM_POWERSETTING_MAPPING_TABLE_SUB_GHZ(x) \
  ((x) == SL_RAIL_TX_POWER_MODE_SUB_GHZ_POWERSETTING_TABLE)
/**
 * Convenience macro for any OFDM mode.
 */
#define SL_RAIL_POWER_MODE_IS_ANY_OFDM(x) \
  SL_RAIL_POWER_MODE_IS_DBM_POWERSETTING_MAPPING_TABLE_OFDM(x)

/** @} */ // PA Power Amplifier (PA)

/******************************************************************************
 * Packet Trace Interface (PTI) Structures
 *****************************************************************************/
/**
 * @addtogroup PTI PTI Packet Trace
 * @{
 *
 * These enumerations and structures are used with RAIL PTI API. EFR32 supports
 * SPI and UART PTI and is configurable in terms of baud rates and PTI
 * pin locations.
 */

/**
 * A channel type enumeration.
 */
SLI_RAIL_ENUM(sl_rail_pti_mode_t) {
  /**
   * Turn PTI off entirely.
   */
  SL_RAIL_PTI_MODE_DISABLED = 0,
  /**
   * 8-bit SPI mode.
   */
  SL_RAIL_PTI_MODE_SPI = 1,
  /**
   * 8-bit UART mode.
   */
  SL_RAIL_PTI_MODE_UART = 2,
  /**
   * 9-bit UART mode.
   */
  SL_RAIL_PTI_MODE_UART_ONEWIRE = 3,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_PTI_MODE_DISABLED     ((sl_rail_pti_mode_t) SL_RAIL_PTI_MODE_DISABLED)
#define SL_RAIL_PTI_MODE_SPI          ((sl_rail_pti_mode_t) SL_RAIL_PTI_MODE_SPI)
#define SL_RAIL_PTI_MODE_UART         ((sl_rail_pti_mode_t) SL_RAIL_PTI_MODE_UART)
#define SL_RAIL_PTI_MODE_UART_ONEWIRE ((sl_rail_pti_mode_t) SL_RAIL_PTI_MODE_UART_ONEWIRE)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_pti_config_t
 * @brief A configuration for PTI.
 */
typedef struct sl_rail_pti_config {
  /**
   * Packet Trace mode (UART or SPI).
   */
  sl_rail_pti_mode_t mode;
  /**
   * Output baudrate for PTI in Hz.
   */
  uint32_t baud;
  /**
   * Reserved for future use.
   */
  uint8_t reserved_0;
  /**
   * Data output (DOUT) GPIO port.
   */
  uint8_t dout_port;
  /**
   * Data output (DOUT) GPIO pin.
   */
  uint8_t dout_pin;
  /**
   * Reserved for future use.
   */
  uint8_t reserved_1;
  /**
   * Data clock (DCLK) GPIO port. Only used in SPI mode.
   */
  uint8_t dclk_port;
  /**
   * Data clock (DCLK) GPIO pin. Only used in SPI mode.
   */
  uint8_t dclk_pin;
  /**
   * Reserved for future use.
   */
  uint8_t reserved_2;
  /**
   * Data frame (DFRAME) GPIO port.
   */
  uint8_t dframe_port;
  /**
   * Data frame (DFRAME) GPIO pin.
   */
  uint8_t dframe_pin;
} sl_rail_pti_config_t;

/**
 * @enum sl_rail_pti_protocol_t
 * @brief The protocol that RAIL outputs via the Packet Trace Interface (PTI).
 */
SLI_RAIL_ENUM(sl_rail_pti_protocol_t) {
  /**
   * PTI output for a custom protocol.
   */
  SL_RAIL_PTI_PROTOCOL_CUSTOM = 0,
  /**
   * PTI output for the Thread protocol.
   */
  SL_RAIL_PTI_PROTOCOL_THREAD = 2,
  /**
   * PTI output for the Bluetooth Smart protocol.
   */
  SL_RAIL_PTI_PROTOCOL_BLE = 3,
  /**
   * PTI output for the Connect protocol.
   */
  SL_RAIL_PTI_PROTOCOL_CONNECT = 4,
  /**
   * PTI output for the Zigbee protocol.
   */
  SL_RAIL_PTI_PROTOCOL_ZIGBEE = 5,
  /**
   * PTI output for the Z-Wave protocol.
   */
  SL_RAIL_PTI_PROTOCOL_ZWAVE = 6,
  /**
   * PTI output for the Wi-SUN protocol.
   */
  SL_RAIL_PTI_PROTOCOL_WI_SUN = 7,
  /**
   * PTI output for a custom protocol using a built-in 802.15.4 radio config.
   */
  SL_RAIL_PTI_PROTOCOL_802154 = 8,
  /**
   * PTI output for Sidewalk protocol.
   */
  SL_RAIL_PTI_PROTOCOL_SIDEWALK = 9,
  /**
   * PTI output for the Bluetooth Classic protocol.
   */
  SL_RAIL_PTI_PROTOCOL_BTC = 10,
#ifndef DOXYGEN_UNDOCUMENTED
  /**
   * PTI output for the ANT protocol.
   */
  SL_RAIL_PTI_PROTOCOL_ANT = 11,
#endif // DOXYGEN_UNDOCUMENTED
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_PTI_PROTOCOL_CUSTOM   ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_CUSTOM)
#define SL_RAIL_PTI_PROTOCOL_THREAD   ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_THREAD)
#define SL_RAIL_PTI_PROTOCOL_BLE      ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_BLE)
#define SL_RAIL_PTI_PROTOCOL_CONNECT  ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_CONNECT)
#define SL_RAIL_PTI_PROTOCOL_ZIGBEE   ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_ZIGBEE)
#define SL_RAIL_PTI_PROTOCOL_ZWAVE    ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_ZWAVE)
#define SL_RAIL_PTI_PROTOCOL_WI_SUN   ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_WI_SUN)
#define SL_RAIL_PTI_PROTOCOL_802154   ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_802154)
#define SL_RAIL_PTI_PROTOCOL_SIDEWALK ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_SIDEWALK)
#define SL_RAIL_PTI_PROTOCOL_BTC      ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_BTC)
#define SL_RAIL_PTI_PROTOCOL_ANT      ((sl_rail_pti_protocol_t) SL_RAIL_PTI_PROTOCOL_ANT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // end of group PTI

/******************************************************************************
 * Radio Configuration Structures
 *****************************************************************************/
/**
 * @addtogroup Radio_Configuration
 * @{
 */

/**
 * @brief Pointer to a radio configuration array.
 *
 * The radio configuration properly configures the
 * radio for operation on a protocol. These configurations are very
 * chip-specific should not be created or edited by hand.
 */
typedef const uint32_t *sl_rail_radio_config_t;

/**
 * @struct sl_rail_frame_type_t
 * @brief Configures if there is a frame type in your frame and the lengths of
 *   each frame. The number of bits set in the mask determines the number of
 *   elements in p_frame_byte_lengths. A maximum of 8 different frame types may be specified.
 *
 * This structure is provided for the radio calculator.
 */
typedef struct sl_rail_frame_type {
  /**
   * A pointer to an array of frame byte lengths for each frame type.
   * The number of elements in this array should be equal to the number of
   * frame types. The memory to which p_frame_byte_lengths points should not
   * change location or be modified.
   */
  uint16_t *p_frame_byte_lengths;
  /**
   * Zero-indexed byte offset location of the byte containing the frame type field.
   */
  uint8_t offset_bytes;
  /**
   * A bitmask of the frame type field, which determines a number of frames expected
   * based on the number of bits set. No more than 3 bits can be set in the mask and
   * they must be contiguous ones. For example, if the highest three bits of the byte
   * specified by offset constitute the frame type, then mask should be 0xE0,
   * which has 3 bits set, indicating 8 possible frame types.
   */
  uint8_t mask;
  /**
   * A bitmask that marks if each frame is valid or should be filtered. Frame type
   * 0 corresponds to the lowest bit in isValid. If the frame is filtered, a
   * \ref SL_RAIL_EVENT_RX_PACKET_ABORTED will be raised.
   */
  uint8_t is_valid;
  /**
   * A bitmask that marks if each frame should have the address filter applied.
   * Frame type 0 corresponds to the least significant bit in address_filter.
   */
  uint8_t address_filter_mask;
} sl_rail_frame_type_t;

/**
 * @def SL_RAIL_SET_FIXED_LENGTH_INVALID
 * @brief An invalid return value when calling \ref sl_rail_set_fixed_length().
 *
 * An invalid return value when calling \ref sl_rail_set_fixed_length() while the radio is
 * not in fixed-length mode.
 */
#define SL_RAIL_SET_FIXED_LENGTH_INVALID (0xFFFFU)

/**
 * @struct sl_rail_channel_config_entry_attr_t
 * @brief A channel configuration entry attribute structure. Items listed
 *   are designed to be altered and updated during run-time.
 */
typedef struct sl_rail_channel_config_entry_attr sl_rail_channel_config_entry_attr_t;

/**
 * @enum sl_rail_channel_config_entry_type_t
 * @brief Define if the channel support using concurrent PHY during channel
 *   hopping. \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC and \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT
 *   can only be used if the channel supports it.
 */
SLI_RAIL_ENUM(sl_rail_channel_config_entry_type_t) {
  /**
   * Not a concurrent PHY.
   */
  SL_RAIL_CH_TYPE_NORMAL = 0,
  /**
   * Base concurrent PHY.
   */
  SL_RAIL_CH_TYPE_CONC_BASE = 1,
  /**
   * Virtual concurrent PHY.
   */
  SL_RAIL_CH_TYPE_CONC_VIRTUAL = 2,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_CH_TYPE_NORMAL       ((sl_rail_channel_config_entry_type_t) SL_RAIL_CH_TYPE_NORMAL)
#define SL_RAIL_CH_TYPE_CONC_BASE    ((sl_rail_channel_config_entry_type_t) SL_RAIL_CH_TYPE_CONC_BASE)
#define SL_RAIL_CH_TYPE_CONC_VIRTUAL ((sl_rail_channel_config_entry_type_t) SL_RAIL_CH_TYPE_CONC_VIRTUAL)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @def SL_RAIL_RADIO_CONFIG_ENABLE_CONC_PHY
 * @brief Indicates this version of RAIL supports concurrent PHY information in
 *   radio configurator output. Needed for backwards compatibility.
 */
#define SL_RAIL_RADIO_CONFIG_ENABLE_CONC_PHY 1

/**
 * @def SL_RAIL_RADIO_CONFIG_ENABLE_STACK_INFO
 * @brief Indicates this version of RAIL supports stack info feature in
 *   radio configurator output. Needed for backwards compatibility.
 */
#define SL_RAIL_RADIO_CONFIG_ENABLE_STACK_INFO

/**
 * @struct sl_rail_alternate_phy_t
 * @brief Alternate PHY configuration entry structure, which gathers some info
 *   on the alternate PHY in the context of concurrent mode.
 */
typedef struct sl_rail_alternate_phy {
  /**
   * A base frequency in Hz of this channel set.
   */
  uint32_t base_frequency_hz;
  /**
   * A channel spacing in Hz of this channel set.
   */
  uint32_t channel_spacing_hz;
  /**
   * The number of channels (and not the channel number !)
   */
  uint16_t number_of_channels;
  /**
   * minimum IF for the alternate PHY in kHz.
   */
  uint16_t min_if_khz;
  /**
   * minimum IF for the base PHY in kHz.
   */
  uint16_t min_base_if_khz;
  /**
   * Indicates that OFDM modem is used by this alternate PHY.
   */
  bool is_ofdm_modem;
  /**
   * Rate info of the alternate PHY.
   */
  uint32_t rate_info;
  /**
   * Used to adjust the AGC for CCA between hard and soft modems.
   */
  uint32_t hw_modem_agc_ctrl1;
} sl_rail_alternate_phy_t;

/**
 * @def SL_RAIL_CHANNEL_INVALID
 * @brief A value representing an invalid channel, used for APIs that return or
 *   output a channel when none can be identified.
 */
#define SL_RAIL_CHANNEL_INVALID (0xFFFFU)

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * @def SL_RAIL_DCDC_RETIMING_NUM_BAND_SHIFT
 * @brief The number of bits to shift to get the number of frequency bands.
 */
#define SL_RAIL_DCDC_RETIMING_NUM_BAND_SHIFT 2

/**
 * @def SL_RAIL_DCDC_RETIMING_NUM_BAND
 * @brief The number of frequency bands.
 *
 * This is used to determine the number of frequency bands in the
 * sl_rail_dcdc_retiming_info_t structure.
 */
#define SL_RAIL_DCDC_RETIMING_NUM_BAND (1U << SL_RAIL_DCDC_RETIMING_NUM_BAND_SHIFT)

/**
 * @struct sl_rail_dcdc_retiming_info_t
 * @brief This structure is used to store information relevant to the DCDC
 *   retiming Nd1 value for the PHY configuration currently loaded in memory.
 * @note
 *   This structure is part of the RAIL (Radio Abstraction Interface Layer)
 *   library provided by Silicon Labs. Modifications to the contents of this
 *   structure should only be made if explicitly advised by Silicon Labs, as
 *   changes may affect the functionality or compatibility of the library.
 */
typedef struct sl_rail_dcdc_retiming_info {
  /**
   * Number of frequency bands.
   */
  uint8_t number_of_frequency_bands;
  /**
   * Nd1 values.
   */
  uint8_t dcdc_nd1[SL_RAIL_DCDC_RETIMING_NUM_BAND];
} sl_rail_dcdc_retiming_info_t;

/**
 * @struct sl_rail_tx_ir_cal_config_t
 * @brief This structure is used to store information relevant to Tx IR cal for
 *   the PHY configuration currently loaded in memory.
 * @note
 *   This structure is part of the RAIL (Radio Abstraction Interface Layer)
 *   library provided by Silicon Labs. Modifications to the contents of this
 *   structure should only be made if explicitly advised by Silicon Labs, as
 *   changes may affect the functionality or compatibility of the library.
 */
typedef struct sl_rail_tx_ir_cal_config {
  /**
   * Softmodem frequency.
   */
  uint16_t softmodem_frequency_hz;
  /**
   * Bin tone offset.
   */
  uint8_t bin_tone_offset;
  /**
   * Int2 ratio.
   */
  uint8_t int_2_ratio;
  /**
   * Int2 gain.
   */
  uint8_t int_2_gain;
} sl_rail_tx_ir_cal_config_t;

/**
 * @struct sl_rail_modem_tx_compensation_t
 * @brief This structure is used to store information relevant to modem tx
 *   compensation for the PHY configuration currently loaded in memory.
 * @note
 *   This structure is part of the RAIL (Radio Abstraction Interface Layer)
 *   library provided by Silicon Labs. Modifications to the contents of this
 *   structure should only be made if explicitly advised by Silicon Labs, as
 *   changes may affect the functionality or compatibility of the library.
 */
typedef struct sl_rail_modem_tx_compensation {
  /**
   * Modindex and length.
   */
  uint16_t modindex_and_length;
  /**
   * TxBrDen.
   */
  uint8_t tx_br_den;
  /**
   * PPM.
   */
  int8_t ppm;
  /**
   * TxBrNum.
   */
  uint16_t tx_br_num;
} sl_rail_modem_tx_compensation_t;

/**
 * @struct sl_rail_hfxo_retiming_band_t
 * @brief This structure is used to store information relevant to the HFXO
 *   retiming for the PHY configuration currently loaded in memory.
 * @note
 *   This structure is part of the RAIL (Radio Abstraction Interface Layer)
 *   library provided by Silicon Labs. Modifications to the contents of this
 *   structure should only be made if explicitly advised by Silicon Labs, as
 *   changes may affect the functionality or compatibility of the library.
 */
typedef struct {
  /**
   * Upper range of Lo frequency.
   */
  uint16_t lo_frequency_upper_range;
  /**
   * 1 if retiming should be enabled on this range.
   */
  uint8_t  valid;
  /**
   * SMUX divider value.
   */
  uint8_t  s_mux_div_value;
  /**
   * HFXO/HFRCO retiming limit low.
   */
  uint8_t  limit_low;
  /**
   * HFXO/HFRCO retiming limit high.
   */
  uint8_t  limit_high;
} sl_rail_hfxo_retiming_band_t;

/**
 * @struct sl_rail_hfxo_retiming_info_t
 * @brief This structure is used to store information relevant to the HFXO retiming
 *   for the PHY configuration currently loaded in memory.
 * @note
 *   This structure is part of the RAIL (Radio Abstraction Interface Layer)
 *   library provided by Silicon Labs. Modifications to the contents of this
 *   structure should only be made if explicitly advised by Silicon Labs, as
 *   changes may affect the functionality or compatibility of the library.
 */
typedef struct sl_rail_hfxo_retiming_info {
  /**
   * DPLL clock setting associated with this structure.
   */
  uint32_t dpll_clock;
  /**
   * The number of \ref sl_rail_hfxo_retiming_band_t elements in an array
   * of such appended to the \ref sl_rail_hfxo_retiming_config_t structure
   * by the radio calculator.
   */
  uint8_t number_of_frequency_bands;
  /**
   * The byte offset from the start of the \ref sl_rail_hfxo_retiming_config_t
   * encompassing this structure to the first element of the corresponding \ref
   * sl_rail_hfxo_retiming_band_t structure for this retiming info element.
   */
  uint8_t offset;
  /**
   * Alignment pad to align to 32-bit boundary.
   */
  uint8_t reserved[2];
} sl_rail_hfxo_retiming_info_t;

/**
 * @struct sl_rail_hfxo_retiming_config_t
 * @brief This structure is used to store information relevant to the HFXO
 *   retiming for the PHY configuration currently loaded in memory.
 * @note
 *   This structure is part of the RAIL (Radio Abstraction Interface Layer)
 *   library provided by Silicon Labs. Modifications to the contents of this
 *   structure should only be made if explicitly advised by Silicon Labs, as
 *   changes may affect the functionality or compatibility of the library.
 */
typedef struct sl_rail_hfxo_retiming_config {
  /**
   * The number of \ref sl_rail_hfxo_retiming_info_t elements in the
   * hfxo_retiming_info array.
   */
  uint8_t number_of_elements;
  /**
   * Alignment pad to align to 32-bit boundary.
   */
  uint8_t reserved[3];
  /**
   * Retiming frequency band info.
   */
  sl_rail_hfxo_retiming_info_t hfxo_retiming_info[1];
} sl_rail_hfxo_retiming_config_t;

/**
 * @struct sl_rail_phy_info_t
 * @brief This structure is used to store information necessary for PHY
 *   configuration.
 * @note
 *   This structure is part of the RAIL (Radio Abstraction Interface Layer)
 *   library provided by Silicon Labs. Modifications to the contents of this
 *   structure should only be made if explicitly advised by Silicon Labs, as
 *   changes may affect the functionality or compatibility of the library.
 */
typedef struct sl_rail_phy_info {
  /**
   * Version of the phyinfo structure.
   */
  uint32_t version;
  /**
   * FreqOffsetFactor for a given PHY, FXP <16.16>.
   */
  uint32_t frequency_offset_factor;
  /**
   * Pointer to frame type settings.
   */
  uint8_t *p_frame_type_config;
  /**
   * Pointer to IR cal settings.
   */
  uint8_t *p_ir_cal_config;
  /**
   * Pointer to timing info.
   */
  uint32_t *p_timing_config;
  /**
   * Antenna Diversity RX auto configuration.
   */
  uint32_t ant_div_rx_auto_config;
  /**
   * Calculator assisted denominator for SRC1 | adcVcoDivisor.
   */
  uint32_t src_1_denominator;
  /**
   * Calculator assisted denominator for SRC2.
   */
  uint32_t src_2_denominator;
  /**
   * TX Baudrate from calculator.
   */
  uint32_t tx_baud_rate;
  /**
   * (rssiAdjustDb << 16) | (baudPerSymbol << 8) | bitsPerSymbol.
   */
  uint32_t rate_info;
  /**
   * Info for SYNTH calculations.
   */
  uint32_t synth_cache;
  /**
   * Pointer to DCDC retiming info.
   */
  sl_rail_dcdc_retiming_info_t *p_dcdc_retiming_info;
  /**
   * Pointer to HFXO retiming info.
   */
  sl_rail_hfxo_retiming_config_t *p_hfxo_retiming_config;
  /**
   * Z-Wave channel hop timing.
   */
  uint32_t channel_hop_timing;
  /**
   * PA configuration parameters.
   */
  uint32_t pa_params;
  /**
   * Theoretical max sleep time (us).
   */
  uint32_t psm_max_sleep_us;
  /**
   * RX Baudrate from calculator.
   */
  uint32_t rx_baud_rate;
  /**
   * RFFPLL configuration from calculator.
   */
  uint32_t *p_rffpll_config;
  /**
   * Pointer to Tx calibration config.
   */
  sl_rail_tx_ir_cal_config_t *p_tx_ir_cal_config;
  /**
   * Pointer to tx modem compensation values.
   */
  sl_rail_modem_tx_compensation_t *p_modem_tx_compensation;
  /**
   * TRECS preamble bits in sync word.
   */
  uint32_t trecs_pre_bits_to_sync;
} sl_rail_phy_info_t;
#endif // DOXYGEN_UNDOCUMENTED

/**
 * @struct sl_rail_channel_config_group_t
 * @brief A channel configuration group structure, which defines a group of
 *   radio configurations.
 *
 * This structure allows grouping multiple radio PHY configurations together,
 * enabling switching between them during runtime.
 */
typedef struct sl_rail_channel_config_group {
  /**
   * The array of radio PHY configurations in this group. Each configuration
   * defines the parameters for a specific radio setup.
   */
  sl_rail_radio_config_t phy_config_group;
  /**
   * The number of radio PHY configurations (words) in the phy_config_group array.
   */
  uint32_t phy_config_group_length;
} sl_rail_channel_config_group_t;

/**
 * @typedef sl_rail_packet_duration_t
 * @brief A pointer to a function that processes packet duration on air.
 */
typedef uint32_t (*sl_rail_packet_duration_t)(uint8_t bit_rate, uint16_t number_of_bytes);

#ifdef SL_RAIL_3_0
/**
 * @struct sl_rail_channel_config_entry_t
 * @brief A channel configuration entry structure, which defines a channel range
 *   and parameters across which a corresponding radio configuration is valid.
 *
 * operating frequency hz = base_frequency_hz
 *   + channel_spacing_hz * (channel - physical_channel_offset);
 */
typedef struct sl_rail_channel_config_entry {
  /**
   * A pointer to a structure containing PHY specific information.
   */
  const sl_rail_phy_info_t *p_phy_info;
  /**
   * The minimum radio configuration to apply to the base
   * configuration for this channel set.
   */
  sl_rail_radio_config_t phy_config_delta_add;
  /**
   * Length in words of phy_config_delta_add array.
   */
  uint32_t phy_config_delta_add_length;
  /**
   * A pointer to a structure containing radio configuration groups used for
   * fast switching between different radio configurations.
   */
  sl_rail_channel_config_group_t *p_groups;
  /* Number of \ref sl_rail_channel_config_group_t groups. */
  uint32_t number_of_groups;
  /**
   * A base frequency in Hz of this channel set.
   */
  uint32_t base_frequency_hz;
  /**
   * A channel spacing in Hz of this channel set.
   */
  uint32_t channel_spacing_hz;
  /**
   * The offset to subtract from the logical
   * channel to align them with the zero based physical channels which are
   * relative to base_frequency_hz. (i.e., By default ch 0 = base freq, but if
   * offset = 11, ch 11 = base freq.)
   */
  uint16_t physical_channel_offset;
  /**
   * The first valid RAIL channel number for this channel set.
   */
  uint16_t channel_number_start;
  /**
   * The last valid RAIL channel number for this channel set.
   */
  uint16_t channel_number_end;
  /**
   * The maximum power allowed in this channel set.
   */
  sl_rail_tx_power_t max_power_ddbm;
  /**
   * A pointer to a structure containing attributes specific to this
   * channel set.
   */
  sl_rail_channel_config_entry_attr_t *p_attr;
  /**
   * Indicates channel config type.
   */
  sl_rail_channel_config_entry_type_t entry_type;
  /**
   * Alignment pad to align to 32-bit boundary.
   */
  uint8_t reserved[3];
  /**
   * Array containing information according to the \ref sl_rail_pti_protocol_t in
   * the first byte of this array. The first 2 fields are common to all
   * protocols and accessible by RAIL, others are ignored by RAIL
   * and only used by the application. Common fields are listed in
   * \ref sl_rail_stack_info_common_t.
   */
  const uint8_t *p_stack_info;
  /**
   * Pointer to alternate PHY.
   */
  sl_rail_alternate_phy_t *p_alternate_phy;
  /**
   * A pointer to a function that processes packet duration on air.
   */
  sl_rail_packet_duration_t packet_duration_on_air;
} sl_rail_channel_config_entry_t;

/**
 * @struct sl_rail_channel_config_t
 * @brief A channel configuration structure, which defines the channel meaning
 *   when a channel number is passed into a RAIL function, e.g., \ref sl_rail_start_tx()
 *   and \ref sl_rail_start_rx().
 *
 * A \ref sl_rail_channel_config_t structure defines the channel scheme that an
 * application uses when registered in \ref sl_rail_config_channels().
 */
typedef struct sl_rail_channel_config {
  /**
   * Common base radio configuration between two or more protocols.
   * NULL if there is a single protocol.
   */
  sl_rail_radio_config_t phy_config_common_base;
  /**
   * Length in words of phy_config_common_base array.
   */
  uint32_t phy_config_common_base_length;
  /**
   * Base radio configuration for the corresponding
   * channel configuration entries.
   */
  sl_rail_radio_config_t phy_config_base;
  /**
   * Length in words of the phy_config_base array.
   */
  uint32_t phy_config_base_length;
  /**
   * Pointer to an array of \ref sl_rail_channel_config_entry_t entries.
   */
  const sl_rail_channel_config_entry_t * p_entries;
  /**
   * Number of \ref sl_rail_channel_config_entry_t entries.
   */
  uint32_t number_of_entries;
  /**
   * Signature for this structure. Only used on modules.
   */
  uint32_t signature;
  /**
   * Crystal Frequency for the channel config.
   */
  uint32_t xtal_frequency_hz;
} sl_rail_channel_config_t;

#else // !SL_RAIL_3_0

/**
 * @struct sl_rail_channel_config_entry_t
 * @brief A channel configuration entry structure, which defines a channel range
 *   and parameters across which a corresponding radio configuration is valid.
 *
 * operating frequency hz = base_frequency_hz
 *   + channel_spacing_hz * (channel - physical_channel_offset);
 */
typedef struct sl_rail_channel_config_entry {
  /**
   * The minimum radio configuration to apply to the base
   * configuration for this channel set.
   */
  sl_rail_radio_config_t phy_config_delta_add;
  /**
   * A base frequency in Hz of this channel set.
   */
  uint32_t base_frequency_hz;
  /**
   * A channel spacing in Hz of this channel set.
   */
  uint32_t channel_spacing_hz;
  /**
   * The offset to subtract from the logical
   * channel to align them with the zero based physical channels which are
   * relative to base_frequency_hz. (i.e., By default ch 0 = base freq, but if
   * offset = 11, ch 11 = base freq.)
   */
  uint16_t physical_channel_offset;
  /**
   * The first valid RAIL channel number for this channel set.
   */
  uint16_t channel_number_start;
  /**
   * The last valid RAIL channel number for this channel set.
   */
  uint16_t channel_number_end;
  /**
   * The maximum power allowed in this channel set.
   */
  sl_rail_tx_power_t max_power_ddbm;
  /**
   * A pointer to a structure containing attributes specific to this
   * channel set.
   */
  sl_rail_channel_config_entry_attr_t *p_attr;
  /**
   * Indicates channel config type.
   */
  sl_rail_channel_config_entry_type_t entry_type;
  /**
   * to align to 32-bit boundary.
   */
  uint8_t reserved[3];
  /**
   * Array containing information according to the \ref sl_rail_pti_protocol_t in
   * the first byte of this array. The first 2 fields are common to all
   * protocols and accessible by RAIL, others are ignored by RAIL
   * and only used by the application. Common fields are listed in
   * \ref sl_rail_stack_info_common_t.
   */
  const uint8_t *p_stack_info;
  /**
   * Pointer to alternate PHY.
   */
  sl_rail_alternate_phy_t *p_alternate_phy;
} sl_rail_channel_config_entry_t;

///
/// @struct sl_rail_channel_config_t
/// @brief A channel configuration structure, which defines the channel meaning
///   when a channel number is passed into a RAIL function, e.g., \ref sl_rail_start_tx()
///   and \ref sl_rail_start_rx().
///
/// A \ref sl_rail_channel_config_t structure defines the channel scheme that an
/// application uses when registered in \ref sl_rail_config_channels().
///
/// These are a few examples of different channel configurations:
/// @code{.c}
/// // 21 channels starting at 2.45 GHz with channel spacing of 1 MHz
/// // ... generated by Simplicity Studio (i.e., rail_config.c) ...
/// const uint32_t generated[] = { ... };
/// sl_rail_channel_config_entry_attr_t generated_entry_attr = { ... };
/// const sl_rail_channel_config_entry_t generated_channels[] = {
///   {
///     .phy_config_delta_add = NULL, // Add this to default configuration for this entry
///     .base_frequency_hz = 2450000000,
///     .channel_spacing_hz = 1000000,
///     .physical_channel_offset = 0,
///     .channel_number_start = 0,
///     .channel_number_end = 20,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX,
///     .p_attr = &generated_entry_attr,
///   },
/// };
/// const sl_rail_channel_config_t generated_channel_config = {
///   .phy_config_base = generated, // Default radio configuration for all entries
///   .phy_config_delta_subtract = NULL, // Subtract this to restore the default configuration
///   .p_entries = generated_channels,
///   .length = 1, // There are this many channel configuration entries
/// };
/// const sl_rail_channel_config_t *channel_configs[] = {
///   &generated_channel_config,
///   NULL,
/// };
/// // ... in main code ...
/// // Associate a specific channel configuration with a particular RAIL instance.
/// sl_rail_config_channels(rail_handle, channel_configs[0]);
///
/// // 4 nonlinear channels
/// // ... in rail_config.c ...
/// const uint32_t generated[] = { ... };
/// sl_rail_channel_config_entry_attr_t generated_entry_attr = { ... };
/// const sl_rail_channel_config_entry_t generated_channels[] = {
///   {
///     .phy_config_delta_add = NULL, // Add this to default configuration for this entry
///     .base_frequency_hz = 910123456,
///     .channel_spacing_hz = 0,
///     .physical_channel_offset = 0,
///     .channel_number_start = 0,
///     .channel_number_end = 0,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX,
///     .p_attr = &generated_entry_attr,
///   },
///   {
///     .phy_config_delta_add = NULL,
///     .base_frequency_hz = 911654789,
///     .channel_spacing_hz = 0,
///     .physical_channel_offset = 0, // Since ch spacing = 0, offset can be 0
///     .channel_number_start = 1,
///     .channel_number_end = 1,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX,
///     .p_attr = &generated_entry_attr,
///   },
///   {
///     .phy_config_delta_add = NULL,
///     .base_frequency_hz = 912321456,
///     .channel_spacing_hz = 100000,
///     .physical_channel_offset = 2, // Since ch spacing != 0, offset = 2
///     .channel_number_start = 2, // ch 2 = base_frequency
///     .channel_number_end = 2,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX,
///     .p_attr = &generated_entry_attr,
///   },
///   {
///     .phy_config_delta_add = NULL,
///     .base_frequency_hz = 913147852,
///     .channel_spacing_hz = 0,
///     .physical_channel_offset = 0,
///     .channel_number_start = 3,
///     .channel_number_end = 3,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX,
///     .p_attr = &generated_entry_attr,
///   },
/// };
/// const sl_rail_channel_config_t generated_channel_config = {
///   .phy_config_base = generated, // Default radio configuration for all entries
///   .phy_config_delta_subtract = NULL, // Subtract this to restore the default configuration
///   .p_entries = generated_channels,
///   .length = 4, // There are this many channel configuration entries
/// };
/// const sl_rail_channel_config_t *channel_configs[] = {
///   &generated_channel_config,
///   NULL,
/// };
/// // ... in main code ...
/// // Associate a specific channel configuration with a particular RAIL instance.
/// sl_rail_config_channels(rail_handle, channel_configs[0]);
///
/// // Multiple radio configurations
/// // ... in rail_config.c ...
/// const uint32_t generated_0[] = { ... };
/// sl_rail_channel_config_entry_attr_t generated_0_entry_attr = { ... };
/// const sl_rail_channel_config_entry_t generated_0_channels[] = {
///   {
///     .phy_config_delta_add = NULL, // Add this to the default configuration for this entry
///     .base_frequency_hz = 2450000000,
///     .channel_spacing_hz = 1000000,
///     .physical_channel_offset = 0,
///     .channel_number_start = 0,
///     .channel_number_end = 20,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX,
///     .p_attr = &generated_0_entry_attr,
///   },
/// };
/// const sl_rail_channel_config_t generated_0_channel_config = {
///   .phy_config_base = generated_0, // Default radio configuration for all entries
///   .phy_config_delta_subtract = NULL, // Subtract this to restore default configuration
///   .p_entries = generated_0_channels,
///   .length = 1 // There are this many channel configuration entries
/// };
/// const uint32_t generated_1[] = { ... };
/// sl_rail_channel_config_entry_attr_t generated_1_entry_attr = { ... };
/// const sl_rail_channel_config_entry_t generated_1_channels[] = {
///   {
///     .phy_config_delta_add = NULL,
///     .base_frequency_hz = 2450000000,
///     .channel_spacing_hz = 1000000,
///     .physical_channel_offset = 0,
///     .channel_number_start = 0,
///     .channel_number_end = 20,
///     .max_power_ddbm = -100, // Use this entry when TX power <= -10dBm
///     .p_attr = &generated_1_entry_attr,
///   },
///   {
///     .phy_config_delta_add = NULL,
///     .base_frequency_hz = 2450000000,
///     .channel_spacing_hz = 1000000,
///     .physical_channel_offset = 0,
///     .channel_number_start = 0,
///     .channel_number_end = 20,
///     .max_power_ddbm = 15, // Use this entry when TX power > -10dBm
///                      // and TX power <= 1.5dBm
///     .p_attr = &generated_1_entry_attr,
///   },
///   {
///     .phy_config_delta_add = NULL,
///     .base_frequency_hz = 2450000000,
///     .channel_spacing_hz = 1000000,
///     .physical_channel_offset = 0,
///     .channel_number_start = 0,
///     .channel_number_end = 20,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX, // Use this entry when TX power > 1.5dBm
///     .p_attr = &generated_1_entry_attr,
///   },
/// };
/// const sl_rail_channel_config_t generated_1_channel_config = {
///   .phy_config_base = generated_1,
///   .phy_config_delta_subtract = NULL,
///   .p_entries = generated_1_channels,
///   .length = 3,
/// };
/// const uint32_t generated_2[] = { ... };
/// sl_rail_channel_config_entry_attr_t generated_2_entry_attr = { ... };
/// const sl_rail_channel_config_entry_t generated_2_channels[] = {
///   {
///     .phy_config_delta_add = NULL,
///     .base_frequency_hz = 2450000000,
///     .channel_spacing_hz = 1000000,
///     .physical_channel_offset = 0,
///     .channel_number_start = 0,
///     .channel_number_end = 20,
///     .max_power_ddbm = SL_RAIL_TX_POWER_MAX,
///     .p_attr = &generated_2_entry_attr,
///   },
/// };
/// const sl_rail_channel_config_t generated_2_channel_config = {
///   .phy_config_base = generated_2,
///   .phy_config_delta_subtract = NULL,
///   .p_entries = generated_2_channels,
///   .length = 1,
/// };
/// const sl_rail_channel_config_t *channel_configs[] = {
///   &generated_0_channel_config,
///   &generated_1_channel_config,
///   &generated_2_channel_config,
///   NULL,
/// };
/// // ... in main code ...
/// // Create a unique RAIL handle for each unique channel configuration.
/// sl_rail_handle_t rail_handle_0 = SL_RAIL_EFR32_HANDLE;
/// sl_rail_init(&rail_handle_0, &rail_config_0, NULL);
/// sl_rail_handle_t rail_handle_1 = SL_RAIL_EFR32_HANDLE;
/// sl_rail_init(&rail_handle_1, &rail_config_1, NULL);
/// sl_rail_handle_t rail_handle_2 = SL_RAIL_EFR32_HANDLE;
/// sl_rail_init(&rail_handle_2, &rail_config_2, NULL);
/// // Associate each channel configuration with its corresponding RAIL handle.
/// sl_rail_config_channels(rail_handle_0, channel_configs[0]);
/// sl_rail_config_channels(rail_handle_1, channel_configs[1]);
/// sl_rail_config_channels(rail_handle_2, channel_configs[2]);
/// // Use a RAIL handle and channel to access the desired channel configuration entry.
/// sl_rail_set_tx_power_dbm(rail_handle_1, 100); // set 10.0 dBm TX power
/// sl_rail_start_rx(rail_handle_1, 0, &sched_info); // RX using generated_1_channels[2]
/// sl_rail_set_tx_power_dbm(rail_handle_1, 0); // set 0 dBm TX power
/// sl_rail_start_rx(rail_handle_1, 0, &sched_info); // RX using generated_1_channels[1]
/// sl_rail_start_rx(rail_handle_2, 0, &sched_info); // RX using generated_2_channels[0]
/// @endcode
///
typedef struct sl_rail_channel_config {
  /**
   * Base radio configuration for the corresponding
   * channel configuration entries.
   */
  sl_rail_radio_config_t phy_config_base;
  /**
   * Minimum radio configuration to restore
   * channel entries back to base configuration.
   */
  sl_rail_radio_config_t phy_config_delta_subtract;
  /**
   * Pointer to an array of \ref sl_rail_channel_config_entry_t entries.
   */
  const sl_rail_channel_config_entry_t *p_entries;
  /**
   * Number of \ref sl_rail_channel_config_entry_t entries.
   */
  uint32_t number_of_entries;
  /**
   * Signature for this structure. Only used on modules.
   */
  uint32_t signature;
  /**
   * Crystal Frequency for the channel config.
   */
  uint32_t xtal_frequency_hz;
} sl_rail_channel_config_t;

#endif // SL_RAIL_3_0

/**
 * @struct sl_rail_channel_metadata_t
 * @brief Container for individual channel metadata.
 */
typedef struct sl_rail_channel_metadata {
  /**
   * Channel number.
   */
  uint16_t channel;
  /**
   * Word alignment.
   */
  uint16_t reserved;
  /**
   * Channel frequency, in Hz.
   */
  uint32_t frequency_hz;
} sl_rail_channel_metadata_t;

/**
 * @struct sl_rail_stack_info_common_t
 * @brief Stack info fields common to all protocols.
 */
typedef struct sl_rail_stack_info_common {
  /**
   * Protocol ID as per \ref sl_rail_pti_protocol_t.
   */
  sl_rail_pti_protocol_t protocol_id;
  /**
   * PHY Id depending on the protocol_id value.
   */
  uint8_t phy_id;
} sl_rail_stack_info_common_t;

/**
 * @typedef sl_rail_radio_config_changed_callback_t
 * @brief A pointer to a function called whenever a radio configuration change occurs.
 *
 * @param[in] rail_handle The corresponding real RAIL instance handle.
 * @param[in] p_entry A pointer to the radio configuration entry being changed to.
 */
typedef void (*sl_rail_radio_config_changed_callback_t)(sl_rail_handle_t rail_handle,
                                                        const sl_rail_channel_config_entry_t *p_entry);

/** @} */ // end of group Radio_Configuration

/******************************************************************************
 * State Transitions
 *****************************************************************************/
/**
 * @addtogroup State_Transitions
 * @{
 */

/**
 * @typedef sl_rail_transition_time_t
 * @brief Suitable type for the supported transition time range.
 *
 * Refer to platform-specific \ref SL_RAIL_MINIMUM_TRANSITION_US and
 * \ref SL_RAIL_MAXIMUM_TRANSITION_US for the valid range of this type.
 */
typedef uint32_t sl_rail_transition_time_t;

/**
 * @def SL_RAIL_TRANSITION_TIME_KEEP
 * @brief A value to use in \ref sl_rail_state_timing_t fields when
 *   calling \ref sl_rail_set_state_timing() to keep that timing
 *   parameter at it current setting.
 */
#define SL_RAIL_TRANSITION_TIME_KEEP ((sl_rail_transition_time_t) -1)

/**
 * @struct sl_rail_state_timing_t
 * @brief A timing configuration structure for the RAIL State Machine.
 *
 * Configure the timings of the radio state transitions for common situations.
 * All of the listed timings are in microseconds. Transitions from an active
 * radio state to idle are not configurable, and will always happen as fast
 * as possible.
 * No timing value can exceed platform-specific \ref SL_RAIL_MAXIMUM_TRANSITION_US.
 * Use \ref SL_RAIL_TRANSITION_TIME_KEEP to keep an existing setting.
 *
 * For idle_to_rx, idle_to_tx, rx_to_tx, tx_to_rx, and tx_to_tx a value of 0 for the
 * transition time means that the specified transition should happen as fast
 * as possible, even if the timing cannot be as consistent. Otherwise, the
 * timing value cannot be below the platform-specific \ref SL_RAIL_MINIMUM_TRANSITION_US.
 *
 * For idle_to_tx, rx_to_tx, and tx_to_tx setting a longer \ref
 * sl_rail_set_tx_pa_ramp_time() may result in a larger minimum value.
 *
 * For rxsearch_timeout and tx_to_rxsearch_timeout, there is no minimum value.
 * A value of 0 disables the feature, functioning as an infinite timeout.
 */
typedef struct sl_rail_state_timing {
  /**
   * Transition time from IDLE to RX.
   */
  sl_rail_transition_time_t idle_to_rx;
  /**
   * Transition time from TX to RX.
   */
  sl_rail_transition_time_t tx_to_rx;
  /**
   * Transition time from IDLE to TX.
   */
  sl_rail_transition_time_t idle_to_tx;
  /**
   * Transition time from RX packet to TX.
   */
  sl_rail_transition_time_t rx_to_tx;
  /**
   * Length of time the radio will search for a packet when coming from idle or RX.
   */
  sl_rail_transition_time_t rxsearch_timeout;
  /**
   * Length of time the radio will search for a packet when coming from TX.
   */
  sl_rail_transition_time_t tx_to_rxsearch_timeout;
  /**
   * Transition time from TX packet to TX.
   */
  sl_rail_transition_time_t tx_to_tx;
} sl_rail_state_timing_t;

/**
 * @enum sl_rail_radio_state_t
 * @brief The state of the radio.
 */
SLI_RAIL_ENUM(sl_rail_radio_state_t) {
  /**
   * Radio is inactive.
   */
  SL_RAIL_RF_STATE_INACTIVE = 0u,
  /**
   * Radio is either idle or, in combination with the RX and TX states,
   * receiving or transmitting a frame.
   */
  SL_RAIL_RF_STATE_ACTIVE = (1u << 0),
  /**
   * Radio is in receive.
   */
  SL_RAIL_RF_STATE_RX = (1u << 1),
  /**
   * Radio is in transmit.
   */
  SL_RAIL_RF_STATE_TX = (1u << 2),
  /**
   * Radio is idle.
   */
  SL_RAIL_RF_STATE_IDLE = (SL_RAIL_RF_STATE_ACTIVE),
  /**
   * Radio is actively receiving a frame.
   */
  SL_RAIL_RF_STATE_RX_ACTIVE = (SL_RAIL_RF_STATE_RX | SL_RAIL_RF_STATE_ACTIVE),
  /**
   * Radio is actively transmitting a frame.
   */
  SL_RAIL_RF_STATE_TX_ACTIVE = (SL_RAIL_RF_STATE_TX | SL_RAIL_RF_STATE_ACTIVE),
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_RF_STATE_INACTIVE  ((sl_rail_radio_state_t) SL_RAIL_RF_STATE_INACTIVE)
#define SL_RAIL_RF_STATE_ACTIVE    ((sl_rail_radio_state_t) SL_RAIL_RF_STATE_ACTIVE)
#define SL_RAIL_RF_STATE_RX        ((sl_rail_radio_state_t) SL_RAIL_RF_STATE_RX)
#define SL_RAIL_RF_STATE_TX        ((sl_rail_radio_state_t) SL_RAIL_RF_STATE_TX)
#define SL_RAIL_RF_STATE_IDLE      ((sl_rail_radio_state_t) SL_RAIL_RF_STATE_IDLE)
#define SL_RAIL_RF_STATE_RX_ACTIVE ((sl_rail_radio_state_t) SL_RAIL_RF_STATE_RX_ACTIVE)
#define SL_RAIL_RF_STATE_TX_ACTIVE ((sl_rail_radio_state_t) SL_RAIL_RF_STATE_TX_ACTIVE)
#endif//DOXYGEN_SHOULD_SKIP_THIS

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * @enum sl_rail_radio_state_efr32_t
 * @brief Detailed EFR32 Radio state machine states.
 */
SLI_RAIL_ENUM(sl_rail_radio_state_efr32_t) {
  /**
   * Radio is off.
   */
  SL_RAIL_RAC_STATE_OFF = 0u,
  /**
   * Radio is enabling the receiver.
   */
  SL_RAIL_RAC_STATE_RXWARM = 1u,
  /**
   * Radio is listening for incoming frames.
   */
  SL_RAIL_RAC_STATE_RXSEARCH = 2u,
  /**
   * Radio is receiving a frame.
   */
  SL_RAIL_RAC_STATE_RXFRAME = 3u,
  /**
   * Radio is powering down receiver and going to OFF state.
   */
  SL_RAIL_RAC_STATE_RXPD = 4u,
  /**
   * Radio is going back to receive mode after receiving a frame.
   */
  SL_RAIL_RAC_STATE_RX2RX = 5u,
  /**
   * Received data was lost due to full receive buffer.
   */
  SL_RAIL_RAC_STATE_RXOVERFLOW = 6u,
  /**
   * Radio is disabling receiver and enabling transmitter.
   */
  SL_RAIL_RAC_STATE_RX2TX = 7u,
  /**
   * Radio is enabling transmitter.
   */
  SL_RAIL_RAC_STATE_TXWARM = 8u,
  /**
   * Radio is transmitting data.
   */
  SL_RAIL_RAC_STATE_TX = 9u,
  /**
   * Radio is powering down transmitter and going to OFF state.
   */
  SL_RAIL_RAC_STATE_TXPD = 10u,
  /**
   * Radio is disabling transmitter and enabling reception.
   */
  SL_RAIL_RAC_STATE_TX2RX = 11u,
  /**
   * Radio is preparing a transmission after the previous transmission was ended.
   */
  SL_RAIL_RAC_STATE_TX2TX = 12u,
  /**
   * Radio is powering down and going to OFF state.
   */
  SL_RAIL_RAC_STATE_SHUTDOWN = 13u,
  /**
   * Radio power-on-reset state (EFR32xG22 and later).
   */
  SL_RAIL_RAC_STATE_POR = 14u,
  /**
   * Invalid Radio state, must be the last entry.
   */
  SL_RAIL_RAC_STATE_NONE
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_RAC_STATE_OFF          ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_OFF)
#define SL_RAIL_RAC_STATE_RXWARM       ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_RXWARM)
#define SL_RAIL_RAC_STATE_RXSEARCH     ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_RXSEARCH)
#define SL_RAIL_RAC_STATE_RXFRAME      ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_RXFRAME)
#define SL_RAIL_RAC_STATE_RXPD         ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_RXPD)
#define SL_RAIL_RAC_STATE_RX2RX        ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_RX2RX)
#define SL_RAIL_RAC_STATE_RXOVERFLOW   ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_RXOVERFLOW)
#define SL_RAIL_RAC_STATE_RX2TX        ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_RX2TX)
#define SL_RAIL_RAC_STATE_TXWARM       ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_TXWARM)
#define SL_RAIL_RAC_STATE_TX           ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_TX)
#define SL_RAIL_RAC_STATE_TXPD         ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_TXPD)
#define SL_RAIL_RAC_STATE_TX2RX        ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_TX2RX)
#define SL_RAIL_RAC_STATE_TX2TX        ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_TX2TX)
#define SL_RAIL_RAC_STATE_SHUTDOWN     ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_SHUTDOWN)
#define SL_RAIL_RAC_STATE_POR          ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_POR)
#define SL_RAIL_RAC_STATE_NONE         ((sl_rail_radio_state_efr32_t) SL_RAIL_RAC_STATE_NONE)
#endif//DOXYGEN_SHOULD_SKIP_THIS
#endif//DOXYGEN_UNDOCUMENTED

/**
 * @struct sl_rail_state_transitions_t
 * @brief Used to specify radio states to transition to on success or failure.
 */
typedef struct sl_rail_state_transitions {
  /**
   * Indicate the state the radio should return to after a successful action.
   */
  sl_rail_radio_state_t success;
  /**
   * Indicate the state the radio should return to after an error.
   */
  sl_rail_radio_state_t error;
} sl_rail_state_transitions_t;

/**
 * @enum sl_rail_radio_state_detail_t
 * @brief The detailed state of the radio.
 *
 * The three radio state bits \ref SL_RAIL_RF_STATE_DETAIL_IDLE_STATE, \ref
 * SL_RAIL_RF_STATE_DETAIL_RX_STATE, and \ref SL_RAIL_RF_STATE_DETAIL_TX_STATE
 * comprise a set of mutually exclusive core radio states. Only one (or none)
 * of these bits can be set at a time. Otherwise, the value is invalid.
 *
 * The precise meaning of each of these three core bits, when set, depends on
 * the value of the two bits \ref SL_RAIL_RF_STATE_DETAIL_TRANSITION and \ref
 * SL_RAIL_RF_STATE_DETAIL_ACTIVE. When \ref SL_RAIL_RF_STATE_DETAIL_TRANSITION is
 * set, the radio is transitioning into the core radio state corresponding
 * to the set state bit. When it is clear, the radio is already in the core
 * radio state that corresponds to the set state bit. When \ref
 * SL_RAIL_RF_STATE_DETAIL_ACTIVE is set, the radio is actively transmitting or
 * receiving. When it is clear, the radio is not actively transmitting or receiving.
 * This bit will always be clear when \ref SL_RAIL_RF_STATE_DETAIL_IDLE_STATE is
 * set, and will always be set when \ref SL_RAIL_RF_STATE_DETAIL_TX_STATE is set.
 * Otherwise, the value is invalid.
 *
 * The bit \ref SL_RAIL_RF_STATE_DETAIL_NO_FRAMES is set if the radio is currently
 * operating with frame detection disabled, and clear otherwise. The bit \ref
 * SL_RAIL_RF_STATE_DETAIL_LBT_SHIFT is set if an LBT/CSMA operation
 * (e.g., performing CCA) is currently ongoing, and clear otherwise.
 */
SLI_RAIL_ENUM(sl_rail_radio_state_detail_t) {
  /**
   * Shift position of \ref SL_RAIL_RF_STATE_DETAIL_IDLE_STATE bit.
   */
  SL_RAIL_RF_STATE_DETAIL_IDLE_STATE_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_RF_STATE_DETAIL_RX_STATE bit.
   */
  SL_RAIL_RF_STATE_DETAIL_RX_STATE_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_RF_STATE_DETAIL_TX_STATE bit.
   */
  SL_RAIL_RF_STATE_DETAIL_TX_STATE_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_RF_STATE_DETAIL_TRANSITION bit.
   */
  SL_RAIL_RF_STATE_DETAIL_TRANSITION_SHIFT = 3,
  /**
   * Shift position of \ref SL_RAIL_RF_STATE_DETAIL_ACTIVE bit.
   */
  SL_RAIL_RF_STATE_DETAIL_ACTIVE_SHIFT = 4,
  /**
   * Shift position of \ref SL_RAIL_RF_STATE_DETAIL_NO_FRAMES bit.
   */
  SL_RAIL_RF_STATE_DETAIL_NO_FRAMES_SHIFT = 5,
  /**
   * Shift position of \ref SL_RAIL_RF_STATE_DETAIL_LBT bit.
   */
  SL_RAIL_RF_STATE_DETAIL_LBT_SHIFT = 6,
};

/**
 * Radio is inactive.
 */
#define SL_RAIL_RF_STATE_DETAIL_INACTIVE (0U)
/**
 * Radio is in or headed to the idle state.
 */
#define SL_RAIL_RF_STATE_DETAIL_IDLE_STATE (1U << SL_RAIL_RF_STATE_DETAIL_IDLE_STATE_SHIFT)
/**
 * Radio is in or headed to the receive state.
 */
#define SL_RAIL_RF_STATE_DETAIL_RX_STATE (1U << SL_RAIL_RF_STATE_DETAIL_RX_STATE_SHIFT)
/**
 * Radio is in or headed to the transmit state.
 */
#define SL_RAIL_RF_STATE_DETAIL_TX_STATE (1U << SL_RAIL_RF_STATE_DETAIL_TX_STATE_SHIFT)
/**
 * Radio is headed to the idle, receive, or transmit state.
 */
#define SL_RAIL_RF_STATE_DETAIL_TRANSITION (1U << SL_RAIL_RF_STATE_DETAIL_TRANSITION_SHIFT)
/**
 * Radio is actively transmitting or receiving.
 */
#define SL_RAIL_RF_STATE_DETAIL_ACTIVE (1U << SL_RAIL_RF_STATE_DETAIL_ACTIVE_SHIFT)
/**
 * Radio has frame detect disabled.
 */
#define SL_RAIL_RF_STATE_DETAIL_NO_FRAMES (1U << SL_RAIL_RF_STATE_DETAIL_NO_FRAMES_SHIFT)
/**
 * LBT/CSMA operation is currently ongoing.
 */
#define SL_RAIL_RF_STATE_DETAIL_LBT (1U << SL_RAIL_RF_STATE_DETAIL_LBT_SHIFT)
/**
 * Mask for core radio state bits.
 */
#define SL_RAIL_RF_STATE_DETAIL_CORE_STATE_MASK (SL_RAIL_RF_STATE_DETAIL_IDLE_STATE \
                                                 | SL_RAIL_RF_STATE_DETAIL_RX_STATE \
                                                 | SL_RAIL_RF_STATE_DETAIL_TX_STATE)

/**
 * @enum sl_rail_idle_mode_t
 * @brief An enumeration for the different types of supported idle modes. These
 *   vary how quickly and destructively they put the radio into idle.
 */
SLI_RAIL_ENUM(sl_rail_idle_mode_t) {
  /**
   * Idle the radio by turning off receive and canceling any future scheduled
   * receive or transmit operations. It does not abort a receive or
   * transmit in progress.
   */
  SL_RAIL_IDLE = 0u,
  /**
   * Idle the radio by turning off receive and any scheduled events. It
   * also aborts any receive, transmit, or scheduled events in progress.
   */
  SL_RAIL_IDLE_ABORT = 1u,
  /**
   * Force the radio into a shutdown mode by stopping whatever state is in
   * progress. This is a more destructive shutdown than \ref SL_RAIL_IDLE or
   * \ref SL_RAIL_IDLE_ABORT and can be useful in certain situations when directed
   * by the support team or for debugging. Note that this method may corrupt
   * receive and transmit buffers so it requires a more thorough cleanup
   * and any held packets will be lost.
   */
  SL_RAIL_IDLE_FORCE_SHUTDOWN = 2u,
  /**
   * Similar to the \ref SL_RAIL_IDLE_FORCE_SHUTDOWN command, however, it will also
   * clear any pending RAIL events related to receive and transmit.
   */
  SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS = 3u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_IDLE                            ((sl_rail_idle_mode_t) SL_RAIL_IDLE)
#define SL_RAIL_IDLE_ABORT                      ((sl_rail_idle_mode_t) SL_RAIL_IDLE_ABORT)
#define SL_RAIL_IDLE_FORCE_SHUTDOWN             ((sl_rail_idle_mode_t) SL_RAIL_IDLE_FORCE_SHUTDOWN)
#define SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS ((sl_rail_idle_mode_t) SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // end of group State_Transitions

/******************************************************************************
 * TX Channel Hopping
 *****************************************************************************/
/**
 * @addtogroup Tx_Channel_Hopping TX Channel Hopping
 * @{
 */

/**
 * @struct sl_rail_tx_channel_hopping_config_entry_t
 * @brief Structure that represents one of the channels that is part of a
 *   \ref sl_rail_tx_channel_hopping_config_t sequence of channels used in
 *   channel hopping.
 */
typedef struct sl_rail_tx_channel_hopping_config_entry {
  /**
   * The channel number to be used for this entry in the channel hopping
   * sequence. If this is an invalid channel for the current PHY, the
   * call to \ref sl_rail_set_next_tx_repeat() will fail.
   */
  uint16_t channel;
  /**
   * Pad bytes reserved for future use and currently ignored.
   */
  uint8_t reserved[2];
  /**
   * Idle time in microseconds to wait before transmitting on the channel
   * indicated by this entry.
   */
  sl_rail_transition_time_t delay_us;
} sl_rail_tx_channel_hopping_config_entry_t;

/**
 * @struct sl_rail_tx_channel_hopping_config_t
 * @brief Wrapper struct that will contain the sequence of
 *   \ref sl_rail_tx_channel_hopping_config_entry_t that represents the channel
 *   sequence to use during TX Channel Hopping.
 */
typedef struct sl_rail_tx_channel_hopping_config {
  /**
   * Pointer to contiguous global read-write memory that will be used
   * by RAIL to store channel hopping information throughout its operation.
   * It need not be initialized and applications should never write
   * data anywhere in this buffer.
   *
   * @note The size of this buffer must be at least as large as
   *   3 + \ref SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL * number_of_channels,
   *   plus the sum of the sizes of the
   *   radio_config_delta_add's of the required channels, plus the size of the
   *   radio_config_delta_subtract. In the case that one channel
   *   appears two or more times in your channel sequence
   *   (e.g., 1, 2, 3, 2), you must account for the radio configuration
   *   size that number of times (i.e., need to count channel 2's
   *   radio configuration size twice for the given example). The buffer is
   *   for internal use to the library.
   */
  uint32_t *p_buffer;
  /**
   * This parameter must be set to the length of the buffer array, in 32 bit
   * words. This way, during configuration, the software can confirm it's
   * writing within the bounds of the buffer. The configuration API will return
   * an error or trigger \ref SL_RAIL_ASSERT_CHANNEL_HOPPING_BUFFER_TOO_SHORT if
   * buffer_words is insufficient.
   */
  uint16_t buffer_words;
  /**
   * The number of channels in the channel hopping sequence, which is the
   * number of elements in the array that entries points to.
   */
  uint8_t number_of_channels;
  /**
   * Pad byte reserved for future use and currently ignored.
   */
  uint8_t reserved;
  /**
   * A pointer to the first element of an array of \ref
   * sl_rail_tx_channel_hopping_config_entry_t that represents the channels
   * used during channel hopping. The length of this array must be
   * number_of_channels.
   */
  sl_rail_tx_channel_hopping_config_entry_t *p_entries;
} sl_rail_tx_channel_hopping_config_t;

/**
 * The worst-case platform-agnostic static amount of memory needed per
 * channel for channel hopping, measured in 32 bit words, regardless of
 * the size of radio configuration structures.
 */
#define SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL_WORST_CASE (65U)

/** @} */ // end of group Tx_Channel_Hopping

/******************************************************************************
 * TX/RX Configuration Structures
 *****************************************************************************/
/**
 * @addtogroup Transmit
 * @{
 */

/**
 * @enum sl_rail_stop_mode_t
 * @brief Stop radio operation options bit mask
 */
SLI_RAIL_ENUM(sl_rail_stop_mode_t) {
  /**
   * Shift position of \ref SL_RAIL_STOP_MODE_ACTIVE bit.
   */
  SL_RAIL_STOP_MODE_ACTIVE_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_STOP_MODE_PENDING bit.
   */
  SL_RAIL_STOP_MODE_PENDING_SHIFT = 1,
};

/**
 * Do not stop any radio operations.
 */
#define SL_RAIL_STOP_MODES_NONE   (0U)
/**
 * Stop active radio operations only.
 */
#define SL_RAIL_STOP_MODE_ACTIVE (1U << SL_RAIL_STOP_MODE_ACTIVE_SHIFT)
/**
 * Stop pending radio operations.
 */
#define SL_RAIL_STOP_MODE_PENDING (1U << SL_RAIL_STOP_MODE_PENDING_SHIFT)
/**
 * Stop all radio operations.
 */
#define SL_RAIL_STOP_MODES_ALL (0xFFU)

/**
 * @enum sl_rail_tx_options_t
 * @brief Transmit options, in reality a bitmask.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_tx_options_t, uint32_t) {
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_WAIT_FOR_ACK bit.
   */
  SL_RAIL_TX_OPTION_WAIT_FOR_ACK_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_REMOVE_CRC bit.
   */
  SL_RAIL_TX_OPTION_REMOVE_CRC_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_SYNC_WORD_ID bit.
   */
  SL_RAIL_TX_OPTION_SYNC_WORD_ID_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_ANTENNA_0 bit.
   */
  SL_RAIL_TX_OPTION_ANTENNA_0_SHIFT = 3,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_ANTENNA_1 bit.
   */
  SL_RAIL_TX_OPTION_ANTENNA_1_SHIFT = 4,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN bit.
   */
  SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN_SHIFT = 5,
  /**
   * Reserved for future use.
   */
  SL_RAIL_TX_OPTION_RESERVED_6_SHIFT = 6,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_CCA_ONLY bit.
   */
  SL_RAIL_TX_OPTION_CCA_ONLY_SHIFT = 7,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_RESEND bit.
   */
  SL_RAIL_TX_OPTION_RESEND_SHIFT = 8,
  /**
   * Shift position of \ref SL_RAIL_TX_OPTION_CONCURRENT_PHY_ID bit.
   */
  SL_RAIL_TX_OPTION_CONCURRENT_PHY_ID_SHIFT = 9,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_TX_OPTIONS_COUNT
};

/**
 * A value representing no options enabled.
 */
#define SL_RAIL_TX_OPTIONS_NONE 0UL

/**
 * All options disabled by default. This is the fastest TX option to apply.
 */
#define SL_RAIL_TX_OPTIONS_DEFAULT SL_RAIL_TX_OPTIONS_NONE

/**
 * An option when Auto-Ack has been configured, enabled, and not TX paused, to
 * configure whether or not the transmitting node will listen for an Ack
 * response.
 * If this is false, the \ref sl_rail_rx_packet_details_t::is_ack flag of a received
 * packet will always be false.
 * If Auto-Ack is enabled, for instance using \ref sl_rail_config_auto_ack() or
 * \ref sl_rail_ieee802154_init(), and if this option is false, the radio
 * transitions to \ref sl_rail_auto_ack_config_t::tx_transitions's
 * \ref sl_rail_state_transitions_t::success state directly after transmitting a
 * packet and does not wait for an Ack.
 */
#define SL_RAIL_TX_OPTION_WAIT_FOR_ACK (1UL << SL_RAIL_TX_OPTION_WAIT_FOR_ACK_SHIFT)

/**
 * An option to remove CRC bytes from TX packets. To receive packets when the
 * sender has this option set true, set \ref SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS
 * on the receive side.
 */
#define SL_RAIL_TX_OPTION_REMOVE_CRC (1UL << SL_RAIL_TX_OPTION_REMOVE_CRC_SHIFT)

/**
 * An option to select which sync word is used during the transmission.
 * When two sync words are configured by the PHY or \ref sl_rail_config_sync_words()
 * enabling this option selects SYNC 1 rather than SYNC 0 for the upcoming transmit.
 *
 * This option should not be used when only one sync word has been configured.
 *
 * @note There are a few special radio configurations (e.g., BLE Viterbi) that do
 *   not support transmitting different sync words.
 */
#define SL_RAIL_TX_OPTION_SYNC_WORD_ID (1UL << SL_RAIL_TX_OPTION_SYNC_WORD_ID_SHIFT)

/**
 * An option to select antenna 0 for transmission. If the antenna selection
 * option is not set or if both antenna options are set, then the transmit
 * will occur on either antenna depending on the last receive or transmit
 * selection. This option is only valid on platforms that support
 * \ref Antenna_Control and have been configured via \ref sl_rail_config_antenna().
 *
 * @note These TX antenna options do not control the antenna used for
 *   \ref Auto_Ack transmissions, which always occur on the same antenna
 *   used to receive the packet being acknowledged.
 */
#define SL_RAIL_TX_OPTION_ANTENNA_0 (1UL << SL_RAIL_TX_OPTION_ANTENNA_0_SHIFT)

/**
 * An option to select antenna 1 for transmission. If the antenna selection
 * option is not set or if both antenna options are set, then the transmit
 * will occur on either antenna depending on the last receive or transmit
 * selection. This option is only valid on platforms that support
 * \ref Antenna_Control and have been configured via \ref sl_rail_config_antenna().
 *
 * @note These TX antenna options do not control the antenna used for
 *   \ref Auto_Ack transmissions, which always occur on the same antenna
 *   used to receive the packet being acknowledged.
 */
#define SL_RAIL_TX_OPTION_ANTENNA_1 (1UL << SL_RAIL_TX_OPTION_ANTENNA_1_SHIFT)

/**
 * An option to use the alternate preamble length established
 * by \ref sl_rail_set_tx_alt_preamble_length() for the transmission.
 * When not set, the PHY configuration's preamble length is used.
 */
#define SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN (1UL << SL_RAIL_TX_OPTION_ALT_PREAMBLE_LEN_SHIFT)

/**
 * An option to only perform the CCA (CSMA/LBT) operation but *not*
 * automatically transmit if the channel is clear. This option is only valid
 * when calling one of the CCA transmit routines: \ref sl_rail_start_cca_csma_tx(),
 * \ref sl_rail_start_cca_lbt_tx(), \ref sl_rail_start_scheduled_cca_csma_tx(), or \ref
 * sl_rail_start_scheduled_cca_lbt_tx().
 *
 * Application can then use the \ref SL_RAIL_EVENT_TX_CHANNEL_CLEAR to
 * initiate transmit manually, e.g., giving it the opportunity to adjust
 * outgoing packet data before the packet goes out.
 *
 * @note Configured state transitions to Rx or Idle are suspended during
 *   this CSMA/LBT operation. If packet reception occurs, the radio will
 *   return to the state it was in just prior to the CSMA/LBT operation
 *   when that reception (including any Auto-Ack response) is complete.
 */
#define SL_RAIL_TX_OPTION_CCA_ONLY (1UL << SL_RAIL_TX_OPTION_CCA_ONLY_SHIFT)

/**
 * An option to resend packet at the beginning of the Transmit FIFO.
 *
 * The packet to be resent must have been previously provided by
 * \ref sl_rail_set_tx_fifo() or \ref sl_rail_write_tx_fifo() passing true for
 * the latter's reset parameter. It works by setting the
 * transmit FIFO's read offset to the beginning of the FIFO while
 * leaving its write offset intact. For this to work,
 * \ref sl_rail_tx_data_config_t::tx_method must be sl_rail_data_method_t::SL_RAIL_DATA_METHOD_PACKET_MODE
 * (i.e., the packet can't exceed the Transmit FIFO's size), otherwise
 * undefined behavior will result.
 *
 * This option can also be used with \ref sl_rail_set_next_tx_repeat() to cause
 * the repeated packet(s) to all be the same as the first.
 */
#define SL_RAIL_TX_OPTION_RESEND (1UL << SL_RAIL_TX_OPTION_RESEND_SHIFT)

/**
 * An option to specify which PHY is used to transmit in the case of concurrent mode.
 * Concurrent mode is only allowed on EFR32xG25 for some predefined combinations of Wi-SUN PHYs.
 * When set/unset, the alternate/base PHY is used to transmit.
 */
#define SL_RAIL_TX_OPTION_CONCURRENT_PHY_ID (1UL << SL_RAIL_TX_OPTION_CONCURRENT_PHY_ID_SHIFT)

/**
 * A value representing all possible options.
 */
#define SL_RAIL_TX_OPTIONS_ALL 0xFFFFFFFFUL

/**
 * @struct sl_rail_tx_packet_details_t
 * @brief Detailed information requested about the packet that was just,
 *   or is currently being, transmitted.
 */
typedef struct sl_rail_tx_packet_details {
  /**
   * The time stamp of the transmitted packet in the RAIL timebase,
   * filled in by \ref sl_rail_get_tx_packet_details().
   */
  sl_rail_packet_time_stamp_t time_sent;
  /**
   * Indicate whether the transmitted packet was an Auto-Ack. In a generic
   * sense, an automatic Ack is defined as a packet sent in response to a
   * received Ack-requesting frame when Auto-Ack is enabled. In a protocol
   * specific sense this definition may be more or less restrictive to match the
   * specification and you should refer to that protocol's documentation.
   */
  bool is_ack;
} sl_rail_tx_packet_details_t;

/**
 * @enum sl_rail_scheduled_tx_during_rx_t
 * @brief Enumerates the possible outcomes of what will occur if a
 *   scheduled TX ends up firing during RX. Because RX and TX can't
 *   happen at the same time, it is up to the user how the TX should be
 *   handled. This enumeration is passed into \ref sl_rail_start_scheduled_tx()
 *   as part of \ref sl_rail_scheduled_tx_config_t.
 */
SLI_RAIL_ENUM(sl_rail_scheduled_tx_during_rx_t) {
  /**
   * The scheduled TX will be postponed until RX completes and then sent.
   */
  SL_RAIL_SCHEDULED_TX_DURING_RX_POSTPONE_TX = 0u,
  /**
   * The scheduled TX will be aborted and a
   * \ref SL_RAIL_EVENT_TX_BLOCKED event will fire.
   */
  SL_RAIL_SCHEDULED_TX_DURING_RX_ABORT_TX = 1u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_SCHEDULED_TX_DURING_RX_POSTPONE_TX ((sl_rail_scheduled_tx_during_rx_t) SL_RAIL_SCHEDULED_TX_DURING_RX_POSTPONE_TX)
#define SL_RAIL_SCHEDULED_TX_DURING_RX_ABORT_TX    ((sl_rail_scheduled_tx_during_rx_t) SL_RAIL_SCHEDULED_TX_DURING_RX_ABORT_TX)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_scheduled_tx_config_t
 * @brief A configuration structure for a scheduled transmit.
 */
typedef struct sl_rail_scheduled_tx_config {
  /**
   * The time when to transmit this packet. The exact interpretation of
   * this value depends on the mode specified below.
   */
  sl_rail_time_t when;
  /**
   * The type of delay. See the \ref sl_rail_time_mode_t documentation for
   * more information. Be sure to use \ref SL_RAIL_TIME_ABSOLUTE delays for
   * time-critical protocols.
   */
  sl_rail_time_mode_t mode;
  /**
   * Indicate which action to take with a scheduled TX if it occurs during RX.
   * See \ref sl_rail_scheduled_tx_during_rx_t structure for more information on
   * potential options.
   */
  sl_rail_scheduled_tx_during_rx_t tx_during_rx;
} sl_rail_scheduled_tx_config_t;

/**
 * @def SL_RAIL_MAX_LBT_TRIES
 * @brief The maximum number of LBT/CSMA retries supported.
 */
#define SL_RAIL_MAX_LBT_TRIES      (15U)

/**
 * @def SL_RAIL_MAX_CSMA_EXPONENT
 * @brief The maximum power-of-2 exponent for CSMA backoffs.
 */
#define SL_RAIL_MAX_CSMA_EXPONENT  (8U)

///
/// @struct sl_rail_csma_config_t
/// @brief A configuration structure for the CSMA transmit algorithm.
///
/// One of RAIL's schemes for polite spectrum access is an implementation of
/// a Carrier-Sense Multiple Access (CSMA) algorithm based on IEEE 802.15.4
/// (unslotted).
/// \n In pseudo-code it works like this, showing relevant event notifications:
/// @code{.c}
/// // Return true to transmit packet, false to not transmit packet.
/// bool perform_csma(const sl_rail_csma_config_t *csma_config)
/// {
///   extern int idle_to_rx_us;
///   bool is_fixed_backoff = ((csma_config->csma_min_bo_exp == 0)
///                            && (csma_config->csma_max_bo_exp == 0));
///   int backoff_exp = csma_config->csma_min_bo_exp; // Initial backoff exponent
///   int backoff_multiplier = 1; // Assume fixed backoff
///   int try;
///
///   // Special-case tries == 0 to transmit immediately without backoff+CCA
///   if (csma_config->csma_tries == 0) {
///     return true;
///   }
///
///   // Start overall timeout if specified:
///   if (csma_config->csma_timeout_us > 0) {
///     start_abort_timer(csma_config->csma_timeout_us, SL_RAIL_EVENT_TX_CHANNEL_BUSY);
///     // If timeout occurs, abort and signal the indicated event.
///   }
///
///   for (try = 0; try < csma_config->csma_tries; try++) {
///     if (try > 0) {
///       signal_event(SL_RAIL_EVENT_TX_CCA_RETRY);
///     }
///     // Determine the backoff multipler for this try:
///     if (is_fixed_backoff) {
///       // backoff_multiplier already set to 1 for fixed backoff
///     } else {
///       // Start with the backoff exponent for this try:
///       if (try > 0) {
///         backoff_exp++;
///         if (backoff_exp > csma_config->csma_max_bo_exp) {
///           backoff_exp = csma_config->csma_max_bo_exp;
///         }
///       }
///       // Pick random multiplier between 0 and 2^backoff_exp - 1 inclusive:
///       backoff_multiplier = pick_random_integer(0, (1 << backoff_exp) - 1);
///     }
///     // Perform the backoff:
///     if (backoff_multiplier > 0) {
///       delay_microseconds((backoff_multiplier * csma_config->cca_backoff_us)
///                          - idle_to_rx_us);
///     }
///     enable_radio_receive();
///     signal_event(SL_RAIL_EVENT_TX_START_CCA);
///     delay_microseconds(idle_to_rx_us);
///     // Perform the Clear-Channel Assessment (CCA):
///     // Channel is considered busy if radio is actively receiving or
///     // transmitting, or the energy detected during the duration period
///     // is above the threshold.
///     signal_event(SL_RAIL_EVENT_TX_CCA_ACTIVATED);
///     if (perform_cca(csma_config->cca_duration_us, csma_config->cca_threshold_dbm)) {
///       // CCA (and CSMA) success: Transmit after RX-to-TX turnaround
///       stop_abort_timer();
///       signal_event(SL_RAIL_EVENT_TX_CHANNEL_CLEAR);
///       return true;
///     } else {
///       // CCA failed: loop to try again, or exit if out of tries
///     }
///   }
///   // Overall CSMA failure: Don't transmit
///   stop_abort_timer();
///   signal_event(SL_RAIL_EVENT_TX_CHANNEL_BUSY);
///   return false;
/// }
/// @endcode
///
typedef struct sl_rail_csma_config {
  /**
   * The minimum (starting) exponent for CSMA random backoff (2^exp - 1).
   * It can range from 0 to \ref SL_RAIL_MAX_CSMA_EXPONENT.
   *
   * @warning On EFR32, due to a hardware limitation, this can only be 0
   *   if \ref csma_max_bo_exp is also 0 specifying a non-random fixed backoff.
   *   \ref SL_RAIL_STATUS_INVALID_PARAMETER will result otherwise.
   *   If you really want CSMA's first iteration to have no backoff prior to
   *   CCA, with subsequent iterations having random backoff as the exponent
   *   is increased, you must do a fixed backoff of 0 operation first
   *   (\ref csma_min_bo_exp = 0, \ref csma_max_bo_exp = 0, \ref cca_backoff_us = 0,
   *   \ref csma_tries = 1), and if that fails (\ref SL_RAIL_EVENT_TX_CHANNEL_BUSY),
   *   follow up with a random backoff operation starting at \ref csma_min_bo_exp
   *   = 1 for the remaining iterations.
   */
  uint8_t csma_min_bo_exp;
  /**
   * The maximum exponent for CSMA random backoff (2^exp - 1).
   * It can range from 0 to \ref SL_RAIL_MAX_CSMA_EXPONENT and must be greater
   * than or equal to \ref csma_min_bo_exp.
   * \n If both exponents are 0, a non-random fixed backoff of \ref cca_backoff_us
   * duration results.
   */
  uint8_t csma_max_bo_exp;
  /**
   * The number of backoff-then-CCA iterations that can fail before reporting
   * \ref SL_RAIL_EVENT_TX_CHANNEL_BUSY. Typically ranges from 1 to \ref
   * SL_RAIL_MAX_LBT_TRIES; higher values are disallowed. A value 0 always
   * transmits immediately without performing CSMA, similar to calling
   * \ref sl_rail_start_tx().
   */
  uint8_t csma_tries;
  /**
   * The CCA RSSI threshold, in dBm, above which the channel is
   * considered 'busy'.
   */
  int8_t cca_threshold_dbm;
  /**
   * The backoff unit period in RAIL's microsecond time base. It is
   * multiplied by the random backoff exponential controlled by \ref
   * csma_min_bo_exp and \ref csma_max_bo_exp to determine the overall backoff
   * period. For random backoffs, any value above 32768 microseconds for
   * the 'EFR Series 2' and 8192 microseconds for the 'Series 3' will be truncated
   * for a single backoff period. Up to 255 backoff periods are supported.
   * For fixed backoffs it can go up to 65535 microseconds.
   */
  uint16_t cca_backoff_us;
  /**
   * The minimum desired CCA check duration in microseconds. The RSSI is
   * sampled during this duration.
   *
   * @note Depending on the radio configuration, due to hardware constraints,
   *   the actual duration may be longer. Also, if the requested duration
   *   is too large for the radio to accommodate, \ref sl_rail_start_cca_csma_tx()
   *   will fail returning \ref SL_RAIL_STATUS_INVALID_PARAMETER.
   */
  uint16_t cca_duration_us;
  /**
   * An overall timeout, in RAIL's microsecond time base, for the operation.
   * If the transmission doesn't start before this timeout expires, the
   * transmission will fail with \ref SL_RAIL_EVENT_TX_CHANNEL_BUSY.
   * A value 0 means no timeout is imposed.
   */
  sl_rail_time_t csma_timeout_us;
} sl_rail_csma_config_t;

/**
 * @def SL_RAIL_CSMA_CONFIG_802_15_4_2003_2P4_GHZ_OQPSK_CSMA
 * @brief \ref sl_rail_csma_config_t initializer configuring CSMA per IEEE 802.15.4-2003
 *   on 2.4 GHz OSPSK, commonly used by Zigbee.
 */
#define SL_RAIL_CSMA_CONFIG_802_15_4_2003_2P4_GHZ_OQPSK_CSMA {             \
    /* CSMA per 802.15.4-2003 on 2.4 GHz OSPSK, commonly used by Zigbee */ \
    .csma_min_bo_exp   = 3,   /* 2^3-1 for 0..7 backoffs on 1st try     */ \
    .csma_max_bo_exp   = 5,   /* 2^5-1 for 0..31 backoffs on 3rd+ tries */ \
    .csma_tries        = 5,   /* 5 tries overall (4 re-tries)           */ \
    .cca_threshold_dbm = -75, /* 10 dB above sensitivity                */ \
    .cca_backoff_us    = 320, /* 20 symbols at 16 us/symbol             */ \
    .cca_duration_us   = 128, /* 8 symbols at 16 us/symbol              */ \
    .csma_timeout_us   = 0,   /* No timeout                             */ \
}

/**
 * @def SL_RAIL_CSMA_CONFIG_SINGLE_CCA
 * @brief \ref sl_rail_csma_config_t initializer configuring a single CCA prior to TX.
 *   It can be used to as a basis for implementing other channel access schemes
 *   with custom backoff delays. Users can override cca_backoff_us with a fixed
 *   delay on each use.
 */
#define SL_RAIL_CSMA_CONFIG_SINGLE_CCA {                                   \
    /* Perform a single CCA after 'fixed' delay                         */ \
    .csma_min_bo_exp   = 0,   /* Used for fixed backoff                 */ \
    .csma_max_bo_exp   = 0,   /* Used for fixed backoff                 */ \
    .csma_tries        = 1,   /* Single try                             */ \
    .cca_threshold_dbm = -75, /* Override if not desired choice         */ \
    .cca_backoff_us    = 0,   /* No backoff (override with fixed value) */ \
    .cca_duration_us   = 128, /* Override if not desired length         */ \
    .csma_timeout_us   = 0,   /* no timeout                             */ \
}

///
/// @struct sl_rail_lbt_config_t
/// @brief A configuration structure for the LBT transmit algorithm.
///
/// One of RAIL's schemes for polite spectrum access is an implementation of
/// a Listen-Before-Talk (LBT) algorithm, loosely based on ETSI 300 220-1.
/// \n Currently, however, it is constrained by the EFR32's CSMA-oriented hardware
/// so is turned into an equivalent \ref sl_rail_csma_config_t configuration and
/// passed to the CSMA engine:
/// @code{.c}
/// if (lbt_max_bo_rand == lbt_min_bo_rand) {
///   // Fixed backoff
///   csma_min_bo_exp = csma_max_bo_exp = 0;
///   if (lbt_min_bo_rand == 0) {
///     cca_backoff_us = lbt_backoff_us;
///   } else {
///     cca_backoff_us = lbt_min_bo_rand * lbt_backoff_us;
///   }
///   cca_duration_us = lbt_duration_us;
/// } else {
///   // Random backoff: map to random range 0 .. (lbt_max_bo_rand - lbt_min_bo_rand)
///   csma_min_bo_exp = csma_max_bo_exp = ceiling(log2(lbt_max_bo_rand - lbt_min_bo_rand));
///   cca_backoff_us = round((lbt_backoff_us * (lbt_max_bo_rand - lbt_min_bo_rand))
///                          / (1 << csma_mi_nb_oexp));
///   cca_duration_us = lbt_duration_us + (lbt_min_bo_rand * lbt_backoff_us);
/// }
/// csma_tries        = lbt_tries;
/// cca_threshold_dbm = lbt_threshold_dbm;
/// csma_timeout_us   = lbt_timeout_us;
/// @endcode
///
typedef struct sl_rail_lbt_config {
  /**
   * The minimum backoff random multiplier.
   */
  uint8_t lbt_min_bo_rand;
  /**
   * The maximum backoff random multiplier.
   * It must be greater than or equal to \ref lbt_min_bo_rand.
   * \n If both backoff multipliers are identical, a non-random fixed backoff
   * of \ref lbt_backoff_us times the multiplier (minimum 1) duration results.
   */
  uint8_t lbt_max_bo_rand;
  /**
   * The number of LBT iterations that can fail before reporting
   * \ref SL_RAIL_EVENT_TX_CHANNEL_BUSY. Typically ranges from 1 to \ref
   * SL_RAIL_MAX_LBT_TRIES; higher values are disallowed. A value 0 always
   * transmits immediately without performing LBT, similar to calling
   * \ref sl_rail_start_tx().
   */
  uint8_t lbt_tries;
  /**
   * The LBT RSSI threshold, in dBm, above which the channel is
   * considered 'busy'.
   */
  int8_t lbt_threshold_dbm;
  /**
   * The backoff unit period, in RAIL's microsecond time base. It is
   * multiplied by the random backoff multiplier controlled by \ref
   * lbt_min_bo_rand and \ref lbt_max_bo_rand to determine the overall backoff
   * period. For random backoffs, any value above 32768 microseconds for
   * the 'EFR Series 2' and 8192 microseconds for the 'Series 3' will be truncated
   * for a single backoff period. Up to 255 backoff periods are supported.
   * For fixed backoffs it can go up to 65535 microseconds.
   */
  uint16_t lbt_backoff_us;
  /**
   * The minimum desired LBT check duration in microseconds.
   *
   * @note Depending on the radio configuration, due to hardware constraints,
   *   the actual duration may be longer. Also, if the requested duration
   *   is too large for the radio to accommodate, \ref sl_rail_start_cca_lbt_tx()
   *   will fail returning \ref SL_RAIL_STATUS_INVALID_PARAMETER.
   */
  uint16_t lbt_duration_us;
  /**
   * An overall timeout, in RAIL's microsecond time base, for the operation.
   * If the transmission doesn't start before this timeout expires, the
   * transmission will fail with \ref SL_RAIL_EVENT_TX_CHANNEL_BUSY.
   * This is important for limiting LBT due to LBT's unbounded requirement
   * that if the channel is busy, the next try must wait for the channel to
   * clear. A value 0 means no timeout is imposed.
   */
  sl_rail_time_t lbt_timeout_us;
} sl_rail_lbt_config_t;

/**
 * @def SL_RAIL_LBT_CONFIG_ETSI_EN_300_220_1_V2_4_1
 * @brief \ref sl_rail_lbt_config_t initializer configuring LBT per ETSI 300 220-1
 *   V2.4.1 for a typical Sub-GHz band. To be practical, users should override
 *   lbt_tries and/or lbt_timeout_us so channel access failure will be reported in a
 *   reasonable time frame rather than the unbounded time frame ETSI defined.
 */
#define SL_RAIL_LBT_CONFIG_ETSI_EN_300_220_1_V2_4_1 {                             \
    /* LBT per ETSI 300 220-1 V2.4.1                                           */ \
    /* LBT time = random backoff of 0-5 ms in .5 ms increments plus 5 ms fixed */ \
    .lbt_min_bo_rand   = 0,    /*                                              */ \
    .lbt_max_bo_rand   = 10,   /*                                              */ \
    .lbt_tries         = SL_RAIL_MAX_LBT_TRIES, /* the maximum supported       */ \
    .lbt_threshold_dbm = -87,  /*                                              */ \
    .lbt_backoff_us    = 500,  /* 0.5 ms                                       */ \
    .lbt_duration_us   = 5000, /* 5 ms                                         */ \
    .lbt_timeout_us    = 0,    /* No timeout (recommend user override)         */ \
}

/**
 * @def SL_RAIL_LBT_CONFIG_ETSI_EN_300_220_1_V3_1_0
 * @brief \ref sl_rail_lbt_config_t initializer configuring LBT per ETSI 300 220-1
 *   V3.1.0 for a typical Sub-GHz band. To be practical, users should override
 *   lbt_tries and/or lbt_timeout_us so channel access failure will be reported in a
 *   reasonable time frame rather than the unbounded time frame ETSI defined.
 */
#define SL_RAIL_LBT_CONFIG_ETSI_EN_300_220_1_V3_1_0 {                            \
    /* LBT per ETSI 300 220-1 V3.1.0                                          */ \
    /* LBT time = random backoff of 160-4960 us in 160 us increments          */ \
    .lbt_min_bo_rand   = 1,    /*                                             */ \
    .lbt_max_bo_rand   = 31,   /* app-chosen; 31*lbt_backoff_us = 4960 us     */ \
    .lbt_tries         = SL_RAIL_MAX_LBT_TRIES, /* the maximum supported      */ \
    .lbt_threshold_dbm = -85,  /* 15 dB above Rx sensitivity per Table 45     */ \
    .lbt_backoff_us    = 160,  /* 160 us per Table 48 Minimum CCA interval    */ \
    .lbt_duration_us   = 160,  /* 160 us per Table 48 Minimum deferral period */ \
    .lbt_timeout_us    = 0,    /* No timeout (recommend user override)        */ \
}

/**
 * @struct sl_rail_sync_word_config_t
 * @brief RAIL sync words and length configuration.
 *
 */
typedef struct sl_rail_sync_word_config {
  /**
   * Sync word length in bits, between 2 and 32, inclusive.
   */
  uint8_t sync_word_bits;
  /**
   * Sync Word 0
   * @note Only the least-significant \ref sync_word_bits bits are used,
   *   which are sent or received on air least-significant-bit first.
   */
  uint32_t sync_word_0;
  /**
   * Sync Word 1
   * @note Only the least-significant \ref sync_word_bits bits are used,
   *   which are sent or received on air least-significant-bit first.
   */
  uint32_t sync_word_1;
} sl_rail_sync_word_config_t;

/**
 * @enum sl_rail_tx_repeat_options_t
 * @brief Transmit repeat options, in reality a bitmask.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_tx_repeat_options_t, uint16_t) {
  /**
   * Shift position of \ref SL_RAIL_TX_REPEAT_OPTION_HOP bit.
   */
  SL_RAIL_TX_REPEAT_OPTION_HOP_SHIFT = 0,
  /**
   * Shift position of the \ref SL_RAIL_TX_REPEAT_OPTION_START_TO_START bit.
   */
  SL_RAIL_TX_REPEAT_OPTION_START_TO_START_SHIFT = 1,
};

/**
 * A value representing no repeat options enabled.
 */
#define SL_RAIL_TX_REPEAT_OPTIONS_NONE 0U
/**
 * All repeat options disabled by default.
 */
#define SL_RAIL_TX_REPEAT_OPTIONS_DEFAULT SL_RAIL_TX_REPEAT_OPTIONS_NONE
/**
 * An option to configure whether or not to channel-hop before each
 * repeated transmit.
 */
#define SL_RAIL_TX_REPEAT_OPTION_HOP (1U << SL_RAIL_TX_REPEAT_OPTION_HOP_SHIFT)

/**
 * An option to configure the delay between transmissions to be from start to start
 * instead of end to start. Delay must be long enough to cover the prior transmit's time.
 */
#define SL_RAIL_TX_REPEAT_OPTION_START_TO_START (1 << SL_RAIL_TX_REPEAT_OPTION_START_TO_START_SHIFT)

/**
 * @struct sl_rail_tx_repeat_config_t
 * @brief A configuration structure for repeated transmits
 *
 * @note The PA will always be ramped down and up in between transmits so
 *   there will always be some minimum delay between transmits depending on the
 *   ramp time configuration.
 */
typedef struct sl_rail_tx_repeat_config {
  /**
   * The number of repeated transmits to run. A total of (iterations + 1)
   * transmits will go on-air in the absence of errors.
   */
  uint16_t iterations;
  /**
   * Repeat option(s) to apply.
   */
  sl_rail_tx_repeat_options_t repeat_options;
  /**
   * Per-repeat delay or hopping configuration, depending on repeat_options.
   */
  union {
    /**
     * When \ref SL_RAIL_TX_REPEAT_OPTION_HOP is not set, specifies
     * the delay time between each repeated transmit. Specify \ref
     * SL_RAIL_TRANSITION_TIME_KEEP to use the current \ref
     * sl_rail_state_timing_t::tx_to_tx transition time setting.
     * When using \ref SL_RAIL_TX_REPEAT_OPTION_START_TO_START the delay
     * must be long enough to cover the prior transmit's time.
     */
    sl_rail_transition_time_t delay_us;
    /**
     * When \ref SL_RAIL_TX_REPEAT_OPTION_HOP is set, this specifies
     * the channel hopping configuration to use when hopping between
     * repeated transmits. Per-hop delays are configured within each
     * \ref sl_rail_tx_channel_hopping_config_entry_t::delay_us rather than
     * this union's delay field.
     * When using \ref SL_RAIL_TX_REPEAT_OPTION_START_TO_START the hop delay
     * must be long enough to cover the prior transmit's time.
     */
    sl_rail_tx_channel_hopping_config_t channel_hopping;
  } delay_or_hop;
} sl_rail_tx_repeat_config_t;

/**
 * \ref sl_rail_tx_repeat_config_t::iterations initializer configuring infinite
 * repeated transmissions.
 */
#define SL_RAIL_TX_REPEAT_INFINITE_ITERATIONS (0xFFFFU)

/** @} */ // end of group Transmit

/******************************************************************************
 * Receive Structures
 *****************************************************************************/
/**
 * @addtogroup Receive
 * @{
 */

/**
 * @addtogroup Address_Filtering
 * @{
 */

/**
 * A default address filtering match table for configurations that use only one
 * address field. The truth table for address matching is shown below.
 *
 * |                | No Match | Address 0 | Address 1 | Address 2 | Address 3 |
 * |----------------|----------|-----------|-----------|-----------|-----------|
 * | __No Match__   |    0     |     1     |     1     |     1     |     1     |
 * | __Address 0__  |    1     |     1     |     1     |     1     |     1     |
 * | __Address 1__  |    1     |     1     |     1     |     1     |     1     |
 * | __Address 2__  |    1     |     1     |     1     |     1     |     1     |
 * | __Address 3__  |    1     |     1     |     1     |     1     |     1     |
 */
#define SL_RAIL_ADDR_CONFIG_MATCH_TABLE_SINGLE_FIELD (0x1FFFFFE)

/**
 * A default address filtering match table for configurations that use two
 * address fields and want to match the same index in each. The truth
 * table for address matching is shown below.
 *
 * |                | No Match | Address 0 | Address 1 | Address 2 | Address 3 |
 * |----------------|----------|-----------|-----------|-----------|-----------|
 * | __No Match__   |    0     |    0      |    0      |    0      |    0      |
 * | __Address 0__  |    0     |    1      |    0      |    0      |    0      |
 * | __Address 1__  |    0     |    0      |    1      |    0      |    0      |
 * | __Address 2__  |    0     |    0      |    0      |    1      |    0      |
 * | __Address 3__  |    0     |    0      |    0      |    0      |    1      |
 */
#define SL_RAIL_ADDR_CONFIG_MATCH_TABLE_DOUBLE_FIELD (0x1041040)

/**
 * The maximum number of address fields that can be used by the address
 * filtering logic.
 */
#define SL_RAIL_ADDR_CONFIG_MAX_ADDRESS_FIELDS (2)

/**
 * @struct sl_rail_addr_config_t
 * @brief A structure to configure the address filtering functionality in RAIL.
 */
typedef struct sl_rail_addr_config {
  /**
   * A list of the start byte offsets for each field.
   *
   * These offsets are specified relative to the previous field's end.
   * For the first field, it is relative to either the beginning of the packet
   * or the end of the frame type byte if frame type decoding is enabled. If a
   * field is unused, it's offset should be set to 0.
   */
  uint8_t offsets[SL_RAIL_ADDR_CONFIG_MAX_ADDRESS_FIELDS];

  /**
   * A list of the address field byte sizes.
   *
   * These sizes are specified in bytes from 0 to 8. If you choose a
   * size of 0, this field is effectively disabled.
   */
  uint8_t sizes[SL_RAIL_ADDR_CONFIG_MAX_ADDRESS_FIELDS];

  /**
   * The truth table to determine how the two fields combine to create a match.
   *
   * For detailed information about how this truth table is formed, see the
   * detailed description of \ref Address_Filtering.
   *
   * For simple predefined configurations use the following defines.
   *  - \ref SL_RAIL_ADDR_CONFIG_MATCH_TABLE_SINGLE_FIELD
   *    - For filtering that only uses a single address field.
   *  - \ref SL_RAIL_ADDR_CONFIG_MATCH_TABLE_DOUBLE_FIELD for two field filtering where you
   *    - For filtering that uses two address fields in a configurations where
   *      you want the following logic `((Field_0, Index_0) && (Field_1, Index_0))
   *      || ((Field_0, Index_1) && (Field_1, Index_1)) || ...`
   */
  uint32_t match_table;
} sl_rail_addr_config_t;

/**
 * @brief A bitmask representation of which 4 filters passed for each
 *   \ref SL_RAIL_ADDR_CONFIG_MAX_ADDRESS_FIELDS when filtering has completed
 *   successfully.
 *
 * It's layout is:
 * |  Bit 7 |  Bit 6 |  Bit 5 |  Bit 4 |  Bit 3 |  Bit 2 |  Bit 1 |  Bit 0 |
 * |--------+--------+--------+--------+--------+--------+--------+--------|
 * |   Second Address Field Nibble     |     First Address Field Nibble    |
 * | Addr 3 | Addr 2 | Addr 1 | Addr 0 | Addr 3 | Addr 2 | Addr 1 | Addr 0 |
 * |  match |  match |  match |  match |  match |  match |  match |  match |
 * |--------+--------+--------+--------+--------+--------+--------+--------|
 *
 * @note This information is valid in \ref sl_rail_ieee802154_address_t on all
 *   platforms, but is only valid in \ref sl_rail_rx_packet_info_t on platforms
 *   where \ref SL_RAIL_SUPPORTS_ADDR_FILTER_MASK is true.
 */
typedef uint8_t sl_rail_addr_filter_mask_t;

/** @} */ // end of group Address_Filtering

/**
 * @enum sl_rail_rx_options_t
 * @brief Receive options, in reality a bitmask.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_rx_options_t, uint32_t) {
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_STORE_CRC bit.
   */
  SL_RAIL_RX_OPTION_STORE_CRC_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS bit.
   */
  SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_ENABLE_DUAL_SYNC bit.
   */
  SL_RAIL_RX_OPTION_ENABLE_DUAL_SYNC_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES bit.
   */
  SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES_SHIFT = 3,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO bit.
   */
  SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO_SHIFT = 4,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_ANTENNA_0 bit.
   */
  SL_RAIL_RX_OPTION_ANTENNA_0_SHIFT = 5,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_ANTENNA_1 bit.
   */
  SL_RAIL_RX_OPTION_ANTENNA_1_SHIFT = 6,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION bit.
   */
  SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION_SHIFT = 7,
#ifndef DOXYGEN_UNDOCUMENTED
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_SKIP_DC_CAL bit.
   */
  SL_RAIL_RX_OPTION_SKIP_DC_CAL_SHIFT = 8,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_SKIP_SYNTH_CAL bit.
   */
  SL_RAIL_RX_OPTION_SKIP_SYNTH_CAL_SHIFT = 9,
#endif //DOXYGEN_UNDOCUMENTED
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_CHANNEL_SWITCHING bit.
   */
  SL_RAIL_RX_OPTION_CHANNEL_SWITCHING_SHIFT = 10,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_FAST_RX_TO_RX bit.
   */
  SL_RAIL_RX_OPTION_FAST_RX_TO_RX_SHIFT = 11,
  /**
   * Shift position of \ref SL_RAIL_RX_OPTION_ENABLE_COLLISION_DETECTION bit.
   */
  SL_RAIL_RX_OPTION_ENABLE_COLLISION_DETECTION_SHIFT = 12,
};

/**
 * A value representing no options enabled.
 */
#define SL_RAIL_RX_OPTIONS_NONE 0
/**
 * All options are disabled by default.
 */
#define SL_RAIL_RX_OPTIONS_DEFAULT SL_RAIL_RX_OPTIONS_NONE

/**
 * An option to configure whether the CRC portion of the packet is included in
 * the packet payload exposed to the app on packet reception.
 * Defaults to false.
 */
#define SL_RAIL_RX_OPTION_STORE_CRC (1UL << SL_RAIL_RX_OPTION_STORE_CRC_SHIFT)

/**
 * An option to configure whether CRC errors will be ignored.
 * If this is set, RX will still be successful, even if
 * the CRC does not pass the check. Defaults to false.
 *
 * @note An expected Ack that fails CRC with this option set
 *   will still be considered the expected Ack, terminating
 *   the \ref sl_rail_auto_ack_config_t::ack_timeout_us period.
 */
#define SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS (1UL << SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS_SHIFT)

/**
 * An option to control which sync words will be accepted. Setting it to
 * 0 (default) will cause the receiver to listen for SYNC 0 only. Setting it to
 * 1 causes the receiver to listen for either SYNC 0 or SYNC 1. RX appended info
 * will contain which sync word was detected. Note, this only affects which
 * sync word(s) are received, but not what each of the sync words actually are.
 * This feature may not be available on some combinations of chips, PHYs, and
 * protocols. Use the compile time symbol \ref SL_RAIL_SUPPORTS_DUAL_SYNC_WORDS or
 * the runtime call \ref sl_rail_supports_dual_sync_words() to check whether the
 * platform supports this feature. Also, dual sync may be incompatible
 * with certain radio configurations. In these cases, setting this bit will
 * be ignored. See the data sheet or support team for more details.
 */
#define SL_RAIL_RX_OPTION_ENABLE_DUAL_SYNC (1UL << SL_RAIL_RX_OPTION_ENABLE_DUAL_SYNC_SHIFT)

/**
 * An option to configure whether frames which are aborted during reception
 * should continue to be tracked. Setting this option allows viewing Packet
 * Trace information for frames which get discarded. Defaults to false.
 *
 * This option is ignored when doing a \ref SL_RAIL_IDLE_FORCE_SHUTDOWN or
 * \ref SL_RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS.
 *
 * @note This option should not be used with coded PHYs since packet data
 *   received after the abort will not be decoded properly.
 */
#define SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES (1UL << SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES_SHIFT)

/**
 * An option to suppress capturing the appended information after
 * received frames. Defaults to false. When suppressed, certain
 * \ref sl_rail_rx_packet_details_t details will not be available for received
 * packets whose \ref sl_rail_rx_packet_status_t is among the SL_RAIL_RX_PACKET_READY_
 * set.
 *
 * @warning This option should be changed only when the radio is idle
 *   and the receive FIFO and Packet Queue are empty or have been reset,
 *   otherwise \ref sl_rail_get_rx_packet_info() and \ref sl_rail_get_rx_packet_details()
 *   may think appended info is packet data or vice-versa.
 */
#define SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO (1UL << SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO_SHIFT)

/**
 * An option to select the use of antenna 0 during receive (including
 * \ref Auto_Ack receive). If no antenna option is selected, the packet
 * will be received on the last antenna used for receive or transmit.
 * Defaults to false. This option is only valid on platforms that support
 * \ref Antenna_Control and have been configured via \ref sl_rail_config_antenna().
 */
#define SL_RAIL_RX_OPTION_ANTENNA_0 (1UL << SL_RAIL_RX_OPTION_ANTENNA_0_SHIFT)

/**
 * An option to select the use of antenna 1 during receive (including
 * \ref Auto_Ack receive). If no antenna option is selected, the packet
 * will be received on the last antenna used for receive or transmit.
 * Defaults to false. This option is only valid on platforms that support
 * \ref Antenna_Control and have been configured via \ref sl_rail_config_antenna().
 */
#define SL_RAIL_RX_OPTION_ANTENNA_1 (1UL << SL_RAIL_RX_OPTION_ANTENNA_1_SHIFT)

/**
 * An option combination to automatically choose an antenna during receive
 * (including \ref Auto_Ack receive). If both antenna 0 and antenna 1
 * options are set, the radio will dynamically switch between antennas
 * during packet detection and choose the best one for completing the
 * reception. This option is only valid when the antenna diversity
 * field is properly configured via Simplicity Studio.
 * This option is only valid on platforms that support
 * \ref Antenna_Control and have been configured via \ref sl_rail_config_antenna().
 */
#define SL_RAIL_RX_OPTION_ANTENNA_AUTO (SL_RAIL_RX_OPTION_ANTENNA_0 | SL_RAIL_RX_OPTION_ANTENNA_1)

/**
 * An option to disable frame detection. This can be useful for doing energy
 * detection without risking packet reception. Enabling this will abort any
 * frame currently being received in addition to preventing further frames
 * from being received. Defaults to false.
 */
#define SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION (1UL << SL_RAIL_RX_OPTION_DISABLE_FRAME_DETECTION_SHIFT)

#ifndef DOXYGEN_UNDOCUMENTED
/**
 * An option to skip DC calibration when transitioning from RX to RX. This can be
 * useful for reducing the state transition time, but risks impacting
 * receive capability. Enabling this bypasses DC calibration (like
 * \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_DC_CAL)
 * Defaults to false.
 */
#define SL_RAIL_RX_OPTION_SKIP_DC_CAL (1UL << SL_RAIL_RX_OPTION_SKIP_DC_CAL_SHIFT)

/**
 * An option to skip synth calibration when transitioning from RX to RX. This can
 * be useful for reducing the state transition time, but risks impacting receive
 * capability. Enabling this bypasses synth calibration (like
 * \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_SYNTH_CAL)
 * Defaults to false.
 */
#define SL_RAIL_RX_OPTION_SKIP_SYNTH_CAL (1U << SL_RAIL_RX_OPTION_SKIP_SYNTH_CAL_SHIFT)
#endif //DOXYGEN_UNDOCUMENTED

/**
 * An option to enable IEEE 802.15.4 RX channel switching.
 * See \ref sl_rail_ieee802154_config_rx_channel_switching() for more information.
 * Defaults to false.
 *
 * @note This option is only supported on specific chips where
 *   \ref SL_RAIL_IEEE802154_SUPPORTS_RX_CHANNEL_SWITCHING is true.
 *
 * @note This option overrides \ref SL_RAIL_RX_OPTION_ANTENNA_0,
 *   \ref SL_RAIL_RX_OPTION_ANTENNA_1 and \ref SL_RAIL_RX_OPTION_ANTENNA_AUTO antenna
 *   selection options.
 */
#define SL_RAIL_RX_OPTION_CHANNEL_SWITCHING (1U << SL_RAIL_RX_OPTION_CHANNEL_SWITCHING_SHIFT)

/**
 * An option to enable fast RX-to-RX state transition.
 *
 * Once enabled, the sequencer will send the radio to RXSEARCH and get ready to
 * receive the next packet while still processing the previous one. This will
 * reduce RX to RX state transition time but risks impacting receive capability.
 *
 * @note This option is only supported on specific chips where
 *   \ref SL_RAIL_SUPPORTS_FAST_RX_TO_RX is true.
 */
#define SL_RAIL_RX_OPTION_FAST_RX_TO_RX (1U << SL_RAIL_RX_OPTION_FAST_RX_TO_RX_SHIFT)

/**
 * An option to enable collision detection.
 *
 * Once enabled, when a collision with a strong enough packet is detected, the demod
 * will stop the current packet decoding and try to detect the preamble of the incoming
 * packet.
 *
 * @note This option is only supported on specific chips where
 *   \ref SL_RAIL_SUPPORTS_COLLISION_DETECTION is true.
 */
#define SL_RAIL_RX_OPTION_ENABLE_COLLISION_DETECTION (1U << SL_RAIL_RX_OPTION_ENABLE_COLLISION_DETECTION_SHIFT)

/**
 * A value representing all possible options.
 */
#define SL_RAIL_RX_OPTIONS_ALL 0xFFFFFFFFUL

/**
 * The value returned by RAIL for an invalid RSSI, in dBm.
 */
#define SL_RAIL_RSSI_INVALID_DBM     (-128)
/**
 * The value returned by RAIL for an invalid RSSI: in quarter dBm.
 */
#define SL_RAIL_RSSI_INVALID         ((int16_t)(SL_RAIL_RSSI_INVALID_DBM * 4))
/**
 * The lowest RSSI value returned by RAIL: in quarter dBm.
 */
#define SL_RAIL_RSSI_LOWEST          ((int16_t)(SL_RAIL_RSSI_INVALID + 1))

/**
 * Maximum absolute value for RSSI offset.
 */
#define SL_RAIL_RSSI_OFFSET_MAX      35

/**
 * A sentinel value to indicate waiting for a valid RSSI without a timeout.
 */
#define SL_RAIL_GET_RSSI_WAIT_WITHOUT_TIMEOUT ((sl_rail_time_t)0xFFFFFFFFU)
/**
 * A sentinel value to indicate no waiting for a valid RSSI.
 */
#define SL_RAIL_GET_RSSI_NO_WAIT ((sl_rail_time_t)0U)

/**
 * @struct sl_rail_scheduled_rx_config_t
 * @brief Configures the scheduled RX algorithm.
 *
 * Defines the start and end times of the receive window created
 * for a scheduled receive. If either start or end times are disabled, they
 * will be ignored.
 */
typedef struct sl_rail_scheduled_rx_config {
  /**
   * The time to start receive. See start_mode for more information about the
   * types of start times that you can specify.
   */
  sl_rail_time_t start;
  /**
   * How to interpret the time value specified in the start parameter. See the
   * \ref sl_rail_time_mode_t documentation for more information. Use
   * \ref SL_RAIL_TIME_ABSOLUTE for absolute times, \ref SL_RAIL_TIME_DELAY for times
   * relative to the current time and \ref SL_RAIL_TIME_DISABLED to ignore the
   * start time.
   */
  sl_rail_time_mode_t start_mode;
  /**
   * The time to end receive. See end_mode for more information about the types
   * of end times you can specify.
   */
  sl_rail_time_t end;
  /**
   * How to interpret the time value specified in the end parameter. See the
   * \ref sl_rail_time_mode_t documentation for more information. Note that, in
   * this API, if you specify a \ref SL_RAIL_TIME_DELAY, it is relative to the
   * start time if given and relative to now if none is specified. Also, using
   * \ref SL_RAIL_TIME_DISABLED means that this window will not end unless you
   * explicitly call \ref sl_rail_idle() or add an end event through a future
   * update to this configuration.
   */
  sl_rail_time_mode_t end_mode;
  /**
   * While in scheduled RX, you can still control the radio state via
   * state transitions. This option configures whether a transition
   * to RX goes back to scheduled RX or to the normal RX state. Once in the
   * normal RX state, you will effectively end the scheduled RX window and can
   * continue to receive indefinitely depending on the state transitions. Set
   * to 1 to transition to normal RX and 0 to stay in the scheduled RX.
   *
   * @note This field's behavior differs from RAIL 2.x behavior: In RAIL 2.x
   *   when this field is true, it would suppress \ref RAIL_EVENT_RX_SCHEDULED_RX_END
   *   when the scheduled Rx window is implicitly ended by a packet receive
   *   (any of the \ref SL_RAIL_EVENTS_RX_COMPLETION events). In RAIL 3 the
   *   \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_END is never suppressed.
   *
   * @note An Rx transition to Idle state will always terminate the
   *   scheduled Rx window, regardless of this setting. This can be used
   *   to ensure Scheduled RX terminates on the first packet received
   *   (or first successful packet if the RX error transition is to Rx
   *   while the Rx success transition is to Idle).
   */
  uint8_t rx_transition_end_schedule;
  /**
   * This setting tells RAIL what to do with a packet being received
   * when the window end event occurs. If set to 0, such a packet
   * will be allowed to complete. Any other setting will cause that
   * packet to be aborted. In either situation, any posting of
   * \ref SL_RAIL_EVENT_RX_SCHEDULED_RX_END is deferred briefly to when
   * the packet's corresponding \ref SL_RAIL_EVENTS_RX_COMPLETION occurs.
   */
  uint8_t hard_window_end;
} sl_rail_scheduled_rx_config_t;

/**
 * @enum sl_rail_rx_packet_status_t
 * @brief The packet status code associated with a packet received or
 *   currently being received.
 *
 * @note RECEIVING implies some packet data may be available, but
 *   is untrustworthy (not CRC-verified) and might disappear if the packet
 *   is rolled back on error. No packet details are yet available.
 * @note In RX \ref sl_rail_data_method_t::SL_RAIL_DATA_METHOD_FIFO_MODE, ABORT statuses imply some
 *   packet data may be available, but it's incomplete and not trustworthy.
 */
SLI_RAIL_ENUM(sl_rail_rx_packet_status_t) {
  /**
   * The radio is idle or searching for a packet.
   */
  SL_RAIL_RX_PACKET_NONE = 0u,
  /**
   * The packet was aborted during filtering because of illegal frame length,
   * CRC or block decoding errors, other RAIL built-in protocol-specific
   * packet content errors, or by the application or multiprotocol scheduler
   * idling the radio with \ref SL_RAIL_IDLE_ABORT or higher.
   *
   * Corresponding \ref SL_RAIL_EVENT_RX_PACKET_ABORTED is triggered.
   */
  SL_RAIL_RX_PACKET_ABORT_FORMAT = 1u,
  /**
   * The packet failed address filtering.
   *
   * Corresponding \ref SL_RAIL_EVENT_RX_ADDRESS_FILTERED is triggered.
   */
  SL_RAIL_RX_PACKET_ABORT_FILTERED = 2u,
  /**
   * The packet passed any filtering but was aborted by the application
   * or multiprotocol scheduler idling the radio with \ref SL_RAIL_IDLE_ABORT
   * or higher.
   *
   * Corresponding \ref SL_RAIL_EVENT_RX_PACKET_ABORTED is triggered.
   */
  SL_RAIL_RX_PACKET_ABORT_ABORTED = 3u,
  /**
   * The packet overflowed the receive buffer.
   *
   * Corresponding \ref SL_RAIL_EVENT_RX_FIFO_OVERFLOW is triggered.
   */
  SL_RAIL_RX_PACKET_ABORT_OVERFLOW = 4u,
  /**
   * The packet passed any filtering but subsequently failed CRC check(s)
   * block decoding, or illegal frame length, and was aborted.
   *
   * Corresponding \ref SL_RAIL_EVENT_RX_FRAME_ERROR is triggered.
   */
  SL_RAIL_RX_PACKET_ABORT_CRC_ERROR = 5u,
  /**
   * The packet passed any filtering but subsequently failed CRC check(s)
   * with \ref SL_RAIL_RX_OPTION_IGNORE_CRC_ERRORS in effect. Can also occur
   * when the packet prematurely ended successfully during filtering,
   * and either the \ref SL_RAIL_EVENT_RX_PACKET_ABORTED or
   * \ref SL_RAIL_EVENT_RX_ADDRESS_FILTERED events had been enabled
   * requesting notification of such packets.
   *
   * Corresponding \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED is triggered.
   */
  SL_RAIL_RX_PACKET_READY_CRC_ERROR = 6u,
  /**
   * The packet was successfully received, passing CRC check(s).
   *
   * Corresponding \ref SL_RAIL_EVENT_RX_PACKET_RECEIVED is triggered.
   */
  SL_RAIL_RX_PACKET_READY_SUCCESS = 7u,
  /**
   * A packet is being received and is not yet complete.
   */
  SL_RAIL_RX_PACKET_RECEIVING = 8u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_RX_PACKET_NONE            ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_NONE)
#define SL_RAIL_RX_PACKET_ABORT_FORMAT    ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_ABORT_FORMAT)
#define SL_RAIL_RX_PACKET_ABORT_FILTERED  ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_ABORT_FILTERED)
#define SL_RAIL_RX_PACKET_ABORT_ABORTED   ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_ABORT_ABORTED)
#define SL_RAIL_RX_PACKET_ABORT_OVERFLOW  ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_ABORT_OVERFLOW)
#define SL_RAIL_RX_PACKET_ABORT_CRC_ERROR ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_ABORT_CRC_ERROR)
#define SL_RAIL_RX_PACKET_READY_CRC_ERROR ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_READY_CRC_ERROR)
#define SL_RAIL_RX_PACKET_READY_SUCCESS   ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_READY_SUCCESS)
#define SL_RAIL_RX_PACKET_RECEIVING       ((sl_rail_rx_packet_status_t) SL_RAIL_RX_PACKET_RECEIVING)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @typedef sl_rail_rx_packet_handle_t
 * @brief A handle used to reference a packet during reception processing.
 *   There are several sentinel handle values that pertain to certain
 *   circumstances: \ref SL_RAIL_RX_PACKET_HANDLE_INVALID, \ref
 *   SL_RAIL_RX_PACKET_HANDLE_OLDEST, \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE
 *   and \ref SL_RAIL_RX_PACKET_HANDLE_NEWEST.
 */
typedef const void *sl_rail_rx_packet_handle_t;

/**
 * An invalid RX packet handle value.
 */
#define SL_RAIL_RX_PACKET_HANDLE_INVALID  (NULL)

/**
 * A special RX packet handle to refer to the oldest unreleased packet.
 * This includes the newest unread packet which is possibly incomplete or not
 * yet started.
 * This handle is used implicitly by \ref sl_rail_read_rx_fifo().
 */
#define SL_RAIL_RX_PACKET_HANDLE_OLDEST   ((sl_rail_rx_packet_handle_t) 1)

/**
 * A special RX packet handle to refer to the oldest unreleased
 * complete packet. This never includes incomplete or unstarted packets.
 * (Using \ref SL_RAIL_RX_PACKET_HANDLE_OLDEST is inappropriate for this
 * purpose because it can refer to an unstarted, incomplete, or
 * unheld packet which are inappropriate to be consumed by the application.)
 */
#define SL_RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE   ((sl_rail_rx_packet_handle_t) 2)

/**
 * A special RX packet handle to refer to the newest unreleased packet
 * when in callback context. For a callback involving a completed
 * receive event, this refers to the packet just completed. For
 * other callback events, this refers to the next packet to be
 * completed, which might be in-progress or might not have even
 * started yet.
 */
#define SL_RAIL_RX_PACKET_HANDLE_NEWEST   ((sl_rail_rx_packet_handle_t) 3)

/**
 * @struct sl_rail_rx_packet_info_t
 * @brief Basic information about a packet being received or already
 *   completed and awaiting processing, including memory pointers to
 *   its data in the circular receive FIFO buffer. This packet information
 *   refers to remaining packet data that has not already been consumed
 *   by \ref sl_rail_read_rx_fifo().
 * @note Because the receive FIFO buffer is circular, a packet might start
 *   near the end of the buffer and wrap around to the beginning of
 *   the buffer to finish, hence the distinction between the first
 *   and last portions. Packets that fit without wrapping only have
 *   a first portion (first_portion_bytes == packet_bytes and last_portion_data
 *   will be NULL).
 */
typedef struct sl_rail_rx_packet_info {
  /**
   * The packet status of this packet.
   */
  sl_rail_rx_packet_status_t packet_status;
  /**
   * The number of packet data bytes available to read in this packet.
   */
  uint16_t packet_bytes;
  /**
   * The number of bytes in the first portion.
   */
  uint16_t first_portion_bytes;
  /**
   * The pointer to the first portion of packet data containing
   * first_portion_bytes number of bytes.
   */
  uint8_t *p_first_portion_data;
  /**
   * The pointer to the last portion of a packet, if any; NULL otherwise.
   * The number of bytes in this portion is
   * packet_bytes - first_portion_bytes.
   */
  uint8_t *p_last_portion_data;
  /**
   * A bitmask representing which address filter(s) this packet has passed.
   * Will be 0 when not filtering or if packet info is retrieved before
   * filtering has completed. It's undefined on platforms lacking \ref
   * SL_RAIL_SUPPORTS_ADDR_FILTER_MASK
   */
  sl_rail_addr_filter_mask_t filter_mask;
} sl_rail_rx_packet_info_t;

/**
 * @struct sl_rail_rx_packet_details_t
 * @brief Received packet details obtained via \ref sl_rail_get_rx_packet_details().
 *
 * @note Certain details are always available, while others are only available
 *   if the \ref sl_rail_rx_options_t \ref SL_RAIL_RX_OPTION_REMOVE_APPENDED_INFO
 *   option is not in effect and the received packet's
 *   \ref sl_rail_rx_packet_status_t is among the SL_RAIL_RX_PACKET_READY_ set.
 *   Each detail's description indicates its availability.
 */
typedef struct sl_rail_rx_packet_details {
  /**
   * The time stamp of the received packet in the RAIL timebase.
   *
   * When not available it will be \ref SL_RAIL_PACKET_TIME_INVALID.
   */
  sl_rail_packet_time_stamp_t time_received;
  /**
   * Indicates whether the received packet successfully passed CRC checks
   * or failed (not just CRC failure).
   * It is true for \ref SL_RAIL_RX_PACKET_READY_SUCCESS packets and false
   * for all other \ref sl_rail_rx_packet_status_t values reported in \ref
   * sl_rail_rx_packet_info_t::packet_status.
   *
   * It is always available.
   */
  bool crc_passed;
  /**
   * Indicate whether the received packet was the expected Ack.
   * It is true for the expected Ack and false otherwise.
   *
   * It is always available.
   *
   * An expected Ack is defined as a protocol-correct Ack packet
   * successfully-received (\ref SL_RAIL_RX_PACKET_READY_SUCCESS or
   * \ref SL_RAIL_RX_PACKET_READY_CRC_ERROR) and whose sync word was
   * detected within the
   * sl_rail_auto_ack_config_t::ack_timeout_us period following a transmit
   * which specified \ref SL_RAIL_TX_OPTION_WAIT_FOR_ACK, requested
   * an Ack, and Auto-Ack is enabled. When true, the ack_timeout_us
   * period was terminated so no \ref SL_RAIL_EVENT_RX_ACK_TIMEOUT
   * will be subsequently posted for the transmit.
   *
   * A "protocol-correct Ack" applies to the 802.15.4 or Z-Wave
   * protocols for which RAIL can discern the frame type and match
   * the Ack's sequence number with that of the transmitted frame.
   * For other protocols, the first packet successfully-received
   * whose sync word was detected within the \ref
   * sl_rail_auto_ack_config_t::ack_timeout_us period is
   * considered the expected Ack; upper layers are responsible for
   * confirming this.
   */
  bool is_ack;
  /**
   * RSSI of the received packet in integer dBm. This RSSI measurement is
   * started as soon as the sync word is detected. The duration of the
   * measurement is PHY-specific.
   *
   * When not available it will be \ref SL_RAIL_RSSI_INVALID_DBM.
   */
  int8_t rssi_dbm;
  /**
   * The link quality indicator of the received packet. A zero would
   * indicate a very low quality packet while a 255 would indicate a very
   * high quality packet.
   *
   * When not available it will be 0.
   */
  uint8_t lqi;
  /**
   * For radios and PHY configurations that support multiple sync words, this
   * number is the ID of the sync word that was used for this packet.
   *
   * It is always available.
   */
  uint8_t sync_word_id;
  /**
   * In configurations where the radio has the option of receiving a given
   * packet in multiple ways, indicates which of the sub-PHY options
   * was used to receive the packet. Most radio configurations do not have
   * this ability and the sub_phy_id is set to 0.
   *
   * Currently, this field is used by the BLE Coded PHY, the BLE Simulscan PHY
   * and the SUN OFDM PHYs.
   * In BLE cases, a value of 0 marks a 500 kbps packet, a value of 1 marks a 125
   * kbps packet, and a value of 2 marks a 1 Mbps packet.
   * Also, see \ref sl_rail_ble_config_phy_coded() and \ref sl_rail_ble_config_phy_simulscan().
   *
   * In SUN OFDM cases, the value corresponds to the numerical value of the
   * Modulation and Coding Scheme (MCS) level of the last received packet.
   * The packet bitrate depends on the MCS value, as well as the OFDM option.
   * Packets bitrates for SUN OFDM PHYs can be found in 802.15.4-2020 specification,
   * chapter 20.3, table 20-10.
   * Ex: Packet bitrate for OFDM option 1 MCS0 is 100kb/s and 2400kb/s for MCS6.
   *
   * In WMBUS cases, when using PHY_wMbus_ModeTC_M2O_100k_frameA with simultaneous
   * RX of T and C modes enabled (\ref sl_rail_wmbus_config()), the value corresponds
   * to \ref sl_rail_wmbus_phy_t.
   *
   * It is always available.
   */
  uint8_t sub_phy_id;
  /**
   * For \ref Antenna_Control configurations where the device has multiple
   * antennas, this indicates which antenna received the packet. When there
   * is only one antenna, this will be set to the default of 0.
   *
   * It is always available.
   */
  uint8_t antenna_id;
  /**
   * When channel hopping is enabled, this field will contain the index
   * of the channel in \ref sl_rail_rx_channel_hopping_config_t::p_entries on which
   * this packet was received, or a sentinel value.
   *
   * It is always available.
   */
  uint8_t channel_hopping_channel_index;
  /**
   * The channel on which the packet was received.
   *
   * It is always available.
   *
   * @note It is best to fully process (empty or clear) the receive FIFO
   *   before changing channel configurations (\ref sl_rail_config_channels()
   *   or a built-in configuration) as unprocessed packets' channel
   *   could reflect the wrong configuration.
   */
  uint16_t channel;
} sl_rail_rx_packet_details_t;

/**
 * @typedef sl_rail_convert_lqi_callback_t
 * @brief A pointer to a function called before LQI is copied into the
 *   \ref sl_rail_rx_packet_details_t structure.
 *
 * @param[in] lqi The LQI value obtained by hardware and being readied for
 *   application consumption. This LQI value is in integral units ranging from
 *   0 to 255.
 * @param[in] rssi_dbm The RSSI value corresponding to the packet from which the
 *   hardware LQI value was obtained. This RSSI value is in integral dBm units.
 * @return uint8_t The converted LQI value that will be loaded into the
 *   \ref sl_rail_rx_packet_details_t structure in preparation for application
 *   consumption. This value should likewise be in integral units ranging from
 *   0 to 255.
 */
typedef uint8_t (*sl_rail_convert_lqi_callback_t)(uint8_t lqi,
                                                  int8_t rssi_dbm);

/**
 * @struct sl_rail_prs_lna_bypass_config_t
 * @brief Configures the automatic PRS LNA bypass.
 */
typedef struct sl_rail_prs_lna_bypass_config {
  /**
   * Maximum time in microseconds to wait for frame detection after the LNA has
   * been bypassed. It must be greater than 0 to enable automatic PRS LNA bypass
   * with \ref sl_rail_enable_prs_lna_bypass().
   */
  uint32_t timeout_us;
  /**
   * Threshold (without unit) from which LNA bypass is turned on.
   * The table below shows EFR32XG25 treshold corresponding to received power
   * level without the LNA gain.
   *
   * |  Level dBm | FSK_1a | FSK_1b | FSK_2a | FSK_2b | FSK_3 | FSK_4a | FSK_4b | FSK_5 | OFDM1 | OFDM2 | OFDM3 | OFDM4 |
   * |------------|--------|--------|--------|--------|-------|--------|--------|-------|-------|-------|-------|-------|
   * |   __-25__  |        |        |        |        |       |        |        |       |   9   |   9   |   9   |   10  |
   * |   __-20__  |        |    7   |    7   |    7   |   8   |    8   |    7   |   8   |   11  |   12  |   12  |   12  |
   * |   __-15__  |    7   |    10  |    10  |    10  |   9   |    9   |    10  |   10  |   14  |   14  |   14  |   15  |
   * |   __-10__  |    9   |    12  |    12  |    12  |   12  |    12  |    12  |   12  |   16  |   16  |   16  |   16  |
   * |   __-5__   |    11  |    14  |    14  |    14  |   16  |    16  |    14  |   16  |       |       |       |       |
   * |    __0__   |    14  |    17  |    18  |    17  |   17  |    18  |    18  |   18  |       |       |       |       |
   *
   * For example, with OFDM1 PHY, setting the threshold to 11 will turn on the
   * bypass when the power level at EFR32XG25 input is greater than -20 dBm.
   */
  uint8_t threshold;
  /**
   * Compensation in dB applied by RAIL to RSSI during LNA bypass. The RSSI
   * offset set using \ref sl_rail_set_rssi_offset() must corespond to the case
   * with FEM LNA not bypassed. delta_rssi_db is typically the FEM LNA gain
   * value.
   */
  uint8_t delta_rssi_db;
  /**
   * PRS Channel used for the bypass.
   * PRS_GetFreeChannel() can be use to find a free channel. Then the signal
   * can be routed to GPIO pin and port using PRS_PinOutput(). This allows
   * logical operations with other PRS channels and so to adapt to the FEM
   * control logic table. Any call to PRS_Combine() with
   * \ref sl_rail_prs_lna_bypass_config_t::prs_channel as chA must be done after
   * the \ref sl_rail_enable_prs_lna_bypass() call.
   */
  uint8_t prs_channel;
  /**
   * PRS signal polarity for bypass.
   *
   * With a polarity of 1, PRS signal is set to 1 for bypass and 0 for un-bypass.
   * with a polarity of 0, PRS signal is set to 0 for bypass and 1 for un-bypass.
   */
  bool polarity;
} sl_rail_prs_lna_bypass_config_t;

/** @} */ // end of group Receive

/******************************************************************************
 * Auto-Ack Structures
 *****************************************************************************/
/**
 * @addtogroup Auto_Ack
 * @{
 */
/**
 * @struct sl_rail_auto_ack_config_t
 * @brief Enable/disable the Auto-Ack algorithm, based on "enable".
 *
 * The structure provides a default state (the "success" of TX/RX transitions
 * when Acking is enabled) for the radio to return to after an Ack
 * operation occurs (transmitting or attempting to receive an Ack), or normal
 * state transitions to return to in the case Acking is
 * disabled. Regardless whether the Ack operation was successful, the
 * radio returns to the specified success state.
 *
 * ack_timeout_us specifies how long to stay in receive and wait for an Ack
 * to start (sync detected) before issuing a \ref SL_RAIL_EVENT_RX_ACK_TIMEOUT
 * event and return to the default state.
 */
typedef struct sl_rail_auto_ack_config {
  /**
   * Indicate whether Auto-Acking should be enabled or disabled.
   */
  bool enable;
  // Unnamed 'uint8_t reserved_1[1]' pad byte field here.
  /**
   * Define the RX Ack timeout duration in microseconds up to 65535
   * microseconds maximum. Only applied when Auto-Acking is enabled.
   * The Ack timeout timer starts at the completion of a \ref
   * SL_RAIL_TX_OPTION_WAIT_FOR_ACK transmit and expires only while waiting
   * for a packet (prior to SYNC detect), triggering \ref
   * SL_RAIL_EVENT_RX_ACK_TIMEOUT. During packet reception that event is
   * held off until packet completion and suppressed entirely if the
   * received packet is the expected Ack.
   */
  uint16_t ack_timeout_us;
  /**
   * State transitions to do after receiving a packet. When Auto-Acking is
   * enabled, the "error" transition is always ignored and the radio will
   * return to the "success" state after any Acking sequence
   * (\ref SL_RAIL_RF_STATE_RX or \ref SL_RAIL_RF_STATE_IDLE).
   * See \ref sl_rail_config_auto_ack() for more details on this.
   */
  sl_rail_state_transitions_t rx_transitions;
  /**
   * State transitions to do after transmitting a packet. When Auto-Acking is
   * enabled, the "error" transition is always ignored and the radio will
   * return to the "success" state after any Acking sequence
   * (\ref SL_RAIL_RF_STATE_RX or \ref SL_RAIL_RF_STATE_IDLE).
   * See \ref sl_rail_config_auto_ack() for more details on this.
   */
  sl_rail_state_transitions_t tx_transitions;
} sl_rail_auto_ack_config_t;

/**
 * @def SL_RAIL_DEFAULT_AUTO_ACK_FIFO_BYTES
 * @brief The default size in bytes of the built-in per-protocol Auto-Ack FIFO.
 */
#define SL_RAIL_DEFAULT_AUTO_ACK_FIFO_BYTES SL_RAIL_MINIMUM_FIFO_BYTES

/** @} */ // end of group Auto_Ack

/******************************************************************************
 * Antenna Control
 *****************************************************************************/
/**
 * @addtogroup Antenna_Control
 * @{
 *
 * These enumerations and structures are used with RAIL Antenna Control API.
 * EFR32 supports up to two antennas with configurable pin locations.
 */

/**
 * Antenna path Selection enumeration.
 */
SLI_RAIL_ENUM(sl_rail_antenna_sel_t) {
  /**
   * Enum for antenna path 0.
   */
  SL_RAIL_ANTENNA_0 = 0u,
  /**
   * Enum for antenna path 1.
   */
  SL_RAIL_ANTENNA_1 = 1u,
  /**
   * Enum for antenna path auto.
   */
  SL_RAIL_ANTENNA_AUTO = 255u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ANTENNA_0    ((sl_rail_antenna_sel_t) SL_RAIL_ANTENNA_0)
#define SL_RAIL_ANTENNA_1    ((sl_rail_antenna_sel_t) SL_RAIL_ANTENNA_1)
#define SL_RAIL_ANTENNA_AUTO ((sl_rail_antenna_sel_t) SL_RAIL_ANTENNA_AUTO)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @struct sl_rail_antenna_config_t
 * @brief A configuration for antenna selection.
 */
typedef struct sl_rail_antenna_config {
  /**
   * Antenna 0 Pin Enable.
   */
  bool antenna_0_pin_enable;
  /**
   * Antenna 1 Pin Enable.
   */
  bool antenna_1_pin_enable;
  /**
   * A \ref sl_rail_antenna_sel_t
   * value specifying the internal default RF path. It is ignored
   * on EFR32 parts that have only one RF path bonded
   * out and on EFR32xG28 dual-band OPNs where the appropriate
   * RF path is automatically set by RAIL to 0 for 2.4 GHz band
   * and 1 for Sub-GHz band PHYs. On EFR32xG23 and EFR32xG28
   * single-band OPNs where both RF paths are bonded out this can
   * be set to \ref SL_RAIL_ANTENNA_AUTO to effect internal RF path
   * diversity on PHYs supporting diversity. This avoids the need
   * for an external RF switch and the associated GPIO(s) needed
   * to control its antenna selection.
   */
  uint8_t default_rf_path;
  /**
   * Antenna 0 output GPIO port.
   */
  sl_rail_antenna_sel_t antenna_0_port;
  /**
   * Antenna 0 output GPIO pin.
   */
  uint8_t antenna_0_pin;
  /**
   * @deprecated No longer used (ignored).
   */
  uint8_t reserved;
  /**
   * Antenna 1 output GPIO port.
   */
  uint8_t antenna_1_port;
  /**
   * Antenna 1 output GPIO pin.
   */
  uint8_t antenna_1_pin;
} sl_rail_antenna_config_t;

/** @} */ // end of group Antenna_Control

/******************************************************************************
 * External_Thermistor Structures
 *****************************************************************************/
/**
 * @addtogroup External_Thermistor
 * @{
 */

/**
 * A sentinel value to indicate an invalid thermistor measurement value.
 */
#define SL_RAIL_INVALID_THERMISTOR_VALUE (0xFFFFFFFFU)
/**
 * A sentinel value to indicate an invalid PPM calculation value.
 */
#define SL_RAIL_INVALID_PPM_VALUE   (-128)

/**
 * @struct sl_rail_hfxo_thermistor_config_t
 * @brief Configure the port and pin of the thermistor.
 *
 * @note This configuration is chip OPN dependent.
 */
typedef struct sl_rail_hfxo_thermistor_config {
  /**
   * The GPIO port to access the thermistor.
   */
  uint8_t port;
  /**
   * The GPIO pin to set the thermistor.
   */
  uint8_t pin;
} sl_rail_hfxo_thermistor_config_t;

/**
 * @struct sl_rail_hfxo_compensation_config_t
 * @brief Set compensation specific parameters
 */
typedef struct sl_rail_hfxo_compensation_config {
  /**
   * Indicates whether the HFXO compensation in temperature is activated.
   */
  bool enable_compensation;
  /**
   * The temperature reference delimiting the nominal zone from the critical one.
   * This field is relevant if enable_compensation is set to true.
   */
  int8_t zone_temperature_celsius;
  /**
   * The temperature shift used to start a new compensation, in the nominal zone.
   * This field is relevant if enable_compensation is set to true.
   */
  uint8_t delta_nominal_celsius;
  /**
   * The temperature shift used to start a new compensation, in the critical zone.
   * This field is relevant if enable_compensation is set to true.
   */
  uint8_t delta_critical_celsius;
} sl_rail_hfxo_compensation_config_t;

/** @} */ // end of group External_Thermistor

/******************************************************************************
 * Calibration Structures
 *****************************************************************************/
/**
 * @addtogroup Calibration
 * @{
 *
 * The EFR32 supports the Image Rejection (IR)
 * calibration and a temperature-dependent calibration. The IR calibration
 * can be computed once and stored off or computed each time at
 * startup. Because it is PHY-specific and provides sensitivity improvements,
 * it is highly recommended. The IR calibration should only be run when the
 * radio is IDLE.
 *
 * The temperature-dependent calibrations are used to recalibrate the synth if
 * the temperature crosses 0C or the temperature delta since the last
 * calibration exceeds 70C while in receive. RAIL will run the VCO calibration
 * automatically upon entering receive or transmit states, so the application
 * can omit this calibration if the stack re-enters receive or transmit with
 * enough frequency to avoid reaching the temperature delta. If the application
 * does not calibrate for temperature, it's possible to miss receive packets due
 * to a drift in the carrier frequency.
 */

/**
 * @typedef sl_rail_cal_mask_t
 * @brief A calibration mask type
 *
 * This type is a bitmask of different RAIL calibration values. The exact
 * meaning of these bits depends on what a particular chip supports.
 */
typedef uint32_t sl_rail_cal_mask_t;

/**
 * EFR32-specific temperature calibration bit.
 */
#define SL_RAIL_CAL_TEMP_VCO         (0x00000001U)
/**
 * EFR32-specific HFXO temperature check bit.
 * (Ignored if platform lacks \ref SL_RAIL_SUPPORTS_HFXO_COMPENSATION.)
 */
#define SL_RAIL_CAL_TEMP_HFXO        (0x00000002U)
/**
 * EFR32-specific HFXO compensation bit.
 * (Ignored if platform lacks \ref SL_RAIL_SUPPORTS_HFXO_COMPENSATION.)
 */
#define SL_RAIL_CAL_COMPENSATE_HFXO  (0x00000004U)
/**
 * EFR32-specific IR calibration bit.
 */
#define SL_RAIL_CAL_RX_IR_CAL        (0x00010000U)
/**
 * EFR32-specific Tx IR calibration bit.
 * (Ignored if platform lacks \ref SL_RAIL_SUPPORTS_OFDM_PA.)
 */
#define SL_RAIL_CAL_OFDM_TX_IR_CAL   (0x00100000U)

/**
 * A mask to run EFR32-specific IR calibrations.
 */
#define SL_RAIL_CAL_ONETIME_IR_CAL   (SL_RAIL_CAL_RX_IR_CAL | SL_RAIL_CAL_OFDM_TX_IR_CAL)
/**
 * A mask to run temperature-dependent calibrations.
 */
#define SL_RAIL_CAL_TEMP             (SL_RAIL_CAL_TEMP_VCO | SL_RAIL_CAL_TEMP_HFXO | SL_RAIL_CAL_COMPENSATE_HFXO)
/**
 * A mask to run one-time calibrations.
 */
#define SL_RAIL_CAL_ONETIME          (SL_RAIL_CAL_ONETIME_IR_CAL)
/**
 * A mask to run optional performance calibrations.
 */
#define SL_RAIL_CAL_PERF             (0)
/**
 * A mask for calibrations that require the radio to be off.
 */
#define SL_RAIL_CAL_OFFLINE          (SL_RAIL_CAL_ONETIME_IR_CAL)
/**
 * A mask to run all possible calibrations for this chip.
 */
#define SL_RAIL_CAL_ALL              (SL_RAIL_CAL_TEMP | SL_RAIL_CAL_ONETIME)
/**
 * A mask to run all pending calibrations.
 */
#define SL_RAIL_CAL_ALL_PENDING      (0x00000000U)
/**
 * An invalid calibration value.
 */
#define SL_RAIL_CAL_INVALID_VALUE    (0xFFFFFFFFU)

/**
 * @def SL_RAIL_MAX_RF_PATHS
 * @brief Indicates the maximum number of RF Paths supported across all
 *   platforms.
 */
#define SL_RAIL_MAX_RF_PATHS 2

/**
 * sl_rail_rx_ir_cal_values_t
 * @brief RX IR calibration values.
 *
 * Platforms with fewer \ref SL_RAIL_RF_PATHS than \ref SL_RAIL_MAX_RF_PATHS
 * will only respect and update \ref SL_RAIL_RF_PATHS indices and ignore
 * the rest.
 */
typedef uint32_t sl_rail_rx_ir_cal_values_t[SL_RAIL_MAX_RF_PATHS];

/**
 * A define to set all \ref sl_rail_rx_ir_cal_values_t values to uninitialized.
 *
 * This define can be used when you have no data to pass to the calibration
 * routines but wish to compute and save all possible calibrations.
 */
#define SL_RAIL_RX_IR_CAL_VALUES_UNINIT {                         \
    [0 ... SL_RAIL_MAX_RF_PATHS - 1] = SL_RAIL_CAL_INVALID_VALUE, \
}

/**
 * @struct sl_rail_tx_ir_cal_values_t
 * @brief A Tx IR calibration value structure.
 *
 * This definition contains the set of persistent calibration values for
 * OFDM on EFR32. You can set these beforehand and apply them at startup
 * to save the time required to compute them. Any of these values may be
 * set to \ref SL_RAIL_CAL_INVALID_VALUE to force the code to compute that
 * calibration value.
 *
 * Only supported on platforms with \ref SL_RAIL_SUPPORTS_OFDM_PA enabled.
 */
typedef struct sl_rail_tx_ir_cal_values {
  /**
   * Tx IR Cal result.
   */
  uint32_t dc_offset_iq;
  /**
   * Tx IR Cal result.
   */
  uint32_t phi_epsilon;
} sl_rail_tx_ir_cal_values_t;

/**
 * A define to set all \ref sl_rail_tx_ir_cal_values_t values to uninitialized.
 *
 * This define can be used when you have no data to pass to the calibration
 * routines but wish to compute and save all possible calibrations.
 */
#define SL_RAIL_TX_IR_CAL_VALUES_UNINIT  {     \
    .dc_offset_iq = SL_RAIL_CAL_INVALID_VALUE, \
    .phi_epsilon = SL_RAIL_CAL_INVALID_VALUE,  \
}

/**
 * @struct sl_rail_ir_cal_values_t
 * @brief An IR calibration value structure.
 *
 * This definition contains the set of persistent calibration values for
 * EFR32. You can set these beforehand and apply them at startup to save the
 * time required to compute them. Any of these values may be set to
 * \ref SL_RAIL_CAL_INVALID_VALUE to force the code to compute that calibration value.
 */
typedef struct sl_rail_ir_cal_values {
  /**
   * RX Image Rejection (IR) calibration value(s).
   */
  sl_rail_rx_ir_cal_values_t rx_ir_cal_values;
  /**
   * TX Image Rejection (IR) calibration value(s) for OFDM.
   */
  sl_rail_tx_ir_cal_values_t tx_ir_cal_values;
} sl_rail_ir_cal_values_t;

/**
 * A define to set all \ref sl_rail_ir_cal_values_t values to uninitialized.
 *
 * This define can be used when you have no data to pass to the calibration
 * routines but wish to compute and save all possible calibrations.
 */
#define SL_RAIL_IR_CAL_VALUES_UNINIT {                   \
    .rx_ir_cal_values = SL_RAIL_RX_IR_CAL_VALUES_UNINIT, \
    .tx_ir_cal_values = SL_RAIL_TX_IR_CAL_VALUES_UNINIT, \
}

/**
 * A define allowing Rx calibration value access compatibility
 * between non-OFDM and OFDM platforms.
 */
#define SL_RAIL_IR_CAL_VAL(ir_cal_struct, rf_path) \
  (((sl_rail_ir_cal_values_t *)(&(ir_cal_struct)))->rx_ir_cal_values[(rf_path)])

/**
 * @typedef sl_rail_cal_values_t
 * @brief A calibration value structure.
 *
 * This structure contains the set of persistent calibration values for
 * EFR32. You can set these beforehand and apply them at startup to save the
 * time required to compute them. Any of these values may be set to \ref
 * SL_RAIL_CAL_INVALID_VALUE to force the code to compute that calibration value.
 */
typedef sl_rail_ir_cal_values_t sl_rail_cal_values_t;

/**
 * A define to set all \ref sl_rail_cal_values_t values to uninitialized.
 *
 * This define can be used when you have no data to pass to the calibration
 * routines but wish to compute and save all possible calibrations.
 */
#define SL_RAIL_CAL_VALUES_UNINIT SL_RAIL_IR_CAL_VALUES_UNINIT

/** @} */ // end of group Calibration

/******************************************************************************
 * RF Sense Structures
 *****************************************************************************/
/**
 * @addtogroup Rf_Sense
 * @{
 */

/**
 * A pointer to an RF Sense callback function.
 *
 * Consider using the event \ref SL_RAIL_EVENT_RF_SENSED as an alternative.
 */
typedef void (*sl_rail_rf_sense_callback_t)(void);

/**
 * RF Sense low sensitivity offset.
 */
#define SL_RAIL_RF_SENSE_LOW_SENSITIVITY_OFFSET   (0x20U)

/**
 * @enum sl_rail_rf_sense_band_t
 * @brief An enumeration for specifying the RF Sense frequency band.
 */
SLI_RAIL_ENUM(sl_rail_rf_sense_band_t) {
  /**
   * RF Sense is disabled.
   */
  SL_RAIL_RF_SENSE_OFF = 0u,
  /**
   * RF Sense is in 2.4 GHz band.
   * The sensitivity level associated with this band is configurable
   * via \ref sl_rail_rf_sense_high_sensitivity_value.
   */
  SL_RAIL_RF_SENSE_2P4_GHZ = 1u,
  /**
   * RF Sense is in Sub-GHz band.
   * The sensitivity level associated with this band is configurable
   * via \ref sl_rail_rf_sense_high_sensitivity_value.
   */
  SL_RAIL_RF_SENSE_SUB_GHZ = 2u,
  /**
   * RF Sense is in both bands.
   * The sensitivity level associated with this band is configurable
   * via \ref sl_rail_rf_sense_high_sensitivity_value.
   */
  SL_RAIL_RF_SENSE_ANY = 3u,
  /**
   * A count of the basic choices in this enumeration.
   * Must be last before sensitivity options.
   */
  SL_RAIL_RF_SENSE_MAX,

  /**
   * RF Sense is in low sensitivity 2.4 GHz band.
   * The sensitivity level associated with this band is configurable
   * via \ref sl_rail_rf_sense_low_sensitivity_value.
   */
  SL_RAIL_RF_SENSE_2P4_GHZ_LOW_SENSITIVITY = SL_RAIL_RF_SENSE_LOW_SENSITIVITY_OFFSET + SL_RAIL_RF_SENSE_2P4_GHZ,
  /**
   * RF Sense is in low sensitivity Sub-GHz band.
   * The sensitivity level associated with this band is configurable
   * via \ref sl_rail_rf_sense_low_sensitivity_value.
   */
  SL_RAIL_RF_SENSE_SUB_GHZ_LOW_SENSITIVITY = SL_RAIL_RF_SENSE_LOW_SENSITIVITY_OFFSET + SL_RAIL_RF_SENSE_SUB_GHZ,
  /**
   * RF Sense is in low sensitivity for both bands.
   * The sensitivity level associated with this band is configurable
   * via \ref sl_rail_rf_sense_low_sensitivity_value.
   */
  SL_RAIL_RF_SENSE_ANY_LOW_SENSITIVITY = SL_RAIL_RF_SENSE_LOW_SENSITIVITY_OFFSET + SL_RAIL_RF_SENSE_ANY,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_RF_SENSE_OFF                     ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_OFF)
#define SL_RAIL_RF_SENSE_2P4_GHZ                 ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_2P4_GHZ)
#define SL_RAIL_RF_SENSE_SUB_GHZ                 ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_SUB_GHZ)
#define SL_RAIL_RF_SENSE_ANY                     ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_ANY)
#define SL_RAIL_RF_SENSE_MAX                     ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_MAX)
#define SL_RAIL_RF_SENSE_2P4_GHZ_LOW_SENSITIVITY ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_2P4_GHZ_LOW_SENSITIVITY)
#define SL_RAIL_RF_SENSE_SUB_GHZ_LOW_SENSITIVITY ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_SUB_GHZ_LOW_SENSITIVITY)
#define SL_RAIL_RF_SENSE_ANY_LOW_SENSITIVITY     ((sl_rail_rf_sense_band_t) SL_RAIL_RF_SENSE_ANY_LOW_SENSITIVITY)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * Use the MODEM default sync word.
 */
#define SL_RAIL_RF_SENSE_USE_HW_SYNC_WORD   (0U)

/**
 * @struct sl_rail_rf_sense_selective_ook_config_t
 * @brief Structure to configure RF Sense Selective(OOK) mode.
 */
typedef struct sl_rail_rf_sense_selective_ook_config {
  /**
   * The frequency band(s) on which to sense the
   * RF energy. To stop RF Sense, specify \ref SL_RAIL_RF_SENSE_OFF.
   */
  sl_rail_rf_sense_band_t band;
  /**
   * Sync word length in bytes, 1-4 bytes.
   * @note When \ref sync_word is set to use \ref SL_RAIL_RF_SENSE_USE_HW_SYNC_WORD,
   *   the \ref sync_word_bytes value will be ignored since we rely on the
   *   HW default settings for sync word.
   */
  uint8_t sync_word_bytes;
  /**
   * Sync Word Value.
   * To use HW default sync word, set to \ref SL_RAIL_RF_SENSE_USE_HW_SYNC_WORD.
   */
  uint32_t sync_word;
  /**
   * The callback function, called when RF is sensed.
   *
   * @note Set to NULL and instead use \ref SL_RAIL_EVENT_RF_SENSED or poll
   *   via \ref sl_rail_is_rf_sensed().
   */
  sl_rail_rf_sense_callback_t cb;
} sl_rail_rf_sense_selective_ook_config_t;

/** @} */ // end of group Rf_Sense

/******************************************************************************
 * RX Channel Hopping
 *****************************************************************************/
/**
 * @addtogroup Rx_Channel_Hopping RX Channel Hopping
 * @{
 */

/**
 * @enum sl_rail_rx_channel_hopping_mode_t
 * @brief Modes by which RAIL can determine when to proceed to the next
 * channel during channel hopping
 */
SLI_RAIL_ENUM(sl_rail_rx_channel_hopping_mode_t) {
  /**
   * Switch to the next channel each time the radio re-enters RX after
   * packet reception or a transmit based on the corresponding \ref
   * State_Transitions. A hop can also be manually triggered by calling
   * \ref sl_rail_trigger_rx_channel_hop() while the radio is listening.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL = 0u,
  /**
   * Switch to the next channel after a certain amount of time passes.
   * The time should be specified in microseconds in \ref
   * sl_rail_rx_channel_hopping_config_entry_t::parameter, and must be less
   * than \ref SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT = 1u,
  /**
   * Listen in receive RX for at least a specified timeout. If,
   * by the end of the timeout, the radio has packet timing,
   * remain in the current channel until the radio loses it. The
   * timeout should be specified in microseconds in \ref
   * sl_rail_rx_channel_hopping_config_entry_t::parameter, and must be less
   * than \ref SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE = 2u,
  /**
   * Listen in receive RX for at least a specified timeout. If,
   * by the end of the timeout, the radio has a packet preamble,
   * remain in the current channel until the radio loses it. The
   * timeout should be specified in microseconds in \ref
   * sl_rail_rx_channel_hopping_config_entry_t::parameter, and must be less
   * than \ref SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE = 3u,
  /**
   * Placeholder for a reserved hopping mode that is not supported.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4 = 4u,
  /**
   * A mode that combines modes TIMING_SENSE, PREAMBLE_SENSE, and
   * TIMEOUT (sync detect) all running in parallel. Refer to \ref
   * sl_rail_rx_channel_hopping_config_multi_mode_t for further details.
   * A pointer to that structure, allocated in global read-write
   * memory and initialized to the desired configuration values, is
   * specified as the \ref sl_rail_rx_channel_hopping_config_entry_t::parameter
   * or \ref sl_rail_rx_duty_cycle_config_t::parameter, cast appropriately:
   * @code{.c}
   *   .parameter = (uint32_t)(void *)&hopping_config_multi_mode,
   * @endcode
   *
   * @note RAIL will overwrite the contents of the \ref
   *   sl_rail_rx_channel_hopping_config_multi_mode_t during operation so it
   *   must be reinitialized with the desired configuration prior to
   *   every call to \ref sl_rail_config_rx_channel_hopping() or
   *   \ref sl_rail_config_rx_duty_cycle().
   *
   * @note Multiple \ref sl_rail_rx_channel_hopping_config_entry_t entries may
   *   share the same \ref sl_rail_rx_channel_hopping_config_multi_mode_t if their
   *   settings are identical, otherwise a separate \ref
   *   sl_rail_rx_channel_hopping_config_multi_mode_t is needed for each
   *   \ref sl_rail_rx_channel_hopping_config_entry_t that uses this mode.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE = 5u,
  /**
   * Switch to the next channel based on the demodulation settings in the PHY
   * config. This mode is PHY and chip dependent. The
   * \ref sl_rail_rx_channel_hopping_config_entry_t::parameter is ignored, and should
   * be set to 0 for future compatibility.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ = 6u,
  /**
   * Marks that the channel is concurrent with another channel, and otherwise
   * behaves identically to \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC = 7u,
  /**
   * Indicates that this channel is a virtual channel that is concurrently
   * detected with the channel indicated by the
   * \ref sl_rail_rx_channel_hopping_config_entry_t::parameter. Otherwise behaves
   * identically to \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT = 8u,
  /**
   * This is the transmit channel used for Auto-Ack if the regular channel,
   * specified in sl_rail_rx_channel_hopping_config_entry::parameter, is
   * optimized for RX which may degrade some TX performance
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX = 9u,
  /**
   * A count of the basic choices in this enumeration.
   * Must be last before _WITH_OPTIONS twins.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODES_COUNT,

  /**
   * The start of equivalent modes requiring non-default \ref
   * sl_rail_rx_duty_cycle_config_t::options, needed for backwards-compatibility
   * with earlier \ref sl_rail_rx_duty_cycle_config_t format. Non-default options
   * are supported with \ref sl_rail_rx_channel_hopping_config_entry_t in all modes.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE = 0x80u,
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4 with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT),
  /**
   * Variant of \ref SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX with options.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX_WITH_OPTIONS
    = (SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE
       + SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX),
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL                      ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT                     ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE                ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE              ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4                  ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE                 ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ                          ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC                        ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT                          ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX                          ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE          ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODES_WITH_OPTIONS_BASE)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL_WITH_OPTIONS         ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT_WITH_OPTIONS        ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMEOUT_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE_WITH_OPTIONS   ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_TIMING_SENSE_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE_WITH_OPTIONS ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_PREAMBLE_SENSE_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4_WITH_OPTIONS     ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_RESERVED_4_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE_WITH_OPTIONS    ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ_WITH_OPTIONS             ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_SQ_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC_WITH_OPTIONS           ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_CONC_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT_WITH_OPTIONS             ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_VT_WITH_OPTIONS)
#define SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX_WITH_OPTIONS             ((sl_rail_rx_channel_hopping_mode_t) SL_RAIL_RX_CHANNEL_HOPPING_MODE_TX_WITH_OPTIONS)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * The maximum sense time supported for those \ref sl_rail_rx_channel_hopping_mode_t
 * modes whose parameter(s) specify a sensing time.
 */
#define SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US 0x08000000UL

/**
 * @typedef sl_rail_rx_channel_hopping_parameter_t
 * @brief Rx channel hopping on-channel time
 */
typedef uint32_t sl_rail_rx_channel_hopping_parameter_t;

/**
 * @enum sl_rail_rx_channel_hopping_options_t
 * @brief Options that can customize channel hopping behavior
 *   on a per-hop basis.
 */
SLI_RAIL_ENUM(sl_rail_rx_channel_hopping_options_t) {
  /**
   * Shift position of \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_SYNTH_CAL bit.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_SYNTH_CAL_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_DC_CAL bit.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_DC_CAL_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_RSSI_THRESHOLD bit.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_OPTION_RSSI_THRESHOLD_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_STOP bit.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_OPTION_STOP_SHIFT = 3,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_RX_CHANNEL_HOPPING_OPTIONS_COUNT
};

/**
 * A value representing no options enabled.
 */
#define SL_RAIL_RX_CHANNEL_HOPPING_OPTIONS_NONE 0U
/**
 * All options disabled by default.
 * Channel hopping will behave as described by other
 * parameters as it did in RAIL 2.7 and earlier.
 */
#define SL_RAIL_RX_CHANNEL_HOPPING_OPTIONS_DEFAULT SL_RAIL_RX_CHANNEL_HOPPING_OPTIONS_NONE
/**
 * An option to skip synth calibration while *hopping into* the channel
 * specified in the current entry.
 */
#define SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_SYNTH_CAL (1U << SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_SYNTH_CAL_SHIFT)
/**
 * An option to skip DC calibration while *hopping into* the channel
 * specified in the current entry.
 */
#define SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_DC_CAL (1U << SL_RAIL_RX_CHANNEL_HOPPING_OPTION_SKIP_DC_CAL_SHIFT)
/**
 * An option to check RSSI after *hopping into* the channel specified
 * in the current entry and hop if that RSSI is below the threshold
 * specified in \ref sl_rail_rx_channel_hopping_config_entry_t::rssi_threshold_dbm.
 * This check runs in parallel with the \ref sl_rail_rx_channel_hopping_mode_t
 * specified and may cause a hop sooner than that mode otherwise would.
 */
#define SL_RAIL_RX_CHANNEL_HOPPING_OPTION_RSSI_THRESHOLD (1U << SL_RAIL_RX_CHANNEL_HOPPING_OPTION_RSSI_THRESHOLD_SHIFT)
/**
 * An option to stop the hopping sequence at this entry in the hop
 * table, which also idles the radio. Intended only for testing
 * purposes and not supported on EFR32xG21.
 */
#define SL_RAIL_RX_CHANNEL_HOPPING_OPTION_STOP (1U << SL_RAIL_RX_CHANNEL_HOPPING_OPTION_STOP_SHIFT)

///
/// @struct sl_rail_rx_channel_hopping_config_multi_mode_t
/// @brief Structure that parameterizes \ref
///   SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE.
///
/// Every \ref sl_rail_rx_channel_hopping_config_entry_t or
/// \ref sl_rail_rx_duty_cycle_config_t that uses \ref
/// SL_RAIL_RX_CHANNEL_HOPPING_MODE_MULTI_SENSE must allocate one of these
/// structures in global read-write memory to provide the settings
/// for this mode and for RAIL to use during hopping or duty cycling.
/// A pointer to this structure, cast appropriately, is what is passed
/// in the corresponding \ref sl_rail_rx_channel_hopping_config_entry_t::parameter
/// or \ref sl_rail_rx_duty_cycle_config_t::parameter.
///
/// The contents of this structure must be initialized prior to each
/// \ref sl_rail_config_rx_channel_hopping() or \ref sl_rail_config_rx_duty_cycle()
/// call and must not be touched thereafter until the next such call.
/// RAIL may change these contents during configuration or operation.
///
/// This mode of operation functions algorithmically like this pseudocode:
/// @code{.c}
/// extern bool channel_hopping; // true if channel hopping, false if duty cycling
/// extern sl_rail_rx_channel_hopping_config_entry_t *hop_config_entry; // current channel
///
/// static sl_rail_rx_channel_hopping_config_multi_mode_t *multi_params;
/// static sl_rail_time_t rx_start_time;
/// static bool preamble_sensed;
///
/// static void hop_or_suspend_rx(uint32_t delay)
/// {
///   disable_demod_events();
///   disable_timer_events();
///   stop_timer();
///   if (channel_hopping) {
///     hop_to_next_channel(delay, &hop_config_entry); // updates hop_config_entry
///   } else {
///     suspend_rx(delay);
///   }
///   on_start_rx(); // resume receive after delay (on new channel if hopping)
/// }
///
/// void on_start_rx(void) // called upon entry to receive
/// {
///   rx_start_time = sl_rail_get_time(SL_RAIL_EFR32_HANDLE);
///   multi_params = (sl_rail_rx_channel_hopping_config_multi_mode_t *)
///                  (void *)hop_config_entry->parameter;
///   start_timer(rx_start_time + multi_params->timing_sense);
///   preamble_sensed = false;
///   enable_timer_events(); // timer will trigger on_timer_event() handler
///   enable_demod_events(); // demod will trigger on_demod_event() handler
/// }
///
/// void on_timer_event(void) // called when timer expires
/// {
///   hop_or_suspend_rx(hop_config_entry->delay);
/// }
///
/// void on_demod_event(void) // called when demodulator state changes
/// {
///   if (DEMOD_TIMING_SENSED) {
///     stop_timer();
///     start_timer(rx_start_time + multi_params->sync_detect);
///   }
///   if (DEMOD_TIMING_LOST) {
///     stop_timer();
///     uint32_t new_timeout = sl_rail_get_time(SL_RAIL_EFR32_HANDLE) + multi_params->timing_re_sense;
///     uint32_t limit_timeout;
///     if (preamble_sensed) {
///       limit_timeout = rx_start_time + multi_params->sync_detect;
///     } else {
///       limit_timeout = rx_start_time + multi_params->preamble_sense;
///     }
///     if (new_timeout > limit_timeout) {
///       new_timeout = limit_timeout;
///     }
///     if (new_timeout > sl_rail_get_time(SL_RAIL_EFR32_HANDLE)) {
///       start_timer(new_timeout);
///     } else {
///       hop_or_suspend_rx(hop_config_entry->delay);
///     }
///   }
///   if (DEMOD_PREAMBLE_SENSED) {
///     preamble_sensed = true;
///   }
///   if (DEMOD_PREAMBLE_LOST) {
///     preamble_sensed = false;
///   }
///   if (DEMOD_SYNC_DETECTED) {
///     disable_demod_events();
///     disable_timer_events();
///     stop_timer();
///     receive_packet(); // stay on channel to receive frame
///     hop_or_suspend_rx(0); // continue RX per state transitions with no delay
///   }
/// }
/// @endcode
///
typedef struct sl_rail_rx_channel_hopping_config_multi_mode {
  /**
   * Switch to the next channel if sync is not detected before
   * this time, in microseconds, measured from entry to Rx.
   * This must be greater than preamble_sense and less than
   * \ref SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US.
   */
  uint32_t sync_detect_us;
  /**
   * Switch to the next channel if timing was sensed but then
   * lost after this time, in microseconds, measured from entry
   * to Rx -- unless preamble had been sensed in which case any
   * switching is deferred to timing_re_sense and, if timing is
   * regained, to sync_detect. This must be greater than timing_sense
   * and less than \ref SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US.
   */
  uint32_t preamble_sense_us;
  /**
   * Switch to the next channel if timing is not sensed before
   * this time, in microseconds, measured from entry to Rx. This
   * must be greater than 2 and less than
   * \ref SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US.
   */
  uint32_t timing_sense_us;
  /**
   * Switch to the next channel if timing was sensed but then
   * lost and not regained before this time, in microseconds,
   * measured from when timing was lost. This must be less than
   * \ref SL_RAIL_RX_CHANNEL_HOPPING_MAX_SENSE_TIME_US.
   */
  uint32_t timing_re_sense_us;
  /**
   * Set this to 0. This field, along with the others, may be
   * used internally by RAIL during configuration or operation.
   */
  uint32_t status;
} sl_rail_rx_channel_hopping_config_multi_mode_t;

/**
 * @struct sl_rail_rx_channel_hopping_config_entry_t
 * @brief Structure that represents one of the channels that is part of a
 *   \ref sl_rail_rx_channel_hopping_config_t sequence of channels used in
 *   channel hopping.
 */
typedef struct sl_rail_rx_channel_hopping_config_entry {
  /**
   * The channel number to be used for this entry in the channel hopping
   * sequence. If this is an invalid channel for the current PHY, the
   * call to \ref sl_rail_config_rx_channel_hopping() will fail.
   */
  uint16_t channel;
  /**
   * The mode by which RAIL determines when to hop to the next channel.
   */
  sl_rail_rx_channel_hopping_mode_t mode;
  // Unnamed 'uint8_t reserved_1[1]' pad byte field here.
  /**
   * Depending on the 'mode' parameter that was specified, this member
   * is used to parameterize that mode. See the comments on each value of
   * \ref sl_rail_rx_channel_hopping_mode_t to learn what to specify here.
   */
  sl_rail_rx_channel_hopping_parameter_t parameter;
  /**
   * Idle time in microseconds to wait before hopping into the
   * channel indicated by this entry.
   */
  uint32_t delay_us;
  /**
   * Reserved for future use.
   */
  uint8_t reserved_0;
  /**
   * Bitmask of various options that can be applied to the current
   * channel hop.
   */
  sl_rail_rx_channel_hopping_options_t options;
  /**
   * The RSSI threshold (in dBm) below which a hop will occur in
   * any mode when \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_RSSI_THRESHOLD is
   * specified.
   */
  int8_t rssi_threshold_dbm;
  /**
   * Pad bytes reserved for future use and currently ignored.
   */
  uint8_t reserved_1[1];
} sl_rail_rx_channel_hopping_config_entry_t;

/**
 * @struct sl_rail_rx_channel_hopping_config_t
 * @brief Wrapper struct that will contain the sequence of
 *   \ref sl_rail_rx_channel_hopping_config_entry_t that represents the channel
 *   sequence to use during RX Channel Hopping.
 */
typedef struct sl_rail_rx_channel_hopping_config {
  /**
   * Pointer to contiguous global read-write memory that will be used
   * by RAIL to store channel hopping information throughout its operation.
   * It need not be initialized and applications should never write
   * data anywhere in this buffer.
   *
   * @note The size of this buffer must be at least as large as
   *   3 + \ref SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL * number_of_channels,
   *   plus the sum of the sizes of the
   *   radio_config_delta_add's of the required channels, plus the size of the
   *   radio_config_delta_subtract. In the case that one channel
   *   appears two or more times in your channel sequence
   *   (e.g., 1, 2, 3, 2), you must account for the radio configuration
   *   size that number of times (i.e., need to count channel 2's
   *   radio configuration size twice for the given example). The buffer is
   *   for internal use to the library.
   */
  uint32_t *p_buffer;
  /**
   * This parameter must be set to the length of the buffer array, in 32 bit
   * words. This way, during configuration, the software can confirm it's
   * writing within the bounds of the buffer. The configuration API will return
   * an error or trigger \ref SL_RAIL_ASSERT_CHANNEL_HOPPING_BUFFER_TOO_SHORT if
   * buffer_words is insufficient.
   */
  uint16_t buffer_words;
  /**
   * The number of channels in the channel hopping sequence, which is the
   * number of elements in the array that entries points to.
   */
  uint8_t number_of_channels;
  /**
   * A pointer to the first element of an array of \ref
   * sl_rail_rx_channel_hopping_config_entry_t that represents the channels
   * used during channel hopping. This array must have number_of_channels
   * entries.
   */
  sl_rail_rx_channel_hopping_config_entry_t *p_entries;
} sl_rail_rx_channel_hopping_config_t;

/**
 * @struct sl_rail_rx_duty_cycle_config_t
 * @brief Structure to configure duty cycled receive mode.
 */
typedef struct sl_rail_rx_duty_cycle_config {
  /**
   * The mode by which RAIL determines when to exit RX.
   */
  sl_rail_rx_channel_hopping_mode_t mode;
  // Unnamed 'uint8_t reserved[3]' pad byte field here.
  /**
   * Depending on the 'mode' parameter that was specified, this member
   * is used to parameterize that mode. See the comments on each value of
   * \ref sl_rail_rx_channel_hopping_mode_t to learn what to specify here.
   */
  sl_rail_rx_channel_hopping_parameter_t parameter;
  /**
   * Idle time in microseconds to wait before re-entering RX.
   */
  uint32_t delay_us;
  /**
   * Reserved for future use.
   */
  uint8_t reserved_0;
  /**
   * Bitmask of various options that can be applied to the current
   * duty cycle operation when the mode is >= \ref
   * SL_RAIL_RX_CHANNEL_HOPPING_MODE_MANUAL_WITH_OPTIONS (ignored otherwise).
   */
  sl_rail_rx_channel_hopping_options_t options;
  /**
   * The RSSI threshold (in dBm) below which Rx will end in
   * any mode when \ref SL_RAIL_RX_CHANNEL_HOPPING_OPTION_RSSI_THRESHOLD
   * is specified.
   */
  int8_t rssi_threshold_dbm;
  /**
   * Pad bytes reserved for future use and currently ignored.
   */
  uint8_t reserved_1[1];
} sl_rail_rx_duty_cycle_config_t;

/**
 * A sentinel value to flag an invalid channel hopping index.
 */
#define SL_RAIL_CHANNEL_HOPPING_INVALID_INDEX (0xFEU)

/** @} */ // end of group Rx_Channel_Hopping

/******************************************************************************
 * Diagnostic Structures
 *****************************************************************************/
/**
 * @addtogroup Diagnostic
 * @{
 */

/**
 * @typedef sl_rail_frequency_offset_t
 * @brief Type that represents the number of Frequency Offset
 *   units. It is used with \ref sl_rail_get_rx_freq_offset() and
 *   \ref sl_rail_set_freq_offset().
 *
 * The units are chip-specific. For EFR32 they are radio synthesizer
 * resolution steps (synth ticks) and is limited to 15 bits.
 * A value of \ref SL_RAIL_FREQUENCY_OFFSET_INVALID
 * means that this value is invalid.
 */
typedef int16_t sl_rail_frequency_offset_t;

/**
 * The maximum frequency offset value supported.
 */
#define SL_RAIL_FREQUENCY_OFFSET_MAX ((sl_rail_frequency_offset_t) 0x3FFF)

/**
 * The minimum frequency offset value supported.
 */
#define SL_RAIL_FREQUENCY_OFFSET_MIN ((sl_rail_frequency_offset_t) -SL_RAIL_FREQUENCY_OFFSET_MAX)

/**
 * Specify an invalid frequency offset value. This will be returned if you
 * call \ref sl_rail_get_rx_freq_offset() at an invalid time.
 */
#define SL_RAIL_FREQUENCY_OFFSET_INVALID ((sl_rail_frequency_offset_t) 0x8000)

/**
 * @struct sl_rail_direct_mode_config_t
 * @brief Allows the user to specify direct mode
 *   parameters using \ref sl_rail_config_direct_mode().
 */
typedef struct sl_rail_direct_mode_config {
  /**
   * Enable synchronous RX DOUT using DCLK vs. asynchronous RX DOUT.
   */
  bool sync_rx;
  /**
   * Enable synchronous TX DIN using DCLK vs. asynchronous TX DIN.
   */
  bool sync_tx;

  /**
   * RX Data output (DOUT) GPIO port.
   */
  uint8_t dout_port;
  /**
   * RX Data output (DOUT) GPIO pin.
   */
  uint8_t dout_pin;

  /**
   * Data clock (DCLK) GPIO port. Only used in synchronous mode.
   */
  uint8_t dclk_port;
  /**
   * Data clock (DCLK) GPIO pin. Only used in synchronous mode.
   */
  uint8_t dclk_pin;

  /**
   * TX Data input (DIN) GPIO port.
   */
  uint8_t din_port;
  /**
   * TX Data input (DIN) GPIO pin.
   */
  uint8_t din_pin;

  /**
   * Reserved for future use.
   */
  uint8_t reserved_0;
  /**
   * Reserved for future use.
   */
  uint8_t reserved_1;
  /**
   * Reserved for future use.
   */
  uint8_t reserved_2;
} sl_rail_direct_mode_config_t;

/**
 * @enum sl_rail_stream_mode_t
 * @brief Possible stream output modes.
 */
SLI_RAIL_ENUM(sl_rail_stream_mode_t) {
  /**
   * An unmodulated carrier wave.
   */
  SL_RAIL_STREAM_CARRIER_WAVE = 0u,
  /**
   * PN9 byte sequence.
   */
  SL_RAIL_STREAM_PN9_STREAM = 1u,
  /**
   * 101010 sequence.
   */
  SL_RAIL_STREAM_10_STREAM = 2u,
  /**
   * An unmodulated carrier wave with no change to PLL BW. Same as \ref SL_RAIL_STREAM_CARRIER_WAVE.
   */
  SL_RAIL_STREAM_CARRIER_WAVE_PHASENOISE = 3u,
  /**
   * ramp sequence starting at a different offset for consecutive packets. Only available for some modulations. Fall back to \ref SL_RAIL_STREAM_PN9_STREAM if not available.
   */
  SL_RAIL_STREAM_RAMP_STREAM = 4u,
  /**
   * An unmodulated carrier wave not centered on DC but shifted roughly by channel_bandwidth/6 allowing an easy check of the residual DC. Only available for OFDM PA. Fall back to \ref SL_RAIL_STREAM_CARRIER_WAVE_PHASENOISE if not available.
   */
  SL_RAIL_STREAM_CARRIER_WAVE_SHIFTED = 5u,
  /**
   * 10001000 sequence.
   */
  SL_RAIL_STREAM_1000_STREAM = 6u,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_STREAM_MODES_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_STREAM_CARRIER_WAVE ((sl_rail_stream_mode_t) SL_RAIL_STREAM_CARRIER_WAVE)
#define SL_RAIL_STREAM_PN9_STREAM   ((sl_rail_stream_mode_t) SL_RAIL_STREAM_PN9_STREAM)
#define SL_RAIL_STREAM_10_STREAM    ((sl_rail_stream_mode_t) SL_RAIL_STREAM_10_STREAM)
#define SL_RAIL_STREAM_CARRIER_WAVE_PHASENOISE ((sl_rail_stream_mode_t) SL_RAIL_STREAM_CARRIER_WAVE_PHASENOISE)
#define SL_RAIL_STREAM_RAMP_STREAM  ((sl_rail_stream_mode_t) SL_RAIL_STREAM_RAMP_STREAM)
#define SL_RAIL_STREAM_CARRIER_WAVE_SHIFTED ((sl_rail_stream_mode_t) SL_RAIL_STREAM_CARRIER_WAVE_SHIFTED)
#define SL_RAIL_STREAM_MODES_COUNT  ((sl_rail_stream_mode_t) SL_RAIL_STREAM_MODES_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // end of group Diagnostic

/******************************************************************************
 * Front End Module Voltage Detection (VDET)
 *****************************************************************************/
/**
 * @addtogroup VDET
 * @{
 */

/**
 * @enum sl_rail_vdet_mode_t
 * @brief VDET Modes.
 *
 * The VDET Mode is passed to \ref sl_rail_config_vdet() via \ref sl_rail_vdet_config_t.
 * The \ref rail_util_vdet component allows customers to measure their Front End Module performance
 * at specified points in the Transmit packet.
 */
SLI_RAIL_ENUM(sl_rail_vdet_mode_t) {
  /**
   * VDET is completely disabled.
   */
  SL_RAIL_VDET_MODE_DISABLED = 0u,
  /**
   * AUTOMATIC causes VDET measurements to be taken every Tx packet at the specified time.
   */
  SL_RAIL_VDET_MODE_AUTOMATIC = 1u,
  /**
   * IMMEDIATE causes an immediate VDET measurement. VDET must not be in \ref SL_RAIL_VDET_MODE_AUTOMATIC.
   */
  SL_RAIL_VDET_MODE_IMMEDIATE = 2u,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_VDET_MODE_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_VDET_MODE_DISABLED  ((sl_rail_vdet_mode_t) SL_RAIL_VDET_MODE_DISABLED)
#define SL_RAIL_VDET_MODE_AUTOMATIC ((sl_rail_vdet_mode_t) SL_RAIL_VDET_MODE_AUTOMATIC)
#define SL_RAIL_VDET_MODE_IMMEDIATE ((sl_rail_vdet_mode_t) SL_RAIL_VDET_MODE_IMMEDIATE)
#define SL_RAIL_VDET_MODE_COUNT     ((sl_rail_vdet_mode_t) SL_RAIL_VDET_MODE_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @def SL_RAIL_VDET_MODE_ENUM_NAMES
 * @brief A macro that is string versions of the calibration enums.
 */
#define SL_RAIL_VDET_MODE_ENUM_NAMES { \
    "SL_RAIL_VDET_MODE_DISABLED",      \
    "SL_RAIL_VDET_MODE_AUTOMATIC",     \
    "SL_RAIL_VDET_MODE_IMMEDIATE",     \
}

/**
 * @enum sl_rail_vdet_resolution_t
 * @brief VDET Resolution for the Aux ADC.
 *
 * The VDET Resolution is passed to \ref sl_rail_config_vdet() via \ref sl_rail_vdet_config_t.
 * Shows available resolution options.
 */
SLI_RAIL_ENUM(sl_rail_vdet_resolution_t) {
  /**
   * ~10 bit resolution.
   */
  SL_RAIL_VDET_RESOLUTION_10_BIT = 0u,
  /**
   * ~11 bit resolution.
   */
  SL_RAIL_VDET_RESOLUTION_11_BIT = 1u,
  /**
   * ~12 bit resolution.
   */
  SL_RAIL_VDET_RESOLUTION_12_BIT = 2u,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_VDET_RESOLUTION_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_VDET_RESOLUTION_10_BIT ((sl_rail_vdet_resolution_t) SL_RAIL_VDET_RESOLUTION_10_BIT)
#define SL_RAIL_VDET_RESOLUTION_11_BIT ((sl_rail_vdet_resolution_t) SL_RAIL_VDET_RESOLUTION_11_BIT)
#define SL_RAIL_VDET_RESOLUTION_12_BIT ((sl_rail_vdet_resolution_t) SL_RAIL_VDET_RESOLUTION_12_BIT)
#define SL_RAIL_VDET_RESOLUTION_COUNT  ((sl_rail_vdet_resolution_t) SL_RAIL_VDET_RESOLUTION_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @def SL_RAIL_VDET_RESOLUTION_ENUM_NAMES
 * @brief A macro that is string versions of the resolution enums.
 */
#define SL_RAIL_VDET_RESOLUTION_ENUM_NAMES { \
    "SL_RAIL_VDET_RESOLUTION_10_BIT",        \
    "SL_RAIL_VDET_RESOLUTION_11_BIT",        \
    "SL_RAIL_VDET_RESOLUTION_12_BIT",        \
}

/**
 * @enum sl_rail_vdet_status_t
 * @brief VDET Status.
 *
 * The VDET Status for internal debug.
 * Shows states.
 */
// MUST BE KEPT IN ALIGNMENT WITH #DEFINES FOR VDET_STATUS IN RFLDMA YAML FILE!
SLI_RAIL_ENUM(sl_rail_vdet_status_t) {
  /**
   * IDLE - Waiting for next command/measurement.
   */
  SL_RAIL_VDET_STATUS_IDLE = 0u,
  /**
   * START of the VDET measurement activity.
   */
  SL_RAIL_VDET_STATUS_START = 1u,
  /**
   * Completion of a 10 bit measurement.
   */
  SL_RAIL_VDET_STATUS_10_BIT_DONE = 2u,
  /**
   * Completion of a 11 bit measurement.
   */
  SL_RAIL_VDET_STATUS_11_BIT_DONE = 3u,
  /**
   * Completion of a 12 bit measurement.
   */
  SL_RAIL_VDET_STATUS_12_BIT_DONE = 4u,
  /**
   * Conflict with another AuxADC user.
   */
  SL_RAIL_VDET_STATUS_BLOCKED = 5u,
  /**
   * An error has occurred.
   */
  SL_RAIL_VDET_STATUS_ERROR = 6u,
  /**
   * A count of the choices in this enumeration. Must be last.
   */
  SL_RAIL_VDET_STATUS_COUNT
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_VDET_STATUS_IDLE        ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_IDLE)
#define SL_RAIL_VDET_STATUS_START       ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_START)
#define SL_RAIL_VDET_STATUS_10_BIT_DONE ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_10_BIT_DONE)
#define SL_RAIL_VDET_STATUS_11_BIT_DONE ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_11_BIT_DONE)
#define SL_RAIL_VDET_STATUS_12_BIT_DONE ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_12_BIT_DONE)
#define SL_RAIL_VDET_STATUS_ERROR       ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_ERROR)
#define SL_RAIL_VDET_STATUS_BLOCKED     ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_BLOCKED)
#define SL_RAIL_VDET_STATUS_COUNT       ((sl_rail_vdet_status_t) SL_RAIL_VDET_STATUS_COUNT)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/**
 * @def SL_RAIL_VDET_STATUS_ENUM_NAMES
 * @brief A macro that is string versions of the status enums.
 */
#define SL_RAIL_VDET_STATUS_ENUM_NAMES { \
    "SL_RAIL_VDET_STATUS_IDLE",          \
    "SL_RAIL_VDET_STATUS_START",         \
    "SL_RAIL_VDET_STATUS_10_BIT_DONE",   \
    "SL_RAIL_VDET_STATUS_11_BIT_DONE",   \
    "SL_RAIL_VDET_STATUS_12_BIT_DONE",   \
    "SL_RAIL_VDET_STATUS_BLOCKED",       \
    "SL_RAIL_VDET_STATUS_ERROR",         \
}

/**
 * @struct sl_rail_vdet_config_t
 * @brief Configuration information for FEM Voltage Detection plugin.
 *
 * A structure of type \ref sl_rail_vdet_config_t is passed to \ref sl_rail_config_vdet().
 */
typedef struct sl_rail_vdet_config {
  /**
   * Mode for the VDET.
   */
  sl_rail_vdet_mode_t mode;
  /**
   * Resolution to use for the capture.
   */
  sl_rail_vdet_resolution_t resolution;
  /**
   * Delay in microseconds for the capture from Tx Start in \ref SL_RAIL_VDET_MODE_AUTOMATIC. Minimum 5 us, maximum 100000 us.
   */
  uint32_t delay_us;
} sl_rail_vdet_config_t;

/** @} */ // end of group VDET

/******************************************************************************
 * Thermal Protection
 *****************************************************************************/
/**
 * @addtogroup Thermal_Protection Thermal Protection
 * @{
 */

/**
 * Maximum junction temperature in Kelvin. A margin is subtracted before using
 * it when \ref SL_RAIL_SUPPORTS_THERMAL_PROTECTION is enabled.
 */
#define SL_RAIL_CHIP_TEMP_THRESHOLD_MAX      (398U)

/**
 * Default number of Kelvin degrees below threshold needed to allow transmissions.
 */
#define SL_RAIL_CHIP_TEMP_COOLDOWN_DEFAULT   (7U)

/**
 * @struct sl_rail_chip_temp_config_t
 * @brief Configuration parameters for thermal protection.
 */
typedef struct sl_rail_chip_temp_config {
  /**
   * Indicates whether the protection is enabled.
   */
  bool enable;
  /**
   * Mandatory temperature cool down when the threshold is exceeded, in degrees Kelvin.
   */
  uint8_t cool_down_kelvin;
  /**
   * Temperature above which transmit is blocked, in degrees Kelvin.
   */
  uint16_t threshold_kelvin;
} sl_rail_chip_temp_config_t;

/**
 * Number of temperature values provided for the chip thermal protection.
 */
#define SL_RAIL_CHIP_TEMP_MEASURE_COUNT      (3U)

/**
 * @struct sl_rail_chip_temp_metrics_t
 * @brief Data used for thermal protection.
 */
typedef struct sl_rail_chip_temp_metrics {
  /**
   * Store chip temperature for metrics.
   */
  uint16_t temp_kelvin;
  /**
   * Minimum temperature recorded.
   */
  uint16_t min_temp_kelvin;
  /**
   * Maximum temperature recorded.
   */
  uint16_t max_temp_kelvin;
  /**
   * Indicates if data should be reset.
   */
  bool reset_pending;
  /**
   * Reserved for future use.
   */
  uint8_t reserved;
} sl_rail_chip_temp_metrics_t;

/** @} */ // end of group Thermal_Protection

/******************************************************************************
 * Retiming Structures
 *****************************************************************************/
/**
 * @addtogroup Retiming
 * @{
 */

/**
 * @enum sl_rail_retime_options_t
 * @brief Retiming options bit shifts.
 */
SLI_RAIL_ENUM(sl_rail_retime_options_t) {
  /**
   * Shift position of \ref SL_RAIL_RETIME_OPTION_HFXO bit.
   */
  SL_RAIL_RETIME_OPTION_HFXO_SHIFT = 0,
  /**
   * Shift position of \ref SL_RAIL_RETIME_OPTION_HFRCO bit.
   */
  SL_RAIL_RETIME_OPTION_HFRCO_SHIFT = 1,
  /**
   * Shift position of \ref SL_RAIL_RETIME_OPTION_DCDC bit.
   */
  SL_RAIL_RETIME_OPTION_DCDC_SHIFT = 2,
  /**
   * Shift position of \ref SL_RAIL_RETIME_OPTION_LCD bit.
   */
  SL_RAIL_RETIME_OPTION_LCD_SHIFT = 3,
};

/**
 * An option to configure HFXO retiming.
 */
#define SL_RAIL_RETIME_OPTION_HFXO \
  (1U << SL_RAIL_RETIME_OPTION_HFXO_SHIFT)

/**
 * An option to configure HFRCO retiming.
 */
#define SL_RAIL_RETIME_OPTION_HFRCO \
  (1U << SL_RAIL_RETIME_OPTION_HFRCO_SHIFT)

/**
 * An option to configure DCDC retiming.
 * Ignored on platforms that lack DCDC.
 */
#define SL_RAIL_RETIME_OPTION_DCDC \
  (1U << SL_RAIL_RETIME_OPTION_DCDC_SHIFT)

/**
 * An option to configure LCD retiming.
 * Ignored on platforms that lack LCD.
 */
#define SL_RAIL_RETIME_OPTION_LCD \
  (1U << SL_RAIL_RETIME_OPTION_LCD_SHIFT)

/**
 * A value representing no retiming options.
 */
#define SL_RAIL_RETIME_OPTIONS_NONE 0x0U

/**
 * A value representing all retiming options.
 */
#define SL_RAIL_RETIME_OPTIONS_ALL 0xFFU

/** @} */ // end of group Retiming

#ifndef DOXYGEN_UNDOCUMENTED

/******************************************************************************
 * Debug Structures
 *****************************************************************************/
/**
 * @addtogroup Debug
 * @{
 */

/**
 * @def SL_RAIL_DEBUG_MODE_FREQ_OVERRIDE
 * @brief A bitmask to enable the frequency override debug mode to
 *   manually tune to a specified frequency. Note that this should only be used
 *   for testing and is not as tuned as frequencies from the calculator.
 */
#define SL_RAIL_DEBUG_MODE_FREQ_OVERRIDE  0x00000001UL

/**
 * @def SL_RAIL_DEBUG_MODE_VALID_MASK
 * @brief Any debug mode bits outside of this mask are invalid and ignored.
 */
#define SL_RAIL_DEBUG_MODE_VALID_MASK     (~(SL_RAIL_DEBUG_MODE_FREQ_OVERRIDE))

/** @} */ // end of group Debug

/******************************************************************************
 * Detailed Timing Structures
 *****************************************************************************/
/**
 * @addtogroup Detailed_Timing Detailed Timing
 * @{
 */

/**
 * @enum sl_rail_timer_tick_type_t
 * @brief Enumerate the timer tick channel.
 */
SLI_RAIL_ENUM(sl_rail_timer_tick_type_t) {
  /**
   * RAIL default timer tick channel.
   * This is used to query the current RAIL timer tick value.
   */
  SL_RAIL_TIMER_TICK_DEFAULT = 0u,
  /**
   * Radio state timer tick channel.
   * This is used to query the timer tick corresponding to the latest radio
   * state.
   */
  SL_RAIL_TIMER_TICK_RADIO_STATE = 1u,
  /**
   * RX time stamp timer tick channel.
   * This is used to query the timer tick at the time of latest RX frame
   * detection.
   */
  SL_RAIL_TIMER_TICK_RXSTAMP = 2u,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_TIMER_TICK_DEFAULT     ((sl_rail_timer_tick_type_t) SL_RAIL_TIMER_TICK_DEFAULT)
#define SL_RAIL_TIMER_TICK_RADIO_STATE ((sl_rail_timer_tick_type_t) SL_RAIL_TIMER_TICK_RADIO_STATE)
#define SL_RAIL_TIMER_TICK_RXSTAMP     ((sl_rail_timer_tick_type_t) SL_RAIL_TIMER_TICK_RXSTAMP)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/** @} */ // end of group Detailed Timing

#endif //DOXYGEN_UNDOCUMENTED

/******************************************************************************
 * TrustZone
 *****************************************************************************/
/**
 * @addtogroup TrustZone
 * @{
 */

/**
 * @typedef sl_rail_tz_changed_dcdc_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 *   \ref sl_rail_changed_dcdc().
 *
 * @return Status code indicating success of the function call.
 */
typedef sl_rail_status_t (*sl_rail_tz_changed_dcdc_callback_t)(void);

/**
 * @typedef sl_rail_tz_config_antenna_gpio_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_config_antenna_gpio().
 *
 * @param[in] p_config A pointer to a configuration structure applied to the relevant Antenna
 *   Configuration registers. A NULL configuration will produce undefined behavior.
 * @return Status code indicating success of the function call.
 *
 */
typedef sl_rail_status_t (*sl_rail_tz_config_antenna_gpio_callback_t)(const sl_rail_antenna_config_t *p_config);

/**
 * @typedef sl_rail_tz_radio_clock_enable_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_radio_clock_enable().
 *
 */
typedef void (*sl_rail_tz_radio_clock_enable_callback_t)(void);

/**
 * @typedef sl_rail_tz_get_radio_clock_freq_hz_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_get_radio_clock_freq_hz().
 *
 * @return Radio subsystem clock frequency in Hz.
 *
 */
typedef uint32_t (*sl_rail_tz_get_radio_clock_freq_hz_callback_t)(void);

/**
 * @typedef sl_rail_tz_rfeca_clock_enable_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_rfeca_clock_enable().
 *
 */
typedef void (*sl_rail_tz_rfeca_clock_enable_callback_t)(void);

/**
 * @typedef sl_rail_tz_rfeca_is_clock_enabled_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_rfeca_is_clock_enabled().
 *
 * @return true if RFECA clocks are enabled; false otherwise
 */
typedef bool (*sl_rail_tz_rfeca_is_clock_enabled_callback_t)(void);

/**
 * @typedef sl_rail_tz_read_internal_temperature_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_read_internal_temperature().
 *
 * @param[out] p_internal_temperature_kelvin A pointer to the internal
 *   temperature in Kelvin.
 * @param[in] enable_temperature_interrupts Indicate whether temperature
 * interrupts are enabled.
 * @return Status code indicating success of the function call.
 */
typedef sl_rail_status_t (*sl_rail_tz_read_internal_temperature_callback_t)(uint16_t *p_internal_temperature_kelvin,
                                                                            bool enable_temperature_interrupts);

/**
 * @typedef sl_rail_tz_enable_secure_radio_irqs_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_enable_secure_radio_irqs().
 */
typedef void (*sl_rail_tz_enable_secure_radio_irqs_callback_t)(void);

/**
 * @typedef sl_rail_tz_disable_secure_radio_irqs_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_disable_secure_radio_irqs().
 */
typedef void (*sl_rail_tz_disable_secure_radio_irqs_callback_t)(void);

/**
 * @typedef sl_rail_tz_radio_perform_m2m_ldma_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_radio_perform_m2m_ldma().
 *
 * @param[in] p_dest A pointer to the destination data.
 * @param[in] p_src A pointer to the source data.
 * @param[in] num_words Number of words to transfer.
 * @return Status code indicating success of the function call.
 */
typedef sl_rail_status_t (*sl_rail_tz_radio_perform_m2m_ldma_callback_t)(uint32_t *p_dest,
                                                                         const uint32_t *p_src,
                                                                         uint32_t num_words);

/**
 * @typedef sl_rail_tz_configure_hfxo_callback_t
 * @brief A pointer to the callback used to switch to secure world and run
 * \ref sl_rail_tz_configure_hfxo().
 */
typedef sl_rail_status_t (*sl_rail_tz_configure_hfxo_callback_t)(void);
/**
 * @struct sl_rail_tz_config_t
 * @brief Gather RAIL TrustZone callbacks pointers and booleans indicating
 *   peripheral secure configuration.
 */
typedef struct sl_rail_tz_config {
  /**
   * See \ref sl_rail_tz_changed_dcdc_callback_t().
   * In non-secure world, it must be NULL if CMU is a non-secure peripheral.
   */
  sl_rail_tz_changed_dcdc_callback_t changed_dcdc_callback;
  /**
   * See \ref sl_rail_tz_config_antenna_gpio_callback_t().
   * In non-secure world, it must be NULL if CMU and GPIO are non-secure
   * peripherals.
   */
  sl_rail_tz_config_antenna_gpio_callback_t config_antenna_gpio_callback;
  /**
   * See \ref sl_rail_tz_radio_clock_enable_callback_t().
   * In non-secure world, it must be NULL if CMU is a non-secure peripheral.
   */
  sl_rail_tz_radio_clock_enable_callback_t radio_clock_enable_callback;
  /**
   * See \ref sl_rail_tz_get_radio_clock_freq_hz_callback_t().
   * In non-secure world, it must be NULL if CMU is a non-secure peripheral.
   */
  sl_rail_tz_get_radio_clock_freq_hz_callback_t get_radio_clock_freq_hz_callback;
  /**
   * See \ref sl_rail_tz_rfeca_clock_enable_callback_t().
   * In non-secure world, it must be NULL if CMU is a non-secure peripheral.
   */
  sl_rail_tz_rfeca_clock_enable_callback_t rfeca_clock_enable_callback;
  /**
   * See \ref sl_rail_tz_rfeca_is_clock_enabled_callback_t().
   * In non-secure world, it must be NULL if CMU is a non-secure peripheral.
   */
  sl_rail_tz_rfeca_is_clock_enabled_callback_t rfeca_is_clock_enabled_callback;
  /**
   * See \ref sl_rail_tz_read_internal_temperature_callback_t().
   * In non-secure world, it must be NULL if EMU is a non-secure peripheral.
   */
  sl_rail_tz_read_internal_temperature_callback_t read_internal_temperature_callback;
  /**
   * See \ref sl_rail_tz_enable_secure_radio_irqs_callback_t().
   * In non-secure world, it must be NULL if EMU is a non-secure peripheral.
   */
  sl_rail_tz_enable_secure_radio_irqs_callback_t enable_secure_radio_irqs_callback;
  /**
   * See \ref sl_rail_tz_disable_secure_radio_irqs_callback_t().
   * In non-secure world, it must be NULL if EMU is a non-secure peripheral.
   */
  sl_rail_tz_disable_secure_radio_irqs_callback_t disable_secure_radio_irqs_callback;
  /**
   * See \ref sl_rail_tz_radio_perform_m2m_ldma_callback_t().
   * In non-secure world, it must be NULL if LDMA is a non-secure peripheral or
   * if RAIL must not use LDMA.
   */
  sl_rail_tz_radio_perform_m2m_ldma_callback_t radio_perform_m2m_ldma_callback;
  /**
   * See \ref sl_rail_tz_configure_hfxo_callback_t().
   * In non-secure world, it must be NULL if HFXO is a non-secure peripheral.
   */
  sl_rail_tz_configure_hfxo_callback_t configure_hfxo_callback;
  /**
   * Indicate whether CMU is configured as secure peripheral.
   */
  bool is_cmu_secure;
  /**
   * Indicate whether EMU is configured as secure peripheral.
   */
  bool is_emu_secure;
  /**
   * Indicate whether GPIO is configured as secure peripheral.
   */
  bool is_gpio_secure;
  /**
   * Indicate whether LDMA is configured as secure peripheral.
   */
  bool is_ldma_secure;
  /**
   * Indicate whether HFXO is configured as secure peripheral.
   */
  bool is_hfxo_secure;
  /**
   * Indicate whether PRS is configured as secure peripheral.
   */
  bool is_prs_secure;
  /**
   * Indicate whether SYSRTC is configured as secure peripheral.
   */
  bool is_sysrtc_secure;
} sl_rail_tz_config_t;

/** @} */ // end of group TrustZone

#ifdef  SLI_LIBRARY_BUILD
#ifndef DOXYGEN_SHOULD_SKIP_THIS
// SLI_LIBRARY_BUILD is defined for library builds that do not include
// chip-dependent defines. This may limit functionality but allows building
// generic libraries that are not tied to any given chip.

// Platform-agnostic worst-case settings and types

#define SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL \
  SL_RAIL_CHANNEL_HOPPING_BUFFER_WORDS_PER_CHANNEL_WORST_CASE

struct sl_rail_channel_config_entry_attr {
  sl_rail_rx_ir_cal_values_t rx_ir_cal_values;
  sl_rail_tx_ir_cal_values_t tx_ir_cal_values;
};

#endif//DOXYGEN_SHOULD_SKIP_THIS
#endif//SLI_LIBRARY_BUILD

/** @} */ // end of RAIL_API

#ifdef __cplusplus
}
#endif

#ifndef SLI_LIBRARY_BUILD
// Include appropriate chip-specific types and APIs *after* common types, and
// *before* types that depend on chip-specific types.
#include "sl_rail_chip_specific.h"

// (Currently no types depend on chip-specific types.)

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Handy macro for plugin code built weakly into librail
#ifndef SLI_LIBRAIL_WEAK
#ifdef  SLI_LIBRAIL_BUILD
#define SLI_LIBRAIL_WEAK __WEAK
#else//!SLI_LIBRAIL_BUILD
#define SLI_LIBRAIL_WEAK //Strong
#endif//SLI_LIBRAIL_BUILD
#endif//SLI_LIBRAIL_WEAK
#endif//DOXYGEN_SHOULD_SKIP_THIS

#endif//!SLI_LIBRARY_BUILD

#endif  // SL_RAIL_TYPES_H
