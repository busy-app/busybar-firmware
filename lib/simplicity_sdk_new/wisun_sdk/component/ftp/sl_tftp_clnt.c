/***************************************************************************//**
 * @file sl_tftp_clnt.c
 * @brief Trivial File Transfer Protocol Client
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sl_assert.h"
#include "cmsis_os2.h"
#include "sl_string.h"
#include "sl_wisun_trace_util.h"
#include "sl_tftp_clnt.h"
#include "sl_memory_manager.h"

#if SL_FTP_ENABLE_TFTP_PROTOCOL
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Default buffer size: 512 bytes data + highest memory footprint of header
#define SL_TFTP_DEFAULT_BUFF_SIZE                           (SL_TFTP_DEFAULT_DATA_BLOCK_SIZE + sizeof(sl_tftp_pkt_t))

/// TFTP message queue length
#define SL_TFTP_MESSAGE_QUEUE_SIZE                          (1UL)

/// TFTP valid (without fragmentation) block size value min
#define SL_TFTP_VALID_BLOCKSIZE_VALUE_MIN                   (8U)
/// TFTP valid (without fragmentation) block size value max
#define SL_TFTP_VALID_BLOCKSIZE_VALUE_MAX                   (1228U)

/// TFTP valid timeout value min in seconds
#define SL_TFTP_VALID_TIMEOUT_VALUE_MIN_SEC                 (1U)
/// TFTP valid timeout value max in seconds
#define SL_TFTP_VALID_TIMEOUT_VALUE_MAX_SEC                 (255U)

/// TFTP error print format
#define SL_TFTP_ERROR_PRINT_FORMAT_STR \
  "[%s] TFTP Error (%u): %s\n"

/// TFTP RRQ print format
#define SL_TFTP_RRQ_PKT_PRINT_FORMAT_STR \
  "{\n"                                  \
  "  \"opcode\": \"RRQ\",\n"             \
  "  \"filename\": \"%s\",\n"            \
  "  \"mode\": \"%s\"\n"                 \
  "}\n"

/// TFTP WRQ print format
#define SL_TFTP_WRQ_PKT_PRINT_FORMAT_STR \
  "{\n"                                  \
  "  \"opcode\": \"WRQ\",\n"             \
  "  \"filename\": \"%s\",\n"            \
  "  \"mode\": \"%s\"\n"                 \
  "}\n"

/// TFTP ACK packet print format
#define SL_TFTP_ACK_PKT_PRINT_FORMAT_STR \
  "{\n"                                  \
  "  \"opcode\": \"ACK\",\n"             \
  "  \"block\": \"%u\"\n"                \
  "}\n"

/// TFTP Error packet print format
#define SL_TFTP_ERR_PKT_PRINT_FORMAT_STR \
  "{\n"                                  \
  "  \"opcode\": \"ERROR\",\n"           \
  "  \"errcode\": \"%u\",\n"             \
  "  \"errmsg\": \"%s\"\n"               \
  "}\n"

/// TFTP Data packet print format
#define SL_TFTP_DATA_PKT_PRINT_FORMAT_STR \
  "{\n"                                   \
  "  \"opcode\": \"DATA\",\n"             \
  "  \"block\": \"%u\",\n"                \
  "  \"size\": \"%u\"\n"                  \
  "}\n"

/// TFTP OACK packet print format
#define SL_TFTP_OACK_PKT_PRINT_FORMAT_STR \
  "{\n"                                   \
  "  \"opcode\": \"OACK\",\n"             \
  "  \"options\": \"%s\"\n"               \
  "}\n"

/// TFTP client termination requested message
#define SL_TFTP_CLNT_TERMINATION_MSG              "Termination requested"

/// TFTP Event all mask
#define SL_TFTP_EVT_ALL_MSK                       (0x00FFFFFFUL)

/// Error mask
#define SL_TFTP_EVT_ERROR_MSK                     (0x00000001UL << 31UL)

/// Get request mask
#define SL_TFTP_EVT_GET_REQUEST_MSK               (0x00000001UL << 0UL)

/// Put request mask
#define SL_TFTP_EVT_PUT_REQUEST_MSK               (0x00000001UL << 1UL)

/// Operation finished mask
#define SL_TFTP_EVT_OP_FINISHED_MSK               (0x00000001UL << 2UL)

/// RRQ/WRQ operation error mask due to receive timeout
#define SL_TFTP_EVT_RRQ_WRQ_ERROR_MSK             (0x00000001UL << 3UL)

/// TFTP error code mask - Not defined, see error message (if any).
#define SL_TFTP_EVT_NOTDEF_ERROR_MSK              (0x00000001UL << 4UL)

/// TFTP error code mask - File not found.
#define SL_TFTP_EVT_FILE_NOT_FOUND_ERROR_MSK      (0x00000001UL << 5UL)

/// TFTP error code mask - Access violation.
#define SL_TFTP_EVT_ACCESS_VIOLATION_ERROR_MSK    (0x00000001UL << 6UL)

/// TFTP error code mask - Disk full or allocation exceeded.
#define SL_TFTP_EVT_DISK_FULL_ERROR_MSK           (0x00000001UL << 7UL)

/// TFTP error code mask - Illegal TFTP operation.
#define SL_TFTP_EVT_ILLEGAL_OP_ERROR_MSK          (0x00000001UL << 8UL)

/// TFTP error code mask - Unknown transfer ID.
#define SL_TFTP_EVT_UNKNOWN_TID_ERROR_MSK         (0x00000001UL << 9UL)

/// TFTP error code mask - File already exists.
#define SL_TFTP_EVT_FILE_EXISTS_ERROR_MSK         (0x00000001UL << 10UL)

/// TFTP error code mask - No such user.
#define SL_TFTP_EVT_NO_USER_ERROR_MSK             (0x00000001UL << 11UL)

/// TFTP error code mask - Terminate transfer due to option negotiation.
#define SL_TFTP_EVT_OPT_NEGOT_FAIL_ERROR_MSK      (0x00000001UL << 12UL)

// Option value string buffer max length (including null terminator)
#define SL_TFTP_OPT_VAL_MAX_LEN                   (11U)

/// Swap bytes in uint16
#define __swap_bytes_16bit(__val) \
  ((uint16_t)(((__val) >> 8U) | ((__val) << 8U)))

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief Default Data handler
 * @details Default callback to dump data in hex format
 * @param[in,out] clnt Client
 * @param[in] data_ptr Data ptr
 * @param[in] data_size Data size
 * @param[in] blocksize Option extension block size
 ******************************************************************************/
