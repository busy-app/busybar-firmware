#include "busy_i.h"

#define BUSY_API_TIMEOUT_MS (1000)

static BusyStatus busy_api_send_message_async(BusyApp* instance, const BusyApiMessage* message) {
    BusyStatus api_status;

    const FuriStatus status = furi_message_queue_put(
        instance->api_queue, message, furi_ms_to_ticks(BUSY_API_TIMEOUT_MS));

    if(status == FuriStatusOk) {
        api_status = BusyStatusOk;
    } else if(status == FuriStatusErrorTimeout) {
        api_status = BusyStatusTimeout;
    } else {
        furi_crash("Unexpected FuriStatus value");
    }

    return api_status;
}

static BusyStatus busy_api_send_message(BusyApp* instance, BusyApiMessage* message) {
    BusyStatus status;

    message->status = &status;
    message->lock = api_lock_alloc_locked();

    BusyStatus api_status = busy_api_send_message_async(instance, message);
    if(api_status == BusyStatusOk) {
        api_lock_wait_unlock_and_free(message->lock);

    } else {
        api_lock_free(message->lock);
        status = api_status;
    }

    return status;
}

void busy_api_unlock_message(BusyApiMessage* api_message, BusyStatus status) {
    if(api_message->lock) {
        furi_assert(api_message->status);
        *api_message->status = status;
        api_lock_unlock(api_message->lock);
    }
}

void busy_api_abort_pending_messages(BusyApp* instance) {
    FURI_CRITICAL_ENTER();

    BusyApiMessage api_message;

    while(furi_message_queue_get(instance->api_queue, &api_message, 0) == FuriStatusOk) {
        busy_api_unlock_message(&api_message, BusyStatusAborted);
    }

    memset(&api_message, 0, sizeof(api_message));

    while(furi_message_queue_put(instance->api_queue, &api_message, 0) == FuriStatusOk) {
        // HACK: Fill up the queue so that it cannot receive any more messages
    }

    FURI_CRITICAL_EXIT();
}

BusyStatus busy_set_config(BusyApp* instance, const BusyAppConfig* config) {
    furi_check(instance);
    furi_check(config);

    BusyApiMessage message = {
        .type = BusyApiMessageTypeSetConfig,
        .data.set_config.config = config,
    };

    return busy_api_send_message(instance, &message);
}

BusyStatus busy_show_timer(BusyApp* instance) {
    furi_check(instance);

    BusyApiMessage message = {
        .type = BusyApiMessageTypeShowTimer,
    };

    return busy_api_send_message(instance, &message);
}

BusyStatus busy_request_exit(BusyApp* instance) {
    furi_check(instance);

    BusyApiMessage message = {
        .type = BusyApiMessageTypeRequestExit,
    };

    return busy_api_send_message_async(instance, &message);
}
