#include <furi.h>
#include <furi_hal_crypto.h>

#include <sl_si91x_aes.h>
#include <sl_si91x_ecdsa.h>
#include <sl_si91x_hmac.h>
#include <sl_si91x_sha.h>
#include <sl_si91x_wrap.h>

#define TAG "Crypto"

//#################### AES ####################
struct FuriHalCryptoAes {
    sl_si91x_aes_config_t config;
};

static const sl_si91x_aes_mode_t furi_hal_crypto_aes_mode[] = {
    [FuriHalCryptoAesModeCBC] = SL_SI91X_AES_CBC,
    [FuriHalCryptoAesModeECB] = SL_SI91X_AES_ECB,
    [FuriHalCryptoAesModeCTR] = SL_SI91X_AES_CTR};

FuriHalCryptoAes* furi_hal_crypto_aes_init(
    FuriHalCryptoAesMode mode,
    uint8_t* key,
    size_t key_size,
    FuriHalCryptoWrappingMode wrapping_mode) {
    FuriHalCryptoAes* handle = malloc(sizeof(FuriHalCryptoAes));
    furi_check(handle != NULL, "Failed to allocate memory for AES handle");

    handle->config.aes_mode = furi_hal_crypto_aes_mode[mode];
    handle->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.iv = NULL;
    switch(key_size) {
    case SL_SI91X_AES_KEY_SIZE_128:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_128;
        break;
    case SL_SI91X_AES_KEY_SIZE_192:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_192;
        break;
    case SL_SI91X_AES_KEY_SIZE_256:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_256;
        break;

    default:
        furi_crash("Invalid key size");
        break;
    }
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    memcpy(handle->config.key_config.b0.key_buffer, key, handle->config.key_config.b0.key_size);

    if(wrapping_mode == FuriHalCryptoWrappingModeOff) {
        handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
        //for 128 bits key, wrap key size is 128 bits,
        //for 192 and 256 bits keys, wrap key size is 256 bits
        if(handle->config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_128) {
            memcpy(handle->config.key_config.b0.key_buffer, key, SL_SI91X_AES_KEY_SIZE_128);
        } else if(
            handle->config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_192 ||
            handle->config.key_config.b0.key_size == SL_SI91X_AES_KEY_SIZE_256) {
            memcpy(handle->config.key_config.b0.key_buffer, key, SL_SI91X_AES_KEY_SIZE_256);
        } else {
            furi_crash("Invalid key size");
        }
    }

    return handle;
}

void furi_hal_crypto_aes_deinit(FuriHalCryptoAes* handle) {
    free(handle);
}

bool furi_hal_crypto_aes_encrypt(
    FuriHalCryptoAes* handle,
    uint8_t* iv,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output) {
    furi_assert(handle);
    furi_assert(input);
    furi_assert(input_length % 16 == 0);
    furi_assert(input_length <= SL_SI91X_MAX_DATA_SIZE_IN_BYTES);
    if(handle->config.aes_mode != SL_SI91X_AES_ECB) {
        furi_check(iv);
    }

    handle->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
    handle->config.msg = input;
    handle->config.msg_length = input_length;
    handle->config.iv = iv;

    sl_status_t status = sl_si91x_aes(&handle->config, output);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "AES encryption failed, Error Code : 0x%08lX", status);
        return false;
    }
    return true;
}

bool furi_hal_crypto_aes_decrypt(
    FuriHalCryptoAes* handle,
    uint8_t* iv,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output) {
    furi_assert(handle);
    furi_assert(input);
    furi_assert(input_length % 16 == 0);
    furi_assert(input_length <= SL_SI91X_MAX_DATA_SIZE_IN_BYTES);
    if(handle->config.aes_mode != SL_SI91X_AES_ECB) {
        furi_check(iv);
    }

    handle->config.encrypt_decrypt = SL_SI91X_AES_DECRYPT;
    handle->config.msg = input;
    handle->config.msg_length = input_length;
    handle->config.iv = iv;

    sl_status_t status = sl_si91x_aes(&handle->config, output);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "AES decryption failed, Error Code : 0x%08lX", status);
        return false;
    }
    return true;
}

