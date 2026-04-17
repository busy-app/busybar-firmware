/***************************************************************************//**
 * @file
 * @brief Definition of error codes that occur in RAIL.
 *   This file is purely informational and optional -
 *   it need not be included even if rail_assert libraries are included.
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

#ifndef SL_RAIL_ASSERT_ERROR_CODES_H
#define SL_RAIL_ASSERT_ERROR_CODES_H

#include "sl_rail_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup Assertions
 * @{
 */

/**
 * Enumeration of all possible error codes from SL_RAIL_ASSERT.
 */
SLI_RAIL_ENUM_GENERIC(sl_rail_assert_error_codes_t, uint32_t)
{
  /** Appended info missing from RX packet. */
  SL_RAIL_ASSERT_FAILED_APPENDED_INFO_MISSING = 0,
  /** Receive FIFO too small for IR calibration. */
  SL_RAIL_ASSERT_FAILED_RX_FIFO_BYTES = 1,
  /** Unexpected register reset value. */
  SL_RAIL_ASSERT_UNEXPECTED_REG_RESET_VALUE = 2,
  /** Receive FIFO entry has invalid status. */
  SL_RAIL_ASSERT_FAILED_ILLEGAL_RXLEN_ENTRY_STATUS = 3,
  /** Receive FIFO entry bad packet length. */
  SL_RAIL_ASSERT_FAILED_BAD_PACKET_LENGTH = 4,
  /** Unable to configure radio for IR calibration. */
  SL_RAIL_ASSERT_FAILED_SYNTH_DIVCTRL_ENUM_CONVERSION_ERROR = 5,
  /** Reached unexpected state while handling RX FIFO events. */
  SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_RX_FIFO = 6,
  /** Reached unexpected state while handling RXLEN FIFO events. */
  SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_RXLEN_FIFO = 7,
  /** Reached unexpected state while handling TX FIFO events. */
  SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_TX_FIFO = 8,
  /** Reached unexpected state while handling TX ACK FIFO events. */
  SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_TXACK_FIFO = 9,
  /** Invalid memory region accessed. */
  SL_RAIL_ASSERT_INVALID_MEMORY_ACCESS = 10,
  /** Unsupported RX Data Source. */
  SL_RAIL_ASSERT_FAILED_UNSUPPORTED_RX_DATA_SOURCE = 11,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_12 = 12,
  /** Error synchronizing the RAIL timebase after sleep. */
  SL_RAIL_ASSERT_FAILED_RTCC_POST_WAKEUP = 13,
  /** VCO frequency outside supported range. */
  SL_RAIL_ASSERT_FAILED_SYNTH_VCO_FREQUENCY = 14,
  /** Radio active while changing channels. */
  SL_RAIL_ASSERT_FAILED_RAC_STATE = 15,
  /** Invalid Synth VCOCTRL field calculation. */
  SL_RAIL_ASSERT_FAILED_SYNTH_INVALID_VCOCTRL = 16,
  /** Nested attempt to lock the sequencer. */
  SL_RAIL_ASSERT_FAILED_NESTED_SEQUENCER_LOCK = 17,
  /** RSSI averaging enabled without a valid callback. */
  SL_RAIL_ASSERT_FAILED_RSSI_AVERAGE_DONE = 18,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_19 = 19,
  /** Unable to seed radio pseudo random number generator. */
  SL_RAIL_ASSERT_FAILED_PROTIMER_RANDOM_SEED = 20,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_21 = 21,
  /** Invalid timer channel specified. */
  SL_RAIL_ASSERT_FAILED_PROTIMER_CHANNEL = 22,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_23 = 23,
  /** LBT config exceeds register size. */
  SL_RAIL_ASSERT_FAILED_BASECNTTOP = 24,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_25 = 25,
  /** Could not synchronize RAIL timebase with the RTC. */
  SL_RAIL_ASSERT_FAILED_RTCC_SYNC_MISSED = 26,
  /** Clock source not ready. */
  SL_RAIL_ASSERT_FAILED_CLOCK_SOURCE_NOT_READY = 27,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_28 = 28,
  /** NULL was supplied as a sl_rail_handle_t argument. */
  SL_RAIL_ASSERT_NULL_HANDLE = 29,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_30 = 30,
  /** API improperly called while protocol inactive. */
  SL_RAIL_ASSERT_FAILED_NO_ACTIVE_CONFIG = 31,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_32 = 32,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_33 = 33,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_34 = 34,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_35 = 35,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_36 = 36,
  /** Sequencer protocol stack running too long. */
  SL_RAIL_ASSERT_SEQ_USER_SEQUENCER_PROTOCOL_TAKE_TOO_LONG = 37,
  /** Failed to enable synth for transmit. */
  SL_RAIL_ASSERT_FAILED_TX_SYNTH_ENABLE = 38,
  /** This function is deprecated and must not be called. */
  SL_RAIL_ASSERT_DEPRECATED_FUNCTION = 39,
  /** Multiprotocol task started with no event to run. */
  SL_RAIL_ASSERT_MULTIPROTOCOL_NO_EVENT = 40,
  /** Invalid interrupt enabled. */
  SL_RAIL_ASSERT_FAILED_INVALID_INTERRUPT_ENABLED = 41,
  /** Invalid assert, no longer used. */
  SL_RAIL_ASSERT_UNUSED_42 = 42,
  /** Division by zero. */
  SL_RAIL_ASSERT_DIVISION_BY_ZERO = 43,
  /** Function cannot be called without access to the hardware. */
  SL_RAIL_ASSERT_CANT_USE_HARDWARE = 44,
  /** Pointer parameter was passed as NULL. */
  SL_RAIL_ASSERT_NULL_PARAMETER = 45,
  /** Secure Element fault */
  SL_RAIL_ASSERT_SECURE_ELEMENT_FAULT = 46,
  /** Synth radio config buffer for channel hopping too small. */
  SL_RAIL_ASSERT_SMALL_SYNTH_RADIO_CONFIG_BUFFER = 47,
  /** Buffer provided for RX Channel Hopping is too small. */
  SL_RAIL_ASSERT_CHANNEL_HOPPING_BUFFER_TOO_SHORT = 48,
  /** Invalid action was attempted on a module. */
  SL_RAIL_ASSERT_INVALID_MODULE_ACTION = 49,
  /** The radio config for this channel is not compatible with channel hopping. */
  SL_RAIL_ASSERT_CHANNEL_HOPPING_INVALID_RADIO_CONFIG = 50,
  /** Channel change failed. */
  SL_RAIL_ASSERT_CHANNEL_CHANGE_FAILED = 51,
  /** Attempted to read invalid register. */
  SL_RAIL_ASSERT_INVALID_REGISTER = 52,
  /** CP/DMA Invalid error. */
  SL_RAIL_ASSERT_CP_DMA_INTERNAL_GENERIC_ERROR = 53,
  /** DMP radio config caching failed. */
  SL_RAIL_ASSERT_CACHE_CONFIG_FAILED = 54,
  /** NULL was supplied as a sl_rail_state_transitions_t argument. */
  SL_RAIL_ASSERT_NULL_TRANSITIONS = 55,
  /** LDMA transfer failed. */
  SL_RAIL_ASSERT_BAD_LDMA_TRANSFER = 56,
  /** Attempted to wake up with invalid RTCC sync data. */
  SL_RAIL_ASSERT_INVALID_RTCC_SYNC_VALUES = 57,
  /** Radio sequencer hit a fault condition. */
  SL_RAIL_ASSERT_SEQUENCER_FAULT = 58,
  /** Bus fault. */
  SL_RAIL_ASSERT_BUS_ERROR = 59,
  /** The current radio config cannot be used with packet filtering. */
  SL_RAIL_ASSERT_INVALID_FILTERING_CONFIG = 60,
  /** Retiming configuration error. */
  SL_RAIL_ASSERT_RETIMING_CONFIG = 61,
  /** TX CRC configuration is corrupt. */
  SL_RAIL_ASSERT_FAILED_TX_CRC_CONFIG = 62,
  /** The current PA config does not allow for this operation. */
  SL_RAIL_ASSERT_INVALID_PA_OPERATION = 63,
  /** The sequencer selected an invalid PA. */
  SL_RAIL_ASSERT_SEQ_INVALID_PA_SELECTED = 64,
  /** Invalid/unsupported channel config. */
  SL_RAIL_ASSERT_FAILED_INVALID_CHANNEL_CONFIG = 65,
  /** Radio Calculator configuration HFXO frequency mismatch with chip */
  SL_RAIL_ASSERT_INVALID_XTAL_FREQUENCY = 66,
  /** Internal error. */
  SL_RAIL_ASSERT_INTERNAL_GENERIC_ERROR = 67,
  /** Software modem image does not support requested modulation  */
  SL_RAIL_ASSERT_UNSUPPORTED_SOFTWARE_MODEM_MODULATION = 68,
  /** Failed to disable RTCC synchronization. */
  SL_RAIL_ASSERT_FAILED_RTCC_SYNC_STOP = 69,
  /** Multitimer linked list corrupted. */
  SL_RAIL_ASSERT_FAILED_MULTITIMER_CORRUPT = 70,
  /** Unable to configure radio for temperature calibration. */
  SL_RAIL_ASSERT_FAILED_TEMPCAL_ERROR = 71,
  /** Invalid VDET configuration. */
  SL_RAIL_ASSERT_INVALID_VDET_CONFIGURATION = 72,
  /** Invalid RFFPLL configuration. */
  SL_RAIL_ASSERT_INVALID_RFFPLL_CONFIGURATION = 73,
  /** Secure access fault. */
  SL_RAIL_ASSERT_SECURE_ACCESS_FAULT = 74,
  /** SYSRTC0 not running. */
  SL_RAIL_ASSERT_FAILED_SYSRTC0_NOT_RUNNING = 75,
  /** Radio Configurator not updated. */
  SL_RAIL_ASSERT_RADIO_CONFIG_NOT_UP_TO_DATE = 76,
  /** Failed to set the event for configurable RSSI threshold. */
  SL_RAIL_ASSERT_FAILED_RSSI_THRESHOLD = 77,
  /** Intended and actual Z-Wave region configuration mismatch. */
  SL_RAIL_ASSERT_INCORRECT_ZWAVE_REGION = 78,
  /** Attempted to sleep with stale RTCC synchronization data */
  SL_RAIL_ASSERT_FAILED_RTCC_SYNC_STALE_DATA = 79,
  /** Attempted to clear LOG2X4 with a DEC1 value not equal to 0 */
  SL_RAIL_ASSERT_INVALID_LOG2X4_CLEAR_CONDITION = 80,
  /** Failed to complete DMA write */
  SL_RAIL_ASSERT_FAILED_DMA_WRITE_INCOMPLETE = 81,
  /** RAIL does not support this Radio Calculator configuration */
  SL_RAIL_ASSERT_CALCULATOR_NOT_SUPPORTED = 82,
  /** Invalid binary image was loaded onto the sequencer */
  SL_RAIL_ASSERT_INVALID_SEQUENCER_IMAGE = 83,
  /** No common or protocol image selected to be loaded onto the sequencer */
  SL_RAIL_ASSERT_MISSING_SEQUENCER_IMAGE = 84,
  /** Software modem image invalid or missing */
  SL_RAIL_ASSERT_INVALID_OR_MISSING_SOFTWARE_MODEM_IMAGE = 85,
  /** The sequencer user generated error. */
  SL_RAIL_ASSERT_SEQ_USER_SEQUENCER_GENERIC_ERROR = 86,
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Self-referencing defines minimize compiler complaints when using SLI_RAIL_ENUM
#define SL_RAIL_ASSERT_FAILED_APPENDED_INFO_MISSING               ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_APPENDED_INFO_MISSING)
#define SL_RAIL_ASSERT_FAILED_RX_FIFO_BYTES                       ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RX_FIFO_BYTES)
#define SL_RAIL_ASSERT_UNEXPECTED_REG_RESET_VALUE                 ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNEXPECTED_REG_RESET_VALUE)
#define SL_RAIL_ASSERT_FAILED_ILLEGAL_RXLEN_ENTRY_STATUS          ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_ILLEGAL_RXLEN_ENTRY_STATUS)
#define SL_RAIL_ASSERT_FAILED_BAD_PACKET_LENGTH                   ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_BAD_PACKET_LENGTH)
#define SL_RAIL_ASSERT_FAILED_SYNTH_DIVCTRL_ENUM_CONVERSION_ERROR ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_SYNTH_DIVCTRL_ENUM_CONVERSION_ERROR)
#define SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_RX_FIFO            ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_RX_FIFO)
#define SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_RXLEN_FIFO         ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_RXLEN_FIFO)
#define SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_TX_FIFO            ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_TX_FIFO)
#define SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_TXACK_FIFO         ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_UNEXPECTED_STATE_TXACK_FIFO)
#define SL_RAIL_ASSERT_INVALID_MEMORY_ACCESS                      ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_MEMORY_ACCESS)
#define SL_RAIL_ASSERT_FAILED_UNSUPPORTED_RX_DATA_SOURCE          ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_UNSUPPORTED_RX_DATA_SOURCE)
#define SL_RAIL_ASSERT_UNUSED_12                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_12)
#define SL_RAIL_ASSERT_FAILED_RTCC_POST_WAKEUP                    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RTCC_POST_WAKEUP)
#define SL_RAIL_ASSERT_FAILED_SYNTH_VCO_FREQUENCY                 ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_SYNTH_VCO_FREQUENCY)
#define SL_RAIL_ASSERT_FAILED_RAC_STATE                           ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RAC_STATE)
#define SL_RAIL_ASSERT_FAILED_SYNTH_INVALID_VCOCTRL               ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_SYNTH_INVALID_VCOCTRL)
#define SL_RAIL_ASSERT_FAILED_NESTED_SEQUENCER_LOCK               ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_NESTED_SEQUENCER_LOCK)
#define SL_RAIL_ASSERT_FAILED_RSSI_AVERAGE_DONE                   ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RSSI_AVERAGE_DONE)
#define SL_RAIL_ASSERT_UNUSED_19                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_19)
#define SL_RAIL_ASSERT_FAILED_PROTIMER_RANDOM_SEED                ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_PROTIMER_RANDOM_SEED)
#define SL_RAIL_ASSERT_UNUSED_21                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_21)
#define SL_RAIL_ASSERT_FAILED_PROTIMER_CHANNEL                    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_PROTIMER_CHANNEL)
#define SL_RAIL_ASSERT_UNUSED_23                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_23)
#define SL_RAIL_ASSERT_FAILED_BASECNTTOP                          ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_BASECNTTOP)
#define SL_RAIL_ASSERT_UNUSED_25                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_25)
#define SL_RAIL_ASSERT_FAILED_RTCC_SYNC_MISSED                    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RTCC_SYNC_MISSED)
#define SL_RAIL_ASSERT_FAILED_CLOCK_SOURCE_NOT_READY              ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_CLOCK_SOURCE_NOT_READY)
#define SL_RAIL_ASSERT_UNUSED_28                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_28)
#define SL_RAIL_ASSERT_NULL_HANDLE                                ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_NULL_HANDLE)
#define SL_RAIL_ASSERT_UNUSED_30                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_30)
#define SL_RAIL_ASSERT_FAILED_NO_ACTIVE_CONFIG                    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_NO_ACTIVE_CONFIG)
#define SL_RAIL_ASSERT_UNUSED_32                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_32)
#define SL_RAIL_ASSERT_UNUSED_33                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_33)
#define SL_RAIL_ASSERT_UNUSED_34                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_34)
#define SL_RAIL_ASSERT_UNUSED_35                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_35)
#define SL_RAIL_ASSERT_UNUSED_36                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_36)
#define SL_RAIL_ASSERT_SEQ_USER_SEQUENCER_PROTOCOL_TAKE_TOO_LONG  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_SEQ_USER_SEQUENCER_PROTOCOL_TAKE_TOO_LONG)
#define SL_RAIL_ASSERT_FAILED_TX_SYNTH_ENABLE                     ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_TX_SYNTH_ENABLE)
#define SL_RAIL_ASSERT_DEPRECATED_FUNCTION                        ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_DEPRECATED_FUNCTION)
#define SL_RAIL_ASSERT_MULTIPROTOCOL_NO_EVENT                     ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_MULTIPROTOCOL_NO_EVENT)
#define SL_RAIL_ASSERT_FAILED_INVALID_INTERRUPT_ENABLED           ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_INVALID_INTERRUPT_ENABLED)
#define SL_RAIL_ASSERT_UNUSED_42                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNUSED_42)
#define SL_RAIL_ASSERT_DIVISION_BY_ZERO                           ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_DIVISION_BY_ZERO)
#define SL_RAIL_ASSERT_CANT_USE_HARDWARE                          ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_CANT_USE_HARDWARE)
#define SL_RAIL_ASSERT_NULL_PARAMETER                             ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_NULL_PARAMETER)
#define SL_RAIL_ASSERT_SECURE_ELEMENT_FAULT                       ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_SECURE_ELEMENT_FAULT)
#define SL_RAIL_ASSERT_SMALL_SYNTH_RADIO_CONFIG_BUFFER            ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_SMALL_SYNTH_RADIO_CONFIG_BUFFER)
#define SL_RAIL_ASSERT_CHANNEL_HOPPING_BUFFER_TOO_SHORT           ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_CHANNEL_HOPPING_BUFFER_TOO_SHORT)
#define SL_RAIL_ASSERT_INVALID_MODULE_ACTION                      ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_MODULE_ACTION)
#define SL_RAIL_ASSERT_CHANNEL_HOPPING_INVALID_RADIO_CONFIG       ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_CHANNEL_HOPPING_INVALID_RADIO_CONFIG)
#define SL_RAIL_ASSERT_CHANNEL_CHANGE_FAILED                      ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_CHANNEL_CHANGE_FAILED)
#define SL_RAIL_ASSERT_INVALID_REGISTER                           ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_REGISTER)
#define SL_RAIL_ASSERT_CP_DMA_INTERNAL_GENERIC_ERROR              ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_CP_DMA_INTERNAL_GENERIC_ERROR)
#define SL_RAIL_ASSERT_CACHE_CONFIG_FAILED                        ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_CACHE_CONFIG_FAILED)
#define SL_RAIL_ASSERT_NULL_TRANSITIONS                           ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_NULL_TRANSITIONS)
#define SL_RAIL_ASSERT_BAD_LDMA_TRANSFER                          ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_BAD_LDMA_TRANSFER)
#define SL_RAIL_ASSERT_INVALID_RTCC_SYNC_VALUES                   ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_RTCC_SYNC_VALUES)
#define SL_RAIL_ASSERT_SEQUENCER_FAULT                            ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_SEQUENCER_FAULT)
#define SL_RAIL_ASSERT_BUS_ERROR                                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_BUS_ERROR)
#define SL_RAIL_ASSERT_INVALID_FILTERING_CONFIG                   ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_FILTERING_CONFIG)
#define SL_RAIL_ASSERT_RETIMING_CONFIG                            ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_RETIMING_CONFIG)
#define SL_RAIL_ASSERT_FAILED_TX_CRC_CONFIG                       ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_TX_CRC_CONFIG)
#define SL_RAIL_ASSERT_INVALID_PA_OPERATION                       ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_PA_OPERATION)
#define SL_RAIL_ASSERT_SEQ_INVALID_PA_SELECTED                    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_SEQ_INVALID_PA_SELECTED)
#define SL_RAIL_ASSERT_FAILED_INVALID_CHANNEL_CONFIG              ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_INVALID_CHANNEL_CONFIG)
#define SL_RAIL_ASSERT_INVALID_XTAL_FREQUENCY                     ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_XTAL_FREQUENCY)
#define SL_RAIL_ASSERT_INTERNAL_GENERIC_ERROR                     ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INTERNAL_GENERIC_ERROR)
#define SL_RAIL_ASSERT_UNSUPPORTED_SOFTWARE_MODEM_MODULATION      ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_UNSUPPORTED_SOFTWARE_MODEM_MODULATION)
#define SL_RAIL_ASSERT_FAILED_RTCC_SYNC_STOP                      ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RTCC_SYNC_STOP)
#define SL_RAIL_ASSERT_FAILED_MULTITIMER_CORRUPT                  ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_MULTITIMER_CORRUPT)
#define SL_RAIL_ASSERT_FAILED_TEMPCAL_ERROR                       ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_TEMPCAL_ERROR)
#define SL_RAIL_ASSERT_INVALID_VDET_CONFIGURATION                 ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_VDET_CONFIGURATION)
#define SL_RAIL_ASSERT_INVALID_RFFPLL_CONFIGURATION               ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_RFFPLL_CONFIGURATION)
#define SL_RAIL_ASSERT_SECURE_ACCESS_FAULT                        ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_SECURE_ACCESS_FAULT)
#define SL_RAIL_ASSERT_FAILED_SYSRTC0_NOT_RUNNING                 ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_SYSRTC0_NOT_RUNNING)
#define SL_RAIL_ASSERT_RADIO_CONFIG_NOT_UP_TO_DATE                ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_RADIO_CONFIG_NOT_UP_TO_DATE)
#define SL_RAIL_ASSERT_FAILED_RSSI_THRESHOLD                      ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RSSI_THRESHOLD)
#define SL_RAIL_ASSERT_INCORRECT_ZWAVE_REGION                     ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INCORRECT_ZWAVE_REGION)
#define SL_RAIL_ASSERT_FAILED_RTCC_SYNC_STALE_DATA                ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_RTCC_SYNC_STALE_DATA)
#define SL_RAIL_ASSERT_INVALID_LOG2X4_CLEAR_CONDITION             ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_LOG2X4_CLEAR_CONDITION)
#define SL_RAIL_ASSERT_FAILED_DMA_WRITE_INCOMPLETE                ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_FAILED_DMA_WRITE_INCOMPLETE)
#define SL_RAIL_ASSERT_CALCULATOR_NOT_SUPPORTED                   ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_CALCULATOR_NOT_SUPPORTED)
#define SL_RAIL_ASSERT_INVALID_SEQUENCER_IMAGE                    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_SEQUENCER_IMAGE)
#define SL_RAIL_ASSERT_MISSING_SEQUENCER_IMAGE                    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_MISSING_SEQUENCER_IMAGE)
#define SL_RAIL_ASSERT_INVALID_OR_MISSING_SOFTWARE_MODEM_IMAGE    ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_INVALID_OR_MISSING_SOFTWARE_MODEM_IMAGE)
#define SL_RAIL_ASSERT_SEQ_USER_SEQUENCER_GENERIC_ERROR           ((sl_rail_assert_error_codes_t) SL_RAIL_ASSERT_SEQ_USER_SEQUENCER_GENERIC_ERROR)
#endif//DOXYGEN_SHOULD_SKIP_THIS

/// Use this define to create an array of error messages that map to the codes
/// in \ref sl_rail_assert_error_codes_t. You can use these to print slightly more
/// detailed error strings related to a particular assert error code if desired.
/// For example, you could implement your assert failed callback as follows to
/// make use of this.
/// @code{.c}
/// void sl_railcb_assert_failed(sl_rail_handle_t rail_handle, sl_rail_assert_error_codes_t error_code)
/// {
///   static const char *rail_error_messages[] = SL_RAIL_ASSERT_ERROR_MESSAGES;
///   const char *error_message = "Unknown";
///
///   // If this error code is within the range of known error messages then use
///   // the appropriate error message.
///   if (error_code < (sizeof(rail_error_messages) / sizeof(char*))) {
///     error_message = rail_error_messages[error_code];
///   }
///   printf("RAIL ASSERT %u: %s\n", error_code, error_message);
///
///   // Reset the chip since an assert is a fatal error
///   sl_interrupt_manager_reset_system();
/// }
/// @endcode
///
#define SL_RAIL_ASSERT_ERROR_MESSAGES {                                                \
    /* 0*/ "Appended info missing from RX packet",                                     \
    /* 1*/ "Receive FIFO too small for IR calibration",                                \
    /* 2*/ "Unexpected register reset value",                                          \
    /* 3*/ "Receive FIFO entry has invalid status",                                    \
    /* 4*/ "Receive FIFO entry bad packet length",                                     \
    /* 5*/ "Unable to configure radio for IR calibration",                             \
    /* 6*/ "Reached unexpected state while handling RX FIFO events",                   \
    /* 7*/ "Reached unexpected state while handling RXLEN FIFO events",                \
    /* 8*/ "Reached unexpected state while handling TX FIFO events",                   \
    /* 9*/ "Reached unexpected state while handling TX ACK FIFO events",               \
    /*10*/ "Invalid memory region accessed",                                           \
    /*11*/ "Unsupported RX Data Source.",                                              \
    /*12*/ "Invalid assert, no longer used",                                           \
    /*13*/ "Error synchronizing the RAIL timebase after sleep",                        \
    /*14*/ "VCO frequency outside supported range",                                    \
    /*15*/ "Radio active while changing channels",                                     \
    /*16*/ "Invalid Synth VCOCTRL field calculation",                                  \
    /*17*/ "Nested attempt to lock the sequencer",                                     \
    /*18*/ "RSSI averaging enabled without a valid callback",                          \
    /*19*/ "Invalid assert, no longer used",                                           \
    /*20*/ "Unable to seed radio pseudo random number generator",                      \
    /*21*/ "Invalid assert, no longer used",                                           \
    /*22*/ "Invalid timer channel specified",                                          \
    /*23*/ "Invalid assert, no longer used",                                           \
    /*24*/ "LBT config exceeds register size",                                         \
    /*25*/ "Invalid assert, no longer used",                                           \
    /*26*/ "Could not synchronize RAIL timebase with the RTC",                         \
    /*27*/ "Clock source not ready",                                                   \
    /*28*/ "Invalid assert, no longer used",                                           \
    /*29*/ "NULL was supplied as a sl_rail_handle_t argument",                         \
    /*30*/ "Invalid assert, no longer used",                                           \
    /*31*/ "API improperly called while protocol inactive",                            \
    /*32*/ "Invalid assert, no longer used",                                           \
    /*33*/ "Invalid assert, no longer used",                                           \
    /*34*/ "Invalid assert, no longer used",                                           \
    /*35*/ "Invalid assert, no longer used",                                           \
    /*36*/ "Invalid assert, no longer used",                                           \
    /*37*/ "Sequencer protocol stack running too long",                                \
    /*38*/ "Failed to enable synth for transmit.",                                     \
    /*39*/ "This function is deprecated and must not be called",                       \
    /*40*/ "Multiprotocol task started with no event to run",                          \
    /*41*/ "Invalid interrupt enabled",                                                \
    /*42*/ "Invalid assert, no longer used",                                           \
    /*43*/ "Division by zero",                                                         \
    /*44*/ "Function cannot be called without access to the hardware",                 \
    /*45*/ "Pointer parameter was passed as NULL",                                     \
    /*46*/ "Secure Element fault",                                                     \
    /*47*/ "Synth radio config buffer for channel hopping too small",                  \
    /*48*/ "Buffer provided for RX Channel Hopping is too small",                      \
    /*49*/ "Invalid action was attempted on a module",                                 \
    /*50*/ "The radio config for this channel is not compatible with channel hopping", \
    /*51*/ "Channel change failed",                                                    \
    /*52*/ "Attempted to read invalid register",                                       \
    /*53*/ "CP/DMA Generic internal error",                                            \
    /*54*/ "DMP radio config caching failed",                                          \
    /*55*/ "NULL was supplied as a sl_rail_state_transitions_t argument",              \
    /*56*/ "LDMA transfer failed",                                                     \
    /*57*/ "Attempted to wake up with invalid RTCC sync data",                         \
    /*58*/ "Radio sequencer hit a fault condition",                                    \
    /*59*/ "Bus fault",                                                                \
    /*60*/ "The current radio config cannot be used with packet filtering",            \
    /*61*/ "Retiming configuration error",                                             \
    /*62*/ "TX CRC configuration is corrupt",                                          \
    /*63*/ "The current PA config does not allow for this operation",                  \
    /*64*/ "The sequencer selected an invalid PA",                                     \
    /*65*/ "Invalid/unsupported channel config",                                       \
    /*66*/ "Radio Calculator configuration HFXO frequency mismatch with chip",         \
    /*67*/ "Generic internal error",                                                   \
    /*68*/ "Software modem image does not support requested modulation",               \
    /*69*/ "Failed to disable RTCC synchronization",                                   \
    /*70*/ "Multitimer linked list corrupted",                                         \
    /*71*/ "Unable to configure radio for temperature calibration",                    \
    /*72*/ "Invalid VDET configuration",                                               \
    /*73*/ "PHY and System RFFPLL settings do not match",                              \
    /*74*/ "Secure access fault",                                                      \
    /*75*/ "SYSRTC0 not running",                                                      \
    /*76*/ "Radio Configurator not updated",                                           \
    /*77*/ "Failed to set the event for configurable RSSI threshold",                  \
    /*78*/ "Intended and actual Z-Wave region configuration mismatch",                 \
    /*79*/ "Attempted to sleep with stale RTCC synchronization data",                  \
    /*80*/ "Attempted to clear LOG2X4 with a DEC1 value not equal to 0",               \
    /*81*/ "Failed to complete DMA write",                                             \
    /*82*/ "RAIL does not support this Radio Calculator configuration",                \
    /*83*/ "Invalid binary image loaded on sequencer",                                 \
    /*84*/ "No common or protocol image selected to be loaded onto the sequencer",     \
    /*85*/ "Software modem image invalid or missing",                                  \
    /*86*/ "The sequencer user generated error",                                       \
}

/** @} */ // end of Assertions

#ifdef __cplusplus
}
#endif

#endif  // SL_RAIL_ASSERT_ERROR_CODES_H
