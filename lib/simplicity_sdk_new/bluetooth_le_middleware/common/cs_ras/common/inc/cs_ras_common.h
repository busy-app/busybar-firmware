/***************************************************************************//**
 * @file
 * @brief CS RAS - Common header
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

#ifndef CS_RAS_COMMON_H
#define CS_RAS_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include "sl_bt_api.h"
#include "sl_enum.h"

// 2.4 Attribute Protocol Application error code
#define ATT_ERR_SUCCESS                     0x00
#define ATT_ERR_REQUEST_NOT_SUPPORTED       ((uint8_t)SL_STATUS_BT_ATT_REQUEST_NOT_SUPPORTED)
#define ATT_ERR_INVALID_OFFSET              ((uint8_t)SL_STATUS_BT_ATT_INVALID_OFFSET)
#define ATT_ERR_INVALID_ATT_LENGTH          ((uint8_t)SL_STATUS_BT_ATT_INVALID_ATT_LENGTH)
#define ATT_ERR_INSUFFICIENT_ENCRYPTION     ((uint8_t)SL_STATUS_BT_ATT_INSUFFICIENT_ENCRYPTION)
#define ATT_ERR_CCCD_IMPROPERLY_CONFIGURED  0xFD
#define ATT_ERR_REQUEST_REJECTED            0xFC

// 3.1.1 RAS Features format
typedef uint32_t cs_ras_features_t;

#define CS_RAS_FEATURE_RT_RANGING_DATA_MASK       (1 << 0)
#define CS_RAS_FEATURE_RETRIEVE_LOST_SEGMENT_MASK (1 << 1)
#define CS_RAS_FEATURE_ABORT_OP_MASK              (1 << 2)
#define CS_RAS_FEATURE_FILTERING_MASK             (1 << 3)

// Ranging data format
// Segment header
typedef uint8_t cs_ras_segment_header_t;

// Segment header size
#define CS_RAS_SEGMENT_HEADER_SIZE                   sizeof(cs_ras_segment_header_t)

// Segment overhead
#define CS_RAS_SEGMENT_OVERHEAD                      4

// Segment data size as a function of ATT_MTU (data only, without header)
#define CS_RAS_SEGMENT_DATA_SIZE(att_mtu)            (att_mtu - CS_RAS_SEGMENT_OVERHEAD)

// Segment size (including header)
#define CS_RAS_SEGMENT_SIZE(att_mtu)                 (CS_RAS_SEGMENT_DATA_SIZE(att_mtu) + CS_RAS_SEGMENT_HEADER_SIZE)

// ATT MTU maximum size
#define ATT_MTU_MAX                                  250
// ATT MTU minimum size
#define ATT_MTU_MIN                                  23

// Maximum size of a segment (including header)
#define CS_RAS_SEGMENT_SIZE_MAX                      (ATT_MTU_MAX - CS_RAS_SEGMENT_OVERHEAD + CS_RAS_SEGMENT_HEADER_SIZE)

// Segment counter shift
#define CS_RAS_SEGMENT_COUNTER_SHIFT                 2

// All segments
#define CS_RAS_ALL_SEGMENTS                          0xFF
#define CS_RAS_MESSAGING_ALL_SEGMENTS                CS_RAS_ALL_SEGMENTS

// Type for storing a messaging segment
typedef PACKSTRUCT (struct {
  union {
    uint8_t                   data[CS_RAS_SEGMENT_SIZE_MAX]; // Array interpretation
    struct {
      cs_ras_segment_header_t header;                        // Segment header (flags and counter)
      uint8_t                 data[];                        // Data content of the segment
    }                         segment;                       // Segment interpretation
  };
}) cs_ras_segment_storage_t;

// Messaging configuration type
typedef struct {
  uint32_t                    data_size;       ///< Size of data array
  uint8_t                     *data;           ///< Data array
  uint8_t                     start_segment;   ///< Start segment to transfer
  uint16_t                    end_segment;     ///< End segment to transfer
  uint16_t                    att_mtu;         ///< ATT MTU size
  uint8_t                     conn_handle;     ///< Connection handle
  bool                        indication;      ///< Send indication (true) or notification (false)
  bool                        real_time;       ///< Use Real-Time (true) or On-Demand (false) Ranging data characteristics
} cs_ras_messaging_config_t;

// Messaging status type
typedef enum {
  CS_RAS_MESSAGING_STATUS_SUCCESS   = 0x00,
  CS_RAS_MESSAGING_STATUS_CANCELLED = 0x01,
  CS_RAS_MESSAGING_STATUS_FAILURE   = 0x02,
  CS_RAS_MESSAGING_STATUS_TIMEOUT   = 0x03,
} cs_ras_messaging_status_t;

// Segment header definitions
#define CS_RAS_SEGMENT_HEADER_EMPTY                  0x00
#define CS_RAS_FIRST_SEGMENT_MASK                    (1 << 0)
#define CS_RAS_LAST_SEGMENT_MASK                     (1 << 1)
#define CS_RAS_SEGMENT_COUNTER_MASK                  ((cs_ras_segment_header_t)((((cs_ras_segment_header_t)(~(0U))) << CS_RAS_SEGMENT_COUNTER_SHIFT)))
#define CS_RAS_SEGMENT_COUNTER_MOD                   (1 << (8 - CS_RAS_SEGMENT_COUNTER_SHIFT))
#define CS_RAS_SET_FIRST_SEGMENT(header)             ((header) |= CS_RAS_FIRST_SEGMENT_MASK)
#define CS_RAS_SET_LAST_SEGMENT(header)              ((header) |= CS_RAS_LAST_SEGMENT_MASK)
#define CS_RAS_IS_FIRST_SEGMENT(header)              (((header) & CS_RAS_FIRST_SEGMENT_MASK) > 0)
#define CS_RAS_IS_LAST_SEGMENT(header)               (((header) & CS_RAS_LAST_SEGMENT_MASK) > 0)
#define CS_RAS_SET_SEGMENT_COUNTER(header, counter)  ((header) |= (((counter) << CS_RAS_SEGMENT_COUNTER_SHIFT) & CS_RAS_SEGMENT_COUNTER_MASK))
#define CS_RAS_GET_SEGMENT_COUNTER(header)           (((header) & CS_RAS_SEGMENT_COUNTER_MASK) >> CS_RAS_SEGMENT_COUNTER_SHIFT)

// Ranging counter
typedef uint16_t cs_ras_ranging_counter_t;

// Ranging header
typedef PACKSTRUCT (struct {
  uint16_t ranging_counter  : 12; // Lower 12 bit of Procedure_Counter
  uint8_t  configuration_id : 4;  // CS configuration identifier Range: 0 to 3
  int8_t   selected_tx_power;     // Transmit power level used for the CS Procedure Range:
                                  // -127 to 20 Units: dBm
  uint8_t  antenna_paths_mask;    // Antenna paths that are reported
}) cs_ras_ranging_header_t;

// Subevent header
typedef PACKSTRUCT (struct {
  uint16_t start_acl_connection_event; // Starting ACL connection event count for the results
                                       // reported in the event
  uint16_t frequency_compensation;     // Frequency compensation value in units of 0.01 ppm
                                       // (15-bit signed integer)
  uint8_t  ranging_done_status   : 4;  // Ranging Done status.
                                       // 0x0: All results complete for the CS Procedure
                                       // 0x1: Partial results with more to follow for the CS procedure
                                       // 0xF: All subsequent CS Procedures aborted
                                       // All other values: RFU
  uint8_t  subevent_done_status  : 4;  // Subevent Done Status
                                       // 0x0: All results complete for the CS Subevent
                                       // 0xF: Current CS Subevent aborted.
                                       // All other values: RFU
  uint8_t  abort_reason;               // Abort Reason
                                       // Ranging Abort Reason (4 bits)
                                       // Indicates the abort reason when Procedure_Done Status
                                       // received from the Core Controller (see Volume 4, Part 4,
                                       // Section 7.7.65.44 in [1]) is set to 0xF,
                                       // otherwise the value is set to zero.
                                       // 0x0: Report with no abort
                                       // 0x1: Abort because of local Host or remote request
                                       // 0x2: Abort because filtered channel map has less than 15 channels
                                       // 0x3: Abort because the channel map update instant has passed
                                       // 0xF: Abort because of unspecified reasons
                                       // All other values: RFU
                                       // Subevent Abort Reason (4 bits)
                                       // Indicates the abort reason when Subevent_Done_Status
                                       // received from the Core Controller (see Volume 4, Part 4,
                                       // Section 7.7.65.44 in [1]) is set to 0xF, otherwise the
                                       // default value is set to zero.
                                       // 0x0: Report with no abort
                                       // 0x1: Abort because of local Host or remote request
                                       // 0x2: Abort because no CS_SYNC (mode 0) received 0x3:
                                       // Abort because of scheduling conflicts or limited resources
                                       // 0xF: Abort because of unspecified reasons
                                       // All other values: RFU
  uint8_t  reference_power_level;      // Reference power level. Range: -127 to 20. Units: dBm
  uint8_t  number_of_steps_reported;   // Number of steps in the CS Subevent for which results are
                                       // reported. If the Subevent is aborted, then the Number Of
                                       // Steps Reported can be set to zero
}) cs_ras_subevent_header_t;

// RAS data header
typedef struct {
  uint8_t step_mode;
  uint8_t step_channel;
  uint8_t step_data_length;
} cs_ras_step_header_t;

// RAS Filter setting
typedef uint16_t cs_ras_filter_t;

typedef struct {
  cs_ras_filter_t mode0_filter;
  cs_ras_filter_t mode1_filter;
  cs_ras_filter_t mode2_filter;
  cs_ras_filter_t mode3_filter;  // Not yet implemented
} cs_ras_filter_bank_t;

// RAS CP

// Opcodes
SL_ENUM(cs_ras_cp_opcode_t) {
  CS_RAS_CP_OPCODE_GET                    = 0x00, // Mandatory
  CS_RAS_CP_OPCODE_ACK                    = 0x01, // Mandatory
  CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS = 0x02, // Optional
  CS_RAS_CP_OPCODE_ABORT                  = 0x03, // Optional
  CS_RAS_CP_OPCODE_SET_FILTER             = 0x04  // Optional
};

// Responses
SL_ENUM(cs_ras_cp_response_opcode_t) {
  CS_RAS_CP_RESPONSE_COMPLETE              = 0x00, // Mandatory
  CS_RAS_CP_RESPONSE_COMPLETE_LOST_SEGMENT = 0x01, // Mandatory, if CS_RAS_CP_OPCODE_RETRIEVE_LOST_SEGMENTS is supported.
  CS_RAS_CP_RESPONSE_CODE                  = 0x02  // Mandatory
};

// Response code
SL_ENUM(cs_ras_cp_response_code_value_t) {
  CS_RAS_CP_RESPONSE_CODE_SUCCESS                 = 0x01, // Normal response for a successful operation.
  CS_RAS_CP_RESPONSE_CODE_NOT_SUPPORTED           = 0x02, // Normal response if unsupported Op Code is received.
  CS_RAS_CP_RESPONSE_CODE_INVALID_PARAMETER       = 0x03, // Normal response if Parameter received does not meet
                                                          // the requirements of the service.
  CS_RAS_CP_RESPONSE_CODE_SUCCESS_PERSISTED       = 0x04, // Normal response for a successful write operation
                                                          // where the values written to RAS-CP are being persisted.
  CS_RAS_CP_RESPONSE_CODE_ABORT_UNSUCCESSFUL      = 0x05, // Normal response if request for Abort is unsuccessful.
  CS_RAS_CP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED = 0x06, // Normal response if unable to complete a procedure
                                                          // for any reason.
  CS_RAS_CP_RESPONSE_CODE_SERVER_BUSY             = 0x07, // Normal response if the Server is still busy with
                                                          // other requests.
  CS_RAS_CP_RESPONSE_CODE_NO_RECORDS_FOUND        = 0x08  // Normal response if the requested Ranging
                                                          // counter is not found.
};

// Mode dependent bit fields for filters
// Mode 0 - Calibration
#define CS_RAS_FILTER_MODE0_PACKET_QUALITY_MASK            (1 << 0)
#define CS_RAS_FILTER_MODE0_PACKET_RSSI_MASK               (1 << 1)
#define CS_RAS_FILTER_MODE0_PACKET_ANTENNA_MASK            (1 << 2)
#define CS_RAS_FILTER_MODE0_MEASURED_FREQ_OFFSET           (1 << 3)

// Mode 1 - RTT
#define CS_RAS_FILTER_MODE1_PACKET_NADM_MASK               (1 << 1)
#define CS_RAS_FILTER_MODE1_PACKET_RSSI_MASK               (1 << 2)
#define CS_RAS_FILTER_MODE1_TOD_TOA_MASK                   (1 << 3)
#define CS_RAS_FILTER_MODE1_PACKET_ANTENNA_MASK            (1 << 4)
#define CS_RAS_FILTER_MODE1_PACKET_PCT1_MASK               (1 << 5)
#define CS_RAS_FILTER_MODE1_PACKET_PCT2_MASK               (1 << 6)

// Mode 2 - PBR
#define CS_RAS_FILTER_MODE2_ANTENNA_PERMUTATION_INDEX_MASK (1 << 0)
#define CS_RAS_FILTER_MODE2_TONE_PCT_MASK                  (1 << 1)
#define CS_RAS_FILTER_MODE2_TONE_QUALITY_INDICATOR_MASK    (1 << 2)
#define CS_RAS_FILTER_MODE2_ANTENNA_PATH_1_MASK            (1 << 3)
#define CS_RAS_FILTER_MODE2_ANTENNA_PATH_2_MASK            (1 << 4)
#define CS_RAS_FILTER_MODE2_ANTENNA_PATH_3_MASK            (1 << 5)
#define CS_RAS_FILTER_MODE2_ANTENNA_PATH_4_MASK            (1 << 6)

// Mode 3 - RTT + PBR Not implemented
#define CS_RAS_FILTER_MODE3_PACKET_QUALITY_MASK            (1 << 0)
#define CS_RAS_FILTER_MODE3_PACKET_NADM_MASK               (1 << 1)
#define CS_RAS_FILTER_MODE3_PACKET_RSSI_MASK               (1 << 2)
#define CS_RAS_FILTER_MODE3_TOD_TOA_MASK                   (1 << 3)
#define CS_RAS_FILTER_MODE3_PACKET_ANTENNA_MASK            (1 << 4)
#define CS_RAS_FILTER_MODE3_Packet_PCT1_MASK               (1 << 5)
#define CS_RAS_FILTER_MODE3_Packet_PCT2_MASK               (1 << 6)
#define CS_RAS_FILTER_MODE3_ANTENNA_PERMUTATION_INDEX_MASK (1 << 7)
#define CS_RAS_FILTER_MODE3_TONE_PCT_MASK                  (1 << 8)

// Characteristic UUIDs
#define CS_RAS_CHAR_UUID_RAS_FEATURES             0x2C14
#define CS_RAS_CHAR_UUID_REAL_TIME_RANGING_DATA   0x2C15
#define CS_RAS_CHAR_UUID_ON_DEMAND_RANGING_DATA   0x2C16
#define CS_RAS_CHAR_UUID_CONTROL_POINT            0x2C17
#define CS_RAS_CHAR_UUID_RANGING_DATA_READY       0x2C18
#define CS_RAS_CHAR_UUID_RANGING_DATA_OVERWRITTEN 0x2C19

// Invalid characteristic handle
#define CS_RAS_INVALID_CHARACTERISTIC_HANDLE       0xFFFF

// Invalid ranging counter value
#define CS_RAS_INVALID_RANGING_COUNTER             0xFFFF
#define CS_RAS_RANGING_COUNTER_MASK                0x0FFFU

// Initial ranging counter value
#define CS_RAS_INITIAL_RANGING_COUNTER             0x0000

#define CS_RAS_STEP_MODE_MASK                      0x3
#define CS_RAS_STEP_ABORTED_MASK                   (1 << 7)

typedef PACKSTRUCT (struct {
  uint16_t ranging_counter;
  uint8_t  start_segment;
  uint8_t  end_segment;
}) cs_ras_segments_t;

typedef PACKSTRUCT (struct {
  cs_ras_ranging_counter_t ranging_counter;
}) cs_ras_counter_t;

typedef cs_ras_counter_t cs_ras_cp_get_ranging_data_t;
typedef cs_ras_counter_t cs_ras_cp_ack_ranging_data_t;
typedef cs_ras_segments_t cs_ras_cp_retreive_lost_t;

typedef PACKSTRUCT (struct {
  uint16_t filter_configuration;
}) cs_ras_cp_set_filter_t;

typedef PACKSTRUCT (struct {
  cs_ras_cp_response_code_value_t value;
}) cs_ras_cp_response_code_t;

typedef cs_ras_segments_t cs_ras_cp_complete_lost_response_t;
typedef cs_ras_ranging_counter_t cs_ras_cp_complete_response_t;

typedef PACKSTRUCT (struct {
  cs_ras_cp_response_opcode_t          op_code;
  union {
    cs_ras_cp_complete_response_t      complete_ranging_data;
    cs_ras_cp_complete_lost_response_t complete_lost_segment;
    cs_ras_cp_response_code_t          response_code;
  }                                    data;
}) cs_ras_cp_response_t;

typedef PACKSTRUCT (struct {
  cs_ras_cp_opcode_t                   op_code;
  union {
    cs_ras_cp_get_ranging_data_t       get_ranging_data;
    cs_ras_cp_ack_ranging_data_t       ack_ranging_data;
    cs_ras_cp_retreive_lost_t          retreive_lost;
    cs_ras_cp_set_filter_t             set_filter;
  }                                    data;
}) cs_ras_cp_command_t;

// RAS mode
typedef enum {
  CS_RAS_MODE_CHANGE_IN_PROGRESS,
  CS_RAS_MODE_NONE,
  CS_RAS_MODE_ON_DEMAND_RANGING_DATA,
  CS_RAS_MODE_REAL_TIME_RANGING_DATA
} cs_ras_mode_t;

/// RAS characteristic index type used in arrays
typedef enum {
  CS_RAS_CHARACTERISTIC_INDEX_RAS_FEATURES = 0,
  CS_RAS_CHARACTERISTIC_INDEX_REAL_TIME_RANGING_DATA,
  CS_RAS_CHARACTERISTIC_INDEX_CONTROL_POINT,
  CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_READY,
  CS_RAS_CHARACTERISTIC_INDEX_RANGING_DATA_OVERWRITTEN,
  CS_RAS_CHARACTERISTIC_INDEX_ON_DEMAND_RANGING_DATA,
  CS_RAS_CHARACTERISTIC_INDEX_COUNT
} cs_ras_characteristic_index_t;

/// GATTDB handle storage type
typedef struct {
  uint16_t array[CS_RAS_CHARACTERISTIC_INDEX_COUNT];
} cs_ras_gattdb_handles_t;

#endif // CS_RAS_COMMON_H
