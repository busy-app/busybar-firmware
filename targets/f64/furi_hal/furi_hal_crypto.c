#include <furi.h>
#include <furi_hal_crypto.h>

#include <sl_si91x_aes.h>
#include <sl_si91x_ecdsa.h>
#include <sl_si91x_hmac.h>
#include <sl_si91x_sha.h>
#include <sl_si91x_wrap.h>

#include <sl_si91x_trng.h>
#include <psa/crypto.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/x509_csr.h>

#define TAG "Crypto"

#define FURI_HAL_CRYPTO_CSR_BUFFER_SIZE_MAX (2048UL)

static const uint8_t wrap_iv[SL_SI91X_IV_SIZE] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};

FuriHalCryptoKey* furi_hal_crypto_key_alloc(void) {
    return malloc(sizeof(FuriHalCryptoKey));
}

void furi_hal_crypto_key_free(FuriHalCryptoKey* key) {
    bzero(key->data, sizeof(key->data));
    free(key);
}

//#################### AES ####################
struct FuriHalCryptoAes {
    sl_si91x_aes_config_t config;
};

static const sl_si91x_aes_mode_t furi_hal_crypto_aes_mode[] = {
    [FuriHalCryptoAesModeCBC] = SL_SI91X_AES_CBC,
    [FuriHalCryptoAesModeECB] = SL_SI91X_AES_ECB,
    [FuriHalCryptoAesModeCTR] = SL_SI91X_AES_CTR};

