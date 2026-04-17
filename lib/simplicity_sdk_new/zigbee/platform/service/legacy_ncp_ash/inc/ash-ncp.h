/***************************************************************************//**
 * @file
 * @brief Header for ASH Network Co-Processor
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef __ASH_NCP_H__
#define __ASH_NCP_H__

/**
 *  Allocates buffers held until ashReleaseStaticBuffers()
 *  is called.  Returns the head of the allocated buffer chain for use
 *  by the higher layer.
 *
 *  @return sli_buffer_manager_buffer_t
 */
sli_buffer_manager_buffer_t ashAllocateStaticBuffers(void);

/**
 *  Releases the buffers allocated by ashAllocateStaticBuffers()
 *  if none are in use.
 *
 *  @return bool value
 */
bool ashReleaseStaticBuffers(void);

/**
 *  Initializes the ASH protocol and serial port.
 */
void ashStart(void);

/**
 *  Sends a frame to the host.
 *
 * @param buffer: index of a linked buffer containing the DATA frame to send
 * @param highPriority: set true if the frame should be sent even if the host
 * is not ready.
 */
void ashSend(sli_buffer_manager_buffer_t buffer, bool highPriority);

/**
 *  Calls ASH protocol to manage transmission of DATA and
 * other frames to the host.
 */
void ashSendExec(void);

/**
 *  Returns a DATA frame, if one has been received.
 *
 * @param outBuf: pointer to a variable where the index of the linked buffers
 * containing a complete data frame will be written. The caller is responsible
 * for freeing these buffers when appropriate. If no frame is returned, the
 * index is set to SL_ZIGBEE_NULL_MESSAGE_BUFFER.
 *
 * @return  SL_ZIGBEE_EZSP_SUCCESS
 *          SL_ZIGBEE_EZSP_ASH_IN_PROGRESS
 *          SL_ZIGBEE_EZSP_NO_RX_DATA
 *          SL_ZIGBEE_EZSP_NO_RX_SPACE
 *          SL_ZIGBEE_EZSP_ASH_NCP_FATAL_ERROR
 */
sl_zigbee_ezsp_status_t ashReceive(sli_buffer_manager_buffer_t *outBuf);

/**
 *  Checks whether there is room in the serial output buffer.
 *
 * @return  SL_ZIGBEE_EZSP_SUCCESS
 *          SL_ZIGBEE_EZSP_NO_TX_SPACE
 */
sl_zigbee_ezsp_status_t ashSerialWriteAvailable(void);

/**
 *  Reads a byte from the serial port, if one is available.
 *
 * @param byte:   pointer to a variable where the byte read will be output
 *
 * @return  SL_ZIGBEE_EZSP_SUCCESS
 *          SL_ZIGBEE_EZSP_NO_RX_DATA
 */
sl_zigbee_ezsp_status_t ashSerialReadByte(uint8_t *byte);

/**
 *  Writes a byte to the serial output buffer.
 *
 * @param byte:   byte to write
 */
void ashSerialWriteByte(uint8_t byte);

#ifdef SLEEPY_EZSP_UART

/**
 *  Serial host is active.
 *
 * @return bool value
 */
bool serialHostIsActive(void);

/**
 *  Serial waiting for host to wake.
 *
 *	@return bool value
 */
bool serialWaitingForHostToWake(void);

/**
 *  Serial monitor Rxd.
 */
void serialMonitorRxd(uint8_t sleepMode);

/**
 *  Serial power down.
 */
void serialPowerDown(void);

/**
 *  Serial power up.
 */
void serialPowerUp(void);

/**
 *  Serial send awake signal.
 */
void serialSendAwakeSignal(void);

/**
 *  Serial inhibit callback Signal.
 */
void serialInhibitCallbackSignal(void);

#endif

/**
 *  Informs ASH whether or not cany callbacks are pending.
 *
 */
void ashPendingCallbacks(bool pending);

typedef struct
{ uint16_t
    baudRate,           ///< SerialBaudRate enum value
    traceFlags,         ///< trace output control bit flags (see defs below)
    unused0,            ///< (not used)
    txK,                ///< max frames that can be sent w/o being ACKed (1-7)
    randomize,          ///< enables randomizing DATA frame payloads
    ackTimeInit,        ///< adaptive rec'd ACK timeout initial value
    ackTimeMin,         ///<  "     "     "     "     "  minimum
    ackTimeMax,         ///<  "     "     "     "     "  maximum
    maxTimeouts,        ///< ACK timeouts needed to enter the ERROR state
    unused1,            ///< (not used)
    rebootDelay,        ///< reboot delay before sending RSTACK
    unused2,            ///< (not used)
    unused3,            ///< (not used)
    unused4,            ///< (not used)
    nrTime;             ///< time after which a rec'd nFlag expires
} sli_ash_ncp_config;

//ASH configuration manufacturing token storage locations
//              word
//             offset
//baudRate      0x80   6=9600, 8=19200, A=38400, C=57600, F=115200
//traceFlags    0x81
//unused0       0x82
//txK           0x83
//randomize     0x84
//ackTimeInit   0x85
//ackTimeMin    0x86
//ackTimeMax    0x87
//maxTimeouts   0x88
//unused1       0x89
//rebootDelay   0x8A
//unused2       0x8B
//unused3       0x8C
//unused4       0x8D
//nrTime        0x8E

