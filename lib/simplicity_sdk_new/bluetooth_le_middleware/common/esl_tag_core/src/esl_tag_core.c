/***************************************************************************//**
 * @file
 * @brief ESL Tag core component logic.
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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sl_core.h"
#include "em_emu.h"
#include "gatt_db.h"
#include "app_scheduler.h"
#include "sl_sleeptimer.h"
#include "esl_tag_log.h"
#include "esl_tag_tlv.h"
#include "esl_tag_core.h"
#include "esl_tag_crypto.h"
#include "esl_tag_opcodes.h"
#include "esl_tag_internal.h"
#include "esl_tag_response.h"
#include "esl_tag_led_core.h"
#include "esl_tag_image_core.h"
#include "esl_tag_sensor_core.h"
#include "esl_tag_display_core.h"
#include "esl_tag_battery_internal.h"

// Encryption requires bonding. This setting will also enable bonding.
#define ESL_SECURITY_BASE_FLAGS   (0x02)

// Allow connections only from bonded devices
#define ESL_SECURITY_BONDED_ONLY  (0x10)

// Maximum allowed bonding count
#define ESL_MAX_BONDINGS          (1)

// Maximum number of PAwR packets that can be skipped after a successful receive
#define ESL_SYNC_SKIP             (0)

// Minimum permitted time between successful receives.
#define ESL_SYNC_MIN_TIMEOUT      (0x0a)    // see sl_bt_sync_set_parameters API

// Maximum permitted time between successful receives.
#define ESL_SYNC_MAX_TIMEOUT      (0x4000)  // see sl_bt_sync_set_parameters API

// Bonding policy: if database is full, new bonding attempts will fail
#define ESL_BONDING_POLICY_FLAGS  (0)

// Invalid ESL Access Point BT address
#define ESL_INVALID_AP_ADDRESS    ("\0\0\0\0\0\0")

// Timeout in milliseconds, ESLP d095r11, Section 8.3 Security Requirements
#define ESL_CORE_SECURITY_TIMEOUT_MS      (60u * 60u * 1000u) // min * sec * ms

// Security timeout callback priority
#define ESL_CORE_SECURITY_TIMER_PRIORITY  1

// some flags for internal use, only:
#define ESL_CONFIG_FLAG_ESL_ADDRESS       (0x01u)
#define ESL_CONFIG_FLAG_ESL_AP_KEY        (0x02u)
#define ESL_CONFIG_FLAG_ESL_RESPONSE_KEY  (0x04u)
#define ESL_CONFIG_FLAG_ESL_ABS_TIME      (0x08u)
#define ESL_CONFIG_FLAG_UPDATE_COMPLETE   (0x10u)
#define ESL_CONFIG_FLAG_RANDOMIZER_READY  (0x20u)
#define ESL_CONFIG_FLAGS_MANDATORY_MASK   (ESL_CONFIG_FLAG_ESL_ADDRESS        \
                                           | ESL_CONFIG_FLAG_ESL_AP_KEY       \
                                           | ESL_CONFIG_FLAG_ESL_RESPONSE_KEY \
                                           | ESL_CONFIG_FLAG_ESL_ABS_TIME)

// Periodic Advertisement with Responses interval resolution from LE Core spec.
#define ESL_PAWR_INTERVAL_RESOLUTION_MS   1.25f

// Multiplier for clock drift compensation with integer math only
#define ESL_DRIFT_INTEGER_MULTIPLIER      100u

// Actual helper constant for clock drift compensation
#define ESL_PAWR_INTEGER_INTERVAL  (uint32_t)(ESL_DRIFT_INTEGER_MULTIPLIER \
                                              * ESL_PAWR_INTERVAL_RESOLUTION_MS)

// Divisor constant for unit to milli unit conversion
#define ESL_DRIFT_THRESHOLD_DIVISOR    1000u

typedef union {
  esl_address_t value;
  PACKSTRUCT(struct {
    uint8_t esl_id;
    uint8_t group_id;
  });
} esl_address_union_t;

typedef union {
  // Note: this will only work as expected on little endian machines!
  sl_bt_ead_randomizer_t randomizer;
  // Use the similar naming to the Core spec. v5.4, Vol 6, Part E, Section 2 (CCM)
  // but in snake_case instead of CamelCase
  uint64_t               packet_counter;
} response_randomizer_t;

// make an alias for the same structure used by the AP sync packets
typedef response_randomizer_t sync_randomizer_t;

typedef struct {
  // Timestamp of last PAwR sync received event
  uint64_t pawr_timestamp;
  // AP sync key material last known randomizer - NULL until no sync packet received
  sync_randomizer_t *last_ap_randomizer;
  // ESL Control Point Notifications status
  bool     notifications_enabled;
  // Helper variable to accumulate time drifts
  int32_t  tick_error;
  // ESL Absolute Time cumulative and continuously adjusted offset
  uint32_t time_offset;
  // PAwR interval value
  uint32_t pawr_interval_ticks;
  // Sync timeout parameter for the PAwR sync.
  uint32_t sync_timeout;
  // ESL Tag Internal status variable
  esl_state_t status;
  // ESL Tag Basic state variable
  esl_basic_state_t basic_state;
  // The Sync handle allocated from Bluetooth stack.
  uint16_t sync_handle;
  // PAwR payload size limit
  uint16_t response_payload_limit;
  // ESL Tag Internal configuration status variable
  uint8_t config_status;
  // The connection handle allocated from Bluetooth stack.
  uint8_t connection_handle;
  // The bonding handle allocated from Bluetooth stack.
  uint8_t bonding_handle;
  // ESL Address local storage
  esl_address_union_t address;
  // ESL Access Point BT address
  bd_addr ap_address;
} esl_struct_t;

typedef struct {
  // ESL AP Sync key local storage
  struct sl_bt_ead_key_material_s ap_key;
  // ESL Response key local storage
  struct sl_bt_ead_key_material_s response_key;
} esl_keys_struct_t;

typedef struct {
  // Security timeout task handle
  sl_sleeptimer_timer_handle_t watchdog_handle;
#if ESL_TAG_SYNC_SCAN_ENABLE
  // Scan timeout handle
  app_scheduler_task_handle_t scan_timer_task;
  // Consecutive sync-by-scan counter
  uint32_t scan_count;
  // Scan permission flag (to avoid endless scan loops)
  bool scan_permitted;
#endif // ESL_TAG_SYNC_SCAN_ENABLE
  // Security timeout trigger flag
  bool watchdog_triggered;
  // PAwR evt_pawr_sync_subevent_report.event_counter backup
  uint16_t request_event;
  // PAwR evt_pawr_sync_subevent_report.subevent field backup
  uint8_t request_subevent;
  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertising_set_handle[ESL_TAG_ADVERTISERS_COUNT];
} esl_persistent_struct_t;

// EAD Randomizer for ESL Responses encryption
static response_randomizer_t esl_core_response_randomizer = { 0 };

#if ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED
// AP sync key material randomizer when we first received a valid PAwR message
static sync_randomizer_t esl_core_ap_randomizer_base = { 0 };
#endif // ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED

// Custom data for advertising, in accordance with ESLS v1.0r01, section 2.7.3.1
static const uint8_t esl_core_advertising_data[] = {
  0x02, 0x01, 0x06,        // Flags
  0x03, 0x03, 0x57, 0x18   // Complete list of 16 bits UUID - ESL Service (only)
};

static const char *esl_states_string[] = {
  "Boot",
  "Unassociated",
  "Configuring",
  "Synchronized",
  "Unsynchronized",
  "Updating"
};

static esl_struct_t esl_tag = {
  .pawr_timestamp         = 0,
#if ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED
  .last_ap_randomizer    = NULL,
#endif // ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED
  .notifications_enabled  = false,
  .tick_error             = 0,
  .time_offset            = 0,
  .pawr_interval_ticks    = 0,
  .sync_timeout           = ESL_SYNC_MIN_TIMEOUT,
  .status                 = esl_state_boot,
  .basic_state            = 0,
  .sync_handle            = SL_BT_INVALID_SYNC_HANDLE,
  .response_payload_limit = ESL_PAYLOAD_MAX_LENGTH,
  .config_status          = 0,
  .connection_handle      = SL_BT_INVALID_CONNECTION_HANDLE,
  .bonding_handle         = SL_BT_INVALID_BONDING_HANDLE,
  .address.value          = 0,
  .ap_address             = { { 0 } }
};

static esl_keys_struct_t esl_keys = {
  .ap_key                 = { { { 0 } } },
  .response_key           = { { { 0 } } }
};

static esl_persistent_struct_t esl_tag_persistent = {
  .watchdog_handle        = { 0 },
#if ESL_TAG_SYNC_SCAN_ENABLE
  .scan_timer_task        = NULL,
  .scan_count             = 0,
  .scan_permitted         = true,
#endif // ESL_TAG_SYNC_SCAN_ENABLE
  .watchdog_triggered     = false,
  .request_event          = 0,
  .request_subevent       = 0,
#if ESL_TAG_INTERMITTENT_ADVERTISING
  .advertising_set_handle = { SL_BT_INVALID_ADVERTISING_SET_HANDLE,
                              SL_BT_INVALID_ADVERTISING_SET_HANDLE }
#else
  .advertising_set_handle = { SL_BT_INVALID_ADVERTISING_SET_HANDLE }
#endif // ESL_TAG_INTERMITTENT_ADVERTISING
};

// ESL Tag asynchronous randomizer callback
static void esl_core_async_randomizer(void *p_event_data, uint16_t event_size)
{
  (void)event_size;

  // Remove ready flag so a new randomizer value will be generated by CSPRNG
  esl_tag.config_status &= (uint8_t) (~ESL_CONFIG_FLAG_RANDOMIZER_READY);
  (void)esl_core_get_randomizer(p_event_data);
}

#if ESL_TAG_INTERMITTENT_ADVERTISING
// ESL Tag secondary advertiser start callback
static void esl_core_secondary_advertiser(void *p_event_data, uint16_t event_size)
{
  (void)event_size;
  (void)p_event_data;
  // start only if there's no active connection
  if (esl_tag.connection_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
    // start the secondary advertiser also as sl_bt_legacy_advertiser_connectable
    // but this advertising doesn't expose any service nor other UUID
    (void)sl_bt_legacy_advertiser_start(esl_tag_persistent.advertising_set_handle[1],
                                        sl_bt_legacy_advertiser_connectable);
  }
}
#endif // ESL_TAG_INTERMITTENT_ADVERTISING

#if ESL_TAG_SYNC_SCAN_ENABLE
static void scan_timeout_task(void *p_event_data, uint16_t event_size)
{
  (void)event_size;
  (void)p_event_data;

  (void)sl_bt_scanner_stop();
  sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                ESL_LOG_LEVEL_INFO,
                "Scan stopped after %u seconds timeout!",
                ESL_TAG_SCAN_TIMEOUT_SEC);
  (void)esl_core_start_advertising();
  // We can reset the sync-by-scan parameters now that we started advertising.
  esl_tag_persistent.scan_permitted = true;
  esl_tag_persistent.scan_count = 0;
}

// ESL Tag periodic advertisement scanner
static sl_status_t start_scanning_for_periodic_adv(void)
{
  sl_status_t sc;
  uint32_t pawr_interval = sl_sleeptimer_tick_to_ms(esl_tag.pawr_interval_ticks);
  uint16_t scan_interval = pawr_interval / (ESL_DRIFT_INTEGER_MULTIPLIER * ESL_TAG_SCAN_SWEEEP_COEFF);
  uint16_t scan_window   = scan_interval > ESL_TAG_SCAN_WINDOW_MS ? ESL_TAG_SCAN_WINDOW_MS : scan_interval;

  // periodic scanner setting - configurator asks for [ms], but actual
  // unit of time values are 0.625ms, hence the numeric conversion below:
  sc = sl_bt_scanner_set_parameters(sl_bt_scanner_scan_mode_passive,
                                    16 * scan_interval / 10,
                                    16 * scan_window / 10);

  if (sc == SL_STATUS_OK) {
    sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                  ESL_LOG_LEVEL_INFO,
                  "Start scanning for sync with interval %u ms, window %u ms for %u s",
                  scan_interval,
                  scan_window,
                  ESL_TAG_SCAN_TIMEOUT_SEC);
    sc = sl_bt_scanner_start(sl_bt_gap_phy_1m, sl_bt_scanner_discover_observation);
  }

  if (sc == SL_STATUS_OK) {
    sc = app_scheduler_add_delayed(&scan_timeout_task,
                                   ESL_TAG_SCAN_TIMEOUT_SEC * 1000u,
                                   NULL, 0,
                                   &esl_tag_persistent.scan_timer_task);
  }

  if (sc != SL_STATUS_OK) {
    (void)sl_bt_scanner_stop();
  }

  return sc;
}

static void esl_core_signal_valid_message(void)
{
  // Allow sync-by-scan on next sync loss, as it is now proven to be successful
  esl_tag_persistent.scan_permitted = true;
}

static void esl_core_unsynchronize(void)
{
  if (esl_tag_persistent.scan_count >= ESL_TAG_SCAN_COUNT_LIMIT) {
    esl_tag_persistent.scan_permitted = false;
    sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                  ESL_LOG_LEVEL_WARNING,
                  "Scanning limit reached after %u successive sync-by-scan, return to standard ESL advertising!",
                  ESL_TAG_SCAN_COUNT_LIMIT);
  }

  if (esl_tag_persistent.scan_permitted
      && start_scanning_for_periodic_adv() == SL_STATUS_OK) {
    esl_tag_persistent.scan_permitted = false;
    esl_tag_persistent.scan_count++;
  } else {
    sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                  ESL_LOG_LEVEL_INFO,
                  "Scanning is %s, return to standard ESL advertising.",
                  esl_tag_persistent.scan_permitted ? "not possible" : "temporarily not allowed");
    (void)esl_core_start_advertising();
    esl_tag_persistent.scan_permitted = true;
    esl_tag_persistent.scan_count = 0;
  }
}
#else
// Substitute for a missing function
#define esl_core_signal_valid_message() (void)0

// Substitute for Synchronized->Unsynchronized state transition
#define esl_core_unsynchronize() (void)esl_core_start_advertising()
#endif // ESL_TAG_SYNC_SCAN_ENABLE

// ESL Tag PAwR sync handle getter
uint16_t esl_core_get_sync_handle(void)
{
  return esl_tag.sync_handle;
}

// ESL Tag PAwR response payload size limit getter
uint16_t esl_core_get_response_payload_limit(void)
{
  return esl_tag.response_payload_limit;
}

// ESL Tag PAwR request event getter
uint16_t esl_core_get_request_event(void)
{
  return esl_tag_persistent.request_event;
}

// ESL Tag PAwR response subevent getter
uint8_t esl_core_get_request_subevent(void)
{
  return esl_tag_persistent.request_subevent;
}

// ESL Tag asynchronous disconnect
void esl_core_async_disconnect(void *p_event_data, uint16_t event_size)
{
  (void)event_size;
  (void)p_event_data;

  if (esl_tag.connection_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
    // there's nothing that we could do with return value below due the async call
    (void)sl_bt_connection_close(esl_tag.connection_handle);
  }
}

// ESL Tag unassociate (resets configuration, connection and bonding)
void esl_core_unassociate(void)
{
  sl_status_t sc;

  (void)sc; // suppress the compiler warning if sl_bt_esl_assert disabled
  esl_state_t last_status = esl_tag.status;

  esl_core_purge_delayed_commands();
  esl_core_clear_last_error();
  esl_core_purge_responses(0);  // default init for max. response payload length
  (void)sl_bt_ead_delete_key(&esl_keys.ap_key);
  (void)sl_bt_ead_delete_key(&esl_keys.response_key);

  memset(&esl_tag, 0, sizeof(esl_tag));
  // set some fundamental values properly after cleanup
  esl_tag.sync_timeout          = ESL_SYNC_MIN_TIMEOUT;
  esl_tag.status                = last_status;
  esl_tag.sync_handle           = SL_BT_INVALID_SYNC_HANDLE;
  esl_tag.connection_handle     = SL_BT_INVALID_CONNECTION_HANDLE;
  esl_tag.bonding_handle        = SL_BT_INVALID_BONDING_HANDLE;

  sl_bt_sm_delete_bondings();

  sc = sl_bt_sm_configure(ESL_SECURITY_BASE_FLAGS,
                          sl_bt_sm_io_capability_noinputnooutput);
  sl_bt_esl_assert(sc == SL_STATUS_OK);

  // call user callback before start advertising
  esl_core_unassociate_callback();

  (void)esl_core_start_advertising();
}

/*******************************************************************************
 * ESL Security timeout handler
 *
 * @param[in] data Unused by this function but mandatory for sleeptimer tasks
 * @param[in] size Unused by this function but mandatory for sleeptimer tasks
 ******************************************************************************/
