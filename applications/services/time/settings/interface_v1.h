#include "time_format.h"

#include <setting_provider/setting_provider.h>
#include <utz/utz.h>

#define TIME_SERVER_ADDRESS_MAX_LENGTH 64

typedef enum {
    TimeSettingV1IdxIsEnabled,
    TimeSettingV1IdxServerAddress,
    TimeSettingV1IdxTimezone,
    TimeSettingV1IdxBootDelay,
    TimeSettingV1IdxBackgroundSyncInterval,
    TimeSettingV1IdxRetrySyncInterval,
    TimeSettingV1IdxTimeFormat,

    TimeSettingV1IdxsCount,
} TimeSettingV1Idx;

typedef struct {
    bool is_enabled; /**< Whether TIME service is enabled */
    char server_address[TIME_SERVER_ADDRESS_MAX_LENGTH]; /**< TIME server address (URL) */
    utz_zone_t timezone; /**< Timezone */
    int boot_delay; /**< Delay after boot before first sync in seconds */
    int background_sync_interval; /**< Interval between automatic syncs in seconds */
    int retry_sync_interval; /**< Interval between sync retry attempts on failure in seconds */
    TimeSettingTimeFormat time_format; /**< Display time format (24h or 12h) */
} TimeSettingsV1;
