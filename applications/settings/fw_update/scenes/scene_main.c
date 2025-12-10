#include "../fw_update.h"

#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <gui/modules/progress_bar.h>
#include <applications/system/updater/updater.h>
#include <applications/system/updater/updater_paths.h>
#include <wifi/wifi.h>

#include <toolbox/sha256_calc.h>
#include <settings_helpers/gui_params.h>

typedef enum {
    SceneEventBackPressed = AppEventSceneEventsStart,
    SceneEventUpdateStatus,
    SceneEventDownloadStarted,
    SceneEventDownloadDone,
    SceneEventInstallationStarted,
    SceneEventErrorOccurred,
} SceneEvent;

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

    Updater* updater;
    FuriStateSub* update_state_subscription;
    FuriStateSub* update_check_subscription;
    FuriString* fw_status;

    FirmwareUpdateInfo fw_info;

    bool update_in_progress;

} SettingsSceneFwUpdate;

static void scene_main_scene_update(FwUpdate* instance) {
    furi_assert(instance);
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);
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

static void scene_main_start_download(FwUpdate* instance) {
    furi_assert(instance);
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);
    furi_assert(data);

    do {
        if(!(data->fw_info.is_new_version && furi_string_size(data->fw_info.fw_url))) {
            break;
        }

        UpdaterStatus session_start_status = updater_session_start(data->updater);
        if(session_start_status != UpdaterStatusOk) {
            furi_string_printf(
                data->fw_status,
                "Update failed: %s",
                updater_get_status_string(session_start_status));
            fw_update_send_custom_event(instance, SceneEventUpdateStatus);
            break;
        }

        data->update_in_progress = true;

        updater_download(data->updater, furi_string_get_cstr(data->fw_info.fw_url), NULL, false);
    } while(false);
}

static bool scene_main_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    FwUpdate* instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    bool consumed = false;
    SceneEvent custom_event;
    UNUSED(custom_event);
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        case InputKeyOk:
            custom_event = SceneEventDownloadStarted;
            data->progress_value = 0;
            consumed = true;
            break;
        default:
            break;
        }
    }

    if(consumed) {
        fw_update_send_custom_event(instance, custom_event);
    }
    return consumed;
}

static void updater_update_state_callback(const void* state_item, void* context) {
    furi_assert(context);
    furi_assert(state_item);

    FwUpdate* instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);
    furi_assert(data);

    const UpdaterUpdateState* state = state_item;

    switch(state->event) {
    case UpdaterUpdateEventActionProgress:
        if(state->action == UpdaterUpdateActionDownload) {
            size_t received = state->as_download.received_size;
            size_t total = state->as_download.total_size;

            uint8_t download_percent = 0;
            if(total > 0) {
                download_percent = (uint8_t)((received * 100) / total);
            }

            data->progress_value = download_percent;

            char* dimension = "B";
            if(total > 2048) {
                received /= 1024;
                total /= 1024;
                dimension = "kB";
            }

            furi_string_printf(
                data->fw_status,
                "%8.2f kB/s, %zu%s/%zu%s",
                (float)state->as_download.speed_bytes_per_sec / 1024.0f,
                received,
                dimension,
                total,
                dimension);

            fw_update_send_custom_event(instance, SceneEventUpdateStatus);
        }
        break;

    case UpdaterUpdateEventDetailChange:
        if(state->detail) {
            furi_string_set(data->fw_status, state->detail);
            fw_update_send_custom_event(instance, SceneEventUpdateStatus);
        }
        break;

    case UpdaterUpdateEventActionDone:
        if(state->action == UpdaterUpdateActionDownload) {
            if(state->status == UpdaterStatusOk) {
                furi_string_set(data->fw_status, "Checking sha256...");
                fw_update_send_custom_event(instance, SceneEventDownloadDone);
            } else {
                furi_string_printf(
                    data->fw_status,
                    "Download failed: %s",
                    updater_get_status_string(state->status));
                fw_update_send_custom_event(instance, SceneEventErrorOccurred);
            }
        }
        break;

    default:
        break;
    }
}

static void updater_check_state_callback(const void* state_item, void* context) {
    furi_assert(state_item);
    furi_assert(context);

    const UpdaterCheckState* state = (const UpdaterCheckState*)state_item;

    if(state->event == UpdaterCheckEventStop) {
        FwUpdate* instance = context;

        SettingsSceneFwUpdate* data =
            scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);
        data->fw_info.is_new_version = false;

        switch(state->result) {
        case UpdaterCheckResultNotAvailable:
            furi_string_set(data->fw_status, "No new version");
            break;
        case UpdaterCheckResultAvailable:
            if(state->version) {
                furi_string_set(
                    data->fw_info.new_fw_version, furi_string_get_cstr(state->version));
            }
            if(state->url) {
                furi_string_set(data->fw_info.fw_url, furi_string_get_cstr(state->url));
            }
            if(state->sha256) {
                furi_string_set(data->fw_info.fw_sha256, furi_string_get_cstr(state->sha256));
            }

            furi_string_set(data->fw_status, "New version, press OK to update");
            data->fw_info.is_new_version = true;

            break;
        case UpdaterCheckResultFailure:
            furi_string_set(data->fw_status, "Error checking update");
            break;
        default:
            break;
        }
        fw_update_send_custom_event(instance, SceneEventUpdateStatus);
    }
}

