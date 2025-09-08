#include <furi.h>
#include <intercom/intercom.h>
#include "tls_crypto_common.h"

#define TAG              "TlsCryptoClient"
#define RESPONSE_TIMEOUT 200

typedef struct {
    Intercom* intercom;
    TlsCryptoSignMessage sign_msg;
    FuriMessageQueue* response_queue;
} TlsCryptoClient;

static void tls_crypto_client_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(
        (data_size == sizeof(TlsCryptoSignMessage)) || (data_size == sizeof(TlsCryptoError)));
    furi_assert(context);
    TlsCryptoClient* instance = context;

    furi_check(
        furi_message_queue_put(instance->response_queue, data, FuriWaitForever) == FuriStatusOk);
}

bool tls_crypto_client_sign(
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

    TlsCryptoClient* instance = malloc(sizeof(TlsCryptoClient));
    instance->response_queue = furi_message_queue_alloc(1, sizeof(TlsCryptoSignMessage));

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelTlsCrypto, tls_crypto_client_rx_callback, instance);

    instance->sign_msg.cmd = TlsCryptoSignRequest;
    instance->sign_msg.key_slot = key_slot;
    instance->sign_msg.data_size = hash_len;
    memcpy(instance->sign_msg.data, hash, hash_len);

    size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelTlsCrypto,
        &(instance->sign_msg),
        sizeof(TlsCryptoSignMessage),
        FuriWaitForever);
    furi_check(tx_size == sizeof(TlsCryptoSignMessage), "Failed to send data");

    if(furi_message_queue_get(instance->response_queue, &instance->sign_msg, RESPONSE_TIMEOUT) ==
       FuriStatusOk) {
        if(instance->sign_msg.cmd == TlsCryptoSignResponse) {
            furi_assert(instance->sign_msg.key_slot == key_slot);
            size_t resp_sign_len = instance->sign_msg.data_size;
            FURI_LOG_E(TAG, "TlsCryptoSignResponse len:%u", resp_sign_len);
            furi_assert(sign_buf_size >= resp_sign_len);
            memcpy(sign_buf, instance->sign_msg.data, resp_sign_len);
            if(sign_len) {
                *sign_len = resp_sign_len;
            }
            success = true;
        } else if(instance->sign_msg.cmd == TlsCryptoError) {
            FURI_LOG_E(TAG, "917 returned error");
            // TODO: Errors handling
        } else {
            furi_crash("Unsupported reponse");
        }
    }

    intercom_set_rx_callback(instance->intercom, IntercomChannelTlsCrypto, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);

    furi_message_queue_free(instance->response_queue);
    free(instance);

    return success;
}

int32_t tls_crypto_client_init(void* arg) {
    UNUSED(arg);
    // TODO: service?
    FURI_LOG_E(TAG, "Hello, world!");
    return 0;
}
