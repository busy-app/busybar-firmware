/***************************************************************************//**
 * @file
 * @brief  ASH protocol Network Co-Processor functions
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
#include PLATFORM_HEADER
#include <stddef.h>
#include "stack/include/sl_zigbee.h"
#include "stack/framework/zigbee_debug_channel.h"
#include "em2xx-reset-defs.h"
#include "hal/hal.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "serial/serial.h"

#include "ash-protocol.h"
#include "ash-ncp.h"
#include "ash-common.h"
#include "legacy_common_ash_config.h"
#if !defined(ZIGBEE_STACK_ON_HOST) && !defined(SL_ZIGBEE_TEST)
#include "sl_token_manufacturing_generic.h"
#endif // !defined(ZIGBEE_STACK_ON_HOST) && !defined(SL_ZIGBEE_TEST)
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
//------------------------------------------------------------------------------
// Preprocessor definitions

// Values for sendState
#define SEND_STATE_IDLE       0
#define SEND_STATE_SHFRAME    1
#define SEND_STATE_TX_DATA    2
#define SEND_STATE_RETX_DATA  3

// Bits in ashFlags
#define FLG_REJ               0x01          // Reject Condition
#define FLG_RETX              0x02          // Retransmit Condition
#define FLG_NAK               0x04          // send NAK
#define FLG_ACK               0x08          // send ACK
#define FLG_RSTACK            0x10          // send RSTACK
#define FLG_ERROR             0x20          // send ERROR
#define FLG_CONNECTED         0x40          // in CONNECTED state, else ERROR
#define FLG_CAN               0x80          // send immediate CAN
#ifdef SLEEPY_EZSP_ASH
  #define FLG_WAKE              0x100         // send WAKE
  #define FLG_CALLBACKS         0x200         // callbacks pending
#endif

// Values returned by ashFrameType()
#define TYPE_INVALID          0
#define TYPE_DATA             1
#define TYPE_ACK              2
#define TYPE_NAK              3
#define TYPE_RST              4

#define txControl       (txBuffer[0])       // more descriptive aliases
#define rxControl       (rxBuffer[0])
#define TX_BUFFER_LEN   ASH_NCP_SHFRAME_TX_LEN
#define RX_BUFFER_LEN   ASH_NCP_SHFRAME_RX_LEN

// Alllocate enough buffers to hold the largest EZSP frame.
// This linked buffer is a combined command input and response output buffer.
#define ASH_PERMANENT_BUFFERS \
  ((EZSP_MAX_FRAME_LENGTH + PACKET_BUFFER_SIZE - 1) / PACKET_BUFFER_SIZE)

// Pull the port information fom the config file
#define ASH_PORT LEGACY_NCP_ASH_SERIAL_PORT

// -1 is a sentinel value indicating automatic selection of port
// That means we try to use the VCOM instance of SL_IOSTREAM_USART when
// compiling for a chip and the hard coded value 1 when compiling for
// simulation or Unix
#if ASH_PORT == (-1)
// Avoid macro redefinition warning
#undef ASH_PORT
#if !defined(SL_ZIGBEE_TEST) && !defined(UNIX_HOST)
#ifdef SL_CATALOG_IOSTREAM_USART_PRESENT
#include "sl_iostream_usart_vcom_config.h"
#define ASH_PORT ((uint8_t)(COM_USART0) + SL_IOSTREAM_USART_VCOM_PERIPHERAL_NO)
#elif defined(SL_CATALOG_IOSTREAM_EUSART_PRESENT)
#include "sl_iostream_eusart_vcom_config.h"
#define ASH_PORT ((uint8_t)(COM_USART0) + SL_IOSTREAM_EUSART_VCOM_PERIPHERAL_NO)
#endif // SL_CATALOG_IOSTREAM_USART_PRESENT
#else
#define ASH_PORT 1
#endif // !defined(SL_ZIGBEE_TEST) && !defined(UNIX_HOST)
#endif // ASH_PORT == (-1)

// The ASH port is used by other pieces of the UART NCP code, so we provide
// access to the value in the form of the variable ashPort
const uint8_t ashPort = ASH_PORT;

// Delay in milliseconds between sending repeat ASH_WAKEs to wake the host
#define ASH_HOST_WAKE_REPEAT_DELAY  50

// define ASH_VUART_TRACE to output ASCII trace info to the vuart
// (This code uses about 400 words of flash on the XAP2b processor.)
//#define ASH_VUART_TRACE

// ASCII trace output function
//Uncomment the below and ASH_VUART_TRACE in order to get debug messages in Network Analyzer
//#define ashDebugPrintf(...) sli_legacy_serial_printf(0, __VA_ARGS__)

//#define DEBUG_ASSERT(x) assert(x)
#define DEBUG_ASSERT(x)

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

static uint8_t txBuffer[TX_BUFFER_LEN];       // outgoing short frames
static uint8_t rxBuffer[RX_BUFFER_LEN];       // incoming short frames
static uint8_t sendState;                     // ashSendExec() state variable
static sl_zigbee_ezsp_status_t ashError;                 // error or reset code
static uint8_t ackRx;                         // frame ack'ed from remote peer
static uint8_t ackTx;                         // frame ack'ed to remote peer
static uint8_t frmTx;                         // next frame to be transmitted
static uint8_t frmReTx;                       // next frame to be retransmitted
static uint8_t frmRx;                         // next frame expected to be rec'd
static uint8_t frmReTxHead;                   // frame at retx queue's head
static uint8_t ashTimeouts;                   // consecutive timeout counter
#ifdef SLEEPY_EZSP_UART
static uint16_t ashFlags;                       // bit flags for top-level logic
#else
static uint8_t ashFlags;                       // bit flags for top-level logic
#endif
static sli_buffer_manager_buffer_t rxDataBuffer;     // rec'd DATA frame linked buffer
static sli_buffer_manager_buffer_t rxFirstDataBuffer;// statically alloc'ed rx buffer
static uint8_t rxLen;                         // rec'd frame length

// Queues that interface to the EZSP application
static sli_zigbee_message_buffer_queue_t txHighQueue = { SL_ZIGBEE_NULL_MESSAGE_BUFFER };
static sli_zigbee_message_buffer_queue_t txQueue = { SL_ZIGBEE_NULL_MESSAGE_BUFFER };
static sli_zigbee_message_buffer_queue_t reTxQueue = { SL_ZIGBEE_NULL_MESSAGE_BUFFER };

static sli_buffer_manager_buffer_t sendBuffer;
#ifdef EM_BUFFER_USAGE
void sli_util_mark_ash_buffers(void)
{
  if (rxDataBuffer != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    sli_legacy_buffer_manager_buffer_usage[rxDataBuffer] |= BIT(EM_BUFFER_USAGE_Q_ASH);
  }
  if (sendState == SEND_STATE_TX_DATA
      || sendState == SEND_STATE_RETX_DATA) {
    sli_legacy_buffer_manager_buffer_usage[sendBuffer] |= BIT(EM_BUFFER_USAGE_Q_ASH);
  }
  WALK_QUEUE(&txHighQueue, finger,
             { sli_legacy_buffer_manager_buffer_usage[finger] |= BIT(EM_BUFFER_USAGE_Q_ASH); });
  WALK_QUEUE(&txQueue, finger,
             { sli_legacy_buffer_manager_buffer_usage[finger] |= BIT(EM_BUFFER_USAGE_Q_ASH); });
  WALK_QUEUE(&reTxQueue, finger,
             { sli_legacy_buffer_manager_buffer_usage[finger] |= BIT(EM_BUFFER_USAGE_Q_ASH); });
  sli_legacy_buffer_manager_buffer_usage[rxFirstDataBuffer] |= BIT(EM_BUFFER_USAGE_Q_ASH);
}
#endif

void sli_util_mark_ash_buffers(void)
{
  sli_legacy_buffer_manager_mark_buffer(&rxDataBuffer);
  sli_legacy_buffer_manager_mark_buffer(&txHighQueue);
  sli_legacy_buffer_manager_mark_buffer(&txQueue);
  sli_legacy_buffer_manager_mark_buffer(&reTxQueue);
  sli_legacy_buffer_manager_mark_buffer(&rxFirstDataBuffer);
  sli_legacy_buffer_manager_mark_buffer(&sendBuffer);
}

#ifdef SLEEPY_EZSP_UART
bool ashRxdActive = true; // set by a transition on the rxd input
uint8_t wakeRepeatTime;        // timer for repeating ASH_WAKEs to host (msecs)
#endif

//------------------------------------------------------------------------------
// Forward Declarations

static sl_zigbee_ezsp_status_t ashReceiveFrame(sli_buffer_manager_buffer_t *outBuf);
static uint16_t ashFrameType(uint16_t control, uint16_t len);
static void ashScrubReTxQueue(void);
static void ashStartRetransmission(void);
static void ashRejectFrame(void);
static sli_buffer_manager_buffer_t ashMessageBufferQueueNthEntry(sli_zigbee_message_buffer_queue_t *q,
                                                                 uint8_t n);
static void ashReleaseRxDataBuffer(void);
static sl_zigbee_ezsp_status_t ashReadFrame(void);
static void ashRandomizeBuffer(sli_buffer_manager_buffer_t buffer);
static void ashTraceDataRXEvent();
static void ashTraceDataTXEvent(sli_buffer_manager_buffer_t buffer);
static void ashTraceRstEvent();
static void ashTraceRstackEvent(uint16_t rstCode);

#ifdef ASH_VUART_TRACE
static void ashTraceFrame(bool sent);
static void ashTraceEvent(sl_zigbee_ezsp_status_t status, const char *string);
static void ashTraceFrameError(sl_zigbee_ezsp_status_t status);

#else
#define ashTraceFrame(x) do {} while (0)
#define ashTraceEvent(status, string) ashTraceEventDebugOnly(status)
#define ashTraceFrameError(x) do {} while (0)
#endif
static void ashTraceEventDebugOnly(sl_zigbee_ezsp_status_t status);

static void ashTraceAckEvent(sl_zigbee_ezsp_status_t status, uint8_t ackNum);

extern uint8_t halGetEm2xxResetInfo(void);

//------------------------------------------------------------------------------
// Functions implementing the interface upward to EZSP
bool rxFirstDataBufferFree = false;

static uint8_t staticBuffer[EZSP_MAX_FRAME_LENGTH];

sli_buffer_manager_buffer_t ashAllocateStaticBuffers(void)
{
  rxFirstDataBuffer = sli_legacy_buffer_manager_allocate_indirect_buffer(staticBuffer, NULL, EZSP_MAX_FRAME_LENGTH);
  rxFirstDataBufferFree = true;

  return rxFirstDataBuffer;
}

void ashStart(void)
{
#if 0   // initial values not needed if ashStart() is only called after a reset
  sendState = SEND_STATE_IDLE;
  ackRx = 0;
  ackTx = 0;
  frmTx = 0;
  frmReTx = 0;
  frmRx = 0;
  frmReTxHead = 0;
  ashTimeouts = 0;
  ashAckTimer = 0;
  ashNrTimer = 0;
  ashDecodeInProgress = false;
  ashSendBufferLen = 0;
  #ifdef SLEEPY_EZSP_UART
  ashRxdActive = true;
  #endif
#endif

  ashAllocateStaticBuffers();

  // In some cases the ncp must delay sending a RSTACK make sure it is not
  // lost by the host. This is needed with some RS232 to TTL converters
  // powered by modem control signals that may take several milliseconds after
  // power is applied to start operating.
  halResetWatchdog();
  ashSetAndStartAckTimer(sli_ash_ncp_get_config_or_default(ASH_CONFIG_REBOOT_DELAY, ASH_REBOOT_DELAY));
  while (!ashAckTimerHasExpired())  // if delay time is zero, timer expires
    simulatedTimePasses();          // immediately
  ashSetAckPeriod(sli_ash_ncp_get_config_or_default(ASH_CONFIG_ACK_TIME_INIT, ASH_TIME_DATA_INIT));

  // Send ASH_CAN before RSTACK and ERROR frames to help ensure they are
  // accepted by the host.
  ashError = halGetEm2xxResetInfo();
  switch (ashError) {
    case EM2XX_RESET_EXTERNAL:  // this code is never returned by EM2XX,
    case EM2XX_RESET_POWERON:   // a pin reset returns the poweron code
    case EM2XX_RESET_SOFTWARE:
    case EM2XX_RESET_BOOTLOADER:
      ashTraceRstackEvent(ashError);
      ashError = SL_ZIGBEE_EZSP_NO_ERROR;
      ashFlags = FLG_CONNECTED | FLG_RSTACK | FLG_CAN;
      ashTraceEvent(SL_ZIGBEE_EZSP_ASH_STARTED, "!\r\n=== ASH started ===\r\n");
      break;
    default:                    // unknown, assert and watchdog resets
      ashTraceEvent(SL_ZIGBEE_EZSP_ASH_STARTED, "!\r\n=== Error reset ===\r\n");
      ashFlags = FLG_ERROR | FLG_CAN;
      break;
  }
}

void ashSend(sli_buffer_manager_buffer_t buffer, bool highPriority)
{
  assert((sl_legacy_buffer_manager_message_buffer_length(buffer) >= ASH_MIN_DATA_FIELD_LEN)
         && (sl_legacy_buffer_manager_message_buffer_length(buffer) <= ASH_MAX_DATA_FIELD_LEN));
  ashTraceDataTXEvent(buffer);
  ashRandomizeBuffer(buffer);
  if (highPriority) {
    sli_legacy_packet_buffer_queue_add(&txHighQueue, buffer);
  } else {
    sli_legacy_packet_buffer_queue_add(&txQueue, buffer);
  }
}

sl_zigbee_ezsp_status_t ashReceive(sli_buffer_manager_buffer_t *outBuf)
{
  sl_zigbee_ezsp_status_t status;

  do {
    status = ashReceiveFrame(outBuf);
    simulatedTimePasses();
  } while (status == SL_ZIGBEE_EZSP_ASH_IN_PROGRESS);
  return status;
}

static sl_zigbee_ezsp_status_t ashReceiveFrame(sli_buffer_manager_buffer_t *outBuf)
{
  uint8_t ackNum;
  uint8_t frmNum;
  uint16_t frameType;
  sl_zigbee_ezsp_status_t status;

  *outBuf = SL_ZIGBEE_NULL_MESSAGE_BUFFER;

  // Read data from serial port and assemble a frame until complete, aborted
  // due to an error, cancelled, or there is no more serial data available.
  do {
    status = ashReadFrame();
    switch (status) {
      case SL_ZIGBEE_EZSP_SUCCESS:
        break;
      case SL_ZIGBEE_EZSP_ASH_IN_PROGRESS:
        break;
      case SL_ZIGBEE_EZSP_NO_RX_DATA:
        return status;
      case SL_ZIGBEE_EZSP_ASH_CANCELLED:
        ashTraceEvent(status, "cancelled");
        break;
      case SL_ZIGBEE_EZSP_ASH_BAD_CRC:
        ashTraceEvent(status, "CRC error");
        ashRejectFrame();
        break;
      case SL_ZIGBEE_EZSP_ASH_COMM_ERROR:
        ashTraceEvent(status, "comm error");
        ashRejectFrame();
        break;
      case SL_ZIGBEE_EZSP_ASH_TOO_SHORT:
        ashTraceEvent(status, "too short");
        ashRejectFrame();
        break;
      case SL_ZIGBEE_EZSP_ASH_TOO_LONG:
        ashTraceEvent(status, "too long");
        ashRejectFrame();
        break;
      default:
        assert(false);
    }
  } while (status != SL_ZIGBEE_EZSP_SUCCESS);

  // Got a complete frame - validate its control, length and ackNum (if
  // applicable). On any error the type will be TYPE_INVALID.
  // If a RST frame, just reboot.
  frameType = ashFrameType(rxControl, rxLen);

  if (frameType == TYPE_ACK) {
    ashTraceAckEvent(SL_ZIGBEE_EZSP_ASH_ACK_RECEIVED, rxControl);
  } else if (frameType == TYPE_NAK) {
    ashTraceAckEvent(SL_ZIGBEE_EZSP_ASH_NAK_RECEIVED, rxControl);
  }

  if (frameType == TYPE_RST) {
    //This function is called twice in order to make sure that Network Analyzer actually gets it.
    //With just one of these calls, it appears that the event is not sent over PTI.
    ashTraceRstEvent();
    ashTraceRstEvent();
    #ifdef EZSP_USB
    ashSetAndStartAckTimer(sli_ash_ncp_get_config_or_default(ASH_CONFIG_REBOOT_DELAY, ASH_REBOOT_DELAY));
    while (!ashAckTimerHasExpired())    // if delay time is zero, timer expires
      simulatedTimePasses();            // immediately
    #endif //EZSP_USB
    halReboot();
  }

  // Free the buffers allocated for a received frame if:
  //    DATA frame, and out of order
  //    DATA frame, and not in the CONNECTED state
  //    not a DATA frame
  if (frameType == TYPE_DATA) {
    if (!(ashFlags & FLG_CONNECTED) || (ASH_GET_FRMNUM(rxControl) != frmRx)) {
      ashReleaseRxDataBuffer();
    }
  } else {
    ashReleaseRxDataBuffer();
  }

  ashTraceFrame(false);                       // trace output (if enabled)

  // Reply to any frames received while not connected with ERROR.
  if (!(ashFlags & FLG_CONNECTED)) {
    ashFlags = FLG_ERROR;
    return SL_ZIGBEE_EZSP_ASH_IN_PROGRESS;
  }

  // Connected - process the ackNum in ACK, NAK and DATA frames
  if ((frameType == TYPE_DATA)
      || (frameType == TYPE_ACK)
      || (frameType == TYPE_NAK)) {
    ackNum = ASH_GET_ACKNUM(rxControl);
    if ( !WITHIN_RANGE(ackRx, ackNum, frmTx)) {
      ashTraceEvent(SL_ZIGBEE_EZSP_ASH_BAD_ACKNUM, "bad ackNum");
      frameType = TYPE_INVALID;
    } else if (ackNum != ackRx) {             // new frame(s) ACK'ed?
      ackRx = ackNum;
      ashTimeouts = 0;
      if (ashFlags & FLG_RETX) {              // start timer if unACK'ed frames
        ashStopAckTimer();
        if (ackNum != frmReTx) {
          ashStartAckTimer();
        }
      } else {
        ashAdjustAckPeriod(false);            // factor ACK time into period
        if (ackNum != frmTx) {                // if more unACK'ed frames,
          ashStartAckTimer();                 // then restart ACK timer
        }
        ashScrubReTxQueue();                  // free buffer(s) in ReTx queue
      }
    }
  }

  // Process nFlags in ACK and NAK frames
  if ((frameType == TYPE_ACK) || (frameType == TYPE_NAK)) {
    if (rxControl & ASH_NFLAG_MASK) {
      ashStartNrTimer();
    } else {
      ashStopNrTimer();
    }
  }

// Process frames received while connected
  switch (frameType) {
    case TYPE_DATA:
      frmNum = ASH_GET_FRMNUM(rxControl);
      if (frmNum == frmRx) {                  // is frame in sequence?
        if (rxDataBuffer == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {// valid but not stored?
          ashRejectFrame();
          ashTraceEvent(SL_ZIGBEE_EZSP_NO_RX_SPACE, "no buffer available");
          return SL_ZIGBEE_EZSP_NO_RX_SPACE;
        }
        if (rxControl & ASH_RFLAG_MASK) {     // if retransmitted, force ACK
          ashFlags |= FLG_ACK;
        }
        ashFlags &= ~(FLG_REJ | FLG_NAK);     // clear the REJ condition
        INC8(frmRx);
        ashRandomizeBuffer(rxDataBuffer);
        ashTraceDataRXEvent();
        sl_legacy_buffer_manager_hold_message_buffer(rxDataBuffer); // keep after transmitting

        *outBuf = rxDataBuffer;               // return the DATA frame
        return SL_ZIGBEE_EZSP_SUCCESS;
      } else {                                // frame is out of sequence
        if (rxControl & ASH_RFLAG_MASK) {     // if retransmitted, force ACK
          ashFlags |= FLG_ACK;
        } else {                              // 1st OOS? then set REJ, send NAK
          if ((ashFlags & FLG_REJ) == 0) {
            ashTraceEvent(SL_ZIGBEE_EZSP_ASH_OUT_OF_SEQUENCE, "out of sequence");
          }
          ashRejectFrame();
        }
      }
      break;
    case TYPE_ACK:                            // already fully processed
      break;
    case TYPE_NAK:                            // start retransmission if needed
      ashStartRetransmission();
      break;
    case TYPE_INVALID:                        // reject invalid frames
      ashRejectFrame();
      break;
  } // end switch(frameType)
  return SL_ZIGBEE_EZSP_ASH_IN_PROGRESS;
} // end of ashReceiveFrame()

void ashSendExec(void)
{
  static sli_zigbee_message_buffer_queue_t *sendQueue;       // source queue for current tx
  static uint8_t offset;      // offset from start of data frame - 0xFF = done
  uint8_t in, out, len;

  // Check for received acknowledgement timer expiry
  if ( ashAckTimerHasExpired() && (ashFlags & FLG_CONNECTED)) {
    if (ackRx != ((ashFlags & FLG_RETX) ? frmReTx : frmTx)) {
      ashAdjustAckPeriod(true);
      ashTraceEvent(SL_ZIGBEE_EZSP_ASH_ACK_TIMEOUT, "!Timer expired waiting for ACK");
      if (++ashTimeouts
          > sli_ash_ncp_get_config_or_default(ASH_CONFIG_MAX_TIMEOUTS, ASH_MAX_TIMEOUTS)) {
        ashError = SL_ZIGBEE_EZSP_ASH_ERROR_TIMEOUTS;
        ashTraceEvent(SL_ZIGBEE_EZSP_ASH_DISCONNECTED, "!ASH disconnected");
        ashFlags = FLG_ERROR;
      } else {
        ashStartRetransmission();
      }
    } else {
      ashStopAckTimer();
    }
  }

  while (ashSerialWriteAvailable() == SL_ZIGBEE_EZSP_SUCCESS) {
    // Send ASH_CAN character immediately, ahead of any other transmit data
    if (ashFlags & FLG_CAN) {
      if (ashFlags & (FLG_RSTACK | FLG_ERROR)) {
        ashSerialWriteByte(ASH_CAN);
      } else if (sendState == SEND_STATE_TX_DATA) {
        ashStopAckTimer();
        sendState = SEND_STATE_IDLE;
        ashSerialWriteByte(ASH_CAN);
      }
      ashFlags &= ~FLG_CAN;
      continue
      ;
    }

    switch (sendState) {
      // In between frames - do some housekeeping and decide what to send next
      case SEND_STATE_IDLE:
        // If retransmitting, set the next frame to send to the last ackNum
        // received, then check to see if retransmission is now complete.
        if (ashFlags & FLG_RETX) {
          if (WITHIN_RANGE(frmReTx, ackRx, frmTx)) {
            frmReTx = ackRx;
          }
          if (frmReTx == frmTx) {
            ashFlags &= ~FLG_RETX;
            ashScrubReTxQueue();
          }
        }

#ifdef SLEEPY_EZSP_ASH
        // Send a wake byte (0xFF) between frames to alert the host to callbacks.
        if (ashFlags & FLG_WAKE) {
          ashSerialWriteByte(ASH_WAKE);
          ashFlags &= ~FLG_WAKE;
          break;
        }
#endif

        // See if a short frame is flagged to be sent
        // The order of the tests below - ERROR, RSTACK, NAK and ACK -
        // sets the relative priority of sending these frame types.
        if (ashFlags & (FLG_ERROR | FLG_RSTACK)) {
          if (ashFlags & FLG_ERROR) {
            txControl = ASH_CONTROL_ERROR;
            txBuffer[2] = ashError;
          } else {
            txControl = ASH_CONTROL_RSTACK;
            txBuffer[2] = halGetEm2xxResetInfo();
          }
          txBuffer[1] = ASH_VERSION;
          ashFlags &= ~(FLG_RSTACK | FLG_ERROR | FLG_NAK | FLG_ACK);
          len = 3;
          sendState = SEND_STATE_SHFRAME;
        } else if (ashFlags & (FLG_NAK | FLG_ACK)) {
          if (ashFlags & FLG_NAK) {
            txControl = ASH_CONTROL_NAK + (frmRx << ASH_ACKNUM_BIT);
            ashTraceAckEvent(SL_ZIGBEE_EZSP_ASH_NAK_SENT, txControl);
            ashFlags &= ~(FLG_NAK | FLG_ACK);
          } else {
            txControl = ASH_CONTROL_ACK + (frmRx << ASH_ACKNUM_BIT);
            ashTraceAckEvent(SL_ZIGBEE_EZSP_ASH_ACK_SENT, txControl);
            ashFlags &= ~FLG_ACK;
          }
          ackTx = frmRx;
          len = 1;
          sendState = SEND_STATE_SHFRAME;
          // See if retransmitting DATA frames for error recovery
        } else if (ashFlags & FLG_RETX) {
          sendBuffer = ashMessageBufferQueueNthEntry(&reTxQueue,
                                                     MOD8(frmReTx - frmReTxHead));
          len = sl_legacy_buffer_manager_message_buffer_length(sendBuffer) + 1;
          txControl = ASH_CONTROL_DATA
                      | (frmReTx << ASH_FRMNUM_BIT)
                      | (frmRx << ASH_ACKNUM_BIT)
                      | ASH_RFLAG_MASK;
          sendState = SEND_STATE_RETX_DATA;
          // See if DATA frame is ready to send:
          //    must have less than K outstanding unACKed frames AND
          //      a high-priority frame is ready OR
          //      a normal frame is ready and the host isn't holding us off
        } else if ((WITHIN_RANGE(ackRx,
                                 frmTx,
                                 ackRx + sli_ash_ncp_get_config_or_default(ASH_CONFIG_TX_K, ASH_TX_K) - 1))
                   && (!sli_legacy_packet_buffer_queue_is_empty(&txHighQueue)
                       || ((!sli_legacy_packet_buffer_queue_is_empty(&txQueue)
                            && ashNrTimerHasExpired())))) {
          sendQueue = sli_legacy_packet_buffer_queue_is_empty(&txHighQueue)
                      ? &txQueue : &txHighQueue;
          sendBuffer = sli_legacy_buffer_manager_buffer_queue_head(sendQueue);
          len = sl_legacy_buffer_manager_message_buffer_length(sendBuffer) + 1;
          txControl = ASH_CONTROL_DATA
                      | (frmTx << ASH_FRMNUM_BIT)
                      | (frmRx << ASH_ACKNUM_BIT);
          sendState = SEND_STATE_TX_DATA;
          // See if an ACK should be generated
        } else if (ackTx != frmRx) {
          ashFlags |= FLG_ACK;
          break;
          // Otherwise there's nothing to send
        } else {
          return;
        }

        // Start frame - ashEncodeByte() is inited by a non-zero length argument
        ashTraceFrame(true);            // trace output (if enabled)
        out = ashEncodeByte(len, txControl, &offset);
        ashSerialWriteByte(out);

        break;

      case SEND_STATE_SHFRAME:                // sending short frame
        if (offset != 0xFF) {
          in = txBuffer[offset];
          out = ashEncodeByte(0, in, &offset);
          ashSerialWriteByte(out);
        } else {
          sendState = SEND_STATE_IDLE;
        }
        break;

      case SEND_STATE_TX_DATA:                // sending data frame
      case SEND_STATE_RETX_DATA:              // resending data frame
        if (offset != 0xFF) {                 // if not done txing frame
          in = (offset == 0)  // if starting frame, send control byte
               ? txControl    // followed by data from buffer
               : sl_legacy_buffer_manager_get_linked_buffers_byte(sendBuffer, offset - 1);
          out = ashEncodeByte(0, in, &offset);
          ashSerialWriteByte(out);
        } else {                              // frame txed - do bookkeeping
          if (sendState == SEND_STATE_TX_DATA) {
            INC8(frmTx);
            (void)sli_legacy_packet_buffer_queue_remove_head(sendQueue); // save txed data
            sli_legacy_packet_buffer_queue_add(&reTxQueue, sendBuffer);  // in retx queue
          } else {
            INC8(frmReTx);
          }
          if (ashAckTimerIsNotRunning()) {
            ashStartAckTimer();
          }
          ackTx = frmRx;
          sendState = SEND_STATE_IDLE;
        }
        break;
    }   // end of switch(sendState)
  }     // end of while (ashSerialWriteAvailable() == SL_ZIGBEE_EZSP_SUCCESS)
} // end of ashSendExec()

//------------------------------------------------------------------------------
// Local utility functions

// If not already retransmitting, and there are unacked frames, start
// retransmitting after the last frame that was acked.
static void ashStartRetransmission(void)
{
  if ( !(ashFlags & FLG_RETX) && (ackRx != frmTx)) {
    ashStopAckTimer();
    frmReTx = ackRx;
    ashFlags |= (FLG_RETX | FLG_CAN);
  }
}

// If the last control byte received was a DATA control,
// and we are connected and not already in the reject condition,
// then send a NAK and set the reject condition.
void ashRejectFrame(void)
{
  if (((rxControl & ASH_DFRAME_MASK) == ASH_CONTROL_DATA)
      && ((ashFlags & (FLG_REJ | FLG_CONNECTED)) == FLG_CONNECTED)) {
    ashFlags |= (FLG_REJ | FLG_NAK);
  }
}

static void ashScrubReTxQueue(void)
{
  sli_buffer_manager_buffer_t buffer;

  while (ackRx != frmReTxHead) {
    buffer = sli_legacy_packet_buffer_queue_remove_head(&reTxQueue);
    if (buffer == rxFirstDataBuffer) {
      rxFirstDataBufferFree = true;
    }
    sl_legacy_buffer_manager_release_message_buffer(buffer);  // free dynamically alloced buffers
    INC8(frmReTxHead);
  }
}

// Return the Nth linked buffer in the queue, where N==0 means the head, N==1
// the entry before the head, and N == (queue length - 1) is the tail.
static sli_buffer_manager_buffer_t ashMessageBufferQueueNthEntry(sli_zigbee_message_buffer_queue_t *q,
                                                                 uint8_t n)
{
  sli_buffer_manager_buffer_t buffer;

  {
    DECLARE_INTERRUPT_STATE;
    DISABLE_INTERRUPTS();
    buffer = sli_legacy_buffer_manager_buffer_queue_head(q);
    while (n) {
      buffer = sli_legacy_buffer_manager_buffer_queue_next(q, buffer);
      n--;
    }
    RESTORE_INTERRUPTS();
  }
  return buffer;
}
uint8_t firstDataBufferIndex = 0;

// Flag there is not a DATA frame in the process of being received.
static void ashReleaseRxDataBuffer(void)
{
  if (rxDataBuffer == rxFirstDataBuffer) {
    rxFirstDataBufferFree = true;
  }
  rxDataBuffer = SL_ZIGBEE_NULL_MESSAGE_BUFFER;
}

// Reads and decodes a frame.
// The length of all types of frames received is returned in rxLen.
// (rxLen is valid even for frames discarded because rxFirstDataBuffer was in
// use, and will be ASH_MAX_FRAME_WITH_CRC_LEN+1 if longer than legal maximum.)
// Short frame types (ACK, NAK, and RST) are returned in rxBuffer, and all
// DATA frames are returned in the linked buffer rxFirstDataBuffer.
// The return value is one of the following, all passed through from
// ashDecodeByte() or ashSerialReadByte().
//  SL_ZIGBEE_EZSP_SUCCESS
//  SL_ZIGBEE_EZSP_ASH_IN_PROGRESS
//  SL_ZIGBEE_EZSP_NO_RX_DATA
//  SL_ZIGBEE_EZSP_ASH_CANCELLED
//  SL_ZIGBEE_EZSP_ASH_BAD_CRC
//  SL_ZIGBEE_EZSP_ASH_COMM_ERROR
//  SL_ZIGBEE_EZSP_ASH_TOO_SHORT
//  SL_ZIGBEE_EZSP_ASH_TOO_LONG
// If a frame is discarded because rxFirstDataBuffer is in use, the return
// value is SL_ZIGBEE_EZSP_SUCCESS but rxDataBuffer is SL_ZIGBEE_NULL_MESSAGE_BUFFER.
static sl_zigbee_ezsp_status_t ashReadFrame(void)
{
  uint8_t index;
  uint8_t in;
  uint8_t out;
  sl_zigbee_ezsp_status_t status;

  if (!ashDecodeInProgress) {
    rxLen = 0;
    ashReleaseRxDataBuffer();
  }

  do {
    // Get next byte from serial port, return if no data
    status = ashSerialReadByte(&in);
    if (status == SL_ZIGBEE_EZSP_NO_RX_DATA) {
      break;
    }

#ifdef SLEEPY_EZSP_UART
    ashRxdActive = true;
    if ( !ashDecodeInProgress && (in == ASH_WAKE)) {
      ashFlags |= FLG_WAKE;
      status = SL_ZIGBEE_EZSP_ASH_IN_PROGRESS;
      continue;
    }
#endif

    // Decode next input byte - note that many input bytes do not produce
    // an output byte. Return on any error in decoding.
    index = rxLen;
    status = ashDecodeByte(in, &out, &rxLen);
    if ((status != SL_ZIGBEE_EZSP_ASH_IN_PROGRESS) && (status != SL_ZIGBEE_EZSP_SUCCESS)) {
      ashTraceFrameError(status);   // if enabled, output frame contents
      ashReleaseRxDataBuffer();
      break;                  // discard an invalid frame
    }

    if (rxLen != index) {           // if input byte produced an output byte
      if (rxLen <= RX_BUFFER_LEN) { // if a short frame, return in rxBuffer
        rxBuffer[index] = out;
        // If a longer DATA frame, copy into the permanent linked packet buffers
        // starting with rxFirstDataBuffer. If this is in use, a situation that
        // could arise when the host is retransmitting due to ACK timeout, none
        // of the frame's payload will be stored, but it will be validated
        // as usual. In this case its control byte will be returned in rxControl.
        // Even if the data is discarded, its ackNum must be processed.
      } else {
        if (rxLen == RX_BUFFER_LEN + 1) {  // need the first linked buffer?
          if (rxFirstDataBufferFree) {
            if (rxFirstDataBuffer == sli_legacy_buffer_manager_buffer_queue_head(&reTxQueue)) {
              // If there is still a buffer that may need to be retransmitted
              // and that buffer has same ID with rxFirstDataBuffer
              // we should allocate a different one and copy the data over
              sli_buffer_manager_buffer_t reTxBuffer = sli_legacy_buffer_manager_really_allocate_buffer(sl_legacy_buffer_manager_message_buffer_length(rxFirstDataBuffer), false);
              // Copy the retransmit data from the original buffer to the new buffer
              sl_legacy_buffer_manager_copy_to_linked_buffers(
                sl_legacy_buffer_manager_get_linked_buffers_pointer(rxFirstDataBuffer, 0),
                reTxBuffer,
                0,
                sl_legacy_buffer_manager_message_buffer_length(rxFirstDataBuffer)
              );
              sli_legacy_packet_buffer_queue_remove_head(&reTxQueue);
              if (reTxBuffer != NULL_BUFFER) {
                sli_legacy_packet_buffer_queue_add(&reTxQueue, reTxBuffer);  // in retx queue
              } else {
                // Handle allocation failure: log, assert, or take other action
                DEBUG_ASSERT(0); // or use your preferred error handling
              }
            }
            rxFirstDataBufferFree = false;
            rxDataBuffer = rxFirstDataBuffer;
            sl_legacy_buffer_manager_set_message_buffer_length(rxDataBuffer, RX_BUFFER_LEN - 1);
            sl_legacy_buffer_manager_copy_to_linked_buffers(&rxBuffer[1],
                                                            rxDataBuffer,
                                                            0,
                                                            RX_BUFFER_LEN - 1);
          }
        }
        // Unless the data is being discarded, save the next byte in
        // the permanent linked buffer
        if (rxDataBuffer == rxFirstDataBuffer) {
          index = sl_legacy_buffer_manager_message_buffer_length(rxFirstDataBuffer);
          sl_legacy_buffer_manager_set_message_buffer_length(rxFirstDataBuffer, index + 1);
          sl_legacy_buffer_manager_set_linked_buffers_byte(rxFirstDataBuffer, index, out);
        }
      }
    }
  } while (status == SL_ZIGBEE_EZSP_ASH_IN_PROGRESS);
  return status;
} // end of ashReadFrame()

// If enabled, exclusive-OR linked buffer data with a pseudo-random sequence
static void ashRandomizeBuffer(sli_buffer_manager_buffer_t buffer)
{
  uint8_t idx, cnt, total;
  uint8_t r = 0;                // zero initializes the random sequence

  if (sli_ash_ncp_get_config_or_default(ASH_CONFIG_RANDOMIZE, ASH_RANDOMIZE)) {
    for (idx = 0; idx < sl_legacy_buffer_manager_message_buffer_length(buffer); idx += cnt) {
      total = sl_legacy_buffer_manager_message_buffer_length(buffer) - idx;
      if (total >= PACKET_BUFFER_SIZE) {
        cnt = PACKET_BUFFER_SIZE;
      } else {
        cnt = total;
      }
      r = ashRandomizeArray(r, sl_legacy_buffer_manager_get_linked_buffers_pointer(buffer, idx), cnt);
    }
  }
}

//------------------------------------------------------------------------------
// Low-level serial I/O functions

void ashSerialWriteByte(uint8_t byte)
{
  (void)sli_legacy_serial_write_byte(ASH_PORT, byte);
}

sl_zigbee_ezsp_status_t ashSerialWriteAvailable(void)
{
  return SL_ZIGBEE_EZSP_SUCCESS;
}

// Return next byte from serial input if there is one, otherwise
// return status indication that there is no data right now.
// If ASH expects to process ASH_SUB bytes, and there was a low-level
// serial error (uart overrun, buffer overflow or framing error),
// then replace the byte received in error with ASH_SUB. Note these
// errors are only detected now in FIFO mode.
sl_zigbee_ezsp_status_t ashSerialReadByte(uint8_t *byte)
{
  switch (sli_legacy_serial_read_byte(ASH_PORT, byte)) {
    case SL_STATUS_OK:         // data was written to *byte
      return SL_ZIGBEE_EZSP_SUCCESS;
    default:                    // serial port error
#if SL_LEGACY_SERIAL1_MODE == SL_LEGACY_SERIAL_FIFO
      *byte = ASH_SUB;
      return SL_ZIGBEE_EZSP_SUCCESS;
    case SL_STATUS_EMPTY: // no data available
#endif
      return SL_ZIGBEE_EZSP_NO_RX_DATA;
  }
}

// Determines frame type from the control byte then validates its length.
// If invalid type or length, returns TYPE_INVALID.
static uint16_t ashFrameType(uint16_t control, uint16_t len)
{
  if (control == ASH_CONTROL_RST) {
    if (len == ASH_FRAME_LEN_RST) {
      return TYPE_RST;
    }
  } else if ((control & ASH_DFRAME_MASK) == ASH_CONTROL_DATA) {
    if (len >= ASH_FRAME_LEN_DATA_MIN) {
      return TYPE_DATA;
    }
  } else if ((control & ASH_SHFRAME_MASK) == ASH_CONTROL_ACK) {
    if (len == ASH_FRAME_LEN_ACK) {
      return TYPE_ACK;
    }
  } else if ((control & ASH_SHFRAME_MASK) == ASH_CONTROL_NAK) {
    if (len == ASH_FRAME_LEN_NAK) {
      return TYPE_NAK;
    }
  } else {
    ashTraceEvent(SL_ZIGBEE_EZSP_ASH_BAD_CONTROL, "illegal control");
    return TYPE_INVALID;
  }
  ashTraceEvent(SL_ZIGBEE_EZSP_ASH_BAD_LENGTH, "illegal length");
  return TYPE_INVALID;
}

//------------------------------------------------------------------------------
// Trace output functions

#ifdef ASH_VUART_TRACE

static void ashPrintFrame(uint8_t c)
{
  if ((c & ASH_DFRAME_MASK) == ASH_CONTROL_DATA) {
    if (c & ASH_RFLAG_MASK) {
      ashDebugPrintf("DATA(%d,%d)", ASH_GET_FRMNUM(c), ASH_GET_ACKNUM(c));
    } else {
      ashDebugPrintf("data(%d,%d)", ASH_GET_FRMNUM(c), ASH_GET_ACKNUM(c));
    }
  } else if ((c & ASH_SHFRAME_MASK) == ASH_CONTROL_ACK) {
    if (ASH_GET_NFLAG(c)) {
      ashDebugPrintf("ack(%d)-  ", ASH_GET_ACKNUM(c));
    } else {
      ashDebugPrintf("ack(%d)+  ", ASH_GET_ACKNUM(c));
    }
  } else if ((c & ASH_SHFRAME_MASK) == ASH_CONTROL_NAK) {
    if (ASH_GET_NFLAG(c)) {
      ashDebugPrintf("NAK(%d)-  ", ASH_GET_ACKNUM(c));
    } else {
      ashDebugPrintf("NAK(%d)+  ", ASH_GET_ACKNUM(c));
    }
  } else if (c  == ASH_CONTROL_RST) {
    ashDebugPrintf("RST      ");
  } else if (c == ASH_CONTROL_RSTACK) {
    ashDebugPrintf("RSTACK   ");
  } else if (c == ASH_CONTROL_ERROR) {
    ashDebugPrintf("ERROR   ");
  } else {
    ashDebugPrintf("???? 0x%X", c);
  }
}

static void ashPrintStatus(void)
{
  sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                 "BBBBBBBBBWB",
                                 ASH_VERSION,
                                 SL_ZIGBEE_EZSP_ASH_STATUS,
                                 ackRx,
                                 ackTx,
                                 frmTx,
                                 frmRx,
                                 frmReTx,
                                 sli_zigbee_message_buffer_queue_length(&reTxQueue),
                                 ashTimeouts,
                                 ashGetAckPeriod(),
                                 emPacketBufferFreeCount);
}

static void ashPrintTime(void)
{
  uint32_t now;
  uint16_t secs, msecs;

  now = halCommonGetInt32uMillisecondTick();
  secs = (uint16_t)(now / 1000);
  msecs = (uint16_t)(now % 1000);
  if (msecs < 10) {
    ashDebugPrintf("Time: %u.00%u", secs, msecs);
  } else if (msecs < 100) {
    ashDebugPrintf("Time: %u.0%u", secs, msecs);
  }
}

static void ashTraceFrame(bool sent)
{
  uint8_t flags;

  flags = sli_ash_ncp_get_config_or_default(ASH_CONFIG_TRACE_FLAGS, ASH_TRACE_FLAGS);
  if (flags & (TRACE_VUART_FRAMES_BASIC | TRACE_VUART_FRAMES_VERBOSE)) {
    ashPrintTime();
    if (sent) {
      ashPrintFrame(txControl);
      if (flags & TRACE_VUART_FRAMES_VERBOSE) {
        ashPrintStatus();
      }
      ashDebugPrintf("Tx: %01x\r\n", txControl);
    } else {
      ashPrintFrame(rxControl);
      if (flags & TRACE_VUART_FRAMES_VERBOSE) {
        ashPrintStatus();
      }
      ashDebugPrintf(" Rx\r\n");
    }
  }
}

static void ashTraceEvent(sl_zigbee_ezsp_status_t status, const char *string)
{
  ashTraceEventDebugOnly(status);
  if (sli_ash_ncp_get_config_or_default(ASH_CONFIG_TRACE_FLAGS, ASH_TRACE_FLAGS)
      & TRACE_VUART_EVENTS) {
    ashPrintTime();
    if ( *string != '!' ) {
      ashDebugPrintf("Rec'd frame: ");
    } else {
      string++;
    }
    ashDebugPrintf(string);
  }
}

static void ashTraceFrameError(sl_zigbee_ezsp_status_t status)
{
  uint8_t *p = sl_legacy_buffer_manager_message_buffer_contents(rxFirstDataBuffer);
  uint8_t i;

  ashDebugPrintf("status = %X length = %X\r\n", status, rxLen);
  for (i = 0; i < rxLen; i++) {
    ashDebugPrintf("%X ", *p++);
    if ((i & 15) == 15) {
      ashDebugPrintf("\r\n");
    }
  }
  ashDebugPrintf("\r\n");
}

#endif //  #ifdef ASH_VUART_TRACE

static void ashTraceEventDebugOnly(sl_zigbee_ezsp_status_t status)
{
#if !defined(SL_ZIGBEE_TEST) && !defined(ZIGBEE_STACK_ON_HOST)
  if (status == SL_ZIGBEE_EZSP_ASH_STARTED) {
    sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                   "BBl",
                                   ASH_VERSION,
                                   SL_ZIGBEE_EZSP_ASH_STARTED,
                                   MFG_ASH_CONFIG_ARRAY_SIZE * sizeof(tokTypeMfgAshConfig));
  } else {
    sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                   "BBBB",
                                   ASH_VERSION,
                                   status,
                                   rxControl,
                                   rxLen);
  }
#else
  (void) status;
#endif
}
//Sends an ACK message.
static void ashTraceAckEvent(sl_zigbee_ezsp_status_t status, uint8_t ackNum)
{
  sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                 "BBB",
                                 ASH_VERSION,
                                 status,
                                 ackNum);
}
//Sends an RSTACK message to Network Analyzer
static void ashTraceRstackEvent(uint16_t rstCode)
{
  sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                 "BBBBB",
                                 ASH_VERSION,
                                 SL_ZIGBEE_EZSP_ASH_TX,
                                 ASH_CONTROL_RSTACK,
                                 ASH_VERSION,
                                 rstCode);
}

//Sends an RST message to Network Analyzer
static void ashTraceRstEvent()
{
  sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                 "BBB",
                                 ASH_VERSION,
                                 SL_ZIGBEE_EZSP_ASH_RST_RECEIVED,
                                 rxControl);
}
//Sends back a received DATA frame to Network Analyzer
static void ashTraceDataRXEvent()
{
  sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                 "BBBb",
                                 ASH_VERSION,
                                 SL_ZIGBEE_EZSP_ASH_RX,
                                 rxControl,
                                 rxDataBuffer);
}
//This function occasionally has acks and naks come through, hence the logic.
static void ashTraceDataTXEvent(sli_buffer_manager_buffer_t buffer)
{
  if ((txControl != ASH_CONTROL_RSTACK) && (txControl != ASH_CONTROL_RST) && (txControl != ASH_CONTROL_ERROR)) {
    if ((txControl & ASH_CONTROL_NAK) == ASH_CONTROL_NAK ) {
      return;
    } else if ((txControl & ASH_CONTROL_ACK) == ASH_CONTROL_ACK ) {
      ashTraceAckEvent(SL_ZIGBEE_EZSP_ASH_ACK_SENT, txControl);
      return;
    }
  }

  sli_zigbee_debug_binary_format(EM_DEBUG_EZSP_UART,
                                 "BBBb",
                                 ASH_VERSION,
                                 SL_ZIGBEE_EZSP_ASH_TX,
                                 txControl,
                                 buffer);
}

//------------------------------------------------------------------------------
// Sleepy mode functions

// Prototype callbackPending function instead of including em260.h to work
// around abstraction layer inversion.
bool callbackPending(void);

// This code is a partial duplicate of the 'serialOkToSleep()',
// however I did not feel confident enough to make changes to that code
// without potentially introducing bugs.  Testing of the sleepy UART at the time
// of this writing had been minimal.
// Hopefully this code will be cleaned up in the future and the duplication
// removed. (RBA)
bool serialOkToBootload(void)
{
  bool ok = true;

  // This code is tenuously constructed.  I modeled it after "serialOkToSleep()"
  // but empirically I found that some of the checks NEVER became true and
  // therefore caused an EZSP timeout.
  // Since the host is expecting us to bootload we just want to make sure
  // they receive the response to their command, we do not have to insure
  // that the state of ASH is maintained since neither the bootloader nor the
  // host will use it during the bootload.  The expectation for the NCP
  // is that it will reboot when it is done.

  ok = (ok
        && !ashDecodeInProgress                 // don't have a partial frame
        && (ashFlags | FLG_CONNECTED
            ? (ackTx == frmRx)                  // do not need to send an ACK
            : true)
        && !(callbackPending())
        && sli_legacy_packet_buffer_queue_is_empty(&txQueue)
        && sli_legacy_packet_buffer_queue_is_empty(&txHighQueue)
        && (sendState == SEND_STATE_IDLE)       // nothing being transmitted now
        );
  return ok;
}

#ifdef SLEEPY_EZSP_ASH

void serialPendingCallbacks(bool pending)
{
  if (pending && uartSynchCallbacks) {
    if (!(ashFlags & FLG_CALLBACKS)) {
      ashFlags |= (FLG_WAKE | FLG_CALLBACKS);
    }
  } else {
    ashFlags &= ~(FLG_WAKE | FLG_CALLBACKS);
  }
}

void serialInhibitCallbackSignal(void)
{
  ashFlags |= FLG_CALLBACKS;
  ashFlags &= ~FLG_WAKE;
}

// Returns true if the ncp can sleep without interfering with ASH.
// IOStream will add its own dependency in Power Manager, which will prevent
// sleep if something is happening on the serial line
bool serialOkToSleep(void)
{
  bool ok;

  ok = !ashDecodeInProgress                    // don't have a partial frame
       && !sli_legacy_serial_read_available(ASH_PORT)      // RX buffer is empty
       && (ashFlags == FLG_CONNECTED)          // no pending ACKs, NAKs, etc.
       && (ackTx == frmRx)                     // do not need to send an ACK
       && (ackRx == frmTx)                     // not waiting to receive an ACK
       && !(callbackPending())
       && sli_legacy_packet_buffer_queue_is_empty(&txQueue)
       && sli_legacy_packet_buffer_queue_is_empty(&txHighQueue)
       && (sendState == SEND_STATE_IDLE)       // nothing being transmitted now
  ;
  return ok;
}

void serialPowerDown(void)
{
  ashRxdActive = false;
}

void serialPowerUp(void)
{
  if ((sleepMode == EZSP_FRAME_CONTROL_POWER_DOWN)
      || serialHostIsActive()) {
    ashSerialWriteByte(ASH_WAKE);
    wakeRepeatTime = halCommonGetInt16uMillisecondTick()
                     + ASH_HOST_WAKE_REPEAT_DELAY;
  }
}

bool serialWaitingForHostToWake(void)
{
  if (serialHostIsActive()) {
    return false;
  }
  if ((int8_t)(halCommonGetInt16uMillisecondTick() - wakeRepeatTime) >= 0) {
    if (ashSerialWriteAvailable() == SL_ZIGBEE_EZSP_SUCCESS) {
      ashSerialWriteByte(ASH_WAKE);
      wakeRepeatTime = halCommonGetInt16uMillisecondTick()
                       + ASH_HOST_WAKE_REPEAT_DELAY;
    }
  }
  return true;
}

void serialSendAwakeSignal(void)
{
  ashSerialWriteByte(ASH_WAKE);
}

#ifdef CORTEXM3
bool serialHostIsActive(void)
{
  // FIXME: hack to make sure that we catch the brief start bit on RxD:
  // assume that any GPIO wake event was due to RxD activity. In the future
  // use PWRUP_SC1 but this is not yet supported by the sleep code.
  if (PWRUP_EVENT & PWRUP_GPIO) {
    NVIC_SetPendingIRQ(IRQD_IRQn);  // isr sets ashRxdActive
    PWRUP_EVENT = PWRUP_GPIO;
  }
  ashRxdActive |= (sli_legacy_serial_read_available(ASH_PORT) != 0);
  return ashRxdActive;
}

// Setup IRQD to interrupt on any transition on SC1RXD (PB2)
void serialMonitorRxd(uint8_t sleepmode)
{
  (void)sleepMode;
  NVIC_DisableIRQ(IRQD_IRQn);  // isr sets ashRxdActive
  GPIO_INTCFGD = GPIOINTMOD_DISABLED << GPIO_INTMOD_BIT;
  GPIO_IRQDSEL = 10;                        // assign IRQD to PB2
  INT_GPIOFLAG = INT_IRQDFLAG;              // clear any stale interrupts
  NVIC_ClearPendingIRQ(IRQD_IRQn);
  NVIC_EnableIRQ(IRQD_IRQn);
  GPIO_INTCFGD = GPIOINTMOD_BOTH_EDGES << GPIO_INTMOD_BIT;
}

void halIrqDIsr(void)
{
  NVIC_DisableIRQ(IRQD_IRQn);  // disable any more interrupts
  GPIO_INTCFGD = GPIOINTMOD_DISABLED << GPIO_INTMOD_BIT;
  INT_GPIOFLAG = INT_IRQDFLAG;
  ashRxdActive = true;
  asm ("DMB");
}

#endif // CORTEXM3

#else //!SLEEPY_EZSP_ASH

// In our power_manager, is_ok_to_sleep() returns true by default (refer to sl_power_manager_handler.c).
// Without SLEEPY_EZSP_ASH defined, we should keep ncp-uart awake. Hence, this serialOkToSleep() is used
// to subscribe the is_ok_to_sleep() power_manager_handler in zigbee_ncp_framework.slcc to make it awake.
bool serialOkToSleep(void)
{
  return false;
}

#endif //SLEEPY_EZSP_ASH

//------------------------------------------------------------------------------
// Configuration token access functions

#if defined(CORTEXM3)
tokTypeMfgAshConfig sli_ash_ncp_get_config(sli_ash_ncp_config_index index)
{
  if (index >= ASH_CONFIG_INDEX_MAX) {
    return INVALID_ASH_CONFIG_VALUE;  // Invalid input
  }

  sl_status_t status = SL_STATUS_OK;
  tokTypeMfgAshConfig tokenData;

  status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_ASH_CONFIG) + index, (void *)&tokenData, sizeof(tokTypeMfgAshConfig));

  if (status != SL_STATUS_OK) {
    return INVALID_ASH_CONFIG_VALUE;
  }
  return tokenData;
}
#endif // defined(CORTEXM3)

tokTypeMfgAshConfig sli_ash_ncp_get_config_or_default(sli_ash_ncp_config_index index, tokTypeMfgAshConfig defaultValue)
{
#if defined(CORTEXM3)
  tokTypeMfgAshConfig value = sli_ash_ncp_get_config(index);
  return (value != INVALID_ASH_CONFIG_VALUE) ? value : defaultValue;
#else
  // always return default value for Zigbeed or simulation
  (void)index;
  return defaultValue;
#endif // defined(CORTEXM3)
}
