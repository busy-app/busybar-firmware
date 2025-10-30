#include "tls_crypto_client.h"
#include <furi.h>
#include <intercom/intercom.h>
#include "tls_crypto_common.h"

#define TAG              "TlsCryptoClient"
#define RESPONSE_TIMEOUT 200
#define ACQUIRE_TIMEOUT  500

struct TlsCryptoClient {
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    FuriMutex* transaction;
    FuriMessageQueue* response_queue;
};

static void tls_crypto_client_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(
        (data_size == sizeof(TlsCryptoSignMessage)) || (data_size == sizeof(TlsCryptoError)));
    furi_assert(context);
    TlsCryptoClient* instance = context;

    furi_check(
        furi_message_queue_put(instance->response_queue, data, FuriWaitForever) == FuriStatusOk);
}

static TlsCryptoClient* tls_crypto_client_alloc(void) {
    TlsCryptoClient* client = malloc(sizeof(TlsCryptoClient));

    client->response_queue = furi_message_queue_alloc(1, sizeof(TlsCryptoSignMessage));

    client->intercom = furi_record_open(RECORD_INTERCOM);
    client->intercom_ch = intercom_channel_open(
        client->intercom, IntercomChannelIdTlsCrypto, tls_crypto_client_rx_callback, client);

    client->transaction = furi_mutex_alloc(FuriMutexTypeNormal);

    return client;
}

bool tls_crypto_client_sign(
    TlsCryptoClient* client,
    uint8_t key_slot,
    const uint8_t* hash,
    size_t hash_len,
    uint8_t* sign_buf,
    size_t sign_buf_size,
    size_t* sign_len) {
    furi_assert(hash);
    furi_assert(sign_buf);
    furi_assert(hash_len <= TLS_CRYPTO_DATA_SIZE_MAX);
    bool success = false;

    furi_check(furi_mutex_acquire(client->transaction, ACQUIRE_TIMEOUT) == FuriStatusOk);

    TlsCryptoSignMessage sign_msg;
    sign_msg.cmd = TlsCryptoSignRequest;
    sign_msg.key_slot = key_slot;
    sign_msg.data_size = hash_len;
    memcpy(sign_msg.data, hash, hash_len);

    size_t tx_size =
        intercom_tx(client->intercom_ch, &sign_msg, sizeof(TlsCryptoSignMessage), FuriWaitForever);
    furi_check(tx_size == sizeof(TlsCryptoSignMessage), "Failed to send data");

    if(furi_message_queue_get(client->response_queue, &sign_msg, RESPONSE_TIMEOUT) ==
       FuriStatusOk) {
        if(sign_msg.cmd == TlsCryptoSignResponse) {
            furi_assert(sign_msg.key_slot == key_slot);
            size_t resp_sign_len = sign_msg.data_size;
            FURI_LOG_D(TAG, "TlsCryptoSignResponse len:%u", resp_sign_len);
            furi_assert(sign_buf_size >= resp_sign_len);
            memcpy(sign_buf, sign_msg.data, resp_sign_len);
            if(sign_len) {
                *sign_len = resp_sign_len;
            }
            success = true;
        } else if(sign_msg.cmd == TlsCryptoError) {
            FURI_LOG_E(TAG, "917 returned error");
        } else {
            furi_crash("Unsupported response");
        }
    }

    furi_check(furi_mutex_release(client->transaction) == FuriStatusOk);

    return success;
}

int32_t tls_crypto_client_init(void* arg) {
    UNUSED(arg);
    TlsCryptoClient* client = tls_crypto_client_alloc();
    furi_record_create(RECORD_TLS_CRYPTO_CLIENT, client);
    return 0;
}
