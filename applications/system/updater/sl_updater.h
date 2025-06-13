#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SlUpdater SlUpdater;

/**
 * @brief Enum defining the phases of the SlUpdater progress.
 */
typedef enum {
    SL_UPDATER_PROGRESS_PHASE_UPLOADING, /**< Firmware image is being uploaded */
    SL_UPDATER_PROGRESS_PHASE_AWAITING_INSTALL, /**< Upload complete, awaiting installation */
} SlUpdaterProgressPhase;

/**
 * @brief Callback type for reporting firmware update progress.
 * @param phase The current phase of the update process.
 * @param percentage The current progress percentage (0-100) for the current phase.
 * @param context User-provided context.
 */
typedef void (
    *SlUpdaterProgressCallback)(SlUpdaterProgressPhase phase, uint8_t percentage, void* context);

/**
 * @brief Allocates and initializes a new SlUpdater instance.
 * @return Pointer to the newly allocated SlUpdater instance.
 */
SlUpdater* sl_updater_alloc(void);

/**
 * @brief Runs the updater process.
 * @param instance Pointer to the SlUpdater instance.
 * @param firmware_path Path to the firmware image.
 * @param is_stack_image True if the image is a stack image, false otherwise.
 * @param install_timeout_seconds Timeout in seconds for the installation phase.
 * @param baud_throttle Baud rate throttle value.
 * @return True if the update was successful, false otherwise.
 * @note Can only be called once per instance.
 */
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

/**
 * @brief Probes the device for version information.
 * @param instance Pointer to the SlUpdater instance.
 * @param baud_throttle Baud rate throttle value.
 * @param version Pointer to an FuriString to store the version information.
 * @return True if probing was successful, false otherwise.
 */
bool sl_update_probe(SlUpdater* instance, uint8_t baud_throttle, FuriString* version);

/**
 * @brief Frees the SlUpdater instance.
 * @param instance Pointer to the SlUpdater instance to be freed.
 */
void sl_updater_free(SlUpdater* instance);

#ifdef __cplusplus
}
#endif
