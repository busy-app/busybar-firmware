/***************************************************************************//**
 * @file sl_wisun_app_setting_common.c
 * @brief Wi-SUN Application Common settings
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "sl_assert.h"
#include "sl_status.h"
#include "sl_string.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "sl_component_catalog.h"
#include "sl_wisun_app_setting_common.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Settings notification error bit
#define APP_SETTINGS_NOTIFICATION_ERROR_FLAG_BIT    (31U)

/// Settings notification error flag mask
#define APP_SETTINGS_NOTIFICATION_ERROR_FLAG_MSK    (1U << APP_SETTINGS_NOTIFICATION_ERROR_FLAG_BIT)

/// Setting notification descriptor
typedef struct app_setting_notif_dsc {
  /// Event ID
  osEventFlagsId_t evt_id;
  /// Notification type
  app_setting_notification_t type;
  /// Subscribed channels
  uint32_t subscribed_chs;
} app_setting_notif_dsc_t;

/// Count of available notifications
#define APP_SETTINGS_NOTIFICATION_COUNT             (8UL)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Acquire application common mutex
 * @details Internal common mutex lock
 *****************************************************************************/
__STATIC_INLINE void _wisun_mutex_common_acquire(void);

/**************************************************************************//**
 * @brief Release application common mutex
 * @details Internal common mutex release
 *****************************************************************************/
__STATIC_INLINE void _wisun_mutex_common_release(void);

/**************************************************************************//**
 * @brief Get notification entry from table
 * @details Helper function
 * @param[in] type Notification type
 * @return app_setting_notif_dsc_t* Notification entry on success or NULL on error
 *****************************************************************************/
static app_setting_notif_dsc_t *_get_notification_entry(app_setting_notification_t type);
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

/// App common settings mutex
static osMutexId_t _wisun_setting_common_mtx;

/// App settings mutex attribute
static const osMutexAttr_t _wisun_setting_common_mtx_attr = {
  .name      = "WisunSettingCommonMutex",
  .attr_bits = osMutexRecursive,
  .cb_mem    = NULL,
  .cb_size   = 0
};

/// Settings event flags attributes
static const osEventFlagsAttr_t _wisun_setting_evt_attr = {
  .name      = "AppWisunSettingEvtFlags",
  .attr_bits = 0,
  .cb_mem    = NULL,
  .cb_size   = 0
};

/// Notifications
static app_setting_notif_dsc_t _notifications[APP_SETTINGS_NOTIFICATION_COUNT] = {
  {
    .type = APP_SETTING_NOTIFICATION_SET_NETWORK_NAME,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  },
  {
    .type = APP_SETTING_NOTIFICATION_SET_NETWORK_SIZE,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  },
  {
    .type = APP_SETTING_NOTIFICATION_SET_TX_POWER,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  },
  {
    .type = APP_SETTING_NOTIFICATION_SET_PHY_CFG,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  },
  {
    .type = APP_SETTING_NOTIFICATION_SET_SSID,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  },
  {
    .type = APP_SETTING_NOTIFICATION_SET_PASSPHRASE,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  },
  {
    .type = APP_SETTING_NOTIFICATION_SET_SECURITY,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  },
  {
    .type = APP_SETTING_NOTIFICATION_SET_BR_SETTINGS,
    .evt_id = NULL,
    .subscribed_chs = (1U << APP_SETTING_DEFAULT_SUBSCRIPT_CH),
  }
};

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/* Initialization of Wi-SUN setting */
void app_wisun_setting_common_init(void)
{
  // init wisun common settings mutex
  _wisun_setting_common_mtx = osMutexNew(&_wisun_setting_common_mtx_attr);
  EFM_ASSERT(_wisun_setting_common_mtx != NULL);

  // init wisun settings event flags
  for (size_t idx = 0; idx < APP_SETTINGS_NOTIFICATION_COUNT; ++idx) {
    _notifications[idx].evt_id = osEventFlagsNew(&_wisun_setting_evt_attr);
    EFM_ASSERT(_notifications[idx].evt_id != NULL);
  }
}

sl_status_t app_wisun_setting_subscribe_notification(const app_setting_notification_t type,
                                                     uint8_t * const channel)
{
  app_setting_notif_dsc_t *notif = NULL;
  uint32_t mask = 0UL;

  notif = _get_notification_entry(type);
  if (notif == NULL || channel == NULL) {
    return SL_STATUS_FAIL;
  }

  _wisun_mutex_common_acquire();
  for (uint8_t i = 0U; i < sizeof(notif->subscribed_chs) * 8U; ++i) {
    mask = (1U << i);
    if (i != APP_SETTINGS_NOTIFICATION_ERROR_FLAG_BIT
        && !(notif->subscribed_chs & mask)) {
      *channel = i;
      notif->subscribed_chs |= mask;
      break;
    }
  }
  _wisun_mutex_common_release();

  return SL_STATUS_OK;
}

bool app_wisun_setting_is_notified(const app_setting_notification_t type,
                                   const uint8_t channel)
{
  uint32_t flags = 0UL;
  app_setting_notif_dsc_t *notif = NULL;

  notif = _get_notification_entry(type);
  if (notif == NULL) {
    return false;
  }
  flags = osEventFlagsWait(notif->evt_id, (1U << channel), osFlagsNoClear, 0UL);

  return (flags & APP_SETTINGS_NOTIFICATION_ERROR_FLAG_MSK)
         ? false : (bool) (flags & (1U << channel));
}

void app_wisun_setting_unsubscribe(const app_setting_notification_t type,
                                   const uint8_t channel)
{
  app_setting_notif_dsc_t *notif = NULL;

  notif = _get_notification_entry(type);
  if (notif == NULL) {
    return;
  }

  _wisun_mutex_common_acquire();
  notif->subscribed_chs &= ~(1U << channel);
  _wisun_mutex_common_release();
}

void app_wisun_setting_ack_notification(const app_setting_notification_t type,
                                        const uint8_t channel)
{
  app_setting_notif_dsc_t *notif = NULL;

  notif = _get_notification_entry(type);
  if (notif == NULL) {
    return;
  }

  (void) osEventFlagsClear(notif->evt_id, 1U << channel);
}

sl_status_t app_wisun_setting_notify(const app_setting_notification_t type)
{
  app_setting_notif_dsc_t *notif = NULL;
  uint32_t flags = 0UL;

  notif = _get_notification_entry(type);
  if (notif == NULL) {
    return SL_STATUS_FAIL;
  }
  flags = osEventFlagsSet(notif->evt_id, notif->subscribed_chs);
  if (flags & APP_SETTINGS_NOTIFICATION_ERROR_FLAG_MSK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/* Mutex acquire */
__STATIC_INLINE void _wisun_mutex_common_acquire(void)
{
  EFM_ASSERT(osMutexAcquire(_wisun_setting_common_mtx, osWaitForever) == osOK);
}

/* Mutex release */
__STATIC_INLINE void _wisun_mutex_common_release(void)
{
  EFM_ASSERT(osMutexRelease(_wisun_setting_common_mtx) == osOK);
}

static app_setting_notif_dsc_t *_get_notification_entry(app_setting_notification_t type)
{
  for (size_t idx = 0; idx < APP_SETTINGS_NOTIFICATION_COUNT; ++idx) {
    if (_notifications[idx].type == type) {
      return &_notifications[idx];
    }
  }
  return NULL;
}
