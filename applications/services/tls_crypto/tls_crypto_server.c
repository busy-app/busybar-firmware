#include <furi.h>
#include <intercom/intercom.h>
#include <furi_hal_crypto.h>
#include <furi_hal_crypto_storage.h>

#include "tls_crypto_common.h"

#define TAG "TlsCryptoServer"

#define KEY_ID_OFFSET (0x10)
#define KEY_SLOTS_MAX (2)

#define SIGNATURE_LEN_MAX (80)
static_assert(SIGNATURE_LEN_MAX <= TLS_CRYPTO_DATA_SIZE_MAX);

typedef struct {
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
} TlsCryptoServer;

static void tls_crypto_server_sign(TlsCryptoServer* instance, TlsCryptoMessageGeneric* msg) {
    bool success = false;

    uint8_t* signature_buf = malloc(SIGNATURE_LEN_MAX);
    size_t signature_len = SIGNATURE_LEN_MAX;

    do {
        if(msg->header.key_slot >= KEY_SLOTS_MAX) break;
        uint32_t key_id = KEY_ID_OFFSET + msg->header.key_slot;

        FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);
        FuriHalCryptoStatus status =
            furi_hal_crypto_storage_read(key, FuriHalCryptoKeyTypeEcdsaPriv256, key_id);

        if(status == FuriHalCryptoStatusOk) {
            FuriHalCryptoEcdsa* sign_ctx = furi_hal_crypto_ecdsa_sign_init(
                FuriHalCryptoEcdsaModeSha256,
                key->data,
                FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256,
                FuriHalCryptoWrappingModeOff);

            success = furi_hal_crypto_ecdsa_sign(
                sign_ctx, msg->data, msg->header.data_size, signature_buf, &signature_len);

            furi_hal_crypto_ecdsa_deinit(sign_ctx);
        }
        furi_hal_crypto_storage_free(key);

    } while(0);

    if(success) {
        FURI_LOG_D(TAG, "Sign done");
        msg->header.type = TlsCryptoSignResponse;
        msg->header.data_size = signature_len;
        memcpy(msg->data, signature_buf, signature_len);
        size_t packet_len = sizeof(TlsCryptoMessageHeader) + msg->header.data_size;
        size_t tx_size = intercom_tx(instance->intercom_ch, msg, packet_len, FuriWaitForever);
        furi_check(tx_size == packet_len, "Failed to send data");
    } else {
        FURI_LOG_E(TAG, "Sign error");
        TlsCryptoErrorMessage error_msg = {.header = {.type = TlsCryptoError}};
        size_t tx_size = intercom_tx(
            instance->intercom_ch, &error_msg, sizeof(TlsCryptoErrorMessage), FuriWaitForever);
        furi_check(tx_size == sizeof(TlsCryptoErrorMessage), "Failed to send data");
    }
    free(signature_buf);
}

static void tls_crypto_server_get_cert(TlsCryptoServer* instance, TlsCryptoMessageGeneric* msg) {
    bool success = false;

    do {
        if(msg->header.key_slot >= KEY_SLOTS_MAX) break;
        uint32_t key_id = KEY_ID_OFFSET + msg->header.key_slot;

        FuriHalCryptoKey* key = furi_hal_crypto_storage_alloc(FuriHalCryptoPartitionMain);
        FuriHalCryptoStatus status =
            furi_hal_crypto_storage_read(key, FuriHalCryptoKeyTypeCrtDerEcdsa256, key_id);

        if(status == FuriHalCryptoStatusOk) {
            furi_check(key->header.size <= TLS_CRYPTO_DATA_SIZE_MAX);
            msg->header.type = TlsCryptoCertResponse;
            msg->header.data_size = key->header.size;
            memcpy(msg->data, key->data, key->header.size);

            size_t packet_len = sizeof(TlsCryptoMessageHeader) + msg->header.data_size;
            size_t tx_size = intercom_tx(instance->intercom_ch, msg, packet_len, FuriWaitForever);
            furi_check(tx_size == packet_len, "Failed to send data");

            success = true;
        }
        furi_hal_crypto_storage_free(key);
    } while(0);

    if(!success) {
        FURI_LOG_E(TAG, "Get cert error");
        TlsCryptoErrorMessage error_msg = {.header = {.type = TlsCryptoError}};
        size_t tx_size = intercom_tx(
            instance->intercom_ch, &error_msg, sizeof(TlsCryptoErrorMessage), FuriWaitForever);
        furi_check(tx_size == sizeof(TlsCryptoErrorMessage), "Failed to send data");
    }
}

static void tls_crypto_server_message_callback(FuriEventLoopObject* object, void* context) {
    TlsCryptoServer* instance = context;
    furi_check(instance);
    furi_check(object == instance->command_queue);

    TlsCryptoMessageGeneric* msg = malloc(sizeof(TlsCryptoMessageGeneric));
    furi_check(furi_message_queue_get(instance->command_queue, msg, 0) == FuriStatusOk);

    if(msg->header.type == TlsCryptoSignRequest) {
        tls_crypto_server_sign(instance, msg);
    } else if(msg->header.type == TlsCryptoCertRequest) {
        tls_crypto_server_get_cert(instance, msg);
    }

    free(msg);
}

static void tls_crypto_server_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    const TlsCryptoDataMessage* msg = data;
    furi_assert((data_size == sizeof(TlsCryptoMessageHeader) + msg->header.data_size));
    furi_assert(context);
    TlsCryptoServer* instance = context;

    furi_check(
        furi_message_queue_put(instance->command_queue, data, FuriWaitForever) == FuriStatusOk);
}

int32_t tls_crypto_server_init(void* arg) {
    UNUSED(arg);
    TlsCryptoServer* instance = malloc(sizeof(TlsCryptoServer));
    instance->event_loop = furi_event_loop_alloc();
    instance->command_queue = furi_message_queue_alloc(1, sizeof(TlsCryptoMessageGeneric));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        tls_crypto_server_message_callback,
        instance);

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        instance->intercom, IntercomChannelIdTlsCrypto, tls_crypto_server_rx_callback, instance);

    FURI_LOG_I(TAG, "Start");

    furi_event_loop_run(instance->event_loop);

    return 0;
}