//#################### ECDSA ####################
static const sl_si91x_crypto_ecdsa_sha_mode_t furi_hal_crypto_ecdsa_sha_mode[] = {
    [FuriHalCryptoEcdsaModeSha256] = SL_SI91X_ECDSA_SHA_256,
    [FuriHalCryptoEcdsaModeSha384] = SL_SI91X_ECDSA_SHA_384,
    [FuriHalCryptoEcdsaModeSha512] = SL_SI91X_ECDSA_SHA_512,
};

struct FuriHalCryptoEcdsa {
    sl_si91x_ecdsa_config_t config;
};

FuriHalCryptoEcdsa* furi_hal_crypto_ecdsa_sign_init(
    FuriHalCryptoEcdsaMode mode,
    uint8_t* key,
    uint32_t key_mode,
    FuriHalCryptoWrappingMode wrapping_mode) {
    FuriHalCryptoEcdsa* handle = malloc(sizeof(FuriHalCryptoEcdsa));
    furi_check(handle != NULL, "Failed to allocate memory for ECDSA handle");

    handle->config.ecdsa_operation = SL_SI91X_ECDSA_GENERATE_SIGN;
    if(key_mode == FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224) {
        handle->config.curve_id = SL_SI91X_ECC_SECP224R1;
    } else if(key_mode == FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256) {
        handle->config.curve_id = SL_SI91X_ECC_SECP256R1;
    } else {
        furi_crash("Invalid key size");
    }
    handle->config.sha_mode = furi_hal_crypto_ecdsa_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.private_key = key;
    handle->config.private_key_length = key_mode;
    handle->config.public_key = NULL;
    handle->config.public_key_length = 0;
    handle->config.signature_length = 0;
    if(wrapping_mode != FuriHalCryptoWrappingModeOn) {
        handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
        handle->config.private_key_length =
            SL_SI91X_ECDSA_PRIV_KEY_SIZE_256; // wrapped key is of fixed output size 32;
        handle->config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    }
    handle->config.key_config.b0.key_size = 0;
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.reserved = 0;

    return handle;
}

FuriHalCryptoEcdsa* furi_hal_crypto_ecdsa_verify_init(
    FuriHalCryptoEcdsaMode mode,
    uint8_t* key,
    uint32_t key_mode) {
    FuriHalCryptoEcdsa* handle = malloc(sizeof(FuriHalCryptoEcdsa));
    furi_check(handle != NULL, "Failed to allocate memory for ECDSA handle");

    handle->config.ecdsa_operation = SL_SI91X_ECDSA_VERIFY_SIGN;
    if(key_mode == FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224) {
        handle->config.curve_id = SL_SI91X_ECC_SECP224R1;
    } else if(key_mode == FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256) {
        handle->config.curve_id = SL_SI91X_ECC_SECP256R1;
    } else {
        furi_crash("Invalid key size");
    }
    handle->config.sha_mode = furi_hal_crypto_ecdsa_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.private_key = NULL;
    handle->config.private_key_length = 0;
    handle->config.public_key = key;
    handle->config.public_key_length = key_mode;
    handle->config.signature_length = 0;
    handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    handle->config.key_config.b0.key_size = 0;
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.reserved = 0;

    return handle;
}

void furi_hal_crypto_ecdsa_deinit(FuriHalCryptoEcdsa* handle) {
    furi_check(handle);
    free(handle);
}

bool furi_hal_crypto_ecdsa_sign(
    FuriHalCryptoEcdsa* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output,
    size_t* output_length) {
    furi_check(handle && input && output && output_length);
    furi_check(handle->config.private_key && handle->config.private_key_length);
    handle->config.msg = input;
    handle->config.msg_length = input_length;
    handle->config.signature_length = *output_length;
    sl_status_t status = sl_si91x_ecdsa(&handle->config, output);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to sign data");
        return false;
    }
    *output_length = handle->config.signature_length;
    return true;
}

