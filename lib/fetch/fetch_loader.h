/**
 * @file fetch_loader.h
 * @brief Simple file downloader library.
 */
#pragma once

#include "fetch_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque FetchLoader structure declaration.
 */
typedef struct FetchLoader FetchLoader;

/**
 * @brief Progress callback function type.
 *
 * @param[in] progress pointer to the structure containing download progress information
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*FetchLoaderProgressCallback)(const FetchProgress* progress, void* context);

/**
 * @brief State callback function type.
 *
 * @param[in] message pointer to a zero-terminated string with a message about current state
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*FetchLoaderStateCallback)(const char* message, void* context);

/**
 * @brief Done callback function type.
 *
 * @param[in] status value from @ref FetchStatus enum representing the completion status
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*FetchLoaderDoneCallback)(FetchStatus status, void* context);

/**
 * @brief Create a FetchLoader instance.
 *
 * @returns pointer to the created instance
 */
FetchLoader* fetch_loader_alloc(void);

/**
 * @brief Delete a FetchLoader instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted
 */
void fetch_loader_free(FetchLoader* instance);

/**
 * @brief Start downloading a file.
 *
 * This function spawns a separate thread and does not block.
 *
 * @param[in,out] instance pointer to the instance to be started
 * @param[in] remote_url zero-terminated string with a full URL of the file to download
 * @param[in] file_path zero-terminated string with a full path to the output file
 */
void fetch_loader_start(FetchLoader* instance, const char* remote_url, const char* file_path);

/**
 * @brief Request the download process to be stopped.
 *
 * This function does not block. Callers must wait
 * for the done callback to be triggered before
 * attempting to restart or delete the instance.
 *
 * @param[in,out] instance pointer to the instance to be stopped
 */
void fetch_loader_stop(FetchLoader* instance);

/**
 * @brief Set the context for callback functions.
 *
 * @param[in,out] instance pointer to the instance to be modified
 * @param[in] context pointer to the user-specific object (will be passed to the callbacks)
 */
void fetch_loader_set_callback_context(FetchLoader* instance, void* context);

/**
 * @brief Set the callback function for progress notifications.
 *
 * @param[in,out] instance pointer to the instance to be modified
 * @param[in] callback pointer to the function to be called on progress changes
 */
void fetch_loader_set_progress_callback(
    FetchLoader* instance,
    FetchLoaderProgressCallback callback);

/**
 * @brief Set the callback function for state notifications.
 *
 * @param[in,out] instance pointer to the instance to be modified
 * @param[in] callback pointer to the function to be called on new state messages
 */
void fetch_loader_set_state_callback(FetchLoader* instance, FetchLoaderStateCallback callback);

/**
 * @brief Set the callback function for completion notifications.
 *
 * @param[in,out] instance pointer to the instance to be modified
 * @param[in] callback pointer to the function to be called upon completion
 */
void fetch_loader_set_done_callback(FetchLoader* instance, FetchLoaderDoneCallback callback);

#ifdef __cplusplus
}
#endif