void esl_security_timeout(sl_sleeptimer_timer_handle_t *timer, void *data)
{
  // suppress compiler warnings
  (void)timer;
  (void)data;
  esl_tag_persistent.watchdog_triggered = true;
}

void esl_core_step(void)
{
  if (esl_tag_persistent.watchdog_triggered) {
    if (esl_state_unsynchronized == esl_tag.status) {
      // ESLS d095r13 2.7.3.5 Unsynchronized state watchdog requirement
      esl_core_unassociate();
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "Unassociated by watchdog timeout.");
      if (!ESL_TAG_POWER_DOWN_ENABLE) {
        (void)sl_sleeptimer_stop_timer(&esl_tag_persistent.watchdog_handle);
      }
    } else if (ESL_TAG_POWER_DOWN_ENABLE && esl_state_unassociated == esl_tag.status) {
      // if power saving is enabled, shut down after the specified idle time
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "Shutdown by watchdog timeout.");
      (void)sl_sleeptimer_stop_timer(&esl_tag_persistent.watchdog_handle);
      EMU_EM4Init_TypeDef em4init;

      esl_core_shutdown_hook();

      em4init.em4State = emuEM4Shutoff;
      em4init.retainLfxo = false;
      em4init.retainLfrco = false;
      em4init.retainUlfrco = false;
      em4init.pinRetentionMode = emuPinRetentionDisable;

      EMU_EM4Init(&em4init);
      CORE_CRITICAL_SECTION(
#if defined(_SILICON_LABS_32B_SERIES_2)
        GPIO_IntClear(GPIO_IntGet());
#endif // defined(_SILICON_LABS_32B_SERIES_2)
        EMU_EnterEM4S();
        );
      NVIC_SystemReset(); // just in case, we should never get here, anyway.
    } else if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
      // implement ESLS d095r13  2.7.3.3 Synchronized state watchdog requirement
      sl_bt_sync_close(esl_tag.sync_handle);
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "Unsynchronized by watchdog timeout.");
    }
    esl_tag_persistent.watchdog_triggered = false;
  }
}

static inline bool is_esl_configured_for(uint8_t mask)
{
  bool result = (mask
                 == (esl_tag.config_status & mask));
  return result;
}

