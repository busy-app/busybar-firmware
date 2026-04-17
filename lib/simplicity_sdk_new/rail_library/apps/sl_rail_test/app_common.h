/***************************************************************************//**
 * @file
 * @brief This header file defines variables to be shared between the main
 *   test application and customer specific sections.
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

#ifndef __APPS_COMMON_H__
#define __APPS_COMMON_H__

// This header will always be included. If CLI support is disabled, all CLI
// functions will be compiled successfully and then be deadstripped from code.
#include "sl_cli.h"
#include "em_device.h"
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif
#include "sl_gpio.h"

#if defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#include "sl_clock_manager.h"
#else
#include "em_cmu.h"
#endif

#ifdef _SILICON_LABS_32B_SERIES_2
#include "em_system.h"
#include "em_prs.h"
#include "em_emu.h"
#include "em_ldma.h"
#else
#include "sl_hal_prs.h"
#include "sl_hal_emu.h"
#include "sl_hal_ldma.h"
#include "sl_device_peripheral.h"
#include "sl_hal_system.h"
#endif

#include "sl_memory_manager.h"
#include "response_print.h"
#include "sl_rail.h"
#include "sl_rail_ble.h"
#include "sl_rail_zwave.h"
#include SL_RAIL_UTIL_PA_PLUGIN_HEADER
#include "sl_rail_util_init_inst0_config.h"

#include "sl_rail_test_config.h"
#if SL_RAIL_UTIL_INIT_RADIO_CONFIG_SUPPORT_INST0_ENABLE
#include "rail_config.h" // If compilation fails here, check the radio configurator output
#endif

#if defined(SL_CATALOG_IOSTREAM_USART_PRESENT)
  #include "sl_iostream_usart_vcom_config.h"
#endif

#if defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)
  #include "sl_iostream_eusart_vcom_config.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
 #include "railtest_response_print_mapping.h"
 #include "app_task_init.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/
#if defined(SL_CATALOG_KERNEL_PRESENT)
  #define RAILTEST_PRINTF(...) \
  do {                         \
    railapp_mutex_acquire();   \
    if (printingEnabled) {     \
      printf(__VA_ARGS__);     \
    }                          \
    railapp_mutex_release();   \
  } while (false)
#else
  #define RAILTEST_PRINTF(...) \
  do {                         \
    if (printingEnabled) {     \
      printf(__VA_ARGS__);     \
    }                          \
  } while (false)
#endif

#ifdef RAIL_MULTIPROTOCOL
#define CHECK_RAIL_HANDLE(command)   \
  do {                               \
    if (!checkRailHandle(command)) { \
      return;                        \
    }                                \
                                     \
  } while (0)
#else //RAIL_MULTIPROTOCOL
#define CHECK_RAIL_HANDLE(command) //no-op
#endif //RAIL_MULTIPROTOCOL

/******************************************************************************
 * Constants
 *****************************************************************************/
#define COUNTOF(a) (sizeof(a) / sizeof(a[0]))
#define TX_CONTINUOUS_COUNT (0xFFFFFFFF)

