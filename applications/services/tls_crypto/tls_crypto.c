#include "tls_crypto_i.h"

#define TLS_CRYPTO_GENERAL_TIMEOUT_MS  (1000)
#define TLS_CRYPTO_RESPONSE_TIMEOUT_MS (100)

static void tls_crypto_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(context);

    TlsCrypto* instance = context;
    furi_check(data_size == sizeof(TlsCryptoResponse));

    const FuriStatus status =
        furi_message_queue_put(instance->response_queue, data, TLS_CRYPTO_RESPONSE_TIMEOUT_MS);
    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        FURI_LOG_W(TAG, "Dropping response");
    }
}

static void
    tls_crypto_wait_for_api_message(const TlsCrypto* instance, TlsCryptoApiMessage* api_message) {
    const FuriStatus status =
        furi_message_queue_get(instance->api_queue, api_message, FuriWaitForever);
    furi_check(status == FuriStatusOk);
}

static void tls_crypto_build_get_cert_request(
    TlsCryptoRequestGetCertificate* request,
    const TlsCryptoApiMessageGetCertificate* api_message) {
    request->key_id = api_message->key_id;
}

static void tls_crypto_build_sign_message_request(
    TlsCryptoRequestSignMessage* request,
    const TlsCryptoApiMessageSignMessage* api_message) {
    request->key_id = api_message->key_id;

    TlsCryptoMessage* message = &request->message;
    memcpy(message->bytes, api_message->data, api_message->data_len);
    message->length = api_message->data_len;
}

static void tls_crypto_build_request(
    TlsCrypto* instance,
    TlsCryptoRequest* request,
    const TlsCryptoApiMessage* api_message) {
    TlsCryptoRequestType request_type;
    const TlsCryptoApiMessageType api_message_type = api_message->type;

    if(api_message_type == TlsCryptoApiMessageTypeGetCertificate) {
        request_type = TlsCryptoRequestTypeGetCertificate;
        tls_crypto_build_get_cert_request(&request->get_cert, &api_message->get_cert);

    } else if(api_message_type == TlsCryptoApiMessageTypeSignMessage) {
        request_type = TlsCryptoRequestTypeSignMessage;
        tls_crypto_build_sign_message_request(&request->sign_message, &api_message->sign_message);

    } else {
        furi_crash("Invalid TlsCryptoApiMessageType value");
    }

    request->type = request_type;
    request->id = instance->current_request_id;

    ++instance->current_request_id;
}

static bool tls_crypto_send_request(const TlsCrypto* instance, const TlsCryptoRequest* request) {
    const size_t tx_size = intercom_tx(
        instance->intercom_ch, request, sizeof(TlsCryptoRequest), TLS_CRYPTO_GENERAL_TIMEOUT_MS);
    return (tx_size == sizeof(TlsCryptoRequest));
}

static bool tls_crypto_wait_for_response_to_request(
    const TlsCrypto* instance,
    const TlsCryptoRequest* request,
    TlsCryptoResponse* response) {
    bool success;

    for(;;) {
        const FuriStatus status = furi_message_queue_get(
            instance->response_queue, response, TLS_CRYPTO_GENERAL_TIMEOUT_MS);

        if(status == FuriStatusOk) {
            if((response->id == request->id) && (response->type == request->type)) {
                success = true;
                break;
            }

        } else {
            furi_check(status == FuriStatusErrorTimeout);
            success = false;
            break;
        }
    }

    return success;
}

static void tls_crypto_process_get_cert_response(
    const TlsCryptoResponseGetCertificate* response,
    TlsCryptoApiMessageGetCertificate* api_message) {
    *api_message->certificate = response->certificate;
}

static void tls_crypto_process_sign_message_response(
    const TlsCryptoResponseSignMessage* response,
    TlsCryptoApiMessageSignMessage* api_message) {
    *api_message->signature = response->signature;
}

