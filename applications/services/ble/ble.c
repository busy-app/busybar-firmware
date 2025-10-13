#include "ble_i.h"
#include "ble_command.h"

#if defined(SI917)
#include "worker/ble_worker.h"
#endif

#define TAG "BLE"

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

    BleIntercomFrameGeneric* frame = ble_command_preprocess(instance, events);
    if(frame) {
        BLE_LOG_D(
            "Rx Frame t: %d c: %d ds: %d fs: %d",
            frame->header.frame_type,
            frame->header.command,
            frame->header.data_size,
            frame->header.data_size + sizeof(BleIntercomFrameHeader));
        const BleCommand command = frame->header.command;
        if(command == BleCommandEnable) {
            ble_command_handler_enable(instance, frame);
        } else if(command == BleCommandDisable) {
            ble_command_handler_disable(instance, frame);
        } else if(command == BleCommandGetStatus) {
            ble_command_handler_get_status(instance, (BleIntercomFrameStatus*)frame);
        } else {
            BleServiceIndex index = frame->header.service_index;
            BleServiceObject* service = instance->services[index];
            ble_service_process_mailbox(service, frame);
        }
    }
    ble_command_postprocess(instance, events, true);
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

static void ble_init_timer_handler(void* context) {
    Ble* instance = context;
    BLE_LOG_D("GetStatus");
    if(furi_semaphore_acquire(instance->mailbox_lock, 100) == FuriStatusOk) {
        BleIntercomFrameStatus* frame = (BleIntercomFrameStatus*)&instance->mailbox;

        frame->header.frame_type = BleIntercomFrameTypeRequest;
        frame->header.command = BleCommandGetStatus;
        frame->header.data_size = 0;

        size_t frame_size = sizeof(BleIntercomFrameHeader);
        size_t tx = intercom_tx(instance->intercom, frame, frame_size, 100);
        furi_assert(tx == frame_size);

        furi_semaphore_release(instance->mailbox_lock);
    }
}

// #if !defined(SI917)
// static void ble_event_loop_on_start(void* context) {
//     BLE_LOG_W("ble_event_loop_on_start");
//     Ble* instance = context;
//     furi_event_loop_timer_start(instance->init_timer, 1000);
// }
// #endif

static Ble* ble_alloc() {
    Ble* instance = malloc(sizeof(Ble));
    instance->state = BleServiceStateReset;
    instance->event_loop = furi_event_loop_alloc();
    instance->mailbox_lock = furi_semaphore_alloc(1, 1);
    instance->access_semaphore = furi_semaphore_alloc(1, 1);

    instance->message_queue =
        furi_message_queue_alloc(BLE_SERVICES_COUNT, sizeof(BleServiceCommand));

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, ble_custom_event_callback, instance);

    instance->init_timer = furi_event_loop_timer_alloc(
        instance->event_loop, ble_init_timer_handler, FuriEventLoopTimerTypePeriodic, instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        ble_event_loop_msg_queue_handler,
        instance);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom = intercom_channel_open(
        intercom, IntercomChannelBle, FuriWaitForever, ble_backend_intercom_rx_callback, instance);

    for(size_t i = 0; i < BLE_SERVICES_COUNT; i++) {
        instance->services[i] =
            ble_service_alloc(service_config[i], instance->message_queue, instance->intercom);
    }

#if defined(SI917)
    // ble_worker_init();
#else
    // furi_event_loop_pend_callback(instance->event_loop, ble_event_loop_on_start, instance);
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