static void _default_data_hnd(sl_tftp_clnt_t * const clnt,
                              const uint8_t * const data_ptr,
                              const uint16_t data_size);

/***************************************************************************//**
 * @brief Default Error handler
 * @details Default callback to print error code and error message
 * @param[in,out] clnt Client
 * @param[in] error_code Error code
 * @param[in] error_msg Error message
 ******************************************************************************/
__STATIC_INLINE void _default_error_hnd(sl_tftp_clnt_t * const clnt,
                                        const uint16_t error_code,
                                        const char *error_msg);

/***************************************************************************//**
 * @brief TFTP Client service thread function
 * @details Thread function
 * @param[in] args Arguments (not used)
 ******************************************************************************/
static void _clnt_thr_fnc(void *args);

/***************************************************************************//**
 * @brief Is flag mask set
 * @details Helper function
 * @param[in] clnt Client
 * @param[in] flags_msk Mask
 * @return True if mask is set, otherwise false
 ******************************************************************************/
__STATIC_INLINE bool _is_flags_set(const sl_tftp_clnt_t * const clnt, uint32_t flags_msk);

/***************************************************************************//**
 * @brief Check pointer in buffer
 * @details Helper function
 * @param ptr Pointer
 * @param buff_ptr Buffer start pointer
 * @param buff_size Buffer size
 * @return bool True if it's in the buffer, otherwise False
 ******************************************************************************/
__STATIC_INLINE bool _is_ptr_in_buff(const uint8_t * const ptr,
                                     const uint8_t * const buff_ptr,
                                     const uint16_t buff_size);

/***************************************************************************//**
 * @brief Prepare and send ack packet
 * @details Helper function
 * @param[in,out] clnt Client
 * @param[in] block_num Block number
 * @param[in] sock_id Socket ID
 * @param[in,out] buff Buffer
 * @param[in] blocksize Size of block
 * @param[in] host_addr Host address bytes
 ******************************************************************************/
static void _prepare_and_send_ack(sl_tftp_clnt_t * const clnt,
                                  const uint16_t block_num,
                                  int32_t sock_id,
                                  uint8_t *buff,
                                  uint16_t blocksize,
                                  void *host_addr);

/***************************************************************************//**
 * @brief Prepare and send data packet
 * @details Helper function
 * @param[in,out] clnt Clinet
 * @param[in] block_num Block number
 * @param[in] data_ptr Data ptr
 * @param[in] data_size Data size
 * @param[in] sock_id Socket ID
 * @param[in,out] buff Buffer
 * @param[in] blocksize Size of block
 * @param[in] host_addr Host address bytes
 ******************************************************************************/
static void _prepare_and_send_data(sl_tftp_clnt_t * const clnt,
                                   const uint16_t block_num,
                                   const uint8_t *data_ptr,
                                   const uint16_t data_size,
                                   int32_t sock_id,
                                   uint8_t *buff,
                                   uint16_t blocksize,
                                   void *host_addr);

/***************************************************************************//**
 * @brief Prepare and send error packet
 * @details Helper function
 * @param[in,out] clnt Client
 * @param[in] error_code Error code
 * @param[in] error_msg Error message
 * @param[in] sock_id Socket ID
 * @param[in,out] buff Buffer
 * @param[in] blocksize Size of block
 * @param[in] host_addr Host address bytes
 ******************************************************************************/
static void _prepare_and_send_error_pkt(sl_tftp_clnt_t * const clnt,
                                        const uint16_t error_code,
                                        const char *error_msg,
                                        int32_t sock_id,
                                        uint8_t *buff,
                                        uint16_t blocksize,
                                        void *host_addr);

/***************************************************************************//**
 * @brief Calculate option size
 * @details Helper function
 * @param[in] clnt TFTP Client
 * @return uint32_t Size of option
 ******************************************************************************/
static uint32_t _calc_option_size(const sl_tftp_clnt_t * const clnt);

/***************************************************************************//**
 * @brief Calculate packet size
 * @details Helper function
 * @param[in] clnt TFTP Client
 * @return uint32_t Size of packet
 ******************************************************************************/
static uint32_t _calc_packet_size(const sl_tftp_clnt_t * const clnt);

/***************************************************************************//**
 * @brief Build packet buffer
 * @details Helper function
 * @param[in] clnt TFTP client
 * @param[in,out] buff Buffer
 * @param[in] buff_size Max size of buffer
 * @return uint32_t Built packet size on success, otherwise 0
 ******************************************************************************/
static uint32_t _build_packet(sl_tftp_clnt_t * const clnt,
                              uint8_t *buff,
                              const uint16_t buff_size);

/***************************************************************************//**
 * @brief Parse packet buffer
 * @details Helper function
 * @param[in,out] clnt TFTP client
 * @param[in] buff Buffer
 * @param[in] buff_size Buffer payload size
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 ******************************************************************************/
static sl_status_t _parse_packet(sl_tftp_clnt_t * const clnt,
                                 const uint8_t * const buff,
                                 const uint16_t buff_size);

/***************************************************************************//**
 * @brief Read Request handler
 * @details Handling socket IO to read file from remote host
 * @param[in,out] clnt Client
 * @param[in,out] buff Buffer
 * @param[in] buff_size Buffer size
 ******************************************************************************/
static void _rrq_hnd(sl_tftp_clnt_t * const clnt,
                     uint8_t **buff,
                     const uint16_t buff_size);

