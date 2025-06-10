#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SlUpdater SlUpdater;

/**
 * @brief Callback type for reporting firmware update progress.
 * @param percentage The current progress percentage (0-100).
 * @param context User-provided context.
 */
typedef void (*SlUpdaterProgressCallback)(uint8_t percentage, void* context);

// Allocates and initializes a new SlUpdater instance
SlUpdater* sl_updater_alloc(void);

// Runs the updater process. Returns true if the update was successful, false otherwise.
// Can only be called once per instance.
bool sl_updater_run(
    SlUpdater* instance,
    const char* firmware_path,
    bool is_stack_image,
    uint8_t install_timeout_seconds,
    uint8_t baud_throttle);

/**
 * @brief Sets the progress callback for the SlUpdater instance.
 * @param instance Pointer to the SlUpdater instance.
 * @param callback The progress callback function.
 * @param context User-provided context for the callback.
 */
void sl_updater_set_progress_callback(
    SlUpdater* instance,
    SlUpdaterProgressCallback callback,
    void* context);

bool sl_update_probe(SlUpdater* instance, uint8_t baud_throttle, FuriString* version);

// Frees the SlUpdater instance
void sl_updater_free(SlUpdater* instance);

#ifdef __cplusplus
}
#endif
