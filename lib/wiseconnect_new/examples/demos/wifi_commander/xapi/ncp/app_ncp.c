/***************************************************************************/ /**
 * @file
 * @brief Bluetooth Network Co-Processor (NCP) Interface
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdint.h>
#include "em_common.h"
#include "em_core.h"
#include "sl_status.h"
#include "app_simple_com.h"
#include "app_ncp.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#include "app_timer.h"
#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
#include "app_wake_lock.h"
#endif // APP_CATALOG_WAKE_LOCK_PRESENT
#if defined(APP_CATALOG_NCP_SEC_PRESENT)
#include "app_ncp_sec.h"
#endif // APP_CATALOG_NCP_SEC_PRESENT
#if defined(APP_CATALOG_COMMANDER_BOOTLOADER_INTERFACE_PRESENT)
#include "btl_interface.h"
#endif // APP_CATALOG_COMMANDER_BOOTLOADER_INTERFACE_PRESENT

// Command buffer
typedef struct {
  uint16_t len;
  uint8_t buf[APP_NCP_CMD_BUF_SIZE];
  bool available;
} cmd_t;

// Event buffer
typedef struct {
  uint16_t len;
  uint8_t buf[APP_NCP_EVT_BUF_SIZE];
  bool available;
} evt_t;

// Timer states
typedef enum { CMD_IDLE = 0, CMD_WAITING, CMD_RECEIVED } timer_state_t;

static cmd_t cmd = { 0 };
static evt_t evt = { 0 };
static bool busy = false;
#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
// semaphore to disable sleep prematurely
static uint8_t sleep_semaphore = 0;
// upper bits are flags to store signal value and wait until first cmd is received
#define SLEEP_SIGNAL_PRESENT   0x80
#define SLEEP_WAITING_FIRST_RX 0x40
#define SLEEP_SIGNAL_MASK      SLEEP_WAITING_FIRST_RX | SLEEP_SIGNAL_PRESENT
#endif // APP_CATALOG_WAKE_LOCK_PRESENT

#define MSG_GET_LEN(x) ((uint16_t)(APP_XAPI_MSG_LEN((x)->header) + APP_XAPI_MSG_HEADER_LEN))

// Command and event buffer helper functions
static void cmd_enqueue(uint16_t len, uint8_t *data);
static void cmd_dequeue(void);
static void evt_enqueue(uint16_t len, uint8_t *data);
static void evt_dequeue(void);

// Command and event helper functions
static inline bool cmd_is_available(void);
static inline void cmd_set_available(void);
static inline void cmd_clr_available(void);
static inline bool evt_is_available(void);
static inline void evt_set_available(void);
static inline void evt_clr_available(void);

// Timer handle and callback for command timeout.
static app_timer_t cmd_timer;
static void cmd_timer_cb(app_timer_t *timer, void *data);

#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
static inline bool sleep_signal_mask_is_set(void);
static inline void sleep_signal_mask_set(void);
static inline void sleep_signal_flag_clr(void);
static inline void sleep_signal_wait_clr(void);
#endif // APP_CATALOG_WAKE_LOCK_PRESENT

// -----------------------------------------------------------------------------
// Public functions (API implementation)

/**************************************************************************/ /**
 * Ncp Initialization function.
 *****************************************************************************/
void app_ncp_init(void)
{
  // Clear all buffers
  cmd_dequeue();
  evt_dequeue();

  // Start reception
  app_simple_com_receive();
  busy = false;
#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
  sleep_semaphore = 0;
#endif // APP_CATALOG_WAKE_LOCK_PRESENT

  app_ncp_os_task_init();
}

/**************************************************************************/ /**
 * Ncp process action function.
 *
 * @note Called through app_system_process_action
 *****************************************************************************/
