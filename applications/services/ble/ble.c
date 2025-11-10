#include "ble_i.h"
#include "ble_system_command.h"

#if !defined(SI917)
#include "http/ble_http_repeater.h"
#endif

#define TAG "BLE"

static void ble_update_state_from_services(Ble* instance) {
    BLE_LOG_D("ble_update_state_from_services");

    uint8_t services_in_states[BleServiceStateCount] = {0};
    for(uint8_t i = 0; i < BLE_SERVICES_COUNT; i++) {
        BleServiceObject* service = instance->services[i];
        BleServiceState state = ble_service_get_state(service);
        services_in_states[state]++;
    }

    if(services_in_states[BleServiceStateError] > 0) {
        BLE_LOG_W("Some service has an error");
        instance->state = BleServiceStateError;
    } else {
        for(uint8_t i = 0; i < BLE_SERVICES_COUNT; i++) {
            if(services_in_states[i] == BLE_SERVICES_COUNT) {
                BLE_LOG_I("State changed to: %d", i);
                instance->state = i;
                break;
            }
        }
    }
}

static void ble_event_loop_msg_queue_handler(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Ble* ble = context;
    furi_assert(object == ble->message_queue);

    BleServiceObject* service = NULL;
    if(furi_message_queue_get(ble->message_queue, &service, FuriWaitForever) == FuriStatusOk) {
        ble_service_process(service);
    } else
        BLE_LOG_W("Unable to get message from queue!");
}

void ble_custom_event_callback(uint32_t events, void* context) {
    Ble* instance = context;

    if(furi_mutex_acquire(instance->ble_lock, 100) == FuriStatusOk) {
        if(events & BleEventTypeServiceStateChanged) {
            ble_update_state_from_services(instance);
        }

        if(events & BleEventTypeDeviceNameChanged) {
            ble_invoke_retry_command_on_internal_event(
                instance, BleCommandSetDeviceName, BleEventTypeDeviceNameChanged, 100);
        }

        if((events & BleEventTypeFrameReceived) || (events & BleEventTypeIncomingMessage)) {
            BleIntercomFrameGeneric* frame = ble_command_preprocess(instance, events);
            ble_command_engine_run(instance->engine, frame, instance);

            if(events & BleEventTypeFrameReceived) {
                furi_semaphore_release(instance->mailbox_lock);
            }
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
        if(furi_semaphore_acquire(instance->mailbox_lock, 100) == FuriStatusOk) {
            memcpy(&instance->mailbox, data, data_size);
            furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeFrameReceived);
        } else
            BLE_LOG_W("Packet lost!");
    } else {
        BleServiceObject* service = instance->services[frame->header.service_index];
        ble_service_process_mailbox(service, frame);
    }
}

static void ble_service_state_change_callback(void* context) {
    furi_assert(context);
    Ble* instance = context;
    BLE_LOG_D("ble_service_state_change_callback");
    furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeServiceStateChanged);
}

static Ble* ble_alloc() {
    Ble* instance = malloc(sizeof(Ble));
    instance->state = BleServiceStateReset;
    instance->event_loop = furi_event_loop_alloc();
    instance->mailbox_lock = furi_semaphore_alloc(1, 1);
    instance->ble_lock = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->message_queue =
        furi_message_queue_alloc(BLE_SERVICES_COUNT, sizeof(BleServiceObject*));
    instance->engine = ble_command_engine_alloc(ble_commands, BleCommandCount, NULL, NULL);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, ble_custom_event_callback, instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        ble_event_loop_msg_queue_handler,
        instance);

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelBle, ble_backend_intercom_rx_callback, instance);

    for(size_t i = 0; i < BLE_SERVICES_COUNT; i++) {
        instance->services[i] = ble_service_alloc(
            service_config[i],
            instance->message_queue,
            instance->intercom,
            ble_service_state_change_callback,
            instance);
    }

#if !defined(SI917)
    ble_http_repeater_init();

    instance->current_message_lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->current_message_api_lock = api_lock_alloc_locked();
    instance->current_message_size = 0;
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
