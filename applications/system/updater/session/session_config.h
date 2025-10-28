#pragma once

#include <toolbox/update_lib/update_config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool do_update_u5_firmware;
    bool do_update_917_firmware;
    bool do_update_917_radio_stack;
    bool do_update_resources;
} UpdaterSessionConfig;

void updater_session_config_compose(const UpdateManifest* manifest, UpdaterSessionConfig* config);

bool updater_session_config_load(const char* dir_path, UpdaterSessionConfig* config);

bool updater_session_config_save(const char* dir_path, const UpdaterSessionConfig* config);

bool updater_session_config_delete(const char* dir_path);

#ifdef __cplusplus
}
#endif