bool furi_hal_crypto_ecdsa_verify(
    FuriHalCryptoEcdsa* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* signature,
    uint16_t signature_length) {
    furi_check(handle && input && signature);
    furi_check(handle->config.public_key && handle->config.public_key_length);
    uint8_t* verify = NULL;
    handle->config.msg = input;
    handle->config.msg_length = input_length;
    handle->config.signature = signature;
    handle->config.signature_length = signature_length;
    sl_status_t status = sl_si91x_ecdsa(&handle->config, verify);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to verify data, Error Code : 0x%08lX", status);
        return false;
    } else if(*verify != 1) {
        FURI_LOG_D(TAG, "Failed to verify data");
        return false;
    }
    return true;
}

//#################### HMAC ####################
struct FuriHalCryptoHmac {
    sl_si91x_hmac_config_t config;
};

static const sl_si91x_hmac_mode_t furi_hal_crypto_hmac_sha_mode[] = {
    [FuriHalCryptoHmacShaModeSha1] = SL_SI91X_HMAC_SHA_1,
    [FuriHalCryptoHmacShaModeSha256] = SL_SI91X_HMAC_SHA_256,
    [FuriHalCryptoHmacShaModeSha384] = SL_SI91X_HMAC_SHA_384,
    [FuriHalCryptoHmacShaModeSha512] = SL_SI91X_HMAC_SHA_512,
};

FuriHalCryptoHmac* furi_hal_crypto_hmac_init(
    FuriHalCryptoHmacShaMode mode,
    uint8_t* key,
    size_t key_size,
    FuriHalCryptoWrappingMode wrapping_mode) {
    FuriHalCryptoHmac* handle = malloc(sizeof(FuriHalCryptoHmac));
    furi_check(handle != NULL, "Failed to allocate memory for HMAC handle");

    handle->config.hmac_mode = furi_hal_crypto_hmac_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    if(wrapping_mode != FuriHalCryptoWrappingModeOn) {
        handle->config.key_config.B0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.B0.key_type = SL_SI91X_WRAPPED_KEY;
        handle->config.key_config.B0.wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    }
    handle->config.key_config.B0.key_size = key_size;
    handle->config.key_config.B0.key = key;

    return handle;
}

void furi_hal_crypto_hmac_deinit(FuriHalCryptoHmac* handle) {
    furi_check(handle);
    free(handle);
}

bool furi_hal_crypto_hmac_digest(
    FuriHalCryptoHmac* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output,
    size_t output_length) {
    furi_check(handle && input && output);
    furi_check(
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_1 &&
         output_length == SL_SI91X_HMAC_SHA_1_DIGEST_LEN) ||
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_256 &&
         output_length == SL_SI91X_HMAC_SHA_256_DIGEST_LEN) ||
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_384 &&
         output_length == SL_SI91X_HMAC_SHA_384_DIGEST_LEN) ||
        (handle->config.hmac_mode == SL_SI91X_HMAC_SHA_512 &&
         output_length == SL_SI91X_HMAC_SHA_512_DIGEST_LEN));

    handle->config.msg = input;
    handle->config.msg_length = input_length;

    sl_status_t status = sl_si91x_hmac(&handle->config, output);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to compute HMAC");
        return false;
    }
    return true;
}

