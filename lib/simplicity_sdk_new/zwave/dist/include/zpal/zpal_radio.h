/**
 * @file
 * Defines a platform abstraction layer for the Z-Wave radio.
 *
 * @copyright 2021 Silicon Laboratories Inc.
 */

#ifndef ZPAL_RADIO_H_
#define ZPAL_RADIO_H_

#include <stdbool.h>
#include <stdint.h>
#include "zpal_status.h"
#include "zpal_misc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup zpal
 * @brief Z-Wave Platform Abstraction Layer.
 * @{
 * @addtogroup zpal-radio
 * @brief Defines a platform abstraction layer for the Z-Wave radio.
 *
 * The ZPAL Radio module contains several APIs which are required
 * to be implemented.
 *
 * A user of this radio module shall first define all parameters
 * of the Z-Wave Radio Profile (i.e., zpal_radio_profile_t) and
 * initialise the radio using zpal_radio_init API.
 *
 * After the initialisation of the radio is executed, the user can use any
 * of the radio APIs to execute radio related paradigm for instance
 * - zpal_radio_transmit shall be used to transmit a Z-Wave frame on radio
 * - zpal_radio_start_receive shall be used to enable the reception of Z-Wave frame
 *
 * The radio API assumes that the radio will return to receive mode with channel
 * hopping enabled after transmitting a frame.
 *
 *
 * @{
 */

/**
 * @brief RSSI value is invalid or not measured
 */
#define ZPAL_RADIO_INVALID_RSSI_DBM (-128)
#define ZPAL_RADIO_RSSI_NOT_AVAILABLE (127)

//deci-dBm values
#define ZW_TX_POWER_100_DDBM  100
#define ZW_TX_POWER_140_DDBM  140
#define ZW_TX_POWER_200_DDBM  200

/**
 * @deprecated use ZW_TX_POWER_100_DDBM instead
 */
#define ZW_TX_POWER_10DBM  ZW_TX_POWER_100_DDBM
/**
 * @deprecated use ZW_TX_POWER_140_DDBM instead
 */
#define ZW_TX_POWER_14DBM  ZW_TX_POWER_140_DDBM
/**
 * @deprecated use ZW_TX_POWER_200_DDBM instead
 */
#define ZW_TX_POWER_20DBM  ZW_TX_POWER_200_DDBM

#define ZPAL_RADIO_STAY_AWAKE_ALWAYS UINT32_MAX

/**
 * @addtogroup ZPAL_RADIO_NUM_CHANNELS Number of Channels
 * @{
 */
#define ZPAL_RADIO_NUM_CHANNELS                 3 ///< Number of channels of classic Z-Wave protocol
#define ZPAL_RADIO_NUM_CHANNELS_LR_CH_CFG_1_2   4 ///< Number of channels of Z-Wave Long Range protocol
                                                  ///< 100kbps, 40kbps, 9.6kbps and LRA/LRB
                                                  ///< (channel configuration 1 & 2)
#define ZPAL_RADIO_NUM_CHANNELS_LR_CH_CFG3      2 ///< LRA and LRB (channel configuration 3)
///@}

/**
 * @brief Node ID type.
 */
typedef uint16_t node_id_t;

/**
 * @brief Parameter type to store deci dBm values.
 */
typedef int16_t zpal_tx_power_decidbm_t;
/// definition for maximum output power to use with zpal_tx_power_decidbm_t.
#define ZPAL_TX_POWER_DECIDBM_MAX   0x7FFF
/**
 * @deprecated use zpal_tx_power_decidbm_t instead
 */
typedef zpal_tx_power_decidbm_t zpal_tx_power_t;

/**
 * @brief Parameter type to store stay awake ids.
 */
typedef uint32_t zpal_radio_stay_awake_id_t;

/**
 * @brief Wakeup interval for the radio. A FLiRS node will use 250 or 1000 ms interval, all other
 * nodes should be configured as always listening.
 */
typedef enum {
  ZPAL_RADIO_LISTEN_NEVER,  ///< Node is not listening (Only listen when application requests it).
  ZPAL_RADIO_LISTEN_ALWAYS, ///< Node is always listening.
  ZPAL_RADIO_LISTEN_FREQUENTLY_250ms,   ///< Node wakes up every 250 ms interval to listen for a wakeup beam.
  ZPAL_RADIO_LISTEN_FREQUENTLY_1000ms,  ///< Node wakes up every 1000 ms interval to listen for a wakeup beam.
} zpal_radio_listen_t;

/**
 * @brief Enumeration containing supported checksum types in Z-Wave.
 */
typedef enum {
  ZPAL_RADIO_CRC_NONE,         ///< No checksum.
  ZPAL_RADIO_CRC_8_BIT_XOR,    ///< 8 bit XOR checksum.
  ZPAL_RADIO_CRC_16_BIT_CCITT, ///< 16 bit CRC-CCITT checksum.
} zpal_radio_crc_t;

/**
 * @brief Enumeration containing supported baud rates.
 */
typedef enum {
  ZPAL_RADIO_SPEED_UNDEFINED,
  ZPAL_RADIO_SPEED_9600,
  ZPAL_RADIO_SPEED_40K,
  ZPAL_RADIO_SPEED_100K,
  ZPAL_RADIO_SPEED_100KLR,
} zpal_radio_speed_t;

/**
 * @brief Enumeration containing Long Range Channels.
 */
typedef enum {
  ZPAL_RADIO_LR_CHANNEL_UNINITIALIZED, ///< Long Range Channel setting not initialized.
  ZPAL_RADIO_LR_CHANNEL_A,             ///< Long Range Channel A.
  ZPAL_RADIO_LR_CHANNEL_B,             ///< Long Range Channel B.
  ZPAL_RADIO_LR_CHANNEL_UNKNOWN,       ///< Long Range Channel Unknown.
  ZPAL_RADIO_LR_CHANNEL_AUTO = 255,   ///< Long Range automatically selected Channel.
} zpal_radio_lr_channel_t;

/**
 * @brief Enumeration containing Z-Wave channels.
 *
 */
typedef enum {
  ZPAL_RADIO_ZWAVE_CHANNEL_0 = 0,             ///< Z-Wave channel 0.
  ZPAL_RADIO_ZWAVE_CHANNEL_1 = 1,             ///< Z-Wave channel 1.
  ZPAL_RADIO_ZWAVE_CHANNEL_2 = 2,             ///< Z-Wave channel 2.
  ZPAL_RADIO_ZWAVE_CHANNEL_3 = 3,             ///< Z-Wave channel 3.
  ZPAL_RADIO_ZWAVE_CHANNEL_4 = 4,             ///< Z-Wave channel 4.
  ZPAL_RADIO_ZWAVE_CHANNEL_NUM,               ///< Number of Z-Wave channels. must be last
  ZPAL_RADIO_ZWAVE_CHANNEL_UNKNOWN = 0xFF,    ///< Z-Wave channel Unknown.
} zpal_radio_zwave_channel_t;

/**
 * @brief Enumeration containing Z-Wave channels known as protocol modes.
 */
typedef enum {
  ZPAL_RADIO_PROTOCOL_MODE_1,                ///< 2 Channel Protocol Mode.
  ZPAL_RADIO_PROTOCOL_MODE_2,                ///< 3 Channel Protocol Mode.
  ZPAL_RADIO_PROTOCOL_MODE_3,                ///< 4 Channel Protocol Mode - Combination of 2 Channel (9.6kb/40kb, 100kb) and Long Range Channel.
  ZPAL_RADIO_PROTOCOL_MODE_4,                ///< 2 Long Range Channel Protocol Mode - End Device.
  ZPAL_RADIO_PROTOCOL_MODE_UNDEFINED = 0xFF, ///< Protocol mode undefined means invalid region code.
} zpal_radio_protocol_mode_t;

/**
 * @brief List of LR channel configuration. Used to select the correct phy configuration.
 */
typedef enum {
  ZPAL_RADIO_LR_CH_CFG_NO_LR, /**< used if region does not support Long range or end device included in classic network.
                                   Phy is configured to listen only on classic Z-Wave channels.*/
  ZPAL_RADIO_LR_CH_CFG1,      /**< used by controller with primary channel A and not included end device
                                   Phy is configured to listen on classic Z-Wave channels and Long Range channel A.*/
  ZPAL_RADIO_LR_CH_CFG2,      /**< used only by controller with primary channel B
                                   Phy is configured to listen on classic Z-Wave channels and Long Range channel B.*/
  ZPAL_RADIO_LR_CH_CFG3,      /**< used only by end device when included in a LR network.
                                   Phy is configured to listen only on Long Range channels A & B.*/
  ZPAL_RADIO_LR_CH_CFG_COUNT  ///< enumeration items count
} zpal_radio_lr_channel_config_t;

/**
 * @brief Enumeration official Z-Wave regions.
 */
typedef enum {
  REGION_2CH_FIRST = 0,                             ///< First 2 channels region (with 3 data rates)
  REGION_EU = REGION_2CH_FIRST,                     ///< Radio is located in Region EU. 2 Channel region.
  REGION_US,                                        ///< Radio is located in Region US. 2 Channel region.
  REGION_ANZ,                                       ///< Radio is located in Region Australia/New Zealand. 2 Channel region.
  REGION_HK,                                        ///< Radio is located in Region Hong Kong. 2 Channel region.
  REGION_DEPRECATED_4,                              ///< Deprecated value, should never be used.
  // Malaysian products now use ANZ region.
  REGION_IN = 5,                                    ///< Radio is located in Region India. 2 Channel region.
  REGION_IL,                                        ///< Radio is located in Region Israel. 2 Channel region.
  REGION_RU,                                        ///< Radio is located in Region Russia. 2 Channel region.
  REGION_CN,                                        ///< Radio is located in Region China. 2 Channel region.
  REGION_US_LR,                                     ///< Radio is located in Region US. 2 Channel LR region.
  REGION_DEPRECATED_10,                             ///< Deprecated value, should never be used.
  REGION_EU_LR,                                     ///< Radio is located in Region EU. 2 Channel LR region.
  REGION_2CH_END,                                   ///< end of 2 channels regions (with 3 data rates)

  REGION_3CH_FIRST = 32,                            ///< First 3 channels region (with 1 data rate)
  REGION_JP = REGION_3CH_FIRST,                     ///< Radio is located in Region Japan. 3 Channel region.
  REGION_KR,                                        ///< Radio is located in Region Korea. 3 Channel region.
  REGION_3CH_END,                                   ///< end of 3 channels regions (with 1 data rate)

  REGION_DEPRECATED_48 = 48,                        ///< Deprecated value, should never be used.
  REGION_UNDEFINED = 0xFE,
  REGION_DEFAULT = 0xFF, ///< Radio is located in Library Default Region EU. 2 Channel region.
} zpal_radio_region_t;

#define REGION_2CH_NUM = (REGION_2CH_END - REGION_2CH_FIRST)    ///< Number of 2 channel regions
#define REGION_3CH_NUM = (REGION_3CH_END - REGION_3CH_FIRST)    ///< Number of 3 channel regions

/**
 * @brief Enumeration containing Tx power settings.
 *
 */
typedef enum {
  ZPAL_RADIO_TX_POWER_DEFAULT, ///< Max Tx power.
  ZPAL_RADIO_TX_POWER_MINUS1_DBM,
  ZPAL_RADIO_TX_POWER_MINUS2_DBM,
  ZPAL_RADIO_TX_POWER_MINUS3_DBM,
  ZPAL_RADIO_TX_POWER_MINUS4_DBM,
  ZPAL_RADIO_TX_POWER_MINUS5_DBM,
  ZPAL_RADIO_TX_POWER_MINUS6_DBM,
  ZPAL_RADIO_TX_POWER_MINUS7_DBM,
  ZPAL_RADIO_TX_POWER_MINUS8_DBM,
  ZPAL_RADIO_TX_POWER_MINUS9_DBM,
  ZPAL_RADIO_TX_POWER_MINUS10_DBM,
  ZPAL_RADIO_TX_POWER_MINUS11_DBM,
  ZPAL_RADIO_TX_POWER_MINUS12_DBM,
  ZPAL_RADIO_TX_POWER_MINUS13_DBM,
  ZPAL_RADIO_TX_POWER_MINUS14_DBM,
  ZPAL_RADIO_TX_POWER_REDUCED = ZPAL_RADIO_TX_POWER_MINUS3_DBM, ///< Default reduced Tx power.
} zpal_radio_tx_power_t;

/**
 * @brief Parameters required when transmitting a frame.
 */
typedef struct {
  zpal_radio_speed_t speed;              ///< Channel Speed to use when transmitting a frame.
  zpal_radio_zwave_channel_t channel_id; ///< Channel id to use when transmitting a frame.
  zpal_radio_crc_t crc;                  ///< CRC Type to use. XOR is used in 2 channel at 9600 and 40k baud rate. CRC CCITT is used in 100k frames.
  uint8_t preamble;                      ///< The byte value used for the preamble sequence.
  uint8_t preamble_length;               ///< Length of the preamble. Minimum preamble length is specified in ITU G.9959-2015.
  uint8_t start_of_frame;                ///< The start of frame byte used to indicate the end of preamble and the start of frame.
  uint8_t repeats;                       ///< Number of repetitions for the frame. This is used for wakeup beams where the beam is to be repeated for 250 or 1000 ms.
  uint8_t use_lbt;                       ///< 1 to transmit frame with LBT, 0 without LBT
  union {
    zpal_radio_tx_power_t tx_power_index;  ///< The RF tx power to use for transmitting.
    zpal_tx_power_decidbm_t lr_tx_power;   ///< The RF tx power to use for transmitting on Long Range channels in deci-dBm.
  };
  zpal_time_t turnaround_ref_tick;       /**< Timestamp (in zpal base time) that must be used as reference for the
                                              Rx-To-Tx turnaround time. Mostly used for Ack.
                                              If not 0, the zpal must guarantee the minimum Rx-To-Tx turnaround
                                              time before the Tx.
                                              Not done with the TxQueue delay because:
                                              - TxQueue delay is 1ms base time which is not enough (Rx-to-Tx turnaround time is 1ms)
                                              - Ack is high priority and should "lock" the TxQueue without any delay (TxQueue
                                              delay is handled before adding the frame to the TxQueue). */
} zpal_radio_transmit_parameter_t;

/**
 * @brief Z-Wave Frame header format types.
 */
typedef enum {
  ZPAL_RADIO_HEADER_TYPE_2CH,       ///< 2 Channel header format.
  ZPAL_RADIO_HEADER_TYPE_3CH,       ///< 3 Channel header format.
  ZPAL_RADIO_HEADER_TYPE_LR,        ///< LR Channel header format.
  ZPAL_RADIO_HEADER_TYPE_UNDEFINED, ///< Undefined Channel header format.
} zpal_radio_header_type_t;

/**
 * @brief Structure with radio parameters for a received frame.
 */
typedef struct {
  zpal_radio_speed_t speed;                       ///< Speed for the frame received.
  zpal_radio_zwave_channel_t channel_id;          ///< Channel id on which the frame was received.
  zpal_radio_header_type_t channel_header_format; ///< Z-Wave Header format used in channel frame was received on.
  int8_t rssi;                                    ///< Rssi value.
  zpal_time_t rx_zpal_tick;                         ///< Timestamp when the frame was received
} zpal_radio_rx_parameters_t;

typedef struct _zpal_radio_beam_info_t_ {
  node_id_t   node_id;
  int8_t      rssi;
  int8_t      tx_power_dbm;
  uint8_t     radio_channel;
  uint8_t     home_id_hash;
} zpal_radio_beam_info_t;

/**
 * @brief Enumeratio radio events.
 */
typedef enum {
  ZPAL_RADIO_EVENT_NONE,
  ZPAL_RADIO_EVENT_RX_COMPLETE,                 ///< Frame received
  ZPAL_RADIO_EVENT_TX_COMPLETE,                 ///< Transmit complete
  ZPAL_RADIO_EVENT_RX_BEAM_COMPLETE,            ///< Beam received
  ZPAL_RADIO_EVENT_TX_BEAM_COMPLETE,            ///< Beam sent
  ZPAL_RADIO_EVENT_RX_ABORT,                    ///< Receive was aborted
  ZPAL_RADIO_EVENT_TX_FAIL,                     ///< Transmit failed
  ZPAL_RADIO_EVENT_TX_FAIL_LBT,                 ///< Transmit failed because of an LBT failure
  ZPAL_RADIO_EVENT_RXTX_CALIBRATE,              ///< Radio needs calibration
  ZPAL_RADIO_EVENT_MASK = 0x1F,
  ZPAL_RADIO_EVENT_FLAG_SUCCESS = 0x80,         ///< Indicates a successful event
  ZPAL_RADIO_EVENT_TX_TIMEOUT = 254,
} zpal_radio_event_t;

typedef void (*zpal_radio_callback_t)(const zpal_radio_event_t event);

#define ZWAVE_MAXIMUM_PAYLOAD_LEGACY        64
#define ZWAVE_MAXIMUM_PAYLOAD_SIZE          170
#define ZWAVE_MAXIMUM_PAYLOAD_SIZE_LR       190

/**
 * @brief Z-Wave receive frame.
 */
typedef struct {
  zpal_radio_rx_parameters_t rx_parameters;              ///< Parameters for the received frame.
  uint8_t frame_content_length;                           ///< Length of payload following this frame.
  uint8_t frame_content[ZWAVE_MAXIMUM_PAYLOAD_SIZE_LR];   ///< Array with complete frame data received.
} zpal_radio_receive_frame_t;

/**
 * @brief Function pointer declaration for handling of received frames.
 *
 * @details When a frame is received this function will be invoked in order to process the frame.
 *
 * @param[in] rx_parameters Pointer to the structure with channel and rssi values.
 * @param[in] frame         Pointer to the received frame. The frame is expected to be located in
 *                          Z-Wave stack reserved memory and allocated throughout lifetime of stack
 *                          processing. Application should copy payload data if required for
 *                          unsynchronized data processing.
 *
 */
typedef void (*zpal_radio_receive_handler_t)(zpal_radio_rx_parameters_t *rx_parameters, zpal_radio_receive_frame_t *frame);

/**
 * @brief Network statistics structure
 */
typedef struct {
  uint32_t tx_frames;          ///< Transmitted Frames.
  uint32_t tx_lbt_back_offs;   ///< LBT backoffs.
  uint32_t rx_frames;          ///< Received Frames (No errors).
  uint32_t rx_lrc_errors;      ///< Checksum Errors.
  uint32_t rx_crc_errors;      ///< CRC16 Errors.
  uint32_t rx_foreign_home_id; ///< Foreign Home ID.
  uint32_t tx_time_channel[ZPAL_RADIO_ZWAVE_CHANNEL_NUM];  ///< Accumulated transmission time in ms for channel 0.
  uint32_t tx_recoveries;      ///< Transmit recoveries - frame that were suposed to be aborted that were succesfully transmited.
} zpal_radio_network_stats_t;

typedef enum {
  ZPAL_RADIO_APPLICATION_CONTROLLER = 0,     ///< Controller mode.
  ZPAL_RADIO_APPLICATION_END_DEVICE,         ///< End Device mode
  ZPAL_RADIO_APPLICATION_ZNIFFER             ///< Zniffer mode.
} zpal_radio_application_t;

/**
 * @brief Radio Profile containing region, baud rate, and wakeup interval for this device.
 */
typedef struct {
  zpal_radio_region_t region;                      ///< Region in which this system operates.
  zpal_radio_listen_t wakeup;                      ///< Wakeup interval for the radio.
  zpal_radio_lr_channel_t primary_lr_channel;      ///< Primary Long Range Channel.
  bool lr_channel_auto_mode;                       ///< Longe Range channel selection mode
  zpal_radio_lr_channel_config_t active_lr_channel_config; /**< Long Range channel configuration. Set only when
                                                              radio chip configuration is updated. Used to check that
                                                              requested channel configuration is different than the
                                                              active one.*/
  int8_t listen_before_talk_threshold;             ///< LBT Threshold for Transmit backoff in dBm.
  zpal_tx_power_decidbm_t tx_power_max;            ///< Z-Wave Transmit Power in deci dBm.
  zpal_tx_power_decidbm_t tx_power_adjust;         ///< Adjustment for antenna gain in deci dBm.
  zpal_tx_power_decidbm_t tx_power_max_lr;         ///< Max transmit power for Z-Wave LR in deci dBm.
  zpal_radio_callback_t rx_cb;                     ///< Pointer to function called by RF on Rx Completion.
  zpal_radio_callback_t tx_cb;                     ///< Pointer to function called by RF on Tx Completion.
  zpal_radio_callback_t region_change_cb;          ///< Pointer to function called by RF on Region change.
  zpal_radio_callback_t assert_cb;                 ///< Pointer to function called by RF on fatal Assert.
  zpal_radio_network_stats_t *network_stats;       ///< Pointer to structure where to RF Statistics are placed.
  uint8_t radio_debug_enable;                      ///< Enable radio debugging which is vendor specific.
  zpal_radio_application_t radio_application;      ///< Application type.
  bool is_joinable;                      ///< Indicate if the node has been correctly included in a network. Correctly included means that the node is joinable with tuple (homeid, nodeid).
} zpal_radio_profile_t;

/**
 * @brief Sets the home ID & the node ID.
 *
 * @param[in] home_id         Network home ID.
 * @param[in] node_id         Network node ID.
 * @param[in] home_id_hash    HomeID Hash. Used only for Flirs.
 */
void zpal_radio_set_network_ids(uint32_t home_id, node_id_t node_id, uint8_t home_id_hash);

/**
 * @brief Initializes the radio hardware with the specified configuration profile.
 *
 * This function sets up the radio module based on the provided profile,
 * ensuring it is ready for operation. The profile contains necessary
 * configuration parameters such as frequency, and other radio-specific settings.
 *
 * @param[in] profile Pointer to a structure containing the configuration
 *                    parameters for the radio. This must be properly
 *                    initialized before calling this function.
 *
 * @note Ensure that the profile is valid and all required fields are set
 *       before invoking this function to avoid undefined behavior.
 * @return @ref ZPAL_STATUS_OK if the zpal radio layer is ready
 *         @ref ZPAL_STATUS_INVALID_ARGUMENT if profile is NULL.
 */
zpal_status_t zpal_radio_init(const zpal_radio_profile_t * const profile);

/**
 * @brief Function used to change region and Long Range mode at the same time.
 *
 * @param[in] eRegion Region to change to.
 * @param[in] eLrChCfg long range mode to change to (in case of no LR region, should be set to ZPAL_RADIO_LR_CH_CFG_NO_LR).
 * @return @ref ZPAL_STATUS_OK if the region was successfully changed and @ref ZPAL_STATUS_FAIL otherwise.
 */
zpal_status_t zpal_radio_change_region(zpal_radio_region_t eRegion, zpal_radio_lr_channel_config_t eLrChCfg);

/**
 * @brief Function for getting REGION runtime.
 *
 * @return Current region.
 */
zpal_radio_region_t zpal_radio_get_region(void);

/**
 * @brief Function for transmitting a Z-Wave frame though the radio.
 *
 * @param[in] tx_parameters         Parameter setting specifying speed, channel, wakeup, crc for transmission.
 * @param[in] frame_header_length   Length of frame header data to transmit.
 * @param[in] frame_header_buffer   Pointer to data array containing the frame header.
 * @param[in] frame_payload_length  Length of frame payload data to transmit.
 * @param[in] frame_payload_buffer  Pointer to data array containing the frame payload.
 * @return @ref ZPAL_STATUS_OK if the data was successfully transmit, @ref ZPAL_STATUS_BUFFER_FULL when queue is full.
 */
zpal_status_t zpal_radio_transmit(zpal_radio_transmit_parameter_t const *const tx_parameters,
                                  uint8_t frame_header_length,
                                  uint8_t const *const frame_header_buffer,
                                  uint8_t frame_payload_length,
                                  uint8_t const *const frame_payload_buffer,
                                  uint8_t is_retransmission);

/**
 * @brief Function for transmitting a Z-Wave Beam frame though the radio.
 *
 * @param[in] tx_parameters Parameter setting specifying speed, channel, wakeup.
 * @param[in] beam_data_len Length of the Beam data to transmit.
 * @param[in] beam_data     Pointer to data array containing the BEAM data.
 * @return @ref ZPAL_STATUS_OK if the data was successfully transmit, @ref ZPAL_STATUS_BUFFER_FULL when queue is full.
 */
zpal_status_t zpal_radio_transmit_beam(zpal_radio_transmit_parameter_t const *const tx_parameters,
                                       uint8_t beam_data_len,
                                       uint8_t const *const beam_data);

/**
 * @brief Starts the receiver and enables reception of frames.
 * If the receiver is already started, nothing will happen.
 */
zpal_status_t zpal_radio_start_receive(void);

/**
 * @brief Checks if data is available for processing in the radio module.
 *
 * This function determines whether there is any data available in the
 * radio module that can be processed or retrieved.
 *
 * @return True if data is available, false otherwise.
 */
bool zpal_radio_is_data_available(void);

/**
 * @brief Restart the radio if a frame reception is in progress. The radio is restarted once the frame is completed.
 *
 * @details If a frame reception is in progress, the function wait for the end of the frame then
 *          restart the radio (in Rx mode). In other case, the function does nothing
 *
 * @return @ref ZPAL_STATUS_OK if the radio ready in receiver mode
 *         @ref ZPAL_STATUS_BUSY if the radio is not in Rx.
 *         @ref ZPAL_STATUS_FAIL if the function has failed to restart the radio
 */
zpal_status_t zpal_radio_restart_if_rx_in_progress(void);

/**
 * @brief Function to get the last received frame.
 *
 * @details This function retrieves the oldest received Z-Wave frame
 *          from the radio module. The frame data is stored in the provided
 *          structure.
 *
 * @param[out] frame Pointer to a structure where the last received frame
 *                   will be stored.
 *
 * @return @ref ZPAL_STATUS_OK if a valid frame is available and retrieved.
 *         @ref ZPAL_STATUS_FAIL if no frame is available or an error occurred.
 */
zpal_status_t zpal_radio_get_last_received_frame(zpal_radio_receive_frame_t* frame);

/**
 * @brief Function to get the protocol mode used in the configured region.
 *
 * @return Protocol mode used in the configured region.
 */
zpal_radio_protocol_mode_t zpal_radio_get_protocol_mode(void);

/**
 * @brief Function for setting the LBT RSSI level.
 *
 * @param[in] channel  uint8_t channel to set LBT threshold for.
 * @param[in] level         int8_t LBT RSSI level in dBm.
 */
void zpal_radio_set_lbt_level(uint8_t channel, int8_t level);

/**
 * @brief Enable or disables reception of broadcast beam.
 * Enable or disable FLiRS broadcast address.
 *
 * @param[in] enable  true to enable FLiRS broadcast address, false to disable.
 */
void zpal_radio_enable_rx_broadcast_beam(bool enable);

/**
 * @brief Function for clearing current Channel Transmit timers.
 */
void zpal_radio_clear_tx_timers(void);

/**
 * @brief Function for clearing current Network statistics.
 */
void zpal_radio_clear_network_stats(void);

/**
 * @brief Returns the background RSSI (in dBm).
 *
 * @param[in]   channel   uint8_t channel Id for measurement.
 * @param[out]  rssi      pointer where to store the background RSSI value.
 * @return @ref ZPAL_STATUS_OK if a valid RSSI value is available and read.
 *         @ref ZPAL_STATUS_BUSY if radio is busy. In this case rssi is set to SL_RAIL_RSSI_INVALID_DBM.
 *         @ref ZPAL_STATUS_INVALID_ARGUMENT if rssi pointer is null.
 */
zpal_status_t zpal_radio_get_background_rssi(uint8_t channel, int8_t *rssi);

/**
 * @brief Function for getting the default tx power in deci dBm.
 *
 * @return The default RF TX power in deci dBm
 */
zpal_tx_power_decidbm_t zpal_radio_get_default_tx_power(void);

/**
 * @brief Puts the radio into an idle state.
 *
 * This function transitions the radio to an idle state where it is neither
 * transmitting nor receiving. It can be used to conserve power or prepare
 * the radio for a new operation.
 */
zpal_status_t zpal_radio_idle(void);

/**
 * @brief Aborts any ongoing radio operation.
 *
 * This function halts any current transmission or reception activity
 * and brings the radio to an idle state. It can be used to immediately
 * stop the radio's operation in case of an error or when a higher-priority
 * operation needs to be performed.
 */
zpal_status_t zpal_radio_abort(void);

/**
 * @brief Shuts down the radio hardware.
 *
 * This function powers down the radio hardware, ensuring it is in a low-power state.
 * It should be called when the radio is no longer needed to conserve energy.
 *
 * @note This function is definitive and should be used with caution.
 *       Once called, the radio cannot be restarted without reinitializing it.
 */
zpal_status_t zpal_radio_shutdown(void);

/**
 * @brief Resets the radio configuration to receive mode after having received a beam.
 *
 * @param[in] start_receiver If set to true, the receiver will start listening. Otherwise, it will
 *                           stay inactive.
 */
zpal_status_t zpal_radio_reset_after_beam_receive(bool start_receiver);

/**
 * @brief Returns whether use of fragmented beams is enabled or not for the active region.
 *
 * @return True if use of fragmented beams is enabled, false otherwise.
 */
bool zpal_radio_is_fragmented_beam_enabled(void);

/**
 * @brief Returns whether listen before talk (LBT) is enabled.
 *
 * @return True if LBT is enabled, false otherwise.
 */
bool zpal_radio_is_lbt_enabled(void);

/**
 * @brief Returns the time it takes to start transmission of wake up beams. Includes
 * time spent on LBT.
 *
 * @return startup time in milli seconds.
 */
uint16_t zpal_radio_get_beam_startup_time(void);

/**
 * @brief Returns the minimum transmit power for Z-Wave Long Range.
 *
 * @return Minimum TX power in deci dBm.
 */
zpal_tx_power_decidbm_t zpal_radio_get_minimum_lr_tx_power(void);

/**
 * @brief Returns the maximum transmit power for Z-Wave Long Range.
 *
 * @return Maximum TX power in deci dBm.
 */
zpal_tx_power_decidbm_t zpal_radio_get_maximum_lr_tx_power(void);

/**
 * @brief Disable the radio debug mode.
 *
 * This function disables the radio debug mode
 *
 */
zpal_status_t zpal_radio_disable_debug(void);

/**
 * @brief a getter on the current rf profile.
 * Function return a pointer (instead of a struct) to reduce RAM memory usage and execution time.
 * The pointer target a const structure because it should not be used to modify the content of the
 * structure.
 *
 * @return pointer on the current rf profile.
 */
const zpal_radio_profile_t * zpal_radio_get_rf_profile(void);

/**
 * @brief Function to read current Long Range Channel Configuration.
 *
 * @return current long rang channel configuration (ZPAL_RADIO_LR_Ch_CFG if region without lr)
 */
zpal_radio_lr_channel_config_t zpal_radio_get_lr_channel_config(void);

/**
 * @brief Function to read current Primary Long Range Channel.
 *
 * @return @ref ZPAL_RADIO_LR_CHANNEL_A or @ref ZPAL_RADIO_LR_CHANNEL_B.
 */
zpal_radio_lr_channel_t zpal_radio_get_primary_long_range_channel(void);

/**
 * @brief Function to set the Primary Long Range Channel.
 *
 * @param[in] channel  @ref zpal_radio_lr_channel_t Long Range Channel to
 *                     set as Primary Long Range Channel.
 */
void zpal_radio_set_primary_long_range_channel(zpal_radio_lr_channel_t channel);

/**
 * @brief Function to read current Long Range Channel selection mode.
 *
 * @return true for automatically slected channel, false for manually selected channel
 */
bool zpal_radio_get_long_range_channel_auto_mode(void);

/**
 * @brief Function to set the Long Range Channel selction mode.
 *
 * @param[in] enable  true to enable the automatically channel selection mode,
 *                    false to enable the manual channel selection mode
 * @return @ref ZPAL_STATUS_OK if the mode was successfully set, @ref ZPAL_STATUS_FAIL otherwise.
 */
zpal_status_t zpal_radio_set_long_range_channel_auto_mode(bool enable);

/**
 * @brief Function to check if the stack implementation supports a given region
 *
 * @param[in] region  Region to check
 *
 * @return  True if the region is supported, False if it is not
 */
bool zpal_radio_is_region_supported(zpal_radio_region_t region);

/**
 * @brief Check if transmission is allowed for specified channel.
 *
 * @param[in] channel         The channel to check.
 * @param[in] frame_length    The length of the frame to send.
 * @param[in] cca             If true, the function will provision cca_duration into transmission time.
 * @return True if node shall use Long Range channel only.
 *
 */
bool zpal_radio_is_transmit_allowed(uint8_t channel, uint8_t frame_length, bool cca);

/**
 * @brief Function to reduce Tx power of classic non-listening devices.
 *
 * @param[in] adjust_tx_power  Reduces the devices default Tx power in dB. Valid range: 0-9 dB.
 *
 * @return  True when reduction is allowed, false for listening devices and out of range input.
 */
bool zpal_radio_attenuate(zpal_radio_tx_power_t adjust_tx_power);

/**
 * @brief Return the maximum board supported tx power for classic z-wave.
 *
 * @return The maximum board supported tx power in deci dBm.
 */
zpal_tx_power_decidbm_t zpal_radio_get_maximum_tx_power(void);

/**
 * @brief Function to adjust the requested tx power for Long Range.
 * This function ensures that the requested power is within the allowed range
 *
 * @param[in] requested_power  The requested tx power in deci dBm.
 *
 * @return The adjusted tx power in deci dBm.
 */
zpal_tx_power_decidbm_t zpal_radio_limit_lr_power_to_capability(zpal_tx_power_decidbm_t requested_power);

/**
 * @brief Function to radio calibration.
 *
 * @param[in] forced  If true, radio calibration is performed regardless if it is required.
 *                    If false, radio calibration is performed only if it is required.
 */
void zpal_radio_request_calibration(bool forced);

/**
 * @brief Retrieves information about the last received beam.
 *
 * @param[out] beamInfo Pointer to the structure where the beam information will be stored.
 * @return @ref ZPAL_STATUS_OK if the beam information was successfully retrieved,
 *         @ref ZPAL_STATUS_INVALID_ARGUMENT if beamInfo is NULL.
 */
zpal_status_t zpal_radio_get_last_beam_info(zpal_radio_beam_info_t * beamInfo);

/**
 * @brief Stores the average RSSI (Received Signal Strength Indicator) for a given Z-Wave channel.
 *
 * This function stores the background RSSI average for the specified Z-Wave channel.
 *
 * @param zwavechannel The Z-Wave channel for which the RSSI average is to be calculated.
 * @param averagerssi The average RSSI value to be used for the calculation.
 */
void zpal_radio_rf_channel_statistic_store_background_rssi_average(zpal_radio_zwave_channel_t zwavechannel, int8_t averagerssi);

/**
 * @brief Requests the radio to stay awake for a specified number of milliseconds.
 *
 * @param msecs Number of millisecs to stay awake (ZPAL_RADIO_STAY_AWAKE_ALWAYS = never ends).
 * @param id    Pointer to a variable where the identifier for the stay-awake request will be stored.
 *              This identifier is used to revoke the request later. NULL value is accepted, but it won't be
 *              possible to revoke the request later.
 * @return @ref ZPAL_STATUS_OK if the request was successful, @ref ZPAL_STATUS_INVALID_ARGUMENT if the duration
 *         exceeds the maximum supported value or another error occurs.
 *
 * @note **Duration Limit**: Maximum duration is limited by INT32_MAX ticks in the underlying sleeptimer.
 *       The actual maximum milliseconds depends on the platform's sleeptimer frequency. At the default
 *       32768 Hz, this is approximately 18.2 hours (1,092,266 ms). Requests exceeding this limit will fail.
 *       For indefinite duration, use ZPAL_RADIO_STAY_AWAKE_ALWAYS instead.
 * @note **Timer Synchronization**: This API uses platform-specific timers (hardware-based on Silicon Labs).
 *       When combining with OS-level timers (e.g., FreeRTOS ctimer), ensure stay_awake duration exceeds
 *       the OS timeout to account for scheduler latency. Example: if ctimer fires at 240ms, set stay_awake
 *       to 440ms (240 + 200ms margin). This prevents device to shutoff radio and eventually sleep before
 *       callback execution on low-power devices.
 */
zpal_status_t zpal_radio_request_stay_awake(uint32_t msecs, zpal_radio_stay_awake_id_t *id);

/**
 * @brief Revokes a previous stay-awake request, allowing the radio to sleep as normal.
 *
 * @param id Pointer to the identifier of the stay-awake request to revoke.
 *           NULL is used as a special value to revoke permanent stay-awake requests.
 *
 * @note The pointer to the identifier is set to 0 after revocation.
 * @note Radio will transition to off or flirs mode if there are no other active stay-awake requests.
 */
zpal_status_t zpal_radio_revoke_stay_awake(zpal_radio_stay_awake_id_t *id);

/**
 * @brief Lengthens the current stay-awake period.
 *
 * @param id Pointer to the identifier of the stay-awake request to update.
 * @param new_msecs The new length (in millisecs) to request from now.
 * @return @ref ZPAL_STATUS_OK if the request was successful, @ref ZPAL_STATUS_INVALID_ARGUMENT if the duration
 *         exceeds the maximum supported value or another error occurs.
 *
 * @note **Duration Limit**: Maximum duration is limited by INT32_MAX ticks in the underlying sleeptimer.
 *       See zpal_radio_request_stay_awake() for details.
 * @note This function is equivalent to revoking the previous request and creating a new one.
 */
zpal_status_t zpal_radio_update_stay_awake(zpal_radio_stay_awake_id_t *id, uint32_t new_msecs);

typedef enum {
  ZPAL_RADIO_STATUS_OFF = 0,           ///< Radio is off
  ZPAL_RADIO_STATUS_FLIRS,             ///< Radio is in Beam receive mode
  ZPAL_RADIO_STATUS_ON                 ///< Radio is in normal operating mode
} zpal_radio_status_t;

/**
 * @brief Getter of wakeup state of the radio
 * @return Current wakeup state of the radio
 */
zpal_radio_status_t zpal_radio_get_wakeup_status(void);

typedef void (*zpal_radio_status_callback_t)(const zpal_radio_status_t state);

/**
 * @brief Registers a callback function to be invoked on radio state changes.
 *
 * This function sets the callback that will be called whenever the radio state changes.
 * The callback function should match the signature defined by zpal_radio_state_callback_t.
 *
 * @param[in] callback  The function pointer to the callback to be registered.
 *                      Pass NULL to unregister the current callback.
 *
 * @return zpal_status_t Returns status code indicating success or failure of the operation.
 */
zpal_status_t zpal_radio_set_status_callback(zpal_radio_status_callback_t callback);

/**
 * @brief Inform the ZPAL radio layer that the node has been correctly included in a network.
 *
 * Correctly included means that the node is joinable with tuple (homeid, nodeid).
 *
 * @param[in] is_joinable true if the node has been correctly included, false otherwise.
 *
 * @return zpal_status_t Returns status code indicating success or failure of the operation.
 * @ref ZPAL_STATUS_OK if the operation was successful.
 */
zpal_status_t zpal_radio_set_joinable(bool is_joinable);

/**
 * @} //zpal-radio
 * @} //zpal
 */

#ifdef __cplusplus
}
#endif

#endif /* ZPAL_RADIO_H_ */