FuriHalCryptoStatus furi_hal_crypto_aes_init(
    FuriHalCryptoAes** aes,
    FuriHalCryptoAesMode mode,
    const FuriHalCryptoKey* key) {
    furi_check(aes);
    furi_check(key);

    switch(key->type) {
    case FuriHalCryptoKeyTypeAes128:
    case FuriHalCryptoKeyTypeAes192:
    case FuriHalCryptoKeyTypeAes256:
        break;
    default:
        return FuriHalCryptoStatusWrongType;
    }

    FuriHalCryptoAes* handle = malloc(sizeof(FuriHalCryptoAes));

    handle->config.aes_mode = furi_hal_crypto_aes_mode[mode];
    handle->config.encrypt_decrypt = SL_SI91X_AES_ENCRYPT;
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.iv = NULL;
    switch(key->type) {
    case FuriHalCryptoKeyTypeAes128:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_128;
        break;
    case FuriHalCryptoKeyTypeAes192:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_192;
        break;
    case FuriHalCryptoKeyTypeAes256:
        handle->config.key_config.b0.key_size = SL_SI91X_AES_KEY_SIZE_256;
        break;
    default:
        // unreachable
        furi_assert(false);
        break;
    }
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
    if(handle->config.key_config.b0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
        memcpy(handle->config.key_config.b0.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);
    }
    memcpy(handle->config.key_config.b0.key_buffer, key->data, key->length);

    if((key->flags & FuriHalCryptoKeyFlagWrap) == 0) {
        handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
        //for 128 bits key, wrap key size is 128 bits,
        //for 192 and 256 bits keys, wrap key size is 256 bits
    }

    *aes = handle;

    return FuriHalCryptoStatusOk;
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
    furi_check(handle);
    furi_check(input);
    furi_check(input_length % 16 == 0);
    furi_check(input_length <= SL_SI91X_MAX_DATA_SIZE_IN_BYTES);
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
    furi_check(handle);
    furi_check(input);
    furi_check(input_length % 16 == 0);
    furi_check(input_length <= SL_SI91X_MAX_DATA_SIZE_IN_BYTES);
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

FuriHalCryptoStatus furi_hal_crypto_ecdsa_sign_init(
    FuriHalCryptoEcdsa** handle_out,
    FuriHalCryptoEcdsaMode mode,
    const FuriHalCryptoKey* key) {
    sl_si91x_crypto_ecc_curve_t curve_id;
    if(key->type == FuriHalCryptoKeyTypeEcdsaPriv224) {
        curve_id = SL_SI91X_ECC_SECP224R1;
    } else if(key->type == FuriHalCryptoKeyTypeEcdsaPriv256) {
        curve_id = SL_SI91X_ECC_SECP256R1;
    } else {
        return FuriHalCryptoStatusWrongType;
    }
    FuriHalCryptoEcdsa* handle = malloc(sizeof(FuriHalCryptoEcdsa));
    handle->config.curve_id = curve_id;
    handle->config.ecdsa_operation = SL_SI91X_ECDSA_GENERATE_SIGN;
    handle->config.sha_mode = furi_hal_crypto_ecdsa_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.private_key = key->data;
    handle->config.private_key_length = key->length;
    handle->config.public_key = NULL;
    handle->config.public_key_length = 0;
    handle->config.signature_length = 0;
    if((key->flags & FuriHalCryptoKeyFlagWrap) == 0) {
        handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    } else {
        handle->config.key_config.b0.key_type = SL_SI91X_WRAPPED_KEY;
        handle->config.private_key_length =
            SL_SI91X_ECDSA_PRIV_KEY_SIZE_256; // wrapped key is of fixed output size 32;
        handle->config.key_config.b0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
        if(handle->config.key_config.b0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
            memcpy(handle->config.key_config.b0.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);
        }
    }
    handle->config.key_config.b0.key_size = 0;
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.reserved = 0;

    *handle_out = handle;

    return FuriHalCryptoStatusOk;
}

FuriHalCryptoStatus furi_hal_crypto_ecdsa_verify_init(
    FuriHalCryptoEcdsa** handle_out,
    FuriHalCryptoEcdsaMode mode,
    const FuriHalCryptoKey* key) {
    sl_si91x_crypto_ecc_curve_t curve_id;
    if(key->type == FuriHalCryptoKeyTypeEcdsaPub224) {
        curve_id = SL_SI91X_ECC_SECP224R1;
    } else if(key->type == FuriHalCryptoKeyTypeEcdsaPub256) {
        curve_id = SL_SI91X_ECC_SECP256R1;
    } else {
        return FuriHalCryptoStatusWrongType;
    }
    FuriHalCryptoEcdsa* handle = malloc(sizeof(FuriHalCryptoEcdsa));
    handle->config.curve_id = curve_id;
    handle->config.ecdsa_operation = SL_SI91X_ECDSA_VERIFY_SIGN;
    handle->config.sha_mode = furi_hal_crypto_ecdsa_sha_mode[mode];
    handle->config.msg = NULL;
    handle->config.msg_length = 0;
    handle->config.private_key = NULL;
    handle->config.private_key_length = 0;
    handle->config.public_key = key->data;
    handle->config.public_key_length = key->length;
    handle->config.signature_length = 0;
    handle->config.key_config.b0.key_type = SL_SI91X_TRANSPARENT_KEY;
    handle->config.key_config.b0.key_size = 0;
    handle->config.key_config.b0.key_slot = 0;
    handle->config.key_config.b0.reserved = 0;

    *handle_out = handle;
    return FuriHalCryptoStatusOk;
}

void furi_hal_crypto_ecdsa_deinit(FuriHalCryptoEcdsa* handle) {
    furi_check(handle);
    free(handle);
}

bool furi_hal_crypto_ecdsa_sign(
    FuriHalCryptoEcdsa* handle,
    const uint8_t* input,
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
        FURI_LOG_E(TAG, "Failed to sign data, Error Code : 0x%08lX", status);
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
    [FuriHalCryptoKeyTypeHmacSha1] = SL_SI91X_HMAC_SHA_1,
    [FuriHalCryptoKeyTypeHmacSha256] = SL_SI91X_HMAC_SHA_256,
    [FuriHalCryptoKeyTypeHmacSha384] = SL_SI91X_HMAC_SHA_384,
    [FuriHalCryptoKeyTypeHmacSha512] = SL_SI91X_HMAC_SHA_512,
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
        handle->config.key_config.B0.wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
        if(handle->config.key_config.B0.wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
            memcpy(handle->config.key_config.B0.wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);
        }
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
        FURI_LOG_E(TAG, "Failed to compute HMAC, , Error Code : 0x%08lX", status);
        return false;
    }
    return true;
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
FuriHalCryptoStatus
    furi_hal_crypto_wrap_key(const FuriHalCryptoKey* key, FuriHalCryptoKey* wrapped_key) {
    furi_assert(key);
    furi_assert(wrapped_key);
    furi_check(key->length <= SL_SI91X_WRAP_KEY_BUFFER_SIZE);
    furi_check((key->flags & FuriHalCryptoKeyFlagWrap) == 0);
    //sl_si91x_wrap_config_t - size 1432 bytes
    sl_si91x_wrap_config_t* wrap_config = malloc(sizeof(sl_si91x_wrap_config_t));
    wrap_config->key_type = SL_SI91X_TRANSPARENT_KEY;
    wrap_config->key_size = key->length;
    switch(key->type) {
    case FuriHalCryptoKeyTypeHmacSha1:
    case FuriHalCryptoKeyTypeHmacSha256:
    case FuriHalCryptoKeyTypeHmacSha384:
    case FuriHalCryptoKeyTypeHmacSha512:
        wrap_config->padding = (1 << 0); //SL_SI91X_HMAC_PADDING;
        wrap_config->hmac_sha_mode = furi_hal_crypto_hmac_sha_mode[key->type];
        break;
    default:
        wrap_config->wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
        wrap_config->padding = 0;
        break;
    }

    memcpy(wrap_config->key_buffer, key->data, wrap_config->key_size);
    if(wrap_config->wrap_iv_mode == SL_SI91X_WRAP_IV_CBC_MODE) {
        memcpy(wrap_config->wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);
    }

    sl_status_t status = sl_si91x_wrap(wrap_config, wrapped_key->data);
    FuriHalCryptoStatus ret = FuriHalCryptoStatusOk;
    if(status == SL_STATUS_OK) {
        wrapped_key->type = key->type;
        wrapped_key->flags = key->flags | FuriHalCryptoKeyFlagWrap;
        wrapped_key->length = wrap_config->key_size;
    } else if(status == SL_STATUS_SI91X_CRYPTO_DEVICE_SECURITY_IS_DISABLED) {
        ret = FuriHalCryptoStatusUnavailable;
    } else {
        FURI_LOG_E(TAG, "Failed to wrap key: 0x%08lX", status);
        ret = FuriHalCryptoStatusFail;
    }

    free(wrap_config);
    return ret;
}

FuriHalCryptoStatus
    furi_hal_crypto_wrap_raw_key(size_t size, const uint8_t* src_buf, uint8_t* dst_buf) {
    furi_assert(src_buf);
    furi_assert(dst_buf);
    furi_check(size <= SL_SI91X_WRAP_KEY_BUFFER_SIZE);
    //sl_si91x_wrap_config_t - size 1432 bytes
    sl_si91x_wrap_config_t* wrap_config = malloc(sizeof(sl_si91x_wrap_config_t));
    wrap_config->key_type = SL_SI91X_TRANSPARENT_KEY;
    wrap_config->key_size = size;
    wrap_config->wrap_iv_mode = SL_SI91X_WRAP_IV_CBC_MODE;
    wrap_config->padding = 0;
    memcpy(wrap_config->key_buffer, src_buf, wrap_config->key_size);
    memcpy(wrap_config->wrap_iv, wrap_iv, SL_SI91X_IV_SIZE);

    sl_status_t status = sl_si91x_wrap(wrap_config, dst_buf);

    free(wrap_config);

    if(status == SL_STATUS_SI91X_CRYPTO_DEVICE_SECURITY_IS_DISABLED) {
        return FuriHalCryptoStatusUnavailable;
    } else if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to wrap key: 0x%08lX", status);
        return FuriHalCryptoStatusFail;
    } else {
        return FuriHalCryptoStatusOk;
    }
}

