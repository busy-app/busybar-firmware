#include "session_config.h"
#include "nwp_session.h"

#include <toolbox/json_helper.h>
#include <toolbox/path.h>

#define SESSION_CONFIG_FILE "session.json"

void updater_session_config_compose(const UpdateManifest* manifest, UpdaterSessionConfig* config) {
    const FuriString* u5_firmware_path =
        updater_manifest_get_path(manifest, UpdateManifestPathDfu);

    const FuriString* sl917_firmware_path =
        updater_manifest_get_path(manifest, UpdateManifestPath917);

    const FuriString* nwp_firmware_path =
        updater_manifest_get_path(manifest, UpdateManifestPath917Radio);

    const FuriString* resources_path =
        updater_manifest_get_path(manifest, UpdateManifestPathResources);

    *config = (UpdaterSessionConfig){
        .do_update_u5_firmware = !furi_string_empty(u5_firmware_path),
        .do_update_917_firmware = !furi_string_empty(sl917_firmware_path),
        .do_update_917_radio_stack =
            !furi_string_empty(nwp_firmware_path) &&
            updater_nwp_session_has_actual_version(furi_string_get_cstr(nwp_firmware_path)),
        .do_update_resources = !furi_string_empty(resources_path),
    };
}

bool updater_session_config_load(const char* dir_path, UpdaterSessionConfig* config) {
    furi_assert(dir_path);
    furi_assert(config);

    FuriString* file_path = furi_string_alloc_set_str(dir_path);
    path_append(file_path, SESSION_CONFIG_FILE);

    JsonConfig* json = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(json, furi_string_get_cstr(file_path));

    if(open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) {
        json_config_read_bool(
            json, "do_update_u5_firmware", &config->do_update_u5_firmware, &(bool){true});
        json_config_read_bool(
            json, "do_update_917_firmware", &config->do_update_917_firmware, &(bool){true});
        json_config_read_bool(
            json, "do_update_917_radio_stack", &config->do_update_917_radio_stack, &(bool){true});
        json_config_read_bool(
            json, "do_update_resources", &config->do_update_resources, &(bool){true});
    } else {
        *config = (UpdaterSessionConfig){
            .do_update_u5_firmware = true,
            .do_update_917_firmware = true,
            .do_update_917_radio_stack = true,
            .do_update_resources = true,
        };
    }

    furi_string_free(file_path);
    JsonConfigStatus free_status = json_config_free(json);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

bool updater_session_config_save(const char* dir_path, const UpdaterSessionConfig* config) {
    furi_assert(dir_path);
    furi_assert(config);

    FuriString* file_path = furi_string_alloc_set_str(dir_path);
    path_append(file_path, SESSION_CONFIG_FILE);

    JsonConfig* json = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(json, furi_string_get_cstr(file_path));

    if(open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) {
        json_config_write_bool(json, "do_update_u5_firmware", config->do_update_u5_firmware);
        json_config_write_bool(json, "do_update_917_firmware", config->do_update_917_firmware);
        json_config_write_bool(
            json, "do_update_917_radio_stack", config->do_update_917_radio_stack);
        json_config_write_bool(json, "do_update_resources", config->do_update_resources);
    }

    furi_string_free(file_path);
    JsonConfigStatus free_status = json_config_free(json);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

bool updater_session_config_delete(const char* dir_path) {
    furi_assert(dir_path);

    FuriString* file_path = furi_string_alloc_set_str(dir_path);
    path_append(file_path, SESSION_CONFIG_FILE);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FS_Error result = storage_common_remove(storage, furi_string_get_cstr(file_path));
    furi_record_close(RECORD_STORAGE);

    furi_string_free(file_path);
    return (result == FSE_OK || result == FSE_NOT_EXIST);
}
