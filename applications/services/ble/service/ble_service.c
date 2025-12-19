#include "ble_service_i.h"
#include "target/ble_service_target.h"

#define TAG "BleServiceBase"

bool ble_service_lock(BleServiceObject* instance) {
    if(furi_mutex_acquire(instance->service_lock, 2000) != FuriStatusOk) {
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
    if(furi_semaphore_acquire(instance->frame_lock, 2000) != FuriStatusOk) {
        BLE_LOG_W("%s - frame lock failed", instance->config->name);
        return false;
    }
    instance->frame_pending = true;
    return true;
}

static void ble_service_unlock_input_frame(BleServiceObject* instance) {
    instance->frame_pending = false;
    if(furi_semaphore_release(instance->frame_lock) != FuriStatusOk) {
        BLE_LOG_W("%s - frame unlock failed", instance->config->name);
    }
}

static inline void
    ble_service_frame_buf_check_alloc(BleServiceObject* instance, size_t new_frame_size) {
    furi_check(new_frame_size < MAX_BLE_INTERCOM_FRAME_SIZE);
    if(new_frame_size > instance->buffer_size) {
        instance->frame_buf = realloc(instance->frame_buf, new_frame_size);
        instance->buffer_size = new_frame_size;
        BLE_LOG_D("%s - buf_size: %d", instance->config->name, new_frame_size);
    }
}

static inline void ble_service_prepare_intercom_frame_header(
    BleIntercomFrameHeader* const header,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    bool result,
    uint16_t service_index,
    size_t data_size) {
    header->source = BleIntercomFrameSourceService;
    header->frame_type = frame_type;
    header->command = command;
    header->service_index = service_index;
    header->data_size = data_size;
    header->result = result;
}

void ble_service_prepare_send_intercom_frame(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    bool result,
    size_t data_size,
    const void* data) {
    size_t frame_size = data_size + sizeof(BleIntercomFrameHeader);
    ble_service_frame_buf_check_alloc(instance, frame_size);

    BleIntercomFrameGeneric* frame = (BleIntercomFrameGeneric*)instance->frame_buf;

    ble_service_prepare_intercom_frame_header(
        &frame->header, frame_type, command, result, instance->config->index, data_size);

    if(data_size && data) memcpy(frame->data, data, data_size);

    BLE_LOG_D(
        "%s - TX frame t: %d c: %d ds: %d fs: %d",
        instance->config->name,
        header->frame_type,
        header->command,
        header->data_size,
        frame_size);

    size_t tx =
        intercom_tx(instance->intercom_ch, instance->frame_buf, frame_size, FuriWaitForever);
    furi_assert(tx == frame_size);
}

bool ble_service_is_ready(BleServiceObject* instance) {
    furi_assert(instance);
    return instance->ready;
}

const char* ble_service_get_name(BleServiceObject* instance) {
    furi_assert(instance);
    return instance->config->name;
}

void ble_service_set_error(BleServiceObject* instance, const char* format, ...) {
    furi_assert(instance);
    furi_assert(format);

    va_list args;
    va_start(args, format);
    furi_string_vprintf(instance->error, format, args);
    va_end(args);

    instance->ready = false;
}

void ble_service_get_error(BleServiceObject* instance, FuriString* error) {
    furi_assert(instance);
    furi_assert(error);
    furi_string_set(error, instance->error);
}

static bool ble_service_process_input_frame(BleServiceObject* instance) {
    BLE_LOG_D("%s - process_input_frame", instance->config->name);

    BleIntercomFrameGeneric* frame = (BleIntercomFrameGeneric*)instance->frame_buf;

    const BleIntercomFrameHeader* hdr = &frame->header;

    bool result = false;
    if(hdr->result) {
        result = ble_service_target_execute(
            instance, hdr->frame_type, hdr->command, hdr->data_size, frame->data);
    } else {
        ble_service_set_error(
            instance, "Error, frame_type: %d, cmd: %d,", hdr->frame_type, hdr->command);
    }
    ble_service_unlock_input_frame(instance);
    return result;
}

BleServiceObject* ble_service_alloc(
    const BleServiceDescriptor* service_config,
    FuriMessageQueue* message_queue,
    IntercomChannel* intercom_ch) {
    furi_assert(service_config);
    furi_assert(message_queue);
    furi_assert(intercom_ch);

    BleServiceObject* instance = malloc(sizeof(BleServiceObject));
    BLE_LOG_D("%s - alloc service", service_config->name);

    instance->ready = false;
    instance->config = service_config;
    instance->intercom_ch = intercom_ch;
    instance->message_queue = message_queue;
    instance->error = furi_string_alloc();
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
    instance->buffer_size = 0;

    return instance;
}

bool ble_service_process(BleServiceObject* instance) {
    furi_assert(instance);

    BLE_LOG_D("%s - ble_service_process", instance->config->name);
    bool result = false;
    if(ble_service_lock(instance)) {
        if(instance->frame_pending) {
            result = ble_service_process_input_frame(instance);
        } else
            result = ble_service_target_execute(
                instance, BleIntercomFrameTypeRequest, BleServiceCommandRun, 0, NULL);
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
        ble_service_enqueue_message(instance);
    }
}

void ble_service_enqueue_message(BleServiceObject* instance) {
    furi_assert(instance);

    uint32_t value = (uint32_t)instance;
    if(furi_message_queue_put(instance->message_queue, &value, 100) != FuriStatusOk) {
        BLE_LOG_W("%s - unable to enqueue for processing", instance->config->name);
    }
}

void ble_service_enqueue_init(BleServiceObject* instance) {
    furi_assert(instance);
    if(ble_service_lock(instance) && ble_service_lock_input_frame(instance)) {
        ble_service_frame_buf_check_alloc(instance, sizeof(BleIntercomFrameHeader));
        BleIntercomFrameGeneric* frame = (BleIntercomFrameGeneric*)instance->frame_buf;

        frame->header.source = BleIntercomFrameSourceService;
        frame->header.frame_type = BleIntercomFrameTypeRequest;
        frame->header.command = BleServiceCommandInit;
        frame->header.service_index = instance->config->index;
        frame->header.data_size = 0;
        frame->header.result = true;
        BLE_LOG_I("%s - enqueue init", instance->config->name);

        ble_service_enqueue_message(instance);
        ble_service_unlock(instance);
    }
}

void ble_service_enqueue_run(BleServiceObject* instance) {
    furi_assert(instance);
    BLE_LOG_D("%s - enqueue run", instance->config->name);

    ble_service_enqueue_message(instance);
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

size_t ble_service_count_characteristics_and_size(
    BleServiceObject* instance,
    bool modified_only,
    BleCharacteristicCountType* characteristics_count) {
    furi_assert(characteristics_count);

    const uint8_t chars_count_max = instance->config->char_count;

    size_t total_data_size = 0;
    uint8_t chars_count = 0;
    for(size_t i = 0; i < chars_count_max; i++) {
        if(modified_only && !ble_characteristic_is_modified(instance->chars[i])) continue;
        total_data_size += ble_characteristic_get_data_size(instance->chars[i]);
        chars_count++;
    }

    *characteristics_count = chars_count;
    return total_data_size;
}

BleIntercomServiceData* ble_service_create_intercom_service_data_pack(
    BleServiceObject* instance,
    bool modified_only,
    size_t* output_pack_size) {
    furi_assert(output_pack_size);
    BleCharacteristicCountType chars_count = 0;
    size_t total_data_size =
        ble_service_count_characteristics_and_size(instance, modified_only, &chars_count);

    size_t total_config_size = sizeof(BleCharacteristicDataHeader) * chars_count +
                               total_data_size + sizeof(BleCharacteristicCountType);

    const BleCharacteristicCountType chars_count_max = instance->config->char_count;

    BleIntercomServiceData* config = malloc(total_config_size);
    config->char_count = chars_count;
    uint8_t offset = 0;
    for(size_t i = 0; i < chars_count_max; i++) {
        BleCharacteristicObject* ch_obj = instance->chars[i];

        if(modified_only && !ble_characteristic_is_modified(ch_obj)) continue;
        BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)config->chars_config + offset);

        offset += ble_characteristic_fill_update_struct(ch_obj, char_init);
    }

    *output_pack_size = total_config_size;
    return config;
}

bool ble_service_parse_intercom_service_data(
    BleServiceObject* instance,
    const BleIntercomServiceData* data,
    BleParseIntercomServiceDataCharacteristicExtraAction action) {
    const BleIntercomServiceData* service_config = data;
    uint8_t offset = 0;

    for(size_t i = 0; i < service_config->char_count; i++) {
        const BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)service_config->chars_config + offset);
        size_t data_size = char_init->header.data_size;

        BleCharacteristicObject* ch = instance->chars[char_init->header.index];
        ble_characteristic_set_data(ch, char_init->data, data_size);

        BLE_LOG_D(
            "Ch: %s new data: %s",
            ble_characteristic_get_config(ch)->name,
            (const char*)char_init->data);

        if(action) action(ch);

        offset += (data_size + sizeof(BleCharacteristicDataHeader));
    }

    return true;
}
