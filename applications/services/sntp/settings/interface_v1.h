#include <toolbox/setting_provider.h>
#include <utz/utz.h>

#define SNTP_SERVER_ADDRESS_MAX_LENGTH 64

typedef enum {
    SntpSettingV1IdxIsEnabled,
    SntpSettingV1IdxServerAddress,
    SntpSettingV1IdxTimezone,
    SntpSettingV1IdxBootDelay,
    SntpSettingV1IdxBackgroundSyncInterval,
    SntpSettingV1IdxRetrySyncInterval,

    SntpSettingV1IdxsCount,
} SntpSettingV1Idx;

typedef struct {
    bool is_enabled; /**< Whether SNTP service is enabled */
    char server_address[SNTP_SERVER_ADDRESS_MAX_LENGTH]; /**< SNTP server address (URL) */
    uzone_t timezone; /**< Timezone name */
    int boot_delay; /**< Delay after boot before first sync in seconds */
    int background_sync_interval; /**< Interval between automatic syncs in seconds */
    int retry_sync_interval; /**< Interval between sync retry attempts on failure in seconds */
} SntpSettingsV1;
