#include "../firmware_i.h"

#include <gui/modules/var_item_list.h>

#include <furi_hal_nvm.h>

typedef enum {
    FirmwareSettingsSceneSettingsEventSettingChange = FirmwareSettingsEventSceneEventsStart,
} FirmwareSettingsSceneSettingsEvent;

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;

    FuriMutex* updater_settings_mutex;
    UpdaterSettings updater_settings;
} FirmwareSettingsSceneSettings;

typedef enum {
    FirmwareSettingsSceneSettingsCheckChannelIdxDevelopment,
    FirmwareSettingsSceneSettingsCheckChannelIdxRc,
    FirmwareSettingsSceneSettingsCheckChannelIdxRelease,

    FirmwareSettingsSceneSettingsCheckChannelIdxsCount
} FirmwareSettingsSceneSettingsCheckChannelIdx;

static const char* firmware_settings_scene_settings_check_channel_values[] = {
    [FirmwareSettingsSceneSettingsCheckChannelIdxDevelopment] =
        UPDATER_SETTINGS_CHECK_CHANNEL_ID_DEVELOPMENT,
    [FirmwareSettingsSceneSettingsCheckChannelIdxRc] =
        UPDATER_SETTINGS_CHECK_CHANNEL_ID_RELEASE_CANDIDATE,
    [FirmwareSettingsSceneSettingsCheckChannelIdxRelease] =
        UPDATER_SETTINGS_CHECK_CHANNEL_ID_RELEASE,
};

static_assert(
    COUNT_OF(firmware_settings_scene_settings_check_channel_values) ==
    FirmwareSettingsSceneSettingsCheckChannelIdxsCount);

static const char* firmware_settings_scene_settings_check_channel_labels[] = {
    [FirmwareSettingsSceneSettingsCheckChannelIdxDevelopment] = "Dev",
    [FirmwareSettingsSceneSettingsCheckChannelIdxRc] = "RC",
    [FirmwareSettingsSceneSettingsCheckChannelIdxRelease] = "Rel",
};

static_assert(
    COUNT_OF(firmware_settings_scene_settings_check_channel_labels) ==
    FirmwareSettingsSceneSettingsCheckChannelIdxsCount);

static void firmware_settings_scene_settings_autoupdate_callback(VarItem* item, void* context) {
    FirmwareSettings* instance = context;
    FirmwareSettingsSceneSettings* scene =
        scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxSettings);

    furi_mutex_acquire(scene->updater_settings_mutex, FuriWaitForever);
    scene->updater_settings.autoupdate_enabled = var_item_get_value(item);
    furi_mutex_release(scene->updater_settings_mutex);

    firmware_settings_internal_fire_event(
        instance, FirmwareSettingsSceneSettingsEventSettingChange);
}

static void firmware_settings_scene_settings_check_channel_callback(VarItem* item, void* context) {
    FirmwareSettings* instance = context;
    FirmwareSettingsSceneSettings* scene =
        scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxSettings);

    uint32_t item_value = var_item_get_value(item);
    furi_check(item_value < FirmwareSettingsSceneSettingsCheckChannelIdxsCount);

    furi_mutex_acquire(scene->updater_settings_mutex, FuriWaitForever);
    strlcpy(
        scene->updater_settings.check_channel_id,
        firmware_settings_scene_settings_check_channel_values[item_value],
        sizeof(scene->updater_settings.check_channel_id));
    furi_mutex_release(scene->updater_settings_mutex);

    firmware_settings_internal_fire_event(
        instance, FirmwareSettingsSceneSettingsEventSettingChange);
}

static void firmware_settings_scene_settings_on_enter(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsSceneSettings* scene =
        scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxSettings);

    scene->updater_settings_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    updater_get_settings(instance->updater, &scene->updater_settings);

    bool is_debug_flag_set = furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug);

    FirmwareSettingsSceneSettingsCheckChannelIdx channel_idx =
        FirmwareSettingsSceneSettingsCheckChannelIdxDevelopment;

    if(is_debug_flag_set) {
        for(size_t i = 0; i < FirmwareSettingsSceneSettingsCheckChannelIdxsCount; i++) {
            if(strncmp(
                   scene->updater_settings.check_channel_id,
                   firmware_settings_scene_settings_check_channel_values[i],
                   sizeof(scene->updater_settings.check_channel_id)) == 0) {
                channel_idx = i;
                break;
            }
        }
    }

    with_gui(instance->gui, {
        scene->front_list = var_item_list_alloc(instance->front_scene_window);

        VarItem* front_auto_update_item = var_item_list_add_switch(
            scene->front_list,
            "Auto-update",
            firmware_settings_scene_settings_autoupdate_callback,
            instance);
        var_item_set_value(front_auto_update_item, scene->updater_settings.autoupdate_enabled);

        if(is_debug_flag_set) {
            VarItem* front_check_channel_item = var_item_list_add_selector(
                scene->front_list,
                "Channel",
                NULL,
                firmware_settings_scene_settings_check_channel_labels,
                COUNT_OF(firmware_settings_scene_settings_check_channel_labels),
                firmware_settings_scene_settings_check_channel_callback,
                instance);
            var_item_set_value(front_check_channel_item, channel_idx);
        }

        scene->back_list = var_item_list_alloc(instance->back_scene_window);

        VarItem* back_auto_update_item =
            var_item_list_add_switch(scene->back_list, "Auto-update", NULL, NULL);
        var_item_set_value(back_auto_update_item, scene->updater_settings.autoupdate_enabled);

        if(is_debug_flag_set) {
            VarItem* back_check_channel_item = var_item_list_add_selector(
                scene->back_list,
                "Channel",
                NULL,
                firmware_settings_scene_settings_check_channel_labels,
                COUNT_OF(firmware_settings_scene_settings_check_channel_labels),
                NULL,
                NULL);
            var_item_set_value(back_check_channel_item, channel_idx);
        }

        widget_set_scrollbar_enabled(var_item_list_get_base(scene->front_list), true);
        widget_set_scrollbar_enabled(var_item_list_get_base(scene->back_list), true);
    });
}

static void firmware_settings_scene_settings_on_exit(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsSceneSettings* scene =
        scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxSettings);

    furi_mutex_free(scene->updater_settings_mutex);

    with_gui(instance->gui, {
        var_item_list_free(scene->back_list);
        var_item_list_free(scene->front_list);
    });
}

static bool
    firmware_settings_scene_settings_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsSceneSettings* scene =
        scene_manager_get_scene_data(instance->scene_manager, FirmwareSettingsSceneIdxSettings);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case FirmwareSettingsSceneSettingsEventSettingChange:
            furi_mutex_acquire(scene->updater_settings_mutex, FuriWaitForever);
            UpdaterSettings updater_settings = scene->updater_settings;
            furi_mutex_release(scene->updater_settings_mutex);

            updater_set_settings(instance->updater, &updater_settings);
            return true;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        with_gui(instance->gui, { nav_bar_pop_location(instance->back_nav_bar); });
    }

    return false;
}

const Scene firmware_settings_internal_scene_settings = {
    .enter_callback = firmware_settings_scene_settings_on_enter,
    .exit_callback = firmware_settings_scene_settings_on_exit,
    .event_callback = firmware_settings_scene_settings_on_event,
    .data_size = sizeof(FirmwareSettingsSceneSettings),
};