/***************************************************************************//**
 * @brief Write Request handler
 * @details Handling socket IO to read file from remote host
 * @param[in,out] clnt Client
 * @param[in,out] buff Buffer
 * @param[in] buff_size Buffer size
 ******************************************************************************/
static void _wrq_hnd(sl_tftp_clnt_t * const clnt,
                     uint8_t **buff,
                     const uint16_t buff_size);

/***************************************************************************//**
 * @brief Set TFTP error event flag
 * @details Helper function
 * @param[in] clnt TFTP client
 * @param[in] errcode Error code
 ******************************************************************************/
static void _set_tftp_err_evt_flag(const sl_tftp_clnt_t * const clnt,
                                   const uint16_t errcode);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// Thread ID
static osThreadId_t _clnt_thr = NULL;

/// TFTP Client task attribute
static const osThreadAttr_t _clnt_thr_attr = {
  .name        = "TftpClntThread",
  .attr_bits   = osThreadDetached,
  .cb_mem      = NULL,
  .cb_size     = 0,
  .stack_mem   = NULL,
  .stack_size  = (SL_TFTP_CLNT_STACK_SIZE_WORD * sizeof(void *)) & 0xFFFFFFF8U,
  .priority    = osPriorityNormal7,
  .tz_module   = 0
};

/// TFTP Client message queue ID
static osMessageQueueId_t _msg_queue_in = NULL;

/// TFTP Client message queue attributes
static const osMessageQueueAttr_t _msg_queue_in_attr = {
  .name      = "TftpClntMsgQueue",
  .attr_bits = 0,
  .cb_mem    = NULL,
  .cb_size   = 0,
  .mq_mem    = NULL,
  .mq_size   = 0
};

