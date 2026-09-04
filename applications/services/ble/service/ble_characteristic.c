
#include "ble_characteristic.h"
#include <furi.h>

#define TAG "BleChar"

#define BLE_CHAR_LOCK_TIMEOUT_MS (250)

typedef enum {
    BleCharacteristicStateInit,
    BleCharacteristicStateIdle,
    BleCharacteristicStateModifiedLocal,
    BleCharacteristicStateModifiedRemote,
    BleCharacteristicStateWaitResponse,
} BleCharacteristicState;

struct BleCharacteristicObject {
    FuriSemaphore* lock;
    BleCharacteristicState state;
    uint32_t sequence_num;

    uint8_t max_data_size;
    uint8_t data_size;
    uint8_t cccd_value;
    uint16_t cccd_handle;
    uint16_t handle;
    void* data;

    BleDataUpdatedCallback update_cb;
    void* update_ctx;

    BleDataTransmitDoneCallback tx_done_cb;
    void* tx_done_ctx;

    const BleCharacteristicConfig* config;
    BleServiceObject* service;
};

BleCharacteristicObject* ble_characteristic_alloc(
    const BleCharacteristicConfig* config,
    BleServiceObject* parent_service) {
    furi_assert(config);
    furi_assert(parent_service);

    BleCharacteristicObject* instance = malloc(sizeof(BleCharacteristicObject));

    instance->lock = furi_semaphore_alloc(1, 1);
    instance->service = parent_service;

    instance->config = config;
    if(config->initial_data_size > 0) {
        instance->data = malloc(config->initial_data_size);
        instance->max_data_size = config->initial_data_size;
        instance->data_size = config->initial_data_size;
    }
    return instance;
}

void ble_characteristic_free(BleCharacteristicObject* instance) {
    furi_assert(instance);
    if(instance->data) free(instance->data);
    furi_semaphore_free(instance->lock);
    free(instance);
}

void ble_characteristic_reset(BleCharacteristicObject* instance) {
    BLE_LOG_D("%s - ble_characteristic_reset", instance->config->name);
    if(instance->state == BleCharacteristicStateWaitResponse ||
       instance->state == BleCharacteristicStateModifiedRemote) {
        furi_semaphore_release(instance->lock);
    }
    instance->sequence_num = 0;
}

const void* ble_characteristic_get_data(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->data;
}

size_t ble_characteristic_get_data_size(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->data_size;
}

static inline void ble_characteristic_initial_size_check_alloc(
    BleCharacteristicObject* instance,
    const size_t data_size) {
    if(instance->data == NULL && instance->config->initial_data_size == 0) {
        instance->data = malloc(data_size);
        instance->max_data_size = data_size;
    }
}

static bool ble_characteristic_set_data_common(
    BleCharacteristicObject* instance,
    const void* data,
    const size_t data_size) {
    furi_assert(instance);
    furi_assert(data);
    furi_assert(data_size > 0);

    bool result = false;
    do {
        if(furi_semaphore_acquire(instance->lock, BLE_CHAR_LOCK_TIMEOUT_MS) != FuriStatusOk) {
            BLE_LOG_W("%s - Lock failed!", instance->config->name);
            break;
        }

        ble_characteristic_initial_size_check_alloc(instance, data_size);

        if(instance->max_data_size < data_size) {
            BLE_LOG_W("%s - Unable to set data, wrong size!", instance->config->name);
            break;
        }

        memcpy(instance->data, data, data_size);
        instance->data_size = data_size;
        result = true;
    } while(false);

    return result;
}

void ble_characteristic_set_data(
    BleCharacteristicObject* instance,
    const void* data,
    const size_t data_size) {
    if(ble_characteristic_set_data_common(instance, data, data_size)) {
        instance->state = BleCharacteristicStateModifiedLocal;
    } else {
        BLE_LOG_W("%s - local set data failed!", instance->config->name);
    }
}

static void ble_characteristic_set_data_from_remote(
    BleCharacteristicObject* instance,
    const void* data,
    const size_t data_size) {
    furi_assert(instance);
    furi_assert(data);
    furi_assert(data_size > 0);

    if(ble_characteristic_set_data_common(instance, data, data_size)) {
        instance->state = BleCharacteristicStateModifiedRemote;
        if(instance->update_cb) {
            instance->update_cb(data_size, instance->data, instance->update_ctx);
        }
    } else {
        BLE_LOG_W("%s - remote set data failed!", instance->config->name);
    }
}

static void ble_characteristic_tx_done(BleCharacteristicObject* instance) {
    furi_assert(instance);
    if(instance->tx_done_cb) {
        instance->tx_done_cb(instance->tx_done_ctx);
    }
}

bool ble_characteristic_is_modified(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->state == BleCharacteristicStateModifiedLocal ||
           instance->state == BleCharacteristicStateModifiedRemote;
}

const BleCharacteristicConfig* ble_characteristic_get_config(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->config;
}

void ble_characteristic_set_handle(BleCharacteristicObject* instance, uint16_t handle) {
    furi_assert(instance);
    furi_assert(instance->handle == 0);
    instance->handle = handle;
}

uint16_t ble_characteristic_get_handle(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->handle;
}

void ble_characteristic_set_cccd_handle(BleCharacteristicObject* instance, uint16_t cccd_handle) {
    furi_assert(instance);
    furi_assert(instance->cccd_handle == 0);
    instance->cccd_handle = cccd_handle;
}