#define RAILTEST_EVENT_STRINGS                                          \
  {                                                                     \
    "RSSI_AVERAGE_DONE",                                                \
    "RX_ACK_TIMEOUT",                                                   \
    "RX_FIFO_ALMOST_FULL",                                              \
    "RX_PACKET_RECEIVED",                                               \
    "RX_PREAMBLE_LOST",                                                 \
    "RX_PREAMBLE_DETECT",                                               \
    "RX_SYNC1_DETECT",                                                  \
    "RX_SYNC2_DETECT",                                                  \
    "RX_FRAME_ERROR",                                                   \
    "RX_FIFO_FULL",                                                     \
    "RX_FIFO_OVERFLOW",                                                 \
    "RX_ADDRESS_FILTERED",                                              \
    "RX_TIMEOUT",                                                       \
    "SCHEDULED_RX/TX_STARTED",                                          \
    "RX_SCHEDULED_RX_END",                                              \
    "RX_SCHEDULED_RX_MISSED",                                           \
    "RX_PACKET_ABORTED",                                                \
    "RX_FILTER_PASSED",                                                 \
    "RX_TIMING_LOST",                                                   \
    "RX_TIMING_DETECT",                                                 \
    "RX_CHANNEL_HOPPING_COMPLETE/RX_DUTY_CYCLE_RX_END",                 \
    "IEEE802154_DATA/ZWAVE_LR_ACK/_REQUEST_COMMAND/TX_MFM_BUFFER_DONE", \
    "ZWAVE_BEAM",                                                       \
    "TX_FIFO_ALMOST_EMPTY",                                             \
    "TX_PACKET_SENT",                                                   \
    "TXACK_PACKET_SENT",                                                \
    "TX_ABORTED",                                                       \
    "TXACK_ABORTED",                                                    \
    "TX_BLOCKED",                                                       \
    "TXACK_BLOCKED",                                                    \
    "TX_UNDERFLOW",                                                     \
    "TXACK_UNDERFLOW",                                                  \
    "TX_CHANNEL_CLEAR",                                                 \
    "TX_CHANNEL_BUSY",                                                  \
    "TX_CCA_RETRY",                                                     \
    "TX_START_CCA",                                                     \
    "TX_STARTED",                                                       \
    "TX_SCHEDULED_TX_MISSED",                                           \
    "CONFIG_UNSCHEDULED",                                               \
    "CONFIG_SCHEDULED",                                                 \
    "SCHEDULED_STATUS",                                                 \
    "CAL_NEEDED",                                                       \
    "RF_SENSED",                                                        \
    "PA_PROTECTION",                                                    \
    "SIGNAL_DETECTED",                                                  \
    "IEEE802154_MODE_SWITCH_START",                                     \
    "IEEE802154_MODE_SWITCH_END",                                       \
    "DETECT_RSSI_THRESHOLD",                                            \
    "THERMISTOR_DONE",                                                  \
    "RESERVED_49",                                                      \
    "TEMPERATURE_TOO_HOT",                                              \
    "TEMPERATURE_COOL_DOWN",                                            \
    "USER_MBOX",                                                        \
    "TX_CCA_ACTIVATED",                                                 \
  }

// Since channel hopping is pretty space intensive, put some limitations on it
// 125 32 bit words per channel should be plenty
// MAX_NUMBER_CHANNELS can generally be safely increased if more channels
// are needed for a channel hopping sequencer, the only limit being chip
// flash size
#if (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
#define MAX_NUMBER_CHANNELS 6 // up to 4 (9.6, 40, 100, LR) + 2 for conc. autoack (9.6 and 40)
#if defined (RAIL_MULTIPROTOCOL) && defined (_SILICON_LABS_32B_SERIES_2_CONFIG_2)
#define CHANNEL_HOPPING_BUFFER_WORDS (400U)
#else
#define CHANNEL_HOPPING_BUFFER_WORDS (1050U)
#endif
#else
#define MAX_NUMBER_CHANNELS 4
#define CHANNEL_HOPPING_BUFFER_WORDS (200U * MAX_NUMBER_CHANNELS)
#endif
#if SL_RAIL_SUPPORTS_OFDM_PA
#define OFDM_MCS_COUNT (8U)
#define SUBPHYID_COUNT (OFDM_MCS_COUNT)
#else //SL_RAIL_SUPPORTS_OFDM_PA
#define SUBPHYID_COUNT (SL_RAIL_BLE_RX_SUBPHY_COUNT)
#endif //SL_RAIL_SUPPORTS_OFDM_PA

// Define generic VCOM RX and TX port-pin definitions to use either the USART
// or the EUSART pins.
#ifdef SL_CATALOG_IOSTREAM_USART_PRESENT
#define VCOM_RX_PORT SL_IOSTREAM_USART_VCOM_RX_PORT
#define VCOM_RX_PIN SL_IOSTREAM_USART_VCOM_RX_PIN
#define VCOM_TX_PORT SL_IOSTREAM_USART_VCOM_TX_PORT
#define VCOM_TX_PIN SL_IOSTREAM_USART_VCOM_TX_PIN
#elif defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)
#define VCOM_RX_PORT SL_IOSTREAM_EUSART_VCOM_RX_PORT
#define VCOM_RX_PIN SL_IOSTREAM_EUSART_VCOM_RX_PIN
#define VCOM_TX_PORT SL_IOSTREAM_EUSART_VCOM_TX_PORT
#define VCOM_TX_PIN SL_IOSTREAM_EUSART_VCOM_TX_PIN
#else
// No VCOM (no CLI)
#endif

