#include "si917_info_client.h"
#include <furi.h>
#include <intercom/intercom.h>
#include "si917_info_common.h"

#define TAG "Si917InfoClient"

#define RESPONSE_TIMEOUT 200

struct Si917InfoClient {
    FuriMutex* lock_mutex;
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    Si917InfoData info_data;
    FuriMessageQueue* response_queue;
    bool data_valid;
};

static void si917_info_client_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    const Si917InfoResponseMessage* msg = data;
    furi_assert(data_size == sizeof(Si917InfoResponseMessage));
    furi_assert(context);
    Si917InfoClient* instance = context;

    furi_check(
        furi_message_queue_put(instance->response_queue, &msg->data, FuriWaitForever) ==
        FuriStatusOk);
}

static Si917InfoClient* si917_info_client_alloc(void) {
    Si917InfoClient* client = malloc(sizeof(Si917InfoClient));

    client->lock_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    client->response_queue = furi_message_queue_alloc(1, sizeof(Si917InfoData));

    client->intercom = furi_record_open(RECORD_INTERCOM);
    client->intercom_ch = intercom_channel_open(
        client->intercom, IntercomChannelIdSi917Info, si917_info_client_rx_callback, client);

    return client;
}

bool si917_info_get(Si917InfoClient* client, Si917InfoData* info) {
    furi_assert(client);
    furi_assert(info);
    bool success = false;

    furi_mutex_acquire(client->lock_mutex, FuriWaitForever);
    if(client->data_valid) {
        memcpy(info, &client->info_data, sizeof(Si917InfoData));
        success = true;
    } else {
        Si917InfoRequestMessage request_msg = {.type = Si917InfoMessageRequest};

        size_t packet_len = sizeof(Si917InfoRequestMessage);
        size_t tx_size =
            intercom_tx(client->intercom_ch, &request_msg, packet_len, FuriWaitForever);
        furi_check(tx_size == packet_len, "Failed to send data");

        if(furi_message_queue_get(client->response_queue, &client->info_data, RESPONSE_TIMEOUT) ==
           FuriStatusOk) {
            memcpy(info, &client->info_data, sizeof(Si917InfoData));
            success = true;
        } else {
            FURI_LOG_E(TAG, "Request timeout");
        }

        client->data_valid = success;
    }
    furi_mutex_release(client->lock_mutex);

    return success;
}

int32_t si917_info_client_init(void* arg) {
    UNUSED(arg);
    Si917InfoClient* client = si917_info_client_alloc();
    furi_record_create(RECORD_SI917_INFO_CLIENT, client);
    return 0;
}
