/***************************************************************************//**
 * @file
 * @brief NCP host application module.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app_log.h"
#include "app_assert.h"
#include "sl_bt_ncp_host.h"
#include "sl_bgapi.h"
#include "ncp_host.h"
#include "app_sleep.h"
#include "app_timer.h"
#include "ncp_host_config.h"
#include "host_comm_config.h"
#include <stdlib.h>

// Default parameter values.
#define MAX_OPT_LEN                   255

#if PEEK_US_SLEEP == 0
#define MSG_RECV_TIMEOUT_COUNT        10000
#else
#define MSG_RECV_TIMEOUT_COUNT        MSG_RECV_TIMEOUT_MS * 1000 / PEEK_US_SLEEP
#endif

#if defined(SECURITY) && SECURITY == 1
#define SEC_BGAPI_RSP_BASE            4
#define SEC_BGAPI_RSP_BASE_LEN        4
#define SEC_BGAPI_RSP_MSG_LEN         80
#define SEC_BGAPI_CMD_BASE            6
#define SEC_BGAPI_CMD_MSG_LEN         78
#endif // defined(SECURITY) && SECURITY == 1

// RX/TX buffer
typedef struct {
  uint16_t len;
  union {
    uint8_t buf[DEFAULT_HOST_BUFLEN];
    uint32_t header;
  };
} buf_ncp_host_t;

static buf_ncp_host_t buf_ncp_raw = { 0 };
static buf_ncp_host_t buf_ncp_in = { 0 };

static bool wait_for_boot;
static uint8_t boot_retry_count;
static app_timer_t boot_timer;
static bool accept_dfu_boot;

#if defined(SECURITY) && SECURITY == 1
static buf_ncp_host_t buf_ncp_out = { 0 };
static bool enable_security = false;
#endif // defined(SECURITY) && SECURITY == 1

static int32_t ncp_host_peek_timeout(uint32_t len, uint32_t timeout);
#if defined(SECURITY) && SECURITY == 1
static void ncp_sec_host_command_handler(buf_ncp_host_t *buf);
#endif // defined(SECURITY) && SECURITY == 1
static int32_t ncp_host_lazy_peek(void);
static int32_t ncp_host_get_msg(void);
static int32_t ncp_host_get_boot_event(void);
static void on_boot_timer_expire(app_timer_t *timer, void *data);

/**************************************************************************//**
 * Initialize NCP connection.
 *****************************************************************************/
sl_status_t ncp_host_init(void)
{
  sl_status_t sc;

  sc = sl_bt_api_initialize_nonblock(ncp_host_tx, ncp_host_rx, ncp_host_lazy_peek);

  if (sc == SL_STATUS_OK) {
    sc = host_comm_init();
  }

  if (sc == SL_STATUS_OK) {
    int exit_reg_status = atexit(ncp_host_deinit);
    sc = (exit_reg_status == 0) ? SL_STATUS_OK : SL_STATUS_FAIL;
  }

#if defined(SECURITY) && SECURITY == 1
  if (sc == SL_STATUS_OK) {
    sc = security_init();
  }
#endif // defined(SECURITY) && SECURITY == 1

  accept_dfu_boot = ACCEPT_DFU_BOOT;

#if defined(WAIT_FOR_SYSTEM_BOOT) && WAIT_FOR_SYSTEM_BOOT == 1
  if (sc == SL_STATUS_OK) {
    wait_for_boot = true;
    boot_retry_count = 0;
    sc = app_timer_start(&boot_timer,
                         NCP_REBOOT_TIMEOUT_INIT_MS,
                         on_boot_timer_expire,
                         NULL,
                         false);
  }
  #else
  wait_for_boot = false;
  #endif //defined(WAIT_FOR_SYSTEM_BOOT) && WAIT_FOR_SYSTEM_BOOT == 1
  return sc;
}

/**************************************************************************//**
 * Set NCP connection options.
 *****************************************************************************/
sl_status_t ncp_host_set_option(char option, char *value)
{
  sl_status_t sc = SL_STATUS_OK;

  switch (option) {
#if defined(SECURITY) && SECURITY == 1
    case 's':
      enable_security = true;
      break;
#endif // defined(SECURITY) && SECURITY == 1
    default:
      sc = host_comm_set_option((char)option, value);
      break;
  }
  return sc;
}

/**************************************************************************//**
 * Deinitialize NCP connection.
 *****************************************************************************/
void ncp_host_deinit(void)
{
  host_comm_deinit();
}

/**************************************************************************//**
 * Write data to NCP.
 *****************************************************************************/