extern uint32_t channelHoppingBufferSpace[CHANNEL_HOPPING_BUFFER_WORDS];

/******************************************************************************
 * Variable Export
 *****************************************************************************/

typedef struct PhySwitchToRx{
  uint8_t extraDelayUs;
  uint8_t iterations;
  sl_rail_ble_phy_t phy;
  uint16_t physicalChannel;
  uint32_t timeDelta;
  sl_rail_ble_state_t params;
} PhySwitchToRx_t;

/**
 * @enum RailTxType
 * @brief Enumeration of the types of tx available in RAIL
 *
 * These are used to decide which type of tx to do, based on
 * what's been configured in sl_rail_test. Scheduled is not included
 * as sl_rail_test handles it somewhat separately.
 */

typedef enum RailTxType {
  TX_TYPE_NORMAL,
  TX_TYPE_CSMA,
  TX_TYPE_LBT,
} RailTxType_t;

typedef struct ButtonArray {
  sl_gpio_port_t   port;
  unsigned int        pin;
} ButtonArray_t;

typedef enum RailAppEventType {
  RAIL_EVENT,
  RX_PACKET,
  BEAM_PACKET,
  MULTITIMER,
  AVERAGE_RSSI,
  MOCK_SCHEDULER_TASK,
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
  MODE_SWITCH_CHANGE_CHANNEL,
#endif
} RailAppEventType_t;

typedef enum RailRfSenseMode {
  RFSENSE_MODE_OFF,
  RFSENSE_MODE_ENERGY_DETECTION,
  RFSENSE_MODE_SELECTIVE_OOK,
} RailRfSenseMode_t;

typedef enum bleCsEvent {
  BLE_CS_PACKET_RECEIVED,
  BLE_CS_PACKET_SENT,
  BLE_CS_PACKET_RX_TIMEOUT,
} bleCsEvent_t;

#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
typedef enum ModeSwitchState {
  IDLE,               /* Not in mode switch*/
  TX_MS_PACKET,       /* TX device is sending MS packet*/
  TX_ON_NEW_PHY,      /* TX device is on new PHY during mode switch sequence*/
  TX_STAY_ON_NEW_PHY, /* TX device is on new PHY after mode switch sequence*/
  RX_ON_NEW_PHY,      /* RX device is on new PHY*/
} ModeSwitchState_t;
#endif

typedef struct RxPacketData {
  /**
   * A structure containing the extra information associated with this received
   * packet.
   */
  sl_rail_rx_packet_details_t appendedInfo;
  /**
   * The railHandle on which this packet was received.
   */
  sl_rail_handle_t railHandle;
  /**
   * A pointer to a buffer that holds receive packet data bytes.
   */
  uint8_t *dataPtr;
  /**
   * The number of bytes that are in the dataPtr array.
   */
  uint16_t dataLength;
  /**
   * The packet's frequency offset
   */
  sl_rail_frequency_offset_t freqOffset;
  /**
   * The packet's status
   */
  sl_rail_rx_packet_status_t packetStatus;
  /**
   * A bitmask representing which address filter(s) this packet
   * has passed.
   */
  sl_rail_addr_filter_mask_t filterMask;
  /**
   * Flag indicating if packet was held for later processing.
   */
  bool packetHeld;
} RxPacketData_t;

typedef struct RailEvent {
  // Use uint32_t array for events rather than sl_rail_events_t to reduce
  // alignment requirement and hence reduce sizeof(RailEvent_t).
  uint32_t events[sizeof(sl_rail_events_t) / sizeof(uint32_t)];
  uint32_t timestamp;
  sl_rail_handle_t handle;
  uint32_t parameter; /**< This field is open to interpretation based on the event type.
                         It may hold information related to the event e.g. status. */
} RailEvent_t;

typedef struct Multitimer {
  sl_rail_time_t currentTime;
  sl_rail_time_t expirationTime;
  uint32_t index;
} Multitimer_t;

