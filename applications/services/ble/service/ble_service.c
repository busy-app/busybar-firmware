#include "ble_service_i.h"
#include "target/ble_service_target.h"

#define TAG "BleServiceBase"

#define BLE_SERVICE_LOCK_TIMEOUT (5000)

static void ble_service_enqueue_message(
    BleServiceObject* instance,
    const size_t data_size,
    const void* data);

bool ble_service_lock(BleServiceObject* instance) {
    if(furi_mutex_acquire(instance->service_lock, BLE_SERVICE_LOCK_TIMEOUT) != FuriStatusOk) {
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

static inline void ble_service_prepare_intercom_frame_header(
    BleIntercomFrameHeader* const header,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    bool result,
    uint16_t service_index,
    size_t data_size,
    uint32_t num) {
    header->source = BleIntercomFrameSourceService;
    header->frame_type = frame_type;
    header->command = command;
    header->service_index = service_index;
    header->data_size = data_size;
    header->result = result;
    header->num = num;
}

static bool ble_service_prepare_send_intercom_frame(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    bool result,
    size_t data_size,
    const void* data) {
    BleIntercomFrameHeader header = {0};
    ble_service_prepare_intercom_frame_header(
        &header,
        frame_type,
        command,
        result,
        instance->config->index,
        data_size,
        instance->sequence_num);

    bool send_result = false;
    if(ble_service_frame_lock(instance->output_frame)) {
        ble_service_frame_append_data(
            instance->output_frame, &header, sizeof(BleIntercomFrameHeader));

        if(data_size && data) {
            ble_service_frame_append_data(instance->output_frame, data, data_size);
        }

        const void* frame = ble_service_frame_get_data_ptr(instance->output_frame);
        const size_t frame_size = ble_service_frame_get_data_size(instance->output_frame);

        size_t tx =
            intercom_tx(instance->intercom_ch, frame, frame_size, BLE_INTERCOM_TX_TIMEOUT_MS);

        if(tx == frame_size) {
            instance->sequence_num += 1;
            send_result = true;
        } else {
            ble_service_set_error(instance, "Unable to send data via intercom");
        }

        ble_service_frame_unlock(instance->output_frame);
    } else {
        BLE_LOG_W("Send intercom frame failed");
    }

    return send_result;
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

BleServiceObject* ble_service_alloc(
    const BleServiceConfig* service_config,
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
    instance->service_lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->output_frame = ble_service_frame_alloc();

    if(service_config->char_count) {
        instance->chars = malloc(sizeof(BleCharacteristicObject*) * service_config->char_count);
        for(size_t i = 0; i < service_config->char_count; i++) {
            const BleCharacteristicConfig* config = &service_config->char_configs[i];
            BleCharacteristicObject* ble_char = ble_characteristic_alloc(config, instance);
            instance->chars[config->intercom_index] = ble_char;
        }
    }

    return instance;
}

BleServiceObjectResult ble_service_process(BleServiceObjectMessage* message) {
    furi_assert(message);

    BleServiceObjectResult ret = {.service = message->header.service, .result = false};
    BleServiceObject* instance = message->header.service;

    if(ble_service_lock(instance)) {
        const BleIntercomFrameGeneric* frame = (BleIntercomFrameGeneric*)message->data;
        const BleIntercomFrameHeader* hdr = &frame->header;

        if(hdr->result) {
            ret.result = ble_service_target_execute(
                instance, hdr->frame_type, hdr->command, hdr->data_size, frame->data);
        } else {
            ble_service_set_error(
                instance, "Error, frame_type: %d, cmd: %d", hdr->frame_type, hdr->command);
        }

        ble_service_unlock(instance);
    }

    free(message);
    return ret;
}

void ble_service_process_mailbox(
    BleServiceObject* instance,
    const BleIntercomFrameGeneric* input_frame) {
    furi_assert(instance);
    furi_assert(input_frame);
    BLE_LOG_D("ble_service_process_mailbox");

    size_t fs = input_frame->header.data_size + sizeof(BleIntercomFrameHeader);
    ble_service_enqueue_message(instance, fs, input_frame);
}

static void ble_service_enqueue_message(
    BleServiceObject* instance,
    const size_t data_size,
    const void* data) {
    furi_assert(instance);

    BleServiceObjectMessage* msg = malloc(sizeof(BleServiceObjectMessageHeader) + data_size);
    msg->header.service = instance;
    msg->header.data_size = data_size;
    if(data_size) {
        memcpy(msg->data, data, data_size);
    }

    if(furi_message_queue_put(instance->message_queue, &msg, 100) != FuriStatusOk) {
        BLE_LOG_W("%s - unable to enqueue for processing", instance->config->name);
        free(msg);
    }
}

void ble_service_enqueue_init(BleServiceObject* instance) {
    furi_assert(instance);

    BleIntercomFrameHeader init_data = {0};
    ble_service_prepare_intercom_frame_header(
        &init_data,
        BleIntercomFrameTypeRequest,
        BleServiceCommandInit,
        true,
        instance->config->index,
        0,
        instance->sequence_num);

    do {
        if(!ble_service_lock(instance)) break;

        ble_service_enqueue_message(instance, sizeof(BleIntercomFrameHeader), &init_data);

        ble_service_unlock(instance);
    } while(false);
}

void ble_service_enqueue_run(BleServiceObject* instance) {
    BLE_LOG_D("%s - enqueue run", instance->config->name);

    BleIntercomFrameHeader data = {0};
    ble_service_prepare_intercom_frame_header(
        &data,
        BleIntercomFrameTypeRequest,
        BleServiceCommandRun,
        true,
        instance->config->index,
        0,
        instance->sequence_num);

    ble_service_enqueue_message(instance, sizeof(BleIntercomFrameHeader), &data);
}

void ble_service_enqueue_run_with_data(
    BleServiceObject* instance,
    size_t data_size,
    const void* data) {
    furi_assert(instance);

    size_t total_size = sizeof(BleIntercomFrameHeader) + data_size;
    BleIntercomFrameGeneric* frame = malloc(total_size);

    ble_service_prepare_intercom_frame_header(
        &frame->header,
        BleIntercomFrameTypeRequest,
        BleServiceCommandRun,
        true,
        instance->config->index,
        data_size,
        instance->sequence_num);
    memcpy(frame->data, data, data_size);

    ble_service_enqueue_message(instance, total_size, frame);
    free(frame);
}

void ble_service_deinit(BleServiceObject* instance) {
    if(instance && ble_service_lock(instance)) {
        BLE_LOG_D("%s - ble_service_reset", instance->config->name);
        ble_service_target_execute(
            instance, BleIntercomFrameTypeRequest, BleServiceCommandDeinit, 0, NULL);

        ble_service_frame_unlock(instance->output_frame);

        for(uint8_t i = 0; i < instance->config->char_count; i++) {
            ble_characteristic_reset(instance->chars[i]);
        }

        instance->ready = false;
        ble_service_unlock(instance);
    }
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

    BleIntercomServiceData* service_config = malloc(total_config_size);
    service_config->char_count = chars_count;
    size_t offset = 0;
    for(size_t i = 0; i < chars_count_max; i++) {
        BleCharacteristicObject* ch_obj = instance->chars[i];

        if(modified_only && !ble_characteristic_is_modified(ch_obj)) continue;
        BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)service_config->chars_config + offset);

        offset += ble_characteristic_encode(ch_obj, char_init);
    }

    *output_pack_size = total_config_size;

    return service_config;
}

bool ble_service_parse_intercom_service_data(
    BleServiceObject* instance,
    const BleIntercomServiceData* data) {
    const BleIntercomServiceData* service_config = data;
    size_t offset = 0;

    bool result = true;
    for(size_t i = 0; i < service_config->char_count; i++) {
        const BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)service_config->chars_config + offset);
        size_t data_size = char_init->header.data_size;

        BleCharacteristicObject* ch = instance->chars[char_init->header.index];
        result = ble_characteristic_decode(ch, char_init);
        if(!result) break;

        offset += (data_size + sizeof(BleCharacteristicDataHeader));
    }

    return result;
}

bool ble_service_send_data(
    BleServiceObject* instance,
    BleServiceCommandEnum command,
    BleIntercomFrameType frame_type,
    bool modified_only) {
    bool result = false;
    size_t total_size = 0;
    BleIntercomServiceData* config =
        ble_service_create_intercom_service_data_pack(instance, modified_only, &total_size);

    if(config->char_count > 0) {
        result = ble_service_prepare_send_intercom_frame(
            instance, frame_type, command, true, total_size, config);
    }

    free(config);
    return result;
}
