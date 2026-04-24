#include "crypto_aes.h"
#include "crypto_common.h"

#include <furi_hal_crypto.h>
#include <cli/cli_ansi.h>

#define TAG "CryptoAES"

#define BUFFER_SIZE 128

static const uint8_t message_const[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd',
                                        '!', '!', '!', '!', '!', 'H', 'e', 'l', 'l', 'o', ' ',
                                        'W', 'o', 'r', 'l', 'd', '!', '!', '!', '1', '1'};

static const uint8_t key_const[FURI_HAL_CRYPTO_AES_KEY_SIZE_256] = {
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!'};

// IV used for SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes.
static const uint8_t iv_const[FURI_HAL_CRYPTO_AES_IV_SIZE] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

FURI_CHECK_RETURN
static bool crypto_aes_check(char* tag, uint8_t* encrypted_buffer, uint8_t* decrypted_buffer) {
    crypto_common_print_buffer_char("msg =\t\t", (uint8_t*)message_const, sizeof(message_const));
    crypto_common_print_buffer_hex("msg =\t\t", (uint8_t*)message_const, sizeof(message_const));

    crypto_common_print_buffer_hex(
        "msg ecrypt =\t", encrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
    crypto_common_print_buffer_hex(
        "msg decrypt= \t", decrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
    if(memcmp(message_const, decrypted_buffer, sizeof(message_const)) != 0) {
        printf(ANSI_FG_RED "%s mode failed\r\n" ANSI_RESET, tag);
        return false;
    } else {
        crypto_common_print_buffer_char(
            "msg decrypt= \t", decrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
        printf(ANSI_FG_GREEN "%s mode success\r\n" ANSI_RESET, tag);
        return true;
    }
}

FURI_CHECK_RETURN
static bool crypto_aes_test_custom_size_key(FuriHalCryptoKeyType type) {
    uint8_t encrypted_buffer[BUFFER_SIZE] = {0};
    uint8_t decrypted_buffer[BUFFER_SIZE] = {0};

    uint8_t message[sizeof(message_const)];
    memcpy(message, message_const, sizeof(message_const));

    size_t length = 0;
    switch(type) {
    case FuriHalCryptoKeyTypeAes128:
        printf("\r\n\r\nCrypto AES 128 bit key\r\n");
        length = FURI_HAL_CRYPTO_AES_KEY_SIZE_128;
        break;
    case FuriHalCryptoKeyTypeAes192:
        printf("\r\n\r\nCrypto AES 192 bit key\r\n");
        length = FURI_HAL_CRYPTO_AES_KEY_SIZE_192;
        break;
    case FuriHalCryptoKeyTypeAes256:
        printf("\r\n\r\nCrypto AES 256 bit key\r\n");
        length = FURI_HAL_CRYPTO_AES_KEY_SIZE_256;
        break;
    default:
        furi_assert(false);
        return false;
    }

    FuriHalCryptoStatus status = furi_hal_crypto_is_key_wrapping_supported();

    bool wrap = false;

    switch(status) {
    case FuriHalCryptoStatusOk:
        wrap = true;
        break;
    case FuriHalCryptoStatusUnavailable:
        printf(ANSI_FG_YELLOW "Key wrapping is unsupported\r\n" ANSI_RESET);
        wrap = false;
        break;
    default:
        printf(ANSI_FG_RED "Key wrapping check failed\r\n" ANSI_RESET);
        return false;
    }

    bool result = true;

    do {
        FuriHalCryptoKey* key = NULL;
        status = furi_hal_crypto_key_init_raw(&key, type, key_const, length);

        CRYPTO_COMMON_CHECK_STATUS(status, "key init");

        uint8_t iv[sizeof(iv_const)];
        memcpy(iv, iv_const, sizeof(iv_const));

        //FuriHalCryptoAesModeECB
        printf("Crypto AES FuriHalCryptoAesModeECB\r\n");
        FuriHalCryptoAes* handle = NULL;
        do {
            status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeECB, key);
            CRYPTO_COMMON_CHECK_STATUS(status, "AES init");

            do {
                status = furi_hal_crypto_aes_encrypt(
                    handle, NULL, message, sizeof(message), encrypted_buffer);
                CRYPTO_COMMON_CHECK_STATUS(status, "AES encrypt");
                status = furi_hal_crypto_aes_decrypt(
                    handle, NULL, encrypted_buffer, sizeof(message), decrypted_buffer);
                CRYPTO_COMMON_CHECK_STATUS(status, "AES decrypt");
                result = crypto_aes_check(
                             "FuriHalCryptoAesModeECB", encrypted_buffer, decrypted_buffer) &&
                         result;
            } while(false);
            furi_hal_crypto_aes_deinit(handle);
        } while(false);

        //FuriHalCryptoAesModeCTR
        printf("Crypto AES FuriHalCryptoAesModeCTR\r\n");
        do {
            status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCTR, key);
            CRYPTO_COMMON_CHECK_STATUS(status, "AES init");
            do {
                status = furi_hal_crypto_aes_encrypt(
                    handle, iv, message, sizeof(message), encrypted_buffer);
                CRYPTO_COMMON_CHECK_STATUS(status, "AES encrypt");
                status = furi_hal_crypto_aes_decrypt(
                    handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
                CRYPTO_COMMON_CHECK_STATUS(status, "AES decrypt");
                result = crypto_aes_check(
                             "FuriHalCryptoAesModeCTR", encrypted_buffer, decrypted_buffer) &&
                         result;
            } while(false);
            furi_hal_crypto_aes_deinit(handle);
        } while(false);

        //FuriHalCryptoAesModeCTR
        printf("Crypto AES FuriHalCryptoAesModeCBC\r\n");

        do {
            status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCBC, key);
            CRYPTO_COMMON_CHECK_STATUS(status, "AES init");

            do {
                status = furi_hal_crypto_aes_encrypt(
                    handle, iv, message, sizeof(message), encrypted_buffer);
                CRYPTO_COMMON_CHECK_STATUS(status, "AES encrypt");
                status = furi_hal_crypto_aes_decrypt(
                    handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
                CRYPTO_COMMON_CHECK_STATUS(status, "AES decrypt");
                result = crypto_aes_check(
                             "FuriHalCryptoAesModeCBC", encrypted_buffer, decrypted_buffer) &&
                         result;
            } while(false);

            furi_hal_crypto_aes_deinit(handle);

        } while(false);

        if(wrap) {
            //Wrap key
            printf("Crypto AES Wrap key\r\n");

            FuriHalCryptoKey* wrapped_key = NULL;
            do {
                status = furi_hal_crypto_wrap_key(key, &wrapped_key);
                CRYPTO_COMMON_CHECK_STATUS(status, "key wrap");

                crypto_common_print_key("Key =\t\t", key);
                crypto_common_print_key("Wrapped key =\t", wrapped_key);

                //FuriHalCryptoAesModeECB, crypt key, decrypt wrapped key
                printf("Crypto AES FuriHalCryptoAesModeECB, Wrap key\r\n");

                do {
                    status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeECB, key);
                    CRYPTO_COMMON_CHECK_STATUS(status, "AES init");
                    do {
                        status = furi_hal_crypto_aes_encrypt(
                            handle, NULL, message, sizeof(message), encrypted_buffer);
                        CRYPTO_COMMON_CHECK_STATUS(status, "AES encrypt");
                    } while(false);
                    furi_hal_crypto_aes_deinit(handle);
                } while(false);

                do {
                    status =
                        furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeECB, wrapped_key);
                    CRYPTO_COMMON_CHECK_STATUS(status, "AES init");
                    do {
                        status = furi_hal_crypto_aes_decrypt(
                            handle, NULL, encrypted_buffer, sizeof(message), decrypted_buffer);
                        CRYPTO_COMMON_CHECK_STATUS(status, "AES decrypt");
                    } while(false);
                    furi_hal_crypto_aes_deinit(handle);
                } while(false);

                result =
                    crypto_aes_check(
                        "FuriHalCryptoAesModeECB, Wrap key", encrypted_buffer, decrypted_buffer) &&
                    result;

                //FuriHalCryptoAesModeCTR, crypt key, decrypt wrapped key
                printf("Crypto AES FuriHalCryptoAesModeCTR, Wrap key\r\n");

                do {
                    status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCTR, key);
                    CRYPTO_COMMON_CHECK_STATUS(status, "AES init");
                    do {
                        status = furi_hal_crypto_aes_encrypt(
                            handle, iv, message, sizeof(message), encrypted_buffer);
                        CRYPTO_COMMON_CHECK_STATUS(status, "AES encrypt");
                    } while(false);
                    furi_hal_crypto_aes_deinit(handle);
                } while(false);

                do {
                    status =
                        furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCTR, wrapped_key);
                    CRYPTO_COMMON_CHECK_STATUS(status, "AES init");
                    do {
                        status = furi_hal_crypto_aes_decrypt(
                            handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
                        CRYPTO_COMMON_CHECK_STATUS(status, "AES decrypt");
                    } while(false);
                    furi_hal_crypto_aes_deinit(handle);
                } while(false);

                result =
                    crypto_aes_check(
                        "FuriHalCryptoAesModeCTR, Wrap key", encrypted_buffer, decrypted_buffer) &&
                    result;

                //FuriHalCryptoAesModeCTR, crypt key, decrypt wrapped key
                printf("Crypto AES FuriHalCryptoAesModeCBC, Wrap key\r\n");

                do {
                    status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCBC, key);
                    CRYPTO_COMMON_CHECK_STATUS(status, "AES init");
                    do {
                        status = furi_hal_crypto_aes_encrypt(
                            handle, iv, message, sizeof(message), encrypted_buffer);
                        CRYPTO_COMMON_CHECK_STATUS(status, "AES encrypt");
                    } while(false);
                    furi_hal_crypto_aes_deinit(handle);
                } while(false);

                do {
                    status =
                        furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCBC, wrapped_key);
                    CRYPTO_COMMON_CHECK_STATUS(status, "AES init");
                    do {
                        status = furi_hal_crypto_aes_decrypt(
                            handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
                        CRYPTO_COMMON_CHECK_STATUS(status, "AES decrypt");
                    } while(false);
                    furi_hal_crypto_aes_deinit(handle);
                } while(false);
                result =
                    crypto_aes_check(
                        "FuriHalCryptoAesModeCBC, Wrap key", encrypted_buffer, decrypted_buffer) &&
                    result;
                furi_hal_crypto_key_free(wrapped_key);
            } while(false);
        }
        furi_hal_crypto_key_free(key);
    } while(false);
    return result;
}

void crypto_aes_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    bool result = true;
    result = crypto_aes_test_custom_size_key(FuriHalCryptoKeyTypeAes128) && result;
    result = crypto_aes_test_custom_size_key(FuriHalCryptoKeyTypeAes192) && result;
    result = crypto_aes_test_custom_size_key(FuriHalCryptoKeyTypeAes256) && result;

    printf("Crypto AES Encryption done\r\n");
    if(result) {
        printf("SUCCESS\r\n");
    } else {
        printf("FAIL\r\n");
    }
}