bool ble_characteristic_is_cccd_handle(BleCharacteristicObject* instance, uint16_t possible_cccd) {
    furi_assert(instance);
    return instance->cccd_handle == possible_cccd;
}

void ble_characteristic_set_cccd_value(BleCharacteristicObject* instance, uint8_t value) {
    furi_assert(instance);
    instance->cccd_value = value;
}

uint8_t ble_characteristic_get_cccd_value(BleCharacteristicObject* instance) {
    furi_assert(instance);
    return instance->cccd_value;
}

static BleIntercomFrameType
    ble_characteristic_encode_get_frame_type_by_state(const BleCharacteristicState state) {
    BleIntercomFrameType frame_type = BleIntercomFrameTypeUnknown;

#if !defined(BSB_MCU_SI917)
    switch(state) {
    case BleCharacteristicStateInit:
    case BleCharacteristicStateModifiedLocal:
        frame_type = BleIntercomFrameTypeRequest;
        break;
    case BleCharacteristicStateModifiedRemote:
        frame_type = BleIntercomFrameTypeResponse;
        break;
    default:
        furi_crash("Wrong encode state");
    }
#else
    switch(state) {
    case BleCharacteristicStateModifiedLocal:
        frame_type = BleIntercomFrameTypeRequest;
        break;
    case BleCharacteristicStateInit:
    case BleCharacteristicStateModifiedRemote:
        frame_type = BleIntercomFrameTypeResponse;
        break;
    default:
        furi_crash("Wrong encode state");
    }
#endif
    return frame_type;
}

size_t
    ble_characteristic_encode(BleCharacteristicObject* instance, BleCharacteristicData* output) {
    furi_assert(instance);
    furi_assert(output);
    BleIntercomFrameType frame_type =
        ble_characteristic_encode_get_frame_type_by_state(instance->state);
    if(frame_type == BleIntercomFrameTypeRequest) {
        BLE_LOG_D("%s - char_encode_request", instance->config->name);

        output->header.data_size = instance->data_size;
        output->header.frame_type = frame_type;
        output->header.index = instance->config->intercom_index;
        output->header.seq_num = instance->sequence_num;

        memcpy(output->data, instance->data, instance->data_size);
        instance->state = BleCharacteristicStateWaitResponse;
    } else if(frame_type == BleIntercomFrameTypeResponse) {
        BLE_LOG_D("%s - char_encode_response", instance->config->name);

        output->header.data_size = 0;
        output->header.frame_type = frame_type;
        output->header.index = instance->config->intercom_index;
        output->header.seq_num = instance->sequence_num;

        instance->state = BleCharacteristicStateIdle;
        instance->sequence_num += 1;
        furi_semaphore_release(instance->lock);
    }
    return (output->header.data_size + sizeof(BleCharacteristicDataHeader));
}

static bool ble_characteristic_decode_validate(
    const BleCharacteristicState state,
    BleIntercomFrameType frame_type) {
#if !defined(BSB_MCU_SI917)
    return (
        (state == BleCharacteristicStateWaitResponse &&
         frame_type == BleIntercomFrameTypeResponse) ||
        (state == BleCharacteristicStateIdle && frame_type == BleIntercomFrameTypeRequest));
#else
    return (frame_type == BleIntercomFrameTypeRequest &&
            (state == BleCharacteristicStateInit || state == BleCharacteristicStateIdle)) ||
           (frame_type == BleIntercomFrameTypeResponse &&
            state == BleCharacteristicStateWaitResponse);
#endif
}

bool ble_characteristic_decode(
    BleCharacteristicObject* instance,
    const BleCharacteristicData* input) {
    furi_assert(instance);
    furi_assert(input);
    BLE_LOG_D("%s - ble_characteristic_decode", instance->config->name);

    if(!ble_characteristic_decode_validate(instance->state, input->header.frame_type)) {
        BLE_LOG_W("%s - DECODE_ERROR!", instance->config->name);
        return false;
    }

    if(input->header.frame_type == BleIntercomFrameTypeResponse) {
        instance->state = BleCharacteristicStateIdle;
        instance->sequence_num += 1;
        ble_characteristic_tx_done(instance);
        furi_semaphore_release(instance->lock);
    }

    else if(input->header.frame_type == BleIntercomFrameTypeRequest) {
        if(input->header.seq_num != instance->sequence_num)
            BLE_LOG_W(
                "%s - sequence mismatch %ld != %ld",
                instance->config->name,
                input->header.seq_num,
                instance->sequence_num);

        ble_characteristic_set_data_from_remote(instance, input->data, input->header.data_size);
    }
    return true;
}

void ble_characteristic_register_update_callback(
    BleCharacteristicObject* instance,
    BleDataUpdatedCallback callback,
    void* ctx) {
    furi_assert(instance);

    if(callback) {
        if(instance->update_cb == NULL) {
            instance->update_cb = callback;
            instance->update_ctx = ctx;
        } else {
            BLE_LOG_D("%s - update callback already set", instance->config->name);
        }
    } else {
        BLE_LOG_D("Reset update callback");
        instance->update_cb = NULL;
        instance->update_ctx = NULL;
    }
}

void ble_characteristic_register_tx_done_callback(
    BleCharacteristicObject* instance,
    BleDataTransmitDoneCallback callback,
    void* ctx) {
    furi_assert(instance);

    if(callback) {
        instance->tx_done_cb = callback;
        instance->tx_done_ctx = ctx;
    } else {
        BLE_LOG_D("Reset tx_done callback");
        instance->tx_done_cb = NULL;
        instance->tx_done_ctx = NULL;
    }
}
