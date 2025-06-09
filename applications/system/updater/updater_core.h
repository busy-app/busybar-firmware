#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <toolbox/update_lib/update_manifest.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque updater state struct
typedef struct UpdaterState UpdaterState;

// Allocate and initialize updater state
UpdaterState* updater_state_alloc(void);

// Free updater state and resources
void updater_state_free(UpdaterState* state);

// Load update configuration from update.json in the given folder
bool updater_state_init_config(UpdaterState* state, const char* update_manifest_path);

// Validate loaded configuration (target, CRC, file existence)
bool updater_state_validate_config(const UpdaterState* state);

const UpdateManifest* updater_state_get_config(const UpdaterState* state);

#ifdef __cplusplus
}
#endif
