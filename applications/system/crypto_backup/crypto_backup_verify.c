#include "crypto_backup_verify.h"

#include <furi_hal_crypto_storage.h>

#define TAG "CryptoBackupVerify"

typedef struct {
    FuriHalCryptoKeyType type;
    uint32_t key_id;
} CryptoBackupVerifyKeySetup;

static const CryptoBackupVerifyKeySetup crypto_backup_verify_key_setups[] = {
    /* validated by AES verification procedure
    {.type = FuriHalCryptoKeyTypeAes256, .key_id = 0x00},
    {.type = FuriHalCryptoKeyTypeAes256, .key_id = 0x01},
    {.type = FuriHalCryptoKeyTypeAes256, .key_id = 0x02},
    {.type = FuriHalCryptoKeyTypeAes256, .key_id = 0x03},
    {.type = FuriHalCryptoKeyTypeAes256, .key_id = 0x04},
    */

    {.type = FuriHalCryptoKeyTypeAes256, .key_id = 0x05},
    {.type = FuriHalCryptoKeyTypeAes256, .key_id = 0x06},
    {.type = FuriHalCryptoKeyTypeEcdsaPriv256, .key_id = 0x00},
    {.type = FuriHalCryptoKeyTypeEcdsaPriv256, .key_id = 0x01},
    {.type = FuriHalCryptoKeyTypeEcdsaPriv256, .key_id = 0x02},
    {.type = FuriHalCryptoKeyTypeEcdsaPriv256, .key_id = 0x11},
    {.type = FuriHalCryptoKeyTypeEcdsaPub256, .key_id = 0x11},
    {.type = FuriHalCryptoKeyTypeCsrDerEcdsa256, .key_id = 0x11},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x00},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x01},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x02},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x03},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x04},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x10},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x11},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x14},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x15},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x16},
    {.type = FuriHalCryptoKeyTypeCrtDerEcdsa256, .key_id = 0x17},
    {.type = FuriHalCryptoKeyTypeMatterAttestation, .key_id = 0x00},
    {.type = FuriHalCryptoKeyTypeMatterAttestation, .key_id = 0x01},
    {.type = FuriHalCryptoKeyTypeMatterAttestation, .key_id = 0x02},
    {.type = FuriHalCryptoKeyTypeMatterSetup, .key_id = 0x00},
    {.type = FuriHalCryptoKeyTypeMatterSetup, .key_id = 0x01},
    {.type = FuriHalCryptoKeyTypeMatterSetup, .key_id = 0x02},
    {.type = FuriHalCryptoKeyTypeMatterSetup, .key_id = 0x03},
    {.type = FuriHalCryptoKeyTypeMatterSetup, .key_id = 0x04},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x00},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x01},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x02},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x03},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x04},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x05},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x06},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x07},
    {.type = FuriHalCryptoKeyTypeMatterDeviceInfo, .key_id = 0x08},
};

static const uint8_t crypto_backup_verify_aes_input[] = {0x00};

typedef struct {
    uint32_t key_id;
    uint8_t expected_output[COUNT_OF(crypto_backup_verify_aes_input)];
} CryptoBackupVerifyAesSetup;

static const CryptoBackupVerifyAesSetup crypto_backup_verify_aes_setups[] = {
    {
        .key_id = 0x00,
        .expected_output = {0x00},
    },
    {
        .key_id = 0x01,
        .expected_output = {0x00},
    },
    {
        .key_id = 0x02,
        .expected_output = {0x00},
    },
    {
        .key_id = 0x03,
        .expected_output = {0x00},
    },
    {
        .key_id = 0x04,
        .expected_output = {0x00},
    },
};

static bool crypto_backup_verify_keys(void) {
    FuriHalCryptoKeyIter iter = furi_hal_crypto_key_iter_init(FuriHalCryptoPartitionMain);
    FuriHalCryptoKey* key;
    FuriHalCryptoKeySlot slot;

    size_t found_keys_count = 0;
    while(furi_hal_crypto_key_iter_get_and_advance(&iter, &key, &slot) == FuriHalCryptoStatusOk) {
        for(size_t i = 0; i < COUNT_OF(crypto_backup_verify_key_setups); i++) {
            const CryptoBackupVerifyKeySetup* setup = &crypto_backup_verify_key_setups[i];

            if(slot.header.type == setup->type && slot.header.id == setup->key_id) {
                found_keys_count++;
                break;
            }
        }

        furi_hal_crypto_key_free(key);

        if(found_keys_count == COUNT_OF(crypto_backup_verify_key_setups)) {
            return true;
        }
    }

    FURI_LOG_E(
        TAG,
        "Some of the keys are missing: %zu/%zu.",
        found_keys_count,
        COUNT_OF(crypto_backup_verify_key_setups));

    return false;
}

static bool crypto_backup_verify_aes(void) {
    for(size_t i = 0; i < COUNT_OF(crypto_backup_verify_aes_setups); i++) {
        const CryptoBackupVerifyAesSetup* setup = &crypto_backup_verify_aes_setups[i];

        FuriHalCryptoKey* key = NULL;
        FuriHalCryptoStatus status = furi_hal_crypto_storage_read(
            &key, FuriHalCryptoPartitionMain, FuriHalCryptoKeyTypeAes256, setup->key_id);

        if(status != FuriHalCryptoStatusOk) {
            FURI_LOG_E(TAG, "AES key 0x%02lx read failed.", setup->key_id);
            return false;
        }

        FuriHalCryptoAes* aes = NULL;
        status = furi_hal_crypto_aes_init(&aes, FuriHalCryptoAesModeECB, key);
        furi_hal_crypto_key_free(key);

        if(status != FuriHalCryptoStatusOk) {
            FURI_LOG_E(TAG, "AES key 0x%02lx init failed.", setup->key_id);
            return false;
        }

        uint8_t encrypted_output[COUNT_OF(crypto_backup_verify_aes_input)];
        status = furi_hal_crypto_aes_encrypt(
            aes, NULL, crypto_backup_verify_aes_input, sizeof(encrypted_output), encrypted_output);
        furi_hal_crypto_aes_deinit(aes);

        if(status != FuriHalCryptoStatusOk) {
            FURI_LOG_E(TAG, "AES key 0x%02lx encrypt failed.", setup->key_id);
            return false;
        }

        if(memcmp(encrypted_output, setup->expected_output, sizeof(encrypted_output)) != 0) {
            FURI_LOG_E(TAG, "AES key 0x%02lx ciphertext mismatch.", setup->key_id);
            return false;
        }
    }

    return true;
}

bool crypto_backup_verify_enclave(void) {
    return crypto_backup_verify_keys() && crypto_backup_verify_aes();
}
