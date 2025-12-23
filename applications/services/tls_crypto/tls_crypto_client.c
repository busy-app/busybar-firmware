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
    TlsCryptoMessageGeneric msg;
    FuriMessageQueue* response_queue;
};

static void tls_crypto_client_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    const TlsCryptoDataMessage* msg = data;
    furi_assert((data_size == sizeof(TlsCryptoMessageHeader) + msg->header.data_size));
    furi_assert(context);
    TlsCryptoClient* instance = context;

    furi_check(
        furi_message_queue_put(instance->response_queue, data, FuriWaitForever) == FuriStatusOk);
}

static TlsCryptoClient* tls_crypto_client_alloc(void) {
    TlsCryptoClient* client = malloc(sizeof(TlsCryptoClient));

    client->response_queue = furi_message_queue_alloc(1, sizeof(TlsCryptoMessageGeneric));

    client->intercom = furi_record_open(RECORD_INTERCOM);
    client->intercom_ch = intercom_channel_open(
        client->intercom, IntercomChannelIdTlsCrypto, tls_crypto_client_rx_callback, client);

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

    client->msg.header.type = TlsCryptoSignRequest;
    client->msg.header.key_slot = key_slot;
    client->msg.header.data_size = hash_len;
    memcpy(client->msg.data, hash, hash_len);

    size_t packet_len = sizeof(TlsCryptoMessageHeader) + hash_len;
    size_t tx_size = intercom_tx(client->intercom_ch, &(client->msg), packet_len, FuriWaitForever);
    furi_check(tx_size == packet_len, "Failed to send data");

    if(furi_message_queue_get(client->response_queue, &client->msg, RESPONSE_TIMEOUT) ==
       FuriStatusOk) {
        if(client->msg.header.type == TlsCryptoSignResponse) {
            furi_assert(client->msg.header.key_slot == key_slot);
            size_t resp_sign_len = client->msg.header.data_size;
            FURI_LOG_D(TAG, "TlsCryptoSignResponse len:%u", resp_sign_len);
            furi_assert(sign_buf_size >= resp_sign_len);
            memcpy(sign_buf, client->msg.data, resp_sign_len);
            if(sign_len) {
                *sign_len = resp_sign_len;
            }
            success = true;
        } else if(client->msg.header.type == TlsCryptoError) {
            FURI_LOG_E(TAG, "917 returned error");
        } else {
            furi_crash("Unsupported response");
        }
    }

    return success;
}

uint8_t* tls_crypto_client_get_cert(TlsCryptoClient* client, uint8_t key_slot, size_t* cert_len) {
    furi_assert(cert_len);

    uint8_t* cert_buf = NULL;

    client->msg.header.type = TlsCryptoCertRequest;
    client->msg.header.key_slot = key_slot;
    client->msg.header.data_size = 0;

    size_t packet_len = sizeof(TlsCryptoMessageHeader);
    size_t tx_size = intercom_tx(client->intercom_ch, &(client->msg), packet_len, FuriWaitForever);
    furi_check(tx_size == packet_len, "Failed to send data");

    if(furi_message_queue_get(client->response_queue, &client->msg, RESPONSE_TIMEOUT) ==
       FuriStatusOk) {
        if(client->msg.header.type == TlsCryptoCertResponse) {
            furi_assert(client->msg.header.key_slot == key_slot);
            size_t resp_cert_len = client->msg.header.data_size;
            FURI_LOG_D(TAG, "TlsCryptoCertResponse len:%u", resp_cert_len);

            cert_buf = malloc(resp_cert_len);
            memcpy(cert_buf, client->msg.data, resp_cert_len);
            *cert_len = resp_cert_len;

        } else if(client->msg.header.type == TlsCryptoError) {
            FURI_LOG_E(TAG, "917 returned error");
        } else {
            furi_crash("Unsupported reponse");
        }
    }

    return cert_buf;
}

int32_t tls_crypto_client_init(void* arg) {
    UNUSED(arg);
    TlsCryptoClient* client = tls_crypto_client_alloc();
    furi_record_create(RECORD_TLS_CRYPTO_CLIENT, client);
    return 0;
}