typedef struct AverageRssi {
  int16_t rssi;
} AverageRssi_t;

#ifdef SL_CATALOG_MOCK_SCHEDULER_PRESENT
typedef struct MockSchedulerTask {
  uint8_t notificationType; //sl_radio_scheduler_notification_type_t
  uint32_t taskId;
} MockSchedulerTask_t;
#endif

#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
typedef struct ModeSwitchChangeChannel {
  uint16_t channel;
} ModeSwitchChangeChannel_t;
#endif

typedef void * QueueEntryPtr_t;

typedef struct RailAppEvent {
  QueueEntryPtr_t next; // linked list chain, must be first field
  RailAppEventType_t type;
  union {
    RxPacketData_t rxPacket;
    sl_rail_zwave_beam_details_t beamPacket;
    RailEvent_t railEvent;
    Multitimer_t multitimer;
    AverageRssi_t rssi;
#ifdef SL_CATALOG_MOCK_SCHEDULER_PRESENT
    MockSchedulerTask_t mockSchedulerTask;
#endif
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
    ModeSwitchChangeChannel_t modeSwitchChangeChannel;
#endif
  };
} RailAppEvent_t;

typedef struct Stats{
  uint32_t samples;
  int32_t min;
  int32_t max;
  float mean;
  float varianceTimesSamples;
} Stats_t;

typedef struct Counters{
  // Counts all successful user transmits
  // "user" in this and following variable names refers to
  // a transmit that a user initiated, i.e. not an ack
  uint32_t userTx;
  // Counts all successful ack transmits
  uint32_t ackTx;
  uint32_t userTxAborted;
  uint32_t ackTxAborted;
  uint32_t userTxBlocked;
  uint32_t ackTxBlocked;
  uint32_t userTxUnderflow;
  uint32_t ackTxUnderflow;
  uint32_t ackTxFpSet;
  uint32_t ackTxFpFail;
  uint32_t ackTxFpAddrFail;
  // Counts all users transmits that get on-air (when TX_STARTED event enabled)
  uint32_t userTxStarted;
  uint32_t userTxRemainingErrors;

  // Channel busy doesn't differentiate
  // between ack/user packets
  uint32_t txChannelBusy;

  uint32_t receive;
  uint32_t receiveCrcErrDrop;
  uint32_t syncDetect;
  uint32_t syncDetect0;
  uint32_t syncDetect1;
  uint32_t preambleLost;
  uint32_t preambleDetect;
  uint32_t frameError;
  uint32_t rxOfEvent;
  uint32_t addrFilterEvent;
  uint32_t rxFail;
  uint32_t calibrations;
  uint32_t noRxBuffer;
  uint32_t rfSensedEvent;
  uint32_t perTriggers;
  uint32_t ackTimeout;
  uint32_t rxTimeout;
  uint32_t rxBlocked;
  uint32_t lbtSuccess;
  uint32_t lbtRetry;
  uint32_t lbtNextCcaNow;
  uint32_t lbtStartCca;
  uint32_t lbtCcaActivated;
  uint32_t txFifoAlmostEmpty;
  uint32_t rxFifoAlmostFull;
  uint32_t rxFifoFull;
  uint32_t timingLost;
  uint32_t timingDetect;
  uint32_t radioConfigChanged;
  uint32_t rxBeams;
  uint32_t dataRequests;
  Stats_t rssi;
  uint32_t paProtect;
  uint32_t subPhyCount[SUBPHYID_COUNT];
  uint64_t rxRawSourceBytes;
} Counters_t;

typedef sl_rail_status_t (*TxTimestampFunc)(sl_rail_handle_t, sl_rail_tx_packet_details_t *);
typedef sl_rail_status_t (*RxTimestampFunc)(sl_rail_handle_t, sl_rail_rx_packet_details_t *);

