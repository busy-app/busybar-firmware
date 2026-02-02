#include "session_config.h"
#include "nwp_session.h"

#include <toolbox/json_helper.h>
#include <toolbox/update_lib/common_vals.h>

#define SESSION_CONFIG_PATH EXT_PATH(SESSION_CONFIG_FILE_NAME)

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
            updater_nwp_session_is_current_version(furi_string_get_cstr(nwp_firmware_path)),
        .do_update_resources = !furi_string_empty(resources_path),
    };
}

bool updater_session_config_load(UpdaterSessionConfig* config) {
    furi_assert(config);

    JsonConfig* json = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(json, SESSION_CONFIG_PATH);

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

    JsonConfigStatus free_status = json_config_free(json);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

bool updater_session_config_save(const UpdaterSessionConfig* config) {
    furi_assert(config);

    JsonConfig* json = json_config_alloc();
    JsonConfigStatus open_status = json_config_open(json, SESSION_CONFIG_PATH);

    if(open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) {
        json_config_write_bool(json, "do_update_u5_firmware", config->do_update_u5_firmware);
        json_config_write_bool(json, "do_update_917_firmware", config->do_update_917_firmware);
        json_config_write_bool(
            json, "do_update_917_radio_stack", config->do_update_917_radio_stack);
        json_config_write_bool(json, "do_update_resources", config->do_update_resources);
    }

    JsonConfigStatus free_status = json_config_free(json);
    return (open_status == JsonConfigStatusOk || open_status == JsonConfigStatusMissing) &&
           free_status == JsonConfigStatusOk;
}

bool updater_session_config_delete(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FS_Error result = storage_common_remove(storage, SESSION_CONFIG_PATH);
    furi_record_close(RECORD_STORAGE);

    return (result == FSE_OK || result == FSE_NOT_EXIST);
}
