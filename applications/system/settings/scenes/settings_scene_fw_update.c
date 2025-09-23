#include "../settings.h"

#include "../storage_macros.h"
#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <gui/modules/progress_bar.h>

#include <toolbox/fetch/fetch_loader.h>
#include <toolbox/update_fw_tar.h>

#define SETTINGS_FW_FILE_PATH EXT_PATH("update/upload.tar")

typedef enum {
    SceneCustomEventVolumeChanged = SettingsCustomEventSceneEventsStart,
    SceneCustomEventBackPressed,
    SceneCustomEventUpdateStatus,
    SceneCustomEventDownloadStarted,
    SceneCustomEventDownloadDone,
    SceneCustomEventErrorOccurred,
} SceneCustomEvent;

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

    uint8_t bar_volume;

    FetchLoader* fw_loader;
    FuriString* fw_status;

} SettingsSceneFwUpdate;

static void settings_scene_fw_update_scene_update(SettingsApp* instance) {
    furi_assert(instance);
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);
    with_gui(instance->gui, {
        progress_bar_set_value(data->bar_front, data->bar_volume);
        progress_bar_set_value(data->bar_back, data->bar_volume);
        label_set_text_fmt(data->label_status_front_percent, "%d%%", data->bar_volume);
        label_set_text_fmt(data->label_status_back_percent, "%d%%", data->bar_volume);
        label_set_text(data->label_fw_name_download, furi_string_get_cstr(data->fw_status));
    });
}

static void settings_scene_fw_update_start_download(SettingsApp* instance) {
    furi_assert(instance);
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    furi_assert(data);
    fetch_loader_run(
        data->fw_loader,
        "https://update.flipperzero.one/builds/busybar-firmware/dev/busybar-f21-update-dev-23092025-cb76191e.tar",
        SETTINGS_FW_FILE_PATH);
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
        /* fall-through */
        case InputKeyOk:
            custom_event = SceneCustomEventDownloadStarted;
            data->bar_volume = 0;
            consumed = true;
            break;
        case InputKeyUp:
            data->bar_volume++;
            if(data->bar_volume > 100) {
                data->bar_volume = 100;
            }
            custom_event = SceneCustomEventUpdateStatus;
            consumed = true;
            break;
        case InputKeyDown:
            if(data->bar_volume > 0) {
                data->bar_volume--;
            }
            custom_event = SceneCustomEventUpdateStatus;
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

    data->bar_volume = download_percent;

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
    SettingsApp* app_instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_current_scene_data(app_instance->scene_manager);
    furi_assert(data);

    // Show error message
    furi_string_set(data->fw_status, error);
    settings_send_custom_event(app_instance, SceneCustomEventUpdateStatus);
}

static void settings_scene_fw_update_done_callback(void* context) {
    furi_assert(context);
    SettingsApp* app_instance = context;
    SettingsSceneFwUpdate* data =
        scene_manager_get_current_scene_data(app_instance->scene_manager);
    furi_assert(data);

    settings_send_custom_event(app_instance, SceneCustomEventDownloadDone);
}

static void settings_scene_fw_update_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneFwUpdate* data = scene_manager_get_current_scene_data(instance->scene_manager);
    data->fw_status = furi_string_alloc();
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
        label_set_text(data->label_status_back, "Downloading");
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
        label_set_text(data->label_fw_name_download, "Press OK to update");

        data->label_fw_current_version = label_alloc(flex_layout_get_base(layout_back));
        label_set_text_font_size(data->label_fw_current_version, LabelFontSizeSmall);
        label_set_text(data->label_fw_current_version, "Current version: 0.0.2");

        // GuiDisplayIdFront
        Widget* root_front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        FlexLayout* layout_front = flex_layout_alloc(root_front, FlexLayoutTypeColumn);
        Widget* label_container_front = widget_alloc(flex_layout_get_base(layout_front));
        widget_set_padding(flex_layout_get_base(layout_front), 2, 2, 1, 2);
        flex_layout_set_spacing(layout_front, 2);
        widget_set_height_content(label_container_front);

        data->label_status_front = label_alloc(label_container_front);
        label_set_text(data->label_status_front, "Downloading");
        widget_set_align(label_get_base(data->label_status_front), AlignLeftMid);

        data->label_status_front_percent = label_alloc(label_container_front);
        label_set_text(data->label_status_front_percent, "0%");
        widget_set_align(label_get_base(data->label_status_front_percent), AlignRightMid);

        data->bar_front = progress_bar_alloc(flex_layout_get_base(layout_front));
        widget_set_height(progress_bar_get_base(data->bar_front), 4);

        // FwLoader
        data->fw_loader = fetch_loader_alloc();
        fetch_loader_set_status_callback(
            data->fw_loader, settings_scene_fw_status_callback, instance);
        fetch_loader_set_state_callback(
            data->fw_loader, settings_scene_fw_update_state_callback, instance);
        fetch_loader_set_done_callback(
            data->fw_loader, settings_scene_fw_update_done_callback, instance);
    });
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
        fetch_loader_free(data->fw_loader);
    });
    furi_string_free(data->fw_status);
}

static bool settings_scene_fw_update_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneCustomEventVolumeChanged: {
            SettingsSceneFwUpdate* data =
                scene_manager_get_current_scene_data(instance->scene_manager);
            UNUSED(data);
            //settings_volume_set(instance, data->volume);
            audio_play_file(instance->audio, SETTINGS_SOUND_PATH("volume_change.snd"));

            consumed = true;
            break;
        }

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
        case SceneCustomEventDownloadDone: {
            // TODO: add scene to show "Success" or "Error"
            update_fw_tar(SETTINGS_FW_FILE_PATH);
            consumed = true;
            break;
        }

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
