#pragma once
#include <furi.h>

/**
 * @brief Opaque BleServiceFrame type declaration.
 */
typedef struct BleServiceFrame BleServiceFrame;

/**
 * @brief Allocates service frame instance for future use.
 *
 * @param[out] BleServiceFrame* Pointer to service frame instance.
 */
BleServiceFrame* ble_service_frame_alloc(void);

/**
 * @brief Deletes service frame instance and free all internals.
 * Currently service frame lives during the whole application lifetime 
 * without removing, so this method is here for consistency only.
 *
 * @param[in] instance Pointer to service frame instance.
 */
void ble_service_frame_free(BleServiceFrame* instance);

/**
 * @brief Locks instance before usage.
 *
 * @param[in] instance Pointer to service frame instance.
 */
bool ble_service_frame_lock(BleServiceFrame* instance);

/**
 * @brief Fills buffer with zeros, resets free space pointer and unlocks instance.
 * Should be used at the end of processing data stored in frame.
 *
 * @param[in] instance Pointer to service frame instance.
 */
void ble_service_frame_unlock(BleServiceFrame* instance);

/**
 * @brief Shows frame lock state, which means are there any data in buffer.
 *
 * @param[in] instance Pointer to service frame instance.
 * @param[out] true if frame has data and it is locked, otherwise false 
 */
bool ble_service_frame_pending(BleServiceFrame* instance);

/**
 * @brief Get size of data stored in frame instance
 *
 * @param[in] instance Pointer to service frame instance.
 * @param[out] size_t size of data currently stored in buffer
 */
size_t ble_service_frame_get_data_size(BleServiceFrame* instance);

/**
 * @brief Get constant pointer to internal data buffer
 *
 * @param[in] instance Pointer to service frame instance.
 * @param[out] const pointer to internal data buffer
 */
const void* ble_service_frame_get_data_ptr(BleServiceFrame* instance);

/**
 * @brief Appends new data to frame, if internal buffer is smaller it will be resized to fit 
 * new data plus already stored data. Maximum size is bounded with @ref MAX_BLE_INTERCOM_FRAME_SIZE
 *
 * @param[in] instance Pointer to service frame instance.
 * @param[in] data Pointer to new data buffer
 * @param[in] size New data length
 */
void ble_service_frame_append_data(BleServiceFrame* instance, const void* data, size_t size);