static void esl_sync_cleanup(uint16_t reason)
{
  esl_tag.sync_handle = SL_BT_INVALID_SYNC_HANDLE;
  esl_tag.response_payload_limit = ESL_PAYLOAD_MAX_LENGTH;
  esl_core_set_basic_state_bit(ESL_BASIC_STATE_SYNCHRONIZED_BIT, ESL_CLEAR);

  if (reason == SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST) {
    sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                  ESL_LOG_LEVEL_INFO,
                  "Sync closed on request.");
  } else {
    sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                  ESL_LOG_LEVEL_WARNING,
                  "Sync lost due 0x%04x",
                  reason);
  }

  if (esl_tag.status == esl_state_synchronized) {
    esl_core_unsynchronize();
  }
}

static uint16_t esl_get_response_payload_limit(uint8_t adv_phy, uint8_t slot_spacing)
{
  // Payload overhead, including up to 13 bytes for LL AUX_SYNC_SUBEVENT_RSP,
  // 9 bytes for encryption and two times 2 bytes for AD Data Type framing
  const uint32_t payload_overhead_bytes = 26u;
  // slot spacing includes the 150 us inter-frame space
  const uint8_t t_ifs_us = 150u;
  const char * const phy_str[4] = { "1m", "2m", "coded", "any" };
  const char *phy_str_p = phy_str[3];

  uint16_t slot_time_us = (slot_spacing * 125u) - t_ifs_us;
  uint16_t size_limit = ESL_PAYLOAD_MIN_LENGTH;
  uint8_t byte_time_us;

  // Immediate return on invalid request with minimum size limit
  if (slot_spacing >= 2) {
    switch (adv_phy) {
      case sl_bt_gap_phy_1m:
        phy_str_p = phy_str[0];
        byte_time_us = 8u;
        break;
      case sl_bt_gap_phy_2m:
        phy_str_p = phy_str[1];
        byte_time_us = 4u;
        break;
      case sl_bt_gap_coded_phy:
        phy_str_p = phy_str[2];
      /* FALLTHRU */
      // @suppress("No break at the end of case")
      default: // unknown / unsupported PHY will fall back to S=8 symbol rate
        byte_time_us = 64u;
        break;
    }

    size_limit = (slot_time_us / byte_time_us);

    if (size_limit < ESL_PAYLOAD_MIN_LENGTH + payload_overhead_bytes) {
      size_limit = ESL_PAYLOAD_MIN_LENGTH;
    } else {
      size_limit -= payload_overhead_bytes;
    }
  }
  sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                ESL_LOG_LEVEL_INFO,
                "Calculated ESL response payload limit for %u slot spacing on %s PHY is %u bytes.",
                slot_spacing,
                phy_str_p,
                size_limit);
  (void)phy_str_p;  // suppress warnings if sl_bt_esl_log is not enabled
  (void)phy_str;
  return size_limit;
}

static void esl_state_boot_handler(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  (void)sc; // suppress the compiler warning if sl_bt_esl_assert is disabled
  (void)esl_states_string; // suppress warning if sl_bt_esl_log is disabled

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id: {
      uint32_t num_bondings;
      size_t   data_lenght;
      uint32_t bondings;
      uint32_t seed;
      uint8_t  flags = ESL_SECURITY_BASE_FLAGS;

      esl_sensor_battery_init();
      // This state  changes ESL internal status and calls for ESL boot
      // event handler (to be written by the users). After that, the data
      // for the characteristics shall be ready to be set for this lifecycle.
      esl_core_boot_event(); // e.g. users add their displays, leds, etc.

      // This function will write the appropriate characteristics - if present
      esl_image_characteristic_update();

      // limit the bonding count
      sc = sl_bt_sm_store_bonding_configuration(ESL_MAX_BONDINGS,
                                                ESL_BONDING_POLICY_FLAGS);
      sl_bt_esl_assert(sc == SL_STATUS_OK);

      sc = sl_bt_sm_get_bonding_handles(0,
                                        &num_bondings,
                                        sizeof(bondings),
                                        &data_lenght,
                                        (uint8_t *)&bondings);
      sl_bt_esl_assert(sc == SL_STATUS_OK);

      if (!num_bondings || num_bondings > ESL_MAX_BONDINGS) {
        // delete all just in case, ignore status
        (void)sl_bt_sm_delete_bondings();
        // accept bonding since now none is available
        sc = sl_bt_sm_set_bondable_mode(ESL_TRUE);
        sl_bt_esl_assert(sc == SL_STATUS_OK);
      } else {
        // do not accept more bondings
        sc = sl_bt_sm_set_bondable_mode(ESL_FALSE);
        sl_bt_esl_assert(sc == SL_STATUS_OK);

#if !ESL_TAG_INTERMITTENT_ADVERTISING
        // ESL_SECURITY_BONDED_ONLY option is incompatible with Intermittent
        // Advertising ESL Core custom feature as the BLE stack doesn't emit the
        // events necessary for proper advertisement handling if the flag
        // ESL_SECURITY_BONDED_ONLY is set and an untrusted device tried
        // connecting. With the bonded-only SM flag set, the stack won't emit
        // any event for such connections eventually, which then leaves the
        // "other" connectable advertising disabled after the untrusted device
        // has been disconnected silently.
        // Extend security flags if allowed by the configuration
        flags |= ESL_SECURITY_BONDED_ONLY;
#endif // ESL_TAG_INTERMITTENT_ADVERTISING

        // set esl_tag.bonding_handle, default 0
        esl_tag.bonding_handle = 0;

        // find non-zero handle, if so
        while ((bondings & 1) == 0 && data_lenght > 0) {
          // increment bonding handle until the proper one found
          if ((++esl_tag.bonding_handle & 0x07) == 0) {
            // decrement the bondings_len after each byte checked
            --data_lenght;
          }

          bondings >>= 1;
        }
      }

      // configure security options
      sc = sl_bt_sm_configure(flags, sl_bt_sm_io_capability_noinputnooutput);
      sl_bt_esl_assert(sc == SL_STATUS_OK);

      // create primary advertising set
      sc = sl_bt_advertiser_create_set(&esl_tag_persistent.advertising_set_handle[0]);
      sl_bt_esl_assert(sc == SL_STATUS_OK);

#if ESL_TAG_INTERMITTENT_ADVERTISING
      // create secondary advertising set for speeding up connection requests
      sc = sl_bt_advertiser_create_set(&esl_tag_persistent.advertising_set_handle[1]);
      sl_bt_esl_assert(sc == SL_STATUS_OK);
#endif // ESL_TAG_INTERMITTENT_ADVERTISING
      sc = sl_bt_system_get_random_data(sizeof(uint32_t) - 1,
                                        sizeof(seed),
                                        &data_lenght,
                                        (uint8_t *)&seed);
      sl_bt_esl_assert(sc == SL_STATUS_OK);
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_DEBUG,
                    "Started with random seed: %lu.",
                    seed);
      // set an arbitrary, initial random value for the ESL EAD Randomizer
      esl_core_response_randomizer.packet_counter = 0xfa7eful * seed;
      esl_tag.config_status |= ESL_CONFIG_FLAG_RANDOMIZER_READY;
      // init stdlib's rand() generator by our TRNG seed
      srand(seed);
      // Set custom data for primary advertising, according to ESLS v1.0, section 2.7.3.1 expose the ESL Service UUID
      sc = sl_bt_legacy_advertiser_set_data(esl_tag_persistent.advertising_set_handle[0],
                                            sl_bt_advertiser_advertising_data_packet,
                                            sizeof(esl_core_advertising_data),
                                            esl_core_advertising_data);
      sl_bt_esl_assert(sc == SL_STATUS_OK);
      // Start advertising before leave the boot state
      sc = esl_core_start_advertising();
      sl_bt_esl_assert(sc == SL_STATUS_OK);
    } break;

    // This state doesn't care about anything else than the stack boot event.
    default:
      break;
  }
}

