#include "ble_service_frame.h"
#include "../ble_intercom_types.h"
#include "../ble_log.h"

#define TAG "BleSrvFrame"

/**
 * @brief This must be less than @ref INTERCOM_SYNC_CHAR_TIMEOUT_MS.
 */
#define BLE_SERVICE_INPUT_FRAME_LOCK_TIMEOUT (500)

struct BleServiceFrame {
    bool pending;
    FuriSemaphore* lock;
    size_t size;
    uint8_t* buf;
    uint8_t* free_offset;
};

static void ble_service_frame_check_resize(BleServiceFrame* instance, size_t new_data_size) {
    furi_assert(instance);
    const size_t current_data_size = (instance->free_offset - instance->buf);
    const size_t desired_size = current_data_size + new_data_size;
    furi_assert(desired_size < MAX_BLE_INTERCOM_FRAME_SIZE);

    if(desired_size > instance->size) {
        instance->buf = realloc(instance->buf, desired_size);
        instance->free_offset = instance->buf + current_data_size;
        instance->size = desired_size;
    }
}

BleServiceFrame* ble_service_frame_alloc() {
    BleServiceFrame* instance = malloc(sizeof(BleServiceFrame));
    instance->lock = furi_semaphore_alloc(1, 1);
    instance->size = 0;
    return instance;
}

void ble_service_frame_free(BleServiceFrame* instance) {
    furi_assert(instance);
    furi_semaphore_free(instance->lock);
    instance->free_offset = NULL;
    free(instance->buf);
    free(instance);
}

bool ble_service_frame_lock(BleServiceFrame* instance) {
    furi_assert(instance);
    instance->pending = furi_semaphore_acquire(
                            instance->lock, BLE_SERVICE_INPUT_FRAME_LOCK_TIMEOUT) == FuriStatusOk;

    if(!instance->pending) BLE_LOG_W("Failed to lock frame");
    return instance->pending;
}

void ble_service_frame_unlock(BleServiceFrame* instance) {
    furi_assert(instance);
    if(instance->pending) {
        memset(instance->buf, 0, instance->size);
        instance->pending = false;
        instance->free_offset = instance->buf;
        furi_semaphore_release(instance->lock);
    }
}

bool ble_service_frame_pending(BleServiceFrame* instance) {
    furi_assert(instance);
    return instance->pending;
}

size_t ble_service_frame_get_data_size(BleServiceFrame* instance) {
    furi_assert(instance);
    return instance->free_offset - instance->buf;
}

const void* ble_service_frame_get_data_ptr(BleServiceFrame* instance) {
    furi_assert(instance);
    return instance->buf;
}

void ble_service_frame_append_data(BleServiceFrame* instance, const void* data, size_t size) {
    furi_assert(instance);
    furi_assert(data);

    ble_service_frame_check_resize(instance, size);
    memcpy(instance->free_offset, data, size);
    instance->free_offset += size;
}