void app_ncp_step(void)
{
  // -------------------------------
  // Command available and NCP not busy
  if (cmd_is_available() && !busy) {
    app_wifi_msg_t *command = (app_wifi_msg_t *)cmd.buf;
    app_wifi_msg_t *response;

#if defined(APP_CATALOG_NCP_SEC_PRESENT)
    uint32_t result = 0;
    // store if cmd was encrypted during reception for further processing
    bool cmd_is_encrypted = app_ncp_sec_is_encrypted(&cmd.buf);
    result                = app_ncp_sec_command_handler((uint8_t *)&cmd.buf);
    if ((result & APP_NCP_SEC_CMD_PROCESS) == APP_NCP_SEC_CMD_PROCESS)
#endif // APP_CATALOG_NCP_SEC_PRESENT
    {
      cmd_clr_available();
      // Call Wifi API binary command handler
      app_wifi_handle_command(command->header, command->data.payload);
    }
#if defined(APP_CATALOG_NCP_SEC_PRESENT)
    if ((result & APP_NCP_SEC_EVT_PROCESS) == APP_NCP_SEC_EVT_PROCESS) {
      // Clear command buffer
      cmd_dequeue();
    } else if ((result & APP_NCP_SEC_RSP_PROCESS) == APP_NCP_SEC_RSP_PROCESS) {
      response = app_ncp_sec_process_response(app_wifi_get_command_response(), cmd_is_encrypted);
#else
    {
      response = app_wifi_get_command_response();
#endif // APP_CATALOG_NCP_SEC_PRESENT
      // Clear command buffer
      cmd_dequeue();
      busy = true;
#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
      // Wake up other controller
      app_wake_lock_set_remote_req();
#endif // APP_CATALOG_WAKE_LOCK_PRESENT
      // Transmit command response
      app_simple_com_transmit((uint32_t)(MSG_GET_LEN(response)), (uint8_t *)response);
    }
  }

  // -------------------------------
  // Event available and NCP not busy
  if (evt_is_available() && !busy) {
    evt_clr_available();
    busy = true;
#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
    // Wake up other controller
    app_wake_lock_set_remote_req();
#endif // APP_CATALOG_WAKE_LOCK_PRESENT

    uint8_t *data_ptr = evt.buf;
    uint16_t len      = evt.len;

#if defined(APP_CATALOG_NCP_SEC_PRESENT)
    // encrypt the outgoing event
    data_ptr = (uint8_t *)app_ncp_sec_process_event((app_wifi_msg_t *)evt.buf);
    // refresh original len as encrypted msg will be longer than original
    len = MSG_GET_LEN((app_wifi_msg_t *)data_ptr);
#endif // APP_CATALOG_NCP_SEC_PRESENT

    // Transmit events
    app_simple_com_transmit((uint32_t)len, data_ptr);
    // Clear event buffer
    evt_dequeue();
  }
}

/**************************************************************************/ /**
 * Local event processor.
 *
 * Use this function to process Bluetooth stack events locally on NCP.
 * Set the return value to true, if the event should be forwarded to the
 * NCP-host. Otherwise, the event is discarded locally.
 * Implement your own function to override this default weak implementation.
 *
 * @param[in] evt The event.
 *
 * @return true, if we shall send the event to the host. This is the default.
 *
 * @note Weak implementation.
 *****************************************************************************/
SL_WEAK bool app_ncp_local_evt_process(app_wifi_msg_t *evt)
{
  (void)evt;
  return true;
}

#if defined(APP_CATALOG_BTMESH_PRESENT)

/**************************************************************************/ /**
 * Local event processor.
 *
 * Use this function to process Bluetooth Mesh stack events locally on NCP.
 * Set the return value to true, if the event should be forwarded to the
 * NCP-host. Otherwise, the event is discarded locally.
 * Implement your own function to override this default weak implementation.
 *
 * @param[in] evt The event.
 *
 * @return true, if we shall send the event to the host. This is the default.
 *
 * @note Weak implementation.
 *****************************************************************************/
SL_WEAK bool app_ncp_local_btmesh_evt_process(app_btmesh_msg_t *evt)
{
  (void)evt;
  return true;
}

#endif

/**************************************************************************/ /**
 * User command (message_to_target) handler callback.
 *
 * @note Weak implementation.
 *****************************************************************************/
SL_WEAK void app_ncp_user_cmd_message_to_target_cb(void *data)
{
  (void)data;
}

// -----------------------------------------------------------------------------
// Functions used for communication between stack and Uart

/**************************************************************************/ /**
 * Bluetooth stack event handler.
 *
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void app_wifi_on_event(app_wifi_msg_t *evt)
{
  if (app_ncp_local_evt_process(evt)) {
    // Enqueue event
    evt_enqueue(MSG_GET_LEN(evt), (uint8_t *)evt);
    app_ncp_os_task_proceed();
  }
}

/**************************************************************************/ /**
 * Signal if ncp application can process a new received event.
 *
 * Called before putting event into event buffer.
 *
 * @param[in] len Message length
 *
 * @return True if message fits into buffer, otherwise false.
 *****************************************************************************/
static inline bool app_ncp_can_process_event(uint32_t len)
{
  bool ret = false;
  // event fits into event buffer; otherwise don't pop it from queue
  if ((len <= (uint32_t)(sizeof(evt.buf) - evt.len)) && !evt_is_available() && !cmd_is_available()) {
    ret = true;
  }
  return ret;
}

/**************************************************************************/ /**
 * Signal if bluetooth stack can process a new received event.
 *
 * @param[in] len Message length
 *
 * @return True if message fits into buffer, otherwise false.
 *****************************************************************************/
bool app_wifi_can_process_event(uint32_t len)
{
  return app_ncp_can_process_event(len);
}

#if defined(APP_CATALOG_BTMESH_PRESENT)
/**************************************************************************/ /**
 * Signal if bluetooth mesh stack can process a new received event.
 *
 * @param[in] len Message length
 *
 * @return True if message fits into buffer, otherwise false.
 *****************************************************************************/
bool app_btmesh_can_process_event(uint32_t len)
{
  return app_ncp_can_process_event(len);
}
#endif

/**************************************************************************/ /**
 * Uart transmit completed callback
 *
 * Called after Uart transmit is finished.
 *
 * @param[in] status Status of the transmission
 *****************************************************************************/
void app_simple_com_transmit_cb(sl_status_t status)
{
  (void)status;

#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
  // Signal other controller that it can go to sleep
  app_wake_lock_clear_remote_req();

  // Command response completed, go to sleep if possible
  if (sleep_signal_mask_is_set()) {
    // Only decrease semaphore if a command was received before
    // i.e. not an event was sent out
    sleep_semaphore--;
    if (!sleep_semaphore) {
      // Go to sleep if signal is down and command answered
      app_wake_lock_set_local();
    }
  }
#endif // APP_CATALOG_WAKE_LOCK_PRESENT
  busy = false;
  app_ncp_os_task_proceed();
}

/**************************************************************************/ /**
 * Uart receive completed callback
 *
 * Called after Uart receive is finished.
 *
 * @param[in] status Status of the reception
 * @param[in] len Received message length
 * @param[in] data Data received
 *****************************************************************************/
void app_simple_com_receive_cb(sl_status_t status, uint32_t len, uint8_t *data)
{
  (void)status;
  // Stop on communication error
  // Copy command into buffer
  cmd_enqueue((uint16_t)len, data);
  app_ncp_os_task_proceed();
}

// -----------------------------------------------------------------------------
// Power manager related functions

/**************************************************************************/ /**
 * Check if NCP allows go to sleep
 *****************************************************************************/
bool app_internal_ncp_is_ok_to_sleep(void)
{
  bool result;

  result = !cmd_is_available() && !evt_is_available() && !busy;
#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
  result = result && !sleep_semaphore;
#endif
  return result;
}

// -----------------------------------------------------------------------------
// Command and event buffer handling functions

/**************************************************************************/ /**
 * Put command to command buffer
 *
 * @param[in] len Command length
 * @param[in] data Command data
 *****************************************************************************/
static void cmd_enqueue(uint16_t len, uint8_t *data)
{
  timer_state_t state = CMD_IDLE;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // Command fits into command buffer; otherwise discard it.
  if (len <= (sizeof(cmd.buf) - cmd.len)) {
    memcpy((void *)&cmd.buf[cmd.len], (void *)data, (size_t)len);
    cmd.len += len;
    // Part of the command received, start timer to not get stuck.
    state = CMD_WAITING;
    // Command header has been received.
    if (cmd.len >= APP_WIFI_MSG_HEADER_LEN) {
      // 4-byte header + len bytes payload
      uint32_t len = APP_WIFI_MSG_LEN(*(uint32_t *)cmd.buf) + APP_WIFI_MSG_HEADER_LEN;
      if (cmd.len >= len) {
        // Command has been received entirely.
        cmd_set_available();
        state = CMD_RECEIVED;
#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
        // Command received fully, increase semaphore.
        sleep_semaphore++;
        // Also signal that a command was received, system can go to sleep after
        sleep_signal_wait_clr();
#endif // APP_CATALOG_WAKE_LOCK_PRESENT
      }
    }
  }
  CORE_EXIT_CRITICAL();

  if (state == CMD_RECEIVED) {
    // Stop timer as the whole command was received. No problem if timer was
    // not started before, app_timer is prepared for that.
    // To Do: Error check
    app_timer_stop(&cmd_timer);
  } else if (state == CMD_WAITING) {
    // Start timer used for max waiting time of fragmented packets.
    // To Do: Error check
    app_timer_start(&cmd_timer, APP_NCP_CMD_TIMEOUT_MS, cmd_timer_cb, NULL, false);
  }
}

/**************************************************************************/ /**
 * Clear command buffer
 *****************************************************************************/
static void cmd_dequeue(void)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  cmd.len = 0;
  cmd_clr_available();
  CORE_EXIT_CRITICAL();
}

