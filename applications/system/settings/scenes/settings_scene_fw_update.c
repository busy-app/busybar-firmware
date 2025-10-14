#include "../settings.h"

#include "../storage_macros.h"
#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <gui/modules/progress_bar.h>

#include <toolbox/fetch/fetch_loader.h>
#include <toolbox/update_fw_tar.h>
#include <applications/services/update_checker/update_checker.h>
#include <toolbox/sha256_calc.h>

#define SETTINGS_FW_FILE_PATH EXT_PATH("update/upload.tar")

typedef enum {
    SceneCustomEventBackPressed = SettingsCustomEventSceneEventsStart,
    SceneCustomEventUpdateStatus,
    SceneCustomEventDownloadStarted,
    SceneCustomEventDownloadDone,
    SceneCustomEventInstallationStarted,
    SceneCustomEventErrorOccurred,
} SceneCustomEvent;

typedef struct {
    FuriString* fw_url;
    FuriString* fw_sha256;
    FuriString* new_fw_version;
    FuriString* fw_current_version;
    bool is_new_version;
} FirmwareUpdateInfo;

typedef struct {
    Label* label_status_back;
    Label* label_status_back_percent;
    Image* image_back;
    ProgressBar* bar_back;
    Label* label_fw_name_download;
    Label* label_fw_current_version;

    Label* label_status_front;
    Label* label_status_front_percent;
    ProgressBar* bar_front;

    uint8_t progress_value;

    FetchLoader* fw_loader;
    FuriString* fw_status;

    FirmwareUpdateInfo fw_info;
    UpdateChecker* update_checker;
    FuriPubSubSubscription* update_checker_subscription;

} SettingsSceneFwUpdate;

static void settings_scene_fw_update_scene_update(SettingsApp* instance) {
    furi_assert(instance);
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);
    with_gui(instance->gui, {
        // Hide progress bar and %% before starting
        const bool is_progressing = data->progress_value != 0;
        widget_set_visible(progress_bar_get_base(data->bar_front), is_progressing);
        widget_set_visible(progress_bar_get_base(data->bar_back), is_progressing);
        widget_set_visible(label_get_base(data->label_status_front_percent), is_progressing);
        widget_set_visible(label_get_base(data->label_status_back_percent), is_progressing);

        progress_bar_set_value(data->bar_front, data->progress_value);
        progress_bar_set_value(data->bar_back, data->progress_value);

        if(is_progressing) {
            label_set_text(data->label_status_back, "Downloading");
            label_set_text(data->label_status_front, "Downloading");
            label_set_text_fmt(data->label_status_front_percent, "%d%%", data->progress_value);
        } else {
            label_set_text(data->label_status_front, "See back screen");
            label_set_text(data->label_status_front_percent, "");
        }
        label_set_text_fmt(data->label_status_back_percent, "%d%%", data->progress_value);
        label_set_text(data->label_fw_name_download, furi_string_get_cstr(data->fw_status));
    });
}

static void settings_scene_fw_update_start_download(SettingsApp* instance) {
    furi_assert(instance);
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);
    if(data->fw_info.is_new_version && furi_string_size(data->fw_info.fw_url)) {
        fetch_loader_run(
            data->fw_loader, furi_string_get_cstr(data->fw_info.fw_url), SETTINGS_FW_FILE_PATH);
    }
}

static bool settings_scene_fw_update_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);

    bool consumed = false;
    SceneCustomEvent custom_event;
    UNUSED(custom_event);
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        case InputKeyOk:
            custom_event = SceneCustomEventDownloadStarted;
            data->progress_value = 0;
            consumed = true;
            break;
        default:
            break;
        }
    }

    if(consumed) {
        settings_send_custom_event(instance, custom_event);
    }
    return consumed;
}

static void settings_scene_fw_status_callback(FetchLoaderStatus status, void* context) {
    furi_assert(context);
    SettingsApp* app_instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_current_scene_data(app_instance->scene_manager);
    furi_assert(data);

    uint8_t download_percent =
        (uint8_t)((status.received_download_size * 100) / status.total_download_size);

    data->progress_value = download_percent;

    char* dimension = "B";
    if(status.total_download_size > 2048) {
        status.received_download_size /= 1024;
        status.total_download_size /= 1024;
        dimension = "kB";
    }

    furi_string_printf(
        data->fw_status,
        "%8.2f kB/s, %zu%s/%zu%s",
        (float)status.speed_bytes_per_sec / 1024.0f,
        status.received_download_size,
        dimension,
        status.total_download_size,
        dimension);

    settings_send_custom_event(app_instance, SceneCustomEventUpdateStatus);
}