//#################### Key generation ##############
FuriHalCryptoStatus furi_hal_crypto_gen_random_buf(uint8_t* buf, size_t size) {
    furi_check(buf);
    furi_check(size > 0);
    furi_check(size <= 1024);

    uint32_t trng_key[TRNG_KEY_SIZE] = {0x16157E2B, 0xA6D2AE28, 0x8815F7AB, 0x3C4FCF09};
    sl_status_t status = SL_STATUS_FAIL;
    // This API checks the Entropy of TRNG i.e source for TRNG
    status = sl_si91x_trng_entropy();
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to check TRNG entropy: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }
    // This API Initializes key which needs to be programmed to TRNG hardware engine
    status = sl_si91x_trng_program_key(trng_key, TRNG_KEY_SIZE);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to program TRNG key: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }
    // Get Random dwords of desired length
    uint32_t reget_num = 10;
    do {
        status = sl_si91x_trng_get_random_num((uint32_t*)buf, size);
        --reget_num;
    } while((status == SL_STATUS_TRNG_DUPLICATE_ENTROPY) && reget_num);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to get random numbers: 0x%08lx\r\n", status);
        return FuriHalCryptoStatusFail;
    }
    return FuriHalCryptoStatusOk;
}

FuriHalCryptoStatus furi_hal_crypto_gen_random_key(
    FuriHalCryptoKey* key,
    FuriHalCryptoKeyType type,
    FuriHalCryptoKeyFlag flags) {
    furi_check(key);

    switch(type) {
    case FuriHalCryptoKeyTypeAes128:
        key->length = FURI_HAL_CRYPTO_AES_KEY_SIZE_128;
        break;
    case FuriHalCryptoKeyTypeAes192:
        key->length = FURI_HAL_CRYPTO_AES_KEY_SIZE_192;
        break;
    case FuriHalCryptoKeyTypeAes256:
        key->length = FURI_HAL_CRYPTO_AES_KEY_SIZE_256;
        break;
    case FuriHalCryptoKeyTypeHmacSha1:
        key->length = FURI_HAL_CRYPTO_HMAC_SHA1_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeHmacSha256:
        key->length = FURI_HAL_CRYPTO_HMAC_SHA256_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeHmacSha384:
        key->length = FURI_HAL_CRYPTO_HMAC_SHA384_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeHmacSha512:
        key->length = FURI_HAL_CRYPTO_HMAC_SHA512_DIGEST_SIZE;
        break;
    case FuriHalCryptoKeyTypeEcdsaPriv224:
        key->length = FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224;
        break;
    case FuriHalCryptoKeyTypeEcdsaPriv256:
        key->length = FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256;
        break;
    default:
        return FuriHalCryptoStatusWrongType;
    }

    key->type = type;
    key->flags = flags;
    return furi_hal_crypto_gen_random_buf(key->data, key->length);
}