/**************************************************************************/ /**
 * Put event to event buffer
 *
 * @param[in] len Event length
 * @param[in] data Event data
 *****************************************************************************/
static void evt_enqueue(uint16_t len, uint8_t *data)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  // event fits into event buffer; otherwise discard it
  if (len <= (sizeof(evt.buf) - evt.len)) {
    memcpy((void *)&evt.buf[evt.len], (void *)data, len);
    evt.len += len;
    evt_set_available();
  }
  CORE_EXIT_CRITICAL();
}

/**************************************************************************/ /**
 * Clear event buffer
 *****************************************************************************/
static void evt_dequeue(void)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  evt.len = 0;
  evt_clr_available();
  CORE_EXIT_CRITICAL();
}

// -----------------------------------------------------------------------------
// Timer callback function

static void cmd_timer_cb(app_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;

  // Signal the other end that the command was received only partially
  // then clean buffer. The timer is already stopped.
  app_wifi_send_system_error(SL_STATUS_COMMAND_INCOMPLETE, 0, NULL);
  cmd_dequeue();
}

// -----------------------------------------------------------------------------
// Helper functions

/**************************************************************************/ /**
 * Get command availability in command buffer
 *****************************************************************************/
static inline bool cmd_is_available(void)
{
  return cmd.available;
}

