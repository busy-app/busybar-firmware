#include "tls_crypto.h"

#include <furi.h>
#include <intercom/intercom.h>

#include "tls_crypto_common.h"

#define TAG "TlsCrypto"

#define RESPONSE_TIMEOUT 200
#define ACQUIRE_TIMEOUT  500

struct TlsCrypto {
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    TlsCryptoMessageGeneric msg;
    FuriMessageQueue* response_queue;
    FuriSemaphore* api_semaphore;
};

static void tls_crypto_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    const TlsCryptoDataMessage* msg = data;
    furi_assert((data_size == sizeof(TlsCryptoMessageHeader) + msg->header.data_size));
    furi_assert(context);
    TlsCrypto* instance = context;

    furi_check(
        furi_message_queue_put(instance->response_queue, data, FuriWaitForever) == FuriStatusOk);
}

static TlsCrypto* tls_crypto_alloc(void) {
    TlsCrypto* instance = malloc(sizeof(TlsCrypto));

    instance->response_queue = furi_message_queue_alloc(1, sizeof(TlsCryptoMessageGeneric));
    instance->api_semaphore = furi_semaphore_alloc(1, 1);

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        instance->intercom, IntercomChannelIdTlsCrypto, tls_crypto_rx_callback, instance);

    return instance;
}

bool tls_crypto_sign(
    TlsCrypto* instance,
    uint8_t key_slot,
    const void* data,
    size_t data_size,
    void* signature_buf,
    size_t signature_buf_size,
    size_t* signature_len) {
    furi_assert(data);
    furi_assert(signature_buf);
    furi_assert(data_size <= TLS_CRYPTO_DATA_SIZE_MAX);

    furi_check(furi_semaphore_acquire(instance->api_semaphore, FuriWaitForever) == FuriStatusOk);

    bool success = false;

    instance->msg.header.type = TlsCryptoSignRequest;
    instance->msg.header.key_slot = key_slot;
    instance->msg.header.data_size = data_size;
    memcpy(instance->msg.data, data, data_size);

    size_t packet_len = sizeof(TlsCryptoMessageHeader) + data_size;
    size_t tx_size =
        intercom_tx(instance->intercom_ch, &(instance->msg), packet_len, FuriWaitForever);
    furi_check(tx_size == packet_len, "Failed to send data");

    if(furi_message_queue_get(instance->response_queue, &instance->msg, RESPONSE_TIMEOUT) ==
       FuriStatusOk) {
        if(instance->msg.header.type == TlsCryptoSignResponse) {
            furi_assert(instance->msg.header.key_slot == key_slot);
            size_t resp_sign_len = instance->msg.header.data_size;
            FURI_LOG_D(TAG, "TlsCryptoSignResponse len:%u", resp_sign_len);
            furi_assert(signature_buf_size >= resp_sign_len);
            memcpy(signature_buf, instance->msg.data, resp_sign_len);
            if(signature_len) {
                *signature_len = resp_sign_len;
            }
            success = true;
        } else if(instance->msg.header.type == TlsCryptoError) {
            FURI_LOG_E(TAG, "917 returned error");
        } else {
            furi_crash("Unsupported response");
        }
    }

    furi_semaphore_release(instance->api_semaphore);

    return success;
}

uint8_t* tls_crypto_get_certificate(TlsCrypto* instance, uint8_t key_slot, size_t* cert_len) {
    furi_assert(cert_len);

    furi_check(furi_semaphore_acquire(instance->api_semaphore, FuriWaitForever) == FuriStatusOk);

    uint8_t* cert_buf = NULL;

    instance->msg.header.type = TlsCryptoCertRequest;
    instance->msg.header.key_slot = key_slot;
    instance->msg.header.data_size = 0;

    size_t packet_len = sizeof(TlsCryptoMessageHeader);
    size_t tx_size =
        intercom_tx(instance->intercom_ch, &(instance->msg), packet_len, FuriWaitForever);
    furi_check(tx_size == packet_len, "Failed to send data");

    if(furi_message_queue_get(instance->response_queue, &instance->msg, RESPONSE_TIMEOUT) ==
       FuriStatusOk) {
        if(instance->msg.header.type == TlsCryptoCertResponse) {
            furi_assert(instance->msg.header.key_slot == key_slot);
            size_t resp_cert_len = instance->msg.header.data_size;
            FURI_LOG_D(TAG, "TlsCryptoCertResponse len:%u", resp_cert_len);

            cert_buf = malloc(resp_cert_len);
            memcpy(cert_buf, instance->msg.data, resp_cert_len);
            *cert_len = resp_cert_len;

        } else if(instance->msg.header.type == TlsCryptoError) {
            FURI_LOG_E(TAG, "917 returned error");
        } else {
            furi_crash("Unsupported reponse");
        }
    }

    furi_semaphore_release(instance->api_semaphore);

    return cert_buf;
}

void tls_crypto_startup(void) {
    TlsCrypto* instance = tls_crypto_alloc();
    furi_record_create(RECORD_TLS_CRYPTO, instance);
}
