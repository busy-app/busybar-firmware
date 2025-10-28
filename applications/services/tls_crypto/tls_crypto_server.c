#include <furi.h>
#include <intercom/intercom.h>
#include <furi_hal_crypto.h>
#include <furi_hal_crypto_storage.h>

#include "tls_crypto_common.h"

#define TAG "TlsCryptoServer"

#define KEY_ID_OFFSET 0x10

typedef struct {
    Intercom* intercom;
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
} TlsCryptoServer;

static void tls_crypto_server_sign(
    TlsCryptoServer* instance,
    uint8_t key_slot,
    uint8_t* hash,
    size_t hash_len) {
    furi_check(instance);

    bool success = false;

    TlsCryptoSignMessage* sign_resp = malloc(sizeof(TlsCryptoSignMessage));
    sign_resp->cmd = TlsCryptoSignResponse;
    sign_resp->key_slot = key_slot;

    uint32_t key_id = KEY_ID_OFFSET + key_slot;
    FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);
    FuriHalCryptoStatus status =
        furi_hal_crypto_storage_read(key, FuriHalCryptoKeyTypeEcdsaPriv256, key_id);

    if(status == FuriHalCryptoStatusOk) {
        FuriHalCryptoEcdsa* sign_ctx = furi_hal_crypto_ecdsa_sign_init(
            FuriHalCryptoEcdsaModeSha256,
            key->data,
            FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
            FuriHalCryptoWrappingModeOff);

        size_t signature_len = sizeof(sign_resp->data);
        success =
            furi_hal_crypto_ecdsa_sign(sign_ctx, hash, hash_len, sign_resp->data, &signature_len);
        sign_resp->data_size = signature_len;

        furi_hal_crypto_ecdsa_deinit(sign_ctx);
    }
    furi_hal_crypto_storage_free(key);

    if(success) {
        FURI_LOG_D(TAG, "Sign done");
        size_t tx_size = intercom_tx(
            instance->intercom,
            IntercomChannelTlsCrypto,
            sign_resp,
            sizeof(TlsCryptoSignMessage),
            FuriWaitForever);
        furi_check(tx_size == sizeof(TlsCryptoSignMessage), "Failed to send data");
    } else {
        FURI_LOG_E(TAG, "Sign error");
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
