#include "tls_crypto.h"

#include <furi.h>
#include <intercom/intercom.h>

#include "tls_crypto_common_i.h"

#define TAG "TlsCrypto"

#define TLS_CRYPTO_RESPONSE_TIMEOUT_MS (500)

struct TlsCrypto {
    FuriEventFlag* flags;
    IntercomChannel* intercom_ch;
    TlsCryptoRequest request;
    TlsCryptoResponse response;
};

typedef enum {
    TlsCryptoEventFlagReady = 1UL << 0,
    TlsCryptoEventFlagResponse = 1UL << 1,
} TlsCryptoEventFlag;

static void tls_crypto_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(context);

    furi_check(data_size == sizeof(TlsCryptoResponse));

    TlsCrypto* instance = context;
    const TlsCryptoResponse* response = data;

    instance->response = *response;
    furi_check(
        furi_event_flag_set(instance->flags, TlsCryptoEventFlagResponse) ==
        TlsCryptoEventFlagResponse);
}

static void tls_crypto_send_request(const TlsCrypto* instance) {
    const size_t tx_size = intercom_tx(
        instance->intercom_ch, &instance->request, sizeof(instance->request), FuriWaitForever);
    furi_check(tx_size == sizeof(instance->request), "Failed to send data");
}

static TlsCryptoStatus tls_crypto_wait_for_response(const TlsCrypto* instance) {
    TlsCryptoStatus status;

    const uint32_t flags = furi_event_flag_wait(
        instance->flags,
        TlsCryptoEventFlagResponse,
        FuriFlagWaitAny,
        TLS_CRYPTO_RESPONSE_TIMEOUT_MS);

    if(flags == TlsCryptoEventFlagResponse) {
        status = instance->response.status;
    } else if(flags == FuriFlagErrorTimeout) {
        status = TlsCryptoStatusErrorTimeout;
    } else {
        furi_crash();
    }

    return status;
}

static void tls_crypto_build_get_cert_request(TlsCrypto* instance, TlsCryptoKeyId key_id) {
    TlsCryptoRequest* request = &instance->request;
    request->type = TlsCryptoRequestTypeGetCertificate;

    TlsCryptoRequestGetCertificate* get_cert_request = &request->get_cert;
    get_cert_request->key_id = key_id;
}

static void tls_crypto_build_sign_request(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    const void* data,
    size_t data_len) {
    TlsCryptoRequest* request = &instance->request;
    request->type = TlsCryptoRequestTypeSign;

    TlsCryptoRequestSign* sign_request = &request->sign;
    sign_request->key_id = key_id;
    sign_request->length = data_len;
    memcpy(sign_request->data, data, data_len);
}

static void tls_crypto_api_acquire(const TlsCrypto* instance) {
    furi_check(
        furi_event_flag_wait(
            instance->flags, TlsCryptoEventFlagReady, FuriFlagWaitAny, FuriWaitForever) ==
        TlsCryptoEventFlagReady);
}

static void tls_crypto_api_release(const TlsCrypto* instance) {
    furi_check(
        furi_event_flag_set(instance->flags, TlsCryptoEventFlagReady) == TlsCryptoEventFlagReady);
}

static TlsCrypto* tls_crypto_alloc(void) {
    TlsCrypto* instance = malloc(sizeof(TlsCrypto));

    instance->flags = furi_event_flag_alloc();

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        intercom, IntercomChannelIdTlsCrypto, tls_crypto_intercom_rx_callback, instance);

    return instance;
}

TlsCryptoStatus tls_crypto_get_certificate(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    TlsCryptoCertificate* certificate) {
    furi_check(instance);
    furi_check(key_id < TlsCryptoKeyIdMax);
    furi_check(certificate);

    tls_crypto_api_acquire(instance);

    tls_crypto_build_get_cert_request(instance, key_id);
    tls_crypto_send_request(instance);

    const TlsCryptoStatus status = tls_crypto_wait_for_response(instance);

    if(status == TlsCryptoStatusOk) {
        const TlsCryptoResponseGetCertificate* get_cert_response = &instance->response.get_cert;
        *certificate = get_cert_response->certificate;
    }

    tls_crypto_api_release(instance);

    return status;
}

TlsCryptoStatus tls_crypto_sign(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    const void* data,
    size_t data_len,
    TlsCryptoSignature* signature) {
    furi_check(instance);
    furi_check(key_id < TlsCryptoKeyIdMax);
    furi_check(data);
    furi_check(data_len > 0 && data_len <= TLS_CRYPTO_DATA_LEN_MAX);
    furi_check(signature);

    tls_crypto_api_acquire(instance);

    tls_crypto_build_sign_request(instance, key_id, data, data_len);
    tls_crypto_send_request(instance);

    const TlsCryptoStatus status = tls_crypto_wait_for_response(instance);

    if(status == TlsCryptoStatusOk) {
        const TlsCryptoResponseSign* sign_response = &instance->response.sign;
        *signature = sign_response->signature;
    }

    tls_crypto_api_release(instance);

    return status;
}

void tls_crypto_startup(void) {
    TlsCrypto* instance = tls_crypto_alloc();
    furi_record_create(RECORD_TLS_CRYPTO, instance);
}
