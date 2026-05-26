#include "updater_i.h"

#ifdef FW_CFG_recovery

UpdaterStatus updater_internal_do_check_for_update(Updater* instance, UpdaterMessage* message) {
    UNUSED(instance);
    UNUSED(message);

    return UpdaterStatusUnknownFailure;
}

UpdaterStatus updater_internal_do_download(Updater* instance, UpdaterMessage* message) {
    UNUSED(instance);
    UNUSED(message);

    return UpdaterStatusUnknownFailure;
}

UpdaterStatus updater_internal_do_verify_bundle_sha(Updater* instance, UpdaterMessage* message) {
    UNUSED(instance);
    UNUSED(message);

    return UpdaterStatusUnknownFailure;
}

UpdaterStatus updater_internal_do_unpack(Updater* instance, UpdaterMessage* message) {
    UNUSED(instance);
    UNUSED(message);

    return UpdaterStatusUnknownFailure;
}

FuriState* updater_get_check_state(Updater* instance) {
    UNUSED(instance);
    return NULL;
}

void updater_get_check_info(Updater* instance, UpdateCheckInfo* info) {
    UNUSED(instance);
    UNUSED(info);
}

UpdaterStatus
    updater_download(Updater* instance, const char* url, const char* path, bool do_wait) {
    UNUSED(instance);
    UNUSED(url);
    UNUSED(path);
    UNUSED(do_wait);

    return UpdaterStatusUnknownFailure;
}

void updater_abort_download(Updater* instance) {
    UNUSED(instance);
}

UpdaterStatus updater_verify_bundle_sha(
    Updater* instance,
    const char* tar_path,
    const char* sha,
    bool do_wait) {
    UNUSED(instance);
    UNUSED(tar_path);
    UNUSED(sha);
    UNUSED(do_wait);

    return UpdaterStatusUnknownFailure;
}

UpdaterStatus updater_unpack(
    Updater* instance,
    const char* tar_path,
    const char* staging_path,
    FuriString* manifest_path,
    bool do_wait) {
    UNUSED(instance);
    UNUSED(tar_path);
    UNUSED(staging_path);
    UNUSED(manifest_path);
    UNUSED(do_wait);

    return UpdaterStatusUnknownFailure;
}

void updater_install_from_url(Updater* instance, const char* url, const char* sha256) {
    UNUSED(instance);
    UNUSED(url);
    UNUSED(sha256);
}

UpdaterStatus updater_check_for_update(Updater* instance) {
    UNUSED(instance);

    return UpdaterStatusUnknownFailure;
}

void updater_pause_autoupdates(Updater* instance) {
    UNUSED(instance);
}

void updater_resume_autoupdates(Updater* instance) {
    UNUSED(instance);
}

void updater_internal_settings_change_build_specific(
    Updater* instance,
    const UpdaterSettings* settings) {
    instance->settings = *settings;
}

void updater_internal_setup_build_specific(Updater* instance) {
    UNUSED(instance);
}

#endif /* FW_CFG_recovery */