void ncp_host_tx(uint32_t len, uint8_t* data)
{
  int32_t ret;

#if  defined(SECURITY) && SECURITY == 1
  if (enable_security) {
    security_encrypt((char*)data, (char*)&buf_ncp_out.buf, &len);
    ret = host_comm_tx(len, buf_ncp_out.buf);
  } else
#endif // defined(SECURITY) && SECURITY == 1
  {
    ret = host_comm_tx(len, data);
  }
  if (ret < 0) {
    host_comm_deinit();
    app_assert(false, "Failed to write data\n");
  }
}

/**************************************************************************//**
 * Read data from NCP.
 *****************************************************************************/
int32_t ncp_host_rx(uint32_t len, uint8_t* data)
{
  int32_t ret;
  static uint16_t read_offset = 0;

  if (wait_for_boot) {
    // Wait for the boot event if the target haven't booted yet.
    ret = ncp_host_get_boot_event();
    read_offset = 0;
  } else if (buf_ncp_in.len == 0) {
    ret = ncp_host_get_msg();
    // Finished receiving a brand new, complete NCP message
    read_offset = 0;
  } else {
    // Protocol parser is still processing the previously received message
    ret = buf_ncp_in.len - read_offset;
  }
  if (ret > 0) {
    if (len <= (uint32_t)ret) {
      memcpy(data, &buf_ncp_in.buf[read_offset], len);
      read_offset += len;
    } else {
      ret = -1;
      // Drop seemingly partial messages
      buf_ncp_in.len = 0;
    }
  } else {
    // Reset the length counter until a full message arrives
    buf_ncp_in.len = 0;
    ret = -1;
  }
  return ret;
}

/******************************************************************************
 * Send system reboot command to the NCP target.
 *****************************************************************************/
void ncp_host_reboot(void)
{
  wait_for_boot = true;
  boot_retry_count = 0;
  on_boot_timer_expire(&boot_timer, NULL);
}

/******************************************************************************
 * Reboot the NCP target into DFU mode.
 *****************************************************************************/
void ncp_host_reboot_dfu(void)
{
  wait_for_boot = true;
  accept_dfu_boot = true;

  // This command is equivalent with sl_bt_user_reset_to_dfu
  uint8_t reboot_dfu_command[] = { 0x20, 0x00, 0xff, 0x02 };
  (void)host_comm_tx(sizeof(reboot_dfu_command), reboot_dfu_command);
}

/******************************************************************************
 * Check if any data is available in receive buffer, sleep if empty
 *****************************************************************************/
static int32_t ncp_host_lazy_peek(void)
{
  int32_t ret = host_comm_peek();

  if (ret < 1) {
    app_sleep_us(PEEK_US_SLEEP);
  }
  return ret;
}

/******************************************************************************
 * Check if given amount of data is available in receive buffer within timeout
 *****************************************************************************/
static int32_t ncp_host_peek_timeout(uint32_t len, uint32_t timeout)
{
  int32_t ret;
  uint32_t timeout_counter = 0;

  do {
    ret = host_comm_peek();

    if (ret < (int64_t)len) {
      timeout_counter++;
      app_sleep_us(PEEK_US_SLEEP);
    }
  } while ((ret < (int64_t)len) && (timeout_counter < timeout));

  return ret;
}

/******************************************************************************
 * Assemble complete BGAPI message from the receive buffer
 *****************************************************************************/