extern const char * const eventNames[];
extern const uint8_t numRailEvents;
extern bool printingEnabled;
extern PhySwitchToRx_t phySwitchToRx;
extern Counters_t counters;
extern int currentConfig;
extern bool receiveModeEnabled;
extern sl_rail_radio_state_t rxSuccessTransition;
extern bool transmitting;
extern bool txParameterChanged;
extern uint16_t channel;
extern uint8_t configIndex;
extern uint32_t continuousTransferPeriod;
extern bool enableRandomTxDelay;
extern int32_t txCount;
extern int32_t txRepeatCount;
// Remap sl_rail_test usage of sl_rail_idle() to clear txRepeatCount
static inline sl_rail_status_t railtest_idle(sl_rail_handle_t rail_handle, sl_rail_idle_mode_t mode, bool wait)
{
  txRepeatCount = 0;
  return sl_rail_idle(rail_handle, mode, wait);
}
#undef  sl_rail_idle
#define sl_rail_idle railtest_idle
extern int32_t txRemainingCount;
extern uint32_t txAfterRxDelay;
extern int32_t txCancelDelay;
extern sl_rail_stop_mode_t txCancelMode;
extern sl_rail_channel_config_entry_t channels[];
extern const sl_rail_channel_config_t channelConfig;
extern bool skipCalibrations;
extern volatile bool serEvent;
extern volatile bool rxPacketEvent;
extern uint32_t perCount;
extern uint32_t perDelay;
extern uint32_t rxOverflowDelay;
extern uint32_t dataReqLatencyUs;
extern bool afterRxCancelAck;
extern bool afterRxUseTxBufferForAck;
extern volatile bool newTxError;
extern volatile uint32_t failPackets;
extern sl_rail_events_t enablePrintEvents;
extern bool printRxErrorPackets;
extern bool printRxFreqOffsetData;
extern uint32_t internalTransmitCounter;
extern const char buildDateTime[];
extern bool ieee802154EnhAckEnabled;
extern uint8_t ieee802154PhrLen; // 15.4 PHY Header Length (1 or 2 bytes)
extern TxTimestampFunc txTimePosition;
extern RxTimestampFunc rxTimePosition;
extern sl_rail_stream_mode_t streamMode;
extern bool rxHeld;
extern volatile bool rxProcessHeld;
extern volatile uint32_t packetsHeld;
extern bool reproFifoAlmostFullBug;
extern bool txAckDirect;
extern sl_rail_time_t txStartTime;
extern sl_rail_time_t txScheduledTime;
#if defined(SL_CATALOG_KERNEL_PRESENT)
// Mutex params for FreeRTOS
extern osMutexId_t railapp_mtx;
extern osMutexAttr_t railapp_mtx_attr;
#endif

void setTxRandomHelper(void); // Extern payload randomizer

#ifdef SL_RAIL_UTIL_IC_SIMULATION_BUILD
#define PERIPHERAL_ENABLE (0x00)
#define ASYNC_RESPONSE (0x00)
#else
#define PERIPHERAL_ENABLE (0x01)
#define ASYNC_RESPONSE (0x02)
#endif

#define RX_DATA_SOURCE_EVENT_STATE_CHECKED   0
#define RX_DATA_SOURCE_EVENT_STATE_OCCURRED  1 // .. up to _SUSPENDED
#define RX_DATA_SOURCE_EVENT_STATE_SUSPENDED 1000
extern volatile uint16_t rxDataSourceEventState;

extern uint8_t logLevel;
extern uint8_t txData[SL_RAIL_TEST_MAX_PACKET_LENGTH];
extern uint16_t txDataLen;
extern uint16_t offsetInTxFifo;

#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
extern uint8_t txData_2B[2];
extern uint8_t txCountAfterModeSwitch;
extern uint16_t modeSwitchNewChannel;
extern uint16_t modeSwitchBaseChannel;
extern ModeSwitchState_t modeSwitchState;
extern uint32_t modeSwitchDelayUs; // Delay in microseconds before switching back to base PHY after all packets have been transmitted on the new PHY
#define RX_MODE_SWITCH_DELAY_US (2000) // Default delay on RX side in microseconds
extern sl_rail_multi_timer_t modeSwitchMultiTimer;
extern uint32_t modeSwitchSequenceIterations;
extern uint32_t modeSwitchSequenceId;
extern uint8_t MSphr[2];
extern uint8_t txCountAfterModeSwitchId;
extern bool modeSwitchLifeReturn;
#endif