/// TFTP Client event flag attributes
static const osEventFlagsAttr_t _clnt_evt_attr = {
  .name      = "TftpClntEvt",
  .attr_bits = 0,
  .cb_mem    = NULL,
  .cb_size   = 0
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_tftp_clnt_service_init(void)
{
  _msg_queue_in = osMessageQueueNew(SL_TFTP_MESSAGE_QUEUE_SIZE,
                                    sizeof(sl_tftp_clnt_t),
                                    &_msg_queue_in_attr);
  EFM_ASSERT(_msg_queue_in != NULL);

  _clnt_thr = osThreadNew(_clnt_thr_fnc, NULL, &_clnt_thr_attr);
  EFM_ASSERT(_clnt_thr != NULL);
}

sl_status_t sl_tftp_clnt_init(sl_tftp_clnt_t * const clnt,
                              const char *host,
                              const uint16_t port,
                              sl_tftp_clnt_data_hnd_t data_hnd,
                              sl_tftp_clnt_error_hnd_t error_hnd)
{
  if (clnt == NULL) {
    return SL_STATUS_FAIL;
  }

  clnt->evt_flags = osEventFlagsNew(&_clnt_evt_attr);
  if (clnt->evt_flags == NULL) {
    return SL_STATUS_FAIL;
  }

  clnt->host = host == NULL ? SL_TFTP_LOCAL_HOST_STR : host;
  clnt->port = port;

  clnt->data_hnd = data_hnd == NULL ? &_default_data_hnd : data_hnd;
  clnt->error_hnd = error_hnd == NULL ? &_default_error_hnd : error_hnd;

  // Default options
  clnt->options.blksize = SL_TFTP_DEFAULT_DATA_BLOCK_SIZE;
  clnt->options.timeout_sec = SL_TFTP_DEFAULT_SRV_RET_TIMEOUT_SEC;

  return SL_STATUS_OK;
}

void sl_tftp_clnt_print_pkt(const sl_tftp_pkt_t * const pkt)
{
  uint8_t *str_buff = NULL;

  if (pkt == NULL) {
    return;
  }

  switch (pkt->opcode) {
    case SL_TFTP_OPCODE_RRQ:
      printf(SL_TFTP_RRQ_PKT_PRINT_FORMAT_STR,
             pkt->content.request.filename,
             pkt->content.request.mode);
      break;

    case SL_TFTP_OPCODE_WRQ:
      printf(SL_TFTP_WRQ_PKT_PRINT_FORMAT_STR,
             pkt->content.request.filename,
             pkt->content.request.mode);
      break;

    case SL_TFTP_OPCODE_DATA:
      printf(SL_TFTP_DATA_PKT_PRINT_FORMAT_STR,
             pkt->content.data.block_num,
             pkt->content.data.data_size);
      break;

    case SL_TFTP_OPCODE_ACK:
      printf(SL_TFTP_ACK_PKT_PRINT_FORMAT_STR,
             pkt->content.ack.block_num);
      break;

    case SL_TFTP_OPCODE_ERROR:
      printf(SL_TFTP_ERR_PKT_PRINT_FORMAT_STR,
             pkt->content.error.errcode,
             pkt->content.error.errmsg);
      break;

    case SL_TFTP_OPCODE_OACK:
      if (!pkt->content.oack.size
          || pkt->content.oack.opt_list == NULL) {
        return;
      }
      str_buff = (uint8_t *) sl_malloc(pkt->content.oack.size);
      if (str_buff == NULL ) {
        return;
      }
      for (uint16_t i = 0U; i < pkt->content.oack.size; ++i) {
        if (pkt->content.oack.opt_list[i] == '\0') {
          str_buff[i] = ' ';
        } else {
          str_buff[i] = pkt->content.oack.opt_list[i];
        }
      }
      str_buff[pkt->content.oack.size - 1U] = '\0';
      printf(SL_TFTP_OACK_PKT_PRINT_FORMAT_STR, str_buff);
      sl_free(str_buff);
      break;

    default:
      printf("[TFTP invalid packet: %u]\n", pkt->opcode);
      break;
  }
}

sl_status_t sl_tftp_clnt_set_option(sl_tftp_clnt_t * const clnt,
                                    const char *opt,
                                    const uint32_t value)
{
  if (clnt == NULL || opt == NULL) {
    return SL_STATUS_FAIL;
  }
  // Blocksize option
  if (!strncmp(opt, SL_TFTP_OPT_EXT_BLOCKSIZE, SL_TFTP_STR_MAX_LEN)
      && value >= SL_TFTP_VALID_BLOCKSIZE_VALUE_MIN
      && value <= SL_TFTP_VALID_BLOCKSIZE_VALUE_MAX) {
    clnt->options.blksize = value;

    // Timeout option
  } else if (!strncmp(opt, SL_TFTP_OPT_EXT_TIMEOUT_INTERVAL, SL_TFTP_STR_MAX_LEN)
             && value >= SL_TFTP_VALID_TIMEOUT_VALUE_MIN_SEC
             && value <= SL_TFTP_VALID_TIMEOUT_VALUE_MAX_SEC) {
    clnt->options.timeout_sec = value;
  } else {
    // Not supported option
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t sl_tftp_clnt_request(sl_tftp_clnt_t * const clnt,
                                 const uint16_t opcode,
                                 const char *file,
                                 const char *mode)
{
  osStatus_t status = osError;

  if (clnt == NULL
      || file == NULL
      || mode == NULL
      || !(opcode == SL_TFTP_OPCODE_RRQ || opcode == SL_TFTP_OPCODE_WRQ)) {
    printf("[TFTP request error]\n");
    return SL_STATUS_FAIL;
  }

  clnt->packet.opcode = opcode;
  clnt->packet.content.request.filename = file;
  clnt->packet.content.request.mode = mode;

  status = osMessageQueuePut(_msg_queue_in, clnt, 0U, osWaitForever);

  return status != osOK ? SL_STATUS_FAIL : SL_STATUS_OK;
}

sl_status_t sl_tftp_clnt_terminate_session(sl_tftp_clnt_t * const clnt)
{
  uint32_t flags = 0UL;

  if (clnt == NULL) {
    printf("[TFTP terminate error]\n");
    return SL_STATUS_FAIL;
  }

  flags = osEventFlagsClear(clnt->evt_flags, SL_TFTP_EVT_ALL_MSK);
  if (flags & SL_TFTP_EVT_ERROR_MSK) {
    return SL_STATUS_FAIL;
  }

  flags = osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_OP_FINISHED_MSK);
  if (flags & SL_TFTP_EVT_ERROR_MSK) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

bool sl_tftp_clnt_is_op_finished(const sl_tftp_clnt_t * const clnt)
{
  return _is_flags_set(clnt, SL_TFTP_EVT_OP_FINISHED_MSK);
}

bool sl_tftp_clnt_is_op_get(const sl_tftp_clnt_t * const clnt)
{
  return _is_flags_set(clnt, SL_TFTP_EVT_GET_REQUEST_MSK);
}

bool sl_tftp_clnt_is_op_put(const sl_tftp_clnt_t * const clnt)
{
  return _is_flags_set(clnt, SL_TFTP_EVT_GET_REQUEST_MSK);
}

bool sl_tftp_clnt_is_op_rrq_wrq_failed(const sl_tftp_clnt_t * const clnt)
{
  return _is_flags_set(clnt, SL_TFTP_EVT_RRQ_WRQ_ERROR_MSK);
}

sl_status_t sl_tftp_clnt_wait_op_finished(const sl_tftp_clnt_t * const clnt)
{
  uint32_t ret = 0UL;
  ret = osEventFlagsWait(clnt->evt_flags,
                         SL_TFTP_EVT_OP_FINISHED_MSK,
                         osFlagsWaitAll | osFlagsNoClear,
                         osWaitForever);
  return (ret & SL_TFTP_EVT_ERROR_MSK) ? SL_STATUS_FAIL : SL_STATUS_OK;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void _set_tftp_err_evt_flag(const sl_tftp_clnt_t * const clnt,
                                   const uint16_t errcode)
{
  switch (errcode) {
    case SL_TFTP_ERRORCODE_NOTDEF:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_NOTDEF_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_FNOTFOUND:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_FILE_NOT_FOUND_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_ACCVIOL:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_ACCESS_VIOLATION_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_DISKFULL:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_DISK_FULL_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_ILLEGALOP:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_ILLEGAL_OP_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_UNKNTID:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_UNKNOWN_TID_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_FEXIST:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_FILE_EXISTS_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_NOUSR:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_NO_USER_ERROR_MSK);
      break;

    case SL_TFTP_ERRORCODE_OPTNEGOTFAIL:
      osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_OPT_NEGOT_FAIL_ERROR_MSK);
      break;

    default:
      // Not defined error
      break;
  }
}

static sl_status_t _parse_packet(sl_tftp_clnt_t * const clnt,
                                 const uint8_t *buff,
                                 const uint16_t buff_size)
{
  uint8_t *ptr = NULL;
  uint32_t str_len = 0UL;
  uint8_t *opt_val_str = NULL;

  if (buff == NULL || clnt == NULL) {
    return SL_STATUS_FAIL;
  }

  ptr = (uint8_t *)buff;

  // Parse opcode
  clnt->packet.opcode = __swap_bytes_16bit(*((uint16_t *)ptr));
  ptr += sizeof(uint16_t);

  if (!_is_ptr_in_buff(ptr, buff, buff_size)) {
    return SL_STATUS_FAIL;
  }

  // Data packet
  if (clnt->packet.opcode == SL_TFTP_OPCODE_DATA) {
    clnt->packet.content.data.block_num = __swap_bytes_16bit(*((uint16_t *)ptr));
    ptr += sizeof(uint16_t);
    if (!_is_ptr_in_buff(ptr, buff, buff_size)) {
      return SL_STATUS_FAIL;
    }
    // Set data
    clnt->packet.content.data.data = ptr;
    // Set data size
    clnt->packet.content.data.data_size = buff_size - 2U * sizeof(uint16_t);

    // ACK packet
  } else if (clnt->packet.opcode == SL_TFTP_OPCODE_ACK) {
    clnt->packet.content.ack.block_num = __swap_bytes_16bit(*((uint16_t *)ptr));

    // Error packet
  } else if (clnt->packet.opcode == SL_TFTP_OPCODE_ERROR) {
    clnt->packet.content.error.errcode = __swap_bytes_16bit(*((uint16_t *)ptr));
    ptr += sizeof(uint16_t);

    if (!_is_ptr_in_buff(ptr, buff, buff_size)) {
      return SL_STATUS_FAIL;
    }

    clnt->packet.content.error.errmsg = (const char *)ptr;

    // OACK packet
  } else if (clnt->packet.opcode == SL_TFTP_OPCODE_OACK) {
    clnt->packet.content.oack.opt_list = (const char *)ptr;
    clnt->packet.content.oack.size = 0;

    // Parse option extension
    while (_is_ptr_in_buff(ptr, buff, buff_size)) {
      str_len = sl_strnlen((char *)ptr, SL_TFTP_STR_MAX_LEN);
      clnt->packet.content.oack.size += str_len + 1U;
      opt_val_str = ptr + str_len + 1U;

      str_len = sl_strnlen((char *)opt_val_str, SL_TFTP_OPT_VAL_MAX_LEN);
      clnt->packet.content.oack.size += str_len + 1;

      if (!_is_ptr_in_buff(opt_val_str, buff, buff_size)
          || str_len >= SL_TFTP_OPT_VAL_MAX_LEN) {
        return SL_STATUS_FAIL;
      }

      // Get option: block size
      if (!strncmp((const char *)ptr, SL_TFTP_OPT_EXT_BLOCKSIZE, SL_TFTP_STR_MAX_LEN)) {
        clnt->options.blksize = (uint32_t)atol((const char *)(opt_val_str));

        if (clnt->options.blksize < SL_TFTP_VALID_BLOCKSIZE_VALUE_MIN
            || clnt->options.blksize > SL_TFTP_VALID_BLOCKSIZE_VALUE_MAX) {
          return SL_STATUS_FAIL;
        }

        // Get option: timeout sec
      } else if (!strncmp((const char *)ptr, SL_TFTP_OPT_EXT_TIMEOUT_INTERVAL, SL_TFTP_STR_MAX_LEN)) {
        clnt->options.timeout_sec = (uint32_t)atol((const char *)(opt_val_str));
        if (clnt->options.timeout_sec < SL_TFTP_VALID_TIMEOUT_VALUE_MIN_SEC
            || clnt->options.timeout_sec > SL_TFTP_VALID_TIMEOUT_VALUE_MAX_SEC) {
          return SL_STATUS_FAIL;
        }
      } else {
        // Not supported option
        return SL_STATUS_FAIL;
      }

      // Get value
      ptr = opt_val_str + str_len + 1U;
    }
  } else {
    // Invalid opcode
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static uint32_t _build_packet(sl_tftp_clnt_t * const clnt,
                              uint8_t *buff,
                              const uint16_t buff_size)
{
  uint32_t req_buff_size      = 0UL;
  uint8_t *ptr                = NULL;
  int32_t res                 = -1L;

  if (buff == NULL || clnt == NULL) {
    return 0UL;
  }

  req_buff_size = _calc_packet_size(clnt);
  if (req_buff_size > (uint32_t)(buff_size + sizeof(sl_tftp_pkt_t))) {
    return 0UL;
  }

  ptr = buff;

  // Set opcode
  *((uint16_t *)ptr) = __swap_bytes_16bit(clnt->packet.opcode);
  ptr += sizeof(uint16_t);

  // RRQ or WRQ request
  if ((clnt->packet.opcode == SL_TFTP_OPCODE_RRQ)
      || (clnt->packet.opcode == SL_TFTP_OPCODE_WRQ)) {
    res = snprintf((char *)ptr, req_buff_size - sizeof(uint16_t),
                   "%s%c%s", clnt->packet.content.request.filename, '\0',
                   clnt->packet.content.request.mode);
    if (res < 0L) {
      return 0UL;
    }
    ptr += res + 1U;

    // Option extension
    if (clnt->options.blksize != SL_TFTP_DEFAULT_DATA_BLOCK_SIZE) {
      res = snprintf((char *)ptr, req_buff_size - (ptr - buff),
                     "%s%c%u", SL_TFTP_OPT_EXT_BLOCKSIZE,
                     '\0', (uint16_t)clnt->options.blksize);
      if (res < 0L) {
        return 0UL;
      }
      ptr += res + 1U;
    }

    if (clnt->options.timeout_sec != SL_TFTP_DEFAULT_SRV_RET_TIMEOUT_SEC) {
      res = snprintf((char *)ptr, req_buff_size - (ptr - buff),
                     "%s%c%u", SL_TFTP_OPT_EXT_TIMEOUT_INTERVAL,
                     '\0', (uint16_t)clnt->options.timeout_sec);
      if (res < 0L) {
        return 0UL;
      }
    }

    return res < 0L ? 0UL : req_buff_size;

    // Data packet
  } else if (clnt->packet.opcode == SL_TFTP_OPCODE_DATA) {
    *((uint16_t *)ptr) = __swap_bytes_16bit(clnt->packet.content.data.block_num);
    ptr += sizeof(uint16_t);
    memcpy(ptr, clnt->packet.content.data.data, clnt->packet.content.data.data_size);

    // ACK packet
  } else if (clnt->packet.opcode == SL_TFTP_OPCODE_ACK) {
    *((uint16_t *)ptr) = __swap_bytes_16bit(clnt->packet.content.ack.block_num);

    // Error packet
  } else if (clnt->packet.opcode == SL_TFTP_OPCODE_ERROR) {
    *((uint16_t *)ptr) = __swap_bytes_16bit(clnt->packet.content.error.errcode);
    ptr += sizeof(uint16_t);
    if (clnt->packet.content.error.errmsg != NULL) {
      res = snprintf((char *)ptr,
                     req_buff_size - sizeof(uint16_t) * 2U,
                     "%s",
                     clnt->packet.content.error.errmsg);
      req_buff_size = res < 0L ? 0UL : req_buff_size;
    }
  } else {
    // Invalid opcode
    return 0UL;
  }

  return req_buff_size;
}

static uint32_t _calc_option_size(const sl_tftp_clnt_t * const clnt)
{
  static uint8_t tmp_buff[SL_TFTP_OPT_VAL_MAX_LEN] = { 0 };
  int32_t val_len = 0L;
  uint32_t tot_opt_len = 0UL;

  // Blocksize option required
  // opt + null terminator + value + null terminator
  if (clnt->options.blksize != SL_TFTP_DEFAULT_DATA_BLOCK_SIZE) {
    val_len = snprintf((char *)tmp_buff, SL_TFTP_OPT_VAL_MAX_LEN, "%lu", clnt->options.blksize);

    if (val_len < 0L) {
      return 0UL;
    }
    tot_opt_len += SL_TFTP_OPT_EXT_BLOCKSIZE_LEN
                   + sizeof(char)
                   + sl_strnlen((char *)tmp_buff, SL_TFTP_OPT_VAL_MAX_LEN)
                   + sizeof(char);
  }

  // Timeout interval option required
  // opt + null terminator + value + null terminator
  if (clnt->options.timeout_sec != SL_TFTP_DEFAULT_SRV_RET_TIMEOUT_SEC) {
    val_len = snprintf((char *)tmp_buff, SL_TFTP_OPT_VAL_MAX_LEN, "%lu", clnt->options.timeout_sec);

    if (val_len < 0L) {
      return 0UL;
    }
    tot_opt_len += SL_TFTP_OPT_EXT_TIMEOUT_INTERVAL_LEN
                   + sizeof(char)
                   + sl_strnlen((char *)tmp_buff, SL_TFTP_OPT_VAL_MAX_LEN)
                   + sizeof(char);
  }

  return tot_opt_len;
}

static uint32_t _calc_packet_size(const sl_tftp_clnt_t * const clnt)
{
  uint32_t pkt_size = 0UL;

  switch (clnt->packet.opcode) {
    // opcode + filename + null terminator + mode + null terminator
    case SL_TFTP_OPCODE_RRQ:
    case SL_TFTP_OPCODE_WRQ:
      pkt_size = sizeof(uint16_t)
                 + sl_strnlen((char *)clnt->packet.content.request.filename, SL_TFTP_STR_MAX_LEN)
                 + sizeof(uint8_t)
                 + sl_strnlen((char *)clnt->packet.content.request.mode, SL_TFTP_STR_MAX_LEN)
                 + sizeof(uint8_t)
                 + _calc_option_size(clnt);
      break;

    case SL_TFTP_OPCODE_DATA:
      // opcode + block number + data
      pkt_size = sizeof(uint16_t)
                 + sizeof(uint16_t)
                 + clnt->packet.content.data.data_size;
      break;

    case SL_TFTP_OPCODE_ACK:
      // opcode + block number
      pkt_size = sizeof(uint16_t)
                 + sizeof(uint16_t);
      break;

    case SL_TFTP_OPCODE_ERROR:
      // opcode + error code + error message + null terminator
      pkt_size = sizeof(uint16_t)
                 + sizeof(uint16_t)
                 + sl_strnlen((char *)clnt->packet.content.error.errmsg, SL_TFTP_STR_MAX_LEN)
                 + sizeof(uint8_t);
      break;

    default:
      // Nothing to do
      break;
  }

  return pkt_size;
}

__STATIC_INLINE bool _is_flags_set(const sl_tftp_clnt_t * const clnt, uint32_t flags_msk)
{
  uint32_t flags = 0UL;
  flags = osEventFlagsGet(clnt->evt_flags);
  return (bool) (flags & flags_msk);
}

__STATIC_INLINE bool _is_ptr_in_buff(const uint8_t * const ptr,
                                     const uint8_t * const buff_ptr,
                                     const uint16_t buff_size)
{
  return (bool) ((ptr >= buff_ptr) && (ptr < (buff_ptr + buff_size)));
}

static void _default_data_hnd(sl_tftp_clnt_t * const clnt,
                              const uint8_t * const data_ptr,
                              const uint16_t data_size)
{
  printf("[%s]\n", clnt->host);
  sl_tftp_dump_buff(data_ptr, data_size);
}

__STATIC_INLINE void _default_error_hnd(sl_tftp_clnt_t * const clnt,
                                        const uint16_t error_code,
                                        const char *error_msg)
{
  printf(SL_TFTP_ERROR_PRINT_FORMAT_STR, clnt->host, error_code, error_msg);
}

static void _prepare_and_send_ack(sl_tftp_clnt_t * const clnt,
                                  const uint16_t block_num,
                                  int32_t sock_id,
                                  uint8_t *buff,
                                  uint16_t blocksize,
                                  void *host_addr)
{
  uint32_t pkt_payload_size = 0U;

  clnt->packet.opcode = SL_TFTP_OPCODE_ACK;
  clnt->packet.content.ack.block_num = block_num;
  pkt_payload_size = _build_packet(clnt, buff, blocksize);
  (void) sl_tftp_udp_sendto(sock_id, buff, pkt_payload_size, host_addr);
}

static void _prepare_and_send_error_pkt(sl_tftp_clnt_t * const clnt,
                                        const uint16_t error_code,
                                        const char* error_msg,
                                        int32_t sock_id,
                                        uint8_t *buff,
                                        uint16_t blocksize,
                                        void *host_addr)
{
  uint32_t pkt_payload_size = 0U;

  clnt->packet.opcode = SL_TFTP_OPCODE_ERROR;
  clnt->packet.content.error.errcode = error_code;
  clnt->packet.content.error.errmsg = error_msg;
  pkt_payload_size = _build_packet(clnt, buff, blocksize);
  (void) sl_tftp_udp_sendto(sock_id, buff, pkt_payload_size, host_addr);
}

static void _prepare_and_send_data(sl_tftp_clnt_t * const clnt,
                                   const uint16_t block_num,
                                   const uint8_t *data_ptr,
                                   const uint16_t data_size,
                                   int32_t sock_id,
                                   uint8_t *buff,
                                   uint16_t blocksize,
                                   void *host_addr)
{
  uint32_t pkt_payload_size = 0U;

  clnt->packet.opcode = SL_TFTP_OPCODE_DATA;
  clnt->packet.content.data.block_num = block_num;
  clnt->packet.content.data.data = (uint8_t *) data_ptr;
  clnt->packet.content.data.data_size = data_size;
  pkt_payload_size = _build_packet(clnt, buff, blocksize);
  (void) sl_tftp_udp_sendto(sock_id, buff, pkt_payload_size, host_addr);
}

static void _rrq_hnd(sl_tftp_clnt_t * const clnt,
                     uint8_t **buff,
                     const uint16_t buff_size)
{
  int32_t sock_id           = SL_TFTP_INVALID_SOCKID;
  uint32_t timeout          = 0UL;
  uint32_t pkt_payload_size = 0UL;
  uint16_t block_num        = 1U;
  void *host_addr           = NULL;
  int32_t res               = SL_FTP_ERROR;
  uint16_t blocksize        = SL_TFTP_DEFAULT_DATA_BLOCK_SIZE;

  sock_id = sl_tftp_udp_socket_create();
  if (sock_id == SL_TFTP_INVALID_SOCKID) {
    sl_tftp_delay_ms(100UL);
    return;
  }

  host_addr = sl_tftp_udp_get_addr(clnt->host, clnt->port);
  if (host_addr == NULL) {
    sl_tftp_socket_close(sock_id);
    printf("[TFTP Address error]\n");
    return;
  }

  pkt_payload_size = _build_packet(clnt, *buff, buff_size);

  (void) sl_tftp_udp_sendto(sock_id, *buff, pkt_payload_size, host_addr);

  while (timeout < SL_TFTP_CLNT_RECV_TIMEOUT_MS) {
    sl_tftp_delay_ms(100UL);

    if (_is_flags_set(clnt, SL_TFTP_EVT_OP_FINISHED_MSK)) {
      _prepare_and_send_error_pkt(clnt,
                                  SL_TFTP_ERRORCODE_NOTDEF,
                                  SL_TFTP_CLNT_TERMINATION_MSG,
                                  sock_id,
                                  *buff,
                                  blocksize,
                                  host_addr);
      break;
    }

    res = sl_tftp_udp_recvfrom(sock_id, *buff, blocksize + sizeof(sl_tftp_pkt_t), host_addr);
    if (res <= 0L) {
      timeout += 100UL;
      continue;
    }

    if (_parse_packet(clnt, *buff, res) != SL_STATUS_OK) {
      timeout += 100UL;
      continue;
    }

#if SL_TFTP_DEBUG
    // Print received packet
    sl_tftp_clnt_print_pkt(&clnt->packet);
#endif

    // OACK packet
    if (clnt->packet.opcode == SL_TFTP_OPCODE_OACK) {
      // reallocate buffer if blocksize is different
      if (clnt->options.blksize != SL_TFTP_DEFAULT_DATA_BLOCK_SIZE) {
        sl_free(*buff);
        *buff = (uint8_t *)sl_malloc(clnt->options.blksize + sizeof(sl_tftp_pkt_t));
        if (!(*buff)) {
          osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_OP_FINISHED_MSK);
          printf("[TFTP buffer allocation failed]\n");
          return;
        }
        blocksize = clnt->options.blksize;
      }
      // Prepare ACK
      _prepare_and_send_ack(clnt, 0U, sock_id, *buff, blocksize, host_addr);
    }
    // Data packet
    else if (clnt->packet.opcode == SL_TFTP_OPCODE_DATA) {
      // Get block number
      block_num = clnt->packet.content.data.block_num;
      if (clnt->data_hnd != NULL) {
        clnt->data_hnd(clnt,
                       clnt->packet.content.data.data,
                       clnt->packet.content.data.data_size);
      }
      // Prepare ACK
      _prepare_and_send_ack(clnt, block_num, sock_id, *buff, blocksize, host_addr);

      // Last packet
      if (clnt->packet.content.data.data_size < blocksize) {
        break;
      }
    }
    // Error packet
    else if (clnt->packet.opcode == SL_TFTP_OPCODE_ERROR) {
      if (clnt->error_hnd != NULL) {
        clnt->error_hnd(clnt,
                        clnt->packet.content.error.errcode,
                        clnt->packet.content.error.errmsg);
      }
      _set_tftp_err_evt_flag(clnt,
                             clnt->packet.content.error.errcode);
      // Prepare ACK
      _prepare_and_send_ack(clnt, block_num, sock_id, *buff, blocksize, host_addr);
    }

    timeout = 0UL;
  }

  if (timeout >= SL_TFTP_CLNT_RECV_TIMEOUT_MS) {
    osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_RRQ_WRQ_ERROR_MSK);
  }

  sl_tftp_udp_free_addr(host_addr);
  sl_tftp_socket_close(sock_id);
}

static void _wrq_hnd(sl_tftp_clnt_t * const clnt,
                     uint8_t **buff,
                     const uint16_t buff_size)
{
  int32_t sock_id             = SL_TFTP_INVALID_SOCKID;
  uint32_t timeout            = 0UL;
  uint32_t pkt_payload_size   = 0UL;
  uint16_t block_num          = 0U;
  uint32_t required_block_num = 0UL;
  void *host_addr             = NULL;
  int32_t res                 = SL_FTP_ERROR;
  uint8_t *ptr                = NULL;
  uint32_t remained_size      = 0UL;
  uint16_t data_size          = 0U;
  uint16_t blocksize          = SL_TFTP_DEFAULT_DATA_BLOCK_SIZE;

  sock_id = sl_tftp_udp_socket_create();
  if (sock_id == SL_TFTP_INVALID_SOCKID) {
    sl_tftp_delay_ms(100UL);
    return;
  }

  host_addr = sl_tftp_udp_get_addr(clnt->host, clnt->port);
  if (host_addr == NULL) {
    sl_tftp_socket_close(sock_id);
    printf("[TFTP Address error]\n");
    return;
  }

  pkt_payload_size = _build_packet(clnt, *buff, buff_size);
  (void) sl_tftp_udp_sendto(sock_id, *buff, pkt_payload_size, host_addr);

  ptr = (uint8_t *) clnt->ext_data;
  remained_size = clnt->ext_data_size;
  required_block_num = remained_size / blocksize + 1U;

  while (timeout < SL_TFTP_CLNT_RECV_TIMEOUT_MS) {
    sl_tftp_delay_ms(100UL);

    if (_is_flags_set(clnt, SL_TFTP_EVT_OP_FINISHED_MSK)) {
      _prepare_and_send_error_pkt(clnt,
                                  SL_TFTP_ERRORCODE_NOTDEF,
                                  SL_TFTP_CLNT_TERMINATION_MSG,
                                  sock_id,
                                  *buff,
                                  blocksize,
                                  host_addr);
      break;
    }

    res = sl_tftp_udp_recvfrom(sock_id, *buff, blocksize + sizeof(sl_tftp_pkt_t), host_addr);
    if (res <= 0L) {
      timeout += 100UL;
      continue;
    }

    if (_parse_packet(clnt, *buff, res) != SL_STATUS_OK) {
      timeout += 100UL;
      continue;
    }
#if SL_TFTP_DEBUG
    // Print received packet
    sl_tftp_clnt_print_pkt(&clnt->packet);
#endif

    // OACK packet
    if (clnt->packet.opcode == SL_TFTP_OPCODE_OACK) {
      // reallocate buffer if blocksize is different
      if (clnt->options.blksize != SL_TFTP_DEFAULT_DATA_BLOCK_SIZE) {
        sl_free(*buff);
        *buff = (uint8_t *)sl_malloc(clnt->options.blksize + sizeof(sl_tftp_pkt_t));
        if (!(*buff)) {
          osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_OP_FINISHED_MSK);
          printf("[TFTP buffer allocation failed]\n");
          return;
        }
        blocksize = clnt->options.blksize;
        required_block_num = remained_size / blocksize + 1U;
      }
      // Prepare ACK
      _prepare_and_send_ack(clnt, 0U, sock_id, *buff, blocksize, host_addr);
    }
    // ACK packet
    else if (clnt->packet.opcode == SL_TFTP_OPCODE_ACK) {
      if (!remained_size && block_num == required_block_num) {
        break;
      }
      // Reset timeout counter
      timeout = 0UL;

      // Store block num
      block_num = clnt->packet.content.ack.block_num;

      // Set ptr and block num
      if (block_num) {
        ptr += blocksize;
      }
      ++block_num;

      // Set data size
      data_size = remained_size < blocksize ? (uint16_t)remained_size : blocksize;
      remained_size -= data_size;
    }
    // Error packet
    else if (clnt->packet.opcode == SL_TFTP_OPCODE_ERROR) {
      if (clnt->error_hnd != NULL) {
        clnt->error_hnd(clnt,
                        clnt->packet.content.error.errcode,
                        clnt->packet.content.error.errmsg);
      }
      _set_tftp_err_evt_flag(clnt,
                             clnt->packet.content.error.errcode);
      break;
    }

    _prepare_and_send_data(clnt,
                           block_num,
                           (const uint8_t *)ptr,
                           data_size,
                           sock_id,
                           *buff,
                           blocksize,
                           host_addr);
#if SL_TFTP_DEBUG
    sl_tftp_clnt_print_pkt(&clnt->packet);
    printf("[%s]\n", clnt->host);
    sl_tftp_dump_buff(ptr, data_size);
#endif
  }

  if (timeout >= SL_TFTP_CLNT_RECV_TIMEOUT_MS) {
    osEventFlagsSet(clnt->evt_flags, SL_TFTP_EVT_RRQ_WRQ_ERROR_MSK);
  }

  sl_tftp_udp_free_addr(host_addr);
  sl_tftp_socket_close(sock_id);
}

static void _clnt_thr_fnc(void * args)
{
  osStatus_t status           = osError;
  uint8_t msg_prio            = 0U;
  uint8_t *buff               = NULL;
  static sl_tftp_clnt_t clnt  = { 0U };

  (void) args;

  // Wait for connected network state
  sl_tftp_wait_for_connection();

  SL_TFTP_SERVICE_LOOP {
    // Pop from the queue
    status = osMessageQueueGet(_msg_queue_in, &clnt, &msg_prio, osWaitForever);
    if (status != osOK) {
      continue;
    }

    (void) osEventFlagsClear(clnt.evt_flags, SL_TFTP_EVT_ALL_MSK);

    buff = (uint8_t *)sl_malloc(SL_TFTP_DEFAULT_BUFF_SIZE);
    EFM_ASSERT(buff);

#if SL_TFTP_DEBUG
    printf("[TFTP Client started]\n");
    sl_tftp_clnt_print_pkt(&clnt.packet);
#endif

    switch (clnt.packet.opcode) {
      case SL_TFTP_OPCODE_RRQ:
        (void) osEventFlagsSet(clnt.evt_flags, SL_TFTP_EVT_GET_REQUEST_MSK);
        _rrq_hnd(&clnt, &buff, SL_TFTP_DEFAULT_DATA_BLOCK_SIZE);
        break;

      case SL_TFTP_OPCODE_WRQ:
        (void) osEventFlagsSet(clnt.evt_flags, SL_TFTP_EVT_PUT_REQUEST_MSK);
        _wrq_hnd(&clnt, &buff, SL_TFTP_DEFAULT_DATA_BLOCK_SIZE);
        break;

      default:
        // Not supported
        break;
    }

    sl_free(buff);

#if SL_TFTP_DEBUG
    printf("[TFTP Client stopped]\n");
#endif

    (void) osEventFlagsSet(clnt.evt_flags, SL_TFTP_EVT_OP_FINISHED_MSK);
  }
}

#endif // SL_FTP_ENABLE_TFTP_PROTOCOL