#define INVALID_ASH_CONFIG_VALUE 0xFFFF

// Enum to identify sli_ash_ncp_config indices
typedef enum {
  ASH_CONFIG_BAUD_RATE = 0,
  ASH_CONFIG_TRACE_FLAGS,
  ASH_CONFIG_UNUSED0,
  ASH_CONFIG_TX_K,
  ASH_CONFIG_RANDOMIZE,
  ASH_CONFIG_ACK_TIME_INIT,
  ASH_CONFIG_ACK_TIME_MIN,
  ASH_CONFIG_ACK_TIME_MAX,
  ASH_CONFIG_MAX_TIMEOUTS,
  ASH_CONFIG_UNUSED1,
  ASH_CONFIG_REBOOT_DELAY,
  ASH_CONFIG_UNUSED2,
  ASH_CONFIG_UNUSED3,
  ASH_CONFIG_UNUSED4,
  ASH_CONFIG_NR_TIME,
  ASH_CONFIG_INDEX_MAX
} sli_ash_ncp_config_index;

// Do not include token manager headers for Zigbeed or simulation builds
#if defined(CORTEXM3)

#include "sl_token_manager_manufacturing.h"
#include "sl_token_manager_api.h"
#include "sl_token_manager_defines.h"

// Define the size of indexed token array (here for S3)
#ifndef MFG_ASH_CONFIG_ARRAY_SIZE
#define MFG_ASH_CONFIG_ARRAY_SIZE       20
#endif

/**
 * @brief Get the value of a specific member from an sli_ash_ncp_config struct
 *
 * @param config Pointer to the sli_ash_ncp_config struct
 * @param index The index to retrieve (use sli_ash_ncp_config_index enum)
 * @return uint16_t The value of the specified member, or 0xFFFF if invalid member
 */
tokTypeMfgAshConfig sli_ash_ncp_get_config(sli_ash_ncp_config_index index);

#else
// Define a dummy type for Zigbeed or simulation builds
typedef uint16_t tokTypeMfgAshConfig;
#endif // defined(CORTEXM3)

/**
 * @brief Get the value of a specific member from an sli_ash_ncp_config struct with default fallback
 *
 * @param config Pointer to the sli_ash_ncp_config struct
 * @param index The index to retrieve (use sli_ash_ncp_config_index enum)
 * @param defaultValue Default value to return if the member value is 0xFFFF
 * @return uint16_t The value of the specified member, or defaultValue if member is 0xFFFF
 */
tokTypeMfgAshConfig sli_ash_ncp_get_config_or_default(sli_ash_ncp_config_index index, tokTypeMfgAshConfig defaultValue);

#ifdef HAL_UARTNCP_BAUD_RATE
#define ASH_BAUD_RATE HAL_UARTNCP_BAUD_RATE
#else
// EMHAL-2285 if using hardware flow control we are using FIFO mode with DMA
// which can support baud 115200
#if (SL_LEGACY_SERIAL1_MODE == SL_LEGACY_SERIAL_FIFO) && defined(EMBER_SERIAL1_RTSCTS)
#define ASH_BAUD_RATE    BAUD_115200
#else
#define ASH_BAUD_RATE    BAUD_57600
#endif
#endif
// Enable VUART trace: B0: basic frame data, B1: verbose frame data, B2: events
#define ASH_TRACE_FLAGS           7
// Define sliding window size: the maximum number of unacknowledged frames
// that can be outstanding before blocking further DATA frame transmission.
// Note that it must be the case that 1 <= K <= 7.
#define ASH_TX_K                  5
// Enable randomizing DATA frame payloads
#define ASH_RANDOMIZE          true
// Adaptive timeout values for newly-transmitted DATA frames to be ACK'ed
// Units are milliseconds. Values must be < 8192 to avoid overflow.
#define ASH_TIME_DATA_INIT     1600
#define ASH_TIME_DATA_MIN       400
#define ASH_TIME_DATA_MAX      3200
// Consecutive ACK timeouts (minus 1) needed to enter the ERROR state.
// The value 0xFF means the NCP will not disconnect due to timeouts.
#define ASH_MAX_TIMEOUTS          3
// Delay before sending RSTACK, needed by some RS-232 to TTL converters.
#ifdef SLEEPY_EZSP_ASH
  #define ASH_REBOOT_DELAY        0
#else
  #define ASH_REBOOT_DELAY       1000
#endif
// Preload value for the NR timer used in flow control. Since this timer
// is only one byte, care must be taken to avoid overflow. Each tick is
// 16 milliseconds, and the largest acceptable value is 127*16 = 2032 ms.
#define ASH_NR_TIME            1000

// Bits in traceFlags
#define TRACE_VUART_FRAMES_BASIC    1
#define TRACE_VUART_FRAMES_VERBOSE  2
#define TRACE_VUART_EVENTS          4

#endif //__ASH_NCP_H__