static void esl_state_connectable_handler(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  (void)sc; // suppress the compiler warning if sl_bt_esl_assert disabled

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_sm_bonding_failed_id:
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_ERROR,
                    "Bond fail reason: %d.",
                    evt->data.evt_sm_bonding_failed.reason);
      if (evt->data.evt_sm_bonding_failed.reason == SL_STATUS_BT_CTRL_PIN_OR_KEY_MISSING) {
        // can happen if the key was cleared on the Tag
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_WARNING,
                      "Bonding key missing, close.");
        // safe to ignore the return value below
        (void)sl_bt_connection_close(evt->data.evt_sm_bonding_failed.connection);
      } else if (evt->data.evt_sm_bonding_failed.reason == SL_STATUS_BT_SMP_PAIRING_NOT_SUPPORTED) {
        // can happen if another AP tries to connect or the bonded AP lost the key
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_WARNING,
                      "Only one bonding is permitted, close.");
        sl_bt_connection_close(evt->data.evt_sm_bonding_failed.connection);

        // check if it's the already bonded AP - if it is, then retry bonding
        if (memcmp(&esl_tag.ap_address.addr, ESL_INVALID_AP_ADDRESS, sizeof(bd_addr)) != 0) {
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                        ESL_LOG_LEVEL_WARNING,
                        "AP seemingly lost LTK key, retry bonding.");

          if (esl_tag.status > esl_state_unassociated) {
            esl_core_unassociate();
          }
        }
      }
      break;

    case sl_bt_evt_connection_parameters_id:
      if (evt->data.evt_connection_parameters.security_mode > sl_bt_connection_mode1_level1
          && esl_tag.bonding_handle != SL_BT_INVALID_BONDING_HANDLE) {
        // pre-configure PAST receiver for the current connection
        sl_bt_past_receiver_set_sync_receive_parameters(evt->data.evt_connection_parameters.connection,
                                                        sl_bt_past_receiver_mode_synchronize,
                                                        ESL_SYNC_SKIP,
                                                        ESL_SYNC_MAX_TIMEOUT,
                                                        sl_bt_sync_report_all);

        // further internal state depends on the status of ESL configuration
        if (is_esl_configured_for(ESL_CONFIG_FLAGS_MANDATORY_MASK)) {
          esl_tag.status = esl_state_updating;
        } else {
          esl_tag.status = esl_state_configuring;
        }
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_INFO,
                      "State transition to %s due connection parameters change",
                      esl_states_string[esl_tag.status]);
      }
      break;

    case sl_bt_evt_sm_bonded_id:
      if (esl_tag.bonding_handle == SL_BT_INVALID_BONDING_HANDLE) {
        uint8_t  flags = ESL_SECURITY_BASE_FLAGS;
        esl_tag.bonding_handle = evt->data.evt_sm_bonded.bonding;
#if !ESL_TAG_INTERMITTENT_ADVERTISING
        // Extend security flags if allowed by the configuration
        flags |= ESL_SECURITY_BONDED_ONLY;
#endif // ESL_TAG_INTERMITTENT_ADVERTISING
        sc = sl_bt_sm_configure(flags,
                                sl_bt_sm_io_capability_noinputnooutput);
        sl_bt_esl_assert(sc == SL_STATUS_OK);
        // do not accept bonding anymore
        (void)sl_bt_sm_set_bondable_mode(ESL_FALSE);
        // at this point the internal state shall be esl_state_configuring
        esl_tag.status = esl_state_configuring;
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_INFO,
                      "State transition to %s on bonding",
                      esl_states_string[esl_tag.status]);
        // pre-configure PAST receiver for the current connection
        sl_bt_past_receiver_set_sync_receive_parameters(evt->data.evt_sm_bonded.connection,
                                                        sl_bt_past_receiver_mode_synchronize,
                                                        ESL_SYNC_SKIP,
                                                        ESL_SYNC_MAX_TIMEOUT,
                                                        sl_bt_sync_report_all);
      } else {
      #if (ESL_MAX_BONDINGS > 1) && (!ESL_SECURITY_BONDED_ONLY)
        // the pre-processor condition optimizes code size if the configuration allows
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_ERROR,
                      "Bonding not permitted, close.");
        (void)sl_bt_sm_delete_bonding(evt->data.evt_sm_bonded.bonding);
        sc = sl_bt_connection_close(evt->data.evt_sm_bonded.connection);
        sl_bt_esl_assert(sc == SL_STATUS_OK);
      #endif // (ESL_MAX_BONDINGS > 1) && (!ESL_SECURITY_BONDED_ONLY)
      }
      break;

    case sl_bt_evt_connection_opened_id:
    #if ESL_TAG_INTERMITTENT_ADVERTISING
      // request stop both advertisers, regardless of which one ended up being used to connect
      (void)sl_bt_advertiser_stop(esl_tag_persistent.advertising_set_handle[0]);
      (void)sl_bt_advertiser_stop(esl_tag_persistent.advertising_set_handle[1]);
    #endif // ESL_TAG_INTERMITTENT_ADVERTISING
      // Remove update complete flag
      esl_tag.config_status &= (uint8_t) (~ESL_CONFIG_FLAG_UPDATE_COMPLETE);
      // store connection handle for later use
      esl_tag.connection_handle = evt->data.evt_connection_opened.connection;

      // check if it is the only allowed connection
      // if not yet bonded, then any connection is allowed until the first
      // bonding request (descriptors etc. can be read by anyone)
      if (evt->data.evt_connection_opened.bonding == esl_tag.bonding_handle) {
        // store AP address
        memcpy(&esl_tag.ap_address.addr, evt->data.evt_connection_opened.address.addr, sizeof(bd_addr));

        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_INFO,
                      "Connection opened");

        // Do not close sync, if it was open before
        if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE | ESL_LOG_FLAG_APPEND,
                        ESL_LOG_LEVEL_INFO,
                        " - keeping sync with AP");
        }
      } else {
        memset(&esl_tag.ap_address, 0, sizeof(bd_addr));
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_WARNING,
                      "Foreign connection detected!");
      }

      // force-stop security watchdog if running
      (void)sl_sleeptimer_stop_timer(&esl_tag_persistent.watchdog_handle);
      esl_tag_persistent.watchdog_triggered = false;
      break;

    case sl_bt_evt_connection_closed_id:
      // reset connection handle - before re-enabling advertising!
      esl_tag.connection_handle     = SL_BT_INVALID_CONNECTION_HANDLE;
      // disable notification responses
      esl_tag.notifications_enabled = false;

      // according to ESL standard, both Unsynchronized and Unassociated states
      // need to re-start advertising on (probably sudden) connection close.
      (void)esl_core_start_advertising();
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "Close reason: %x",
                    evt->data.evt_connection_closed.reason);
      break;

    case sl_bt_evt_sync_closed_id:
      esl_sync_cleanup(evt->data.evt_sync_closed.reason);
      break;

    default:
      break;
  }
}

static void esl_state_configuring_handler(sl_bt_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_connection_parameters_id:
    // connection could be opened over PAwR, so this event needs to be handled
    // @suppress("No break at the end of case")
    /* FALLTHRU */
    case sl_bt_evt_sm_bonding_failed_id:
      // can happen if the bonded AP unexpectedly requests new bonding (e.g. lost the bonding key)
      esl_state_connectable_handler(evt);
      break;

    case sl_bt_evt_gatt_server_user_write_request_id: {
      const uint16_t offset = evt->data.evt_gatt_server_user_write_request.offset;
      const uint16_t len = evt->data.evt_gatt_server_user_write_request.value.len;
      const uint16_t overall_size = offset + len;
      static uint8_t execute_write_flag = 0;

      if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_esl_control_point) {
        uint8_t cmd_len;
        cmd_len = evt->data.evt_gatt_server_attribute_value.value.data[0];
        cmd_len = esl_core_get_tlv_len(cmd_len) + 1; // opcode is one more byte!
        if (evt->data.evt_gatt_server_attribute_value.value.len == cmd_len) {
          sl_status_t sc;
          uint8_t *data = evt->data.evt_gatt_server_attribute_value.value.data;

          esl_core_purge_responses(ESL_CP_MAX_PAYLOAD_LENGTH);
          // process command sent to the ESL control point
          sc = esl_core_parse_all_opcodes(data, cmd_len);

          // send response only if required by the client
          if (evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_write_request) {
            (void)sl_bt_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
                                                             evt->data.evt_gatt_server_user_write_request.characteristic,
                                                             SL_STATUS_OK);
          } else {
            // in case of Write Without Response request, the result's just logged
            sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                          ESL_LOG_LEVEL_INFO,
                          "Write Without Response result: 0x%04lX",
                          sc);
          }
          (void)sc; // suppress warning if sl_bt_esl_log is disabled

          if (esl_tag.notifications_enabled) {
            uint8_t response[ESL_CP_MAX_PAYLOAD_LENGTH];
            // send notification from response queue
            uint8_t len = esl_core_get_responses(sizeof(response), response);

            if (len) {
              sl_bt_gatt_server_send_notification(evt->data.evt_gatt_server_user_write_request.connection,
                                                  evt->data.evt_gatt_server_user_write_request.characteristic,
                                                  len,
                                                  response);
            }
          }
          // restore current PAwR payload limit after opcode processing
          esl_core_purge_responses((uint8_t)esl_tag.response_payload_limit);
        }
      } else {
        sl_status_t sc = SL_STATUS_OK;

        switch (evt->data.evt_gatt_server_user_write_request.characteristic) {
          case gattdb_esl_address:
            if (evt->data.evt_gatt_server_user_write_request.value.len == sizeof(esl_address_t)) {
              // backup copy of old value, just in case
              esl_address_union_t address = *(esl_address_union_t *)evt->data.evt_gatt_server_user_write_request.value.data;

              if (esl_core_get_id(address.value) == ESL_BROADCAST_ID) {
                // do not allow the broadcast address to be set - neither by mistake!
                sc = SL_STATUS_BT_ATT_VALUE_NOT_ALLOWED;
                break;
              } else {
                // ignore RFU bit
                address.value  &= (ESL_GROUP_ID_MASK | ESL_ID_MASK);
              }

              if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
                // change subevent according to new group if already in sync
                sc = sl_bt_pawr_sync_set_sync_subevents(esl_tag.sync_handle,
                                                        sizeof(address.group_id),
                                                        &address.group_id);
                sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                              ESL_LOG_LEVEL_ERROR,
                              "ESL Address configuration failed, sc: 0x%04x",
                              sc);
              }

              // do not allow higher group address to be set than the subevent count
              if (sc == SL_STATUS_OK) {
                esl_tag.address = address;
                esl_tag.config_status |= ESL_CONFIG_FLAG_ESL_ADDRESS;

                sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                              ESL_LOG_LEVEL_INFO,
                              "ESL Address configured: 0x%04x",
                              esl_tag.address.value);
              } else {
                // this value is informative only, it's not standardized by ESLP
                sc = SL_STATUS_BT_ATT_OUT_OF_RANGE;
              }
            } else {
              sc = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;
            }
            break;

          case gattdb_esl_ap_sync_key:
            execute_write_flag = ESL_CONFIG_FLAG_ESL_AP_KEY;
            if ((evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_prepare_write_request && overall_size <= SL_BT_EAD_KEY_MATERIAL_SIZE)
                || (evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_write_request && overall_size == SL_BT_EAD_KEY_MATERIAL_SIZE)) {
              memcpy((void *)&esl_keys.ap_key.key[offset],
                     (void *)evt->data.evt_gatt_server_user_write_request.value.data,
                     len);

              sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                            ESL_LOG_LEVEL_INFO,
                            "Write AP Sync Key Material");

              if (overall_size == SL_BT_EAD_KEY_MATERIAL_SIZE) {
                psa_key_id_t key_id = ESL_CORE_CRYPTO_AP_SYNC_KEY_ID;
                // Store the session key part
                sc = sl_bt_ead_store_key(PSA_KEY_USAGE_DECRYPT,
                                         PSA_KEY_LIFETIME_VOLATILE,
                                         &esl_keys.ap_key, &key_id);

                if (sc == SL_STATUS_OK) {
                  esl_tag.config_status |= execute_write_flag;
                }
              }
            } else {
              sc = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;
              memset((void *)&esl_keys.ap_key, 0, SL_BT_EAD_KEY_MATERIAL_SIZE);
              esl_tag.config_status &= (uint8_t) ~execute_write_flag;
            }