static int32_t ncp_host_get_msg(void)
{
  int32_t msg_len;
  int32_t ret;
  uint32_t msg_header;

  msg_len = ncp_host_lazy_peek();
  // wait for the full header before attempting to decode it
  if (msg_len < SL_BGAPI_MSG_HEADER_LEN) {
    return 0;
  }

  // Read the header
  (void)host_comm_rx(SL_BGAPI_MSG_HEADER_LEN, &buf_ncp_raw.buf[0]);
  msg_header = buf_ncp_raw.header;
  msg_len = SL_BGAPI_MSG_LEN(msg_header);
  // Check if length will fit to buffer
  if (SL_BGAPI_MSG_HEADER_LEN + msg_len > DEFAULT_HOST_BUFLEN) {
    app_log_error("Invalid message length: %d, the BGAPI data stream may become corrupted." APP_LOG_NL, msg_len);
    return -1;
  }
  ret = ncp_host_peek_timeout(msg_len, MSG_RECV_TIMEOUT_COUNT * msg_len);
  if (ret < 0) {
    app_log_error("Message reveice timeout occured, the BGAPI data stream has been corrupted!" APP_LOG_NL);
    return -1;
  }
  // Read the rest of the message
  ret = host_comm_rx(msg_len, (void *)&buf_ncp_raw.buf[SL_BGAPI_MSG_HEADER_LEN]);
  if (ret < 0) {
    app_log_error("Message receive failed, expected %d, return value %d!" APP_LOG_NL, msg_len, ret);
    return -1;
  } else if (ret != msg_len) {
    app_log_warning("Message length mismatch, expected %d, received %d!" APP_LOG_NL, msg_len, ret);
  }
  // add the header length to the whole message size in the end
  msg_len += SL_BGAPI_MSG_HEADER_LEN;
  buf_ncp_raw.len = msg_len;
#if defined(SECURITY) && SECURITY == 1
  if (SL_BT_MSG_ENCRYPTED(msg_header) !=  0) {
    security_decrypt((char *)&buf_ncp_raw.buf[0], (char *)&buf_ncp_in.buf[0], (unsigned *)&msg_len);
  } else
#endif // defined(SECURITY) && SECURITY == 1
  {
    memcpy(buf_ncp_in.buf, buf_ncp_raw.buf, msg_len);
  }
  buf_ncp_in.len = msg_len;
#if defined(SECURITY) && SECURITY == 1
  if (enable_security) {
    ncp_sec_host_command_handler(&buf_ncp_in);
  }
#endif // defined(SECURITY) && SECURITY == 1
  return msg_len;
}

/******************************************************************************
 * Receive until boot event arrives
 *
 * buf_ncp_in can be used directly because the boot event is always unencrypted
 * even if the security is enabled.
 *****************************************************************************/
static int32_t ncp_host_get_boot_event(void)
{
  // This event header is equivalent with sl_bt_evt_system_boot
  static const uint32_t boot_event_header = 0x000112a0;
  uint32_t shift_counter = 0;
  int32_t ret, msg_len;

  ret = ncp_host_lazy_peek();
  if (ret < SL_BGAPI_MSG_HEADER_LEN) {
    return -1;
  }
  // Read header
  ret = host_comm_rx(SL_BGAPI_MSG_HEADER_LEN, &buf_ncp_in.buf[0]);
  if (ret < 0) {
    return -1;
  }
  // Read bytes one by one until a valid boot event header is received.
  while (buf_ncp_in.header != boot_event_header) {
    if (accept_dfu_boot && buf_ncp_in.header == 0x000004a0) {
      break;                                    // DFU boot header - keep it as is, exit before further shifting
    }

    if (shift_counter > SL_BGAPI_MAX_PAYLOAD_SIZE) {
      // Abort reception if the target sends data continuously.
      return -1;
    }
    shift_counter++;
    buf_ncp_in.header >>= 8;
    ret = ncp_host_peek_timeout(1, MSG_RECV_TIMEOUT_COUNT);
    if (ret < 0) {
      return -1;
    }
    ret = host_comm_rx(1, (void *)&buf_ncp_in.buf[3]);
    if (ret < 0) {
      return -1;
    }
  }
  // Get boot event payload
  msg_len = SL_BGAPI_MSG_LEN(buf_ncp_in.header);
  ret = ncp_host_peek_timeout(msg_len, MSG_RECV_TIMEOUT_COUNT * msg_len);
  if (ret < 0) {
    return -1;
  }
  ret = host_comm_rx(msg_len, (void *)&buf_ncp_in.buf[SL_BGAPI_MSG_HEADER_LEN]);
  if (ret < 0) {
    return -1;
  }
  buf_ncp_in.len = SL_BGAPI_MSG_HEADER_LEN + msg_len;
  wait_for_boot = false;
  accept_dfu_boot = false;
  (void)app_timer_stop(&boot_timer);

#if defined(SECURITY) && SECURITY == 1
  if (enable_security) {
    ncp_sec_host_command_handler(&buf_ncp_in);
  }
#endif // defined(SECURITY) && SECURITY == 1

  return buf_ncp_in.len;
}

/******************************************************************************
 * Boot event timeout callback
 *****************************************************************************/
static void on_boot_timer_expire(app_timer_t *timer, void *data)
{
  (void)data;
  // This command is equivalent with sl_bt_system_reboot
  uint8_t reboot_command[] = { 0x20, 0x00, 0x01, 0x1f };

  if (boot_retry_count < NCP_REBOOT_RETRY_COUNT) {
    app_log_info("Rebooting NCP target (%d)..." APP_LOG_NL, boot_retry_count);
    boot_retry_count++;
    accept_dfu_boot = ACCEPT_DFU_BOOT;
    (void)host_comm_tx(sizeof(reboot_command), reboot_command);
    sl_status_t sc = app_timer_start(timer,
                                     NCP_REBOOT_TIMEOUT_RETRY_MS,
                                     on_boot_timer_expire,
                                     NULL,
                                     false);
    app_assert_status(sc);
  } else {
    app_assert(false, "NCP target unreachable.");
  }
}