static void tls_crypto_process_response(
    const TlsCryptoResponse* response,
    TlsCryptoApiMessage* api_message) {
    const TlsCryptoRequestType response_type = response->type;

    if(response_type == TlsCryptoRequestTypeGetCertificate) {
        furi_assert(api_message->type == TlsCryptoApiMessageTypeGetCertificate);
        tls_crypto_process_get_cert_response(&response->get_cert, &api_message->get_cert);

    } else if(response_type == TlsCryptoRequestTypeSignMessage) {
        furi_assert(api_message->type == TlsCryptoApiMessageTypeSignMessage);
        tls_crypto_process_sign_message_response(
            &response->sign_message, &api_message->sign_message);

    } else {
        furi_crash("Invalid TlsCryptoRequestType value");
    }

    *api_message->status = response->status;
}

static void tls_crypto_run(TlsCrypto* instance) {
    TlsCryptoApiMessage api_message;
    tls_crypto_wait_for_api_message(instance, &api_message);

    do {
        TlsCryptoRequest request;
        tls_crypto_build_request(instance, &request, &api_message);

        if(!tls_crypto_send_request(instance, &request)) {
            break;
        }

        TlsCryptoResponse response;
        if(!tls_crypto_wait_for_response_to_request(instance, &request, &response)) {
            break;
        }

        tls_crypto_process_response(&response, &api_message);

        tls_crypto_log_response_status(&response);

    } while(false);

    if(api_message.lock) {
        api_lock_unlock(api_message.lock);
    }
}

static TlsCrypto* tls_crypto_alloc(void) {
    TlsCrypto* instance = malloc(sizeof(TlsCrypto));
    instance->api_queue = furi_message_queue_alloc(1, sizeof(TlsCryptoApiMessage));
    instance->response_queue = furi_message_queue_alloc(1, sizeof(TlsCryptoResponse));

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        intercom, IntercomChannelIdTlsCrypto, tls_crypto_intercom_rx_callback, instance);

    furi_record_create(RECORD_TLS_CRYPTO, instance);

    return instance;
}

static TlsCryptoStatus
    tls_crypto_send_api_message(TlsCrypto* instance, TlsCryptoApiMessage* message) {
    TlsCryptoStatus status = TlsCryptoStatusErrorTimeout;

    message->status = &status;
    message->lock = api_lock_alloc_locked();

    const FuriStatus api_status =
        furi_message_queue_put(instance->api_queue, message, TLS_CRYPTO_GENERAL_TIMEOUT_MS);

    if(api_status != FuriStatusOk) {
        furi_check(api_status == FuriStatusErrorTimeout);
        api_lock_unlock(message->lock);
    }

    api_lock_wait_unlock_and_free(message->lock);

    return status;
}

TlsCryptoStatus tls_crypto_get_certificate(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    TlsCryptoCertificate* certificate) {
    furi_check(instance);
    furi_check(key_id < TlsCryptoKeyIdMax);
    furi_check(certificate);

    TlsCryptoApiMessage api_message = {
        .type = TlsCryptoApiMessageTypeGetCertificate,
        .get_cert =
            {
                .key_id = key_id,
                .certificate = certificate,
            },
    };

    return tls_crypto_send_api_message(instance, &api_message);
}

TlsCryptoStatus tls_crypto_sign(
    TlsCrypto* instance,
    TlsCryptoKeyId key_id,
    const void* message,
    size_t message_len,
    TlsCryptoSignature* signature) {
    furi_check(instance);
    furi_check(key_id < TlsCryptoKeyIdMax);
    furi_check(message);
    furi_check(message_len > 0 && message_len <= TLS_CRYPTO_DATA_LEN_MAX);
    furi_check(signature);

    TlsCryptoApiMessage api_message = {
        .type = TlsCryptoApiMessageTypeSignMessage,
        .sign_message =
            {
                .key_id = key_id,
                .data = message,
                .data_len = message_len,
                .signature = signature,
            },
    };

    return tls_crypto_send_api_message(instance, &api_message);
}

int32_t tls_crypto_srv(void* arg) {
    UNUSED(arg);

    TlsCrypto* instance = tls_crypto_alloc();

    for(;;) {
        tls_crypto_run(instance);
    }
}
