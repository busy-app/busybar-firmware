#include <furi.h>

#include <furi_hal_crypto.h>
#include <furi_hal_crypto_storage.h>

#include <intercom/intercom.h>

#include "tls_crypto_common_i.h"

#define KEY_ID_OFFSET (0x10)

typedef struct {
    FuriEventLoop* event_loop;
    IntercomChannel* intercom_ch;
    TlsCryptoRequest request;
    TlsCryptoResponse response;
} TlsCrypto;

typedef enum {
    TlsCryptoCustomEventRequest = 1UL << 0,
} TlsCryptoCustomEvent;

typedef TlsCryptoStatus (
    *TlsCryptoRequestHandler)(const TlsCryptoRequest* request, TlsCryptoResponse* response);

static const TlsCryptoRequestHandler tls_crypto_request_handlers[];

static void tls_crypto_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(context);

    furi_check(data_size == sizeof(TlsCryptoRequest));

    TlsCrypto* instance = context;
    const TlsCryptoRequest* request = data;

    instance->request = *request;
    furi_event_loop_set_custom_event(instance->event_loop, TlsCryptoCustomEventRequest);
}

static TlsCryptoStatus tls_crypto_sign_message_request_handler(
    const TlsCryptoRequest* request,
    TlsCryptoResponse* response) {
    TlsCryptoStatus status = TlsCryptoStatusErrorInternal;

    const TlsCryptoRequestSignMessage* sign_message_request = &request->sign_message;
    TlsCryptoResponseSignMessage* sign_message_response = &response->sign_message;

    const uint32_t internal_key_id = (uint32_t)sign_message_request->key_id + KEY_ID_OFFSET;

    FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);
    FuriHalCryptoStatus hal_status =
        furi_hal_crypto_storage_read(key, FuriHalCryptoKeyTypeEcdsaPriv256, internal_key_id);

    if(hal_status == FuriHalCryptoStatusOk) {
        FuriHalCryptoEcdsa* sign_ctx = furi_hal_crypto_ecdsa_sign_init(
            FuriHalCryptoEcdsaModeSha256,
            key->data,
            FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
            FuriHalCryptoWrappingModeOff);

        const TlsCryptoMessage* message = &sign_message_request->message;
        TlsCryptoSignature* signature = &sign_message_response->signature;

        const bool sign_success = furi_hal_crypto_ecdsa_sign(
            sign_ctx, message->bytes, message->length, signature->bytes, &signature->length);

        furi_hal_crypto_ecdsa_deinit(sign_ctx);

        if(sign_success) {
            status = TlsCryptoStatusOk;
        }
    }

    furi_hal_crypto_storage_free(key);

    return status;
}

static TlsCryptoStatus tls_crypto_get_certificate_request_handler(
    const TlsCryptoRequest* request,
    TlsCryptoResponse* response) {
    TlsCryptoStatus status = TlsCryptoStatusErrorInternal;

    const TlsCryptoRequestGetCertificate* get_cert_request = &request->get_cert;
    TlsCryptoResponseGetCertificate* get_cert_response = &response->get_cert;

    const uint32_t internal_key_id = (uint32_t)get_cert_request->key_id + KEY_ID_OFFSET;

    FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);
    FuriHalCryptoStatus hal_status =
        furi_hal_crypto_storage_read(key, FuriHalCryptoKeyTypeCrtDerEcdsa256, internal_key_id);

    if(hal_status == FuriHalCryptoStatusOk) {
        const size_t data_len = key->header.size;
        furi_check(data_len <= TLS_CRYPTO_DATA_LEN_MAX);

        TlsCryptoCertificate* certificate = &get_cert_response->certificate;
        memcpy(certificate->bytes, key->data, data_len);
        certificate->length = data_len;

        status = TlsCryptoStatusOk;
    }

    furi_hal_crypto_storage_free(key);

    return status;
}

static void tls_crypto_send_response(const TlsCrypto* instance) {
    const TlsCryptoResponse* response = &instance->response;
    const size_t tx_size =
        intercom_tx(instance->intercom_ch, response, sizeof(TlsCryptoResponse), FuriWaitForever);
    furi_check(tx_size == sizeof(TlsCryptoResponse));
}

static void tls_crypto_handle_request(TlsCrypto* instance) {
    const TlsCryptoRequest* request = &instance->request;
    const TlsCryptoRequestType request_type = request->type;
    furi_check(request_type < TlsCryptoRequestTypeMax);

    TlsCryptoResponse* response = &instance->response;

    response->type = request_type;
    response->id = request->id;
    response->status = tls_crypto_request_handlers[request_type](request, response);

    tls_crypto_log_response_status(response);

    tls_crypto_send_response(instance);
}

static void tls_crypto_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    TlsCrypto* instance = context;

    if(events & TlsCryptoCustomEventRequest) {
        tls_crypto_handle_request(instance);
    }
}

static TlsCrypto* tls_crypto_alloc(void) {
    TlsCrypto* instance = malloc(sizeof(TlsCrypto));

    instance->event_loop = furi_event_loop_alloc();
    furi_event_loop_set_custom_event_callback(
        instance->event_loop, tls_crypto_custom_event_callback, instance);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        intercom, IntercomChannelIdTlsCrypto, tls_crypto_intercom_rx_callback, instance);

    return instance;
}

int32_t tls_crypto_srv(void* arg) {
    UNUSED(arg);

    TlsCrypto* instance = tls_crypto_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const TlsCryptoRequestHandler tls_crypto_request_handlers[TlsCryptoRequestTypeMax] = {
    [TlsCryptoRequestTypeGetCertificate] = tls_crypto_get_certificate_request_handler,
    [TlsCryptoRequestTypeSignMessage] = tls_crypto_sign_message_request_handler,
};