#if ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED
            // "delete" the known previous randomizer, whenever the AP writes its key material
            esl_tag.last_ap_randomizer = NULL;
#endif // ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED
            break;

          case gattdb_esl_response_key:
            execute_write_flag = ESL_CONFIG_FLAG_ESL_RESPONSE_KEY;
            if ((evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_prepare_write_request && overall_size <= SL_BT_EAD_KEY_MATERIAL_SIZE)
                || (evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_write_request && overall_size == SL_BT_EAD_KEY_MATERIAL_SIZE)) {
              memcpy((void *)&esl_keys.response_key.key[offset],
                     (void *)evt->data.evt_gatt_server_user_write_request.value.data,
                     len);

              sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                            ESL_LOG_LEVEL_INFO,
                            "Write AP Response Key Material");

              if (overall_size == SL_BT_EAD_KEY_MATERIAL_SIZE) {
                psa_key_id_t key_id = ESL_CORE_CRYPTO_RESPONSE_KEY_ID;
                // Store the session key part
                sc = sl_bt_ead_store_key(PSA_KEY_USAGE_ENCRYPT,
                                         PSA_KEY_LIFETIME_VOLATILE,
                                         &esl_keys.response_key, &key_id);
                // get new randomizer value whenever the Response Key Material is written
                (void)app_scheduler_add(&esl_core_async_randomizer, NULL, 0, NULL);
                if (sc == SL_STATUS_OK) {
                  esl_tag.config_status |= execute_write_flag;
                }
                break;
              }
            } else {
              sc = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;
              memset((void *)&esl_keys.response_key, 0, SL_BT_EAD_KEY_MATERIAL_SIZE);
              esl_tag.config_status &= (uint8_t) ~execute_write_flag;
            }
            break;

          case gattdb_esl_absolute_time:
            if (evt->data.evt_gatt_server_user_write_request.value.len == sizeof(uint32_t)) {
              CORE_ATOMIC_SECTION(
                esl_tag.time_offset  = *(uint32_t *)evt->data.evt_gatt_server_user_write_request.value.data;
                esl_tag.time_offset -= sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
                )
              esl_tag.config_status |= ESL_CONFIG_FLAG_ESL_ABS_TIME;
              sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                            ESL_LOG_LEVEL_INFO,
                            "ESL Absolute Time configured to: 0x%lx",
                            *(uint32_t *)evt->data.evt_gatt_server_user_write_request.value.data);
            } else {
              esl_tag.config_status &= (uint8_t) ~ESL_CONFIG_FLAG_ESL_ABS_TIME;
              sc = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;
            }
            break;

          default:
            if (evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_execute_write_request) {
              // characteristic value will be 0 for sl_bt_gatt_execute_write_request and shall be ignored
              if (!execute_write_flag) {
                // flag shall be set before sl_bt_gatt_execute_write_request can be accepted!
                sc = SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED;
                sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                              ESL_LOG_LEVEL_WARNING,
                              "Invalid GATT Execute Write Request ignored");
              } else if ((esl_tag.config_status & execute_write_flag) == 0) {
                void *data = &esl_keys.ap_key;
                // error case: insufficient write length before the sl_bt_gatt_execute_write_request
                sc = SL_STATUS_BT_ATT_INVALID_ATT_LENGTH;

                if (execute_write_flag == ESL_CONFIG_FLAG_ESL_RESPONSE_KEY) {
                  // change data pointer if needed
                  data = &esl_keys.response_key;
                }
                // clear corresponding data
                memset(data, 0, SL_BT_EAD_KEY_MATERIAL_SIZE);

                sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                              ESL_LOG_LEVEL_ERROR,
                              "Key Material writing failed due length mismatch");
              } else {
                sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                              ESL_LOG_LEVEL_INFO,
                              "Key Material written successfully");
              }
              // clear the flag after execution - either was it successful or not
              execute_write_flag = 0;
            } else {
              // let other components try process the requests unknown to ESL core
              sc = SL_STATUS_BT_ATT_REQUEST_NOT_SUPPORTED;
            }
            break;
        }

        if (sc != SL_STATUS_BT_ATT_REQUEST_NOT_SUPPORTED) {
          if (evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_write_request
              || evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_execute_write_request) {
            sl_bt_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
                                                       evt->data.evt_gatt_server_user_write_request.characteristic,
                                                       (uint8_t)sc);
          } else if (evt->data.evt_gatt_server_user_write_request.att_opcode == sl_bt_gatt_prepare_write_request) {
            sl_bt_gatt_server_send_user_prepare_write_response(evt->data.evt_gatt_server_user_write_request.connection,
                                                               evt->data.evt_gatt_server_user_write_request.characteristic,
                                                               (uint8_t)sc,
                                                               evt->data.evt_gatt_server_user_write_request.offset,
                                                               evt->data.evt_gatt_server_user_write_request.value.len,
                                                               (uint8_t *)evt->data.evt_gatt_server_user_write_request.value.data);
          }
        }
      }
    } break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      if (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_esl_control_point
          && evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_client_config) {
        switch (evt->data.evt_gatt_server_characteristic_status.client_config_flags) {
          case sl_bt_gatt_server_notification:
          case sl_bt_gatt_server_notification_and_indication:
            esl_tag.notifications_enabled = true;
            sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                          ESL_LOG_LEVEL_INFO,
                          "AP subscribed to ECP notifications");
            break;

          case sl_bt_gatt_server_disable:
            sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                          ESL_LOG_LEVEL_DEBUG,
                          "AP unsubscribed from ECP notifications");
            esl_tag.notifications_enabled = false;
            break;
        }
      }
      break;

    case sl_bt_evt_periodic_sync_transfer_received_id:
      // error case, close the sync and connection immediately.
      (void)sl_bt_sync_close(evt->data.evt_periodic_sync_transfer_received.sync);
      // closing the connection will force the fall-back mechanism (sync by scan)
      (void)sl_bt_connection_close(evt->data.evt_periodic_sync_transfer_received.connection);
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_WARNING,
                    "Unsupported PAST sync detected, ignore.");
      break;

    case sl_bt_evt_pawr_sync_transfer_received_id:
      // the connection will be closed regardless of status
      (void)sl_bt_connection_close(evt->data.evt_pawr_sync_transfer_received.connection);

      if (evt->data.evt_pawr_sync_transfer_received.status == SL_STATUS_OK) {
        sl_status_t sc = SL_STATUS_OK;
        uint32_t interval;
        uint32_t sync_timeout;

        // init the internal anti-drift helper
        esl_tag.pawr_timestamp = sl_sleeptimer_get_tick_count64();
        esl_tag.tick_error = 0;
        // as we're going to use only integer math for speed, we need the
        // following tricks for keeping full accuracy
        interval = ESL_PAWR_INTEGER_INTERVAL
                   * evt->data.evt_pawr_sync_transfer_received.adv_interval;
        // get the base for the timeout value from the adv_interval that arrives in unit of 1.25 ms
        sync_timeout = (10 * evt->data.evt_pawr_sync_transfer_received.adv_interval) / 8;
        // accept the sync transfer only from the bonded AP according to ESL spec.

        if (evt->data.evt_pawr_sync_transfer_received.bonding != esl_tag.bonding_handle) {
          // close connection if the transfer is not from the bonded AP
          (void)sl_bt_sync_close(evt->data.evt_pawr_sync_transfer_received.sync);
        } else {
          esl_core_set_basic_state_bit(ESL_BASIC_STATE_SYNCHRONIZED_BIT, ESL_SET);
          esl_tag.sync_handle = evt->data.evt_pawr_sync_transfer_received.sync;
          esl_tag.response_payload_limit =
            esl_get_response_payload_limit(evt->data.evt_pawr_sync_transfer_received.adv_phy,
                                           evt->data.evt_pawr_sync_transfer_received.response_slot_spacing);
          // calculated response slot payload size may significantly exceed the limit specified by the ESL spec.!
          if (esl_tag.response_payload_limit > ESL_PAYLOAD_MAX_LENGTH) {
            // ensure the actual payload limit conforms to the ESLP.
            esl_tag.response_payload_limit = ESL_PAYLOAD_MAX_LENGTH;
          }
          // prepare the response queue size limit
          esl_core_purge_responses((uint8_t)esl_tag.response_payload_limit);
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                        ESL_LOG_LEVEL_INFO,
                        "PAST done, %d ms PAwR sync service opened, response payload size limit is %u bytes",
                        sync_timeout,
                        esl_tag.response_payload_limit);
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                        ESL_LOG_LEVEL_DEBUG,
                        "Remote clock accuracy: %d ppm",
                        evt->data.evt_pawr_sync_transfer_received.clock_accuracy);

          esl_core_update_sync_parameters(sync_timeout, ESL_SYNC_SKIP);

          // due to the earlier multiplication trick, this will always give an
          // integer value equal to one hundred times the current interval!
          (void)sl_sleeptimer_ms32_to_tick(interval,
                                           &esl_tag.pawr_interval_ticks);
          // set listener subevent according to current ESL group
          sc = sl_bt_pawr_sync_set_sync_subevents(esl_tag.sync_handle,
                                                  sizeof(esl_tag.address.group_id),
                                                  &esl_tag.address.group_id);
          if (sc != SL_STATUS_OK) {
            (void)sl_bt_sync_close(esl_tag.sync_handle);
#if ESL_TAG_SYNC_SCAN_ENABLE
            esl_tag_persistent.scan_permitted = false;
#endif // #if ESL_TAG_SYNC_SCAN_ENABLE
            sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                          ESL_LOG_LEVEL_ERROR,
                          "Sync close due set sync subevent failure with sc: 0x%04x for se: 0x%02x",
                          sc,
                          esl_tag.address.group_id);
          }
        }
      } else {
        // clean up sync info
        esl_sync_cleanup(evt->data.evt_pawr_sync_transfer_received.status);
      }
      break;

    case sl_bt_evt_connection_closed_id:
      // reset connection handle - before re-enabling advertising!
      esl_tag.connection_handle = SL_BT_INVALID_CONNECTION_HANDLE;
      // disable notification responses
      esl_tag.notifications_enabled = false;

      if (!is_esl_configured_for(ESL_CONFIG_FLAGS_MANDATORY_MASK)) {
        // esl_service_d095r08 Section 2.7.3.2.1
        esl_core_unassociate();
      } else {
        // set synchronized state if sync exists and Update Complete received
        if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE
            && is_esl_configured_for(ESL_CONFIG_FLAG_UPDATE_COMPLETE)) {
          esl_tag.status = esl_state_synchronized;
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                        ESL_LOG_LEVEL_INFO,
                        "State transition to %s",
                        esl_states_string[esl_tag.status]);
          // start watchdog again according to ESLP security requirement (d095r16, 8.3)
          (void)sl_sleeptimer_restart_periodic_timer_ms(&esl_tag_persistent.watchdog_handle,
                                                        ESL_CORE_SECURITY_TIMEOUT_MS,
                                                        &esl_security_timeout,
                                                        NULL,
                                                        ESL_CORE_SECURITY_TIMER_PRIORITY,
                                                        SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
        } else {
          // out of sync or link loss before Update Complete: start advertising
          (void)esl_core_start_advertising(); // will set appropriate state
        }
      }
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "Close reason: %x",
                    evt->data.evt_connection_closed.reason);
      break;

    case sl_bt_evt_sync_closed_id:
      // In case of connection opened over PAwR the sync may be closed already
      if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_INFO,
                      "Sync closed due: %x",
                      evt->data.evt_sync_closed.reason);
        esl_sync_cleanup(evt->data.evt_sync_closed.reason);
      }
      break;

    default:
      break;
  }
}