/**************************************************************************/ /**
 * Set command available in command buffer
 *****************************************************************************/
static inline void cmd_set_available(void)
{
  cmd.available = true;
}

/**************************************************************************/ /**
 * Clear command available in command buffer
 *****************************************************************************/
static inline void cmd_clr_available(void)
{
  cmd.available = false;
}

/**************************************************************************/ /**
 * Get event availability in command buffer
 *****************************************************************************/
static inline bool evt_is_available(void)
{
  return evt.available;
}

/**************************************************************************/ /**
 * Set event available in command buffer
 *****************************************************************************/
static inline void evt_set_available(void)
{
  evt.available = true;
}

/**************************************************************************/ /**
 * Clear event available in command buffer
 *****************************************************************************/
static inline void evt_clr_available(void)
{
  evt.available = false;
}

/**************************************************************************/ /**
 * OS initialization function
 *
 * @note Weak implementation.
 *****************************************************************************/
SL_WEAK void app_ncp_os_task_init(void)
{
  // No action on bare metal implementation
}

/**************************************************************************/ /**
 * Function to trigger the OS task to proceed
 *
 * @note Weak implementation.
 *****************************************************************************/
SL_WEAK void app_ncp_os_task_proceed(void)
{
  // No action on bare metal implementation
}

#if defined(APP_CATALOG_WAKE_LOCK_PRESENT)
/**************************************************************************/ /**
 * Get sleep signal flag and wait for rx cmd flag state
 *****************************************************************************/
static inline bool sleep_signal_mask_is_set(void)
{
  return (sleep_semaphore & ~(SLEEP_SIGNAL_MASK));
}

/**************************************************************************/ /**
 * Set sleep signal flag and wait for rx cmd flag state
 *****************************************************************************/
static inline void sleep_signal_mask_set(void)
{
  sleep_semaphore |= (SLEEP_SIGNAL_PRESENT | SLEEP_WAITING_FIRST_RX);
}

/**************************************************************************/ /**
 * Clear sleep signal flag state
 *****************************************************************************/
static inline void sleep_signal_flag_clr(void)
{
  sleep_semaphore &= ~(SLEEP_SIGNAL_PRESENT);
}

/**************************************************************************/ /**
 * Clear wait for rx cmd flag state
 *****************************************************************************/
static inline void sleep_signal_wait_clr(void)
{
  sleep_semaphore &= ~(SLEEP_WAITING_FIRST_RX);
}
#endif // APP_CATALOG_WAKE_LOCK_PRESENT
