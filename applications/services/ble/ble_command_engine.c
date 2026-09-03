#include "ble_command_engine.h"
#include "ble_log.h"
#include <api_lock.h>

#define BLE_ENGINE_TIMEOUT (100)

#define TAG "BleCmdEng"

typedef struct {
    BleSystemCommand command_code;
    FuriApiLock lock;
    size_t data_size;
    void* data;
    bool result;
    bool internal;
} BleCommand;

typedef struct {
    BleIntercomFrameHeader header;
    uint8_t data[];
} BleCommandIntercomFrame;

struct BleCommandEngine {
    uint8_t commands_count;
    const BleCommandItem* commands;
    Ble* ble;

    FuriMessageQueue* command_queue;
    FuriMutex* current_command_lock;
    BleCommand* current_command;

    BleCommandIntercomFrame* frame;
    size_t frame_size;
};

static inline void
    ble_command_frame_check_alloc(BleCommandEngine* instance, const size_t new_frame_size) {
    if(new_frame_size > instance->frame_size) {
        instance->frame = realloc(instance->frame, new_frame_size);
        furi_check(instance->frame);
        instance->frame_size = new_frame_size;
    }
}

static void ble_command_convert_to_intercom_frame(
    BleCommandEngine* instance,
    const BleCommand* const command) {
    const size_t new_msg_size = sizeof(BleIntercomFrameHeader) + command->data_size + sizeof(bool);
    ble_command_frame_check_alloc(instance, new_msg_size);

    BleCommandIntercomFrame* frame = instance->frame;
    frame->header.command = command->command_code;
    frame->header.frame_type = BleIntercomFrameTypeRequest;
    frame->header.source = BleIntercomFrameSourceSystem;
    frame->header.data_size = command->data_size;
    frame->header.result = command->result;

    if(command->data_size > 0) {
        memcpy(frame->data, command->data, command->data_size);
    }
}

static void ble_command_engine_queue_handler(FuriEventLoopObject* object, void* context) {
    BleCommandEngine* instance = context;
    furi_assert(context);
    furi_assert(object == instance->command_queue);

    if(furi_mutex_acquire(instance->current_command_lock, BLE_ENGINE_TIMEOUT) == FuriStatusOk) {
        BleCommand* command = NULL;
        furi_check(furi_message_queue_get(object, &command, BLE_ENGINE_TIMEOUT) == FuriStatusOk);

        ble_command_convert_to_intercom_frame(instance, command);
        instance->current_command = command;

        if(!ble_command_engine_run(instance, (BleIntercomFrameGeneric*)instance->frame)) {
            ble_command_engine_unblock_with_result(instance, NULL, 0, false);
        }
    } else {
        BLE_LOG_D("Command lock failed");
    }
}

static BleCommand*
    ble_command_alloc(BleCommandCode code, size_t data_size, void* data, bool sync) {
    BleCommand* command = malloc(sizeof(BleCommand));

    command->command_code = code;
    command->internal = !sync;
    command->result = false;
    command->data_size = data_size;
    command->data = NULL;
    command->lock = sync ? api_lock_alloc_locked() : NULL;
    if(data_size > 0 && data != NULL) {
        command->data = malloc(data_size);
        memcpy(command->data, data, data_size);
    }
    return command;
}

static void ble_command_free(BleCommand* command) {
    if(command->data) {
        free(command->data);
    }
    free(command);
}

BleCommandEngine* ble_command_engine_alloc(
    Ble* ble,
    const BleCommandItem* commands,
    uint8_t commands_count,
    FuriEventLoop* event_loop) {
    furi_assert(ble);
    furi_assert(commands);
    furi_assert(commands_count > 0);

    BleCommandEngine* instance = malloc(sizeof(BleCommandEngine));
    instance->ble = ble;

    instance->commands = commands;
    instance->commands_count = commands_count;

    instance->current_command_lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->current_command = NULL;
    instance->command_queue = furi_message_queue_alloc(10, sizeof(BleCommand*));
    instance->frame_size = 0;
    instance->frame = NULL;

    furi_event_loop_subscribe_message_queue(
        event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        ble_command_engine_queue_handler,
        instance);

    return instance;
}

bool ble_command_engine_run(BleCommandEngine* instance, BleIntercomFrameGeneric* frame) {
    furi_assert(instance);
    furi_assert(frame);

    const BleIntercomFrameType frame_type = frame->header.frame_type;
    const BleCommandCode command = (BleCommandCode)frame->header.command;
    const BleCommandCode unknown_command = 0;
    furi_check(command != unknown_command);
    furi_check(command < instance->commands_count);
    const BleCommandItem* const item = &instance->commands[command];

    bool result = false;

    if(frame_type == BleIntercomFrameTypeRequest && item->request) {
        result = item->request(frame, instance->ble);
    } else if(frame_type == BleIntercomFrameTypeResponse && item->response) {
        result = item->response(frame, instance->ble);
    } else {
        furi_crash("Unknown frame");
    }

    return result;
}

void ble_command_engine_put_command_no_wait(
    BleCommandEngine* instance,
    BleSystemCommand code,
    void* data,
    size_t data_size) {
    furi_assert(instance);
    furi_assert(code > BleCommandUnknown && code < BleCommandCount);
    BleCommand* command = ble_command_alloc(code, data_size, data, false);
    furi_check(
        furi_message_queue_put(instance->command_queue, &command, BLE_ENGINE_TIMEOUT) ==
        FuriStatusOk);
}

bool ble_command_engine_put_command(
    BleCommandEngine* instance,
    BleSystemCommand code,
    void* data,
    size_t data_size) {
    furi_assert(instance);
    furi_assert(code > BleCommandUnknown && code < BleCommandCount);

    BleCommand* command = ble_command_alloc(code, data_size, data, true);
    furi_check(
        furi_message_queue_put(instance->command_queue, &command, BLE_ENGINE_TIMEOUT) ==
        FuriStatusOk);

    api_lock_wait_unlock_and_free(command->lock);

    bool result = command->result;

    if(data_size > 0) {
        if(data_size == command->data_size) {
            memcpy(data, command->data, command->data_size);
        } else {
            BLE_LOG_W("Buffer size not equal to data size");
            result = false;
        }
    }

    ble_command_free(command);
    return result;
}

void ble_command_engine_unblock_with_result(
    BleCommandEngine* instance,
    const void* const data,
    const size_t data_size,
    bool result) {
    furi_assert(instance);
    BLE_LOG_D("%s", __func__);

    if(instance->current_command == NULL) {
        BLE_LOG_W("No command, skip");
        return;
    }

    instance->current_command->result = result;
    if(data_size > 0) {
        if(data_size == instance->current_command->data_size) {
            memcpy(instance->current_command->data, data, data_size);
        } else {
            BLE_LOG_W("Buffer size not equal to data size!!!");
            instance->current_command->result = false;
        }
    }

    if(instance->current_command->internal) {
        ble_command_free(instance->current_command);
    } else {
        BLE_LOG_D("Release api lock");
        api_lock_unlock(instance->current_command->lock);
    }

    instance->current_command = NULL;
    memset(instance->frame, 0, instance->frame_size);

    BLE_LOG_D("Release command lock");
    furi_mutex_release(instance->current_command_lock);
}
