#include "ble_i.h"
#include "ble_command.h"

#if defined(SI917)
#include "worker/ble_worker.h"
#else
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

    BleServiceCommand msg;
    if(furi_message_queue_get(ble->message_queue, &msg, FuriWaitForever) == FuriStatusOk) {
        BleServiceObject* service = ble->services[msg.service_index];
        ble_service_process(service, &msg);
    } else
        BLE_LOG_W("MsgQueue is full!");
}

void ble_custom_event_callback(uint32_t events, void* context) {
    Ble* instance = context;

    if(furi_mutex_acquire(instance->ble_lock, 100) == FuriStatusOk) {
        if(events & BleEventTypeServiceStateChanged) {
            ble_update_state_from_services(instance);
        }

        if((events & BleEventTypeFrameReceived) || (events & BleEventTypeIncomingMessage)) {
            BleIntercomFrameGeneric* frame = ble_command_preprocess(instance, events);
            if(frame) {
                BLE_LOG_D(
                    "Rx Frame t: %d c: %d s: %d ds: %d fs: %d",
                    frame->header.frame_type,
                    frame->header.command,
                    frame->header.service_index,
                    frame->header.data_size,
                    frame->header.data_size + sizeof(BleIntercomFrameHeader));
                const BleCommand command = frame->header.command;
                if(command == BleCommandInit) {
                    ble_command_handler_init(instance, frame);
                } else if(command == BleCommandEnable) {
                    ble_command_handler_enable(instance, frame);
                } else if(command == BleCommandDisable) {
                    ble_command_handler_disable(instance, frame);
                } else if(command == BleCommandGetState) {
                    ble_command_handler_get_state(instance, (BleIntercomFrameStatus*)frame);
                } else if(events & BleEventTypeFrameReceived) {
                    BleServiceIndex index = frame->header.service_index;
                    BleServiceObject* service = instance->services[index];
                    ble_service_process_mailbox(service, frame);
                }
            }

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
    if(furi_semaphore_acquire(instance->mailbox_lock, 100) == FuriStatusOk) {
        memcpy(&instance->mailbox, data, data_size);
        furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeFrameReceived);
    } else
        BLE_LOG_W("Packet lost!");
}

static void ble_service_state_change_callback(void* context) {
    furi_assert(context);
    Ble* instance = context;
    BLE_LOG_D("ble_service_state_change_callback");
    furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeServiceStateChanged);
}

#if !defined(SI917) && defined(BLE_AUTO_INIT)
static void ble_init_timer_callback(void* context) {
    BLE_LOG_D("ble_init_timer_callback");
    Ble* instance = context;
    ble_init(instance);
}
#endif

static Ble* ble_alloc() {
    Ble* instance = malloc(sizeof(Ble));
    instance->state = BleServiceStateReset;
    instance->event_loop = furi_event_loop_alloc();
    instance->mailbox_lock = furi_semaphore_alloc(1, 1);
    instance->ble_lock = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->message_queue =
        furi_message_queue_alloc(BLE_SERVICES_COUNT, sizeof(BleServiceCommand));

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

#if !defined(SI917) && defined(BLE_AUTO_INIT)
    ble_http_repeater_init();
    instance->init_timer = furi_timer_alloc(ble_init_timer_callback, FuriTimerTypeOnce, instance);
    furi_timer_start(instance->init_timer, 3000);
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
