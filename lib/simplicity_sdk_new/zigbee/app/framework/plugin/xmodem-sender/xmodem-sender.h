/***************************************************************************//**
 * @file
 * @brief Definitions for the XMODEM Sender plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

/**
 * @defgroup xmodem-sender XModem Sender
 * @ingroup component
 * @brief API and Callbacks for the XModem Sender Component
 *
 */

/**
 * @addtogroup xmodem-sender
 * @{
 */

#define XMODEM_SOH   (0x01)
#define XMODEM_EOT   (0x04)
#define XMODEM_ACK   (0x06)
#define XMODEM_NAK   (0x15)
#define XMODEM_CANCEL (0x18)
#define XMODEM_BLOCKOK (0x19)
#define XMODEM_FILEDONE (0x17)

typedef sl_status_t (sl_zigbee_af_xmodem_sender_transmit_function_t)(uint8_t* data, uint8_t length);

typedef sl_status_t (sl_zigbee_af_xmodem_sender_get_next_block_function_t)(uint32_t address,
                                                                           uint8_t length,
                                                                           uint8_t* returnData,
                                                                           uint8_t* returnLength,
                                                                           bool* done);
typedef void (sl_zigbee_af_xmodem_sender_finished_function_t)(bool success);

/**
 * @name API
 * @{
 */

void sl_zigbee_af_xmodem_sender_incoming_block(uint8_t* data,
                                               uint8_t  length);

// The maxSizeOfBlock does not include the Xmodem overhead (5-bytes)
sl_status_t sl_zigbee_af_xmodem_sender_start(sl_zigbee_af_xmodem_sender_transmit_function_t* sendRoutine,
                                             sl_zigbee_af_xmodem_sender_get_next_block_function_t* getNextBlockRoutine,
                                             sl_zigbee_af_xmodem_sender_finished_function_t* finishedRoutine,
                                             uint8_t maxSizeOfBlock,
                                             bool waitForReady);

void sl_zigbee_af_xmodem_sender_abort(void);

/** @} */ // end of name API
/** @} */ // end of xmodem-sender