static void esl_state_synchronized_handler(sl_bt_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_connection_opened_id:
      // connection can be initiated over PAwR, so this event has to be handled
      esl_state_connectable_handler(evt);
      break;

    case sl_bt_evt_pawr_sync_subevent_report_id:
      // skip any incomplete data
      if (evt->data.evt_pawr_sync_subevent_report.data_status == 0) {
        uint64_t tick_now = sl_sleeptimer_get_tick_count64();
        uint64_t ticks_elapsed, ticks_rounded_up, ticks_expected;
        uint32_t interval_count;
        uint8_t* msg = evt->data.evt_pawr_sync_subevent_report.data.data;
        uint8_t len = evt->data.evt_pawr_sync_subevent_report.data.len;
        const int32_t error_threshold = sl_sleeptimer_get_timer_frequency();

        // try decrypting the message
        msg = esl_core_decrypt_message(msg, &len);

        // also skip processing any improperly encrypted data:
        // (skip msg == NULL and improper length and/or type)
        if (msg && msg[0] == --len && msg[1] == ESL_AD_TYPE) {
#if ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED
          uint64_t packet_counter = 0;
          // get AP sync packet current randomizer value
          memcpy(&packet_counter,
                 &evt->data.evt_pawr_sync_subevent_report.data.data[SL_BT_EAD_HEADER_SIZE],
                 SL_BT_EAD_RANDOMIZER_SIZE);
          // initial state check for substitute previous value generation
          if (esl_tag.last_ap_randomizer == NULL) {
            // make the base "valid" by setting the static pointer if it was NULL
            esl_tag.last_ap_randomizer = &esl_core_ap_randomizer_base;
            // but create a substitute value as there was a previous value
            // (since we can never know the start "random" value in advance)
            // masking is necessary to avoid the 40 bit underflow
            esl_tag.last_ap_randomizer->packet_counter = (packet_counter - 1) & 0xffffffffffu;
          }
          // this check below now can prevent (2^40) replay attempts
          if (((int64_t)packet_counter - (int64_t)esl_tag.last_ap_randomizer->packet_counter) <= 0) {
            // 0 result means wrap-around while negative values mean a replay attempt
            break;
          }
          // save current AP sync randomizer value as last known good value
          esl_tag.last_ap_randomizer->packet_counter = packet_counter;
#endif // ESL_TAG_EAD_REPLAY_MITIGATION_ENABLED
          esl_core_signal_valid_message();
          // backup vales for response
          esl_tag_persistent.request_event = evt->data.evt_pawr_sync_subevent_report.event_counter;
          esl_tag_persistent.request_subevent = evt->data.evt_pawr_sync_subevent_report.subevent;

          // While in the Updating state, the ESL shall ignore any data present
          // on the LE PAwR logical transport
          if (esl_tag.status == esl_state_synchronized) {
            esl_group_id_t group_id = esl_core_get_group_id(esl_tag.address.value);

            // re-start watchdog according to ESLP security requirement
            (void)sl_sleeptimer_restart_periodic_timer_ms(&esl_tag_persistent.watchdog_handle,
                                                          ESL_CORE_SECURITY_TIMEOUT_MS,
                                                          &esl_security_timeout,
                                                          NULL,
                                                          ESL_CORE_SECURITY_TIMER_PRIORITY,
                                                          SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);

            // check if command(s) addressed to this group
            if (*(esl_group_id_t *)&msg[ESL_PAYLOAD_OVERHEAD] == group_id) {
              // declare local static for proper battery voltage measure interval
              static uint32_t last_measured = (UINT32_MAX - ESL_TAG_BATTERY_MEASUREMENT_INTERVAL_MS);
              uint32_t now = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());

              // only measure if the requested interval has passed (saves power)
              if ((uint32_t)(now - last_measured) >= ESL_TAG_BATTERY_MEASUREMENT_INTERVAL_MS) {
                // save time stamp
                last_measured = now;
                // do the measurement once
                esl_sensor_battery_read();
              }

              (void)esl_core_parse_all_opcodes(&msg[ESL_PAYLOAD_OVERHEAD + 1],
                                               (uint8_t)(len - ESL_PAYLOAD_OVERHEAD));
            }
          } else {
            sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                          ESL_LOG_LEVEL_INFO,
                          "Ignore command over PAwR while connected: ");
            sl_bt_esl_log_hexdump(ESL_LOG_COMPONENT_CORE | ESL_LOG_FLAG_APPEND,
                                  ESL_LOG_LEVEL_INFO,
                                  &msg[ESL_PAYLOAD_OVERHEAD + 1],
                                  (uint8_t)(len - ESL_PAYLOAD_OVERHEAD));
          }
        }
        ticks_elapsed = (uint64_t)(tick_now - esl_tag.pawr_timestamp);
        // 'tick_now' may have overflowed - in that edge case the elapsed time
        // also overflows and become suddenly "negative", so handle this properly
        if ((int64_t)ticks_elapsed < 0) {
          ticks_elapsed = -ticks_elapsed; // turn back to positive
        }

        // as we used multiplication for keeping full integer accuracy, we need
        // to do the same here as well
        ticks_elapsed      *= ESL_DRIFT_INTEGER_MULTIPLIER;
        ticks_rounded_up    = ticks_elapsed + (esl_tag.pawr_interval_ticks >> 1);
        interval_count      = ticks_rounded_up / esl_tag.pawr_interval_ticks;
        ticks_expected      = interval_count * esl_tag.pawr_interval_ticks;

        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_DEBUG,
                      "Interval%c %u, measured: %lu, expected: %lu [Ticks]",
                      (interval_count == 1 ? '\0' : 's'),
                      (uint8_t)interval_count,
                      ((uint32_t)ticks_elapsed / ESL_DRIFT_INTEGER_MULTIPLIER),
                      ((uint32_t)ticks_expected) / ESL_DRIFT_INTEGER_MULTIPLIER);

        if (interval_count != 0) {
          // as we still have 'only' hundred times of the real difference,
          // we have multiply by ten again, to reach the full sleeptimer
          // resolution and thus not to lose the threshold precision.
          esl_tag.tick_error -= (int32_t)(ticks_expected - ticks_elapsed)
                                * (int32_t)(ESL_DRIFT_THRESHOLD_DIVISOR
                                            / ESL_DRIFT_INTEGER_MULTIPLIER);
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                        ESL_LOG_LEVEL_DEBUG,
                        "Error: %ld mTicks (sum: %d.%03d T) @ nominal %ldHz",
                        (int32_t)(ticks_elapsed - ticks_expected),
                        (int16_t)(esl_tag.tick_error / (int32_t)ESL_DRIFT_THRESHOLD_DIVISOR),
                        (uint16_t)abs((esl_tag.tick_error % (int32_t)ESL_DRIFT_THRESHOLD_DIVISOR)),
                        error_threshold);
        }

        // save current tick count
        esl_tag.pawr_timestamp = tick_now;

        if (esl_tag.tick_error >= error_threshold) {
          esl_tag.tick_error -= error_threshold;
          esl_tag.time_offset--;
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                        ESL_LOG_LEVEL_DEBUG,
                        "LFXO is too fast, Absolute Time adjusted down!");
        } else if (esl_tag.tick_error <= -error_threshold) {
          esl_tag.tick_error += error_threshold;
          esl_tag.time_offset++;
          sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                        ESL_LOG_LEVEL_DEBUG,
                        "LFXO is too slow, Absolute Time adjusted up!");
        } else {
          // nothing more to do until the time drifts
          break;
        }

        if (sl_sleeptimer_tick64_to_ms(tick_now, &ticks_elapsed) == SL_STATUS_OK) {
          // do the time offset correction if it's possible and necessary
          interval_count = (uint32_t)ticks_elapsed + esl_tag.time_offset;
          esl_reschedule_delayed_commands(interval_count);
        }
      }
      break;
