#include "crypto_aes_enclave.h"

#include <furi_hal_crypto_storage.h>
#include <cli/cli_ansi.h>

#define TAG "CryptoAESEnclave"

static const uint32_t aes_enclave_key_ids[] = {0x00, 0x01, 0x02, 0x03, 0x04};

void crypto_aes_enclave_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    const uint8_t input[16] = {0};

    printf("AES-256-ECB vectors for enclave keys, plaintext: 16x0x00\r\n");

    for(size_t i = 0; i < COUNT_OF(aes_enclave_key_ids); i++) {
        uint32_t key_id = aes_enclave_key_ids[i];

        FuriHalCryptoKey* key = NULL;
        FuriHalCryptoStatus status = furi_hal_crypto_storage_read(
            &key, FuriHalCryptoPartitionMain, FuriHalCryptoKeyTypeAes256, key_id);

        if(status != FuriHalCryptoStatusOk) {
            printf(
                ANSI_FG_RED "key 0x%02x: read failed (status %u)\r\n" ANSI_RESET,
                (unsigned)key_id,
                (unsigned)status);
            continue;
        }

        FuriHalCryptoAes* aes = NULL;
        status = furi_hal_crypto_aes_init(&aes, FuriHalCryptoAesModeECB, key);
        furi_hal_crypto_key_free(key);

        if(status != FuriHalCryptoStatusOk) {
            printf(
                ANSI_FG_RED "key 0x%02x: init failed (status %u)\r\n" ANSI_RESET,
                (unsigned)key_id,
                (unsigned)status);
            continue;
        }

        uint8_t output[16] = {0};
        status = furi_hal_crypto_aes_encrypt(aes, NULL, input, sizeof(input), output);
        furi_hal_crypto_aes_deinit(aes);

        if(status != FuriHalCryptoStatusOk) {
            printf(
                ANSI_FG_RED "key 0x%02x: encrypt failed (status %u)\r\n" ANSI_RESET,
                (unsigned)key_id,
                (unsigned)status);
            continue;
        }

        printf("key 0x%02x:", (unsigned)key_id);
        for(size_t b = 0; b < sizeof(output); b++) {
            printf(" %02x", output[b]);
        }
        printf("\r\n");
    }
}
