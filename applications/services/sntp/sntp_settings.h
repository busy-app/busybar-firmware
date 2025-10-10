/**
 * @file sntp_settings.h
 * @brief SNTP service settings and configuration.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SNTP_TIMEZONE_OFFSET_MIN     (-12 * 60)
#define SNTP_TIMEZONE_OFFSET_MAX     (+14 * 60)
#define SNTP_TIMEZONE_OFFSET_DEFAULT (+4 * 60)

#define SNTP_BACKGROUND_SYNC_INTERVAL_MIN     (2 * 60 * 60)
#define SNTP_BACKGROUND_SYNC_INTERVAL_MAX     (48 * 60 * 60)
#define SNTP_BACKGROUND_SYNC_INTERVAL_DEFAULT (3 * 60 * 60)

#define SNTP_RETRY_SYNC_INTERVAL_MIN     (5 * 60)
#define SNTP_RETRY_SYNC_INTERVAL_MAX     (60 * 60)
#define SNTP_RETRY_SYNC_INTERVAL_DEFAULT (10 * 60)

#define SNTP_BOOT_DELAY_MIN     (1 * 60)
#define SNTP_BOOT_DELAY_MAX     (10 * 60)
#define SNTP_BOOT_DELAY_DEFAULT (5 * 60)

#define SNTP_SERVER_ADDRESS_MIN_LEN 1
#define SNTP_SERVER_ADDRESS_MAX_LEN 64
#define SNTP_SERVER_ADDRESS_DEFAULT "udp://time.google.com:123"

#define SNTP_IS_ENABLED_DEFAULT true

/**
 * @brief SNTP service settings structure.
 */
typedef struct {
    int timezone_offset; /**< Timezone offset from UTC in minutes */
    int background_sync_interval; /**< Interval between automatic syncs in seconds */
    int retry_sync_interval; /**< Interval between sync retry attempts on failure in seconds */
    int boot_delay; /**< Delay after boot before first sync in seconds */
    char server_address[SNTP_SERVER_ADDRESS_MAX_LEN]; /**< SNTP server address (URL) */
    bool is_enabled; /**< Whether SNTP service is enabled */
} SntpSettings;

bool sntp_settings_load(SntpSettings* settings);

bool sntp_settings_save(const SntpSettings* settings);
