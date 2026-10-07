#include "migrations.h"

#include "interface_v1.h"
#include "interface_v2.h"

#define APPS_MENU_SETTINGS_V1_CLOCK_NAME "clock"
#define APPS_MENU_SETTINGS_V2_CLOCK_NAME "app.busy.clock"

typedef enum {
    AppsMenuSettingsMigrationIdxV1V2,
    AppsMenuSettingsMigrationIdxMax,
} AppsMenuSettingsMigrationIdx;

static bool apps_menu_settings_migration_v1_v2_callback(SettingProvider* settings) {
    furi_assert(settings);
    bool success = false;

    do {
        AppsMenuSettingsV1 settings_v1;

        if(!setting_provider_load(settings, &apps_menu_v1_settings_root, &settings_v1)) {
            break;
        }

        if(strncmp(
               settings_v1.active_application,
               APPS_MENU_SETTINGS_V1_CLOCK_NAME,
               APPS_MENU_ACTIVE_APPLICATION_MAX_LEN) != 0) {
            break;
        }

        AppsMenuSettingsV2 settings_v2;

        strncpy(
            settings_v2.active_application,
            APPS_MENU_SETTINGS_V2_CLOCK_NAME,
            APPS_MENU_ACTIVE_APPLICATION_MAX_LEN);

        if(!setting_provider_save(settings, &apps_menu_v2_settings_root, &settings_v2)) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

const SettingProviderMigration apps_menu_settings_migrations[] = {
    [AppsMenuSettingsMigrationIdxV1V2] =
        (const SettingProviderMigration){
            .target_version = 2,
            .migrate_callback = apps_menu_settings_migration_v1_v2_callback,
        },
};

static_assert(COUNT_OF(apps_menu_settings_migrations) == AppsMenuSettingsMigrationIdxMax);

const size_t apps_menu_settings_migrations_count = COUNT_OF(apps_menu_settings_migrations);
