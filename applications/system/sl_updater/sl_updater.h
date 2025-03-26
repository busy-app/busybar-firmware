
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SlUpdater SlUpdater;

// Allocates and initializes a new SlUpdater instance
SlUpdater* sl_updater_alloc(void);

// Runs the updater process. Returns true if the update was successful, false otherwise.
// Can only be called once per instance.
bool sl_updater_run(
    SlUpdater* instance,
    const char* firmware_path,
    bool is_stack_image,
    uint8_t timeout_seconds);

// Frees the SlUpdater instance
void sl_updater_free(SlUpdater* instance);

#ifdef __cplusplus
}
#endif