#if ESL_TAG_SYNC_SCAN_ENABLE
    case sl_bt_evt_scanner_extended_advertisement_report_id:
      // Only interested in extended advertisements
      if (esl_tag.sync_handle == SL_BT_INVALID_SYNC_HANDLE) {
        // Accept incoming advertisements only from the bonded ESL Access Point
        if (evt->data.evt_scanner_extended_advertisement_report.bonding == esl_tag.bonding_handle) {
          // get proper timeout value, which arrives in unit of 1.25 ms
          uint32_t pawr_interval = (10 * evt->data.evt_scanner_extended_advertisement_report.periodic_interval) / 8;
          uint32_t sync_timeout = ((ESL_TAG_MAX_SYNC_LOST_COUNT * pawr_interval) + ESL_SYNC_MIN_TIMEOUT) / 10;

          // Only if the periodic_interval implies the previously lost periodic advertisement train...
          if (sync_timeout == esl_tag.sync_timeout) {
            sl_status_t sc;

            // This will cause the sl_bt_evt_scanner_extended_advertisement_report_id to be stopped from now on
            (void)sl_bt_scanner_stop();
            sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                          ESL_LOG_LEVEL_INFO,
                          "Found %d ms periodic sync service, attempting to open",
                          pawr_interval);

            sc = sl_bt_sync_scanner_set_sync_parameters(0,
                                                        sync_timeout,
                                                        sl_bt_sync_report_all);
            if (sc == SL_STATUS_OK) {
              // sl_bt_evt_pawr_sync_opened_id event must arrive within the total scan timeout or the ESL will start advertising
              sc = sl_bt_sync_scanner_open(evt->data.evt_scanner_extended_advertisement_report.address,
                                           evt->data.evt_scanner_extended_advertisement_report.address_type,
                                           evt->data.evt_scanner_extended_advertisement_report.adv_sid,
                                           &esl_tag.sync_handle);
            }

            if (sc != SL_STATUS_OK) {
              (void)app_scheduler_remove(esl_tag_persistent.scan_timer_task);
              // Fall back to advertising in case of error
              esl_tag.sync_handle = SL_BT_INVALID_SYNC_HANDLE;

              sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                            ESL_LOG_LEVEL_WARNING,
                            "Sync-by-scan failed due 0x%04x",
                            sc);
              (void)esl_core_start_advertising();
              esl_tag_persistent.scan_permitted = true;
              esl_tag_persistent.scan_count = 0;
            }
          }
        }
      }
      break;

    case sl_bt_evt_periodic_sync_opened_id:
      (void)app_scheduler_remove(esl_tag_persistent.scan_timer_task);
      esl_tag_persistent.scan_timer_task = NULL;
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_WARNING,
                    "Unsupported sync train detected, will close!");
      sl_bt_sync_close(esl_tag.sync_handle);
      break;

    case sl_bt_evt_pawr_sync_opened_id:
      (void)app_scheduler_remove(esl_tag_persistent.scan_timer_task);
      esl_tag_persistent.scan_timer_task = NULL;
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "Synchronized by scanning successfully for the %u%s time!",
                    esl_tag_persistent.scan_count,
                    sli_bt_esl_log_get_ordinal(esl_tag_persistent.scan_count));
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "PAwR clock accuracy: %d ppm",
                    evt->data.evt_pawr_sync_opened.clock_accuracy);
      // now that sync is open, we can set ESL_BASIC_STATE_SYNCHRONIZED_BIT again
      esl_core_set_basic_state_bit(ESL_BASIC_STATE_SYNCHRONIZED_BIT, ESL_SET);
      // reset the correct subevent to listen to at the end
      (void)sl_bt_pawr_sync_set_sync_subevents(esl_tag.sync_handle,
                                               sizeof(esl_tag.address.group_id),
                                               &esl_tag.address.group_id);
      break;
#endif // ESL_TAG_SYNC_SCAN_ENABLE

    case sl_bt_evt_sync_closed_id:
      esl_sync_cleanup(evt->data.evt_sync_closed.reason);
      break;

    default:
      break;
  }
}

void esl_core_init(void)
{
  esl_core_init_hook();
  esl_core_respones_init();
  esl_led_init();
  esl_image_init();
  esl_sensor_init();
  esl_display_init();
}

SL_WEAK void esl_core_boot_event(void)
{
  // Do nothing special by default on stack boot event.
  // ESL vendors can override this behavior by writing their own implementation.
}

SL_WEAK void esl_core_unassociate_callback(void)
{
  // Do nothing by default right after the ESL entered to Unassociated state.
  // ESL vendors can override this behavior by writing their own implementation.
}

SL_WEAK void esl_core_update_complete_callback(void)
{
  // Do nothing by default after an update complete command executed successfully.
  // ESL vendors can override this behavior by writing their own implementation.
}

SL_WEAK void esl_core_service_reset_event(void)
{
  // Do nothing by default on service reset.
  // ESL vendors can override this behavior by writing their own implementation.
}

SL_WEAK void esl_core_init_hook(void)
{
  // Do nothing special by default on early init.
  // ESL vendors can override this behavior by writing their own implementation.
}

SL_WEAK void esl_core_shutdown_hook(void)
{
  // Do nothing special by default before complete shutdown.
  // ESL vendors can override this behavior by writing their own implementation.
}

void esl_core_bt_on_event(sl_bt_msg_t *evt)
{
  // Internal state machine skeleton code - actual implementation of the states
  // are out of scope of this event handler.
  switch (esl_tag.status) {
    case esl_state_boot:
      esl_state_boot_handler(evt);
      break;

    case esl_state_unassociated:
      // ESL Unassociated state is almost the same as the unsynchronized state,
      // thus both are calling the same event handler, while the small
      // differences will be handled internally.
      esl_state_connectable_handler(evt);
      break;

    case esl_state_configuring:
      esl_state_configuring_handler(evt);
      break;

    case esl_state_synchronized:
      esl_state_synchronized_handler(evt);
      if (esl_tag.connection_handle != SL_BT_INVALID_CONNECTION_HANDLE) {
        esl_state_configuring_handler(evt);
      }
      break;

    case esl_state_unsynchronized:
      // See the notes in esl_state_unassociated case.
      esl_state_connectable_handler(evt);
      break;

    case esl_state_updating:
      // ESL Updating state is almost the same as the configuring state, however
      // they are handled slightly different, internally.
      esl_state_configuring_handler(evt);
      // moreover, it can handle sync events as well (if needed).
      if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
        esl_state_synchronized_handler(evt);
      }
      break;

    default:
      // Invalid ESL state, recover by reset
      sl_bt_system_reboot();
      break;
  }
}

esl_state_t esl_core_get_status(void)
{
  return esl_tag.status;
}

