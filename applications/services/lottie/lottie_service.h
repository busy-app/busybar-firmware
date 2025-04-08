/**
 * @file lottie_service.h
 * @brief A service thread that renders Lottie animations.
 *
 * The Lottie service loads Lottie files and renders them continously in an off-screen buffer.
 * When a frame is fully rendered, a callback is called where the application code must
 * copy the off-screen buffer to its draw bufer in order to display it.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RECORD_LOTTIE "lottie"

#ifdef __cplusplus
extern "C" {
#endif

/** LottieService opaque type declaration. */
typedef struct LottieService LottieService;

/** LottieServiceTask opaque type declaration. */
typedef struct LottieServiceTask LottieServiceTask;

/**
 * @brief Task callback function type.
 *
 * A function of this signature is to be called
 * each time a task completes a single frame.
 *
 * @param[in] canvas_buf pointer to the buffer containing a completed frame
 * @param[in,out] context pointer to a user-specific object, given at task creation
 */
typedef void (*LottieServiceTaskCallback)(const void* canvas_buf, void* context);

/** Lottie task information structure. */
typedef struct {
    int32_t canvas_width; /**< Canvas width, taken from the file */
    int32_t canvas_height; /**< Canvas height, taken from the file */
    size_t canvas_buf_size; /**< Canvas buffer size, in bytes */
} LottieServiceTaskInfo;

/**
 * @brief Create a Lottie task.
 *
 * @param[in,out] instance pointer to a LottieService instance
 * @param[in] callback pointer to a function to be called after a frame has been completed
 * @param[in,out] context pointer to a user-specific object, will be passed to the callback
 */
LottieServiceTask* lottie_service_task_alloc(
    LottieService* instance,
    LottieServiceTaskCallback callback,
    void* context);

/**
 * @brief Delete a Lottie task.
 *
 * If the task is running at the time of deletion, it will be stopped automatically.
 *
 * @param[in,out] task pointer to a LottieServiceTask instance to be deleted
 */
void lottie_service_task_free(LottieServiceTask* task);

/**
 * @brief Load the animation source file.
 *
 * The file MUST be in the Lottie JSON format.
 *
 * @param[in,out] task pointer to a LottieServiceTask instance to be modified
 * @param[in] file_path zero-terminated string containing a full path to the file
 * @returns true if the file was successfully loaded, false otherwise
 */
bool lottie_service_task_set_source(LottieServiceTask* task, const char* file_path);

/**
 * @brief Override a Lottie slot.
 *
 * The source file MUST be loaded using lottie_service_task_set_source() before calling this function.
 *
 * @param[in,out] task pointer to a LottieServiceTask instance to be modified
 * @param[in] slot_str zero-terminated string containing a JSON object with the slot description
 * @returns true if the slot was successfully overridden, false otherwise
 */
bool lottie_service_task_override_slot(LottieServiceTask* task, const char* slot_str);

/**
 * @brief Get the information about a Lottie task.
 *
 * The source file MUST be loaded using lottie_service_task_set_source() before calling this function.
 *
 * @param[in] task pointer to a LottieServiceTask instance to be queried
 * @param[out] info pointer to a LottieServiceTaskInfo structure to contain the task info
 * @returns true if the information could be successfully read, false otherwise
 */
bool lottie_service_task_get_info(const LottieServiceTask* task, LottieServiceTaskInfo* info);

/**
 * @brief Start a Lottie task.
 *
 * The source file MUST be loaded using lottie_service_task_set_source() before calling this function.
 *
 * @param[in,out] task pointer to a LottieServiceTask instance to be started
 * @returns true if the task could be successfully started, false otherwise
 */
bool lottie_service_task_start(LottieServiceTask* task);

#ifdef __cplusplus
}
#endif