extern uint8_t ackData[SL_RAIL_DEFAULT_AUTO_ACK_FIFO_BYTES];
extern uint8_t ackDataLen;

extern RailTxType_t txType;
extern sl_rail_lbt_config_t *lbtConfig;
extern sl_rail_csma_config_t *csmaConfig;

// Structure that holds txOptions
extern sl_rail_tx_options_t txOptions;

// Structure that holds Antenna Options
extern sl_rail_tx_options_t antOptions;

// Structure that holds concurrent PHY ID Options
extern sl_rail_tx_options_t concPhyIdOptions;

// Structure that holds (default) rxOptions
extern sl_rail_rx_options_t rxOptions;

// Data Management
typedef struct Queue {
  QueueEntryPtr_t *head;
  QueueEntryPtr_t *tail;
} Queue_t;
extern Queue_t railAppEventQueue;
extern volatile uint32_t eventsMissed;
extern sl_rail_tx_data_config_t railTxDataConfig;
extern sl_rail_rx_data_config_t railRxDataConfig;

// Fifo mode Test bits
extern bool rxFifoManual;
extern bool txFifoManual;

// RAIL instance handle
extern sl_rail_handle_t railHandle;

// Indicates whether to print tx acks as they happen
extern bool printTxAck;

// Strings representing the possible PA selections
extern const char * const paStrings[];
extern const char * const paStrings2x[]; // For RAIL 2.x compat

// Variable containing current RSSI
extern float averageRssi;

// Channel Hopping configuration structures
extern uint32_t* channelHoppingBuffer;

// Variable containing current receive frequency offset
extern sl_rail_frequency_offset_t rxFreqOffset;

// Variables tracking the compensation mode
extern bool isHFXOCompensationSystematic;

// Variable for enabling setting up the TX packet in the host side on RX to TX autotransitions
extern bool fillTxPacketOnRx;

/**
 * @enum AppMode
 * @brief Enumeration of sl_rail_test transmit states.
 */
typedef enum AppMode{
  NONE = 0,           /**< sl_rail_test is not doing anything special */
  TX_STREAM = 1,      /**< Send a stream of pseudo-random bits */
  TX_CONTINUOUS = 3,  /**< Send an unending stream of packets*/
  DIRECT = 4,         /**< Send data to and from a GPIO, without any packet handling */
  TX_N_PACKETS = 5,   /**< Send a specific number of packets */
  TX_SCHEDULED = 6,   /**< Send one packet scheduled in the future */
  SCHTX_AFTER_RX = 7, /**< Schedule a TX for a fixed delay after receiving a packet */
  RX_OVERFLOW = 8,    /**< Cause overflow on receive */
  TX_UNDERFLOW = 9,   /**< Cause underflows on the next TX sequence */
  TX_CANCEL = 10,     /**< Cancel a single packet transmit to force an error event */
  RF_SENSE = 11,      /**< Sense RF energy to wake the radio */
  PER = 12,           /**< Packet Error Rate test mode */
  BER = 13,           /**< Bit Error Rate test mode */
  RX_SCHEDULED = 14,  /**< Enable receive at a time scheduled in the future */
  TX_SCHEDULED_N_PACKETS = 15, /**< Schedule a TX for a specific number of packets */
} AppMode_t;

void sl_rail_test_internal_app_init(void);
void sl_rail_test_internal_app_process_action(void);

void railtest_TimerExpired(sl_rail_handle_t railHandle);
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH && defined(WISUN_MODESWITCHPHRS_ARRAY_SIZE)
void railtest_ModeSwitchMultiTimerExpired(sl_rail_multi_timer_t *tmr,
                                          sl_rail_time_t expectedTimeOfEvent,
                                          void *cbArg);
