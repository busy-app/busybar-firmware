#include <furi.h>
#include <intercom/intercom.h>
#include <mbedtls/library/pk_wrap.h>
#include <furi_hal_crypto.h>
#include <furi_hal_crypto_storage.h>

#include "tls_crypto_common.h"

#define TAG "TlsCryptoServer"

typedef struct {
    Intercom* intercom;
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
} TlsCryptoServer;

static const char priv_key[] = "-----BEGIN EC PRIVATE KEY-----\n"
                               "MHcCAQEEIB7FooM2FmMvHm2eO/bkpAf+4cOpu/pledOBmU7IuF+5oAoGCCqGSM49\n"
                               "AwEHoUQDQgAEV06XdA1WR0mDHEGd4EVl+aBcO8uKG51nawvLS+JWlBpvJyLs1wC3\n"
                               "gCI0cTPazzzx5PhoH7rzE44q9v4MvI6QIg==\n"
                               "-----END EC PRIVATE KEY-----\n";

static int tls_crypto_random(void* ctx, unsigned char* buf, size_t len) {
    UNUSED(ctx);
    FuriHalCryptoStatus status = furi_hal_crypto_storage_gen_random_buf(buf, len);
    return (status == FuriHalCryptoStatusOk) ? 0 : 1;
}

static void tls_crypto_server_sign(
    TlsCryptoServer* instance,
    uint8_t key_slot,
    uint8_t* hash,
    size_t hash_len) {
    furi_check(instance);

    TlsCryptoSignMessage* sign_resp = malloc(sizeof(TlsCryptoSignMessage));
    sign_resp->cmd = TlsCryptoSignResponse;
    sign_resp->key_slot = key_slot;

    int ret = 0;
    bool success = false;
    mbedtls_pk_context sign_pk;
    mbedtls_pk_init(&sign_pk);
    do {
        ret = mbedtls_pk_parse_key(
            &sign_pk, (uint8_t*)priv_key, sizeof(priv_key), NULL, 0, tls_crypto_random, NULL);
        if(ret != 0) {
            FURI_LOG_E(TAG, "Key load err -%04X", -ret);
            break;
        }
        ret = mbedtls_pk_sign(
            &sign_pk,
            MBEDTLS_MD_SHA256,
            hash,
            hash_len,
            sign_resp->data,
            sizeof(sign_resp->data),
            &sign_resp->data_size,
            tls_crypto_random,
            NULL);
        if(ret != 0) {
            FURI_LOG_E(TAG, "Sign err -%04X", -ret);
            break;
        }
        success = true;
    } while(0);
    mbedtls_pk_free(&sign_pk);

    if(success) {
        size_t tx_size = intercom_tx(
            instance->intercom,
            IntercomChannelTlsCrypto,
            sign_resp,
            sizeof(TlsCryptoSignMessage),
            FuriWaitForever);
        furi_check(tx_size == sizeof(TlsCryptoSignMessage), "Failed to send data");
    } else {
        TlsCryptoErrorMessage error_msg = {.cmd = TlsCryptoError};
        size_t tx_size = intercom_tx(
            instance->intercom,
            IntercomChannelTlsCrypto,
            &error_msg,
            sizeof(TlsCryptoErrorMessage),
            FuriWaitForever);
        furi_check(tx_size == sizeof(TlsCryptoErrorMessage), "Failed to send data");
    }

    free(sign_resp);
}

static void tls_crypto_server_message_callback(FuriEventLoopObject* object, void* context) {
    TlsCryptoServer* instance = context;
    furi_check(object == instance->command_queue);

    TlsCryptoSignMessage msg;
    furi_check(furi_message_queue_get(instance->command_queue, &msg, 0) == FuriStatusOk);

    if(msg.cmd == TlsCryptoSignRequest) {
        tls_crypto_server_sign(instance, msg.key_slot, msg.data, msg.data_size);
    }
}

static void tls_crypto_server_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(data_size == sizeof(TlsCryptoSignMessage));
    furi_assert(context);
    TlsCryptoServer* instance = context;

    furi_check(
        furi_message_queue_put(instance->command_queue, data, FuriWaitForever) == FuriStatusOk);
}

int32_t tls_crypto_server_init(void* arg) {
    UNUSED(arg);
    TlsCryptoServer* instance = malloc(sizeof(TlsCryptoServer));
    instance->event_loop = furi_event_loop_alloc();
    instance->command_queue = furi_message_queue_alloc(2, sizeof(TlsCryptoSignMessage));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        tls_crypto_server_message_callback,
        instance);

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelTlsCrypto, tls_crypto_server_rx_callback, instance);

    FURI_LOG_I(TAG, "Start");

    furi_event_loop_run(instance->event_loop);

    return 0;
}
