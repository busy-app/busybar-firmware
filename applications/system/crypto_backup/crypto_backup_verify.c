#include "crypto_backup_verify.h"

#include <furi_hal_crypto_storage.h>

#include <inttypes.h>

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

static const uint8_t crypto_backup_verify_aes_input[] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

typedef struct {
    uint32_t key_id;
    uint8_t expected_output[COUNT_OF(crypto_backup_verify_aes_input)];
} CryptoBackupVerifyAesSetup;

static const CryptoBackupVerifyAesSetup crypto_backup_verify_aes_setups[] = {
    {
        .key_id = 0x00,
        .expected_output =
            {0x6a,
             0x2c,
             0xad,
             0xdb,
             0x7c,
             0xd9,
             0x2a,
             0x3d,
             0x08,
             0x21,
             0xf7,
             0x97,
             0xec,
             0x2c,
             0x65,
             0x2f},
    },
    {
        .key_id = 0x01,
        .expected_output =
            {0x0f,
             0x7c,
             0x8e,
             0x22,
             0xae,
             0xda,
             0xc1,
             0x9d,
             0x53,
             0xb9,
             0xaa,
             0xb2,
             0xc3,
             0x28,
             0x22,
             0x2b},
    },
    {
        .key_id = 0x02,
        .expected_output =
            {0x96,
             0x0d,
             0x90,
             0x5d,
             0xca,
             0x9e,
             0xaf,
             0x16,
             0xab,
             0x61,
             0xdf,
             0x86,
             0x20,
             0xe4,
             0x38,
             0x2a},
    },
    {
        .key_id = 0x03,
        .expected_output =
            {0x79,
             0x8d,
             0x0d,
             0x69,
             0x52,
             0xb3,
             0x44,
             0x5c,
             0x90,
             0x0e,
             0x69,
             0x3b,
             0x65,
             0xcc,
             0x4e,
             0xe8},
    },
    {
        .key_id = 0x04,
        .expected_output =
            {0xe2,
             0xe9,
             0xa6,
             0x37,
             0xa6,
             0x3a,
             0xe8,
             0x9e,
             0x97,
             0x89,
             0xb8,
             0x18,
             0x82,
             0x94,
             0xb7,
             0x8b},
    },
};

static bool crypto_backup_verify_keys(void) {
    FuriHalCryptoKeyIter iter = furi_hal_crypto_key_iter_init(FuriHalCryptoPartitionMain);
    FuriHalCryptoKey* key;
    FuriHalCryptoKeySlot slot;

    uint8_t keys_count[COUNT_OF(crypto_backup_verify_key_setups)] = {
        [0 ... COUNT_OF(crypto_backup_verify_key_setups) - 1] = 0};

    while(furi_hal_crypto_key_iter_get_and_advance(&iter, &key, &slot) == FuriHalCryptoStatusOk) {
        for(size_t i = 0; i < COUNT_OF(crypto_backup_verify_key_setups); i++) {
            const CryptoBackupVerifyKeySetup* setup = &crypto_backup_verify_key_setups[i];

            if(slot.header.type == setup->type && slot.header.id == setup->key_id) {
                keys_count[i]++;
                break;
            }
        }

        furi_hal_crypto_key_free(key);
    }

    bool are_keys_valid = true;
    for(size_t i = 0; i < COUNT_OF(crypto_backup_verify_key_setups); i++) {
        uint8_t key_count = keys_count[i];

        switch(key_count) {
        case 0: {
            const CryptoBackupVerifyKeySetup* key_setup = &crypto_backup_verify_key_setups[i];
            FURI_LOG_E(
                TAG,
                "Key 0x%02" PRIX32 " of type 0x%02X is missing.",
                key_setup->key_id,
                key_setup->type);
            are_keys_valid = false;
            break;
        }

        case 1:
            break;

        default: {
            const CryptoBackupVerifyKeySetup* key_setup = &crypto_backup_verify_key_setups[i];
            FURI_LOG_E(
                TAG,
                "Key 0x%02" PRIX32 " of type 0x%02X has %" PRIu8 " duplicates.",
                key_setup->key_id,
                key_setup->type,
                key_count - 1);
            are_keys_valid = false;
            break;
        }
        }
    }

    return are_keys_valid;
}

static bool crypto_backup_verify_aes(void) {
    for(size_t i = 0; i < COUNT_OF(crypto_backup_verify_aes_setups); i++) {
        const CryptoBackupVerifyAesSetup* setup = &crypto_backup_verify_aes_setups[i];

        FuriHalCryptoKey* key = NULL;
        FuriHalCryptoStatus status = furi_hal_crypto_storage_read(
            &key, FuriHalCryptoPartitionMain, FuriHalCryptoKeyTypeAes256, setup->key_id);

        if(status != FuriHalCryptoStatusOk) {
            FURI_LOG_E(TAG, "AES key 0x%02" PRIX32 " read failed.", setup->key_id);
            return false;
        }

        FuriHalCryptoAes* aes = NULL;
        status = furi_hal_crypto_aes_init(&aes, FuriHalCryptoAesModeECB, key);
        furi_hal_crypto_key_free(key);

        if(status != FuriHalCryptoStatusOk) {
            FURI_LOG_E(TAG, "AES key 0x%02" PRIX32 " init failed.", setup->key_id);
            return false;
        }

        uint8_t encrypted_output[COUNT_OF(crypto_backup_verify_aes_input)];
        status = furi_hal_crypto_aes_encrypt(
            aes, NULL, crypto_backup_verify_aes_input, sizeof(encrypted_output), encrypted_output);
        furi_hal_crypto_aes_deinit(aes);

        if(status != FuriHalCryptoStatusOk) {
            FURI_LOG_E(TAG, "AES key 0x%02" PRIX32 " encrypt failed.", setup->key_id);
            return false;
        }

        if(memcmp(encrypted_output, setup->expected_output, sizeof(encrypted_output)) != 0) {
            FURI_LOG_E(TAG, "AES key 0x%02" PRIX32 " ciphertext mismatch.", setup->key_id);
            return false;
        }
    }

    return true;
}

bool crypto_backup_verify_enclave(void) {
    return crypto_backup_verify_keys() && crypto_backup_verify_aes();
}

bool furi_hal_info_verify_crypto_enclave(void)
    __attribute__((alias(STRINGIFY(crypto_backup_verify_enclave))));