void restartModeSwitchSequence(bool applyDelay);
void endModeSwitchSequence(void);
void scheduleNextModeSwitchTx(bool MSPktSent);
#endif
void railtest_SwTimerExpired(sl_rail_handle_t railHandle);
AppMode_t previousAppMode(void);
AppMode_t currentAppMode(void);
void enableAppMode(AppMode_t appMode, bool enable, char *command);
bool enableAppModeSync(AppMode_t appMode, bool enable, char *command);
void setNextAppMode(AppMode_t appMode, char *command);
void changeAppModeIfPending(void);
const char *appModeNames(AppMode_t appMode);
bool inAppMode(AppMode_t appMode, char *command);
bool inRadioState(sl_rail_radio_state_t state, char *command);
bool parseTimeModeFromString(char *str, sl_rail_time_mode_t *mode);
const char *configuredRxAntenna(sl_rail_rx_options_t rxOptions);

void updateStats(int32_t newValue, Stats_t *stats);
void rfSensedCheck(void);

uint8_t *getTxFifoBaseAddress(void);

sl_rail_frequency_offset_t getRxFreqOffset(void);
void changeChannel(uint32_t i);
void pendPacketTx(void);
sl_rail_rx_packet_handle_t processRxPacket(sl_rail_handle_t railHandle,
                                           sl_rail_rx_packet_handle_t packetHandle,
                                           bool heldPacket);
void pendFinishTxSequence(void);
void pendFinishTxAckSequence(void);
void radioTransmit(uint32_t iterations, char *command);
void radioTransmitGeneric(uint32_t iterations, bool scheduled, char *command);
void configureTxAfterRx(uint32_t delay, bool enable, char *command);
void scheduleNextTx(void);
void printPacket(char *cmdName,
                 uint8_t *data,
                 uint16_t dataLength,
                 RxPacketData_t *packetInfo);
void sendPacketIfPending(void);

void updateGraphics(void);
void enableGraphics(void);
void disableGraphics(void);

void initButtons(void);
void deinitButtons(void);

void LedSet(int led);
void LedToggle(int led);
void LedsDisable(void);

void appHalInit(void);
void PeripheralDisable(void);
void PeripheralEnable(void);
void usDelay(uint32_t microseconds);
void serialWaitForTxIdle(void);
bool queueAdd(Queue_t *queue, QueueEntryPtr_t *data);
QueueEntryPtr_t *queueRemove(Queue_t *queue);
bool queueIsEmpty(Queue_t *queue);
void enqueueEvents(sl_rail_events_t events);
void rxFifoPrep(void);
void printRailEvents(RailEvent_t *railEvent);
void printRailAppEvents(void);
sl_rail_status_t chooseTxType(void);
const char *getRfStateName(sl_rail_radio_state_t state);
char *getRfStateDetailName(sl_rail_radio_state_detail_t state, char *buffer);
const char *getStatusMessage(sl_rail_status_t status);
sl_rail_status_t disableIncompatibleProtocols(sl_rail_pti_protocol_t newProtocol);
bool checkRailHandle(char *command);
bool getRxDutyCycleSchedWakeupEnable(sl_rail_time_t *sleepInterval);
uint16_t getLikelyChannel(void);

void railtest_TxPacketSent(sl_rail_handle_t railHandle, bool isAck);
void railtest_RxPacketAborted(sl_rail_handle_t railHandle);
void railtest_RxPacketReceived(sl_rail_handle_t railHandle);
void railtest_RxTimeout(sl_rail_handle_t railHandle);
void railtest_RxBlocked(sl_rail_handle_t railHandle);
void railtest_TxFifoAlmostEmpty(sl_rail_handle_t railHandle);
void railtest_RxFifoAlmostFull(sl_rail_handle_t railHandle);
void railtest_RxChannelHoppingComplete(sl_rail_handle_t railHandle);
void railtest_IEEE802154_DataRequestCommand(sl_rail_handle_t railHandle);
void railtest_ZWAVE_BeamFrame(sl_rail_handle_t railHandle);
void railtest_ZWAVE_LrAckData(sl_rail_handle_t railHandle);

void printAddresses(sl_cli_command_arg_t *args);
void getAddressFilter(sl_cli_command_arg_t *args);
void printTxPacket(sl_cli_command_arg_t *args);
void resetCounters(sl_cli_command_arg_t *args);
void getPti(sl_cli_command_arg_t *args);
void setTxFifo(sl_cli_command_arg_t *args);
void printChipInfo(void);
#ifdef __cplusplus
}
#endif

#endif // __APPS_COMMON_H__