static void settings_scene_fw_update_state_callback(FuriString* error, void* context) {
    furi_assert(context);
    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);

    // Show error message
    furi_string_set(data->fw_status, error);
    settings_send_custom_event(instance, SceneCustomEventUpdateStatus);
}

static void settings_scene_fw_update_done_callback(void* context) {
    furi_assert(context);
    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);

    furi_string_set(data->fw_status, "Checking sha256...");

    settings_send_custom_event(instance, SceneCustomEventDownloadDone);
}

static void settings_scene_fw_update_check(const void* message, void* context) {
    UpdateCheckerEvent* status = (UpdateCheckerEvent*)message;
    furi_assert(status);
    furi_assert(context);
    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    data->fw_info.is_new_version = false;

    switch(status->type) {
    case UpdateCheckerEventNoNewVersion:
        furi_string_set(data->fw_status, "No new version");
        break;
    case UpdateCheckerEventNewVersion:
        update_checker_get_new_version(data->update_checker, data->fw_info.new_fw_version);
        update_checker_get_new_firmware_url(data->update_checker, data->fw_info.fw_url);
        update_checker_get_new_firmware_sha256(data->update_checker, data->fw_info.fw_sha256);

        furi_string_set(data->fw_status, "New version, press OK to update");
        data->fw_info.is_new_version = true;

        break;
    case UpdateCheckerEventError:
        furi_string_set(data->fw_status, "Error checking update");
        break;
    case UpdateCheckerEventNoWifiConnection:
        furi_string_set(data->fw_status, "No WiFi connection");
        break;
    default:
        break;
    }
    settings_send_custom_event(instance, SceneCustomEventUpdateStatus);
}

static void settings_scene_fw_update_check_sha256(void* context) {
    furi_assert(context);
    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);
    if(data->fw_info.is_new_version) {
        FuriString* sha256_calc = furi_string_alloc();
        FS_Error file_error = FSE_OK;
        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(storage);
        bool is_ok = false;

        sha256_string_calc_file(file, SETTINGS_FW_FILE_PATH, sha256_calc, &file_error);

        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);

        if((file_error == FSE_OK) &&
           (furi_string_cmp(sha256_calc, data->fw_info.fw_sha256) == 0)) {
            furi_string_set(data->fw_status, "Installing...");
            is_ok = true;
        } else {
            furi_string_set(data->fw_status, "Error: sha256 mismatch");
        }

        furi_string_free(sha256_calc);

        if(is_ok) {
            settings_send_custom_event(instance, SceneCustomEventInstallationStarted);
        }
        settings_send_custom_event(instance, SceneCustomEventErrorOccurred);
    }
}

static void settings_scene_fw_update_install(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    UNUSED(data);
    update_fw_tar_install(SETTINGS_FW_FILE_PATH);
}