FuriHalCryptoStatus furi_hal_crypto_gen_asymmetric_pub_key(
    const FuriHalCryptoKey* priv_key,
    FuriHalCryptoKey* pub_key) {
    furi_check(priv_key);
    furi_check(pub_key);
    if(priv_key->type != FuriHalCryptoKeyTypeEcdsaPriv224 &&
       priv_key->type != FuriHalCryptoKeyTypeEcdsaPriv256) {
        return FuriHalCryptoStatusWrongType;
    }
    if(priv_key->flags & FuriHalCryptoKeyFlagWrap) {
        return FuriHalCryptoStatusWrongType;
    }

    psa_status_t psa_status;
    psa_key_id_t psa_key_id;
    psa_key_attributes_t key_attr;
    size_t pubkey_len;
    FuriHalCryptoStatus status = FuriHalCryptoStatusFail;

    do {
        psa_status = psa_crypto_init();
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(
                TAG, "PSA crypto library initialization failed with error: %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "PSA crypto library initialization Success");
        }
        // Set up attributes for a volatile private key
        key_attr = psa_key_attributes_init();
        psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));

        if(priv_key->type == FuriHalCryptoKeyTypeEcdsaPriv224) {
            psa_set_key_bits(&key_attr, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_224_BITS);
            pub_key->length = FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_224; // Get size in bytes
            pub_key->type = FuriHalCryptoKeyTypeEcdsaPub224;
        } else if(priv_key->type == FuriHalCryptoKeyTypeEcdsaPriv256) {
            psa_set_key_bits(&key_attr, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS);
            pub_key->length = FURI_HAL_CRYPTO_ECDSA_PUB_KEY_SIZE_256; // Get size in bytes
            pub_key->type = FuriHalCryptoKeyTypeEcdsaPub256;
        }
        pub_key->flags = 0;

        psa_set_key_usage_flags(
            &key_attr, PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE);
        psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

        // Import a private key
        psa_status = psa_import_key(&key_attr, priv_key->data, priv_key->length, &psa_key_id);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(TAG, "Import Key failed with error: status %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Import Key success");
        }

        // Export a public key from a volatile private key
        psa_status =
            psa_export_public_key(psa_key_id, pub_key->data, pub_key->length, &pubkey_len);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(
                TAG, "Exporting a Public Key from Private key Failed with error: %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Export Public Key from Private Key Success");
        }

        furi_check(pubkey_len == pub_key->length);

        // Destroy the private key
        psa_status = psa_destroy_key(psa_key_id);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(TAG, "Destroy Key failed with error : %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Destroy Key Success");
        }
        status = FuriHalCryptoStatusOk;
    } while(false);
    return status;
}

