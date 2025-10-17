#include "ble_service_i.h"
#include "target/ble_service_target.h"

#define TAG "BleServiceBase"

bool ble_service_lock(BleServiceObject* instance) {
    if(furi_mutex_acquire(instance->service_lock, 100) != FuriStatusOk) {
        BLE_LOG_W("%s - service lock failed", instance->config->name);
        return false;
    }
    return true;
}

void ble_service_unlock(BleServiceObject* instance) {
    if(furi_mutex_release(instance->service_lock) != FuriStatusOk) {
        BLE_LOG_W("%s - service unlock failed", instance->config->name);
    }
}

static bool ble_service_lock_input_frame(BleServiceObject* instance) {
    if(furi_semaphore_acquire(instance->frame_lock, 100) != FuriStatusOk) {
        BLE_LOG_W("%s - frame lock failed", instance->config->name);
        return false;
    }
    return true;
}

static void ble_service_unlock_input_frame(BleServiceObject* instance) {
    if(furi_semaphore_release(instance->frame_lock) != FuriStatusOk) {
        BLE_LOG_W("%s - frame unlock failed", instance->config->name);
    }
}

static inline void
    ble_service_frame_buf_check_alloc(BleServiceObject* instance, size_t new_frame_size) {
    furi_check(new_frame_size < MAX_BLE_INTERCOM_FRAME_SIZE);
    if(new_frame_size > instance->frame_size) {
        free(instance->frame_buf);
        instance->frame_buf = malloc(new_frame_size);
        instance->frame_size = new_frame_size;
        BLE_LOG_D("%s - buf_size: %d", instance->config->name, new_frame_size);
    }
}

void ble_service_prepare_send_intercom_frame(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleCommand command,
    size_t data_size,
    void* data) {
    size_t frame_size = data_size + sizeof(BleIntercomFrameHeader);
    ble_service_frame_buf_check_alloc(instance, frame_size);

    BleIntercomFrameGeneric* frame = (BleIntercomFrameGeneric*)instance->frame_buf;
    BleIntercomFrameHeader* header = &frame->header;

    header->frame_type = frame_type;
    header->command = command;
    header->service_index = instance->config->index;
    header->data_size = data_size;
    ///TODO: need more checks if there_is_enough memory in buffer
    if(data_size && data) memcpy(frame->data, data, data_size);

    BLE_LOG_D(
        "%s - TX frame t: %d c: %d ds: %d fs: %d",
        instance->config->name,
        header->frame_type,
        header->command,
        header->data_size,
        frame_size);

    size_t tx =
        intercom_tx(instance->intercom, IntercomChannelBle, instance->frame_buf, frame_size, 100);
    furi_assert(tx == frame_size);
}

void ble_service_switch_state(BleServiceObject* instance, BleServiceState new_state) {
    BLE_LOG_D("%s - set state: %d", instance->config->name, new_state);
    instance->state = new_state;

    if(instance->state_change_callback && instance->state_callback_context)
        instance->state_change_callback(instance->state_callback_context);
}

static bool ble_service_process_input_frame(BleServiceObject* instance) {
    BLE_LOG_D("%s - process_input_frame", instance->config->name);

    BleIntercomFrameGeneric* frame = (BleIntercomFrameGeneric*)instance->frame_buf;

    ble_service_target_execute(
        instance,
        frame->header.frame_type,
        frame->header.command,
        frame->header.data_size,
        frame->data);

    ble_service_unlock_input_frame(instance);
    return true;
}

BleServiceObject* ble_service_alloc(
    const BleServiceDescriptor* service_config,
    FuriMessageQueue* message_queue,
    Intercom* intercom,
    BleServiceStateChangeCallback state_callback,
    BleServiceStateChangeCallbackContext* ctx) {
    furi_assert(service_config);
    furi_assert(message_queue);
    furi_assert(intercom);
    furi_assert(state_callback);
    furi_assert(ctx);

    BleServiceObject* instance = malloc(sizeof(BleServiceObject));
    BLE_LOG_D("%s - alloc service", service_config->name);

    instance->state = BleServiceStateReset;
    instance->config = service_config;
    instance->intercom = intercom;
    instance->message_queue = message_queue;
    instance->state_change_callback = state_callback;
    instance->state_callback_context = ctx;
    instance->frame_lock = furi_semaphore_alloc(1, 1);
    instance->service_lock = furi_mutex_alloc(FuriMutexTypeNormal);

    if(service_config->char_count) {
        instance->chars = malloc(sizeof(BleCharacteristicObject*) * service_config->char_count);
        for(size_t i = 0; i < service_config->char_count; i++) {
            const BleCharacteristicDescriptor* config = &service_config->char_descriptors[i];
            BleCharacteristicObject* ble_char = ble_characteristic_alloc(config);
            instance->chars[config->intercom_index] = ble_char;
        }
    }
    instance->frame_size = 0;

    return instance;
}