void furi_hal_crypto_hmac_wrap_key(
    uint32_t key_size,
    uint8_t* key,
    FuriHalCryptoHmacShaMode hmac_sha_mode,
    uint8_t* wrapped_key,
    size_t* wrapped_key_size) {
    furi_assert(key);
    furi_assert(wrapped_key);
    furi_check(key_size <= SL_SI91X_WRAP_KEY_BUFFER_SIZE);
    //sl_si91x_wrap_config_t - size 1432 bytes
    sl_si91x_wrap_config_t* wrap_config = malloc(sizeof(sl_si91x_wrap_config_t));
    furi_check(wrap_config != NULL, "Failed to allocate memory for wrap config");
    wrap_config->key_type = SL_SI91X_TRANSPARENT_KEY;
    wrap_config->key_size = key_size;
    wrap_config->wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    wrap_config->padding = (1 << 0); //SL_SI91X_HMAC_PADDING;
    wrap_config->hmac_sha_mode = furi_hal_crypto_hmac_sha_mode[hmac_sha_mode];

    //memset(wrapped_key, 0, *wrapped_key_size);
    memcpy(wrap_config->key_buffer, key, wrap_config->key_size);

    sl_status_t status = sl_si91x_wrap(wrap_config, wrapped_key);
    *wrapped_key_size = wrap_config->key_size;
    free(wrap_config);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to wrap key: 0x%08lX", status);
        furi_crash("Failed to wrap key");
    }
}

//#################### SHA ####################
static const sl_si91x_crypto_sha_mode_t furi_hal_crypto_sha_mode[] = {
    [FuriHalCryptoShaModeSha1] = SL_SI91X_SHA_1,
    [FuriHalCryptoShaModeSha256] = SL_SI91X_SHA_256,
    [FuriHalCryptoShaModeSha384] = SL_SI91X_SHA_384,
    [FuriHalCryptoShaModeSha512] = SL_SI91X_SHA_512,
    [FuriHalCryptoShaModeSha244] = SL_SI91X_SHA_224,
};

bool furi_hal_crypto_sha(
    FuriHalCryptoShaMode sha_mode,
    uint8_t* msg,
    uint16_t msg_length,
    uint8_t* digest,
    size_t digest_length) {
    furi_check(sha_mode < FuriHalCryptoShaModeMAX, "Invalid SHA mode");
    furi_assert(msg && msg_length && digest);
    furi_assert(
        (sha_mode == FuriHalCryptoShaModeSha1 &&
         digest_length == FURI_HAL_CRYPTO_SHA1_DIGEST_SIZE) ||
        (sha_mode == FuriHalCryptoShaModeSha256 &&
         digest_length == FURI_HAL_CRYPTO_SHA256_DIGEST_SIZE) ||
        (sha_mode == FuriHalCryptoShaModeSha384 &&
         digest_length == FURI_HAL_CRYPTO_SHA384_DIGEST_SIZE) ||
        (sha_mode == FuriHalCryptoShaModeSha512 &&
         digest_length == FURI_HAL_CRYPTO_SHA512_DIGEST_SIZE) ||
        (sha_mode == FuriHalCryptoShaModeSha244 &&
         digest_length == FURI_HAL_CRYPTO_SHA224_DIGEST_SIZE));
    sl_status_t status = sl_si91x_sha(furi_hal_crypto_sha_mode[sha_mode], msg, msg_length, digest);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed, Error Code : 0x%08lX", status);
        return false;
    }
    return true;
}

//#################### Wrap Key ####################
void furi_hal_crypto_wrap_key(uint32_t key_size, uint8_t* key, uint8_t* wrapped_key) {
    furi_assert(key);
    furi_assert(wrapped_key);
    furi_check(key_size <= SL_SI91X_WRAP_KEY_BUFFER_SIZE);
    //sl_si91x_wrap_config_t - size 1432 bytes
    sl_si91x_wrap_config_t* wrap_config = malloc(sizeof(sl_si91x_wrap_config_t));
    furi_check(wrap_config != NULL, "Failed to allocate memory for wrap config");
    wrap_config->key_type = SL_SI91X_TRANSPARENT_KEY;
    wrap_config->key_size = key_size;
    wrap_config->wrap_iv_mode = SL_SI91X_WRAP_IV_ECB_MODE;
    wrap_config->padding = 0;
    memcpy(wrap_config->key_buffer, key, wrap_config->key_size);
    sl_status_t status = sl_si91x_wrap(wrap_config, wrapped_key);
    furi_check(key_size == wrap_config->key_size, "Invalid key size");
    free(wrap_config);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to wrap key: 0x%08lX", status);
        furi_crash("Failed to wrap key");
    }
}