#if defined(SECURITY) && SECURITY == 1
static void ncp_sec_host_command_handler(buf_ncp_host_t *buf)
{
  uint8_t response[DEFAULT_HOST_BUFLEN] = { 0 };
  sl_bt_msg_t *command = NULL;
  sl_bt_msg_t *resp_cmd = NULL;
  int32_t ret;

  command = (sl_bt_msg_t *)buf->buf;

  switch (SL_BT_MSG_ID(command->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
    case sl_bt_evt_dfu_boot_id:
      security_reset();
      // Wait for the security handshake response (80 bytes length)
      ret = ncp_host_peek_timeout(SEC_BGAPI_RSP_MSG_LEN,
                                  MSG_RECV_TIMEOUT_COUNT * SEC_BGAPI_RSP_MSG_LEN);
      if (ret < 0) {
        return;
      }
      // Read the rest of the message
      ret = host_comm_rx(SEC_BGAPI_RSP_MSG_LEN, (void *) &response);
      resp_cmd = (sl_bt_msg_t *)response;
      if (SL_BT_MSG_ID(resp_cmd->header)
          == sl_bt_rsp_user_message_to_target_id) {
        // Check if the correct response was received to security handshake
        // Length has to be 80, and 0x00004900 has to come after 4 byte header
        if (ret == SEC_BGAPI_RSP_MSG_LEN
            && response[SEC_BGAPI_RSP_BASE] == 0x00
            && response[SEC_BGAPI_RSP_BASE + 1] == 0x00
            && response[SEC_BGAPI_RSP_BASE + 2] == 0x49
            && response[SEC_BGAPI_RSP_BASE + 3] == 0x00) {
          uint8_t public[PUBLIC_KEYPAIR_SIZE];
          uint8_t target_iv_to_target[IV_SIZE];
          uint8_t target_iv_to_host[IV_SIZE];
          memcpy(public, &response[SEC_BGAPI_RSP_BASE + SEC_BGAPI_RSP_BASE_LEN],
                 PUBLIC_KEYPAIR_SIZE);
          memcpy(target_iv_to_target, &response[PUBLIC_KEYPAIR_SIZE
                                                + SEC_BGAPI_RSP_BASE + SEC_BGAPI_RSP_BASE_LEN], IV_SIZE);
          memcpy(target_iv_to_host, &response[PUBLIC_KEYPAIR_SIZE
                                              + SEC_BGAPI_RSP_BASE + SEC_BGAPI_RSP_BASE_LEN + IV_SIZE], IV_SIZE);
          security_increase_security_rsp(public, target_iv_to_target,
                                         target_iv_to_host);
          return;
        } else {
          app_log_warning("NCP Encryption Failed 0x%02x%02x"
                          APP_LOG_NL,
                          response[SEC_BGAPI_RSP_BASE + 1],
                          response[SEC_BGAPI_RSP_BASE]);
        }
      }
      break;
    default:
      break;
  }
}

void security_state_change_cb(security_state_t state)
{
  switch (state) {
    case SECURITY_STATE_UNENCRYPTED:
      //start security handshake
      security_start();
      break;
    case SECURITY_STATE_ENCRYPTED:
      app_log_info("Communication encrypted" APP_LOG_NL);
    default:
      break;
  }
}

int sl_bgapi_user_cmd_increase_security(uint8_t *public_key,
                                        uint8_t *host_iv_to_target,
                                        uint8_t *host_iv_to_host)
{
  uint8_t bgapi_packet[SEC_BGAPI_CMD_MSG_LEN] =
  { 0x20, 0x4a, 0xff, 0x00, 0x49, 0x00 };
  memcpy(bgapi_packet + SEC_BGAPI_CMD_BASE, public_key,
         PUBLIC_KEYPAIR_SIZE);
  memcpy(bgapi_packet + SEC_BGAPI_CMD_BASE + PUBLIC_KEYPAIR_SIZE,
         host_iv_to_target, IV_SIZE);
  memcpy(bgapi_packet + SEC_BGAPI_CMD_BASE + PUBLIC_KEYPAIR_SIZE
         + IV_SIZE, host_iv_to_host, IV_SIZE);

  ncp_host_tx(sizeof(bgapi_packet), bgapi_packet);
  return 0;
}
#endif // defined(SECURITY) && SECURITY == 1