bool ble_service_process(BleServiceObject* instance, const BleServiceCommand* msg) {
    furi_assert(instance);
    furi_assert(msg);

    BLE_LOG_D("%s - ble_service_process", instance->config->name);
    bool result = false;
    if(ble_service_lock(instance)) {
        if(msg->command == BleCommandServiceProcessFrame) {
            result = ble_service_process_input_frame(instance);
        } else
            result = ble_service_target_execute(
                instance, BleIntercomFrameTypeRequest, msg->command, 0, NULL);

        ble_service_unlock(instance);
    }
    return result;
}

void ble_service_process_mailbox(
    BleServiceObject* instance,
    const BleIntercomFrameGeneric* input_frame) {
    furi_assert(instance);
    furi_assert(input_frame);
    BLE_LOG_D("ble_service_process_mailbox");

    size_t fs = input_frame->header.data_size + sizeof(BleIntercomFrameHeader);

    if(ble_service_lock_input_frame(instance)) {
        ble_service_frame_buf_check_alloc(instance, fs);
        memcpy(instance->frame_buf, input_frame, fs);
        ble_service_enqueue_message(instance, BleCommandServiceProcessFrame, 0);
    }
}

BleServiceState ble_service_get_state(BleServiceObject* instance) {
    furi_assert(instance);
    ///TODO: Think of taking service_lock here
    return instance->state;
}

void ble_service_enqueue_message(BleServiceObject* instance, BleCommand command, uint8_t ch_index) {
    furi_assert(instance);

    BleServiceCommand msg = {
        .command = command, .service_index = instance->config->index, .char_index = ch_index};

    if(furi_message_queue_put(instance->message_queue, &msg, 100) != FuriStatusOk) {
        BLE_LOG_W("%s - unable to enqueue for processing", instance->config->name);
    }
}

void ble_service_enqueue_init(BleServiceObject* instance) {
    furi_assert(instance);
    BLE_LOG_D("%s - enqueue init", instance->config->name);
    if(ble_service_lock(instance)) {
        ble_service_enqueue_message(instance, BleCommandServiceInit, 0);
        ble_service_unlock(instance);
    }
}

void ble_service_enqueue_run(BleServiceObject* instance) {
    furi_assert(instance);
    BLE_LOG_D("%s - enqueue run", instance->config->name);
    ble_service_enqueue_message(instance, BleCommandServiceRun, 0);
}

void ble_service_write_data(
    BleServiceObject* instance,
    uint8_t index,
    const void* data,
    const size_t data_size) {
    furi_assert(instance);
    furi_assert(index < instance->config->char_count);
    furi_assert(data);
    furi_assert(data_size > 0);

    if(ble_service_lock(instance)) {
        BleCharacteristicObject* ch = instance->chars[index];
        ble_characteristic_set_data(ch, data, data_size);
        ble_service_enqueue_run(instance);
        ble_service_unlock(instance);
    }
}

void ble_service_register_update_callback(
    BleServiceObject* instance,
    uint16_t index,
    BleDataUpdatedCallback cb,
    void* ctx) {
    furi_assert(instance);
    furi_assert(index < instance->config->char_count);
    if(ble_service_lock(instance)) {
        BleCharacteristicObject* ch = instance->chars[index];
        ble_characteristic_register_update_callback(ch, cb, ctx);
        ble_service_unlock(instance);
    }
}

void ble_service_register_transmission_done_callback(
    BleServiceObject* instance,
    uint16_t index,
    BleDataTransmitDoneCallback cb,
    void* ctx) {
    furi_assert(instance);
    furi_assert(index < instance->config->char_count);
    if(ble_service_lock(instance)) {
        BleCharacteristicObject* ch = instance->chars[index];
        ble_characteristic_register_tx_done_callback(ch, cb, ctx);
        ble_service_unlock(instance);
    }
}
