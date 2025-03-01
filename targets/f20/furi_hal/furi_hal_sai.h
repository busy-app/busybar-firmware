/**
 * @file furi_hal_sai.h
 *
 * @brief HAL APIs for Serial Audio Interface (SAI).
 *
 * Data format expected by SAI:
 * - Rate: 44100 Hz
 * - Bits: 16bit LE
 *
 * Once started, it will output the data in a circular manner, providing
 * two events: Half Transfer and Transfer complete to indicate which
 * portion of the buffer can be updated by the application code.
 *
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Enumeration of SAI events. */
typedef enum {
    FuriHalSaiEventHalfTransfer, /**< Half of the buffer has been transferred */
    FuriHalSaiEventTransferComplete, /**< Complete buffer has been transferred */
} FuriHalSaiEvent;

/**
 * @brief SAI event callback type.
 *
 * @param[in] event one of FuriHalSaiEvent values to indicate the occurred event
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*FuriHalSaiCallback)(FuriHalSaiEvent event, void* context);

/**
 * @brief Initialise the SAI peripheral.
 *
 * The application code doesn't need to call this function, as it is done in furi_hal_init().
 *
 * @returns true if the initialisation was successful, false otherwise
 */
bool furi_hal_sai_init(void);

/**
 * @brief Set the circular buffer pointer and depth.
 *
 * The application code must fill the buffer with sample data in a timely manner in
 * accordance to the events produced by the SAI.
 *
 * @param[in] buffer pointer to the sample buffer
 * @param[in] buffer_depth buffer depth in samples (NOT bytes)
 */
void furi_hal_sai_set_buffer(const int16_t* buffer, uint32_t buffer_depth);

/**
 * @brief Set the callback function to be called upon SAI events.
 *
 * @param[in] callback
 * @param[in,out] context pointer to a user-specific object (will be passed as context param in callback)
 */
void furi_hal_sai_set_callback(FuriHalSaiCallback callback, void* context);

/**
 * @brief Start transferring sample data via SAI.
 *
 * @warning Before calling this function, a buffer MUST be set using furi_hal_set_buffer() and,
 * in most cases, the event callback must be registered by calling furi_hal_sai_set_callback().
 */
void furi_hal_sai_start(void);

/**
 * @brief Stop transferring sample data via SAI.
 *
 * Data transfer stops immediately after calling this function.
 */
void furi_hal_sai_stop(void);

#ifdef __cplusplus
}
#endif