FuriHalCryptoStatus furi_hal_crypto_gen_csr_der_ecdsa256(
    const FuriHalCryptoKey* priv_key,
    FuriHalCryptoKey* csr_der_key,
    const char* subject_name) {
    furi_check(priv_key);

    if(priv_key->type != FuriHalCryptoKeyTypeEcdsaPriv256 ||
       (priv_key->flags & FuriHalCryptoKeyFlagWrap) != 0) {
        return FuriHalCryptoStatusWrongType;
    }

    psa_key_id_t psa_key_id = 0;
    mbedtls_pk_context key_ctx;
    mbedtls_x509write_csr csr_ctx;
    int csr_der_status = 0;

    size_t max_size = FURI_HAL_CRYPTO_CSR_BUFFER_SIZE_MAX;
    uint8_t* buffer = malloc(max_size);
    psa_key_attributes_t key_attr;
    FuriHalCryptoStatus status = FuriHalCryptoStatusFail;
    psa_status_t psa_status;

    do {
        psa_status = psa_crypto_init();
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(
                TAG, "PSA crypto library initialization failed with error: %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "PSA crypto library initialization Success");
        }

        // import key
        key_attr = psa_key_attributes_init();
        psa_set_key_type(&key_attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
        psa_set_key_bits(&key_attr, FURI_HAL_CRYPTO_ECDSA_PRIV_KEY_SIZE_256_BITS);
        psa_set_key_algorithm(&key_attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
        psa_set_key_usage_flags(
            &key_attr,
            PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_MESSAGE |
                PSA_KEY_USAGE_VERIFY_MESSAGE);

        // Import a private key
        psa_status = psa_import_key(&key_attr, priv_key->data, priv_key->length, &psa_key_id);
        if(psa_status != PSA_SUCCESS) {
            FURI_LOG_E(TAG, "Import Key failed with error: status %ld", psa_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Import Key success");
        }

        // Generate CSR
        mbedtls_x509write_csr_init(&csr_ctx);
        csr_der_status = mbedtls_x509write_csr_set_subject_name(&csr_ctx, subject_name);
        if(csr_der_status != 0) {
            FURI_LOG_E(TAG, "Failed to set subject name for CSR: %d", csr_der_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Subject name set for CSR: %s", subject_name);
        }
        mbedtls_x509write_csr_set_md_alg(&csr_ctx, MBEDTLS_MD_SHA256);
        mbedtls_pk_init(&key_ctx);
        csr_der_status = mbedtls_pk_setup_opaque(&key_ctx, psa_key_id);
        if(csr_der_status != 0) {
            FURI_LOG_E(TAG, "Failed to setup key context for CSR: %d", csr_der_status);
            break;
        } else {
            FURI_LOG_D(TAG, "Key context setup for CSR");
        }
        mbedtls_x509write_csr_set_key(&csr_ctx, &key_ctx);
        int len_or_err =
            mbedtls_x509write_csr_der(&csr_ctx, (uint8_t*)buffer, max_size, NULL, NULL);
        if(len_or_err < 0) {
            FURI_LOG_E(TAG, "Failed to generate CSR DER: %d", len_or_err);
            break;
        } else {
            FURI_LOG_D(TAG, "CSR DER generated successfully");
        }

        csr_der_key->length = len_or_err;
        csr_der_key->type = FuriHalCryptoKeyTypeCsrDerEcdsa256;

        memcpy(csr_der_key->data, buffer + (max_size - len_or_err), len_or_err);
        status = FuriHalCryptoStatusOk;
    } while(false);

    mbedtls_x509write_csr_free(&csr_ctx);
    mbedtls_pk_free(&key_ctx);
    psa_destroy_key(psa_key_id);
    free(buffer);

    return status;
}

static const char* const key_type_names[] = {
    [FuriHalCryptoKeyTypeAes128] = "AES-128",
    [FuriHalCryptoKeyTypeAes192] = "AES-192",
    [FuriHalCryptoKeyTypeAes256] = "AES-256",
    [FuriHalCryptoKeyTypeHmacSha1] = "SHA-1",
    [FuriHalCryptoKeyTypeHmacSha256] = "SHA-256",
    [FuriHalCryptoKeyTypeHmacSha384] = "SHA-384",
    [FuriHalCryptoKeyTypeHmacSha512] = "SHA-512",
    [FuriHalCryptoKeyTypeEcdsaPriv224] = "ECDSA-224",
    [FuriHalCryptoKeyTypeEcdsaPriv256] = "ECDSA-256",
    [FuriHalCryptoKeyTypeEcdsaPub224] = "ECDSA-224-pub",
    [FuriHalCryptoKeyTypeEcdsaPub256] = "ECDSA-256-pub",
    [FuriHalCryptoKeyTypeCsrDerEcdsa256] = "CSR-DER-ECDSA-256",
    [FuriHalCryptoKeyTypeCrtDerEcdsa256] = "CRT-DER-ECDSA-256",
    [FuriHalCryptoKeyTypeMatterAttestation] = "Matter Attestation",
    [FuriHalCryptoKeyTypeMatterSetup] = "Matter Setup",
    [FuriHalCryptoKeyTypeMatterDeviceInfo] = "Matter Device Info",
};

const char* furi_hal_crypto_get_key_type_name(FuriHalCryptoKeyType type) {
    if(type < COUNT_OF(key_type_names)) {
        return key_type_names[type];
    } else {
        return "Unknown";
    }
}