static void scene_main_check_sha256(void* context) {
    furi_assert(context);
    FwUpdate* instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);
    furi_assert(data);
    if(data->fw_info.is_new_version) {
        FuriString* sha256_calc = furi_string_alloc();
        FS_Error file_error = FSE_OK;
        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(storage);
        bool is_ok = false;

        sha256_string_calc_file(file, UPDATER_DEFAULT_DOWNLOAD_PATH, sha256_calc, &file_error);

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
            fw_update_send_custom_event(instance, SceneEventInstallationStarted);
        }
        fw_update_send_custom_event(instance, SceneEventErrorOccurred);
    }
}

static bool is_wifi_connected(void) {
    Wifi* wifi = furi_record_open(RECORD_WIFI);

    WifiInfo wifi_info;
    const WifiStatus status = wifi_get_info(wifi, &wifi_info);

    bool is_wifi_connected = false;
    if(status == WifiStatusOk && wifi_info.state == WifiStateConnected) {
        is_wifi_connected = true;
    }

    furi_record_close(RECORD_WIFI);
    return is_wifi_connected;
}

static void scene_main_install(void* context) {
    furi_assert(context);

    FwUpdate* instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);
    furi_assert(data);

    bool update_started = false;

    do {
        if(!data->update_in_progress) {
            UpdaterStatus session_start_status = updater_session_start(data->updater);
            if(session_start_status != UpdaterStatusOk) {
                break;
            }
            update_started = true;
        }

        UpdaterStatus unpack_status = updater_unpack(data->updater, NULL, NULL, NULL, true);
        if(unpack_status != UpdaterStatusOk) {
            break;
        }

        UpdaterStatus installation_prepare_status =
            updater_installation_prepare(data->updater, NULL, true);
        if(installation_prepare_status != UpdaterStatusOk) {
            break;
        }

        updater_installation_apply(data->updater, false);
    } while(false);

    if(update_started && !data->update_in_progress) {
        updater_session_stop(data->updater);
    }
}

static void scene_main_on_enter(void* context) {
    furi_assert(context);

    FwUpdate* instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    data->updater = furi_record_open(RECORD_UPDATER);

    data->fw_status = furi_string_alloc_set("Checking for update...");
    data->fw_info.fw_url = furi_string_alloc();
    data->fw_info.fw_sha256 = furi_string_alloc();
    data->fw_info.new_fw_version = furi_string_alloc();
    data->fw_info.fw_current_version = furi_string_alloc_set_str(updater_get_active_version());
    data->progress_value = 0;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, scene_main_input_callback, instance);

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
        image_set_source(data->image_back, IMG_PATH("download_12x12.bin"));

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

    scene_main_scene_update(instance);

    data->update_in_progress = false;

    data->update_state_subscription = furi_state_subscribe(
        updater_get_update_state(data->updater), updater_update_state_callback, instance);

    data->update_check_subscription = furi_state_subscribe(
        updater_get_check_state(data->updater), updater_check_state_callback, instance);

    if(is_wifi_connected()) {
        updater_check_for_update(data->updater);
    } else {
        furi_string_set(data->fw_status, "No WiFi connection");
        fw_update_send_custom_event(instance, SceneEventUpdateStatus);
    }
}

static void scene_main_on_exit(void* context) {
    furi_assert(context);

    FwUpdate* instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, scene_main_input_callback);

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

    if(data->update_state_subscription) {
        furi_state_unsubscribe(data->update_state_subscription);
    }

    if(data->update_check_subscription) {
        furi_state_unsubscribe(data->update_check_subscription);
    }

    if(data->update_in_progress) {
        updater_session_stop(data->updater);
    }

    furi_record_close(RECORD_UPDATER);

    // Free data
    furi_string_free(data->fw_status);
    furi_string_free(data->fw_info.fw_url);
    furi_string_free(data->fw_info.fw_sha256);
    furi_string_free(data->fw_info.new_fw_version);
    furi_string_free(data->fw_info.fw_current_version);
}

static bool scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    FwUpdate* instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);
    UNUSED(data);
    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventBackPressed:
            scene_manager_handle_back_event(instance->scene_manager);
            consumed = true;
            break;
        case SceneEventUpdateStatus:
            scene_main_scene_update(instance);
            consumed = true;
            break;
        case SceneEventDownloadStarted:
            scene_main_start_download(instance);
            consumed = true;
            break;
        case SceneEventDownloadDone:
            scene_main_scene_update(instance);
            scene_main_check_sha256(instance);
            consumed = true;
            break;
        case SceneEventErrorOccurred:
            scene_main_scene_update(instance);
            consumed = true;
            break;
        case SceneEventInstallationStarted:
            scene_main_scene_update(instance);
            scene_main_install(instance);
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene fw_update_scene_main = {
    .enter_callback = scene_main_on_enter,
    .exit_callback = scene_main_on_exit,
    .event_callback = scene_main_on_event,
    .data_size = sizeof(SettingsSceneFwUpdate),
};