sl_status_t esl_core_start_advertising(void)
{
  sl_status_t result = SL_STATUS_FAIL;
  esl_state_t new_esl_status = esl_tag.bonding_handle != SL_BT_INVALID_BONDING_HANDLE
                               ? esl_state_unsynchronized
                               : esl_state_unassociated;

  if (esl_tag.connection_handle == SL_BT_INVALID_CONNECTION_HANDLE) {
    // set random advertising interval.
    const uint32_t max_interval = ESL_TAG_ADVERTISING_INTERVAL_MAX;
    uint32_t       min_interval = ESL_TAG_ADVERTISING_INTERVAL_MIN;

    // force close sync if any (may happen if AP disconnects suddenly)
    if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
      (void)sl_bt_sync_close(esl_tag.sync_handle);
    }

    // this number has nothing to do with security, so it's OK to use stdlib's rand()
    min_interval  = (uint32_t)rand() % (max_interval - min_interval + 1);
    min_interval += ESL_TAG_ADVERTISING_INTERVAL_MIN;

    result = sl_bt_advertiser_set_timing(esl_tag_persistent.advertising_set_handle[0],
                                         16 * min_interval / 10, // configurator asks users for [ms],
                                         16 * max_interval / 10, // but our API expects in 0.625ms units
                                         0, // adv. duration
                                         0); // max. num. adv. events
#if ESL_TAG_INTERMITTENT_ADVERTISING
    // set the secondary advertiser with the same timing parameters as the primary one
    (void)sl_bt_advertiser_set_timing(esl_tag_persistent.advertising_set_handle[1],
                                      16 * min_interval / 10,
                                      16 * max_interval / 10,
                                      0,
                                      0);
#endif // ESL_TAG_INTERMITTENT_ADVERTISING
    sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                  ESL_LOG_LEVEL_INFO,
                  "Random advertising interval between %u and %u ms selected",
                  min_interval,
                  max_interval);

    if (result == SL_STATUS_OK) {
      // Start primary advertising and enable connections
      result = sl_bt_legacy_advertiser_start(esl_tag_persistent.advertising_set_handle[0],
                                             sl_bt_legacy_advertiser_connectable);
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "State transition from %s to %s",
                    esl_states_string[esl_tag.status],
                    esl_states_string[new_esl_status]);
      // change internal status
      esl_tag.status = new_esl_status;
    }

    if (esl_tag.status != esl_state_unassociated || ESL_TAG_POWER_DOWN_ENABLE) {
      uint32_t timeout_ms = ESL_CORE_SECURITY_TIMEOUT_MS;

      if (ESL_TAG_POWER_DOWN_ENABLE && esl_tag.status == esl_state_unassociated) {
        // override default timeout value only if the current ESL state is Unassociated AND energy saving is enabled
        timeout_ms = ESL_TAG_POWER_DOWN_TIMEOUT_MS;
        sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                      ESL_LOG_LEVEL_INFO,
                      "Will enter a power save mode after %d minutes of ineffective advertising.",
                      ESL_TAG_POWER_DOWN_TIMEOUT_MIN);
      }
      // start / restart watchdog timer whenever ESL starts advertising
      // except in Unassociated state IF the energy saving is disabled!
      (void)sl_sleeptimer_restart_periodic_timer_ms(&esl_tag_persistent.watchdog_handle,
                                                    timeout_ms,
                                                    &esl_security_timeout,
                                                    NULL,
                                                    ESL_CORE_SECURITY_TIMER_PRIORITY,
                                                    SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
    } else {
      sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                    ESL_LOG_LEVEL_INFO,
                    "Power saving disabled by ESL_TAG_POWER_DOWN_ENABLE config!");
    }
#if ESL_TAG_INTERMITTENT_ADVERTISING
    (void)app_scheduler_add_delayed(&esl_core_secondary_advertiser,
                                    ESL_TAG_SECONDARY_ADVERTISING_DELAY,
                                    NULL, 0, NULL);
#endif // ESL_TAG_INTERMITTENT_ADVERTISING
  }

  return result;
}

bool esl_core_has_notifications()
{
  return esl_tag.notifications_enabled;
}

esl_basic_state_t esl_core_get_basic_state()
{
  return esl_tag.basic_state;
}

uint8_t esl_core_get_basic_state_bit(uint8_t bit)
{
  uint8_t result = ESL_FALSE;

  if (!!(esl_tag.basic_state & (esl_basic_state_t) (1u << bit))) {
    result = ESL_TRUE;
  }

  return result;
}

esl_basic_state_t esl_core_set_basic_state_bit(uint8_t bit, uint8_t value)
{
  // Invalid basic state request.
  sl_bt_esl_assert(ESL_SET == value || ESL_CLEAR == value);

  CORE_ATOMIC_SECTION(
    esl_tag.basic_state &= (esl_basic_state_t) ~(1u << bit);
    esl_tag.basic_state |= (esl_basic_state_t) (value << bit);
    )

  return esl_tag.basic_state;
}

sl_status_t esl_core_get_randomizer(sl_bt_ead_randomizer_t randomizer)
{
  sl_status_t result = SL_STATUS_NOT_READY;

  if (!!(esl_tag.config_status & ESL_CONFIG_FLAG_RANDOMIZER_READY)) {
    // Increase packet counter for every new packet
    ++esl_core_response_randomizer.packet_counter;
    result = SL_STATUS_OK;
  } else {
    struct sl_bt_ead_nonce_s nonce;

    result = sl_bt_ead_randomizer_update(&nonce);

    if (result == SL_STATUS_OK) {
      memcpy((void *)esl_core_response_randomizer.randomizer, (void *)nonce.randomizer,
             SL_BT_EAD_RANDOMIZER_SIZE);
      esl_tag.config_status |= ESL_CONFIG_FLAG_RANDOMIZER_READY;
    }
  }

  if (result == SL_STATUS_OK && randomizer != NULL) {
    memcpy((void *)randomizer, (void *)esl_core_response_randomizer.randomizer,
           SL_BT_EAD_RANDOMIZER_SIZE);
  }

  return result;
}

sl_status_t esl_core_read_ap_key(sl_bt_ead_key_material_p ap_key)
{
  sl_status_t result = SL_STATUS_NOT_READY;

  if (!!(esl_tag.config_status & ESL_CONFIG_FLAG_ESL_AP_KEY)) {
    memcpy((void *)ap_key, (void *)&esl_keys.ap_key,
           SL_BT_EAD_KEY_MATERIAL_SIZE);
    result = SL_STATUS_OK;
  }

  return result;
}

sl_status_t esl_core_read_response_key(sl_bt_ead_key_material_p response_key)
{
  sl_status_t result = SL_STATUS_NOT_READY;

  if (!!(esl_tag.config_status & ESL_CONFIG_FLAG_ESL_RESPONSE_KEY)) {
    memcpy((void *)response_key, (void *)&esl_keys.response_key,
           SL_BT_EAD_KEY_MATERIAL_SIZE);
    result = SL_STATUS_OK;
  }

  return result;
}

sl_status_t esl_core_read_esl_address(esl_address_t *address)
{
  sl_status_t result = SL_STATUS_NOT_READY;

  if (!!(esl_tag.config_status & ESL_CONFIG_FLAG_ESL_ADDRESS)) {
    *address = esl_tag.address.value;
    result = SL_STATUS_OK;
  }
  return result;
}

sl_status_t esl_core_read_absolute_time(uint32_t *time)
{
  sl_status_t result = SL_STATUS_NOT_READY;

  if (!!(esl_tag.config_status & ESL_CONFIG_FLAG_ESL_ABS_TIME)) {
    uint64_t current_tick = sl_sleeptimer_get_tick_count64();
    uint64_t ms64         = 0;

    result = sl_sleeptimer_tick64_to_ms(current_tick, &ms64);

    if (result == SL_STATUS_OK) {
      // overflow is allowed, arithmetics will do the trick, anyway:
      *time = (uint32_t)ms64 + esl_tag.time_offset;
    }
  }

  return result;
}

sl_status_t esl_core_update_sync_parameters(uint32_t timeout, uint16_t skip)
{
  sl_status_t result = SL_STATUS_INVALID_STATE; // default response if unsynced

  if (!!timeout) {
    // calculate an appropriate sync timeout parameter value - but the latter
    // is expected in units of 10ms.
    timeout = ((ESL_TAG_MAX_SYNC_LOST_COUNT * timeout) + ESL_SYNC_MIN_TIMEOUT) / 10;

    // finally, keep sync_timeout within its limits according to documentation
    if (timeout < ESL_SYNC_MIN_TIMEOUT) {
      timeout = ESL_SYNC_MIN_TIMEOUT;
    } else if (timeout > ESL_SYNC_MAX_TIMEOUT) {
      timeout = ESL_SYNC_MAX_TIMEOUT;
    }
    // store new timeout value if any
    esl_tag.sync_timeout = timeout;
  }

  // adjust actual timeout value to the current skip parameter
  timeout = esl_tag.sync_timeout * (skip + 1);
  timeout = timeout > ESL_SYNC_MAX_TIMEOUT ? ESL_SYNC_MAX_TIMEOUT : timeout;

  if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
    // set an appropriate timeout for possible sync lost
    result = sl_bt_sync_update_sync_parameters(esl_tag.sync_handle,
                                               skip,
                                               (uint16_t)timeout);

    sl_bt_esl_log(ESL_LOG_COMPONENT_CORE,
                  ESL_LOG_LEVEL_INFO,
                  "Sync loss timeout was set to: %d ms, skip: %d.",
                  timeout * 10, skip);
  }

  return result;
}

/**************************************************************************//**
 * Request ESL to return the synchronized state once synchronized
 * @return sl_status_t
 *****************************************************************************/
sl_status_t esl_core_update_complete(void)
{
  sl_status_t result = SL_STATUS_INVALID_STATE;
  if (esl_tag.status == esl_state_updating
      || esl_tag.status == esl_state_configuring) {
    // Set update complete flag
    esl_tag.config_status |= ESL_CONFIG_FLAG_UPDATE_COMPLETE;
    esl_core_update_complete_callback();
    if (esl_tag.sync_handle != SL_BT_INVALID_SYNC_HANDLE) {
      // If ESL receives the Update Complete command and it is synchronized,
      // it shall immediately terminate the ACL connection and transition
      // to the Synchronized state.
      result = app_scheduler_add(&esl_core_async_disconnect, NULL, 0, NULL);
      // However, doing it literally "immediately" would cause possible loss of the
      // write response sent by the sl_bt_gatt_server_send_user_write_response()
      // following the opcode processing in the ESL, therefore we're going to
      // try defer close by one step cycle to avoid such unwanted side-effect.
      if (result != SL_STATUS_OK) {
        // Fall-back: disconnect immediately if defer attempt fails! Even though
        // the Core spec. allows remote device to flush all ACL packets when it
        // receives the disconnection command, we tried to avoid this at least.
        result = sl_bt_connection_close(esl_tag.connection_handle);
      }
    } else {
      // If ESL receives the Update Complete command and it is not
      // synchronized, it shall wait for synchronization to be established.
      result = SL_STATUS_OK;
    }
  }
  return result;
}

void esl_core_invalidate_config(void)
{
  esl_tag.config_status = 0;
}