static void settings_scene_fw_update_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    // Init data
    data->fw_status = furi_string_alloc_set("Checking for update...");
    data->fw_info.fw_url = furi_string_alloc();
    data->fw_info.fw_sha256 = furi_string_alloc();
    data->fw_info.new_fw_version = furi_string_alloc();
    data->fw_info.fw_current_version = furi_string_alloc();
    data->progress_value = 0;

    data->update_checker = furi_record_open(RECORD_UPDATE_CHECKER);
    update_checker_get_current_version(data->update_checker, data->fw_info.fw_current_version);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, settings_scene_fw_update_input_callback, instance);

        // GuiDisplayIdBack
        Widget* root_back = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        FlexLayout* layout_back = flex_layout_alloc(root_back, FlexLayoutTypeColumn);
        Widget* label_container_back = widget_alloc(flex_layout_get_base(layout_back));
        widget_set_padding(flex_layout_get_base(layout_back), 7, 7, 21, 6);
        flex_layout_set_spacing(layout_back, 7);
        widget_set_height_content(label_container_back);

        data->label_status_back = label_alloc(label_container_back);
        label_set_text(data->label_status_back, "");
        widget_set_padding(label_get_base(data->label_status_back), 0, 38 + 10, 0, 0);
        widget_set_align(label_get_base(data->label_status_back), AlignRightMid);

        data->label_status_back_percent = label_alloc(label_container_back);
        label_set_text(data->label_status_back_percent, "0%");
        widget_set_padding(label_get_base(data->label_status_back_percent), 0, 10, 0, 0);
        widget_set_align(label_get_base(data->label_status_back_percent), AlignRightMid);

        data->image_back = image_alloc(label_container_back);
        image_set_source(data->image_back, SETTINGS_IMG_PATH("download_12x12.bin"));

        data->bar_back = progress_bar_alloc(flex_layout_get_base(layout_back));
        widget_set_height(progress_bar_get_base(data->bar_back), 4);

        data->label_fw_name_download = label_alloc(flex_layout_get_base(layout_back));
        label_set_text_font_size(data->label_fw_name_download, LabelFontSizeSmall);
        label_set_text(data->label_fw_name_download, "Checking for update...");

        data->label_fw_current_version = label_alloc(flex_layout_get_base(layout_back));
        label_set_text_font_size(data->label_fw_current_version, LabelFontSizeSmall);
        label_set_text_fmt(
            data->label_fw_current_version,
            "Current version: %s",
            furi_string_get_cstr(data->fw_info.fw_current_version));

        // GuiDisplayIdFront
        Widget* root_front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        FlexLayout* layout_front = flex_layout_alloc(root_front, FlexLayoutTypeColumn);
        Widget* label_container_front = widget_alloc(flex_layout_get_base(layout_front));
        widget_set_padding(flex_layout_get_base(layout_front), 2, 2, 1, 2);
        flex_layout_set_spacing(layout_front, 2);
        widget_set_height_content(label_container_front);

        data->label_status_front = label_alloc(label_container_front);
        label_set_text(data->label_status_front, "");
        widget_set_align(label_get_base(data->label_status_front), AlignLeftMid);

        data->label_status_front_percent = label_alloc(label_container_front);
        label_set_text(data->label_status_front_percent, "See back screen");
        widget_set_align(label_get_base(data->label_status_front_percent), AlignRightMid);

        data->bar_front = progress_bar_alloc(flex_layout_get_base(layout_front));
        widget_set_height(progress_bar_get_base(data->bar_front), 4);
    });

    settings_scene_fw_update_scene_update(instance);

    // FwLoader
    data->fw_loader = fetch_loader_alloc();
    fetch_loader_set_status_callback(data->fw_loader, settings_scene_fw_status_callback, instance);
    fetch_loader_set_state_callback(
        data->fw_loader, settings_scene_fw_update_state_callback, instance);
    fetch_loader_set_done_callback(
        data->fw_loader, settings_scene_fw_update_done_callback, instance);

    data->update_checker_subscription = furi_pubsub_subscribe(
        update_checker_get_pubsub(data->update_checker), settings_scene_fw_update_check, instance);
    update_checker_check_update(data->update_checker);
}

static void settings_scene_fw_update_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, settings_scene_fw_update_input_callback);

        label_free(data->label_status_back);
        label_free(data->label_status_back_percent);
        image_free(data->image_back);
        progress_bar_free(data->bar_back);
        label_free(data->label_status_front);
        label_free(data->label_status_front_percent);
        label_free(data->label_fw_name_download);
        label_free(data->label_fw_current_version);
        progress_bar_free(data->bar_front);
    });

    furi_pubsub_unsubscribe(
        update_checker_get_pubsub(data->update_checker), data->update_checker_subscription);
    furi_record_close(RECORD_UPDATE_CHECKER);

    // Free fw loader
    if(!fetch_loader_is_processing_done(data->fw_loader)) {
        fetch_loader_forced_done(data->fw_loader);
    }
    fetch_loader_set_status_callback(data->fw_loader, NULL, NULL);
    fetch_loader_set_state_callback(data->fw_loader, NULL, NULL);
    fetch_loader_set_done_callback(data->fw_loader, NULL, NULL);
    fetch_loader_free(data->fw_loader);

    // Free data
    furi_string_free(data->fw_status);
    furi_string_free(data->fw_info.fw_url);
    furi_string_free(data->fw_info.fw_sha256);
    furi_string_free(data->fw_info.new_fw_version);
    furi_string_free(data->fw_info.fw_current_version);
}

static bool settings_scene_fw_update_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    UNUSED(data);
    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneCustomEventBackPressed:
            scene_manager_handle_back_event(instance->scene_manager);
            consumed = true;
            break;
        case SceneCustomEventUpdateStatus:
            settings_scene_fw_update_scene_update(instance);
            consumed = true;
            break;
        case SceneCustomEventDownloadStarted:
            settings_scene_fw_update_start_download(instance);
            consumed = true;
            break;
        case SceneCustomEventDownloadDone:
            settings_scene_fw_update_scene_update(instance);
            settings_scene_fw_update_check_sha256(instance);
            consumed = true;
            break;
        case SceneCustomEventErrorOccurred:
            settings_scene_fw_update_scene_update(instance);
            consumed = true;
            break;
        case SceneCustomEventInstallationStarted:
            settings_scene_fw_update_scene_update(instance);
            settings_scene_fw_update_install(instance);
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(instance);
    }

    return consumed;
}

const Scene settings_scene_fw_update = {
    .enter_callback = settings_scene_fw_update_on_enter,
    .exit_callback = settings_scene_fw_update_on_exit,
    .event_callback = settings_scene_fw_update_on_event,
    .data_size = sizeof(SettingsSceneFwUpdate),
};
