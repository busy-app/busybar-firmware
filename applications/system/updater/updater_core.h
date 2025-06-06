#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque updater state struct
typedef struct UpdaterState UpdaterState;

// Allocate and initialize updater state
UpdaterState* updater_state_alloc(const char* update_path);

// Free updater state and resources
void updater_state_free(UpdaterState* state);

// Load update configuration from update.json in the given folder
bool updater_load_configuration(UpdaterState* state);

// Validate loaded configuration (target, CRC, file existence)
bool updater_validate_config(const UpdaterState* state);

// Updater service entry point
int32_t updater_srv(void* arg);

#ifdef __cplusplus
}
#endif
