#include "ble_i.h"
#include "ble_system_command.h"

#if !defined(BSB_MCU_SI917)
#include "http/ble_http_repeater.h"
#include "streaming/ble_streaming.h"
#endif

#define TAG "BLE"

#define BLE_SERVICE_LOCK_TIMEOUT         (100)
#define BLE_SERVICE_MAILBOX_LOCK_TIMEOUT (100)
#define BLE_COMMAND_INVOKE_RETRY_TIMEOUT (100)

static void ble_backend_intercom_rx_callback(const void* data, size_t data_size, void* context);

void ble_set_service_post_process_callback(Ble* ble, BleServicePostProcessCallback callback) {
    furi_assert(ble);
    ble->service_post_process_callback = callback;
}

static void ble_check_invoke_service_process_result(
    Ble* instance,
    const BleServiceObjectResult* service_result) {
    furi_assert(instance);
    furi_assert(service_result);

    BleServiceObject* service = service_result->service;
    bool result = service_result->result;

#if !defined(BSB_MCU_SI917)
    if(!result && !ble_service_is_ready(service)) {
        FuriString* buf = furi_string_alloc();
        ble_service_get_error(service, buf);

        furi_string_printf(
            instance->error, "%s - %s", ble_service_get_name(service), furi_string_get_cstr(buf));

        furi_string_free(buf);

        BLE_LOG_W("Error: %s", furi_string_get_cstr(instance->error));
        instance->status = BleServiceStatusError;

        ble_command_engine_unblock_with_result(instance->engine, NULL, 0, false);
    } else if(instance->service_post_process_callback) {
        instance->service_post_process_callback(service, result, instance);
    }
#else
    if(instance->service_post_process_callback) {
        instance->service_post_process_callback(service, result, instance);
    }
#endif
}

static void ble_allocate_services(Ble* instance) {
    for(size_t i = 0; i < BleServiceIndexCount; i++) {
        instance->services[i] =
            ble_service_alloc(service_config[i], instance->service_queue, instance->intercom_ch);
    }
}

static void ble_custom_event_handler_init(Ble* instance) {
    instance->intercom_ch = intercom_channel_open(
        instance->intercom, IntercomChannelIdBle, ble_backend_intercom_rx_callback, instance);

    ble_allocate_services(instance);
#if !defined(BSB_MCU_SI917)
    ble_command_engine_put_command_no_wait(instance->engine, BleCommandInit, NULL, 0);
#endif
}

static void ble_event_loop_msg_queue_handler(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Ble* ble = context;
    furi_assert(object == ble->service_queue);

    BleServiceObjectMessage* message = NULL;
    furi_check(furi_message_queue_get(ble->service_queue, &message, 0) == FuriStatusOk);
    BleServiceObjectResult result = ble_service_process(message);
    ble_check_invoke_service_process_result(ble, &result);
}

static void ble_custom_event_callback(uint32_t events, void* context) {
    Ble* instance = context;

    if(furi_mutex_acquire(instance->ble_lock, BLE_SERVICE_LOCK_TIMEOUT) == FuriStatusOk) {
        if(events & BleEventTypeIntercomInit) {
            ble_custom_event_handler_init(instance);
        }

        if(events & BleEventTypeIntercomDeinit) {
            ble_command_engine_unblock_with_result(instance->engine, NULL, 0, false);
            ble_command_engine_put_command_no_wait(instance->engine, BleCommandDeinit, NULL, 0);
        }

        if(events & BleEventTypeFrameReceived) {
            ble_command_engine_run(instance->engine, &instance->mailbox);
            furi_semaphore_release(instance->mailbox_lock);
        }

        if(events & BleEventTypeFrameLost) {
            ble_command_engine_unblock_with_result(instance->engine, NULL, 0, false);
            furi_semaphore_release(instance->mailbox_lock);
        }

        furi_mutex_release(instance->ble_lock);
    } else
        BLE_LOG_W("Unable to lock BLE");
}

static void ble_backend_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size < MAX_BLE_INTERCOM_FRAME_SIZE);
    Ble* instance = context;
    const BleIntercomFrameGeneric* const frame = data;

    furi_check(frame->header.source != BleIntercomFrameSourceUnknown);
    furi_check(frame->header.frame_type != BleIntercomFrameTypeUnknown);

    if(frame->header.source == BleIntercomFrameSourceSystem) {
        if(furi_semaphore_acquire(instance->mailbox_lock, BLE_SERVICE_MAILBOX_LOCK_TIMEOUT) ==
           FuriStatusOk) {
            memcpy(&instance->mailbox, data, data_size);
            furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeFrameReceived);
        } else {
            BLE_LOG_W("Packet lost!");
            furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeFrameLost);
        }
    } else {
        BleServiceObject* service = instance->services[frame->header.service_index];
        ble_service_process_mailbox(service, frame);
    }
}

static void ble_intercom_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    Ble* instance = context;
    const IntercomStatus intercom_status = *(IntercomStatus*)item;

    if(intercom_status == IntercomStatusOk) {
        furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeIntercomInit);
    } else if(intercom_status != IntercomStatusUnknown) {
        furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeIntercomDeinit);
    }
}

static Ble* ble_alloc() {
    Ble* instance = malloc(sizeof(Ble));
    instance->status = BleServiceStatusReset;
    instance->event_loop = furi_event_loop_alloc();
    instance->mailbox_lock = furi_semaphore_alloc(1, 1);
    instance->ble_lock = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->engine =
        ble_command_engine_alloc(instance, ble_commands, BleCommandCount, instance->event_loop);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, ble_custom_event_callback, instance);

    instance->service_queue =
        furi_message_queue_alloc(BleServiceIndexCount, sizeof(BleServiceObjectMessage*));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->service_queue,
        FuriEventLoopEventIn,
        ble_event_loop_msg_queue_handler,
        instance);

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    furi_state_subscribe(
        intercom_get_state(instance->intercom), ble_intercom_state_callback, instance);

    instance->error = furi_string_alloc();
#if !defined(BSB_MCU_SI917)
    ble_http_repeater_init();
    instance->streaming = ble_streaming_alloc(instance);
    instance->on_status_change = furi_pubsub_alloc();
#endif

    furi_record_create(RECORD_BLE, instance);

    return instance;
}

int32_t ble_srv(void* arg) {
    UNUSED(arg);
    Ble* instance = ble_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
